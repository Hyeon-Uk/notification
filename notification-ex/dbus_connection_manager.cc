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

#include <dlog.h>
#include <glib.h>

#include "notification-ex/exception.h"
#include "notification-ex/dbus_connection_manager.h"
#include "notification-ex/ex_util.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "NOTIFICATION_EX"

#define NOTIFICATION_EX_BUS_NAME_PREFIX "org.tizen.notification_ex._"
#define NOTIFICATION_EX_INTERFACE "org.tizen.notification_ex"
#define DATA_PROVIDER_MASTER_ID "data-provider-master"

using namespace std;
namespace notification {

/* LCOV_EXCL_START */
DBusConnectionManager::DBusConnectionManager() = default;
DBusConnectionManager::~DBusConnectionManager() = default;

DBusConnectionManager& DBusConnectionManager::GetInst() {
  static DBusConnectionManager w_inst;
  int ret;
  if (w_inst.connection_ == nullptr) {
    ret = w_inst.Init();
    if (ret != ERROR_NONE)
      THROW(ret);
  }
  return w_inst;
}

bool DBusConnectionManager::IsDataProviderMaster() const {
  return is_DPM_;
}

bool DBusConnectionManager::IsDataProviderMaster(string appid) const {
  if (appid == DATA_PROVIDER_MASTER_ID)
    return true;
  return false;
}

string DBusConnectionManager::GetDataProviderMasterName() const {
  return GetBusName(DATA_PROVIDER_MASTER_ID);
}

string DBusConnectionManager::GetInterfaceName() const {
  return NOTIFICATION_EX_INTERFACE;
}

GDBusConnection* DBusConnectionManager::GetConnection() {
  return connection_;
}

std::string DBusConnectionManager::GetBusName(string appid) const {
  if (appid.empty())
    return "";

  if (IsDataProviderMaster(appid))
    return string(NOTIFICATION_EX_BUS_NAME_PREFIX) + DATA_PROVIDER_MASTER_ID;

  char* encoded_str = g_compute_checksum_for_string(G_CHECKSUM_MD5,
                appid.c_str(), -1);
  if (encoded_str == NULL)
    return "";

  string bus_name = string(NOTIFICATION_EX_BUS_NAME_PREFIX) + encoded_str;
  g_free(encoded_str);

  return bus_name;
}

int DBusConnectionManager::Init() {
  GError* error = nullptr;
  connection_ = g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, &error);
  if (connection_ == NULL) {
    if (error != NULL) {
      LOGE("Failed to get dbus [%s]", error->message);
      g_error_free(error);
    }
    return ERROR_IO_ERROR;
  }

  string appid = util::GetAppId();
  if (appid.empty())
    return ERROR_IO_ERROR;

  if (IsDataProviderMaster(appid))
    is_DPM_ = true;

  string encoded_name = GetBusName(appid);
  LOGI("own name %s", encoded_name.c_str());
  int owner_id = g_bus_own_name_on_connection(connection_, encoded_name.c_str(),
                                          G_BUS_NAME_OWNER_FLAGS_NONE,
                                          NULL, NULL, NULL, NULL);
  if (!owner_id) {
    g_object_unref(connection_);
    LOGE("g_bus_own_name_on_connection, error");
    return ERROR_IO_ERROR;
  }
  return ERROR_NONE;
}
/* LCOV_EXCL_STOP */

}  // namespace notification
