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

#ifndef NOTIFICATION_EX_DBUS_CONNECTION_MANAGER_H_
#define NOTIFICATION_EX_DBUS_CONNECTION_MANAGER_H_

#include <gio/gio.h>

#include <string>

#ifndef EXPORT_API
#define EXPORT_API __attribute__((visibility("default")))
#endif

namespace notification {

/* LCOV_EXCL_START */
class EXPORT_API DBusConnectionManager {
 public:
  static DBusConnectionManager& GetInst();
  GDBusConnection* GetConnection();
  bool IsDataProviderMaster(std::string appid) const;
  std::string GetDataProviderMasterName() const;
  std::string GetInterfaceName() const;
  std::string GetBusName(std::string appid) const;

 private:
  bool IsDataProviderMaster() const;
  DBusConnectionManager();
  ~DBusConnectionManager();
  int Init();
  GDBusConnection* connection_ = nullptr;
  bool is_DPM_ = false;
};
/* LCOV_EXCL_STOP */

}  // namespace notification

#endif  // NOTIFICATION_EX_DBUS_CONNECTION_MANAGER_H_
