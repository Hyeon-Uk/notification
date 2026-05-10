/*
 * Copyright (c) 2000 - 2017 Samsung Electronics Co., Ltd. All rights reserved.
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <glib.h>
#include <gio/gio.h>

#include <notification_db.h>
#include <notification_debug.h>
#include <notification_ongoing.h>
#include <notification_private.h>

extern "C" {

EXPORT_API
int notification_ongoing_update_cb_set(notification_ongoing_update_cb callback,
					void *user_data)
{
	WARN("notification_ongoing_update_cb_set not working now");

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API
int notification_ongoing_update_cb_unset(void)
{
	WARN("notification_ongoing_update_cb_unset not working now");

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_ongoing_update_progress(const char *caller_app_id,
					int priv_id, double progress)
{
	WARN("notification_ongoing_update_progress not working now");

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_ongoing_update_size(const char *caller_app_id,
					int priv_id, double size)
{
	WARN("notification_ongoing_update_size not working now");

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_ongoing_update_content(const char *caller_app_id,
					int priv_id, const char *content)
{
	WARN("notification_ongoing_update_content not working now");

	return NOTIFICATION_ERROR_NONE;
}

}  // extern "C"
