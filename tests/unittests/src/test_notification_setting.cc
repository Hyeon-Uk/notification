/*
 * Copyright (c) 2023 Samsung Electronics Co., Ltd.
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

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <cstdlib>
#include <cstring>

#include "mock/test_fixture.hh"
#include "mock/aul_mock.hh"
#include "mock/glib_mock.hh"
#include "mock/sqlite_mock.hh"
#include "mock/noti_service_proxy_mock.hh"

#include "notification/include/notification.h"
#include "notification/include/notification_setting.h"
#include "notification/include/notification_setting_internal.h"

namespace {

rpc_port_proxy_noti_service_h kFakeSettingServiceHandle =
    reinterpret_cast<rpc_port_proxy_noti_service_h>(0x1);

int __fake_rpc_port_proxy_noti_service_create(const char*,
    rpc_port_proxy_noti_service_callback_s*, void*,
    rpc_port_proxy_noti_service_h* handle) {
  *handle = kFakeSettingServiceHandle;
  return RPC_PORT_ERROR_NONE;
}

class Mocks :
    virtual public ::testing::NiceMock<AulMock>,
    virtual public ::testing::NiceMock<GlibMock>,
    virtual public ::testing::NiceMock<SqliteMock>,
    virtual public ::testing::NiceMock<NotiServiceProxyMock> {};
} // namespace

using ::testing::_;
using ::testing::Invoke;
using ::testing::StrEq;
using ::testing::Return;

class NotificationSettingTest : public TestFixture {
 public:
  NotificationSettingTest() : TestFixture(std::make_unique<::Mocks>()) {}
  virtual void SetUp() {
    ON_CALL(GetMock<NotiServiceProxyMock>(),
        rpc_port_proxy_noti_service_create(_, _, _, _))
        .WillByDefault(Invoke(__fake_rpc_port_proxy_noti_service_create));
    ON_CALL(GetMock<NotiServiceProxyMock>(),
        rpc_port_proxy_noti_service_connect_sync(_))
        .WillByDefault(Return(RPC_PORT_ERROR_NONE));
  }
  virtual void TearDown() {
  }
};

namespace {

int __fake_sqlite3_open_v2(const char* arg0, sqlite3** arg1, int arg2, const char* arg3) {
  (void)arg0;
  (void)arg2;
  (void)arg3;
  char** table = (char**)calloc(10, sizeof(char*));
  *arg1 = (sqlite3*)table;

  return SQLITE_OK;
}

int __fake_sqlite3_prepare_v2(sqlite3* db, const char* query, int, sqlite3_stmt** stmt, const char**) {
  (void)db;
  (void)query;
  *stmt = (sqlite3_stmt*)calloc(1, 8);
  return SQLITE_OK;
}

int __fake_sqlite3_step_done(sqlite3_stmt* stmt) {
  (void)stmt;
  return SQLITE_DONE;
}

int __fake_sqlite3_step_row(sqlite3_stmt* stmt) {
  (void)stmt;
  return SQLITE_ROW;
}

int __fake_sqlite3_finalize(sqlite3_stmt* stmt) {
  free(stmt);
  return SQLITE_OK;
}

int __fake_sqlite3_close(sqlite3* db) {
  free(db);
  return SQLITE_OK;
}

}  // namespace

TEST_F(NotificationSettingTest, notification_setting_test) {
  int ret;

  notification_setting_h setting;
  setting = (struct notification_setting *)malloc(sizeof(struct notification_setting));

  setting->package_name = strdup("org.tizen.testpkg");
  setting->app_id = strdup("org.tizen.testapp");
  setting->allow_to_notify = true;
  setting->do_not_disturb_except = false;
  setting->pop_up_notification = false;
  setting->visibility_class = 1;
  setting->lock_screen_content_level = SHOW_ALL_CONTENT;
  setting->app_disabled = false;

  int visibility;
  ret = notification_setting_get_visibility_class(setting, &visibility);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  ret = notification_setting_set_visibility_class(setting, 3);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  ret = notification_setting_update_setting(nullptr);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);

  ret = notification_setting_get_visibility_class(nullptr, &visibility);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);

  ret = notification_setting_set_visibility_class(nullptr, 3);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);

  EXPECT_CALL(GetMock<NotiServiceProxyMock>(),
      rpc_port_proxy_noti_service_invoke_update_noti_setting(_, _, _))
      .WillOnce(Return(RPC_PORT_ERROR_NONE));
  ret = notification_setting_update_setting(setting);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  ret = notification_setting_free_notification(setting);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  ret = notification_setting_free_notification(nullptr);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}
