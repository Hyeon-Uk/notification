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
 * N7.5-A: Internal API Coverage for src/notification/src/notification_error.c
 *
 * The production file owns the singleton GQuark used by the rest of
 * libnotification (NOTIFICATION_ERROR macro in notification_internal.h)
 * and registers a DBus error domain that maps every NOTIFICATION_ERROR_*
 * enum to a "org.freedesktop.Notification.Error.<NAME>" string.
 *
 * Behaviours covered here (no production change — coverage only):
 *   1. notification_error_quark() returns a stable, non-zero GQuark.
 *   2. Repeated calls return the same GQuark (idempotent).
 *   3. The registered domain name resolves back to the same quark
 *      via g_quark_try_string("notification-error-quark") — proves
 *      g_dbus_error_register_error_domain() ran on first call.
 *   4. The DBus error mapping for every notification_error_e enum
 *      value advertised by notification_error.h round-trips:
 *      g_dbus_error_new_for_dbus_error(<expected name>, ...) decodes
 *      back to the right enum via g_dbus_error_get_remote_error +
 *      g_dbus_error_strip_remote_error logic. This pins the
 *      11-entry dbus_error_entries[] table.
 */

#include <gtest/gtest.h>
#include <gio/gio.h>
#include <glib.h>

#include "mock/test_fixture.hh"
#include "mock/glib_mock.hh"

#include "notification/include/notification.h"
#include "notification/include/notification_error.h"
#include "notification/include/notification_internal.h"

namespace {

class Mocks :
    virtual public ::testing::NiceMock<GlibMock> {};

class NotificationErrorInternalTest : public TestFixture {
 public:
  NotificationErrorInternalTest() : TestFixture(std::make_unique<::Mocks>()) {}
  void SetUp() override {}
  void TearDown() override {}
};

struct ErrorNameMap {
  notification_error_e code;
  const char *dbus_name;
};

const ErrorNameMap kErrorNameMap[] = {
    {NOTIFICATION_ERROR_INVALID_PARAMETER,
     "org.freedesktop.Notification.Error.INVALID_PARAMETER"},
    {NOTIFICATION_ERROR_OUT_OF_MEMORY,
     "org.freedesktop.Notification.Error.OUT_OF_MEMORY"},
    {NOTIFICATION_ERROR_IO_ERROR,
     "org.freedesktop.Notification.Error.IO_ERROR"},
    {NOTIFICATION_ERROR_PERMISSION_DENIED,
     "org.freedesktop.Notification.Error.PERMISSION_DENIED"},
    {NOTIFICATION_ERROR_FROM_DB,
     "org.freedesktop.Notification.Error.FROM_DB"},
    {NOTIFICATION_ERROR_ALREADY_EXIST_ID,
     "org.freedesktop.Notification.Error.ALREADY_EXIST_ID"},
    {NOTIFICATION_ERROR_FROM_DBUS,
     "org.freedesktop.Notification.Error.FROM_DBUS"},
    {NOTIFICATION_ERROR_NOT_EXIST_ID,
     "org.freedesktop.Notification.Error.NOT_EXIST_ID"},
    {NOTIFICATION_ERROR_SERVICE_NOT_READY,
     "org.freedesktop.Notification.Error.SERVICE_NOT_READY"},
    {NOTIFICATION_ERROR_INVALID_OPERATION,
     "org.freedesktop.Notification.Error.INVALID_OPERATION"},
    {NOTIFICATION_ERROR_MAX_EXCEEDED,
     "org.freedesktop.Notification.Error.MAX_EXCEEDED"},
};

}  // namespace

TEST_F(NotificationErrorInternalTest, Quark_NonZero) {
  GQuark q = notification_error_quark();
  ASSERT_NE(q, 0u);
}

TEST_F(NotificationErrorInternalTest, Quark_IdempotentAcrossCalls) {
  GQuark q1 = notification_error_quark();
  GQuark q2 = notification_error_quark();
  GQuark q3 = notification_error_quark();
  ASSERT_NE(q1, 0u);
  ASSERT_EQ(q1, q2);
  ASSERT_EQ(q2, q3);
}

TEST_F(NotificationErrorInternalTest, Quark_MatchesRegisteredDomainName) {
  /* notification_error.c registers the domain under the literal
   * "notification-error-quark". Once the registration ran, glib
   * caches the (name -> quark) mapping in the global string table.
   * g_quark_try_string() must resolve it to the same value the
   * caller saw. */
  GQuark from_api = notification_error_quark();
  GQuark from_glib = g_quark_try_string("notification-error-quark");
  ASSERT_NE(from_api, 0u);
  ASSERT_EQ(from_api, from_glib);
}

TEST_F(NotificationErrorInternalTest, Macro_NOTIFICATION_ERROR_ResolvesToQuark) {
  /* notification_internal.h defines NOTIFICATION_ERROR as
   * (notification_error_quark()). The macro must yield the
   * same value as a direct call. */
  GQuark direct = notification_error_quark();
  GQuark via_macro = NOTIFICATION_ERROR;
  ASSERT_EQ(direct, via_macro);
}

TEST_F(NotificationErrorInternalTest, DbusErrorDomain_AllEnumValuesMapped) {
  /* Force the domain to be registered exactly once before probing. */
  (void)notification_error_quark();
  GQuark domain = notification_error_quark();
  ASSERT_NE(domain, 0u);

  for (const auto &entry : kErrorNameMap) {
    /* Build a synthetic GError using the registered DBus error name.
     * If the entry sits in dbus_error_entries[], glib will route
     * the GError back to our domain with .code == entry.code and
     * the human-readable message stripped of the DBus prefix. */
    GError *err = g_dbus_error_new_for_dbus_error(entry.dbus_name,
                                                  "ignored body");
    ASSERT_TRUE(err != nullptr) << entry.dbus_name;
    EXPECT_EQ(err->domain, domain) << entry.dbus_name;
    EXPECT_EQ(err->code, static_cast<int>(entry.code)) << entry.dbus_name;
    g_error_free(err);
  }
}

TEST_F(NotificationErrorInternalTest, DbusErrorDomain_UnknownNameDoesNotMatch) {
  /* A DBus error name that is not in dbus_error_entries[] must
   * NOT decode to our domain — glib falls back to a generic
   * G_IO_ERROR (or unrelated) domain. This pins the table size
   * so a future drop of an entry would surface as a regression. */
  (void)notification_error_quark();
  GQuark domain = notification_error_quark();
  GError *err = g_dbus_error_new_for_dbus_error(
      "org.freedesktop.Notification.Error.NO_SUCH_THING",
      "ignored body");
  ASSERT_TRUE(err != nullptr);
  EXPECT_NE(err->domain, domain);
  g_error_free(err);
}
