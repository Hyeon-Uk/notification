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
#include "mock/package_manager_mock.hh"

#include "notification/include/notification.h"
#include "notification/include/notification_internal.h"

namespace {

int __fake_aul_app_get_appid_bypid(int pid, char* appid, int len) {
  snprintf(appid, len, "%s", "org.tizen.testappid");
  return AUL_R_OK;
}

class Mocks :
    virtual public ::testing::NiceMock<AulMock>,
    virtual public ::testing::NiceMock<GlibMock>,
    virtual public ::testing::NiceMock<PackageManagerMock> {};
} // namespace

using ::testing::_;
using ::testing::Invoke;
using ::testing::Return;

class NotificationTidlTest : public TestFixture {
 public:
  NotificationTidlTest() : TestFixture(std::make_unique<::Mocks>()) {}
  virtual void SetUp() {}
  virtual void TearDown() {}
};

TEST_F(NotificationTidlTest, notification_post_null_noti) {
  int ret = notification_post(nullptr);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationTidlTest, notification_update_null_noti) {
  int ret = notification_update(nullptr);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationTidlTest, notification_delete_null_noti) {
  int ret = notification_delete(nullptr);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationTidlTest, notification_delete_all_invalid_type) {
  int ret = notification_delete_all((notification_type_e)-1);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationTidlTest, notification_load_by_tag_null_tag) {
  notification_h noti = notification_load_by_tag(nullptr);
  EXPECT_EQ(noti, nullptr);
}

TEST_F(NotificationTidlTest, notification_delete_all_type_none_returns_error) {
  int ret = notification_delete_all(NOTIFICATION_TYPE_NONE);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationTidlTest, notification_save_as_template_both_null) {
  int ret = notification_save_as_template(nullptr, nullptr);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationTidlTest, notification_create_from_template_null_returns_null) {
  notification_h noti = notification_create_from_template(nullptr);
  EXPECT_EQ(noti, nullptr);
}
