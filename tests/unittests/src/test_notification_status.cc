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
#include "notification/include/notification_status.h"
#include "notification/include/notification_status_internal.h"

namespace {

GDBusConnection* __fake_g_bus_get_sync_fail(GBusType type,
    GCancellable *cancellable, GError **error) {
  if (error)
    *error = g_error_new(g_quark_from_string("test"), 1, "mock dbus error");
  return nullptr;
}

class Mocks :
    virtual public ::testing::NiceMock<GlibMock> {};
} // namespace

using ::testing::_;
using ::testing::Invoke;
using ::testing::Return;

class NotificationStatusTest : public TestFixture {
 public:
  NotificationStatusTest() : TestFixture(std::make_unique<::Mocks>()) {}
  virtual void SetUp() {}
  virtual void TearDown() {}
};

TEST_F(NotificationStatusTest, notification_status_monitor_message_cb_set_null_callback) {
  int ret = notification_status_monitor_message_cb_set(nullptr, nullptr);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationStatusTest, notification_status_monitor_message_cb_unset_success) {
  int ret = notification_status_monitor_message_cb_unset();
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);
}

TEST_F(NotificationStatusTest, notification_status_message_post_null_message) {
  int ret = notification_status_message_post(nullptr);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationStatusTest, notification_status_message_post_valid) {
  GDBusConnection *fake_conn = (GDBusConnection *)g_object_new(G_TYPE_OBJECT, NULL);

  EXPECT_CALL(GetMock<GlibMock>(), g_bus_get_sync(_, _, _))
    .WillOnce(Return(fake_conn));

  EXPECT_CALL(GetMock<GlibMock>(), g_dbus_connection_emit_signal(_, _, _, _, _, _, _))
    .WillOnce(Return(TRUE));

  EXPECT_CALL(GetMock<GlibMock>(), g_dbus_connection_flush_sync(_, _, _))
    .WillOnce(Return(TRUE));

  int ret = notification_status_message_post("test message");
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);
}

TEST_F(NotificationStatusTest, notification_status_message_post_dbus_fail) {
  EXPECT_CALL(GetMock<GlibMock>(), g_bus_get_sync(_, _, _))
    .WillOnce(Invoke(__fake_g_bus_get_sync_fail));

  int ret = notification_status_message_post("test message");
  ASSERT_EQ(ret, NOTIFICATION_ERROR_FROM_DBUS);
}
