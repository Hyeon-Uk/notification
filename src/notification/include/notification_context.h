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
 * notification_context.h — N1-T4 internal/private header.
 *
 * Purpose
 *   Introduce an explicit opaque context for notification library state that
 *   has historically lived as `__attribute__((constructor))`-initialized
 *   file-static globals across notification_viewer.c, notification_internal.c
 *   and notification_tidl.c. The context is the seam we need in order to,
 *   over subsequent phases, remove every file-static and make the library
 *   exercise-in-process testable (LeakCheckFixture, forked parallel tests,
 *   deterministic state reset, dependency injection).
 *
 * Scope
 *   This header is *internal only*. It is NOT installed under
 *   /usr/include/notification/ and is explicitly excluded from the -devel
 *   package. The public ABI (notification.h / notification_internal.h) is
 *   unchanged. The opaque handle type is forward-declared via a struct
 *   pointer so we can extend the context layout freely in later phases.
 *
 * Lifetime
 *   The library exposes a lazily-constructed *default* context which mirrors
 *   the legacy per-process file-statics. Every existing public API path
 *   that previously touched those file-statics now goes through the default
 *   context internally, so behavior is preserved. Additionally, callers may
 *   create independent contexts via notification_context_create() and free
 *   them with notification_context_destroy(). Independent contexts do not
 *   share mutable state with the default context (isolation invariant).
 *
 * Thread safety
 *   - The default context is singleton and may be accessed from any thread.
 *   - User-created contexts are not internally synchronized and must be
 *     used from a single thread (or externally serialized).
 */

#ifndef __NOTIFICATION_CONTEXT_H__
#define __NOTIFICATION_CONTEXT_H__

#include <glib.h>
#include <stdbool.h>

#include <notification_error.h>

/*
 * The production library is built with `-fvisibility=hidden`, so internal
 * symbols are not reachable from test binaries. This header is internal
 * and NOT shipped in the devel package, but the unittest binary must link
 * against it; we therefore mark the entry points with default visibility.
 * Equivalent to the pattern used in notification_bundle_util.h.
 */
#ifndef NOTI_CONTEXT_API
#define NOTI_CONTEXT_API __attribute__((visibility("default")))
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* Opaque handle. Definition lives in notification_context.c. */
typedef struct _notification_context* notification_context_h;

/*
 * Create a fresh, independent context. The returned handle owns its own
 * default-viewer string, event-callback list, mutex and connected flag,
 * independent of the default (process-wide) context.
 *
 * On success, *ctx is set and the return value is NOTIFICATION_ERROR_NONE.
 * On invalid parameter (ctx == NULL) the return value is
 * NOTIFICATION_ERROR_INVALID_PARAMETER and *ctx is left untouched.
 * On allocation failure the return value is NOTIFICATION_ERROR_OUT_OF_MEMORY.
 */
NOTI_CONTEXT_API int notification_context_create(notification_context_h* ctx);

/*
 * Destroy a context previously produced by notification_context_create().
 * Destroying the default context handle (if a caller somehow obtained it)
 * is a no-op to avoid process-wide state corruption; the function still
 * returns NOTIFICATION_ERROR_NONE in that case.
 *
 * Passing NULL is NOTIFICATION_ERROR_INVALID_PARAMETER.
 */
NOTI_CONTEXT_API int notification_context_destroy(notification_context_h ctx);

/*
 * Internal accessor for the lazily-constructed process default context.
 * All library internals that previously accessed file-static globals
 * should route through this accessor. It is safe to call from any thread;
 * initialization is performed once via g_once_init_enter/leave.
 *
 * Must NEVER be called from a destructor (`__attribute__((destructor))`)
 * if we are in the process of unloading: the returned context is then
 * being torn down.
 */
NOTI_CONTEXT_API notification_context_h _notification_get_default_context(void);

/*
 * Internal accessors for fields that Phase 1 has migrated onto the context.
 * Each returns a pointer into the context's storage so callers can still
 * read/modify in-place. Phase 2+ will progressively hide these behind
 * dedicated getter/setter functions.
 */
NOTI_CONTEXT_API char**  _notification_context_default_viewer_slot(notification_context_h ctx);
NOTI_CONTEXT_API GList** _notification_context_event_cb_list_slot(notification_context_h ctx);
NOTI_CONTEXT_API bool*   _notification_context_connected_slot(notification_context_h ctx);

#ifdef __cplusplus
}
#endif

#endif /* __NOTIFICATION_CONTEXT_H__ */
