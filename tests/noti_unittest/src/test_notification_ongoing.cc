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

#include "notification/include/notification.h"
#include "notification/include/notification_ongoing.h"

namespace {

GDBusConnection* __fake_g_bus_get_sync(GBusType type, GCancellable* cancellable, GError** error) {
  GDBusConnection* conn = reinterpret_cast<GDBusConnection*>(calloc(1, 4));

  return conn;
}

guint __fake_g_dbus_connection_signal_subscribe(GDBusConnection* arg0,
    const gchar* arg1, const gchar* arg2, const gchar* arg3, const gchar* arg4,
    const gchar* arg5, GDBusSignalFlags arg6, GDBusSignalCallback arg7,
    gpointer arg8, GDestroyNotify arg9) {
  arg7(nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);

  return 1;
}

class Mocks :
    virtual public ::testing::NiceMock<GlibMock> {};
} // namespace

using ::testing::_;
using ::testing::Invoke;
using ::testing::Return;

class NotificationOngoingTest : public TestFixture {
 public:
  NotificationOngoingTest() : TestFixture(std::make_unique<::Mocks>()) {}
  virtual void SetUp() {
  }
  virtual void TearDown() {
  }
};

void __notification_ongoing_update_cb(struct ongoing_info_s *info, void *data) {
}

TEST_F(NotificationOngoingTest, notification_ongoing_test) {
  EXPECT_CALL(GetMock<GlibMock>(), g_bus_get_sync(_, _, _))
      .WillRepeatedly(Invoke(__fake_g_bus_get_sync));

  EXPECT_CALL(GetMock<GlibMock>(),
      g_dbus_connection_signal_subscribe(_, _, _, _, _, _, _, _, _, _))
      .WillOnce(Invoke(__fake_g_dbus_connection_signal_subscribe));

  EXPECT_CALL(GetMock<GlibMock>(), g_dbus_connection_emit_signal(_, _, _, _, _, _, _))
      .WillRepeatedly(Return(true));

  EXPECT_CALL(GetMock<GlibMock>(), g_dbus_connection_flush_sync(_, _, _))
      .WillRepeatedly(Return(true));

  int ret;

  ret = notification_ongoing_update_cb_set(__notification_ongoing_update_cb, nullptr);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  ret = notification_ongoing_update_cb_unset();
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  ret = notification_ongoing_update_progress("org.tizen.caller_appid", 1, 10.0);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  ret = notification_ongoing_update_size("org.tizen.caller_appid", 1, 10.0);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  ret = notification_ongoing_update_content("org.tizen.caller_appid", 1, "test_content");
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);
}
