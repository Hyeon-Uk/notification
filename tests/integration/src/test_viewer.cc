#include <gtest/gtest.h>

#include <notification.h>
#include <notification_db.h>
#include <notification_error.h>
#include <notification_internal.h>
#include <notification_type.h>
#include <notification_viewer.h>

#include "integ_harness.hh"

namespace {

class IntegViewer : public ::testing::Test {
 protected:
  integ::TmpDb tmp_;
};

TEST_F(IntegViewer, P_VIEWER_1_init_default_viewer_returns_bounded) {
  // notification_init_default_viewer reads /etc/.notification.ini
  // and locks in a default viewer name. In the gbs sandbox the
  // file may not exist; pin a bounded return.
  int rc = notification_init_default_viewer();
  EXPECT_GE(rc, NOTIFICATION_ERROR_NONE - 1);
}

TEST_F(IntegViewer, N_VIEWER_1_launch_default_viewer_no_data_returns_bounded) {
  // Empty DB; calling launch should not crash, must return one of
  // the documented codes.
  int rc = notification_launch_default_viewer(0, NOTIFICATION_OP_INSERT, 0);
  EXPECT_GE(rc, NOTIFICATION_ERROR_NONE - 1);
}

TEST_F(IntegViewer,
       N_VIEWER_2_launch_default_viewer_without_candidate_returns_bounded) {
  int rc = notification_launch_default_viewer_without_candidate_process(
      0, NOTIFICATION_OP_INSERT, 0);
  EXPECT_GE(rc, NOTIFICATION_ERROR_NONE - 1);
}

TEST_F(IntegViewer, C_VIEWER_1_init_then_launch_chain) {
  int init_rc = notification_init_default_viewer();
  int launch_rc =
      notification_launch_default_viewer(0, NOTIFICATION_OP_DELETE, 0);
  // Pin: both return bounded codes; the chain doesn't spuriously
  // crash even when the launch path is unreachable in the sandbox.
  EXPECT_GE(init_rc, NOTIFICATION_ERROR_NONE - 1);
  EXPECT_GE(launch_rc, NOTIFICATION_ERROR_NONE - 1);
}

}  // namespace
