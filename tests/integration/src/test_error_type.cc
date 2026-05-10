#include <gio/gio.h>
#include <glib.h>
#include <gtest/gtest.h>

#include <notification.h>
#include <notification_error.h>
#include <notification_internal.h>
#include <notification_text_domain.h>
#include <notification_type.h>
#include <notification_type_internal.h>

#include <set>

namespace {

TEST(IntegErrType, P_ERRTYPE_1_quark_first_call_returns_nonzero) {
  GQuark q = notification_error_quark();
  EXPECT_NE(q, 0u);
}

TEST(IntegErrType, P_ERRTYPE_2_quark_is_idempotent) {
  GQuark q1 = notification_error_quark();
  GQuark q2 = notification_error_quark();
  EXPECT_EQ(q1, q2);
  EXPECT_NE(q1, 0u);
}

TEST(IntegErrType, C_ERRTYPE_1_error_enum_to_dbus_name_round_trip) {
  notification_error_quark();

  struct Entry {
    int code;
    const char* dbus_name;
  };
  static const Entry kEntries[] = {
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

  for (const auto& e : kEntries) {
    GError* err = g_error_new_literal(notification_error_quark(), e.code, "x");
    ASSERT_NE(err, nullptr) << "code=" << e.code;
    char* remote = g_dbus_error_encode_gerror(err);
    ASSERT_NE(remote, nullptr) << "code=" << e.code;
    EXPECT_STREQ(remote, e.dbus_name) << "code=" << e.code;
    g_free(remote);
    g_error_free(err);
  }
}

TEST(IntegErrType, C_ERRTYPE_2_enum_values_are_distinct_across_boundary) {
  std::set<int> error_values{
      NOTIFICATION_ERROR_NONE,
      NOTIFICATION_ERROR_INVALID_PARAMETER,
      NOTIFICATION_ERROR_OUT_OF_MEMORY,
      NOTIFICATION_ERROR_IO_ERROR,
      NOTIFICATION_ERROR_PERMISSION_DENIED,
      NOTIFICATION_ERROR_INVALID_OPERATION,
      NOTIFICATION_ERROR_FROM_DB,
      NOTIFICATION_ERROR_ALREADY_EXIST_ID,
      NOTIFICATION_ERROR_FROM_DBUS,
      NOTIFICATION_ERROR_NOT_EXIST_ID,
      NOTIFICATION_ERROR_SERVICE_NOT_READY,
      NOTIFICATION_ERROR_MAX_EXCEEDED,
  };
  EXPECT_EQ(error_values.size(), 12u);

  std::set<int> type_values{
      NOTIFICATION_TYPE_NONE,    NOTIFICATION_TYPE_NOTI,
      NOTIFICATION_TYPE_ONGOING,
  };
  EXPECT_GE(type_values.size(), 3u);

  std::set<int> image_values{
      NOTIFICATION_IMAGE_TYPE_ICON,
      NOTIFICATION_IMAGE_TYPE_ICON_FOR_INDICATOR,
      NOTIFICATION_IMAGE_TYPE_ICON_FOR_LOCK,
      NOTIFICATION_IMAGE_TYPE_THUMBNAIL,
      NOTIFICATION_IMAGE_TYPE_THUMBNAIL_FOR_LOCK,
      NOTIFICATION_IMAGE_TYPE_ICON_SUB,
      NOTIFICATION_IMAGE_TYPE_BACKGROUND,
  };
  EXPECT_GE(image_values.size(), 7u);

  std::set<int> text_values{
      NOTIFICATION_TEXT_TYPE_TITLE,
      NOTIFICATION_TEXT_TYPE_CONTENT,
      NOTIFICATION_TEXT_TYPE_EVENT_COUNT,
      NOTIFICATION_TEXT_TYPE_INFO_1,
      NOTIFICATION_TEXT_TYPE_INFO_SUB_1,
      NOTIFICATION_TEXT_TYPE_INFO_2,
      NOTIFICATION_TEXT_TYPE_INFO_SUB_2,
  };
  EXPECT_GE(text_values.size(), 7u);

  std::set<int> ly_values{NOTIFICATION_LY_NONE,
                          NOTIFICATION_LY_NOTI_EVENT_SINGLE,
                          NOTIFICATION_LY_NOTI_EVENT_MULTIPLE,
                          NOTIFICATION_LY_NOTI_THUMBNAIL,
                          NOTIFICATION_LY_ONGOING_EVENT,
                          NOTIFICATION_LY_ONGOING_PROGRESS};
  EXPECT_GE(ly_values.size(), 6u);

  std::set<int> internal_event_values{
      NOTIFICATION_EVENT_TYPE_HIDDEN_BY_USER,
      NOTIFICATION_EVENT_TYPE_HIDDEN_BY_TIMEOUT,
      NOTIFICATION_EVENT_TYPE_HIDDEN_BY_EXTERNAL,
      NOTIFICATION_EVENT_TYPE_PRESSED,
      NOTIFICATION_EVENT_TYPE_DELETED,
      NOTIFICATION_EVENT_TYPE_CHECK_BOX,
  };
  EXPECT_GE(internal_event_values.size(), 6u);

  std::set<int> ongoing_value_values{
      NOTIFICATION_ONGOING_VALUE_TYPE_PERCENT,
      NOTIFICATION_ONGOING_VALUE_TYPE_TIME,
  };
  EXPECT_GE(ongoing_value_values.size(), 2u);
}

}  // namespace
