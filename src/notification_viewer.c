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

#include <app_control_internal.h>
#include <iniparser.h>

#include <notification.h>
#include <notification_debug.h>
#include <notification_viewer.h>

EXPORT_API int notification_get_default_viewer(const char *path, char **default_viewer)
{
	char *viewer = NULL;
	dictionary *dict = NULL;

	if (access(path, F_OK) != 0) {
		NOTIFICATION_ERR("can't access file_path(%s)", path);
		return -1;
	}

	dict = iniparser_load(path);
	if (!dict) {
		NOTIFICATION_ERR("can't load file");
		return -1;
	}

	viewer = iniparser_getstring(dict, "Notification:DefaultViewer", NULL);
	if (viewer != NULL)
		*default_viewer = strdup(viewer);

	iniparser_freedict(dict);

	return 0;
}

EXPORT_API int notification_launch_default_viewer(const char *default_viewer, int priv_id)
{
	int ret;
	char buf[32] = {0,};
	app_control_h app_control = NULL;

	ret = app_control_create(&app_control);
	if (ret != APP_CONTROL_ERROR_NONE) {
		NOTIFICATION_ERR("app_control_create failed [%x]", ret);
		goto out;
	}

	ret = app_control_set_app_id(app_control, default_viewer);
	if (ret != APP_CONTROL_ERROR_NONE) {
		NOTIFICATION_ERR("app_control_set_app_id failed [%x]", ret);
		goto out;
	}

	snprintf(buf, sizeof(buf), "%d", priv_id);

	ret = app_control_add_extra_data(app_control, "NOTIFICATION_PRIVATE_ID", buf);
	if (ret != APP_CONTROL_ERROR_NONE) {
		NOTIFICATION_ERR("app_control_set_extra_data failed [%x]", ret);
		goto out;
	}

	ret = app_control_send_launch_request(app_control, NULL, NULL);
	if (ret != APP_CONTROL_ERROR_NONE) {
		NOTIFICATION_ERR("app_control_send_launch_request failed [%x]", ret);
		goto out;
	}

out:
	if (app_control)
		app_control_destroy(app_control);

	if (ret == APP_CONTROL_ERROR_NONE)
		ret = NOTIFICATION_ERROR_NONE;
	else
		ret = NOTIFICATION_ERROR_IO_ERROR;

	return ret;
}
