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

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <gio/gio.h>
#include <glib.h>

#include "notification/include/notification_error.h"

extern "C" {
GQuark notification_error_quark(void);
}

namespace {

struct ErrorCodeMapping {
  notification_error_e code;
  const char* dbus_name;
};

const ErrorCodeMapping kDbusMappings[] = {
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

class NotificationErrorTest : public ::testing::Test {};

TEST_F(NotificationErrorTest, QuarkIsNonZero) {
  GQuark q = notification_error_quark();
  EXPECT_NE(q, static_cast<GQuark>(0));
}

TEST_F(NotificationErrorTest, QuarkIsIdempotent) {
  GQuark first = notification_error_quark();
  GQuark second = notification_error_quark();
  GQuark third = notification_error_quark();
  EXPECT_EQ(first, second);
  EXPECT_EQ(second, third);
}

TEST_F(NotificationErrorTest, QuarkResolvesToDocumentedName) {
  GQuark q = notification_error_quark();
  const char* name = g_quark_to_string(q);
  ASSERT_NE(name, nullptr);
  EXPECT_STREQ(name, "notification-error-quark");
}

TEST_F(NotificationErrorTest, RegistersAllElevenDbusErrorEntries) {
  GQuark domain = notification_error_quark();
  for (const auto& mapping : kDbusMappings) {
    GError* err = g_error_new_literal(domain, mapping.code, "test");
    ASSERT_NE(err, nullptr) << "code=" << mapping.code;

    char* dbus_name = g_dbus_error_encode_gerror(err);
    ASSERT_NE(dbus_name, nullptr) << "code=" << mapping.code;
    EXPECT_STREQ(dbus_name, mapping.dbus_name)
        << "code=" << mapping.code;

    g_free(dbus_name);
    g_error_free(err);
  }
}

TEST_F(NotificationErrorTest, RoundTripPreservesEachDbusCode) {
  GQuark domain = notification_error_quark();
  for (const auto& mapping : kDbusMappings) {
    GError* original = g_error_new_literal(domain, mapping.code, "boom");
    char* encoded = g_dbus_error_encode_gerror(original);
    ASSERT_NE(encoded, nullptr);

    GError* decoded = g_dbus_error_new_for_dbus_error(encoded, "boom");
    ASSERT_NE(decoded, nullptr);
    EXPECT_EQ(decoded->domain, domain) << "code=" << mapping.code;
    EXPECT_EQ(decoded->code, static_cast<int>(mapping.code))
        << "code=" << mapping.code;

    g_free(encoded);
    g_error_free(decoded);
    g_error_free(original);
  }
}

TEST_F(NotificationErrorTest, NoneCodeIsNotRegisteredInDbusTable) {
  GQuark domain = notification_error_quark();
  GError* err = g_error_new_literal(domain, NOTIFICATION_ERROR_NONE, "ok");
  ASSERT_NE(err, nullptr);

  char* dbus_name = g_dbus_error_encode_gerror(err);
  ASSERT_NE(dbus_name, nullptr);
  // NONE has no entry in dbus_error_entries[]; GIO falls back to a
  // generic "org.gtk.GDBus.UnmappedGError.Quark..." encoding rather
  // than to one of the documented names.
  EXPECT_THAT(dbus_name, ::testing::HasSubstr("UnmappedGError"));
  EXPECT_THAT(dbus_name,
              ::testing::Not(::testing::HasSubstr(
                  "org.freedesktop.Notification.Error")));

  g_free(dbus_name);
  g_error_free(err);
}

TEST_F(NotificationErrorTest, UnknownCodeFallsBackToGenericDbusName) {
  GQuark domain = notification_error_quark();
  // Pick a code outside both the TIZEN_ERROR_NOTIFICATION range AND
  // the standard tizen.h slots used by the upper rows of the enum.
  const int kUnknown = 0x7FFFFFFE;
  GError* err = g_error_new_literal(domain, kUnknown, "wat");
  ASSERT_NE(err, nullptr);

  char* dbus_name = g_dbus_error_encode_gerror(err);
  ASSERT_NE(dbus_name, nullptr);
  EXPECT_THAT(dbus_name, ::testing::HasSubstr("UnmappedGError"));

  g_free(dbus_name);
  g_error_free(err);
}

TEST_F(NotificationErrorTest, NegativeCodeFallsBackToGenericDbusName) {
  GQuark domain = notification_error_quark();
  GError* err = g_error_new_literal(domain, -1, "neg");
  ASSERT_NE(err, nullptr);

  char* dbus_name = g_dbus_error_encode_gerror(err);
  ASSERT_NE(dbus_name, nullptr);
  EXPECT_THAT(dbus_name, ::testing::HasSubstr("UnmappedGError"));

  g_free(dbus_name);
  g_error_free(err);
}
