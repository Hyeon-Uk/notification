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

#include "mock/test_fixture.hh"
#include "mock/aul_mock.hh"
#include "mock/config_mock.hh"
#include "mock/glib_mock.hh"
#include "mock/noti_service_proxy_mock.hh"
#include "mock/rpc_port_mock.hh"
#include "mock/sqlite_mock.hh"

#include "notification/include/notification.h"
#include "notification/include/notification_setting.h"
#include "notification/include/notification_setting_internal.h"

namespace {

class Mocks :
    virtual public ::testing::NiceMock<AulMock>,
    virtual public ::testing::NiceMock<ConfigMock>,
    virtual public ::testing::NiceMock<GlibMock>,
    virtual public ::testing::NiceMock<SqliteMock> {};

/* Extended Mocks that include IPC layer for TIDL-reachable paths. */
class MocksWithTidl :
    virtual public ::testing::NiceMock<AulMock>,
    virtual public ::testing::NiceMock<ConfigMock>,
    virtual public ::testing::NiceMock<GlibMock>,
    virtual public ::testing::NiceMock<NotiServiceProxyMock>,
    virtual public ::testing::NiceMock<RpcPortMock>,
    virtual public ::testing::NiceMock<SqliteMock> {};

int __setting_fake_aul_app_get_appid_bypid(int pid, char* appid, int len) {
  snprintf(appid, len, "%s", "org.tizen.settingtest");
  return AUL_R_OK;
}

} // namespace

using ::testing::_;
using ::testing::Invoke;
using ::testing::Return;

class NotificationSettingTest : public TestFixture {
 public:
  NotificationSettingTest() : TestFixture(std::make_unique<::Mocks>()) {}
  virtual void SetUp() {
  }
  virtual void TearDown() {
  }
};

/* ============================================================== */
/* notification_setting_get_setting — AUL failure (N1)            */
/* ============================================================== */

TEST_F(NotificationSettingTest, setting_get_setting_aul_fail_returns_not_exist) {
  /* [N1] AUL pid lookup fails → NOT_EXIST_ID before IPC */
  ON_CALL(GetMock<AulMock>(), aul_app_get_appid_bypid(_, _, _))
      .WillByDefault(Return(AUL_R_ERROR));
  notification_setting_h setting = nullptr;
  EXPECT_EQ(notification_setting_get_setting(&setting),
            NOTIFICATION_ERROR_NOT_EXIST_ID);
}

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

  ret = notification_setting_free_notification(setting);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);
}

/* ============================================================== */
/* notification_setting_refresh_setting_table — DB fail (N1)      */
/* ============================================================== */

TEST_F(NotificationSettingTest,
       refresh_setting_table_db_fail_returns_error) {
  /* TV profile: TIZEN_FEATURE_ENABLE_NOTI_SETTING is false, so DB path is reached */
  ON_CALL(GetMock<ConfigMock>(), _config_get_tizen_profile())
      .WillByDefault(Return(TIZEN_PROFILE_TV));
  /* [N1] sqlite3_open_v2 fails → db_open returns NULL → non-NONE error */
  ON_CALL(GetMock<SqliteMock>(), sqlite3_open_v2(_, _, _, _))
      .WillByDefault(Return(SQLITE_ERROR));
  int ret = notification_setting_refresh_setting_table(5001);
  EXPECT_NE(ret, NOTIFICATION_ERROR_NONE);
}

/* ============================================================== */
/* notification_system_setting_init_system_setting_table — DB (N1) */
/* ============================================================== */

TEST_F(NotificationSettingTest,
       system_setting_init_table_db_fail_returns_error) {
  /* [N1] sqlite3_open_v2 fails → db_open returns NULL → non-NONE error */
  ON_CALL(GetMock<SqliteMock>(), sqlite3_open_v2(_, _, _, _))
      .WillByDefault(Return(SQLITE_ERROR));
  int ret = notification_system_setting_init_system_setting_table(5001);
  EXPECT_NE(ret, NOTIFICATION_ERROR_NONE);
}

/* ============================================================== */
/* NotificationSettingTidlTest — fixture with TIDL mocks          */
/* ============================================================== */

class NotificationSettingTidlTest : public TestFixture {
 public:
  NotificationSettingTidlTest()
      : TestFixture(std::make_unique<::MocksWithTidl>()) {}
};

/* ============================================================== */
/* notification_setting_get_setting — AUL success → TIDL path     */
/* (L92-96 in notification_setting.c)                              */
/* ============================================================== */

TEST_F(NotificationSettingTidlTest,
       setting_get_setting_aul_success_enters_tidl_path) {
  /* [P1-partial] AUL lookup succeeds → function calls
   * notification_setting_get_setting_by_package_name which calls TIDL.
   * With TIDL connect mocked to fail we get a non-NONE error that is NOT
   * NOT_EXIST_ID (which would mean the AUL path failed). The key invariant
   * is that the AUL+free branch (L92-96) executes. */
  ON_CALL(GetMock<AulMock>(), aul_app_get_appid_bypid(_, _, _))
      .WillByDefault(Invoke(__setting_fake_aul_app_get_appid_bypid));
  ON_CALL(GetMock<NotiServiceProxyMock>(),
          rpc_port_proxy_noti_service_connect_sync(_))
      .WillByDefault(Return(RPC_PORT_ERROR_IO_ERROR));

  notification_setting_h setting = nullptr;
  int ret = notification_setting_get_setting(&setting);
  /* AUL succeeded so we should NOT get NOT_EXIST_ID. */
  EXPECT_NE(ret, NOTIFICATION_ERROR_NOT_EXIST_ID);
  if (setting) notification_setting_free_notification(setting);
}

