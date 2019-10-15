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
#include <aul_svc.h>

#include <notification.h>
#include <notification_debug.h>
#include <notification_viewer.h>

#define DEFAULT_VIEWER_CONF_FILE "/usr/share/notification/notification.ini"
static char *_default_viewer;

/* LCOV_EXCL_START */
EXPORT_API int notification_init_default_viewer()
{
	char *viewer = NULL;
	dictionary *dict = NULL;

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
