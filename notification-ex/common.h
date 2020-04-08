/*
 * Copyright (c) 2019 Samsung Electronics Co., Ltd.
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

#ifndef NOTIFICATION_EX_COMMON_H_
#define NOTIFICATION_EX_COMMON_H_

#include <tizen.h>
#include <gio/gio.h>

#define REGULAR_UID_MIN 5000

namespace notification {

enum NotificationError {
  ERROR_NONE = TIZEN_ERROR_NONE, /**< Success */
  ERROR_INVALID_PARAMETER = TIZEN_ERROR_INVALID_PARAMETER, /**< Invalid parameter */
  ERROR_OUT_OF_MEMORY = TIZEN_ERROR_OUT_OF_MEMORY, /**< Out of memory */
  ERROR_IO_ERROR = TIZEN_ERROR_IO_ERROR, /**< I/O error */
  ERROR_PERMISSION_DENIED = TIZEN_ERROR_PERMISSION_DENIED, /**< Permission denied */
  ERROR_FROM_DB = TIZEN_ERROR_NOTIFICATION | 0x01, /**< Error from DB query */
  ERROR_ALREADY_EXIST_ID = TIZEN_ERROR_NOTIFICATION | 0x02, /**< Already exist private ID */
  ERROR_FROM_DBUS = TIZEN_ERROR_NOTIFICATION | 0x03, /**< Error from DBus */
  ERROR_NOT_EXIST_ID = TIZEN_ERROR_NOTIFICATION | 0x04, /**< Not exist private ID */
  ERROR_SERVICE_NOT_READY = TIZEN_ERROR_NOTIFICATION | 0x05, /**< No response from notification service */
};

GQuark noti_ex_error_quark(void);

}  // namespace notification

#endif  // NOTIFICATION_EX_COMMON_H_
