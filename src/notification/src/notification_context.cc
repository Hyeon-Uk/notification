/*
 * Copyright (c) 2026 Samsung Electronics Co., Ltd. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * notification_context.c — N1-T4 C API context seam implementation.
 *
 * See notification_context.h for rationale and the public-internal API
 * surface. This implementation provides:
 *
 *   1. The opaque `_notification_context` struct, mirroring the file-static
 *      state that previously lived as `__attribute__((constructor))`-
 *      initialized globals in notification_viewer.c, notification_internal.c
 *      and notification_tidl.c. For Phase 1 we intentionally mirror a small
 *      but representative slice (default-viewer string, event-cb list,
 *      connected flag, per-context recursive mutex). Additional fields will
 *      migrate in later phases without ABI churn because the struct is
 *      opaque.
 *
 *   2. Lazy default-context construction via g_once_init_enter/leave. The
 *      default context is owned by the library and is NOT destroyable via
 *      notification_context_destroy() — destroying it would invalidate any
 *      in-flight public API call that was mid-flight on another thread.
 *
 *   3. Creation / destruction of independent user contexts. User contexts
 *      own their storage (the default-viewer strdup, the GList event CBs,
 *      the mutex) and must not share mutable state with the default context.
 *      The ContextIsolation_TwoContextsDoNotShare TEST asserts this
 *      invariant.
 *
 * ABI invariants
 *   - Nothing in this file is EXPORT_API. All public notification APIs
 *     (notification.h, notification_internal.h, notification_viewer.h) keep
 *     their pre-Phase 1 signatures; the new context surface is opt-in and
 *     internal.
 *
 * Thread safety
 *   - g_once_init_enter/leave serialize first-time construction of the
 *     default context.
 *   - Per-context access is guarded by ctx->mutex (recursive, matching the
 *     legacy GRecMutex pattern used in the migrated modules).
 *
 * N8-A.1 (Stage 8): migrated from C to C++.  Every helper advertised in
 * notification_context.h carries C linkage via the header's `extern "C"`
 * guard; the definitions below live inside a matching `extern "C"`
 * block so the language linkage stays consistent.  File-static
 * helpers (`__alloc_context` / `__free_context`) and the file-static
 * gate (`__default_ctx_init` / `__default_ctx`) stay inside the block
 * to avoid forward-decl/definition language-linkage drift.  The C
 * `<stdbool.h>` is dropped (`bool` is a built-in C++ type);
 * `<stdlib.h>` / `<string.h>` swap to `<cstdlib>` / `<cstring>`.
 * `calloc`'s implicit `void*` cast becomes an explicit
 * `static_cast<struct _notification_context *>` per the dpm D8-D
 * pattern.
 */

#include <cstdlib>
#include <cstring>

#include <glib.h>

#include <notification_error.h>
#include <notification_debug.h>

#include "notification_context.h"

extern "C" {

struct _notification_context {
	/* Default-viewer app-id string, mirrored from notification_viewer.c's
	 * file-static `_default_viewer`. NULL until
	 * notification_init_default_viewer() populates it. Owned by the
	 * context; released in notification_context_destroy() for user
	 * contexts, and in the library destructor for the default context. */
	char *default_viewer;

	/* Event-callback list, mirroring __noti_event_cb_list in
	 * notification_internal.c. Each node is a caller-allocated cb-info
	 * struct; destroy() frees the list nodes but not the payload, matching
	 * the legacy ownership contract where call sites free their own cb
	 * infos on removal. */
	GList *event_cb_list;

	/* TIDL connected flag, mirroring `_connected` in notification_tidl.c.
	 * True once the RPC proxy has emitted its connected_cb. */
	bool connected;

	/* Per-context recursive mutex. Matches the GRecMutex pattern used by
	 * every legacy module that uses file-static state. */
	GRecMutex mutex;

	/* True if this context is the lazily-constructed process default. We
	 * use this to make notification_context_destroy() a no-op on the
	 * default context — destroying process-wide state mid-flight would
	 * invalidate in-progress public API calls on other threads. */
	bool is_default;
};

/*
 * Lazy-init of the singleton default context. g_once_init_enter guarantees
 * the allocation happens exactly once even under concurrent first-callers.
 */
static gsize __default_ctx_init = 0;
static struct _notification_context *__default_ctx = NULL;

static struct _notification_context *__alloc_context(bool is_default)
{
	struct _notification_context *ctx =
			static_cast<struct _notification_context *>(calloc(1,
					sizeof(struct _notification_context)));
	if (ctx == NULL)
		return NULL;

	ctx->default_viewer = NULL;
	ctx->event_cb_list = NULL;
	ctx->connected = false;
	ctx->is_default = is_default;
	g_rec_mutex_init(&ctx->mutex);
	return ctx;
}

static void __free_context(struct _notification_context *ctx)
{
	if (ctx == NULL)
		return;

	g_rec_mutex_clear(&ctx->mutex);

	if (ctx->default_viewer != NULL) {
		free(ctx->default_viewer);
		ctx->default_viewer = NULL;
	}

	if (ctx->event_cb_list != NULL) {
		/* We only own the list spine; payloads are owned by callers
		 * that registered the callbacks (matching the legacy
		 * __noti_event_cb_list ownership contract). If callers leak
		 * their cb payloads that is a pre-existing bug, not one
		 * introduced by this shim. */
		g_list_free(ctx->event_cb_list);
		ctx->event_cb_list = NULL;
	}

	free(ctx);
}

notification_context_h _notification_get_default_context(void)
{
	if (g_once_init_enter(&__default_ctx_init)) {
		struct _notification_context *ctx = __alloc_context(true);
		if (ctx == NULL) {
			/* Leave the once-gate unset so a later allocation can
			 * retry. Report to the debug channel; the library
			 * cannot proceed meaningfully without a default ctx. */
			ERR("failed to allocate default notification context");
			g_once_init_leave(&__default_ctx_init, 0);
			return NULL;
		}
		__default_ctx = ctx;
		g_once_init_leave(&__default_ctx_init, 1);
	}
	return __default_ctx;
}

int notification_context_create(notification_context_h *ctx)
{
	struct _notification_context *new_ctx;

	if (ctx == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	new_ctx = __alloc_context(false);
	if (new_ctx == NULL)
		return NOTIFICATION_ERROR_OUT_OF_MEMORY;

	*ctx = new_ctx;
	return NOTIFICATION_ERROR_NONE;
}

int notification_context_destroy(notification_context_h ctx)
{
	if (ctx == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	/* Refuse to tear down the singleton default context. Returning
	 * NOTIFICATION_ERROR_NONE keeps callers happy if they call destroy()
	 * on a handle they obtained via _notification_get_default_context().
	 * The default context lives for the full process lifetime. */
	if (ctx->is_default)
		return NOTIFICATION_ERROR_NONE;

	__free_context(ctx);
	return NOTIFICATION_ERROR_NONE;
}

char **_notification_context_default_viewer_slot(notification_context_h ctx)
{
	if (ctx == NULL)
		return NULL;
	return &ctx->default_viewer;
}

GList **_notification_context_event_cb_list_slot(notification_context_h ctx)
{
	if (ctx == NULL)
		return NULL;
	return &ctx->event_cb_list;
}

bool *_notification_context_connected_slot(notification_context_h ctx)
{
	if (ctx == NULL)
		return NULL;
	return &ctx->connected;
}

}  // extern "C"
