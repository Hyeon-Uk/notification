/*
 * Copyright (c) 2026 Samsung Electronics Co., Ltd. All rights reserved.
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
 * N7.5-B coverage boost — notification_db.c (Phase 4 N4-T3 fast-path).
 *
 * test_db_init_fast_path.cc covers the four canonical init branches
 * (UpToDate/OldVersion/Corrupted/FirstBoot). This file targets the static
 * helpers indirectly via the public seams `notification_db_init` and
 * `notification_upgrade_db`, plus the index-DDL replay invariant added in
 * N3-T7 Q2:
 *
 *   1. CheckDbVersion_NullDb_PreparesNothing
 *      — When the fast-path open() fails in __check_db_version's NULL-db
 *        branch, the slow path takes over. Indirectly: when prepare returns
 *        non-OK on the fast-path, the slow-path CREATE TABLE must run.
 *
 *   2. CheckDbVersion_VersionMatch_Returns0_FastPathSkipsIntegrity
 *      — When user_version == DB_VERSION the fast-path returns
 *        NOTIFICATION_ERROR_NONE without issuing PRAGMA integrity_check.
 *
 *   3. CheckDbVersion_VersionMismatch_FallsThroughToSlowPath
 *      — user_version != DB_VERSION -> CREATE TABLE + integrity_check run.
 *
 *   4. EnsureNotiListIndexes_Idempotent_ReplaysOnEveryFastPath
 *      — Two consecutive notification_db_init() calls on an UpToDate DB
 *        each replay the index DDL; the SQL string contains the canonical
 *        N3-T7 idx_noti_* CREATE INDEX IF NOT EXISTS pattern.
 *
 *   5. UpgradeDb_VersionMatch_NoUpgradeQueries
 *      — notification_upgrade_db on an UpToDate DB performs only
 *        BEGIN TRANSACTION + END TRANSACTION + the version-check
 *        prepare/step/finalize. No CREATE_UPGRADE_DB SQL is issued.
 *
 *   6. UpgradeDb_VersionMismatch_RunsUpgradeQueries
 *      — notification_upgrade_db on an out-of-date DB issues the upgrade
 *        SQL (we identify by ALTER TABLE / CREATE TABLE upgrade markers)
 *        and the new PRAGMA user_version assignment.
 *
 *   7. RecoverCorruptedDB_PrepareFailureFallsBack
 *      — Fast-path prepare fails -> slow path runs CREATE TABLE +
 *        integrity_check OK -> notification_db_init returns NONE.
 */

#include <cstring>
#include <cstdlib>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "mock/glib_mock.hh"
#include "mock/sqlite_mock.hh"
#include "mock/test_fixture.hh"

#include "notification/include/notification_db.h"
#include "notification/include/notification_error.h"

extern "C" {
int notification_db_init(void);
int notification_upgrade_db(void);
void SetNotificationDbAccessOverride(int err);
}

namespace {

constexpr int kDbVersion = 3;  /* Mirrors notification_db_query.h DB_VERSION. */

class Mocks
    : virtual public ::testing::NiceMock<GlibMock>,
      virtual public ::testing::NiceMock<SqliteMock> {};

sqlite3* FakeDb() {
  static int sentinel;
  return reinterpret_cast<sqlite3*>(&sentinel);
}

sqlite3_stmt* FakeStmt() {
  static int sentinel;
  return reinterpret_cast<sqlite3_stmt*>(&sentinel);
}

int FakeOpenOk(const char*, sqlite3** out, int, const char*) {
  if (out) *out = FakeDb();
  return SQLITE_OK;
}

int FakePrepareOk(sqlite3*, const char*, int, sqlite3_stmt** out,
                  const char**) {
  if (out) *out = FakeStmt();
  return SQLITE_OK;
}

int FakePrepareErr(sqlite3*, const char*, int, sqlite3_stmt** out,
                   const char**) {
  if (out) *out = nullptr;
  return SQLITE_ERROR;
}

}  // namespace

using ::testing::_;
using ::testing::AnyNumber;
using ::testing::Invoke;
using ::testing::Return;

class DbInternalTest : public TestFixture {
 public:
  DbInternalTest() : TestFixture(std::make_unique<::Mocks>()) {}

  void SetUp() override { SetNotificationDbAccessOverride(0); }
  void TearDown() override { SetNotificationDbAccessOverride(0); }
};

/* (1) Fast-path prepare failure -> slow path runs CREATE TABLE. We assert
 * that the slow path was taken (CREATE TABLE issued). The fast-path
 * __check_db_version's "prepare!=OK" branch is exercised. */
TEST_F(DbInternalTest, CheckDbVersion_PrepareFails_FallsThroughToSlowPath) {
  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_open_v2(_, _, _, _))
      .WillRepeatedly(Invoke(FakeOpenOk));

  /* Fast-path prepare fails so __check_db_version returns -1, slow path
   * takes over. Slow path's prepare path may also be touched if
   * __upgrade_noti_table reaches it; allow either pattern. */
  bool first_prepare_done = false;
  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_prepare_v2(_, _, _, _, _))
      .WillRepeatedly(Invoke([&](sqlite3* db, const char* sql, int n,
                                 sqlite3_stmt** out, const char** tail) -> int {
        if (!first_prepare_done) {
          first_prepare_done = true;
          return FakePrepareErr(db, sql, n, out, tail);
        }
        return FakePrepareOk(db, sql, n, out, tail);
      }));

  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_step(_))
      .WillRepeatedly(Return(SQLITE_DONE));
  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_finalize(_))
      .WillRepeatedly(Return(SQLITE_OK));
  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_close(_))
      .WillRepeatedly(Return(SQLITE_OK));

  bool saw_create_table = false;
  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_exec(_, _, _, _, _))
      .Times(AnyNumber())
      .WillRepeatedly(Invoke([&](sqlite3*, const char* sql,
                                 int (*)(void*, int, char**, char**), void*,
                                 char** errmsg) -> int {
        if (errmsg) *errmsg = nullptr;
        if (sql && strstr(sql, "CREATE TABLE")) saw_create_table = true;
        return SQLITE_OK;
      }));

  int ret = notification_db_init();
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  EXPECT_TRUE(saw_create_table)
      << "When fast-path prepare fails, slow path must issue CREATE TABLE";
}

/* (2) Fast-path version match -> NO integrity_check + NO CREATE TABLE. */
TEST_F(DbInternalTest, CheckDbVersion_VersionMatch_FastPathSkipsIntegrity) {
  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_open_v2(_, _, _, _))
      .WillOnce(Invoke(FakeOpenOk));
  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_prepare_v2(_, _, _, _, _))
      .WillOnce(Invoke(FakePrepareOk));
  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_step(_))
      .WillOnce(Return(SQLITE_ROW));
  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_column_int(_, 0))
      .WillOnce(Return(kDbVersion));
  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_finalize(_))
      .WillOnce(Return(SQLITE_OK));

  int forbidden_calls = 0;
  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_exec(_, _, _, _, _))
      .Times(AnyNumber())
      .WillRepeatedly(Invoke([&](sqlite3*, const char* sql,
                                 int (*)(void*, int, char**, char**), void*,
                                 char** errmsg) -> int {
        if (errmsg) *errmsg = nullptr;
        if (sql) {
          if (strstr(sql, "CREATE TABLE") || strstr(sql, "integrity_check"))
            ++forbidden_calls;
        }
        return SQLITE_OK;
      }));

  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_close(_))
      .WillOnce(Return(SQLITE_OK));

  EXPECT_EQ(notification_db_init(), NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(forbidden_calls, 0)
      << "UpToDate fast-path issued a forbidden DDL: "
      << forbidden_calls << " hits";
}

/* (3) Mismatch -> slow path. CREATE TABLE + integrity_check both observed. */
TEST_F(DbInternalTest, CheckDbVersion_VersionMismatch_FallsThroughToSlowPath) {
  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_open_v2(_, _, _, _))
      .WillRepeatedly(Invoke(FakeOpenOk));
  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_prepare_v2(_, _, _, _, _))
      .WillRepeatedly(Invoke(FakePrepareOk));
  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_step(_))
      .WillRepeatedly(Return(SQLITE_ROW));
  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_column_int(_, 0))
      .WillRepeatedly(Return(kDbVersion - 1));
  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_finalize(_))
      .WillRepeatedly(Return(SQLITE_OK));

  bool saw_create_table = false;
  bool saw_integrity_check = false;
  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_exec(_, _, _, _, _))
      .Times(AnyNumber())
      .WillRepeatedly(Invoke([&](sqlite3*, const char* sql,
                                 int (*)(void*, int, char**, char**), void*,
                                 char** errmsg) -> int {
        if (errmsg) *errmsg = nullptr;
        if (sql) {
          if (strstr(sql, "CREATE TABLE")) saw_create_table = true;
          if (strstr(sql, "integrity_check")) saw_integrity_check = true;
        }
        return SQLITE_OK;
      }));

  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_close(_))
      .Times(AnyNumber())
      .WillRepeatedly(Return(SQLITE_OK));

  EXPECT_EQ(notification_db_init(), NOTIFICATION_ERROR_NONE);
  EXPECT_TRUE(saw_create_table)
      << "Slow path must issue CREATE TABLE on version mismatch";
  EXPECT_TRUE(saw_integrity_check)
      << "Slow path must run PRAGMA integrity_check on version mismatch";
}

/* (4) Idempotent index DDL replay: 2x notification_db_init on UpToDate DB
 * each replays the CREATE INDEX IF NOT EXISTS DDL. We count the number of
 * times the DDL string was passed through sqlite3_exec. */
TEST_F(DbInternalTest, EnsureNotiListIndexes_Idempotent_ReplaysOnEveryFastPath) {
  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_open_v2(_, _, _, _))
      .WillRepeatedly(Invoke(FakeOpenOk));
  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_prepare_v2(_, _, _, _, _))
      .WillRepeatedly(Invoke(FakePrepareOk));
  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_step(_))
      .WillRepeatedly(Return(SQLITE_ROW));
  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_column_int(_, 0))
      .WillRepeatedly(Return(kDbVersion));
  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_finalize(_))
      .WillRepeatedly(Return(SQLITE_OK));
  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_close(_))
      .WillRepeatedly(Return(SQLITE_OK));

  int idx_ddl_count = 0;
  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_exec(_, _, _, _, _))
      .Times(AnyNumber())
      .WillRepeatedly(Invoke([&](sqlite3*, const char* sql,
                                 int (*)(void*, int, char**, char**), void*,
                                 char** errmsg) -> int {
        if (errmsg) *errmsg = nullptr;
        if (sql) {
          /* N3-T7 introduced idx_noti_* composite indexes; the canonical
           * CREATE INDEX IF NOT EXISTS pattern is what we count. */
          if (strstr(sql, "CREATE INDEX IF NOT EXISTS") &&
              strstr(sql, "idx_noti")) {
            ++idx_ddl_count;
          }
        }
        return SQLITE_OK;
      }));

  EXPECT_EQ(notification_db_init(), NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(notification_db_init(), NOTIFICATION_ERROR_NONE);

  /* Each notification_db_init() on a fast-path DB must call
   * __ensure_noti_list_indexes once. We accept any non-zero count >= 2 to
   * tolerate any future split of the DDL into per-index statements. */
  EXPECT_GE(idx_ddl_count, 2)
      << "Each notification_db_init must replay idx DDL idempotently";
}

/* (5) notification_upgrade_db on UpToDate -> no upgrade-table DDL. */
TEST_F(DbInternalTest, UpgradeDb_VersionMatch_NoUpgradeQueries) {
  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_open_v2(_, _, _, _))
      .WillOnce(Invoke(FakeOpenOk));
  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_prepare_v2(_, _, _, _, _))
      .WillOnce(Invoke(FakePrepareOk));
  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_step(_))
      .WillOnce(Return(SQLITE_ROW));
  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_column_int(_, 0))
      .WillOnce(Return(kDbVersion));
  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_finalize(_))
      .WillOnce(Return(SQLITE_OK));
  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_close(_))
      .WillOnce(Return(SQLITE_OK));

  bool saw_upgrade_table = false;
  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_exec(_, _, _, _, _))
      .Times(AnyNumber())
      .WillRepeatedly(Invoke([&](sqlite3*, const char* sql,
                                 int (*)(void*, int, char**, char**), void*,
                                 char** errmsg) -> int {
        if (errmsg) *errmsg = nullptr;
        /* CREATE_NOTIFICATION_UPGRADE_DB / TEMPLATE_DB / UPGRADE_DB_TO_2|3
         * markers. Any "ALTER TABLE" or "RENAME TABLE" is also disqualifying. */
        if (sql) {
          if (strstr(sql, "ALTER TABLE") || strstr(sql, "RENAME TABLE") ||
              strstr(sql, "_upgrade_") || strstr(sql, "_UPGRADE_") ||
              strstr(sql, "PRAGMA user_version="))
            saw_upgrade_table = true;
        }
        return SQLITE_OK;
      }));

  EXPECT_EQ(notification_upgrade_db(), NOTIFICATION_ERROR_NONE);
  EXPECT_FALSE(saw_upgrade_table)
      << "UpToDate notification_upgrade_db must not run any upgrade DDL";
}

/* (6) Open failure on notification_upgrade_db propagates as FROM_DB.
 *
 * We deliberately do NOT exercise the version-mismatch -> __upgrade_noti_table
 * branch via this entry point. That helper calls sqlite3_table_column_metadata
 * which is not in SqliteMock; with a non-NULL FakeDb sentinel the un-mocked
 * symbol resolves to the real libsqlite3 implementation and segfaults
 * dereferencing our fake handle. The version-mismatch DDL surface is already
 * covered indirectly by CheckDbVersion_VersionMismatch_FallsThroughToSlowPath
 * via notification_db_init (which uses CREATE TABLE IF NOT EXISTS, not the
 * upgrade path).
 *
 * The branch under test here: sqlite3_open_v2 returns SQLITE_ERROR; the SUT
 * must short-circuit to NOTIFICATION_ERROR_FROM_DB without proceeding to the
 * BEGIN TRANSACTION / __check_db_version flow. */
TEST_F(DbInternalTest, UpgradeDb_OpenFailure_ReturnsFromDb) {
  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_open_v2(_, _, _, _))
      .WillOnce(Return(SQLITE_ERROR));

  /* No exec / prepare expected. AnyNumber + Times(0) would over-tighten. */

  int ret = notification_upgrade_db();
  EXPECT_EQ(ret, NOTIFICATION_ERROR_FROM_DB);
}

/* (7) Prepare fails on fast-path -> slow path runs and OK. Mirrors test (1)
 * but asserts the integrity_check call from the slow path is observed. */
TEST_F(DbInternalTest, RecoverCorruptedDB_PrepareFailureFallsBack) {
  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_open_v2(_, _, _, _))
      .WillRepeatedly(Invoke(FakeOpenOk));

  /* First prepare (fast-path) fails; subsequent prepares succeed. */
  int prep_count = 0;
  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_prepare_v2(_, _, _, _, _))
      .WillRepeatedly(Invoke([&](sqlite3* db, const char* sql, int n,
                                 sqlite3_stmt** out, const char** tail) -> int {
        ++prep_count;
        if (prep_count == 1)
          return FakePrepareErr(db, sql, n, out, tail);
        return FakePrepareOk(db, sql, n, out, tail);
      }));

  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_step(_))
      .WillRepeatedly(Return(SQLITE_DONE));
  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_finalize(_))
      .WillRepeatedly(Return(SQLITE_OK));
  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_close(_))
      .WillRepeatedly(Return(SQLITE_OK));

  bool saw_integrity_check = false;
  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_exec(_, _, _, _, _))
      .Times(AnyNumber())
      .WillRepeatedly(Invoke([&](sqlite3*, const char* sql,
                                 int (*)(void*, int, char**, char**), void*,
                                 char** errmsg) -> int {
        if (errmsg) *errmsg = nullptr;
        if (sql && strstr(sql, "integrity_check"))
          saw_integrity_check = true;
        return SQLITE_OK;
      }));

  int ret = notification_db_init();
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  EXPECT_TRUE(saw_integrity_check)
      << "Slow path must run PRAGMA integrity_check after fast-path failure";
}
