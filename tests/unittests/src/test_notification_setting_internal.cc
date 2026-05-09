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

namespace {
class NotificationSettingInternalTest : public ::testing::Test {};
constexpr int kInvalid = NOTIFICATION_ERROR_INVALID_PARAMETER;
}

TEST_F(NotificationSettingInternalTest, get_appid_null_handle) {
  char *id = nullptr;
  EXPECT_EQ(notification_setting_get_appid(nullptr, &id), kInvalid);
}

TEST_F(NotificationSettingInternalTest, get_package_name_null_handle) {
  char *p = nullptr;
  EXPECT_EQ(notification_setting_get_package_name(nullptr, &p), kInvalid);
}

TEST_F(NotificationSettingInternalTest, get_allow_to_notify_null_handle) {
  bool a = false;
  EXPECT_EQ(notification_setting_get_allow_to_notify(nullptr, &a), kInvalid);
}

TEST_F(NotificationSettingInternalTest, get_do_not_disturb_except_null_handle) {
  bool dnd = false;
  EXPECT_EQ(notification_setting_get_do_not_disturb_except(nullptr, &dnd), kInvalid);
}

TEST_F(NotificationSettingInternalTest, get_lock_screen_content_null_handle) {
  lock_screen_content_level_e l;
  EXPECT_EQ(notification_setting_get_lock_screen_content(nullptr, &l), kInvalid);
}

TEST_F(NotificationSettingInternalTest, get_pop_up_notification_null_handle) {
  bool p = false;
  EXPECT_EQ(notification_setting_get_pop_up_notification(nullptr, &p), kInvalid);
}

TEST_F(NotificationSettingInternalTest, set_allow_to_notify_null_handle) {
  EXPECT_EQ(notification_setting_set_allow_to_notify(nullptr, true), kInvalid);
}

TEST_F(NotificationSettingInternalTest, set_do_not_disturb_except_null_handle) {
  EXPECT_EQ(notification_setting_set_do_not_disturb_except(nullptr, true), kInvalid);
}

TEST_F(NotificationSettingInternalTest, system_setting_get_dnd_null_handle) {
  bool d = false;
  EXPECT_EQ(notification_system_setting_get_do_not_disturb(nullptr, &d), kInvalid);
}

TEST_F(NotificationSettingInternalTest, system_setting_set_dnd_null_handle) {
  EXPECT_EQ(notification_system_setting_set_do_not_disturb(nullptr, true), kInvalid);
}

TEST_F(NotificationSettingInternalTest, system_setting_free_null) {
  EXPECT_EQ(notification_system_setting_free_system_setting(nullptr), kInvalid);
}
