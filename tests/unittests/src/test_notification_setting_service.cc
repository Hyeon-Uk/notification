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
#include "mock/glib_mock.hh"
#include "mock/sqlite_mock.hh"

#include "notification/include/notification.h"
#include "notification/include/notification_setting.h"
#include "notification/include/notification_setting_internal.h"
#include "notification/include/notification_setting_service.h"

namespace {

int __fake_sqlite3_open_v2(const char* arg0, sqlite3** arg1, int arg2, const char* arg3) {
  char** table = (char**)calloc(10, sizeof(char*));
  *arg1 = (sqlite3*)table;

  return SQLITE_OK;
}

class Mocks :
    virtual public ::testing::NiceMock<GlibMock>,
    virtual public ::testing::NiceMock<SqliteMock> {};
} // namespace

using ::testing::_;
using ::testing::Invoke;
using ::testing::Return;

class NotificationSettingServiceTest : public TestFixture {
 public:
  NotificationSettingServiceTest() : TestFixture(std::make_unique<::Mocks>()) {}
  virtual void SetUp() {}
  virtual void TearDown() {}
};

TEST_F(NotificationSettingServiceTest, noti_setting_service_get_setting_by_app_id_null_app_id) {
  notification_setting_h setting = nullptr;
  int ret = noti_setting_service_get_setting_by_app_id(nullptr, &setting, 5001);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationSettingServiceTest, noti_setting_service_get_setting_by_app_id_null_setting) {
  int ret = noti_setting_service_get_setting_by_app_id("org.tizen.testapp", nullptr, 5001);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationSettingServiceTest, noti_setting_service_get_setting_by_app_id_not_exist) {
  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_open_v2(_, _, _, _))
    .WillRepeatedly(Invoke(__fake_sqlite3_open_v2));

  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_close(_))
    .WillRepeatedly(Return(SQLITE_OK));

  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_get_table(_, _, _, _, _, _))
    .WillOnce(Return(SQLITE_OK));

  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_free(_))
    .WillRepeatedly(Return());

  notification_setting_h setting = nullptr;
  int ret = noti_setting_service_get_setting_by_app_id("org.tizen.testapp", &setting, 5001);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NOT_EXIST_ID);
}

TEST_F(NotificationSettingServiceTest, noti_setting_service_get_setting_by_app_id_both_null) {
  int ret = noti_setting_service_get_setting_by_app_id(nullptr, nullptr, 5001);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}
