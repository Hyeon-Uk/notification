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
#include "notification/include/notification_ipc.h"

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

class NotificationIpcTest : public TestFixture {
 public:
  NotificationIpcTest() : TestFixture(std::make_unique<::Mocks>()) {}
  virtual void SetUp() {}
  virtual void TearDown() {}
};

TEST_F(NotificationIpcTest, notification_ipc_make_noti_from_gvariant_null_noti) {
  GVariant *body = g_variant_new("a{iv}", NULL);
  int ret = notification_ipc_make_noti_from_gvariant(nullptr, body);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
  if (body)
    g_variant_unref(body);
}

TEST_F(NotificationIpcTest, notification_ipc_make_noti_from_gvariant_null_variant) {
  EXPECT_CALL(GetMock<AulMock>(), aul_app_get_appid_bypid(_, _, _))
      .WillRepeatedly(Invoke(__fake_aul_app_get_appid_bypid));

  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  int ret = notification_ipc_make_noti_from_gvariant(noti, nullptr);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);

  notification_free(noti);
}

TEST_F(NotificationIpcTest, notification_ipc_make_gvariant_from_noti_valid) {
  EXPECT_CALL(GetMock<AulMock>(), aul_app_get_appid_bypid(_, _, _))
      .WillRepeatedly(Invoke(__fake_aul_app_get_appid_bypid));

  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  GVariant *body = notification_ipc_make_gvariant_from_noti(noti, false);
  EXPECT_NE(body, nullptr);

  if (body)
    g_variant_unref(body);

  notification_free(noti);
}

TEST_F(NotificationIpcTest, notification_ipc_make_gvariant_from_noti_with_translate) {
  EXPECT_CALL(GetMock<AulMock>(), aul_app_get_appid_bypid(_, _, _))
      .WillRepeatedly(Invoke(__fake_aul_app_get_appid_bypid));

  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  GVariant *body = notification_ipc_make_gvariant_from_noti(noti, true);
  EXPECT_NE(body, nullptr);

  if (body)
    g_variant_unref(body);

  notification_free(noti);
}

TEST_F(NotificationIpcTest, notification_ipc_make_noti_from_gvariant_roundtrip) {
  EXPECT_CALL(GetMock<AulMock>(), aul_app_get_appid_bypid(_, _, _))
      .WillRepeatedly(Invoke(__fake_aul_app_get_appid_bypid));

  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  notification_set_text(noti, NOTIFICATION_TEXT_TYPE_TITLE, "test_title",
      NULL, NOTIFICATION_VARIABLE_TYPE_NONE);

  GVariant *body = notification_ipc_make_gvariant_from_noti(noti, false);
  ASSERT_NE(body, nullptr);

  notification_h noti2 = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti2, nullptr);

  int ret = notification_ipc_make_noti_from_gvariant(noti2, body);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  g_variant_unref(body);
  notification_free(noti);
  notification_free(noti2);
}
