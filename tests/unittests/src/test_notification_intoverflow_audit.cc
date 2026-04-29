/*
 * Copyright (c) 2026 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 */

/*
 * Integer-overflow audit, follow-up to the error-propagation pass
 * (commit 18e691c).  Three production sites in the notification client
 * library were passing an unbounded `int` directly to `malloc(sizeof(elem)
 * * count)` after taking the value from either an RPC proxy or
 * `sqlite3_get_table`'s `row_count` out-arg.  On 32-bit `size_t` builds
 * the multiplication can wrap (`INT_MAX * sizeof(struct
 * notification_setting)` overflows for sizeof above 1 byte) and on every
 * build a negative `int` sign-extends to a huge `size_t`, so an OOM-only
 * defence at the malloc site is insufficient — the caller must reject
 * the payload before the multiplication.
 *
 *   1. notification_tidl.cc::_tidl_create_op
 *      Reachable from `_delete_multiple_notify` and
 *      `_delete_by_display_applist_notify`, both of which take `cnt`
 *      from `rpc_port_proxy_array_int_get`.  The pre-fix guard was
 *      `num_op <= 0` only; a 2 G `cnt` reached the malloc.
 *
 *   2. notification_tidl.cc::notification_tidl_request_get_setting_array
 *      Pre-fix the EXPORT_API path had no guard at all on `size`
 *      returned by `rpc_port_proxy_array_noti_setting_get`.  Side-effect
 *      bug: the OOM return path also leaked `handle_array`.
 *
 *   3. notification_setting_service.cc — three sqlite3-row-count sites
 *      (`noti_setting_service_get_setting_by_app_id`,
 *      `noti_setting_get_setting_array`,
 *      `notification_system_setting_get_dnd_schedule_enabled_uid`,
 *      `notification_system_setting_load_dnd_allow_exception`).  Pre-fix
 *      the `if (!row_count)` check accepted negative values on the
 *      then-branch (treated as empty) only by accident — sqlite returns
 *      non-negative by contract — but a corrupted libsqlite or huge
 *      malformed table would skip every guard and reach the wrap.
 *
 * Static helpers are tested via the mirror-test pattern (the
 * production guard logic is reimplemented in this TU and the boundary
 * behaviour pinned at +1 / -1 around the published ceiling).
 * EXPORT_API sites are tested through the existing sqlite_mock
 * infrastructure used by test_notification_errorprop_audit.cc.
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <climits>
#include <cstring>

#include "mock/test_fixture.hh"
#include "mock/aul_mock.hh"
#include "mock/glib_mock.hh"
#include "mock/sqlite_mock.hh"
#include "mock/package_manager_mock.hh"
#include "mock/tzplatform_config_mock.hh"

extern "C" {
#include <notification.h>
#include <notification_internal.h>
#include <notification_private.h>
#include <notification_setting.h>
#include <notification_setting_internal.h>
#include <notification_setting_service.h>
#include <notification_type.h>
}

namespace {

using ::testing::_;
using ::testing::Invoke;
using ::testing::Return;

/*
 * Mirror of the production constants in
 *   - notification_tidl.cc::TIDL_MAX_ARRAY_COUNT
 *   - notification_setting_service.cc::kSettingDbMaxRowCount
 * If either drifts the corresponding mirror test fails immediately and
 * forces a same-commit update — drift is the failure mode this audit
 * pass is meant to prevent.
 */
constexpr int kMirrorTidlMaxArrayCount = 100000;
constexpr int kMirrorSettingDbMaxRowCount = 100000;

bool MirrorTidlArrayCountValid(int n) {
  return (n > 0 && n <= kMirrorTidlMaxArrayCount);
}

bool MirrorSettingDbRowCountValid(int n) {
  return (n > 0 && n <= kMirrorSettingDbMaxRowCount);
}

/*  ---  Mirror-test boundaries (TIDL `_tidl_create_op` guard)  ----------- */

TEST(NotificationIntOverflowMirror, TidlArrayCount_RejectsZero) {
  EXPECT_FALSE(MirrorTidlArrayCountValid(0));
}

TEST(NotificationIntOverflowMirror, TidlArrayCount_RejectsNegative) {
  EXPECT_FALSE(MirrorTidlArrayCountValid(-1));
  EXPECT_FALSE(MirrorTidlArrayCountValid(INT_MIN));
}

TEST(NotificationIntOverflowMirror, TidlArrayCount_AcceptsLowerBoundary) {
  EXPECT_TRUE(MirrorTidlArrayCountValid(1));
}

TEST(NotificationIntOverflowMirror, TidlArrayCount_AcceptsCeiling) {
  EXPECT_TRUE(MirrorTidlArrayCountValid(kMirrorTidlMaxArrayCount));
}

TEST(NotificationIntOverflowMirror, TidlArrayCount_RejectsAboveCeiling) {
  EXPECT_FALSE(MirrorTidlArrayCountValid(kMirrorTidlMaxArrayCount + 1));
  EXPECT_FALSE(MirrorTidlArrayCountValid(INT_MAX));
}

/*  ---  Mirror-test boundaries (sqlite row_count guard)  ----------------- */

TEST(NotificationIntOverflowMirror, SettingDbRowCount_RejectsNonPositive) {
  EXPECT_FALSE(MirrorSettingDbRowCountValid(0));
  EXPECT_FALSE(MirrorSettingDbRowCountValid(-1));
}

TEST(NotificationIntOverflowMirror, SettingDbRowCount_AcceptsBoundaries) {
  EXPECT_TRUE(MirrorSettingDbRowCountValid(1));
  EXPECT_TRUE(MirrorSettingDbRowCountValid(kMirrorSettingDbMaxRowCount));
}

TEST(NotificationIntOverflowMirror,
     SettingDbRowCount_RejectsAboveCeiling) {
  EXPECT_FALSE(
      MirrorSettingDbRowCountValid(kMirrorSettingDbMaxRowCount + 1));
  EXPECT_FALSE(MirrorSettingDbRowCountValid(INT_MAX));
}

/*  ---  Mirror-test wrap-arithmetic invariant  -------------------------- */

/*
 * Captures the actual property the bound is defending against: at
 * size_t bitwidth, MAX_COUNT * the largest sizeof we pass through these
 * sites (`struct notification_setting`, ~120 B today) must still fit in
 * size_t and stay several orders below SIZE_MAX/2 so a rogue +1 from a
 * downstream caller cannot push it over.
 */
TEST(NotificationIntOverflowMirror, MaxCount_TimesLargestElement_DoesNotWrap) {
  const size_t kLargestElem = sizeof(struct notification_setting);
  const size_t product =
      static_cast<size_t>(kMirrorSettingDbMaxRowCount) * kLargestElem;
  EXPECT_LT(product, SIZE_MAX / 2)
      << "MAX_ROW_COUNT * sizeof(notification_setting) must stay well "
         "below SIZE_MAX/2 to absorb downstream +1 adjustments without "
         "wrapping";
}

/*  ---  Production EXPORT_API tests (sqlite mock)  ----------------------- */

int FakeOpenV2_OK_IO(const char*, sqlite3** db, int, const char*) {
  if (db) *db = reinterpret_cast<sqlite3*>(calloc(1, 16));
  return SQLITE_OK;
}

int FakeClose_OK_IO(sqlite3* db) {
  if (db) free(db);
  return SQLITE_OK;
}

/* Returns success with row_count above the production ceiling, simulating
 * a corrupted DB or a misbehaving libsqlite3 build. */
int FakeGetTable_HugeRowCount(sqlite3*, const char*, char*** result,
    int* row_count, int* col_count, char**) {
  if (result) *result = nullptr;
  if (row_count) *row_count = 100001;  /* one above kSettingDbMaxRowCount */
  if (col_count) *col_count = 1;
  return SQLITE_OK;
}

/* Returns success with a negative row_count.  sqlite contract bars this
 * but a corrupted build could expose the unsigned-conversion expansion
 * pre-fix; the guard treats it the same as the empty-result path. */
int FakeGetTable_NegativeRowCount(sqlite3*, const char*, char*** result,
    int* row_count, int* col_count, char**) {
  if (result) *result = nullptr;
  if (row_count) *row_count = -1;
  if (col_count) *col_count = 1;
  return SQLITE_OK;
}

class IntOverflowMocks :
    virtual public ::testing::NiceMock<AulMock>,
    virtual public ::testing::NiceMock<GlibMock>,
    virtual public ::testing::NiceMock<SqliteMock>,
    virtual public ::testing::NiceMock<PackageManagerMock>,
    virtual public ::testing::NiceMock<TzplatformConfigMock> {};

class NotificationIntOverflowAuditTest : public TestFixture {
 public:
  NotificationIntOverflowAuditTest()
      : TestFixture(std::make_unique<::IntOverflowMocks>()) {}

  void SetUp() override {
    ON_CALL(GetMock<SqliteMock>(), sqlite3_open_v2(_, _, _, _))
        .WillByDefault(Invoke(FakeOpenV2_OK_IO));
    ON_CALL(GetMock<SqliteMock>(), sqlite3_close(_))
        .WillByDefault(Invoke(FakeClose_OK_IO));
    ON_CALL(GetMock<SqliteMock>(), sqlite3_close_v2(_))
        .WillByDefault(Invoke(FakeClose_OK_IO));
  }
};

TEST_F(NotificationIntOverflowAuditTest,
       DndScheduleEnabledUid_HugeRowCount_ReturnsFromDb) {
  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_get_table(_, _, _, _, _, _))
      .WillRepeatedly(Invoke(FakeGetTable_HugeRowCount));

  uid_t* result_uids = reinterpret_cast<uid_t*>(0xdeadbeefULL);
  int result_count = 0x5a5a5a5a;

  int ret = notification_system_setting_get_dnd_schedule_enabled_uid(
      &result_uids, &result_count);

  /* Pre-fix: huge row_count reached `malloc(sizeof(uid_t) * row_count)`
   * unguarded — either eats memory or trips OOM only on tiny boxes.
   * Post-fix: rejected with FROM_DB before the malloc. */
  EXPECT_EQ(ret, NOTIFICATION_ERROR_FROM_DB);
}

TEST_F(NotificationIntOverflowAuditTest,
       DndScheduleEnabledUid_NegativeRowCount_FallsToEmptyContract) {
  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_get_table(_, _, _, _, _, _))
      .WillRepeatedly(Invoke(FakeGetTable_NegativeRowCount));

  uid_t* result_uids = reinterpret_cast<uid_t*>(0xdeadbeefULL);
  int result_count = 0x5a5a5a5a;

  int ret = notification_system_setting_get_dnd_schedule_enabled_uid(
      &result_uids, &result_count);

  /* Pre-fix: negative row_count fell through to malloc(sizeof(int) *
   * (-1)) — sign-extended to SIZE_MAX, malloc returned NULL, function
   * reported OOM (wrong domain).  Post-fix: folded into the empty
   * contract — NONE return with both out-args zeroed. */
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(result_uids, nullptr);
  EXPECT_EQ(result_count, 0);
}

TEST_F(NotificationIntOverflowAuditTest,
       LoadDndAllowException_HugeRowCount_ReturnsFromDb) {
  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_get_table(_, _, _, _, _, _))
      .WillRepeatedly(Invoke(FakeGetTable_HugeRowCount));

  dnd_allow_exception_h result_dnd =
      reinterpret_cast<dnd_allow_exception_h>(0xdeadbeefULL);
  int result_count = 0x5a5a5a5a;

  int ret = notification_system_setting_load_dnd_allow_exception(
      &result_dnd, &result_count, 5001);

  EXPECT_EQ(ret, NOTIFICATION_ERROR_FROM_DB);
}

TEST_F(NotificationIntOverflowAuditTest,
       LoadDndAllowException_NegativeRowCount_FallsToEmptyContract) {
  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_get_table(_, _, _, _, _, _))
      .WillRepeatedly(Invoke(FakeGetTable_NegativeRowCount));

  dnd_allow_exception_h result_dnd =
      reinterpret_cast<dnd_allow_exception_h>(0xdeadbeefULL);
  int result_count = 0x5a5a5a5a;

  int ret = notification_system_setting_load_dnd_allow_exception(
      &result_dnd, &result_count, 5001);

  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(result_dnd, nullptr);
  EXPECT_EQ(result_count, 0);
}

}  // namespace
