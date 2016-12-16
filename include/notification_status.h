/*
 * Copyright (c) 2000 - 2016 Samsung Electronics Co., Ltd. All rights reserved.
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


#ifndef __NOTIFICATION_STATUS_DEF_H__
#define __NOTIFICATION_STATUS_DEF_H__


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/**
 * @file notification_status.h
 */


/**
 * @addtogroup NOTIFICATION_STATUS
 * @{
 */


/**
 * @brief Shows a toast popup window with given message.
 * @since_tizen @if WEARABLE 2.3.1 @elseif MOBILE 2.3 @endif
 * @param[in] message The messages to be posted
 * @return #NOTIFICATION_ERROR_NONE on success,
 *         otherwise any other value on failure
 * @retval #NOTIFICATION_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #NOTIFICATION_ERROR_FROM_DBUS Error from DBus
 * @see #notification_error_e
 */
int notification_status_message_post(const char *message);


/**
 * @}
 */


#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __NOTIFICATION_STATUS_DEF_H__ */

