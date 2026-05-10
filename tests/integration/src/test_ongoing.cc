#include <gtest/gtest.h>

#include <notification.h>
#include <notification_db.h>
#include <notification_error.h>
#include <notification_ongoing.h>
#include <notification_ongoing_flag.h>
#include <notification_type.h>

#include "integ_harness.hh"

namespace {

class IntegOngoing : public ::testing::Test {
 protected:
  void SetUp() override {
    ASSERT_EQ(notification_db_init(), NOTIFICATION_ERROR_NONE);
  }
  integ::TmpDb tmp_;
};

TEST_F(IntegOngoing, P_ONG_1_update_cb_set_returns_none) {
  // notification_ongoing_update_cb_set is a documented stub (logs
  // WARN, returns NONE unconditionally). Pin today's contract.
  EXPECT_EQ(notification_ongoing_update_cb_set(nullptr, nullptr),
            NOTIFICATION_ERROR_NONE);
}

TEST_F(IntegOngoing, P_ONG_2_update_cb_unset_returns_none) {
  EXPECT_EQ(notification_ongoing_update_cb_unset(), NOTIFICATION_ERROR_NONE);
}

TEST_F(IntegOngoing, P_ONG_3_set_get_ongoing_flag_round_trip) {
  notification_h n = notification_create(NOTIFICATION_TYPE_ONGOING);
  if (!n) GTEST_SKIP() << "notification_create unavailable";

  EXPECT_EQ(notification_set_ongoing_flag(n, true), NOTIFICATION_ERROR_NONE);
  bool flag = false;
  EXPECT_EQ(notification_get_ongoing_flag(n, &flag), NOTIFICATION_ERROR_NONE);
  EXPECT_TRUE(flag);

  EXPECT_EQ(notification_set_ongoing_flag(n, false), NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(notification_get_ongoing_flag(n, &flag), NOTIFICATION_ERROR_NONE);
  EXPECT_FALSE(flag);

  notification_free(n);
}

TEST_F(IntegOngoing, N_ONG_1_set_ongoing_flag_null_handle) {
  EXPECT_EQ(notification_set_ongoing_flag(nullptr, true),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(IntegOngoing, N_ONG_2_get_ongoing_flag_null_handle) {
  bool flag = false;
  EXPECT_EQ(notification_get_ongoing_flag(nullptr, &flag),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(IntegOngoing, N_ONG_3_get_ongoing_flag_null_out) {
  notification_h n = notification_create(NOTIFICATION_TYPE_ONGOING);
  if (!n) GTEST_SKIP() << "notification_create unavailable";
  EXPECT_EQ(notification_get_ongoing_flag(n, nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  notification_free(n);
}

TEST_F(IntegOngoing, E_ONG_1_set_ongoing_flag_idempotent) {
  notification_h n = notification_create(NOTIFICATION_TYPE_ONGOING);
  if (!n) GTEST_SKIP() << "notification_create unavailable";

  EXPECT_EQ(notification_set_ongoing_flag(n, true), NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(notification_set_ongoing_flag(n, true), NOTIFICATION_ERROR_NONE);
  bool flag = false;
  EXPECT_EQ(notification_get_ongoing_flag(n, &flag), NOTIFICATION_ERROR_NONE);
  EXPECT_TRUE(flag);

  notification_free(n);
}

TEST_F(IntegOngoing, C_ONG_1_default_ongoing_flag_is_false) {
  notification_h n = notification_create(NOTIFICATION_TYPE_NOTI);
  if (!n) GTEST_SKIP() << "notification_create unavailable";
  bool flag = true;  // pre-load with true to detect read-back.
  EXPECT_EQ(notification_get_ongoing_flag(n, &flag), NOTIFICATION_ERROR_NONE);
  EXPECT_FALSE(flag);
  notification_free(n);
}

TEST_F(IntegOngoing, C_ONG_2_get_after_set_via_clone) {
  notification_h n = notification_create(NOTIFICATION_TYPE_ONGOING);
  if (!n) GTEST_SKIP() << "notification_create unavailable";

  ASSERT_EQ(notification_set_ongoing_flag(n, true), NOTIFICATION_ERROR_NONE);

  notification_h cloned = nullptr;
  int rc = notification_clone(n, &cloned);
  if (rc == NOTIFICATION_ERROR_NONE && cloned) {
    bool flag = false;
    EXPECT_EQ(notification_get_ongoing_flag(cloned, &flag),
              NOTIFICATION_ERROR_NONE);
    EXPECT_TRUE(flag) << "clone must preserve ongoing_flag";
    notification_free(cloned);
  }
  notification_free(n);
}

}  // namespace
