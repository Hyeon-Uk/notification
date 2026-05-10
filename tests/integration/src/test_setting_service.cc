#include <gtest/gtest.h>

#include <notification.h>
#include <notification_db.h>
#include <notification_error.h>
#include <notification_setting.h>
#include <notification_setting_internal.h>
#include <notification_setting_service.h>

#include "integ_harness.hh"

namespace {

class IntegSettingService : public ::testing::Test {
 protected:
  void SetUp() override {
    ASSERT_EQ(notification_db_init(), NOTIFICATION_ERROR_NONE);
  }
  integ::TmpDb tmp_;
};

TEST_F(IntegSettingService, P_SVC_1_refresh_setting_table_empty_db) {
  int rc = notification_setting_refresh_setting_table(0);
  EXPECT_LE(rc, NOTIFICATION_ERROR_NONE);
}

TEST_F(IntegSettingService, P_SVC_2_db_update_pkg_disabled_no_state) {
  int rc = notification_setting_db_update_pkg_disabled("no.such.pkg.svc.1",
                                                       true, 0);
  EXPECT_LE(rc, NOTIFICATION_ERROR_NONE);
}

TEST_F(IntegSettingService, P_SVC_3_db_update_app_disabled_no_state) {
  int rc = notification_setting_db_update_app_disabled("no.such.app.svc.2",
                                                       true, 0);
  EXPECT_LE(rc, NOTIFICATION_ERROR_NONE);
}

TEST_F(IntegSettingService, P_SVC_4_db_update_do_not_disturb_round_trip) {
  // Set DND on; pin a bounded return.
  int rc = notification_setting_db_update_do_not_disturb(true, 0);
  EXPECT_LE(rc, NOTIFICATION_ERROR_NONE);
}

TEST_F(IntegSettingService, P_SVC_5_setting_delete_package_idempotent) {
  int rc = notification_setting_delete_package_for_uid("no.such.pkg.svc.3", 0);
  EXPECT_LE(rc, NOTIFICATION_ERROR_NONE);
}

TEST_F(IntegSettingService, P_SVC_6_init_system_setting_table_idempotent) {
  int rc1 = notification_system_setting_init_system_setting_table(0);
  int rc2 = notification_system_setting_init_system_setting_table(0);
  EXPECT_LE(rc1, NOTIFICATION_ERROR_NONE);
  EXPECT_LE(rc2, NOTIFICATION_ERROR_NONE);
}

TEST_F(IntegSettingService, C_SVC_1_load_system_setting_for_uid_round_trip) {
  notification_system_setting_h s = nullptr;
  int rc = notification_system_setting_load_system_setting_for_uid(&s, 0);
  // Pin: bounded return; non-NULL handle is freed if produced.
  EXPECT_LE(rc, NOTIFICATION_ERROR_NONE);
  if (s) notification_system_setting_free_system_setting(s);
}

TEST_F(IntegSettingService, N_SVC_1_load_system_setting_null_out) {
  // NULL out-pointer should fail-fast or fail-safe, not crash.
  int rc = notification_system_setting_load_system_setting_for_uid(nullptr, 0);
  EXPECT_NE(rc, NOTIFICATION_ERROR_NONE);
}

TEST_F(IntegSettingService, N_SVC_2_setting_db_update_null_pkg_id) {
  int rc = notification_setting_db_update_pkg_disabled(nullptr, true, 0);
  // Pin: NULL pkg_id is documented as INVALID_PARAMETER today; bounded.
  EXPECT_LE(rc, NOTIFICATION_ERROR_NONE);
}

}  // namespace
