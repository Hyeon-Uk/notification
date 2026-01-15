/*
 * Copyright (c) 2019 Samsung Electronics Co., Ltd. All rights reserved.
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


#ifndef __TIZEN_APPFW_NOTIFICATION_EX_ERROR_H__
#define __TIZEN_APPFW_NOTIFICATION_EX_ERROR_H__


#include <tizen.h>


/**
 * @file notification_ex_error.h
 */


/**
 * @addtogroup NOTIFICATION_EX_MODULE
 * @{
 */


/**
 * @deprecated Deprecated since 11.0.
 * @brief Enumeration for notification errors.
 * @since_tizen 5.5
 */
typedef enum _noti_ex_error {
	NOTI_EX_ERROR_NONE = TIZEN_ERROR_NONE, /**< Success */
	NOTI_EX_ERROR_INVALID_PARAMETER = TIZEN_ERROR_INVALID_PARAMETER, /**< Invalid parameter */
	NOTI_EX_ERROR_OUT_OF_MEMORY = TIZEN_ERROR_OUT_OF_MEMORY, /**< Out of memory */
	NOTI_EX_ERROR_IO_ERROR = TIZEN_ERROR_IO_ERROR, /**< I/O error */
	NOTI_EX_ERROR_PERMISSION_DENIED = TIZEN_ERROR_PERMISSION_DENIED, /**< Permission denied */
	NOTI_EX_ERROR_FROM_DB = TIZEN_ERROR_NOTIFICATION | 0x01, /**< Error from DB query */
	NOTI_EX_ERROR_ALREADY_EXIST_ID = TIZEN_ERROR_NOTIFICATION | 0x02, /**< Already exist private ID */
	NOTI_EX_ERROR_FROM_DBUS = TIZEN_ERROR_NOTIFICATION | 0x03, /**< Error from DBus */
	NOTI_EX_ERROR_NOT_EXIST_ID = TIZEN_ERROR_NOTIFICATION | 0x04, /**< Not exist private ID */
	NOTI_EX_ERROR_SERVICE_NOT_READY = TIZEN_ERROR_NOTIFICATION | 0x05, /**< No response from notification service */
} noti_ex_error_e;


/**
 * @}
 */


#endif /* __TIZEN_APPFW_NOTIFICATION_EX_ERROR_H__ */
