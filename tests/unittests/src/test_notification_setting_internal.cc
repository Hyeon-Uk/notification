/*
 * Copyright (c) 2026 Samsung Electronics Co., Ltd.
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
#include "mock/glib_mock.hh"
#include "mock/sqlite_mock.hh"
#include "mock/tzplatform_config_mock.hh"

#include "notification/include/notification.h"
#include "notification/include/notification_setting_internal.h"

namespace {

class Mocks : virtual public ::testing::NiceMock<AulMock>,
              virtual public ::testing::NiceMock<TzplatformConfigMock>,
              virtual public ::testing::NiceMock<GlibMock>,
              virtual public ::testing::NiceMock<SqliteMock> {};

constexpr int kInvalid = NOTIFICATION_ERROR_INVALID_PARAMETER;
constexpr int kNone = NOTIFICATION_ERROR_NONE;

}  // namespace

class NotificationSettingInternalTest : public TestFixture {
 public:
  NotificationSettingInternalTest()
      : TestFixture(std::make_unique<::Mocks>()) {}
};

// notification_setting_get_setting requires real DB to populate
// the handle; in this fixture it returns garbage / NULL. _P tests
// for the per-handle getters/setters use GTEST_SKIP() when the
// handle cannot be obtained.

TEST_F(NotificationSettingInternalTest, get_appid_N_null_handle) {
  char *id = nullptr;
  EXPECT_EQ(notification_setting_get_appid(nullptr, &id), kInvalid);
}

TEST_F(NotificationSettingInternalTest, get_appid_P_skipped_no_handle) {
  notification_setting_h s = nullptr;
  if (notification_setting_get_setting(&s) != kNone || s == nullptr) {
    GTEST_SKIP() << "notification_setting_get_setting unavailable in fixture";
  }
  char *id = nullptr;
  EXPECT_EQ(notification_setting_get_appid(s, &id), kNone);
  notification_setting_free_notification(s);
}

TEST_F(NotificationSettingInternalTest, get_package_name_N_null_handle) {
  char *p = nullptr;
  EXPECT_EQ(notification_setting_get_package_name(nullptr, &p), kInvalid);
}

TEST_F(NotificationSettingInternalTest, get_allow_to_notify_N_null_handle) {
  bool a = false;
  EXPECT_EQ(notification_setting_get_allow_to_notify(nullptr, &a), kInvalid);
}

TEST_F(NotificationSettingInternalTest, set_allow_to_notify_N_null_handle) {
  EXPECT_EQ(notification_setting_set_allow_to_notify(nullptr, true), kInvalid);
}

TEST_F(NotificationSettingInternalTest,
       set_get_allow_to_notify_P_skipped_no_handle) {
  notification_setting_h s = nullptr;
  if (notification_setting_get_setting(&s) != kNone || s == nullptr) {
    GTEST_SKIP() << "no setting handle";
  }
  EXPECT_EQ(notification_setting_set_allow_to_notify(s, true), kNone);
  bool a = false;
  EXPECT_EQ(notification_setting_get_allow_to_notify(s, &a), kNone);
  EXPECT_TRUE(a);
  notification_setting_free_notification(s);
}

TEST_F(NotificationSettingInternalTest, get_do_not_disturb_except_N_null_handle) {
  bool dnd = false;
  EXPECT_EQ(notification_setting_get_do_not_disturb_except(nullptr, &dnd),
            kInvalid);
}

TEST_F(NotificationSettingInternalTest, set_do_not_disturb_except_N_null_handle) {
  EXPECT_EQ(notification_setting_set_do_not_disturb_except(nullptr, true),
            kInvalid);
}

TEST_F(NotificationSettingInternalTest, get_lock_screen_content_N_null_handle) {
  lock_screen_content_level_e l;
  EXPECT_EQ(notification_setting_get_lock_screen_content(nullptr, &l),
            kInvalid);
}

TEST_F(NotificationSettingInternalTest, get_pop_up_notification_N_null_handle) {
  bool p = false;
  EXPECT_EQ(notification_setting_get_pop_up_notification(nullptr, &p),
            kInvalid);
}

TEST_F(NotificationSettingInternalTest, system_setting_get_dnd_N_null_handle) {
  bool d = false;
  EXPECT_EQ(notification_system_setting_get_do_not_disturb(nullptr, &d),
            kInvalid);
}

TEST_F(NotificationSettingInternalTest, system_setting_set_dnd_N_null_handle) {
  EXPECT_EQ(notification_system_setting_set_do_not_disturb(nullptr, true),
            kInvalid);
}

TEST_F(NotificationSettingInternalTest, system_setting_free_N_null) {
  EXPECT_EQ(notification_system_setting_free_system_setting(nullptr), kInvalid);
}

TEST_F(NotificationSettingInternalTest,
       system_setting_set_get_dnd_P_skipped_no_handle) {
  notification_system_setting_h s = nullptr;
  if (notification_system_setting_load_system_setting(&s) != kNone ||
      s == nullptr) {
    GTEST_SKIP() << "no system_setting handle";
  }
  EXPECT_EQ(notification_system_setting_set_do_not_disturb(s, true), kNone);
  bool d = false;
  EXPECT_EQ(notification_system_setting_get_do_not_disturb(s, &d), kNone);
  EXPECT_TRUE(d);
  notification_system_setting_free_system_setting(s);
}
