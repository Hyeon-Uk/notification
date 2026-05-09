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

#include "notification/include/notification.h"
#include "notification/include/notification_setting_internal.h"
#include "notification/include/notification_setting_service.h"

namespace {
class NotificationSettingServiceTest : public ::testing::Test {};
constexpr int kInvalid = NOTIFICATION_ERROR_INVALID_PARAMETER;
}  // namespace

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
