/*
 * Copyright (c) 2020 Samsung Electronics Co., Ltd.
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

#include <string.h>

#include "notification-ex/common.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "NOTIFICATION_EX"
#define NOTIFICATION_EX_ERROR_QUARK "notification-ex-error-quark"

namespace notification {

static const GDBusErrorEntry dbus_error_entries[] = {
  {ERROR_INVALID_PARAMETER, "org.freedesktop.Notification-ex.Error.INVALID_PARAMETER"},
  {ERROR_OUT_OF_MEMORY,     "org.freedesktop.Notification-ex.Error.OUT_OF_MEMORY"},
  {ERROR_IO_ERROR,          "org.freedesktop.Notification-ex.Error.IO_ERROR"},
  {ERROR_PERMISSION_DENIED, "org.freedesktop.Notification-ex.Error.PERMISSION_DENIED"},
  {ERROR_FROM_DB,           "org.freedesktop.Notification-ex.Error.FROM_DB"},
  {ERROR_ALREADY_EXIST_ID,  "org.freedesktop.Notification-ex.Error.ALREADY_EXIST_ID"},
  {ERROR_FROM_DBUS,         "org.freedesktop.Notification-ex.Error.FROM_DBUS"},
  {ERROR_NOT_EXIST_ID,      "org.freedesktop.Notification-ex.Error.NOT_EXIST_ID"},
  {ERROR_SERVICE_NOT_READY, "org.freedesktop.Notification-ex.Error.SERVICE_NOT_READY"},
};

GQuark noti_ex_error_quark(void) {
  static volatile gsize quark_volatile = 0;
  static const char *domain_name = NULL;

  GQuark quark = g_quark_try_string(NOTIFICATION_EX_ERROR_QUARK);
  if (quark == 0) {
    if (domain_name == NULL)
      domain_name = strdup(NOTIFICATION_EX_ERROR_QUARK);
  } else {
    domain_name = NOTIFICATION_EX_ERROR_QUARK;
  }

  g_dbus_error_register_error_domain(domain_name, &quark_volatile,
        dbus_error_entries, G_N_ELEMENTS(dbus_error_entries));
  return (GQuark)quark_volatile;
}

}  // namespace notification
