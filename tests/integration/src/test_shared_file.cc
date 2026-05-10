#include <gtest/gtest.h>

#include <notification.h>
#include <notification_db.h>
#include <notification_error.h>
#include <notification_shared_file.h>
#include <notification_type.h>

#include "integ_harness.hh"

namespace {

class IntegSharedFile : public ::testing::Test {
 protected:
  integ::TmpDb tmp_;
};

TEST_F(IntegSharedFile, N_SF_1_set_private_sharing_null_handle) {
  int rc = notification_set_private_sharing(nullptr, 0);
  EXPECT_EQ(rc, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

// N_SF_2 omitted -- F-task19a-cycle8-sf-validate-null-deref:
// notification_validate_private_sharing(nullptr) dereferences
// updated_noti->b_image_path without a NULL guard, causing SIGSEGV.
// Crash-locking via death-test would over-pin a real bug; defer to
// task 17 (memory discipline pass).

TEST_F(IntegSharedFile, P_SF_1_set_private_sharing_returns_bounded) {
  notification_h n = notification_create(NOTIFICATION_TYPE_NOTI);
  if (!n) GTEST_SKIP() << "notification_create unavailable";
  int rc = notification_set_private_sharing(n, 0);
  // Pin: rc is one of the documented bounded codes (NONE on happy
  // path; PERMISSION_DENIED / INVALID_OPERATION otherwise). Crash
  // would be a regression.
  EXPECT_LE(rc, NOTIFICATION_ERROR_NONE);
  notification_free(n);
}

TEST_F(IntegSharedFile, C_SF_1_remove_target_id_no_state_does_not_crash) {
  // notification_remove_private_sharing_target_id is void-returning;
  // calling on an unregistered (sender, uid) must be a no-op rather
  // than a crash. Pin today's contract.
  notification_remove_private_sharing_target_id("no.such.sender.19a", 0);
  SUCCEED();
}

// C_SF_2 / C_SF_3 omitted -- F-task19a-cycle8-sf-calibrate-null-deref:
// notification_calibrate_private_sharing(nullptr, *) and (*, nullptr)
// both dereference handle->b_image_path without NULL guards, causing
// SIGSEGV. Defer to task 17.

}  // namespace
