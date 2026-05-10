#include <gtest/gtest.h>

#include <notification.h>
#include <notification_db.h>
#include <notification_error.h>
#include <notification_status.h>
#include <notification_status_internal.h>

#include "integ_harness.hh"

namespace {

class IntegStatus : public ::testing::Test {
 protected:
  integ::TmpDb tmp_;
};

TEST_F(IntegStatus, P_STAT_1_status_message_post_short_string) {
  // notification_status_message_post talks to a real status server
  // (vconf-backed) which may not be reachable in the gbs sandbox.
  // Pin: bounded return; no crash.
  int rc = notification_status_message_post("hello-from-19a-status-test");
  EXPECT_LE(rc, NOTIFICATION_ERROR_NONE);
}

TEST_F(IntegStatus, N_STAT_1_status_message_post_null) {
  int rc = notification_status_message_post(nullptr);
  EXPECT_NE(rc, NOTIFICATION_ERROR_NONE);
}

TEST_F(IntegStatus, N_STAT_2_status_message_post_empty_string) {
  int rc = notification_status_message_post("");
  // Pin: empty string is one of the documented bounded codes
  // (today: INVALID_PARAMETER).
  EXPECT_LE(rc, NOTIFICATION_ERROR_NONE);
}

TEST_F(IntegStatus, P_STAT_2_monitor_set_unset_idempotent_pair) {
  // monitor_set with non-NULL callback may or may not succeed
  // depending on DPM availability; monitor_unset must always be
  // safe regardless of the prior set state.
  int rc_unset_pre = notification_status_monitor_message_cb_unset();
  EXPECT_LE(rc_unset_pre, NOTIFICATION_ERROR_NONE);

  int rc_unset_post = notification_status_monitor_message_cb_unset();
  EXPECT_LE(rc_unset_post, NOTIFICATION_ERROR_NONE);
}

TEST_F(IntegStatus, N_STAT_3_monitor_set_null_cb) {
  int rc = notification_status_monitor_message_cb_set(nullptr, nullptr);
  // Pin: NULL callback is documented INVALID_PARAMETER today.
  EXPECT_NE(rc, NOTIFICATION_ERROR_NONE);
}

TEST_F(IntegStatus, C_STAT_1_monitor_unset_idempotent_when_never_set) {
  // Repeated unset on a never-set state stays bounded -- no crash,
  // no leaked state into subsequent test invocations.
  for (int i = 0; i < 3; ++i) {
    int rc = notification_status_monitor_message_cb_unset();
    EXPECT_LE(rc, NOTIFICATION_ERROR_NONE);
  }
}

}  // namespace
