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
#include "mock/glib_mock.hh"
#include "mock/sqlite_mock.hh"

#include "notification/include/notification.h"
#include "notification/include/notification_setting.h"
#include "notification/include/notification_setting_internal.h"

namespace {

class Mocks :
    virtual public ::testing::NiceMock<AulMock>,
    virtual public ::testing::NiceMock<GlibMock> {};
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

/*
 * NULL handle tests for notification_setting get/set functions
 */
TEST_F(NotificationSettingTest, get_package_name_null_handle) {
  char *value = nullptr;
  int ret = notification_setting_get_package_name(nullptr, &value);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationSettingTest, get_appid_null_handle) {
  char *value = nullptr;
  int ret = notification_setting_get_appid(nullptr, &value);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationSettingTest, get_allow_to_notify_null_handle) {
  bool value = false;
  int ret = notification_setting_get_allow_to_notify(nullptr, &value);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationSettingTest, set_allow_to_notify_null_handle) {
  int ret = notification_setting_set_allow_to_notify(nullptr, true);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationSettingTest, get_do_not_disturb_except_null_handle) {
  bool value = false;
  int ret = notification_setting_get_do_not_disturb_except(nullptr, &value);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationSettingTest, set_do_not_disturb_except_null_handle) {
  int ret = notification_setting_set_do_not_disturb_except(nullptr, true);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationSettingTest, get_visibility_class_null_handle) {
  int value = 0;
  int ret = notification_setting_get_visibility_class(nullptr, &value);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationSettingTest, set_visibility_class_null_handle) {
  int ret = notification_setting_set_visibility_class(nullptr, 1);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationSettingTest, get_pop_up_notification_null_handle) {
  bool value = false;
  int ret = notification_setting_get_pop_up_notification(nullptr, &value);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationSettingTest, set_pop_up_notification_null_handle) {
  int ret = notification_setting_set_pop_up_notification(nullptr, true);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationSettingTest, get_lock_screen_content_null_handle) {
  lock_screen_content_level_e level;
  int ret = notification_setting_get_lock_screen_content(nullptr, &level);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationSettingTest, set_lock_screen_content_null_handle) {
  int ret = notification_setting_set_lock_screen_content(nullptr, SHOW_ALL_CONTENT);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationSettingTest, get_app_disabled_null_handle) {
  bool value = false;
  int ret = notification_setting_get_app_disabled(nullptr, &value);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationSettingTest, update_setting_null_handle) {
  int ret = notification_setting_update_setting(nullptr);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationSettingTest, free_notification_null_handle) {
  int ret = notification_setting_free_notification(nullptr);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationSettingTest, get_setting_array_null_array) {
  int count = 0;
  int ret = notification_setting_get_setting_array(nullptr, &count);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationSettingTest, get_setting_array_null_count) {
  notification_setting_h setting_array = nullptr;
  int ret = notification_setting_get_setting_array(&setting_array, nullptr);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationSettingTest, get_setting_array_both_null) {
  int ret = notification_setting_get_setting_array(nullptr, nullptr);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/*
 * NULL output pointer tests (valid handle, NULL output parameter)
 */
TEST_F(NotificationSettingTest, get_package_name_null_value) {
  notification_setting_h setting;
  setting = (struct notification_setting *)calloc(1, sizeof(struct notification_setting));
  setting->package_name = strdup("org.tizen.testpkg");
  setting->app_id = strdup("org.tizen.testapp");

  int ret = notification_setting_get_package_name(setting, nullptr);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);

  notification_setting_free_notification(setting);
}

TEST_F(NotificationSettingTest, get_appid_null_value) {
  notification_setting_h setting;
  setting = (struct notification_setting *)calloc(1, sizeof(struct notification_setting));
  setting->package_name = strdup("org.tizen.testpkg");
  setting->app_id = strdup("org.tizen.testapp");

  int ret = notification_setting_get_appid(setting, nullptr);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);

  notification_setting_free_notification(setting);
}

TEST_F(NotificationSettingTest, get_allow_to_notify_null_value) {
  notification_setting_h setting;
  setting = (struct notification_setting *)calloc(1, sizeof(struct notification_setting));
  setting->package_name = strdup("org.tizen.testpkg");
  setting->app_id = strdup("org.tizen.testapp");

  int ret = notification_setting_get_allow_to_notify(setting, nullptr);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);

  notification_setting_free_notification(setting);
}

TEST_F(NotificationSettingTest, get_do_not_disturb_except_null_value) {
  notification_setting_h setting;
  setting = (struct notification_setting *)calloc(1, sizeof(struct notification_setting));
  setting->package_name = strdup("org.tizen.testpkg");
  setting->app_id = strdup("org.tizen.testapp");

  int ret = notification_setting_get_do_not_disturb_except(setting, nullptr);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);

  notification_setting_free_notification(setting);
}

TEST_F(NotificationSettingTest, get_visibility_class_null_value) {
  notification_setting_h setting;
  setting = (struct notification_setting *)calloc(1, sizeof(struct notification_setting));
  setting->package_name = strdup("org.tizen.testpkg");
  setting->app_id = strdup("org.tizen.testapp");

  int ret = notification_setting_get_visibility_class(setting, nullptr);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);

  notification_setting_free_notification(setting);
}

TEST_F(NotificationSettingTest, get_pop_up_notification_null_value) {
  notification_setting_h setting;
  setting = (struct notification_setting *)calloc(1, sizeof(struct notification_setting));
  setting->package_name = strdup("org.tizen.testpkg");
  setting->app_id = strdup("org.tizen.testapp");

  int ret = notification_setting_get_pop_up_notification(setting, nullptr);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);

  notification_setting_free_notification(setting);
}

TEST_F(NotificationSettingTest, get_lock_screen_content_null_value) {
  notification_setting_h setting;
  setting = (struct notification_setting *)calloc(1, sizeof(struct notification_setting));
  setting->package_name = strdup("org.tizen.testpkg");
  setting->app_id = strdup("org.tizen.testapp");

  int ret = notification_setting_get_lock_screen_content(setting, nullptr);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);

  notification_setting_free_notification(setting);
}

TEST_F(NotificationSettingTest, get_app_disabled_null_value) {
  notification_setting_h setting;
  setting = (struct notification_setting *)calloc(1, sizeof(struct notification_setting));
  setting->package_name = strdup("org.tizen.testpkg");
  setting->app_id = strdup("org.tizen.testapp");

  int ret = notification_setting_get_app_disabled(setting, nullptr);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);

  notification_setting_free_notification(setting);
}

/*
 * Valid handle get/set round-trip tests
 */
TEST_F(NotificationSettingTest, get_package_name_valid) {
  notification_setting_h setting;
  setting = (struct notification_setting *)calloc(1, sizeof(struct notification_setting));
  setting->package_name = strdup("org.tizen.testpkg");
  setting->app_id = strdup("org.tizen.testapp");

  char *value = nullptr;
  int ret = notification_setting_get_package_name(setting, &value);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);
  ASSERT_STREQ(value, "org.tizen.testpkg");
  free(value);

  notification_setting_free_notification(setting);
}

TEST_F(NotificationSettingTest, get_appid_valid) {
  notification_setting_h setting;
  setting = (struct notification_setting *)calloc(1, sizeof(struct notification_setting));
  setting->package_name = strdup("org.tizen.testpkg");
  setting->app_id = strdup("org.tizen.testapp");

  char *value = nullptr;
  int ret = notification_setting_get_appid(setting, &value);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);
  ASSERT_STREQ(value, "org.tizen.testapp");
  free(value);

  notification_setting_free_notification(setting);
}

TEST_F(NotificationSettingTest, get_set_allow_to_notify_valid) {
  notification_setting_h setting;
  setting = (struct notification_setting *)calloc(1, sizeof(struct notification_setting));
  setting->package_name = strdup("org.tizen.testpkg");
  setting->app_id = strdup("org.tizen.testapp");
  setting->allow_to_notify = false;

  int ret = notification_setting_set_allow_to_notify(setting, true);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  bool value = false;
  ret = notification_setting_get_allow_to_notify(setting, &value);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);
  ASSERT_EQ(value, true);

  notification_setting_free_notification(setting);
}

TEST_F(NotificationSettingTest, get_set_do_not_disturb_except_valid) {
  notification_setting_h setting;
  setting = (struct notification_setting *)calloc(1, sizeof(struct notification_setting));
  setting->package_name = strdup("org.tizen.testpkg");
  setting->app_id = strdup("org.tizen.testapp");
  setting->do_not_disturb_except = false;

  int ret = notification_setting_set_do_not_disturb_except(setting, true);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  bool value = false;
  ret = notification_setting_get_do_not_disturb_except(setting, &value);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);
  ASSERT_EQ(value, true);

  notification_setting_free_notification(setting);
}

TEST_F(NotificationSettingTest, get_set_visibility_class_valid) {
  notification_setting_h setting;
  setting = (struct notification_setting *)calloc(1, sizeof(struct notification_setting));
  setting->package_name = strdup("org.tizen.testpkg");
  setting->app_id = strdup("org.tizen.testapp");
  setting->visibility_class = 0;

  int ret = notification_setting_set_visibility_class(setting, 5);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  int value = 0;
  ret = notification_setting_get_visibility_class(setting, &value);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);
  ASSERT_EQ(value, 5);

  notification_setting_free_notification(setting);
}

TEST_F(NotificationSettingTest, get_set_pop_up_notification_valid) {
  notification_setting_h setting;
  setting = (struct notification_setting *)calloc(1, sizeof(struct notification_setting));
  setting->package_name = strdup("org.tizen.testpkg");
  setting->app_id = strdup("org.tizen.testapp");
  setting->pop_up_notification = false;

  int ret = notification_setting_set_pop_up_notification(setting, true);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  bool value = false;
  ret = notification_setting_get_pop_up_notification(setting, &value);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);
  ASSERT_EQ(value, true);

  notification_setting_free_notification(setting);
}

TEST_F(NotificationSettingTest, get_set_lock_screen_content_valid) {
  notification_setting_h setting;
  setting = (struct notification_setting *)calloc(1, sizeof(struct notification_setting));
  setting->package_name = strdup("org.tizen.testpkg");
  setting->app_id = strdup("org.tizen.testapp");
  setting->lock_screen_content_level = SHOW_ALL_CONTENT;

  int ret = notification_setting_set_lock_screen_content(setting, HIDE_SENSITIVE_CONTENT);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  lock_screen_content_level_e level;
  ret = notification_setting_get_lock_screen_content(setting, &level);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);
  ASSERT_EQ(level, HIDE_SENSITIVE_CONTENT);

  notification_setting_free_notification(setting);
}

TEST_F(NotificationSettingTest, get_app_disabled_valid) {
  notification_setting_h setting;
  setting = (struct notification_setting *)calloc(1, sizeof(struct notification_setting));
  setting->package_name = strdup("org.tizen.testpkg");
  setting->app_id = strdup("org.tizen.testapp");
  setting->app_disabled = true;

  bool value = false;
  int ret = notification_setting_get_app_disabled(setting, &value);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);
  ASSERT_EQ(value, true);

  notification_setting_free_notification(setting);
}

TEST_F(NotificationSettingTest, get_package_name_null_field) {
  notification_setting_h setting;
  setting = (struct notification_setting *)calloc(1, sizeof(struct notification_setting));
  setting->package_name = nullptr;
  setting->app_id = strdup("org.tizen.testapp");

  char *value = nullptr;
  int ret = notification_setting_get_package_name(setting, &value);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NOT_EXIST_ID);

  notification_setting_free_notification(setting);
}

TEST_F(NotificationSettingTest, get_appid_null_field) {
  notification_setting_h setting;
  setting = (struct notification_setting *)calloc(1, sizeof(struct notification_setting));
  setting->package_name = strdup("org.tizen.testpkg");
  setting->app_id = nullptr;

  char *value = nullptr;
  int ret = notification_setting_get_appid(setting, &value);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NOT_EXIST_ID);

  notification_setting_free_notification(setting);
}

/*
 * NULL handle tests for notification_system_setting functions
 */
TEST_F(NotificationSettingTest, system_setting_free_null_handle) {
  int ret = notification_system_setting_free_system_setting(nullptr);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationSettingTest, system_setting_get_do_not_disturb_null_handle) {
  bool value = false;
  int ret = notification_system_setting_get_do_not_disturb(nullptr, &value);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationSettingTest, system_setting_set_do_not_disturb_null_handle) {
  int ret = notification_system_setting_set_do_not_disturb(nullptr, true);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationSettingTest, system_setting_get_visibility_class_null_handle) {
  int value = 0;
  int ret = notification_system_setting_get_visibility_class(nullptr, &value);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationSettingTest, system_setting_set_visibility_class_null_handle) {
  int ret = notification_system_setting_set_visibility_class(nullptr, 1);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationSettingTest, system_setting_dnd_schedule_get_enabled_null_handle) {
  bool enabled = false;
  int ret = notification_system_setting_dnd_schedule_get_enabled(nullptr, &enabled);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationSettingTest, system_setting_dnd_schedule_set_enabled_null_handle) {
  int ret = notification_system_setting_dnd_schedule_set_enabled(nullptr, true);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationSettingTest, system_setting_dnd_schedule_get_day_null_handle) {
  int day = 0;
  int ret = notification_system_setting_dnd_schedule_get_day(nullptr, &day);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationSettingTest, system_setting_dnd_schedule_set_day_null_handle) {
  int ret = notification_system_setting_dnd_schedule_set_day(nullptr, DND_SCHEDULE_WEEK_FLAG_MONDAY);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationSettingTest, system_setting_dnd_schedule_get_start_time_null_handle) {
  int hour = 0, min = 0;
  int ret = notification_system_setting_dnd_schedule_get_start_time(nullptr, &hour, &min);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationSettingTest, system_setting_dnd_schedule_set_start_time_null_handle) {
  int ret = notification_system_setting_dnd_schedule_set_start_time(nullptr, 22, 0);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationSettingTest, system_setting_dnd_schedule_get_end_time_null_handle) {
  int hour = 0, min = 0;
  int ret = notification_system_setting_dnd_schedule_get_end_time(nullptr, &hour, &min);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationSettingTest, system_setting_dnd_schedule_set_end_time_null_handle) {
  int ret = notification_system_setting_dnd_schedule_set_end_time(nullptr, 7, 0);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationSettingTest, system_setting_get_lock_screen_content_null_handle) {
  lock_screen_content_level_e level;
  int ret = notification_system_setting_get_lock_screen_content(nullptr, &level);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationSettingTest, system_setting_set_lock_screen_content_null_handle) {
  int ret = notification_system_setting_set_lock_screen_content(nullptr, SHOW_ALL_CONTENT);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationSettingTest, system_setting_get_dnd_allow_exceptions_null_handle) {
  int value = 0;
  int ret = notification_system_setting_get_dnd_allow_exceptions(nullptr,
      NOTIFICATION_DND_ALLOWED_CALLS, &value);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationSettingTest, system_setting_set_dnd_allow_exceptions_null_handle) {
  int ret = notification_system_setting_set_dnd_allow_exceptions(nullptr,
      NOTIFICATION_DND_ALLOWED_CALLS, 0);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationSettingTest, system_setting_update_system_setting_null_handle) {
  int ret = notification_system_setting_update_system_setting(nullptr);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/*
 * NULL output pointer tests for notification_system_setting functions
 */
TEST_F(NotificationSettingTest, system_setting_get_do_not_disturb_null_value) {
  notification_system_setting_h sys_setting;
  sys_setting = (struct notification_system_setting *)calloc(1, sizeof(struct notification_system_setting));

  int ret = notification_system_setting_get_do_not_disturb(sys_setting, nullptr);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);

  notification_system_setting_free_system_setting(sys_setting);
}

TEST_F(NotificationSettingTest, system_setting_get_visibility_class_null_value) {
  notification_system_setting_h sys_setting;
  sys_setting = (struct notification_system_setting *)calloc(1, sizeof(struct notification_system_setting));

  int ret = notification_system_setting_get_visibility_class(sys_setting, nullptr);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);

  notification_system_setting_free_system_setting(sys_setting);
}

TEST_F(NotificationSettingTest, system_setting_dnd_schedule_get_enabled_null_value) {
  notification_system_setting_h sys_setting;
  sys_setting = (struct notification_system_setting *)calloc(1, sizeof(struct notification_system_setting));

  int ret = notification_system_setting_dnd_schedule_get_enabled(sys_setting, nullptr);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);

  notification_system_setting_free_system_setting(sys_setting);
}

TEST_F(NotificationSettingTest, system_setting_dnd_schedule_get_day_null_value) {
  notification_system_setting_h sys_setting;
  sys_setting = (struct notification_system_setting *)calloc(1, sizeof(struct notification_system_setting));

  int ret = notification_system_setting_dnd_schedule_get_day(sys_setting, nullptr);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);

  notification_system_setting_free_system_setting(sys_setting);
}

TEST_F(NotificationSettingTest, system_setting_dnd_schedule_get_start_time_null_hour) {
  notification_system_setting_h sys_setting;
  sys_setting = (struct notification_system_setting *)calloc(1, sizeof(struct notification_system_setting));

  int min = 0;
  int ret = notification_system_setting_dnd_schedule_get_start_time(sys_setting, nullptr, &min);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);

  notification_system_setting_free_system_setting(sys_setting);
}

TEST_F(NotificationSettingTest, system_setting_dnd_schedule_get_start_time_null_min) {
  notification_system_setting_h sys_setting;
  sys_setting = (struct notification_system_setting *)calloc(1, sizeof(struct notification_system_setting));

  int hour = 0;
  int ret = notification_system_setting_dnd_schedule_get_start_time(sys_setting, &hour, nullptr);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);

  notification_system_setting_free_system_setting(sys_setting);
}

TEST_F(NotificationSettingTest, system_setting_dnd_schedule_get_end_time_null_hour) {
  notification_system_setting_h sys_setting;
  sys_setting = (struct notification_system_setting *)calloc(1, sizeof(struct notification_system_setting));

  int min = 0;
  int ret = notification_system_setting_dnd_schedule_get_end_time(sys_setting, nullptr, &min);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);

  notification_system_setting_free_system_setting(sys_setting);
}

TEST_F(NotificationSettingTest, system_setting_dnd_schedule_get_end_time_null_min) {
  notification_system_setting_h sys_setting;
  sys_setting = (struct notification_system_setting *)calloc(1, sizeof(struct notification_system_setting));

  int hour = 0;
  int ret = notification_system_setting_dnd_schedule_get_end_time(sys_setting, &hour, nullptr);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);

  notification_system_setting_free_system_setting(sys_setting);
}

TEST_F(NotificationSettingTest, system_setting_get_lock_screen_content_null_value) {
  notification_system_setting_h sys_setting;
  sys_setting = (struct notification_system_setting *)calloc(1, sizeof(struct notification_system_setting));

  int ret = notification_system_setting_get_lock_screen_content(sys_setting, nullptr);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);

  notification_system_setting_free_system_setting(sys_setting);
}

TEST_F(NotificationSettingTest, system_setting_get_dnd_allow_exceptions_null_value) {
  notification_system_setting_h sys_setting;
  sys_setting = (struct notification_system_setting *)calloc(1, sizeof(struct notification_system_setting));

  int ret = notification_system_setting_get_dnd_allow_exceptions(sys_setting,
      NOTIFICATION_DND_ALLOWED_CALLS, nullptr);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);

  notification_system_setting_free_system_setting(sys_setting);
}

/*
 * Valid handle get/set round-trip tests for notification_system_setting
 */
TEST_F(NotificationSettingTest, system_setting_get_set_do_not_disturb_valid) {
  notification_system_setting_h sys_setting;
  sys_setting = (struct notification_system_setting *)calloc(1, sizeof(struct notification_system_setting));

  int ret = notification_system_setting_set_do_not_disturb(sys_setting, true);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  bool value = false;
  ret = notification_system_setting_get_do_not_disturb(sys_setting, &value);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);
  ASSERT_EQ(value, true);

  notification_system_setting_free_system_setting(sys_setting);
}

TEST_F(NotificationSettingTest, system_setting_get_set_visibility_class_valid) {
  notification_system_setting_h sys_setting;
  sys_setting = (struct notification_system_setting *)calloc(1, sizeof(struct notification_system_setting));

  int ret = notification_system_setting_set_visibility_class(sys_setting, 3);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  int value = 0;
  ret = notification_system_setting_get_visibility_class(sys_setting, &value);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);
  ASSERT_EQ(value, 3);

  notification_system_setting_free_system_setting(sys_setting);
}

TEST_F(NotificationSettingTest, system_setting_dnd_schedule_get_set_enabled_valid) {
  notification_system_setting_h sys_setting;
  sys_setting = (struct notification_system_setting *)calloc(1, sizeof(struct notification_system_setting));

  int ret = notification_system_setting_dnd_schedule_set_enabled(sys_setting, true);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  bool enabled = false;
  ret = notification_system_setting_dnd_schedule_get_enabled(sys_setting, &enabled);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);
  ASSERT_EQ(enabled, true);

  notification_system_setting_free_system_setting(sys_setting);
}

TEST_F(NotificationSettingTest, system_setting_dnd_schedule_get_set_day_valid) {
  notification_system_setting_h sys_setting;
  sys_setting = (struct notification_system_setting *)calloc(1, sizeof(struct notification_system_setting));

  int ret = notification_system_setting_dnd_schedule_set_day(sys_setting, DND_SCHEDULE_WEEK_FLAG_MONDAY);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  int day = 0;
  ret = notification_system_setting_dnd_schedule_get_day(sys_setting, &day);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);
  ASSERT_EQ(day, DND_SCHEDULE_WEEK_FLAG_MONDAY);

  notification_system_setting_free_system_setting(sys_setting);
}

TEST_F(NotificationSettingTest, system_setting_dnd_schedule_get_set_start_time_valid) {
  notification_system_setting_h sys_setting;
  sys_setting = (struct notification_system_setting *)calloc(1, sizeof(struct notification_system_setting));

  int ret = notification_system_setting_dnd_schedule_set_start_time(sys_setting, 22, 30);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  int hour = 0, min = 0;
  ret = notification_system_setting_dnd_schedule_get_start_time(sys_setting, &hour, &min);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);
  ASSERT_EQ(hour, 22);
  ASSERT_EQ(min, 30);

  notification_system_setting_free_system_setting(sys_setting);
}

TEST_F(NotificationSettingTest, system_setting_dnd_schedule_get_set_end_time_valid) {
  notification_system_setting_h sys_setting;
  sys_setting = (struct notification_system_setting *)calloc(1, sizeof(struct notification_system_setting));

  int ret = notification_system_setting_dnd_schedule_set_end_time(sys_setting, 7, 15);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  int hour = 0, min = 0;
  ret = notification_system_setting_dnd_schedule_get_end_time(sys_setting, &hour, &min);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);
  ASSERT_EQ(hour, 7);
  ASSERT_EQ(min, 15);

  notification_system_setting_free_system_setting(sys_setting);
}

TEST_F(NotificationSettingTest, system_setting_get_set_lock_screen_content_valid) {
  notification_system_setting_h sys_setting;
  sys_setting = (struct notification_system_setting *)calloc(1, sizeof(struct notification_system_setting));

  int ret = notification_system_setting_set_lock_screen_content(sys_setting, DO_NOT_SHOW_NOTIFICATIONS);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  lock_screen_content_level_e level;
  ret = notification_system_setting_get_lock_screen_content(sys_setting, &level);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);
  ASSERT_EQ(level, DO_NOT_SHOW_NOTIFICATIONS);

  notification_system_setting_free_system_setting(sys_setting);
}

TEST_F(NotificationSettingTest, system_setting_set_dnd_allow_exceptions_valid) {
  notification_system_setting_h sys_setting;
  sys_setting = (struct notification_system_setting *)calloc(1, sizeof(struct notification_system_setting));

  int ret = notification_system_setting_set_dnd_allow_exceptions(sys_setting,
      NOTIFICATION_DND_ALLOWED_CALLS, NOTIFICATION_DND_ALLOWED_CALLS_FAVORITE);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  notification_system_setting_free_system_setting(sys_setting);
}

TEST_F(NotificationSettingTest, free_notification_valid) {
  notification_setting_h setting;
  setting = (struct notification_setting *)calloc(1, sizeof(struct notification_setting));
  setting->package_name = strdup("org.tizen.testpkg");
  setting->app_id = strdup("org.tizen.testapp");

  int ret = notification_setting_free_notification(setting);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);
}

TEST_F(NotificationSettingTest, system_setting_free_valid) {
  notification_system_setting_h sys_setting;
  sys_setting = (struct notification_system_setting *)calloc(1, sizeof(struct notification_system_setting));

  int ret = notification_system_setting_free_system_setting(sys_setting);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);
}

TEST_F(NotificationSettingTest, system_setting_load_null_handle) {
  int ret = notification_system_setting_load_system_setting(nullptr);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationSettingTest, system_setting_load_for_uid_null_handle) {
  int ret = notification_system_setting_load_system_setting_for_uid(nullptr, 5001);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationSettingTest, update_setting_for_uid_null_handle) {
  int ret = notification_setting_update_setting_for_uid(nullptr, 5001);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationSettingTest, system_setting_update_for_uid_null_handle) {
  int ret = notification_system_setting_update_system_setting_for_uid(nullptr, 5001);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationSettingTest, get_setting_by_appid_for_uid_null_app_id) {
  notification_setting_h setting = nullptr;
  int ret = notification_setting_get_setting_by_appid_for_uid(nullptr, &setting, 5001);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationSettingTest, get_setting_by_appid_for_uid_null_setting) {
  int ret = notification_setting_get_setting_by_appid_for_uid("org.tizen.testapp", nullptr, 5001);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationSettingTest, get_setting_by_appid_for_uid_both_null) {
  int ret = notification_setting_get_setting_by_appid_for_uid(nullptr, nullptr, 5001);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationSettingTest, get_setting_array_for_uid_null_array) {
  int count = 0;
  int ret = notification_setting_get_setting_array_for_uid(nullptr, &count, 5001);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationSettingTest, get_setting_array_for_uid_null_count) {
  notification_setting_h setting_array = nullptr;
  int ret = notification_setting_get_setting_array_for_uid(&setting_array, nullptr, 5001);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationSettingTest, get_setting_array_for_uid_both_null) {
  int ret = notification_setting_get_setting_array_for_uid(nullptr, nullptr, 5001);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}
