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

#ifndef __NOTIFICATION_STATUS_INTERNAL_DEF_H__
#define __NOTIFICATION_STATUS_INTERNAL_DEF_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @file notification_status_internal.h
 */

/**
 * @addtogroup NOTIFICATION_STATUS
 * @{
 */

/**
 * @internal
 * @brief Called when a new message is posted.
 * @since_tizen 2.3
 * @param[in] message The message posted
 * @param[in] data    The user data
 * @pre notification_status_monitor_message_cb_set() used to register this callback.
 * @see notification_status_monitor_message_cb_set()
*/
typedef void (*notification_status_message_cb)(const char *message, void *data);

/**
 * @internal
 * @brief Registers a callback to receive a message.
 * @since_tizen 2.3
 * @param[in] callback The callback function
 * @param[in] data     The user_data
 * @return #NOTIFICATION_ERROR_NONE if success, other value if failure
 * @retval #NOTIFICATION_ERROR_NONE Success
 * @retval #NOTIFICATION_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #NOTIFICATION_ERROR_FROM_DBUS Error from DBus
 */
int notification_status_monitor_message_cb_set(notification_status_message_cb callback, void *user_data);

/**
 * @internal
 * @brief Unregisters a callback to receive a message.
 * @since_tizen 2.3
 * @param[in] callback The callback function
 * @param[in] data     The user_data
 * @return #NOTIFICATION_ERROR_NONE if success, other value if failure
 * @retval #NOTIFICATION_ERROR_NONE Success
 */
int notification_status_monitor_message_cb_unset(void);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __NOTIFICATION_STATUS_INTERNAL_DEF_H__ */

