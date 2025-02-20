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
#include <unistd.h>

#include <notification.h>
#include <notification_debug.h>
#include <notification_viewer.h>

#define DEFAULT_VIEWER_CONF_FILE "/usr/share/notification/notification.ini"
static char *_default_viewer;
static GList *_delayed_noti_list;
static guint _timer;

static void __app_control_result_cb(app_control_h app_control,
		app_control_error_e result, void *user_data);

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

/* LCOV_EXCL_START */
static gboolean __pop_delayed_noti_cb(gpointer user_data)
{
	app_control_h app_control;
	int ret;

	if (_delayed_noti_list == NULL) {
		_timer = 0;
		return G_SOURCE_REMOVE;
	}

	app_control = g_list_nth_data(_delayed_noti_list, 0);
	ret = app_control_send_launch_request_async(app_control,
			__app_control_result_cb, NULL, NULL);
	if (ret != APP_CONTROL_ERROR_NONE) {
		ERR("Failed to send launch request. error(%d)", ret);
		return G_SOURCE_CONTINUE;
	}

	_delayed_noti_list = g_list_remove(_delayed_noti_list, app_control);
	app_control_destroy(app_control);
	return G_SOURCE_CONTINUE;
}

static int __push_delayed_noti(app_control_h app_control)
{
	app_control_h handle = NULL;
	int ret;

	ret = app_control_clone(&handle, app_control);
	if (ret != APP_CONTROL_ERROR_NONE) {
		ERR("Failed to clone app control. error(%d)", ret);
		return NOTIFICATION_ERROR_OUT_OF_MEMORY;
	}

	_delayed_noti_list = g_list_append(_delayed_noti_list, handle);

	if (_timer == 0)
		_timer = g_timeout_add(100, __pop_delayed_noti_cb, NULL);

	return NOTIFICATION_ERROR_NONE;
}
/* LCOV_EXCL_STOP */

static void __app_control_result_cb(app_control_h app_control,
		app_control_error_e result, void *user_data)
{
	bundle *b = NULL;

	INFO("result(%d)", result);
	if (result != APP_CONTROL_ERROR_NONE) {
		ERR("Failed to send launch request. error(%d)", result);
		if (__push_delayed_noti(app_control) != NOTIFICATION_ERROR_NONE)
			ERR("Failed to push delayed noti");
	}
}

/* LCOV_EXCL_START */
EXPORT_API int notification_launch_default_viewer(int priv_id,
		notification_op_type_e status, uid_t uid)
{
	int ret;
	char buf[32] = {0,};
	bundle *b = NULL;
	app_control_h app_control = NULL;

	if (_default_viewer == NULL)
		return NOTIFICATION_ERROR_NONE;

	ret = app_control_create(&app_control);
	if (ret != APP_CONTROL_ERROR_NONE) {
		ERR("Failed to create app control. error(%d)", ret);
		return NOTIFICATION_ERROR_OUT_OF_MEMORY;
	}

	ret = app_control_set_app_id(app_control, _default_viewer);
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
/* LCOV_EXCL_STOP */
