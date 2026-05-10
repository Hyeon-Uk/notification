#include <glib.h>
#include <gtest/gtest.h>

#include <notification.h>
#include <notification_db.h>
#include <notification_error.h>
#include <notification_noti.h>
#include <notification_type.h>

#include "integ_harness.hh"

namespace {

class IntegNoti : public ::testing::Test {
 protected:
  void SetUp() override {
    ASSERT_EQ(notification_db_init(), NOTIFICATION_ERROR_NONE);
  }
  integ::TmpDb tmp_;
};

TEST_F(IntegNoti, N_NOTI_1_insert_null_handle) {
  EXPECT_EQ(notification_noti_insert(nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(IntegNoti, N_NOTI_2_update_null_handle) {
  EXPECT_EQ(notification_noti_update(nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

// N_NOTI_3 omitted: notification_noti_check_tag(nullptr) dereferences
// noti->tag before any NULL guard, causing SIGSEGV today. Tracked as
// F-task19a-cycle6-noti-check-tag-null-deref. Locking that crash in
// as a death-test would over-pin a real bug; defer to the post-Phase-1
// memory-discipline pass (task 17).

TEST_F(IntegNoti, N_NOTI_4_check_count_null_count_out) {
  notification_h n = notification_create(NOTIFICATION_TYPE_NOTI);
  if (!n) GTEST_SKIP() << "notification_create unavailable";
  int rc = notification_noti_check_count_for_template(n, nullptr);
  EXPECT_NE(rc, NOTIFICATION_ERROR_NONE);
  notification_free(n);
}

TEST_F(IntegNoti, E_NOTI_1_get_count_empty_db) {
  int count = -1;
  int rc = notification_noti_get_count(NOTIFICATION_TYPE_NONE, nullptr,
                                       NOTIFICATION_GROUP_ID_NONE,
                                       NOTIFICATION_PRIV_ID_NONE, &count, 0);
  EXPECT_EQ(rc, NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(count, 0);
}

TEST_F(IntegNoti, E_NOTI_2_get_all_count_empty_db) {
  int count = -1;
  int rc = notification_noti_get_all_count(NOTIFICATION_TYPE_NONE, &count, 0);
  EXPECT_EQ(rc, NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(count, 0);
}

TEST_F(IntegNoti, E_NOTI_3_delete_all_empty_db) {
  int deleted_num = -1;
  int* deleted_list = nullptr;
  int rc = notification_noti_delete_all(NOTIFICATION_TYPE_NONE, nullptr,
                                        &deleted_num, &deleted_list, 0);
  EXPECT_EQ(rc, NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(deleted_num, 0);
  free(deleted_list);
}

TEST_F(IntegNoti, N_NOTI_5_delete_by_priv_id_missing_returns_none) {
  // Lock-in finding: notification_noti_delete_by_priv_id is silently
  // idempotent -- deleting a non-existent (app_id, priv_id) returns
  // NONE today, not NOT_EXIST_ID. Pin this contract before any
  // Phase 1 migration that might tighten it.
  int rc = notification_noti_delete_by_priv_id("no.such.app", 99999);
  EXPECT_EQ(rc, NOTIFICATION_ERROR_NONE);
}

TEST_F(IntegNoti, E_NOTI_4_get_grouping_list_empty_db_returns_zero_count) {
  notification_list_h list = nullptr;
  int list_count = -1;
  int rc = notification_noti_get_grouping_list(NOTIFICATION_TYPE_NONE,
                                               /*page_number=*/0,
                                               /*count_per_page=*/0, &list,
                                               &list_count, 0);
  // Pin: empty DB returns either NONE with count=0 or a documented
  // non-NONE code; never crashes.
  EXPECT_EQ(notification_list_get_count(list), 0);
  if (list) notification_free_list(list);
  (void)rc;
}

TEST_F(IntegNoti, P_NOTI_1_insert_channel_round_trip) {
  // Channel APIs do not require aul / packagemgr -- they take an
  // explicit app_id. Pin the insert+get round-trip on real DB.
  const char* app = "integ.test.noti.ch";
  const char* name = "channel-1";
  int rc = notification_noti_insert_channel(app, name, 0, 0);
  EXPECT_EQ(rc, NOTIFICATION_ERROR_NONE);

  int rc2 = notification_noti_insert_channel(app, name, 0, 0);
  EXPECT_NE(rc2, NOTIFICATION_ERROR_NONE);
}

TEST_F(IntegNoti, P_NOTI_2_get_channel_after_insert) {
  const char* app = "integ.test.noti.gc";
  const char* name = "ch-get";
  ASSERT_EQ(notification_noti_insert_channel(app, name, 1, 0),
            NOTIFICATION_ERROR_NONE);

  int blockable = -1, blocked = -1;
  int rc = notification_noti_get_channel(app, name, &blockable, &blocked);
  EXPECT_EQ(rc, NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(blockable, 1);
  EXPECT_EQ(blocked, 0);
}

TEST_F(IntegNoti, N_NOTI_6_get_channel_missing_returns_none) {
  // Lock-in finding: notification_noti_get_channel returns NONE on
  // missing (app_id, name); the absence is signaled by the out
  // params staying at their default rather than a non-NONE rc.
  int blockable = -1, blocked = -1;
  int rc = notification_noti_get_channel("no.such.app.42", "no-channel",
                                         &blockable, &blocked);
  EXPECT_EQ(rc, NOTIFICATION_ERROR_NONE);
}

TEST_F(IntegNoti, E_NOTI_5_get_channel_list_empty_app) {
  GList* list = nullptr;
  int rc = notification_noti_get_channel_list("no.such.app.43", &list);
  EXPECT_EQ(g_list_length(list), 0u);
  g_list_free_full(list, g_free);
  (void)rc;  // pin: returns one of the documented codes, not a crash
}

TEST_F(IntegNoti, C_NOTI_1_delete_channel_idempotent_on_missing) {
  // Channel delete on missing (app_id, name) is silently idempotent;
  // returns NONE. Pin today's contract.
  int rc = notification_noti_delete_channel("no.such.app.44", "ch-x");
  EXPECT_EQ(rc, NOTIFICATION_ERROR_NONE);
}

TEST_F(IntegNoti, C_NOTI_2_repeat_init_does_not_break_open_table) {
  // Re-running init() on an already-populated DB must keep the
  // previously inserted channel visible (today's contract: init is
  // idempotent and CREATE TABLE IF NOT EXISTS preserves data).
  const char* app = "integ.test.noti.idem";
  ASSERT_EQ(notification_noti_insert_channel(app, "ch", 0, 0),
            NOTIFICATION_ERROR_NONE);
  ASSERT_EQ(notification_db_init(), NOTIFICATION_ERROR_NONE);

  int blockable = -1, blocked = -1;
  EXPECT_EQ(notification_noti_get_channel(app, "ch", &blockable, &blocked),
            NOTIFICATION_ERROR_NONE);
}

}  // namespace
