/*
 * Copyright (c) 2017 Samsung Electronics Co., Ltd. All rights reserved.
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

#define _GNU_SOURCE
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <glib.h>

#include <app_control_internal.h>
#include <iniparser.h>
#include <unistd.h>
#include <tizen_core.h>

#include <notification.h>
#include <notification_debug.h>
#include <notification_viewer.h>

#include "notification_context.h"

/*
 * N8-A.2 (Stage 8): migrated from C to C++.  The two EXPORT_API entry
 * points (`notification_launch_default_viewer`,
 * `notification_launch_default_viewer_without_candidate_process`) and the
 * `notification_init_default_viewer` setup helper are advertised with C
 * linkage in notification_viewer.h.  Static helpers (constructor /
 * destructor, mutex, delayed-noti queue, default-viewer slot, app-control
 * launch helper) are wrapped together inside the same `extern "C"` block
 * so the public symbols stay unmangled.  `<stdio.h>`/`<stdlib.h>` swap to
 * the C++ `<cstdio>`/`<cstdlib>`; the implicit `void *` → typed-pointer
 * assignment from `g_list_nth_data` is now an explicit cast as C++
 * requires.
 */

#define DEFAULT_VIEWER_CONF_FILE "/usr/share/notification/notification.ini"

#define NOTIFICATION_VIEWER_CTOR __attribute__((constructor))
#define NOTIFICATION_VIEWER_DTOR __attribute__((destructor))

#define DELAY_LIMIT 50

extern "C" {

/* N1-T4: The default-viewer string now lives on the lazily-constructed
 * default notification context. This helper returns a writable pointer
 * to the slot (or NULL if the default context could not be allocated,
 * which matches the pre-Phase 1 early-exit behavior). */
static char **__default_viewer_slot(void)
{
	notification_context_h ctx = _notification_get_default_context();
	if (ctx == NULL)
		return NULL;
	return _notification_context_default_viewer_slot(ctx);
}

static GList *_delayed_noti_list;
static bool _is_timer_added = false;
static GRecMutex __rec_mutex;

static void __app_control_result_cb(app_control_h app_control,
		app_control_error_e result, void *user_data);

NOTIFICATION_VIEWER_CTOR static void __notification_init(void)
{
	g_rec_mutex_init(&__rec_mutex);
}

NOTIFICATION_VIEWER_DTOR static void __notification_fini(void)
{
	char **slot;

	g_rec_mutex_clear(&__rec_mutex);

	/* L10: free the lazily-populated default viewer string. After N1-T4
	 * the string lives on the default context, but we must not touch the
	 * context in an unload destructor — the once-init gate may have run
	 * on another thread that no longer exists. Only free if the slot is
	 * reachable AND populated. */
	slot = __default_viewer_slot();
	if (slot != NULL && *slot != NULL) {
		free(*slot);
		*slot = NULL;
	}
}

static void __notification_mutex_lock(void)
{
	g_rec_mutex_lock(&__rec_mutex);
}

static void __notification_mutex_unlock(void)
{
	g_rec_mutex_unlock(&__rec_mutex);
}

/* LCOV_EXCL_START */
EXPORT_API int notification_init_default_viewer()
{
	const char *viewer;
	dictionary *dict;
	char **slot;

	slot = __default_viewer_slot();
	if (slot == NULL) {
		ERR("default notification context unavailable");
		return -1;
	}

	if (*slot != NULL)
		return 0;

	if (access(DEFAULT_VIEWER_CONF_FILE, F_OK) != 0) {
		ERR("can't access file_path(%s)", DEFAULT_VIEWER_CONF_FILE);
		return -1;
	}

	dict = iniparser_load(DEFAULT_VIEWER_CONF_FILE);
	if (!dict) {
		ERR("can't load file");
		return -1;
	}

	viewer = iniparser_getstring(dict, "Notification:DefaultViewer", NULL);
	/* L10: guard against overwriting an already-populated value, and
	 * therefore leaking the previously-duplicated string if
	 * notification_init_default_viewer() is invoked twice. The
	 * NOTIFICATION_VIEWER_DTOR frees on unload. */
	if (viewer != NULL && *slot == NULL)
		*slot = strdup(viewer);

	iniparser_freedict(dict);

	return 0;
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
static void __check_limit()
{
	app_control_h app_control;
	char *priv_id = NULL;
	int ret;
	guint count = g_list_length(_delayed_noti_list);

	INFO("_delayed_noti_list length : [%u]", count);
	if (count <= DELAY_LIMIT)
		return;

	app_control = (app_control_h)_delayed_noti_list->data;

	ret = app_control_get_extra_data(app_control, "NOTIFICATION_PRIVATE_ID",
			&priv_id);
	if (ret != APP_CONTROL_ERROR_NONE)
		ERR("app_control_get_extra_data failed:[%d]", ret);

	ERR("_delayed_noti_list has exceeded the maximum limit." \
			"[del priv id : %s]", priv_id);
	free(priv_id);

	_delayed_noti_list = g_list_remove(_delayed_noti_list, app_control);
	app_control_destroy(app_control);
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
static bool __pop_delayed_noti_cb(void *user_data)
{
	app_control_h app_control;
	int ret;

	WARN("__pop_delayed_noti_cb");

	__notification_mutex_lock();

	if (_delayed_noti_list == NULL) {
		WARN("pop delayed noti not exist");
		_is_timer_added = false;
		__notification_mutex_unlock();
		return false;
	}

	app_control = static_cast<app_control_h>(
			g_list_nth_data(_delayed_noti_list, 0));
	ret = app_control_send_launch_request_async(app_control,
			__app_control_result_cb, NULL, NULL);
	if (ret != APP_CONTROL_ERROR_NONE) {
		ERR("Failed to send launch request. error(%d)", ret);
		__notification_mutex_unlock();
		return true;
	}

	_delayed_noti_list = g_list_remove(_delayed_noti_list, app_control);
	app_control_destroy(app_control);
	__notification_mutex_unlock();

	WARN("__pop_delayed_noti_cb done");

	return true;
}

static int __push_delayed_noti(app_control_h app_control)
{
	app_control_h handle = NULL;
	tizen_core_h core = NULL;
	tizen_core_source_h source = NULL;
	int ret;

	WARN("__push_delayed_noti");

	__notification_mutex_lock();

	ret = app_control_clone(&handle, app_control);
	if (ret != APP_CONTROL_ERROR_NONE) {
		ERR("Failed to clone app control. error(%d)", ret);
		__notification_mutex_unlock();
		return NOTIFICATION_ERROR_OUT_OF_MEMORY;
	}

	_delayed_noti_list = g_list_append(_delayed_noti_list, handle);
	__check_limit();

	if (!_is_timer_added) {
		ret = tizen_core_find("data-provider-master", &core);
		if (ret != TIZEN_CORE_ERROR_NONE) {
			LOGE("Failed to find data-provider-master core. ret(%d)", ret);
			__notification_mutex_unlock();
			return NOTIFICATION_ERROR_IO_ERROR;
		}

		ret = tizen_core_add_timer(core, 100, __pop_delayed_noti_cb, NULL, &source);
		if (ret != TIZEN_CORE_ERROR_NONE) {
			LOGE("Failed to add timer. ret(%d)", ret);
			__notification_mutex_unlock();
			return NOTIFICATION_ERROR_IO_ERROR;
		}

		_is_timer_added = true;
	}
	__notification_mutex_unlock();

	WARN("__push_delayed_noti done");

	return NOTIFICATION_ERROR_NONE;
}

static bool __push_delayed_noti_task(void *user_data)
{
	app_control_h app_control = (app_control_h)user_data;

	if (__push_delayed_noti(app_control) != NOTIFICATION_ERROR_NONE)
		ERR("Failed to push noti");

	app_control_destroy(app_control);

	return false;
}
/* LCOV_EXCL_STOP */

static void __app_control_result_cb(app_control_h app_control,
		app_control_error_e result, void *user_data)
{
	int ret;
	tizen_core_h core = NULL;
	tizen_core_source_h source = NULL;
	app_control_h cloned;

	INFO("result(%d)", result);
	if (result != APP_CONTROL_ERROR_NONE) {
		ret = app_control_clone(&cloned, app_control);
		if (ret != APP_CONTROL_ERROR_NONE) {
			ERR("Failed to clone app control. error(%d)", ret);
			return;
		}

		ret = tizen_core_find("data-provider-master", &core);
		if (ret != TIZEN_CORE_ERROR_NONE) {
			LOGE("Failed to find data-provider-master core. ret(%d)", ret);
			app_control_destroy(cloned);
			return;
		}

		ret = tizen_core_add_idle_job(core, __push_delayed_noti_task,
				cloned, &source);
		if (ret != TIZEN_CORE_ERROR_NONE) {
			LOGE("Failed to add idle job. ret(%d)", ret);
			app_control_destroy(cloned);
			return;
		}
	}
}

/* LCOV_EXCL_START */
static int __launch_default_viewer(int priv_id,
		notification_op_type_e status, uid_t uid, bool candidate)
{
	int ret;
	char buf[32] = {0,};
	app_control_h app_control = NULL;
	char **slot = __default_viewer_slot();
	const char *default_viewer = (slot != NULL) ? *slot : NULL;

	if (default_viewer == NULL)
		return NOTIFICATION_ERROR_NONE;

	ret = app_control_create(&app_control);
	if (ret != APP_CONTROL_ERROR_NONE) {
		ERR("Failed to create app control. error(%d)", ret);
		return NOTIFICATION_ERROR_OUT_OF_MEMORY;
	}

	ret = app_control_set_app_id(app_control, default_viewer);
	if (ret != APP_CONTROL_ERROR_NONE) {
		ERR("Failed to set appid to app_control. error(%d)", ret);
		ret = NOTIFICATION_ERROR_OUT_OF_MEMORY;
		goto out;
	}

	snprintf(buf, sizeof(buf), "%d", priv_id);
	ret = app_control_add_extra_data(app_control, "NOTIFICATION_PRIVATE_ID",
			buf);
	if (ret != APP_CONTROL_ERROR_NONE) {
		ERR("Failed to add extra data. error(%d)", ret);
		ret = NOTIFICATION_ERROR_OUT_OF_MEMORY;
		goto out;
	}

	memset(buf, 0, sizeof(buf));
	snprintf(buf, sizeof(buf), "%d", status);
	ret = app_control_add_extra_data(app_control, "NOTIFICATION_OP_TYPE",
			buf);
	if (ret != APP_CONTROL_ERROR_NONE) {
		ERR("Failed to add operation type. error(%d)", ret);
		ret = NOTIFICATION_ERROR_OUT_OF_MEMORY;
		goto out;
	}

	/* for candidate */
	if (candidate) {
		WARN("set candidate for viewer");
		ret = app_control_add_extra_data(app_control,
				"__K_EX_USE_CANDIDATE_PROCESS", "true");
		if (ret != APP_CONTROL_ERROR_NONE) {
			ERR("Failed to set candidate key(%d)", ret);
		}
	}

	if (_delayed_noti_list != NULL) {
		ret = __push_delayed_noti(app_control);
		if (ret != NOTIFICATION_ERROR_NONE)
			ERR("Failed to push delayed noti");
		else
			__pop_delayed_noti_cb(NULL);
	} else {
		ret = app_control_send_launch_request_async(app_control,
				__app_control_result_cb, NULL, NULL);
		if (ret != APP_CONTROL_ERROR_NONE) {
			ERR("Failed to send launch request. error(%d)", ret);
			ret = __push_delayed_noti(app_control);
			if (ret != NOTIFICATION_ERROR_NONE)
				ERR("Failed to push delayed noti");
		} else {
			INFO("Successful");
		}
	}

out:
	if (app_control)
		app_control_destroy(app_control);

	return ret;
}

EXPORT_API int notification_launch_default_viewer(int priv_id,
		notification_op_type_e status, uid_t uid)
{
	return __launch_default_viewer(priv_id, status, uid, true);
}

EXPORT_API int notification_launch_default_viewer_without_candidate_process(
		int priv_id, notification_op_type_e status, uid_t uid)
{
	return __launch_default_viewer(priv_id, status, uid, false);
}
/* LCOV_EXCL_STOP */

}  // extern "C"
