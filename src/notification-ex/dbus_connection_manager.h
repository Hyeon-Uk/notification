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

#include <atomic>
#include <mutex>
#include <string>

#ifndef EXPORT_API
#define EXPORT_API __attribute__((visibility("default")))
#endif

namespace notification {

/* Forward declaration so DBusConnectionManager can befriend the test-reset
 * helper. Defined in dbus_connection_manager.cc as file-static; not in any
 * public API surface. */
namespace dbus_test_hooks {
void ResetSingletonForTest();
}  // namespace dbus_test_hooks

/* LCOV_EXCL_START */
/*
 * DBusConnectionManager
 * ---------------------
 * Singleton that owns the process-wide system-bus connection used by the
 * notification-ex transport (dbus_sender / dbus_event_listener).
 *
 * N1-T1 lazy-connect contract:
 *   - GetInst()                   : cheap, never touches D-Bus.
 *   - GetInterfaceName()          : pure, never touches D-Bus.
 *   - GetBusName(appid)           : pure, never touches D-Bus.
 *   - GetDataProviderMasterName() : pure, never touches D-Bus.
 *   - IsDataProviderMaster(appid) : pure, never touches D-Bus.
 *   - GetConnection()             : connects on first call (std::call_once).
 *
 * ABI:
 *   - Public method signatures are unchanged from the pre-N1-T1 layout.
 *   - Test seams are file-static (see dbus_connection_manager.cc) and are
 *     NOT exposed via public symbols, so downstream consumers link against
 *     the same surface.
 */
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

  /*
   * EnsureConnected() performs the synchronous bus acquisition + own-name
   * on first invocation. Thread-safe via std::call_once; subsequent callers
   * observe connection_ through the acquire/release ordering provided by
   * call_once.
   */
  int EnsureConnected();

  GDBusConnection* connection_ = nullptr;
  bool is_DPM_ = false;
  std::once_flag connect_once_;
  std::atomic<int> last_connect_result_{0};

  /* Test-only reset: re-seats connect_once_ and clears the cached state.
   * Production code must never call this. ABI: friend declarations do not
   * contribute symbols. */
  friend void dbus_test_hooks::ResetSingletonForTest();
};
/* LCOV_EXCL_STOP */

}  // namespace notification

#endif  // NOTIFICATION_EX_DBUS_CONNECTION_MANAGER_H_
