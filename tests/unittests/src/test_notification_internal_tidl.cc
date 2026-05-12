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

#include "notification/include/notification.h"
#include "notification/include/notification_setting_internal.h"
#include "notification/include/notification_internal.h"

namespace {

int __fake_aul_app_get_appid_bypid(int pid, char* appid, int len) {
  snprintf(appid, len, "%s", "org.tizen.testappid");
  return AUL_R_OK;
}

class Mocks :
    virtual public ::testing::NiceMock<AulMock>,
    virtual public ::testing::NiceMock<GlibMock> {};
} // namespace

using ::testing::_;
using ::testing::Invoke;
using ::testing::Return;

class NotificationInternalTidlTest : public TestFixture {
 public:
  NotificationInternalTidlTest() : TestFixture(std::make_unique<::Mocks>()) {}
  virtual void SetUp() {
    EXPECT_CALL(GetMock<AulMock>(), aul_app_get_appid_bypid(_, _, _))
        .WillRepeatedly(Invoke(__fake_aul_app_get_appid_bypid));
  }
  virtual void TearDown() {}
};

TEST_F(NotificationInternalTidlTest, make_dnd_allow_exception_null_handle) {
  dnd_allow_exception_h exception = (dnd_allow_exception_h)calloc(1,
      sizeof(struct notification_system_setting_dnd_allow_exception));
  ASSERT_NE(exception, nullptr);

  int ret = make_dnd_allow_exception_from_exception(nullptr, exception);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);

  free(exception);
}

TEST_F(NotificationInternalTidlTest, make_dnd_allow_exception_null_exception) {
  int ret = make_dnd_allow_exception_from_exception((void *)0x1234, nullptr);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationInternalTidlTest, make_dnd_allow_exception_both_null) {
  int ret = make_dnd_allow_exception_from_exception(nullptr, nullptr);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* make_empty_notification: null notihandle */
TEST_F(NotificationInternalTidlTest, make_empty_notification_null) {
  int ret = make_empty_notification(nullptr);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* make_notification_from_noti: null notihandle */
TEST_F(NotificationInternalTidlTest, make_notification_from_noti_null_handle) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  int ret = make_notification_from_noti(nullptr, noti, false);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);

  notification_free(noti);
}

/* make_notification_from_noti: null noti */
TEST_F(NotificationInternalTidlTest, make_notification_from_noti_null_noti) {
  int ret = make_notification_from_noti((void *)0x1234, nullptr, false);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* make_notification_from_noti: both null */
TEST_F(NotificationInternalTidlTest, make_notification_from_noti_both_null) {
  int ret = make_notification_from_noti(nullptr, nullptr, false);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* make_noti_from_notification: null noti output */
TEST_F(NotificationInternalTidlTest, make_noti_from_notification_null_noti) {
  int ret = make_noti_from_notification(nullptr, (void *)0x1234);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* make_noti_from_notification: null notihandle */
TEST_F(NotificationInternalTidlTest, make_noti_from_notification_null_handle) {
  notification_h noti;
  int ret = make_noti_from_notification(&noti, nullptr);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* make_noti_from_notification: both null */
TEST_F(NotificationInternalTidlTest, make_noti_from_notification_both_null) {
  int ret = make_noti_from_notification(nullptr, nullptr);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* make_setting_from_noti_system_setting: null setting output */
TEST_F(NotificationInternalTidlTest,
    make_setting_from_noti_system_setting_null_setting) {
  int ret = make_setting_from_noti_system_setting(nullptr, (void *)0x1234);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* make_setting_from_noti_system_setting: null settinghandle */
TEST_F(NotificationInternalTidlTest,
    make_setting_from_noti_system_setting_null_handle) {
  notification_system_setting_h setting;
  int ret = make_setting_from_noti_system_setting(&setting, nullptr);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* make_setting_from_noti_system_setting: both null */
TEST_F(NotificationInternalTidlTest,
    make_setting_from_noti_system_setting_both_null) {
  int ret = make_setting_from_noti_system_setting(nullptr, nullptr);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* make_noti_system_setting_from_setting: null settinghandle */
TEST_F(NotificationInternalTidlTest,
    make_noti_system_setting_from_setting_null_handle) {
  notification_system_setting_h setting =
      (notification_system_setting_h)calloc(1,
          sizeof(struct notification_system_setting));
  ASSERT_NE(setting, nullptr);

  int ret = make_noti_system_setting_from_setting(nullptr, setting);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);

  free(setting);
}

/* make_noti_system_setting_from_setting: null setting */
TEST_F(NotificationInternalTidlTest,
    make_noti_system_setting_from_setting_null_setting) {
  int ret = make_noti_system_setting_from_setting((void *)0x1234, nullptr);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* make_noti_system_setting_from_setting: both null */
TEST_F(NotificationInternalTidlTest,
    make_noti_system_setting_from_setting_both_null) {
  int ret = make_noti_system_setting_from_setting(nullptr, nullptr);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* make_setting_from_noti_setting: null setting */
TEST_F(NotificationInternalTidlTest,
    make_setting_from_noti_setting_null_setting) {
  int ret = make_setting_from_noti_setting(nullptr, (void *)0x1234);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* make_setting_from_noti_setting: null settinghandle */
TEST_F(NotificationInternalTidlTest,
    make_setting_from_noti_setting_null_handle) {
  struct notification_setting ns = {};
  notification_setting_h setting = &ns;

  int ret = make_setting_from_noti_setting(setting, nullptr);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* make_setting_from_noti_setting: both null */
TEST_F(NotificationInternalTidlTest,
    make_setting_from_noti_setting_both_null) {
  int ret = make_setting_from_noti_setting(nullptr, nullptr);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* make_noti_setting_from_setting: null settinghandle */
TEST_F(NotificationInternalTidlTest,
    make_noti_setting_from_setting_null_handle) {
  struct notification_setting ns = {};
  notification_setting_h setting = &ns;

  int ret = make_noti_setting_from_setting(nullptr, setting);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* make_noti_setting_from_setting: null setting */
TEST_F(NotificationInternalTidlTest,
    make_noti_setting_from_setting_null_setting) {
  int ret = make_noti_setting_from_setting((void *)0x1234, nullptr);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* make_noti_setting_from_setting: both null */
TEST_F(NotificationInternalTidlTest,
    make_noti_setting_from_setting_both_null) {
  int ret = make_noti_setting_from_setting(nullptr, nullptr);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}
