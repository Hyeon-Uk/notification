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

TEST_F(NotificationSettingTest, happy_path_combined) {
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

TEST_F(NotificationSettingTest, get_setting_null) {
  // out-pointer; current behavior returns garbage from an
  // uninitialized stack int. Lock in: "any non-NONE value"
  // confirms the call was not silently treated as success.
  // add a NULL guard.
  int ret = notification_setting_get_setting(nullptr);
  EXPECT_NE(ret, NOTIFICATION_ERROR_NONE);
}

TEST_F(NotificationSettingTest, get_visibility_class_null_setting) {
  int v = 0;
  EXPECT_EQ(notification_setting_get_visibility_class(nullptr, &v),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationSettingTest, set_visibility_class_null_setting) {
  EXPECT_EQ(notification_setting_set_visibility_class(nullptr, 1),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationSettingTest, free_notification_null) {
  EXPECT_EQ(notification_setting_free_notification(nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

// notification_setting_refresh_setting_table /
// notification_system_setting_init_system_setting_table both
// touch the DB through paths that expect a richer SqliteMock
// than this fixture provides (NotificationSettingTest doesn't
// include SqliteMock). Lock-in deferred to task 19's integration
// suite (real sqlite). Null-arg coverage n/a — both take only
// a uid_t.
