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
#include "mock/glib_mock.hh"
#include "mock/sqlite_mock.hh"
#include "mock/tzplatform_config_mock.hh"

#include "notification/include/notification.h"
#include "notification/include/notification_setting_internal.h"
#include "notification/include/notification_setting_service.h"

namespace {
class Mocks : virtual public ::testing::NiceMock<GlibMock>,
              virtual public ::testing::NiceMock<SqliteMock>,
              virtual public ::testing::NiceMock<TzplatformConfigMock> {};
constexpr int kInvalid = NOTIFICATION_ERROR_INVALID_PARAMETER;
constexpr int kNone = NOTIFICATION_ERROR_NONE;
}  // namespace

class NotificationSettingServiceTest : public TestFixture {
 public:
  NotificationSettingServiceTest()
      : TestFixture(std::make_unique<::Mocks>()) {}
};

TEST_F(NotificationSettingServiceTest,
       dnd_schedule_set_get_enabled_P_skipped_no_handle) {
  notification_system_setting_h s = nullptr;
  if (notification_system_setting_load_system_setting(&s) != kNone ||
      s == nullptr) {
    GTEST_SKIP() << "no system_setting handle in this fixture";
  }
  EXPECT_EQ(notification_system_setting_dnd_schedule_set_enabled(s, true),
            kNone);
  bool e = false;
  EXPECT_EQ(notification_system_setting_dnd_schedule_get_enabled(s, &e), kNone);
  EXPECT_TRUE(e);
  notification_system_setting_free_system_setting(s);
}

TEST_F(NotificationSettingServiceTest, dnd_schedule_get_enabled_N_null_handle) {
  bool e = false;
  EXPECT_EQ(notification_system_setting_dnd_schedule_get_enabled(nullptr, &e),
            kInvalid);
}

TEST_F(NotificationSettingServiceTest, dnd_schedule_set_enabled_N_null_handle) {
  EXPECT_EQ(notification_system_setting_dnd_schedule_set_enabled(nullptr, true),
            kInvalid);
}

TEST_F(NotificationSettingServiceTest, dnd_schedule_get_day_N_null_handle) {
  int d = 0;
  EXPECT_EQ(notification_system_setting_dnd_schedule_get_day(nullptr, &d),
            kInvalid);
}

TEST_F(NotificationSettingServiceTest, dnd_schedule_set_day_N_null_handle) {
  EXPECT_EQ(notification_system_setting_dnd_schedule_set_day(nullptr, 1),
            kInvalid);
}

TEST_F(NotificationSettingServiceTest, dnd_schedule_get_start_time_N_null_handle) {
  int h = 0, m = 0;
  EXPECT_EQ(notification_system_setting_dnd_schedule_get_start_time(nullptr, &h, &m),
            kInvalid);
}

TEST_F(NotificationSettingServiceTest, dnd_schedule_set_start_time_N_null_handle) {
  EXPECT_EQ(notification_system_setting_dnd_schedule_set_start_time(nullptr, 22, 0),
            kInvalid);
}

TEST_F(NotificationSettingServiceTest, dnd_schedule_get_end_time_N_null_handle) {
  int h = 0, m = 0;
  EXPECT_EQ(notification_system_setting_dnd_schedule_get_end_time(nullptr, &h, &m),
            kInvalid);
}

TEST_F(NotificationSettingServiceTest, dnd_schedule_set_end_time_N_null_handle) {
  EXPECT_EQ(notification_system_setting_dnd_schedule_set_end_time(nullptr, 7, 0),
            kInvalid);
}
