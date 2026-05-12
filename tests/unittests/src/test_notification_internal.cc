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

/* 1. Default button */
TEST_F(NotificationInternalTest, notification_set_get_default_button_valid) {
  EXPECT_CALL(GetMock<AulMock>(), aul_app_get_appid_bypid(_, _, _))
      .WillRepeatedly(Invoke(__fake_aul_app_get_appid_bypid));

  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  int ret = notification_set_default_button(noti, NOTIFICATION_BUTTON_1);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  notification_button_index_e index;
  ret = notification_get_default_button(noti, &index);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(index, NOTIFICATION_BUTTON_1);

  notification_free(noti);
}

TEST_F(NotificationInternalTest, notification_set_default_button_null_noti) {
  int ret = notification_set_default_button(nullptr, NOTIFICATION_BUTTON_1);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationInternalTest, notification_get_default_button_null_noti) {
  notification_button_index_e index;
  int ret = notification_get_default_button(nullptr, &index);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationInternalTest, notification_get_default_button_null_output) {
  EXPECT_CALL(GetMock<AulMock>(), aul_app_get_appid_bypid(_, _, _))
      .WillRepeatedly(Invoke(__fake_aul_app_get_appid_bypid));

  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  int ret = notification_get_default_button(noti, nullptr);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);

  notification_free(noti);
}

/* 2. Ongoing value type */
TEST_F(NotificationInternalTest, notification_set_get_ongoing_value_type_valid) {
  EXPECT_CALL(GetMock<AulMock>(), aul_app_get_appid_bypid(_, _, _))
      .WillRepeatedly(Invoke(__fake_aul_app_get_appid_bypid));

  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  int ret = notification_set_ongoing_value_type(noti, NOTIFICATION_ONGOING_VALUE_TYPE_TIME);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  notification_ongoing_value_type_e type;
  ret = notification_get_ongoing_value_type(noti, &type);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(type, NOTIFICATION_ONGOING_VALUE_TYPE_TIME);

  notification_free(noti);
}

TEST_F(NotificationInternalTest, notification_set_ongoing_value_type_null_noti) {
  int ret = notification_set_ongoing_value_type(nullptr, NOTIFICATION_ONGOING_VALUE_TYPE_PERCENT);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationInternalTest, notification_get_ongoing_value_type_null_noti) {
  notification_ongoing_value_type_e type;
  int ret = notification_get_ongoing_value_type(nullptr, &type);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* 3. Ongoing time */
TEST_F(NotificationInternalTest, notification_set_get_ongoing_time_valid) {
  EXPECT_CALL(GetMock<AulMock>(), aul_app_get_appid_bypid(_, _, _))
      .WillRepeatedly(Invoke(__fake_aul_app_get_appid_bypid));

  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  int ret = notification_set_ongoing_time(noti, 5, 30);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  int current = 0, duration = 0;
  ret = notification_get_ongoing_time(noti, &current, &duration);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(current, 5);
  EXPECT_EQ(duration, 30);

  notification_free(noti);
}

TEST_F(NotificationInternalTest, notification_set_ongoing_time_null_noti) {
  int ret = notification_set_ongoing_time(nullptr, 0, 30);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationInternalTest, notification_get_ongoing_time_null_noti) {
  int current, duration;
  int ret = notification_get_ongoing_time(nullptr, &current, &duration);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* 4. Hide timeout */
TEST_F(NotificationInternalTest, notification_set_get_hide_timeout_valid) {
  EXPECT_CALL(GetMock<AulMock>(), aul_app_get_appid_bypid(_, _, _))
      .WillRepeatedly(Invoke(__fake_aul_app_get_appid_bypid));

  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  int ret = notification_set_hide_timeout(noti, 10);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  int timeout = 0;
  ret = notification_get_hide_timeout(noti, &timeout);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(timeout, 10);

  notification_free(noti);
}

TEST_F(NotificationInternalTest, notification_set_hide_timeout_null_noti) {
  int ret = notification_set_hide_timeout(nullptr, 10);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationInternalTest, notification_get_hide_timeout_null_noti) {
  int timeout;
  int ret = notification_get_hide_timeout(nullptr, &timeout);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* 5. Delete timeout */
TEST_F(NotificationInternalTest, notification_set_get_delete_timeout_valid) {
  EXPECT_CALL(GetMock<AulMock>(), aul_app_get_appid_bypid(_, _, _))
      .WillRepeatedly(Invoke(__fake_aul_app_get_appid_bypid));

  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  int ret = notification_set_delete_timeout(noti, 20);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  int timeout = 0;
  ret = notification_get_delete_timeout(noti, &timeout);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(timeout, 20);

  notification_free(noti);
}

TEST_F(NotificationInternalTest, notification_set_delete_timeout_null_noti) {
  int ret = notification_set_delete_timeout(nullptr, 10);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationInternalTest, notification_get_delete_timeout_null_noti) {
  int timeout;
  int ret = notification_get_delete_timeout(nullptr, &timeout);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* 6. Text input max length */
TEST_F(NotificationInternalTest, notification_get_text_input_max_length_valid) {
  EXPECT_CALL(GetMock<AulMock>(), aul_app_get_appid_bypid(_, _, _))
      .WillRepeatedly(Invoke(__fake_aul_app_get_appid_bypid));

  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  int max_length = 0;
  int ret = notification_get_text_input_max_length(noti, &max_length);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  notification_free(noti);
}

TEST_F(NotificationInternalTest, notification_get_text_input_max_length_null_noti) {
  int max_length;
  int ret = notification_get_text_input_max_length(nullptr, &max_length);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationInternalTest, notification_get_text_input_max_length_null_output) {
  EXPECT_CALL(GetMock<AulMock>(), aul_app_get_appid_bypid(_, _, _))
      .WillRepeatedly(Invoke(__fake_aul_app_get_appid_bypid));

  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  int ret = notification_get_text_input_max_length(noti, nullptr);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);

  notification_free(noti);
}

/* 7. Event flag */
TEST_F(NotificationInternalTest, notification_get_event_flag_valid) {
  EXPECT_CALL(GetMock<AulMock>(), aul_app_get_appid_bypid(_, _, _))
      .WillRepeatedly(Invoke(__fake_aul_app_get_appid_bypid));

  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  bool event_flag = true;
  int ret = notification_get_event_flag(noti, &event_flag);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);
  EXPECT_FALSE(event_flag);

  notification_free(noti);
}

TEST_F(NotificationInternalTest, notification_get_event_flag_null_noti) {
  bool event_flag;
  int ret = notification_get_event_flag(nullptr, &event_flag);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationInternalTest, notification_get_event_flag_null_output) {
  EXPECT_CALL(GetMock<AulMock>(), aul_app_get_appid_bypid(_, _, _))
      .WillRepeatedly(Invoke(__fake_aul_app_get_appid_bypid));

  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  int ret = notification_get_event_flag(noti, nullptr);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);

  notification_free(noti);
}

/* 8. Extension data */
TEST_F(NotificationInternalTest, notification_set_extension_data_valid) {
  EXPECT_CALL(GetMock<AulMock>(), aul_app_get_appid_bypid(_, _, _))
      .WillRepeatedly(Invoke(__fake_aul_app_get_appid_bypid));

  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  bundle *b = bundle_create();
  ASSERT_NE(b, nullptr);
  bundle_add_str(b, "test_key", "test_value");

  int ret = notification_set_extension_data(noti, "ext_key", b);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  bundle *out = nullptr;
  ret = notification_get_extension_data(noti, "ext_key", &out);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);
  ASSERT_NE(out, nullptr);
  bundle_free(out);

  bundle_free(b);
  notification_free(noti);
}

TEST_F(NotificationInternalTest, notification_set_extension_data_null_noti) {
  bundle *b = bundle_create();
  int ret = notification_set_extension_data(nullptr, "key", b);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
  bundle_free(b);
}

TEST_F(NotificationInternalTest, notification_set_extension_data_null_key) {
  EXPECT_CALL(GetMock<AulMock>(), aul_app_get_appid_bypid(_, _, _))
      .WillRepeatedly(Invoke(__fake_aul_app_get_appid_bypid));

  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  bundle *b = bundle_create();
  int ret = notification_set_extension_data(noti, nullptr, b);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);

  bundle_free(b);
  notification_free(noti);
}

TEST_F(NotificationInternalTest, notification_get_extension_data_null_noti) {
  bundle *out;
  int ret = notification_get_extension_data(nullptr, "key", &out);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationInternalTest, notification_get_extension_data_null_key) {
  EXPECT_CALL(GetMock<AulMock>(), aul_app_get_appid_bypid(_, _, _))
      .WillRepeatedly(Invoke(__fake_aul_app_get_appid_bypid));

  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  bundle *out;
  int ret = notification_get_extension_data(noti, nullptr, &out);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);

  notification_free(noti);
}

/* 9. App label */
TEST_F(NotificationInternalTest, notification_set_get_app_label_valid) {
  EXPECT_CALL(GetMock<AulMock>(), aul_app_get_appid_bypid(_, _, _))
      .WillRepeatedly(Invoke(__fake_aul_app_get_appid_bypid));

  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  int ret = notification_set_app_label(noti, const_cast<char*>("TestLabel"));
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  char *label = nullptr;
  ret = notification_get_app_label(noti, &label);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);
  ASSERT_NE(label, nullptr);
  EXPECT_STREQ(label, "TestLabel");

  notification_free(noti);
}

TEST_F(NotificationInternalTest, notification_set_app_label_null_noti) {
  int ret = notification_set_app_label(nullptr, const_cast<char*>("label"));
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationInternalTest, notification_set_app_label_null_label) {
  EXPECT_CALL(GetMock<AulMock>(), aul_app_get_appid_bypid(_, _, _))
      .WillRepeatedly(Invoke(__fake_aul_app_get_appid_bypid));

  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  int ret = notification_set_app_label(noti, nullptr);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);

  notification_free(noti);
}

TEST_F(NotificationInternalTest, notification_get_app_label_null_noti) {
  char *label;
  int ret = notification_get_app_label(nullptr, &label);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationInternalTest, notification_get_app_label_null_label) {
  EXPECT_CALL(GetMock<AulMock>(), aul_app_get_appid_bypid(_, _, _))
      .WillRepeatedly(Invoke(__fake_aul_app_get_appid_bypid));

  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  int ret = notification_get_app_label(noti, nullptr);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);

  notification_free(noti);
}

/* 10. Pairing type */
TEST_F(NotificationInternalTest, notification_set_get_pairing_type_valid) {
  EXPECT_CALL(GetMock<AulMock>(), aul_app_get_appid_bypid(_, _, _))
      .WillRepeatedly(Invoke(__fake_aul_app_get_appid_bypid));

  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  int ret = notification_set_pairing_type(noti, true);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  bool pairing = false;
  ret = notification_get_pairing_type(noti, &pairing);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);
  EXPECT_TRUE(pairing);

  notification_free(noti);
}

TEST_F(NotificationInternalTest, notification_set_pairing_type_null_noti) {
  int ret = notification_set_pairing_type(nullptr, true);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationInternalTest, notification_get_pairing_type_null_noti) {
  bool pairing;
  int ret = notification_get_pairing_type(nullptr, &pairing);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* 11. Channel name */
TEST_F(NotificationInternalTest, notification_set_get_channel_name_valid) {
  EXPECT_CALL(GetMock<AulMock>(), aul_app_get_appid_bypid(_, _, _))
      .WillRepeatedly(Invoke(__fake_aul_app_get_appid_bypid));

  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  int ret = notification_set_channel_name(noti, "my_channel");
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  const char *name = nullptr;
  ret = notification_get_channel_name(noti, &name);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);
  ASSERT_NE(name, nullptr);
  EXPECT_STREQ(name, "my_channel");

  notification_free(noti);
}

TEST_F(NotificationInternalTest, notification_set_channel_name_null_noti) {
  int ret = notification_set_channel_name(nullptr, "channel");
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationInternalTest, notification_get_channel_name_null_noti) {
  const char *name;
  int ret = notification_get_channel_name(nullptr, &name);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* 12. Channel operations */
TEST_F(NotificationInternalTest, notification_channel_create_free_valid) {
  EXPECT_CALL(GetMock<AulMock>(), aul_app_get_appid_bypid(_, _, _))
      .WillRepeatedly(Invoke(__fake_aul_app_get_appid_bypid));

  notification_channel_h channel = nullptr;
  int ret = notification_channel_create("test_channel", &channel);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);
  ASSERT_NE(channel, nullptr);

  notification_channel_free(channel);
}

TEST_F(NotificationInternalTest, notification_channel_set_get_blockable) {
  EXPECT_CALL(GetMock<AulMock>(), aul_app_get_appid_bypid(_, _, _))
      .WillRepeatedly(Invoke(__fake_aul_app_get_appid_bypid));

  notification_channel_h channel = nullptr;
  int ret = notification_channel_create("test_channel", &channel);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  ret = notification_channel_set_blockable(channel, true);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  bool blockable = false;
  ret = notification_channel_get_blockable(channel, &blockable);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);
  EXPECT_TRUE(blockable);

  notification_channel_free(channel);
}

TEST_F(NotificationInternalTest, notification_channel_set_get_block) {
  EXPECT_CALL(GetMock<AulMock>(), aul_app_get_appid_bypid(_, _, _))
      .WillRepeatedly(Invoke(__fake_aul_app_get_appid_bypid));

  notification_channel_h channel = nullptr;
  int ret = notification_channel_create("test_channel", &channel);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  ret = notification_channel_set_block(channel, true);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  bool block = false;
  ret = notification_channel_get_block(channel, &block);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);
  EXPECT_TRUE(block);

  notification_channel_free(channel);
}

TEST_F(NotificationInternalTest, notification_channel_get_name_valid) {
  EXPECT_CALL(GetMock<AulMock>(), aul_app_get_appid_bypid(_, _, _))
      .WillRepeatedly(Invoke(__fake_aul_app_get_appid_bypid));

  notification_channel_h channel = nullptr;
  int ret = notification_channel_create("my_ch", &channel);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  const char *name = nullptr;
  ret = notification_channel_get_name(channel, &name);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);
  ASSERT_NE(name, nullptr);
  EXPECT_STREQ(name, "my_ch");

  notification_channel_free(channel);
}

TEST_F(NotificationInternalTest, notification_channel_clone_valid) {
  EXPECT_CALL(GetMock<AulMock>(), aul_app_get_appid_bypid(_, _, _))
      .WillRepeatedly(Invoke(__fake_aul_app_get_appid_bypid));

  notification_channel_h channel = nullptr;
  int ret = notification_channel_create("clone_test", &channel);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  notification_channel_h cloned = nullptr;
  ret = notification_channel_clone(channel, &cloned);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);
  ASSERT_NE(cloned, nullptr);

  const char *name = nullptr;
  ret = notification_channel_get_name(cloned, &name);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);
  EXPECT_STREQ(name, "clone_test");

  notification_channel_free(cloned);
  notification_channel_free(channel);
}

TEST_F(NotificationInternalTest, notification_channel_set_blockable_null) {
  int ret = notification_channel_set_blockable(nullptr, true);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationInternalTest, notification_channel_get_blockable_null) {
  bool blockable;
  int ret = notification_channel_get_blockable(nullptr, &blockable);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationInternalTest, notification_channel_set_block_null) {
  int ret = notification_channel_set_block(nullptr, true);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationInternalTest, notification_channel_get_block_null) {
  bool block;
  int ret = notification_channel_get_block(nullptr, &block);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationInternalTest, notification_channel_get_name_null) {
  const char *name;
  int ret = notification_channel_get_name(nullptr, &name);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationInternalTest, notification_channel_clone_null) {
  notification_channel_h cloned;
  int ret = notification_channel_clone(nullptr, &cloned);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* 13. Check box checked */
TEST_F(NotificationInternalTest, notification_set_get_check_box_checked_valid) {
  EXPECT_CALL(GetMock<AulMock>(), aul_app_get_appid_bypid(_, _, _))
      .WillRepeatedly(Invoke(__fake_aul_app_get_appid_bypid));

  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  int ret = notification_set_check_box(noti, true, false);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  ret = notification_set_check_box_checked(noti, true);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  bool checked = false;
  ret = notification_get_check_box_checked(noti, &checked);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);
  EXPECT_TRUE(checked);

  notification_free(noti);
}

TEST_F(NotificationInternalTest, notification_set_check_box_checked_null_noti) {
  int ret = notification_set_check_box_checked(nullptr, true);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationInternalTest, notification_get_check_box_checked_null_noti) {
  bool checked;
  int ret = notification_get_check_box_checked(nullptr, &checked);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* 14. Post/update/delete for uid - NULL noti tests */
TEST_F(NotificationInternalTest, notification_post_for_uid_null_noti) {
  int ret = notification_post_for_uid(nullptr, 5001);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationInternalTest, notification_update_for_uid_null_noti) {
  int ret = notification_update_for_uid(nullptr, 5001);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationInternalTest, notification_delete_for_uid_null_noti) {
  int ret = notification_delete_for_uid(nullptr, 5001);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* 15. Load by tag for uid - NULL tag test */
TEST_F(NotificationInternalTest, notification_load_by_tag_for_uid_null_tag) {
  notification_h noti = notification_load_by_tag_for_uid(nullptr, 5001);
  ASSERT_EQ(noti, nullptr);
}

/* 16. Delete by display applist - invalid param test */
TEST_F(NotificationInternalTest, notification_delete_by_display_applist_invalid) {
  int ret = notification_delete_by_display_applist(0);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* 17. Indirect request - NULL noti test */
TEST_F(NotificationInternalTest, notification_set_indirect_request_null_noti) {
  int ret = notification_set_indirect_request(nullptr, 100, 5001);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* 18. Deprecated wrappers */
TEST_F(NotificationInternalTest, notification_set_icon_null_noti) {
  int ret = notification_set_icon(nullptr, "/path/to/icon.png");
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationInternalTest, notification_get_icon_null_noti) {
  char *icon_path;
  int ret = notification_get_icon(nullptr, &icon_path);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationInternalTest, notification_set_get_icon_valid) {
  EXPECT_CALL(GetMock<AulMock>(), aul_app_get_appid_bypid(_, _, _))
      .WillRepeatedly(Invoke(__fake_aul_app_get_appid_bypid));

  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  int ret = notification_set_icon(noti, "/path/to/icon.png");
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  char *icon_path = nullptr;
  ret = notification_get_icon(noti, &icon_path);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);
  ASSERT_NE(icon_path, nullptr);
  EXPECT_STREQ(icon_path, "/path/to/icon.png");

  notification_free(noti);
}

TEST_F(NotificationInternalTest, notification_set_title_null_noti) {
  int ret = notification_set_title(nullptr, "title", nullptr);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationInternalTest, notification_get_title_null_noti) {
  char *title;
  int ret = notification_get_title(nullptr, &title, nullptr);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationInternalTest, notification_set_get_title_valid) {
  EXPECT_CALL(GetMock<AulMock>(), aul_app_get_appid_bypid(_, _, _))
      .WillRepeatedly(Invoke(__fake_aul_app_get_appid_bypid));

  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  int ret = notification_set_title(noti, "My Title", nullptr);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  char *title = nullptr;
  ret = notification_get_title(noti, &title, nullptr);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);
  ASSERT_NE(title, nullptr);
  EXPECT_STREQ(title, "My Title");

  notification_free(noti);
}

TEST_F(NotificationInternalTest, notification_set_content_null_noti) {
  int ret = notification_set_content(nullptr, "content", nullptr);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationInternalTest, notification_get_content_null_noti) {
  char *content;
  int ret = notification_get_content(nullptr, &content, nullptr);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationInternalTest, notification_set_get_content_valid) {
  EXPECT_CALL(GetMock<AulMock>(), aul_app_get_appid_bypid(_, _, _))
      .WillRepeatedly(Invoke(__fake_aul_app_get_appid_bypid));

  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  int ret = notification_set_content(noti, "My Content", nullptr);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  char *content = nullptr;
  ret = notification_get_content(noti, &content, nullptr);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);
  ASSERT_NE(content, nullptr);
  EXPECT_STREQ(content, "My Content");

  notification_free(noti);
}

/* 19. Insert for uid - NULL noti test */
TEST_F(NotificationInternalTest, notification_insert_for_uid_null_noti) {
  int priv_id;
  int ret = notification_insert_for_uid(nullptr, &priv_id, 5001);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}
