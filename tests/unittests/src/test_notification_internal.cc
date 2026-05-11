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

class NotificationInternalTest : public TestFixture {
 public:
  NotificationInternalTest() : TestFixture(std::make_unique<::Mocks>()) {}
  virtual void SetUp() {}
  virtual void TearDown() {}
};

TEST_F(NotificationInternalTest, notification_add_deferred_task_null_cb) {
  int ret = notification_add_deferred_task(nullptr, nullptr);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationInternalTest, notification_del_deferred_task_null_cb) {
  int ret = notification_del_deferred_task(nullptr);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationInternalTest, notification_resister_changed_cb_null_callback) {
  int ret = notification_resister_changed_cb(nullptr, nullptr);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationInternalTest, notification_unresister_changed_cb_null_callback) {
  int ret = notification_unresister_changed_cb(nullptr);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationInternalTest, notification_update_progress_null_noti) {
  int ret = notification_update_progress(nullptr, NOTIFICATION_PRIV_ID_NONE, 0.5);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationInternalTest, notification_update_size_null_noti) {
  int ret = notification_update_size(nullptr, NOTIFICATION_PRIV_ID_NONE, 1024.0);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationInternalTest, notification_update_content_null_noti) {
  int ret = notification_update_content(nullptr, NOTIFICATION_PRIV_ID_NONE, "content");
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationInternalTest, notification_set_uid_null_noti) {
  int ret = notification_set_uid(nullptr, 5001);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationInternalTest, notification_get_uid_null_noti) {
  uid_t uid;
  int ret = notification_get_uid(nullptr, &uid);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationInternalTest, notification_get_uid_null_uid) {
  EXPECT_CALL(GetMock<AulMock>(), aul_app_get_appid_bypid(_, _, _))
      .WillRepeatedly(Invoke(__fake_aul_app_get_appid_bypid));

  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  int ret = notification_get_uid(noti, nullptr);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);

  notification_free(noti);
}

TEST_F(NotificationInternalTest, notification_set_get_uid_valid) {
  EXPECT_CALL(GetMock<AulMock>(), aul_app_get_appid_bypid(_, _, _))
      .WillRepeatedly(Invoke(__fake_aul_app_get_appid_bypid));

  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  int ret = notification_set_uid(noti, 5001);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  uid_t uid = 0;
  ret = notification_get_uid(noti, &uid);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(uid, 5001);

  notification_free(noti);
}

TEST_F(NotificationInternalTest, notification_set_priv_id_null_noti) {
  int ret = notification_set_priv_id(nullptr, 1);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationInternalTest, notification_set_priv_id_invalid_id) {
  EXPECT_CALL(GetMock<AulMock>(), aul_app_get_appid_bypid(_, _, _))
      .WillRepeatedly(Invoke(__fake_aul_app_get_appid_bypid));

  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  int ret = notification_set_priv_id(noti, 0);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);

  notification_free(noti);
}

TEST_F(NotificationInternalTest, notification_insert_null_noti) {
  int priv_id;
  int ret = notification_insert(nullptr, &priv_id);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationInternalTest, notification_register_detailed_changed_cb_null) {
  int ret = notification_register_detailed_changed_cb(nullptr, nullptr);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationInternalTest, notification_set_check_box_null_noti) {
  int ret = notification_set_check_box(nullptr, true, false);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationInternalTest, notification_get_check_box_null_noti) {
  bool flag, checked;
  int ret = notification_get_check_box(nullptr, &flag, &checked);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationInternalTest, notification_get_check_box_null_params) {
  EXPECT_CALL(GetMock<AulMock>(), aul_app_get_appid_bypid(_, _, _))
      .WillRepeatedly(Invoke(__fake_aul_app_get_appid_bypid));

  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  int ret = notification_get_check_box(noti, nullptr, nullptr);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);

  notification_free(noti);
}

TEST_F(NotificationInternalTest, notification_set_get_check_box_valid) {
  EXPECT_CALL(GetMock<AulMock>(), aul_app_get_appid_bypid(_, _, _))
      .WillRepeatedly(Invoke(__fake_aul_app_get_appid_bypid));

  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  int ret = notification_set_check_box(noti, true, true);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  bool flag = false, checked = false;
  ret = notification_get_check_box(noti, &flag, &checked);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);
  EXPECT_TRUE(flag);
  EXPECT_TRUE(checked);

  notification_free(noti);
}

TEST_F(NotificationInternalTest, notification_channel_create_null_params) {
  int ret = notification_channel_create(nullptr, nullptr);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationInternalTest, notification_channel_create_null_channel) {
  int ret = notification_channel_create("test_channel", nullptr);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationInternalTest, notification_channel_free_null) {
  notification_channel_free(nullptr);
  /* Should not crash */
}
