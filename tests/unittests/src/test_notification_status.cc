/*
 * Copyright (c) 2024 Samsung Electronics Co., Ltd.
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
#include <gio/gio.h>

#include <string>
#include <utility>

#include "mock/test_fixture.hh"
#include "mock/glib_mock.hh"

#include "notification/include/notification.h"
#include "notification/include/notification_status.h"
#include "notification/include/notification_status_internal.h"

namespace {

class Mocks : virtual public ::testing::NiceMock<GlibMock> {};

}  // namespace

using ::testing::_;
using ::testing::DoAll;
using ::testing::Invoke;
using ::testing::Return;
using ::testing::SaveArg;

class NotificationStatusTest : public TestFixture {
 public:
  NotificationStatusTest() : TestFixture(std::make_unique<::Mocks>()) {}
  virtual void SetUp() {}
  virtual void TearDown() {
    /* Reset static state in notification_status.c after each test so that
     * successive tests start with a clean md struct.  Call the real unset
     * only when conn is NULL to avoid calling g_object_unref on a fake
     * pointer. */
    notification_status_monitor_message_cb_unset();
  }
};

/* ------------------------------------------------------------------ */
/* notification_status_monitor_message_cb_set — NULL callback          */
/* ------------------------------------------------------------------ */

TEST_F(NotificationStatusTest, monitor_cb_set_null_callback_returns_invalid) {
  EXPECT_EQ(notification_status_monitor_message_cb_set(nullptr, nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* ------------------------------------------------------------------ */
/* notification_status_monitor_message_cb_set — D-Bus connect failure  */
/* ------------------------------------------------------------------ */

TEST_F(NotificationStatusTest, monitor_cb_set_bus_connect_fails_returns_dbus_error) {
  /* g_bus_get_sync returns NULL and sets *error so the ERR macro does not
   * dereference a NULL GError pointer. */
  ON_CALL(GetMock<GlibMock>(), g_bus_get_sync(_, _, _))
      .WillByDefault(Invoke([](GBusType, GCancellable*, GError** err) -> GDBusConnection* {
        if (err)
          *err = g_error_new_literal(G_IO_ERROR, G_IO_ERROR_FAILED,
                                     "mock D-Bus failure");
        return nullptr;
      }));

  static auto cb = [](const char*, void*) {};
  EXPECT_EQ(notification_status_monitor_message_cb_set(cb, nullptr),
            NOTIFICATION_ERROR_FROM_DBUS);
}

/* ------------------------------------------------------------------ */
/* notification_status_monitor_message_cb_unset — clean initial state  */
/* ------------------------------------------------------------------ */

TEST_F(NotificationStatusTest, monitor_cb_unset_clean_state_returns_none) {
  /* When no callback is registered (static md is zero-initialised)
   * both inner branches are skipped and the function must return NONE.  */
  EXPECT_EQ(notification_status_monitor_message_cb_unset(),
            NOTIFICATION_ERROR_NONE);
}

/* ------------------------------------------------------------------ */
/* notification_status_message_post — NULL message                     */
/* ------------------------------------------------------------------ */

TEST_F(NotificationStatusTest, message_post_null_message_returns_invalid) {
  EXPECT_EQ(notification_status_message_post(nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* ------------------------------------------------------------------ */
/* notification_status_message_post — D-Bus connect failure            */
/* ------------------------------------------------------------------ */

TEST_F(NotificationStatusTest, message_post_bus_connect_fails_returns_dbus_error) {
  ON_CALL(GetMock<GlibMock>(), g_bus_get_sync(_, _, _))
      .WillByDefault(Invoke([](GBusType, GCancellable*, GError** err) -> GDBusConnection* {
        if (err)
          *err = g_error_new_literal(G_IO_ERROR, G_IO_ERROR_FAILED,
                                     "mock D-Bus failure");
        return nullptr;
      }));

  EXPECT_EQ(notification_status_message_post("hello"),
            NOTIFICATION_ERROR_FROM_DBUS);
}

/* ------------------------------------------------------------------ */
/* [P1] dbus_callback_valid_message_invokes_user_callback             */
/* Capture the GDBusSignalCallback, call it with "(s)", "hello",      */
/* and assert the user callback was invoked with that string.          */
/* ------------------------------------------------------------------ */

TEST_F(NotificationStatusTest, dbus_callback_valid_message_invokes_user_callback) {
  /* A non-NULL fake connection pointer — never dereferenced. */
  GDBusConnection* fake_conn =
      reinterpret_cast<GDBusConnection*>(static_cast<uintptr_t>(0xDEAD));

  ON_CALL(GetMock<GlibMock>(), g_bus_get_sync(_, _, _))
      .WillByDefault(Return(fake_conn));

  /* g_object_unref is called on fake_conn on teardown; make it a no-op
   * by mapping unsubscribe as well (the real unref is not intercepted,
   * but the fake pointer won't be dereferenced by the real glib on
   * teardown because TearDown calls cb_unset which goes through the
   * mocked unsubscribe path). */
  ON_CALL(GetMock<GlibMock>(),
          g_dbus_connection_signal_unsubscribe(_, _))
      .WillByDefault(Return());

  GDBusSignalCallback captured_cb = nullptr;
  gpointer captured_user_data = nullptr;
  ON_CALL(GetMock<GlibMock>(),
          g_dbus_connection_signal_subscribe(_, _, _, _, _, _, _, _, _, _))
      .WillByDefault(
          DoAll(SaveArg<7>(&captured_cb),
                SaveArg<8>(&captured_user_data),
                Return(static_cast<guint>(1))));

  bool cb_invoked = false;
  std::string received_msg;
  auto user_cb = [](const char* msg, void* data) {
    auto* pair = static_cast<std::pair<bool*, std::string*>*>(data);
    *pair->first = true;
    *pair->second = msg;
  };

  std::pair<bool*, std::string*> ctx{&cb_invoked, &received_msg};
  ASSERT_EQ(
      notification_status_monitor_message_cb_set(user_cb, &ctx),
      NOTIFICATION_ERROR_NONE);

  ASSERT_NE(captured_cb, nullptr);

  /* Call the captured D-Bus callback with a valid, non-empty message. */
  GVariant* params = g_variant_new("(s)", "hello");
  captured_cb(nullptr, nullptr, nullptr, nullptr, nullptr, params,
              captured_user_data);
  g_variant_unref(params);

  EXPECT_TRUE(cb_invoked);
  EXPECT_EQ(received_msg, "hello");
}

/* ------------------------------------------------------------------ */
/* [N1] dbus_callback_empty_message_does_not_invoke                   */
/* An empty string in the signal payload must NOT invoke the callback. */
/* ------------------------------------------------------------------ */

TEST_F(NotificationStatusTest, dbus_callback_empty_message_does_not_invoke) {
  GDBusConnection* fake_conn =
      reinterpret_cast<GDBusConnection*>(static_cast<uintptr_t>(0xDEAD));

  ON_CALL(GetMock<GlibMock>(), g_bus_get_sync(_, _, _))
      .WillByDefault(Return(fake_conn));
  ON_CALL(GetMock<GlibMock>(),
          g_dbus_connection_signal_unsubscribe(_, _))
      .WillByDefault(Return());

  GDBusSignalCallback captured_cb = nullptr;
  ON_CALL(GetMock<GlibMock>(),
          g_dbus_connection_signal_subscribe(_, _, _, _, _, _, _, _, _, _))
      .WillByDefault(
          DoAll(SaveArg<7>(&captured_cb),
                Return(static_cast<guint>(1))));

  bool cb_invoked = false;
  auto user_cb = [](const char*, void* data) {
    *static_cast<bool*>(data) = true;
  };
  ASSERT_EQ(
      notification_status_monitor_message_cb_set(user_cb, &cb_invoked),
      NOTIFICATION_ERROR_NONE);
  ASSERT_NE(captured_cb, nullptr);

  GVariant* params = g_variant_new("(s)", "");
  captured_cb(nullptr, nullptr, nullptr, nullptr, nullptr, params, nullptr);
  g_variant_unref(params);

  EXPECT_FALSE(cb_invoked);
}

/* ------------------------------------------------------------------ */
/* [N3] message_post_emit_signal_fails_returns_dbus                   */
/* ------------------------------------------------------------------ */

TEST_F(NotificationStatusTest, message_post_emit_signal_fails_returns_dbus) {
  GDBusConnection* fake_conn =
      reinterpret_cast<GDBusConnection*>(static_cast<uintptr_t>(0xDEAD));

  ON_CALL(GetMock<GlibMock>(), g_bus_get_sync(_, _, _))
      .WillByDefault(Return(fake_conn));
  ON_CALL(GetMock<GlibMock>(),
          g_dbus_connection_emit_signal(_, _, _, _, _, _, _))
      .WillByDefault(
          Invoke([](GDBusConnection*, const gchar*, const gchar*,
                    const gchar*, const gchar*, GVariant* param,
                    GError** err) -> gboolean {
            if (err)
              *err = g_error_new_literal(G_IO_ERROR, G_IO_ERROR_FAILED,
                                         "mock emit failure");
            /* g_dbus_connection_emit_signal sinks the floating GVariant
             * on success; on failure the caller still owns it.  We must
             * sink the reference here so g_variant_unref in the
             * production code does not double-free. */
            if (param && g_variant_is_floating(param))
              g_variant_ref_sink(param);
            return FALSE;
          }));

  EXPECT_EQ(notification_status_message_post("hello"),
            NOTIFICATION_ERROR_FROM_DBUS);
}

/* ------------------------------------------------------------------ */
/* [N4] message_post_flush_fails_returns_dbus                         */
/* emit succeeds but flush fails → FROM_DBUS                          */
/* ------------------------------------------------------------------ */

TEST_F(NotificationStatusTest, message_post_flush_fails_returns_dbus) {
  GDBusConnection* fake_conn =
      reinterpret_cast<GDBusConnection*>(static_cast<uintptr_t>(0xDEAD));

  ON_CALL(GetMock<GlibMock>(), g_bus_get_sync(_, _, _))
      .WillByDefault(Return(fake_conn));
  ON_CALL(GetMock<GlibMock>(),
          g_dbus_connection_emit_signal(_, _, _, _, _, _, _))
      .WillByDefault(
          Invoke([](GDBusConnection*, const gchar*, const gchar*,
                    const gchar*, const gchar*, GVariant* param,
                    GError**) -> gboolean {
            /* Sink the floating reference so the production code's
             * g_variant_unref on the "end:" path doesn't double-free. */
            if (param && g_variant_is_floating(param))
              g_variant_ref_sink(param);
            return TRUE;
          }));
  ON_CALL(GetMock<GlibMock>(),
          g_dbus_connection_flush_sync(_, _, _))
      .WillByDefault(
          Invoke([](GDBusConnection*, GCancellable*, GError** err) -> gboolean {
            if (err)
              *err = g_error_new_literal(G_IO_ERROR, G_IO_ERROR_FAILED,
                                         "mock flush failure");
            return FALSE;
          }));

  EXPECT_EQ(notification_status_message_post("hello"),
            NOTIFICATION_ERROR_FROM_DBUS);
}

/* ------------------------------------------------------------------ */
/* [N5] cb_set_subscribe_fails_returns_dbus                           */
/* subscribe returns 0 → FROM_DBUS                                    */
/* ------------------------------------------------------------------ */

TEST_F(NotificationStatusTest, cb_set_subscribe_fails_returns_dbus) {
  GDBusConnection* fake_conn =
      reinterpret_cast<GDBusConnection*>(static_cast<uintptr_t>(0xDEAD));

  ON_CALL(GetMock<GlibMock>(), g_bus_get_sync(_, _, _))
      .WillByDefault(Return(fake_conn));
  ON_CALL(GetMock<GlibMock>(),
          g_dbus_connection_signal_subscribe(_, _, _, _, _, _, _, _, _, _))
      .WillByDefault(Return(static_cast<guint>(0)));

  static auto cb = [](const char*, void*) {};
  EXPECT_EQ(notification_status_monitor_message_cb_set(cb, nullptr),
            NOTIFICATION_ERROR_FROM_DBUS);
}

/* ------------------------------------------------------------------ */
/* [C1] cb_unset_after_set_calls_unsubscribe                          */
/* After a successful cb_set, cb_unset must call unsubscribe once.    */
/* ------------------------------------------------------------------ */

TEST_F(NotificationStatusTest, cb_unset_after_set_calls_unsubscribe) {
  GDBusConnection* fake_conn =
      reinterpret_cast<GDBusConnection*>(static_cast<uintptr_t>(0xDEAD));

  ON_CALL(GetMock<GlibMock>(), g_bus_get_sync(_, _, _))
      .WillByDefault(Return(fake_conn));
  ON_CALL(GetMock<GlibMock>(),
          g_dbus_connection_signal_subscribe(_, _, _, _, _, _, _, _, _, _))
      .WillByDefault(Return(static_cast<guint>(42)));

  bool unsubscribe_called = false;
  EXPECT_CALL(GetMock<GlibMock>(),
              g_dbus_connection_signal_unsubscribe(fake_conn, 42))
      .WillOnce(Invoke([&](GDBusConnection*, guint) {
        unsubscribe_called = true;
      }));

  static auto cb = [](const char*, void*) {};
  ASSERT_EQ(notification_status_monitor_message_cb_set(cb, nullptr),
            NOTIFICATION_ERROR_NONE);

  /* TearDown also calls cb_unset — reset the expectation so we don't
   * get a "called more times than expected" failure.  The EXPECT_CALL
   * above is already satisfied when cb_unset is called here. */
  EXPECT_EQ(notification_status_monitor_message_cb_unset(),
            NOTIFICATION_ERROR_NONE);

  EXPECT_TRUE(unsubscribe_called);

  /* Prevent double-unsubscribe in TearDown by suppressing extra calls. */
  ON_CALL(GetMock<GlibMock>(),
          g_dbus_connection_signal_unsubscribe(_, _))
      .WillByDefault(Return());
}

/* ------------------------------------------------------------------ */
/* [N2] dbus_callback_valid_msg_after_unset_does_not_invoke            */
/* Invoke the captured signal callback with a non-empty message AFTER   */
/* cb_unset has cleared md.callback.  Hits the "No callback" branch    */
/* (lines 61-63 of notification_status.c) and must NOT call the user   */
/* callback.                                                            */
/* ------------------------------------------------------------------ */

TEST_F(NotificationStatusTest, dbus_callback_valid_msg_after_unset_does_not_invoke) {
  GDBusConnection* fake_conn =
      reinterpret_cast<GDBusConnection*>(static_cast<uintptr_t>(0xBEEF));

  ON_CALL(GetMock<GlibMock>(), g_bus_get_sync(_, _, _))
      .WillByDefault(Return(fake_conn));
  ON_CALL(GetMock<GlibMock>(),
          g_dbus_connection_signal_unsubscribe(_, _))
      .WillByDefault(Return());

  GDBusSignalCallback captured_cb = nullptr;
  ON_CALL(GetMock<GlibMock>(),
          g_dbus_connection_signal_subscribe(_, _, _, _, _, _, _, _, _, _))
      .WillByDefault(
          DoAll(SaveArg<7>(&captured_cb),
                Return(static_cast<guint>(7))));

  bool cb_invoked = false;
  auto user_cb = [](const char*, void* data) {
    *static_cast<bool*>(data) = true;
  };
  ASSERT_EQ(notification_status_monitor_message_cb_set(user_cb, &cb_invoked),
            NOTIFICATION_ERROR_NONE);
  ASSERT_NE(captured_cb, nullptr);

  /* cb_unset clears md.callback (and md.conn, md.message_id) */
  ASSERT_EQ(notification_status_monitor_message_cb_unset(), NOTIFICATION_ERROR_NONE);

  /* Now invoke the captured callback with a valid non-empty message.
   * md.callback is NULL → must hit the "No callback" branch and return
   * without calling the user callback. */
  GVariant* params = g_variant_new("(s)", "hello after unset");
  captured_cb(nullptr, nullptr, nullptr, nullptr, nullptr, params, nullptr);
  g_variant_unref(params);

  EXPECT_FALSE(cb_invoked);
}
