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

GDBusMessage* __fake_g_dbus_connection_send_message_with_reply_sync(
    GDBusConnection* conn, GDBusMessage* msg, GDBusSendMessageFlags flags,
    gint timeout, volatile guint32* out_serial, GCancellable* cancellable,
    GError** error) {
  GDBusMessage* ret_msg = static_cast<GDBusMessage*>(g_object_new(G_TYPE_OBJECT, NULL));

  return ret_msg;
}

GDBusMessage* __fake_g_dbus_message_new_method_call(const gchar* arg0,
    const gchar* arg1, const gchar* arg2, const gchar* arg3) {
  GDBusMessage* ret_msg = static_cast<GDBusMessage*>(g_object_new(G_TYPE_OBJECT, NULL));

  return ret_msg;
}

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
  EXPECT_CALL(GetMock<GlibMock>(), g_bus_watch_name(_, _, _, _, _, _, _))
      .WillOnce(Return(1));

  EXPECT_CALL(GetMock<GlibMock>(), g_dbus_message_new_method_call(_, _, _, _))
      .WillOnce(Invoke(__fake_g_dbus_message_new_method_call));

  EXPECT_CALL(GetMock<GlibMock>(), g_dbus_connection_send_message_with_reply_sync(_, _, _, _, _, _, _))
      .WillOnce(Invoke(__fake_g_dbus_connection_send_message_with_reply_sync));

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

  ret = notification_setting_update_setting(setting);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  ret = notification_setting_free_notification(setting);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);
}
