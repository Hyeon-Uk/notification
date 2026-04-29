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

/*
 * Error-code propagation audit, mirror of dpm 18e691c (5 bugs fixed:
 * dropped returns + uninitialized scalar out-args read after dropped
 * getter calls). This TU pins four real bugs in the notification
 * client-lib EXPORT_API surface:
 *
 *   1. notification_system_setting_get_dnd_schedule_enabled_uid
 *      empty-result path returned NOTIFICATION_ERROR_NONE without
 *      writing `*uids` or `*count` — caller read uninit stack.
 *      (anti-pattern #2: uninitialized scalar out-arg)
 *
 *   2. notification_system_setting_load_dnd_allow_exception
 *      same shape: empty-result path returned NOTIFICATION_ERROR_NONE
 *      without writing `*dnd_allow_exception` or `*count`.
 *      (anti-pattern #2)
 *
 *   3. notification_noti_check_count_for_template
 *      sqlite3_step's return value (SQLITE_ROW=100, SQLITE_DONE=101,
 *      SQLITE_ERROR=1) was leaked to the EXPORT_API caller as the
 *      function's int return — none of those equal
 *      NOTIFICATION_ERROR_NONE (=0).
 *      (anti-pattern #4: wrong-domain return)
 *
 *   4. notification_noti_check_tag
 *      sqlite3_prepare_v2 failure path leaked SQLITE_* error code
 *      directly to the EXPORT_API caller instead of mapping to
 *      NOTIFICATION_ERROR_FROM_DB.
 *      (anti-pattern #4)
 *
 * Each test pins both the value contract (correct return code) and,
 * where applicable, the out-arg-zeroing contract (success-with-empty
 * payload writes 0 / NULL through the out pointers).
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>

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
#include <notification_noti.h>
#include <notification_private.h>
#include <notification_setting_service.h>
#include <notification_type.h>
}

namespace {

using ::testing::_;
using ::testing::DoAll;
using ::testing::Invoke;
using ::testing::Return;
using ::testing::SetArgPointee;

int FakeAulGetAppidBypid_EP(int /*pid*/, char* appid, int len) {
  if (appid && len > 0) snprintf(appid, len, "%s", "org.tizen.testapp.ep");
  return 0;
}

int FakeAulGetPkgidBypid_EP(int /*pid*/, char* pkgid, int len) {
  if (pkgid && len > 0) snprintf(pkgid, len, "%s", "org.tizen.testpkg.ep");
  return 0;
}

int FakeOpenV2_OK(const char*, sqlite3** db, int, const char*) {
  /* Allocate a small placeholder block so the production code's
   * `if (db != NULL)` check passes; finalize/close are also stubbed
   * out so we won't free into libsqlite3. */
  if (db) *db = reinterpret_cast<sqlite3*>(calloc(1, 16));
  return SQLITE_OK;
}

int FakePrepareV2_OK(sqlite3*, const char*, int, sqlite3_stmt** stmt,
    const char**) {
  if (stmt) *stmt = reinterpret_cast<sqlite3_stmt*>(calloc(1, 16));
  return SQLITE_OK;
}

int FakePrepareV2_Error(sqlite3*, const char*, int, sqlite3_stmt** stmt,
    const char**) {
  if (stmt) *stmt = nullptr;
  return SQLITE_ERROR;  /* SQLITE_ERROR == 1; this is the wrong-domain leak. */
}

int FakeFinalize_OK(sqlite3_stmt* stmt) {
  if (stmt) free(stmt);
  return SQLITE_OK;
}

int FakeClose_OK(sqlite3* db) {
  if (db) free(db);
  return SQLITE_OK;
}

int FakeGetTable_EmptyResult(sqlite3*, const char*, char*** result,
    int* row_count, int* col_count, char**) {
  /* Return success with zero rows. The pre-fix code does
   * `goto out` without setting *uids / *count. */
  if (result) *result = nullptr;
  if (row_count) *row_count = 0;
  if (col_count) *col_count = 0;
  return SQLITE_OK;
}

class ErrPropMocks :
    virtual public ::testing::NiceMock<AulMock>,
    virtual public ::testing::NiceMock<GlibMock>,
    virtual public ::testing::NiceMock<SqliteMock>,
    virtual public ::testing::NiceMock<PackageManagerMock>,
    virtual public ::testing::NiceMock<TzplatformConfigMock> {};

class NotificationErrorPropAuditTest : public TestFixture {
 public:
  NotificationErrorPropAuditTest()
      : TestFixture(std::make_unique<::ErrPropMocks>()) {}

  void SetUp() override {
    ON_CALL(GetMock<AulMock>(), aul_app_get_appid_bypid(_, _, _))
        .WillByDefault(Invoke(FakeAulGetAppidBypid_EP));
    ON_CALL(GetMock<AulMock>(), aul_app_get_pkgid_bypid(_, _, _))
        .WillByDefault(Invoke(FakeAulGetPkgidBypid_EP));
    ON_CALL(GetMock<PackageManagerMock>(), package_info_create(_, _))
        .WillByDefault(Return(-1));
    ON_CALL(GetMock<PackageManagerMock>(),
            pkgmgrinfo_appinfo_get_usr_appinfo(_, _, _))
        .WillByDefault(Return(-1));

    /* Default sqlite stubs that succeed and don't crash the test. */
    ON_CALL(GetMock<SqliteMock>(), sqlite3_open_v2(_, _, _, _))
        .WillByDefault(Invoke(FakeOpenV2_OK));
    ON_CALL(GetMock<SqliteMock>(), sqlite3_close(_))
        .WillByDefault(Invoke(FakeClose_OK));
    ON_CALL(GetMock<SqliteMock>(), sqlite3_close_v2(_))
        .WillByDefault(Invoke(FakeClose_OK));
    ON_CALL(GetMock<SqliteMock>(), sqlite3_finalize(_))
        .WillByDefault(Invoke(FakeFinalize_OK));
  }
};

}  // namespace

/* ----------------------------------------------------------------------- */
/*  Bug 1: dnd_schedule_enabled_uid empty-result leaves out-args uninit.   */
/* ----------------------------------------------------------------------- */

TEST_F(NotificationErrorPropAuditTest,
       DndScheduleEnabledUid_EmptyResult_ZeroesOutArgs) {
  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_get_table(_, _, _, _, _, _))
      .WillRepeatedly(Invoke(FakeGetTable_EmptyResult));

  /* Sentinel-poison both out-args. Pre-fix: function returns NONE but
   * leaves these untouched, so a downstream caller would walk a wild
   * `result_uids` pointer (or report a non-zero `count`). */
  uid_t* result_uids = reinterpret_cast<uid_t*>(0xdeadbeefULL);
  int result_count = 0x5a5a5a5a;

  int ret = notification_system_setting_get_dnd_schedule_enabled_uid(
      &result_uids, &result_count);

  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(result_uids, nullptr) << "empty-result path must zero *uids";
  EXPECT_EQ(result_count, 0) << "empty-result path must zero *count";
}

/* ----------------------------------------------------------------------- */
/*  Bug 2: load_dnd_allow_exception empty-result leaves out-args uninit.  */
/* ----------------------------------------------------------------------- */

TEST_F(NotificationErrorPropAuditTest,
       LoadDndAllowException_EmptyResult_ZeroesOutArgs) {
  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_get_table(_, _, _, _, _, _))
      .WillRepeatedly(Invoke(FakeGetTable_EmptyResult));

  dnd_allow_exception_h result_dnd =
      reinterpret_cast<dnd_allow_exception_h>(0xdeadbeefULL);
  int result_count = 0x5a5a5a5a;

  int ret = notification_system_setting_load_dnd_allow_exception(
      &result_dnd, &result_count, getuid());

  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(result_dnd, nullptr)
      << "empty-result path must zero *dnd_allow_exception";
  EXPECT_EQ(result_count, 0) << "empty-result path must zero *count";
}

/* ----------------------------------------------------------------------- */
/*  Bug 3: noti_check_count_for_template leaks SQLITE_ROW (=100) as ret.  */
/* ----------------------------------------------------------------------- */

TEST_F(NotificationErrorPropAuditTest,
       CheckCountForTemplate_StepRow_ReturnsNotificationDomain) {
  /* Build a notification_h: only caller_app_id is read by the SUT. */
  struct _notification noti = {};
  noti.caller_app_id = strdup("org.tizen.testpkg.ep");

  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_prepare_v2(_, _, _, _, _))
      .WillRepeatedly(Invoke(FakePrepareV2_OK));

  /* SQLITE_ROW = 100. Pre-fix the function returns 100 directly to
   * the caller — never equals NOTIFICATION_ERROR_NONE = 0. */
  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_step(_))
      .WillRepeatedly(Return(SQLITE_ROW));

  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_column_int(_, _))
      .WillRepeatedly(Return(7));

  int count = -1;
  int ret = notification_noti_check_count_for_template(&noti, &count);

  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE)
      << "SQLITE_ROW must be mapped to NOTIFICATION_ERROR_NONE";
  EXPECT_EQ(count, 7);

  free(noti.caller_app_id);
}

TEST_F(NotificationErrorPropAuditTest,
       CheckCountForTemplate_StepDone_ReturnsNotificationDomain) {
  struct _notification noti = {};
  noti.caller_app_id = strdup("org.tizen.testpkg.ep");

  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_prepare_v2(_, _, _, _, _))
      .WillRepeatedly(Invoke(FakePrepareV2_OK));

  /* SQLITE_DONE = 101. Pre-fix leaks 101 to caller. */
  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_step(_))
      .WillRepeatedly(Return(SQLITE_DONE));

  int count = -1;
  int ret = notification_noti_check_count_for_template(&noti, &count);

  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE)
      << "SQLITE_DONE must be mapped to NOTIFICATION_ERROR_NONE";
  EXPECT_EQ(count, 0);

  free(noti.caller_app_id);
}

/* ----------------------------------------------------------------------- */
/*  Bug 4: notification_noti_check_tag prepare_v2 failure leaks SQLITE_*. */
/* ----------------------------------------------------------------------- */

TEST_F(NotificationErrorPropAuditTest,
       CheckTag_PrepareFails_ReturnsNotificationDomain) {
  struct _notification noti = {};
  noti.caller_app_id = strdup("org.tizen.testpkg.ep");
  noti.tag = strdup("test_tag");

  /* prepare_v2 returns SQLITE_ERROR (=1). Pre-fix leaks `1` to the
   * EXPORT_API caller; post-fix maps to NOTIFICATION_ERROR_FROM_DB. */
  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_prepare_v2(_, _, _, _, _))
      .WillRepeatedly(Invoke(FakePrepareV2_Error));

  int ret = notification_noti_check_tag(&noti);

  EXPECT_EQ(ret, NOTIFICATION_ERROR_FROM_DB)
      << "sqlite3 prepare error must map to NOTIFICATION_ERROR_FROM_DB";
  EXPECT_NE(ret, SQLITE_ERROR)
      << "wrong-domain SQLITE_ERROR must not leak to EXPORT_API surface";

  free(noti.caller_app_id);
  free(noti.tag);
}
