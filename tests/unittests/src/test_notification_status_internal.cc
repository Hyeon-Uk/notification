/*
 * Copyright (c) 2026 Samsung Electronics Co., Ltd.
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

/*
 * N7.5-A: Internal API Coverage for src/notification/src/notification_status.c
 *
 * The production file owns the toast-popup DBus surface:
 *   - notification_status_message_post (public)
 *   - notification_status_monitor_message_cb_set    (internal)
 *   - notification_status_monitor_message_cb_unset  (internal)
 *
 * Behaviours covered (no production change — coverage only):
 *   1. _message_post(NULL) returns INVALID_PARAMETER without
 *      touching the bus (early-out before g_bus_get_sync).
 *   2. _message_post with a real string + g_bus_get_sync failing
 *      returns FROM_DBUS.
 *   3. _message_post with a real string + bus OK + emit_signal OK +
 *      flush OK returns NONE.
 *   4. _monitor_message_cb_set(NULL, ...) returns INVALID_PARAMETER.
 *   5. _monitor_message_cb_set with bus failure returns FROM_DBUS.
 *   6. _monitor_message_cb_unset on a fresh state returns NONE
 *      (no-op tolerant).
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <gio/gio.h>

#include "mock/test_fixture.hh"
#include "mock/glib_mock.hh"

#include "notification/include/notification.h"
#include "notification/include/notification_status.h"
#include "notification/include/notification_status_internal.h"

namespace {

class Mocks :
    virtual public ::testing::NiceMock<GlibMock> {};

class NotificationStatusInternalTest : public TestFixture {
 public:
  NotificationStatusInternalTest()
      : TestFixture(std::make_unique<::Mocks>()) {}
  void SetUp() override {}
  void TearDown() override {
    /* Clean any leftover subscription state across test cases. */
    notification_status_monitor_message_cb_unset();
  }
};

GDBusConnection *__bus_fail(GBusType, GCancellable *, GError **err) {
  if (err)
    *err = g_error_new_literal(G_IO_ERROR, G_IO_ERROR_FAILED, "fake bus fail");
  return nullptr;
}

void __status_msg_cb(const char *, void *) {}

}  // namespace

using ::testing::_;
using ::testing::Invoke;
using ::testing::Return;
using ::testing::AnyNumber;

TEST_F(NotificationStatusInternalTest, MessagePost_NullMessage_InvalidParam) {
  /* Early NULL check happens BEFORE g_bus_get_sync. We deliberately
   * do NOT set a g_bus_get_sync expectation — if the early-out is
   * removed in the future, NiceMock returns a default zero
   * GDBusConnection* which would then NULL-deref the next stage. */
  ASSERT_EQ(notification_status_message_post(nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationStatusInternalTest, MessagePost_BusGetFails_ReturnsFromDbus) {
  EXPECT_CALL(GetMock<GlibMock>(), g_bus_get_sync(_, _, _))
      .WillRepeatedly(Invoke(__bus_fail));

  int ret = notification_status_message_post("hello");
  EXPECT_EQ(ret, NOTIFICATION_ERROR_FROM_DBUS);
}

TEST_F(NotificationStatusInternalTest,
       MessagePost_EmitSignalSucceeds_ReturnsNone) {
  /* Provide a non-NULL bus + emit_signal/flush both succeed. The
   * function should return NONE and unref the bus exactly once. */
  /* Each test creates its own GObject; production unrefs it once
   * so we must NOT share across cases. */
  GDBusConnection *fake_conn = reinterpret_cast<GDBusConnection *>(
      g_object_new(G_TYPE_OBJECT, nullptr));

  EXPECT_CALL(GetMock<GlibMock>(), g_bus_get_sync(_, _, _))
      .WillRepeatedly(Return(fake_conn));
  EXPECT_CALL(GetMock<GlibMock>(),
              g_dbus_connection_emit_signal(_, _, _, _, _, _, _))
      .WillRepeatedly(Return(TRUE));
  EXPECT_CALL(GetMock<GlibMock>(),
              g_dbus_connection_flush_sync(_, _, _))
      .WillRepeatedly(Return(TRUE));

  int ret = notification_status_message_post("hello");
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
}

TEST_F(NotificationStatusInternalTest,
       MessagePost_EmitSignalFails_ReturnsFromDbus) {
  /* Each test creates its own GObject; production unrefs it once
   * so we must NOT share across cases. */
  GDBusConnection *fake_conn = reinterpret_cast<GDBusConnection *>(
      g_object_new(G_TYPE_OBJECT, nullptr));

  EXPECT_CALL(GetMock<GlibMock>(), g_bus_get_sync(_, _, _))
      .WillRepeatedly(Return(fake_conn));
  EXPECT_CALL(GetMock<GlibMock>(),
              g_dbus_connection_emit_signal(_, _, _, _, _, _, _))
      .WillRepeatedly(::testing::DoAll(
          ::testing::WithArg<6>(
              ::testing::Invoke([](GError **err) {
                if (err)
                  *err = g_error_new_literal(G_IO_ERROR,
                                             G_IO_ERROR_FAILED,
                                             "fake emit fail");
              })),
          Return(FALSE)));

  int ret = notification_status_message_post("hello");
  EXPECT_EQ(ret, NOTIFICATION_ERROR_FROM_DBUS);
}

TEST_F(NotificationStatusInternalTest, MonitorCbSet_NullCallback_InvalidParam) {
  ASSERT_EQ(notification_status_monitor_message_cb_set(nullptr, nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationStatusInternalTest, MonitorCbSet_BusGetFails_FromDbus) {
  EXPECT_CALL(GetMock<GlibMock>(), g_bus_get_sync(_, _, _))
      .WillRepeatedly(Invoke(__bus_fail));

  int ret = notification_status_monitor_message_cb_set(__status_msg_cb,
                                                       nullptr);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_FROM_DBUS);
}

TEST_F(NotificationStatusInternalTest, MonitorCbUnset_FreshState_ReturnsNone) {
  /* Calling unset before set must be a no-op safe path. */
  ASSERT_EQ(notification_status_monitor_message_cb_unset(),
            NOTIFICATION_ERROR_NONE);
  /* And it must remain idempotent on repeat. */
  ASSERT_EQ(notification_status_monitor_message_cb_unset(),
            NOTIFICATION_ERROR_NONE);
}

TEST_F(NotificationStatusInternalTest,
       MonitorCbSet_SubscribeSucceeds_ReturnsNone) {
  /* Each test creates its own GObject; production unrefs it once
   * so we must NOT share across cases. */
  GDBusConnection *fake_conn = reinterpret_cast<GDBusConnection *>(
      g_object_new(G_TYPE_OBJECT, nullptr));

  EXPECT_CALL(GetMock<GlibMock>(), g_bus_get_sync(_, _, _))
      .WillRepeatedly(Return(fake_conn));
  EXPECT_CALL(GetMock<GlibMock>(),
              g_dbus_connection_signal_subscribe(_, _, _, _, _, _, _, _, _, _))
      .WillRepeatedly(Return(7u));  /* nonzero subscription id */

  int ret = notification_status_monitor_message_cb_set(__status_msg_cb,
                                                       nullptr);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  /* Now unset must release the subscription cleanly. */
  EXPECT_EQ(notification_status_monitor_message_cb_unset(),
            NOTIFICATION_ERROR_NONE);
}
