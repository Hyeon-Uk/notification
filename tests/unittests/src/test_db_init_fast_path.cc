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
 * N4-T3: notification_db_init() fast-path tests.
 *
 * Goal: cut common-case `notification_init` cost from ~80-200ms to <5ms by
 * skipping CREATE TABLE IF NOT EXISTS + PRAGMA integrity_check when the DB
 * file exists, opens RW, and reports the expected DB_VERSION.
 *
 * Each TEST validates one branch of the new fast-path logic:
 *
 *   UpToDate_SkipsIntegrityCheck   — fast-path taken; sqlite3_exec must
 *                                    NEVER be invoked.
 *   OldVersion_FullInit            — version mismatch -> slow path runs
 *                                    (CREATE TABLE + integrity_check).
 *   CorruptedDB_RecoveryPath       — integrity_check returns SQLITE_CORRUPT
 *                                    -> __recover_corrupted_db replays.
 *   FirstBoot_FullInit             — DBPATH absent -> slow path runs.
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
void SetNotificationDbAccessOverride(int err);
}

namespace {

/* DB_VERSION mirrors notification_db_query.h:22 (#define DB_VERSION 3). */
constexpr int kDbVersion = 3;

class Mocks
    : virtual public ::testing::NiceMock<GlibMock>,
      virtual public ::testing::NiceMock<SqliteMock> {};

}  // namespace

using ::testing::_;
using ::testing::AnyNumber;
using ::testing::Invoke;
using ::testing::Return;

class DbInitFastPathTest : public TestFixture {
 public:
  DbInitFastPathTest() : TestFixture(std::make_unique<::Mocks>()) {}

  void SetUp() override {
    /* Default: DB file is present (legacy mock behaviour). */
    SetNotificationDbAccessOverride(0);
  }

  void TearDown() override {
    /* Always restore so subsequent tests inherit the legacy behaviour. */
    SetNotificationDbAccessOverride(0);
  }
};

/* ---------- shared mock helpers ----------------------------------------- */

namespace {

/*
 * Faux sqlite3 / sqlite3_stmt handles. The mock returns the address of these
 * via the out-pointer so the SUT can pass them around without crashing.
 */
sqlite3* FakeDb() {
  static int sentinel;
  return reinterpret_cast<sqlite3*>(&sentinel);
}

sqlite3_stmt* FakeStmt() {
  static int sentinel;
  return reinterpret_cast<sqlite3_stmt*>(&sentinel);
}

int FakeOpenOk(const char* /*path*/, sqlite3** out, int /*flags*/,
               const char* /*vfs*/) {
  if (out) *out = FakeDb();
  return SQLITE_OK;
}

int FakePrepareOk(sqlite3* /*db*/, const char* /*sql*/, int /*n*/,
                  sqlite3_stmt** out, const char** /*tail*/) {
  if (out) *out = FakeStmt();
  return SQLITE_OK;
}

}  // namespace

/* ------------------------------------------------------------------------ *
 * 1. UpToDate_SkipsIntegrityCheck
 *
 * Pre-conditions: DB file present (default), sqlite3_open RW succeeds,
 *   __check_db_version returns 0 (PRAGMA user_version == DB_VERSION).
 * Expectation:  sqlite3_exec is invoked at most for the cheap idempotent
 *   index DDL (__ensure_noti_list_indexes). It MUST NOT be called with
 *   "PRAGMA integrity_check" or with a CREATE TABLE statement.
 * ------------------------------------------------------------------------ */
TEST_F(DbInitFastPathTest, UpToDate_SkipsIntegrityCheck) {
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

  /* Fast-path: only the cheap idempotent index DDL may run via sqlite3_exec.
   * Critically NO "integrity_check" and NO "CREATE TABLE" statement may be
   * issued. We assert by inspecting the SQL string of every exec call. */
  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_exec(_, _, _, _, _))
      .Times(AnyNumber())
      .WillRepeatedly(Invoke([](sqlite3*, const char* sql,
                                int (*)(void*, int, char**, char**), void*,
                                char** errmsg) -> int {
        if (errmsg) *errmsg = nullptr;
        EXPECT_TRUE(sql != nullptr);
        if (sql) {
          EXPECT_EQ(strstr(sql, "integrity_check"), nullptr)
              << "Fast-path must NOT run PRAGMA integrity_check";
          EXPECT_EQ(strstr(sql, "CREATE TABLE"), nullptr)
              << "Fast-path must NOT issue CREATE TABLE";
        }
        return SQLITE_OK;
      }));

  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_close(_))
      .WillOnce(Return(SQLITE_OK));

  int ret = notification_db_init();
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
}

/* ------------------------------------------------------------------------ *
 * 2. OldVersion_FullInit
 *
 * Pre-conditions: DB present, fast-path open succeeds, but PRAGMA
 *   user_version reports an older value -> __check_db_version returns -1.
 *   The slow path then re-opens with CREATE|RW, runs CREATE TABLE
 *   IF NOT EXISTS, and PRAGMA integrity_check.
 * Expectation:  sqlite3_exec gets called with both "CREATE TABLE" and
 *   "integrity_check" SQL.
 * ------------------------------------------------------------------------ */
TEST_F(DbInitFastPathTest, OldVersion_FullInit) {
  /* Fast-path open + slow-path open. */
  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_open_v2(_, _, _, _))
      .WillRepeatedly(Invoke(FakeOpenOk));

  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_prepare_v2(_, _, _, _, _))
      .WillRepeatedly(Invoke(FakePrepareOk));

  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_step(_))
      .WillRepeatedly(Return(SQLITE_ROW));

  /* Old version -> __check_db_version returns -1 -> fall through. */
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

  int ret = notification_db_init();
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  EXPECT_TRUE(saw_create_table)
      << "Slow path must issue CREATE TABLE IF NOT EXISTS";
  EXPECT_TRUE(saw_integrity_check)
      << "Slow path must run PRAGMA integrity_check";
}

/* ------------------------------------------------------------------------ *
 * 3. CorruptedDB_RecoveryPath
 *
 * Pre-conditions: fast-path falls through (version mismatch). Slow path
 *   issues CREATE TABLE OK, then sqlite3_exec("PRAGMA integrity_check")
 *   returns SQLITE_CORRUPT. Production code then calls
 *   __recover_corrupted_db: unlink + re-open CREATE|RW + replay
 *   CREATE TABLE.
 * Expectation:  sqlite3_open_v2 invoked >= 2 times (slow-path + recovery
 *   re-open). At least one CREATE TABLE issued during recovery.
 * ------------------------------------------------------------------------ */
TEST_F(DbInitFastPathTest, CorruptedDB_RecoveryPath) {
  int open_calls = 0;
  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_open_v2(_, _, _, _))
      .Times(AnyNumber())
      .WillRepeatedly(
          Invoke([&](const char* path, sqlite3** out, int flags,
                     const char* vfs) -> int {
            ++open_calls;
            return FakeOpenOk(path, out, flags, vfs);
          }));

  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_prepare_v2(_, _, _, _, _))
      .WillRepeatedly(Invoke(FakePrepareOk));

  /* Fast-path version-check returns old version -> fall-through. */
  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_step(_))
      .WillRepeatedly(Return(SQLITE_ROW));

  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_column_int(_, 0))
      .WillRepeatedly(Return(kDbVersion - 1));

  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_finalize(_))
      .WillRepeatedly(Return(SQLITE_OK));

  int create_table_count = 0;
  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_exec(_, _, _, _, _))
      .Times(AnyNumber())
      .WillRepeatedly(Invoke([&](sqlite3*, const char* sql,
                                 int (*)(void*, int, char**, char**), void*,
                                 char** errmsg) -> int {
        if (errmsg) *errmsg = nullptr;
        if (!sql) return SQLITE_OK;
        if (strstr(sql, "integrity_check")) {
          /* Trigger recovery branch. */
          return SQLITE_CORRUPT;
        }
        if (strstr(sql, "CREATE TABLE")) ++create_table_count;
        return SQLITE_OK;
      }));

  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_close(_))
      .Times(AnyNumber())
      .WillRepeatedly(Return(SQLITE_OK));

  int ret = notification_db_init();
  /* Recovery path returns NOTIFICATION_ERROR_NONE on success. */
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  EXPECT_GE(open_calls, 2)
      << "Recovery path must re-open the DB after unlink";
  EXPECT_GE(create_table_count, 2)
      << "Recovery path replays CREATE TABLE after unlink";
}

/* ------------------------------------------------------------------------ *
 * 4. FirstBoot_FullInit
 *
 * Pre-conditions: DBPATH absent (access -> -1 ENOENT). Fast-path branch is
 *   skipped entirely; slow-path runs CREATE|RW open + CREATE TABLE +
 *   PRAGMA integrity_check.
 * Expectation:  CREATE TABLE and integrity_check both observed.
 * ------------------------------------------------------------------------ */
TEST_F(DbInitFastPathTest, FirstBoot_FullInit) {
  /* Simulate "file missing" for the access(DBPATH) probe. */
  SetNotificationDbAccessOverride(ENOENT);

  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_open_v2(_, _, _, _))
      .WillRepeatedly(Invoke(FakeOpenOk));

  /* The fast-path version probe never runs because access() failed first;
   * still allow prepare/step/finalize to be defaulted by NiceMock if the
   * slow path would happen to use them later. */
  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_finalize(_))
      .Times(AnyNumber())
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

  int ret = notification_db_init();
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  EXPECT_TRUE(saw_create_table)
      << "First-boot path must issue CREATE TABLE";
  EXPECT_TRUE(saw_integrity_check)
      << "First-boot path must run PRAGMA integrity_check";
}
