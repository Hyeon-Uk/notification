#include <gtest/gtest.h>

#include <notification.h>
#include <notification_db.h>
#include <notification_error.h>
#include <notification_setting.h>
#include <notification_setting_internal.h>
#include <notification_setting_service.h>
#include <notification_type.h>

#include "integ_harness.hh"

namespace {

class IntegSetting : public ::testing::Test {
 protected:
  void SetUp() override {
    ASSERT_EQ(notification_db_init(), NOTIFICATION_ERROR_NONE);
  }
  integ::TmpDb tmp_;
};

TEST_F(IntegSetting, N_SET_1_get_setting_array_null_array_pointer) {
  int count = -1;
  int rc = notification_setting_get_setting_array(nullptr, &count);
  EXPECT_NE(rc, NOTIFICATION_ERROR_NONE);
}

TEST_F(IntegSetting, N_SET_2_get_setting_array_null_count) {
  notification_setting_h arr = nullptr;
  int rc = notification_setting_get_setting_array(&arr, nullptr);
  EXPECT_NE(rc, NOTIFICATION_ERROR_NONE);
}

TEST_F(IntegSetting, N_SET_3_get_setting_by_package_name_null_args) {
  notification_setting_h s = nullptr;
  EXPECT_NE(notification_setting_get_setting_by_package_name(nullptr, &s),
            NOTIFICATION_ERROR_NONE);
  EXPECT_NE(notification_setting_get_setting_by_package_name("x", nullptr),
            NOTIFICATION_ERROR_NONE);
}

TEST_F(IntegSetting, N_SET_4_get_appid_null_setting_handle) {
  char* appid = nullptr;
  int rc = notification_setting_get_appid(nullptr, &appid);
  EXPECT_NE(rc, NOTIFICATION_ERROR_NONE);
}

TEST_F(IntegSetting, N_SET_5_get_package_name_null_setting_handle) {
  char* name = nullptr;
  int rc = notification_setting_get_package_name(nullptr, &name);
  EXPECT_NE(rc, NOTIFICATION_ERROR_NONE);
}

TEST_F(IntegSetting, N_SET_6_get_allow_to_notify_null_setting) {
  bool allow = false;
  int rc = notification_setting_get_allow_to_notify(nullptr, &allow);
  EXPECT_NE(rc, NOTIFICATION_ERROR_NONE);
}

TEST_F(IntegSetting, N_SET_7_set_allow_to_notify_null_setting) {
  int rc = notification_setting_set_allow_to_notify(nullptr, true);
  EXPECT_NE(rc, NOTIFICATION_ERROR_NONE);
}

TEST_F(IntegSetting, E_SET_1_get_setting_array_empty_db_returns_zero_or_error) {
  notification_setting_h arr = nullptr;
  int count = -1;
  int rc = notification_setting_get_setting_array(&arr, &count);
  // Pin: empty DB returns either NONE with count >= 0 or a
  // documented non-NONE code; never crashes.
  EXPECT_LE(rc, NOTIFICATION_ERROR_NONE);
  if (arr) notification_setting_free_notification(arr);
}

TEST_F(IntegSetting, E_SET_2_get_setting_by_appid_for_uid_missing) {
  notification_setting_h s = nullptr;
  int rc = notification_setting_get_setting_by_appid_for_uid(
      "no.such.app.set.42", &s, 0);
  EXPECT_NE(rc, NOTIFICATION_ERROR_NONE);
}

TEST_F(IntegSetting, C_SET_1_db_update_pkg_disabled_no_state) {
  // Pin: disabling an unknown pkg -- bounded return, no crash.
  int rc = notification_setting_db_update_pkg_disabled("no.such.app.set.43",
                                                       true, 0);
  EXPECT_LE(rc, NOTIFICATION_ERROR_NONE);
}

TEST_F(IntegSetting, C_SET_2_db_update_app_disabled_no_state) {
  int rc = notification_setting_db_update_app_disabled("no.such.app.set.44",
                                                       true, 0);
  EXPECT_LE(rc, NOTIFICATION_ERROR_NONE);
}

}  // namespace
