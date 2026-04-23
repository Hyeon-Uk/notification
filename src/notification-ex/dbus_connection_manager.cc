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

#include <atomic>

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

/*
 * Test-only override hooks.
 *
 * Unit tests can install a replacement connection (typically a sentinel
 * non-null pointer that the mocked D-Bus layer recognises) so that the
 * production lazy-init path can be exercised without talking to a real
 * system bus. The overrides are file-static: they add zero public symbols
 * and preserve ABI. Production code never touches these (they stay null
 * unless a test explicitly sets them).
 *
 * Not thread-safe; tests must install/clear before spawning threads.
 */
namespace {
std::atomic<GDBusConnection*> s_override_connection_{nullptr};
std::atomic<int> s_override_connect_count_{0};
std::atomic<bool> s_force_connect_failure_{false};
}  // namespace

namespace dbus_test_hooks {
/* N2-T5: dbus_test_hooks is consumed from the unittest binary. With the
 * library-wide -fvisibility=hidden on C++ code, each of these entry points
 * must carry EXPORT_API explicitly; otherwise the test TU cannot link. */
/* Install a sentinel connection pointer returned from GetConnection() without
 * calling g_bus_get_sync(). Pass nullptr to clear. */
EXPORT_API void SetOverrideConnection(GDBusConnection* conn) {
  s_override_connection_.store(conn, std::memory_order_release);
}
/* Counter incremented each time the override path is taken (i.e. the lazy
 * EnsureConnected succeeds without touching real g_bus_get_sync). */
EXPORT_API int  GetOverrideConnectCount() {
  return s_override_connect_count_.load(std::memory_order_acquire);
}
EXPORT_API void ResetOverrideConnectCount() {
  s_override_connect_count_.store(0, std::memory_order_release);
}
/* Force EnsureConnected() to return an error even when an override is set.
 * Lets tests exercise the failure path deterministically. */
EXPORT_API void SetForceConnectFailure(bool v) {
  s_force_connect_failure_.store(v, std::memory_order_release);
}
/*
 * Reset the singleton's lazy state so the next GetConnection() acts as the
 * first caller again. std::once_flag has no public reset, so we destroy
 * and reconstruct it in place (placement-new). Caller must ensure no
 * other thread touches the singleton during this call.
 */
EXPORT_API void ResetSingletonForTest() {
  auto& w = notification::DBusConnectionManager::GetInst();
  w.connection_ = nullptr;
  w.is_DPM_ = false;
  w.last_connect_result_.store(0, std::memory_order_release);
  w.connect_once_.~once_flag();
  new (&w.connect_once_) std::once_flag();
}
}  // namespace dbus_test_hooks

DBusConnectionManager::DBusConnectionManager() = default;
DBusConnectionManager::~DBusConnectionManager() = default;

DBusConnectionManager& DBusConnectionManager::GetInst() {
  /*
   * N1-T1: GetInst() is now a pure accessor. It constructs the singleton
   * on first call but does NOT talk to D-Bus. The connection is acquired
   * lazily inside GetConnection() via EnsureConnected().
   */
  static DBusConnectionManager w_inst;
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
  /*
   * Lazy path: trigger EnsureConnected once. After call_once returns, either
   * connection_ is non-null (success) or last_connect_result_ is non-zero
   * (failure: throw to preserve legacy caller contract).
   */
  int ret = EnsureConnected();
  if (ret != ERROR_NONE)
    THROW(ret);
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

int DBusConnectionManager::EnsureConnected() {
  std::call_once(connect_once_, [this]() {
    /* Test override path: use sentinel connection, skip real bus I/O. */
    GDBusConnection* override_conn =
        s_override_connection_.load(std::memory_order_acquire);
    if (override_conn != nullptr) {
      if (s_force_connect_failure_.load(std::memory_order_acquire)) {
        last_connect_result_.store(ERROR_IO_ERROR,
            std::memory_order_release);
        return;
      }
      connection_ = override_conn;
      string appid = util::GetAppId();
      if (IsDataProviderMaster(appid))
        is_DPM_ = true;
      s_override_connect_count_.fetch_add(1, std::memory_order_acq_rel);
      last_connect_result_.store(ERROR_NONE, std::memory_order_release);
      return;
    }

    GError* error = nullptr;
    connection_ = g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, &error);
    if (connection_ == NULL) {
      if (error != NULL) {
        LOGE("Failed to get dbus [%s]", error->message);
        g_error_free(error);
      }
      last_connect_result_.store(ERROR_IO_ERROR, std::memory_order_release);
      return;
    }

    string appid = util::GetAppId();
    if (appid.empty()) {
      last_connect_result_.store(ERROR_IO_ERROR, std::memory_order_release);
      return;
    }

    if (IsDataProviderMaster(appid))
      is_DPM_ = true;

    string encoded_name = GetBusName(appid);
    LOGI("own name %s", encoded_name.c_str());
    int owner_id = g_bus_own_name_on_connection(connection_,
                                            encoded_name.c_str(),
                                            G_BUS_NAME_OWNER_FLAGS_NONE,
                                            NULL, NULL, NULL, NULL);
    if (!owner_id) {
      g_object_unref(connection_);
      connection_ = nullptr;
      LOGE("g_bus_own_name_on_connection, error");
      last_connect_result_.store(ERROR_IO_ERROR, std::memory_order_release);
      return;
    }
    last_connect_result_.store(ERROR_NONE, std::memory_order_release);
  });
  return last_connect_result_.load(std::memory_order_acquire);
}
/* LCOV_EXCL_STOP */

}  // namespace notification
