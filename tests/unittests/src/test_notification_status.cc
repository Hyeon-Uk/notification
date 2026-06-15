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

#include "mock/test_fixture.hh"
#include "mock/glib_mock.hh"

#include "notification/include/notification.h"
#include "notification/include/notification_status.h"
#include "notification/include/notification_status_internal.h"

namespace {

class Mocks : virtual public ::testing::NiceMock<GlibMock> {};

}  // namespace

using ::testing::_;
using ::testing::Invoke;
using ::testing::Return;

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
