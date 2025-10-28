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

#ifndef __NOTIFICATION_ONGOING_H__
#define __NOTIFICATION_ONGOING_H__

#include <notification.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @file notification_ongoing.h
 */

typedef enum {
	ONGOING_TYPE_PROGRESS,
	ONGOING_TYPE_SIZE,
	ONGOING_TYPE_CONTENT,
} ongoing_type_e;

struct ongoing_info_s {
	char *pkgname;
	int priv_id;
	ongoing_type_e type;
	double progress;
	double size;
	char *content;
};

/**
 * @internal
 * @brief Called when a notification ongoing data is updated.
 * @since_tizen 3.0
 * @param[in] info    The ongoing information handle
 * @param[in] data    The user data
 * @pre notification_ongoing_update_cb_set() used to register this callback.
 * @see notification_ongoing_update_cb_set()
*/
typedef void (*notification_ongoing_update_cb)(struct ongoing_info_s *info, void *data);

/**
 * @internal
 * @brief Registers a callback to receive the ongoing progress, size ,content.
 * @since_tizen 3.0
 * @param[in] callback The callback function
 * @param[in] data     The user_data
 * @return #NOTIFICATION_ERROR_NONE if success, other value if failure
 * @retval #NOTIFICATION_ERROR_NONE Success
 * @retval #NOTIFICATION_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #NOTIFICATION_ERROR_FROM_DBUS Error from DBus
 */
int notification_ongoing_update_cb_set(notification_ongoing_update_cb callback, void *user_data);

/**
 * @internal
 * @brief Unregisters a callback to receive.
 * @since_tizen 3.0
 * @retval #NOTIFICATION_ERROR_NONE Success
 */
int notification_ongoing_update_cb_unset(void);

/**
 * @brief Updates progress.
 * @since_tizen 2.3
 * @param[in] caller_app_id
 * @param[in] priv_id
 * @param[in] progress
 * @return #NOTIFICATION_ERROR_NONE on success,
 *         otherwise any other value on failure
 * @retval #NOTIFICATION_ERROR_NONE         Success
 * @retval #NOTIFICATION_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #NOTIFICATION_ERROR_FROM_DBUS Error from DBus
 */
int notification_ongoing_update_progress(const char *caller_app_id,
					int priv_id, double progress);

/**
 * @brief Updates size.
 * @since_tizen 2.3
 * @param[in] caller_app_id
 * @param[in] priv_id
 * @param[in] size
 * @return #NOTIFICATION_ERROR_NONE on success,
 *         otherwise any other value on failure
 * @retval #NOTIFICATION_ERROR_NONE         Success
 * @retval #NOTIFICATION_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #NOTIFICATION_ERROR_FROM_DBUS Error from DBus
 */
int notification_ongoing_update_size(const char *caller_app_id,
				int priv_id, double size);

/**
 * @brief Updates content.
 * @since_tizen 2.3
 * @param[in] caller_app_id
 * @param[in] priv_id
 * @param[in] content
 * @return #NOTIFICATION_ERROR_NONE on success,
 *         otherwise any other value on failure
 * @retval #NOTIFICATION_ERROR_NONE         Success
 * @retval #NOTIFICATION_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #NOTIFICATION_ERROR_FROM_DBUS Error from DBus
 */
int notification_ongoing_update_content(const char *caller_app_id,
				int priv_id, const char *content);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __NOTIFICATION_ONGOING_H__ */

