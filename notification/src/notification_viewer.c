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
#include <stdio.h>
#include <stdlib.h>
#include <glib.h>

#include <app_control_internal.h>
#include <iniparser.h>
#include <aul_svc.h>
#include <aul_app_lifecycle.h>

#include <notification.h>
#include <notification_debug.h>
#include <notification_viewer.h>

#define DEFAULT_VIEWER_CONF_FILE "/usr/share/notification/notification.ini"
static char *_default_viewer;

static GList *_delayed_noti_list;
static int _cb_registered = -1;

typedef struct delayed_noti {
	uid_t uid;
	bundle *noti_info;
} delayed_noti_info_s;

/* LCOV_EXCL_START */
EXPORT_API int notification_init_default_viewer()
{
	const char *viewer;
	dictionary *dict;

	if (_default_viewer != NULL)
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
	if (viewer != NULL)
		_default_viewer = strdup(viewer);

	iniparser_freedict(dict);

	return 0;
}
/* LCOV_EXCL_STOP */

static int __pop_delayed_noti() {
	delayed_noti_info_s *noti = g_list_nth_data(_delayed_noti_list, 0);

	int ret = aul_svc_run_service_async_for_uid(noti->noti_info,
			0, NULL, NULL, noti->uid);
	if (ret < 0) {
		ERR("Failed to request app launch[%d]", ret);
		return ret;
	}

	_delayed_noti_list = g_list_remove(_delayed_noti_list, noti);

	bundle_free(noti->noti_info);
	free(noti);

	return NOTIFICATION_ERROR_NONE;
}

static void __aul_app_lifecycle_state_changed_cb(const char *app_id, pid_t pid,
		aul_app_lifecycle_state_e state, bool has_focus, void *user_data) {
	int ret;

	if (strncmp(app_id, _default_viewer, strlen(app_id)))
		return;

	if (state == AUL_APP_LIFECYCLE_STATE_CREATED ||
		state == AUL_APP_LIFECYCLE_STATE_DESTROYED) {
		while (g_list_length(_delayed_noti_list) != 0) {
			ret = __pop_delayed_noti();
			if (ret != NOTIFICATION_ERROR_NONE) {
				ERR("Failed to pop delayed noti");
				return;
			}
		}

		ret = aul_app_lifecycle_deregister_state_changed_cb();
		if (ret == AUL_R_OK)
			_cb_registered = -1;
		else
			ERR("Failed to deregister lifecycle cb");
	}
}

static int __push_delayed_noti(bundle *noti_info, uid_t uid) {
	delayed_noti_info_s *noti = (delayed_noti_info_s *)calloc(1,
			sizeof(delayed_noti_info_s));
	if (noti == NULL) {
		ERR("Failed to calloc delayed noti info");
		return NOTIFICATION_ERROR_OUT_OF_MEMORY;
	}

	noti->uid = uid;
	noti->noti_info = bundle_dup(noti_info);
	if (noti->noti_info == NULL) {
		ERR("Failed to bundle_dup ");
		free(noti);
		return NOTIFICATION_ERROR_OUT_OF_MEMORY;
	}

	_delayed_noti_list = g_list_append(_delayed_noti_list, noti);

	if (_cb_registered) {
		_cb_registered = aul_app_lifecycle_register_state_changed_cb(
				__aul_app_lifecycle_state_changed_cb, NULL);
		if (_cb_registered)
			ERR("Failed to register app lifecycle cb");
	}

	return NOTIFICATION_ERROR_NONE;
}

/* LCOV_EXCL_START */
EXPORT_API int notification_launch_default_viewer(int priv_id,
		notification_op_type_e status, uid_t uid)
{
	int ret;
	char buf[32] = {0,};
	bundle *b = NULL;

	if (_default_viewer == NULL)
		return NOTIFICATION_ERROR_NONE;

	b = bundle_create();
	if (b == NULL) {
		ERR("Failed to create bundle");
		return NOTIFICATION_ERROR_OUT_OF_MEMORY;
	}

	ret = aul_svc_set_appid(b, _default_viewer);
	if (ret != AUL_SVC_RET_OK) {
		ERR("Failed to set appid to bundle[%x]", ret);
		goto out;
	}

	snprintf(buf, sizeof(buf), "%d", priv_id);

	ret = aul_svc_add_data(b, "NOTIFICATION_PRIVATE_ID", buf);
	if (ret != AUL_SVC_RET_OK) {
		ERR("Failed to add extra_data[%x]", ret);
		goto out;
	}

	memset(buf, 0, sizeof(buf));
	snprintf(buf, sizeof(buf), "%d", status);

	ret = aul_svc_add_data(b, "NOTIFICATION_OP_TYPE", buf);
	if (ret != AUL_SVC_RET_OK) {
		ERR("Failed to add extra_data[%x]", ret);
		goto out;
	}

	ret = aul_svc_run_service_async_for_uid(b, 0, NULL, NULL, uid);
	if (ret < 0) {
		ERR("Failed to request app launch[%d]", ret);
		if (__push_delayed_noti(b, uid) != NOTIFICATION_ERROR_NONE)
			ERR("Failed to push delayed noti");
	} else {
		INFO("successed to request app launch[%d],[%d]",
				ret, uid);
		ret = APP_CONTROL_ERROR_NONE;
	}

out:
	if (b)
		bundle_free(b);

	if (ret == 0)
		ret = NOTIFICATION_ERROR_NONE;
	else
		ret = NOTIFICATION_ERROR_IO_ERROR;

	return ret;
}
/* LCOV_EXCL_STOP */
