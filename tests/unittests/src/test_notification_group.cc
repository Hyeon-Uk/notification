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
#include "notification/include/notification_internal.h"

namespace {

int __fake_aul_app_get_appid_bypid(int pid, char* appid, int len) {
  snprintf(appid, len, "%s", "org.tizen.testappid");
  return AUL_R_OK;
}

class Mocks :
    virtual public ::testing::NiceMock<AulMock>,
    virtual public ::testing::NiceMock<GlibMock>,
    virtual public ::testing::NiceMock<SqliteMock> {};
} // namespace

using ::testing::_;
using ::testing::Invoke;
using ::testing::Return;

class NotificationGroupTest : public TestFixture {
 public:
  NotificationGroupTest() : TestFixture(std::make_unique<::Mocks>()) {}
  virtual void SetUp() {}
  virtual void TearDown() {}
};

TEST_F(NotificationGroupTest, notification_get_grouping_list_null_list) {
  notification_list_h list = nullptr;
  int ret = notification_get_grouping_list(NOTIFICATION_TYPE_NOTI, 1, nullptr);
  EXPECT_NE(ret, NOTIFICATION_ERROR_NONE);
}

TEST_F(NotificationGroupTest, notification_delete_group_by_priv_id_valid) {
  EXPECT_CALL(GetMock<AulMock>(), aul_app_get_appid_bypid(_, _, _))
      .WillRepeatedly(Invoke(__fake_aul_app_get_appid_bypid));

  int ret = notification_delete_group_by_priv_id("org.tizen.testapp",
      NOTIFICATION_TYPE_NOTI, 1);
  EXPECT_NE(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationGroupTest, notification_get_count_null_count) {
  int ret = notification_get_count(NOTIFICATION_TYPE_NOTI, "org.tizen.test", 0, 0, nullptr);
  EXPECT_NE(ret, NOTIFICATION_ERROR_NONE);
}
