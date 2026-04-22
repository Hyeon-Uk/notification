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
 */

/*
 * N1-T3: DBManager connection-pool unit tests.
 *
 * The DBManager now keeps a single long-lived sqlite3 connection for the
 * lifetime of the process, protected by a recursive_mutex and reopened on
 * demand when a corruption event invalidates the cache. These tests pin
 * that contract using the SqliteMock (which counts extern "C"
 * sqlite3_open_v2 / sqlite3_close_v2 entries):
 *
 *   1. PoolReuse_SingleConnectionUsed     - N calls to a hot-path DB
 *                                           method issue exactly 1
 *                                           sqlite3_open_v2.
 *   2. PoolReuse_ConnectionClosedOnReset  - sqlite3_close_v2 fires when
 *                                           the pool cache is dropped
 *                                           (Invalidate / test-only reset).
 *   3. PoolReuse_RecoversOnOpenFailure    - a failing first open returns
 *                                           cleanly and the next call
 *                                           retries the open instead of
 *                                           caching the failure.
 *
 * These tests exercise DBManager::GetCount(app_id, channel, uid, *count)
 * because it is the smallest hot-path method: one prepare, one step, one
 * column_int, one finalize per call.
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <sqlite3.h>

#include <cstdlib>
#include <cstdint>
#include <memory>

#include "mock/test_fixture.hh"
#include "mock/sqlite_mock.hh"

#include "notification-ex/db_manager.h"
#include "notification-ex/idb_manager.h"

using ::testing::_;
using ::testing::DoAll;
using ::testing::Invoke;
using ::testing::Return;
using ::testing::SetArgPointee;

namespace {

/*
 * Fake a successful open by returning a small heap-allocated sentinel
 * pointer (the real library would return an opaque sqlite3* that we
 * never dereference because every subsequent sqlite3_* call is also
 * mocked).
 */
int FakeOpenSuccess(const char* /*path*/, sqlite3** out_db,
                    int /*flags*/, const char* /*vfs*/) {
  /* The mocked sqlite3_prepare_v2 ignores this pointer. Allocate a
   * dummy so the pool cache has a unique shared_ptr. */
  static int sentinel;
  *out_db = reinterpret_cast<sqlite3*>(&sentinel);
  return SQLITE_OK;
}

int FakeOpenCorrupt(const char* /*path*/, sqlite3** out_db,
                    int /*flags*/, const char* /*vfs*/) {
  *out_db = nullptr;
  return SQLITE_CORRUPT;
}

int FakePrepareOk(sqlite3* /*db*/, const char* /*sql*/, int /*nbyte*/,
                  sqlite3_stmt** out_stmt, const char** /*tail*/) {
  static int stmt_sentinel;
  *out_stmt = reinterpret_cast<sqlite3_stmt*>(&stmt_sentinel);
  return SQLITE_OK;
}

class Mocks :
    virtual public ::testing::NiceMock<SqliteMock> {};

}  // namespace

class DbPoolTest : public TestFixture {
 public:
  DbPoolTest() : TestFixture(std::make_unique<::Mocks>()) {}

  void SetUp() override {
    /* Always start from a cold pool so open counts are deterministic. */
    ::notification::detail::ResetDefaultConnectionForTest();
  }

  void TearDown() override {
    /* Drop any cached connection produced by this test so later tests
     * in the same binary see a fresh state. */
    ::notification::detail::ResetDefaultConnectionForTest();
  }
};

TEST_F(DbPoolTest, PoolReuse_SingleConnectionUsed) {
  auto& sqlite = TestFixture::GetMock<SqliteMock>();

  /* Pool contract: exactly one open for N hot-path calls. */
  EXPECT_CALL(sqlite, sqlite3_open_v2(_, _, _, _))
      .Times(1)
      .WillOnce(Invoke(FakeOpenSuccess));

  EXPECT_CALL(sqlite, sqlite3_prepare_v2(_, _, _, _, _))
      .WillRepeatedly(Invoke(FakePrepareOk));
  EXPECT_CALL(sqlite, sqlite3_step(_))
      .WillRepeatedly(Return(SQLITE_ROW));
  EXPECT_CALL(sqlite, sqlite3_column_int(_, _))
      .WillRepeatedly(Return(0));
  EXPECT_CALL(sqlite, sqlite3_finalize(_))
      .WillRepeatedly(Return(SQLITE_OK));
  EXPECT_CALL(sqlite, sqlite3_free(_))
      .WillRepeatedly(Return());

  for (int i = 0; i < 10; ++i) {
    int count = -1;
    int rc = ::notification::DBManager::GetCount(
        "org.tizen.pooltest", std::string(), 5001, &count);
    ASSERT_EQ(rc, 0 /* ERROR_NONE */);
  }
}

TEST_F(DbPoolTest, PoolReuse_ConnectionClosedOnReset) {
  auto& sqlite = TestFixture::GetMock<SqliteMock>();

  EXPECT_CALL(sqlite, sqlite3_open_v2(_, _, _, _))
      .Times(1)
      .WillOnce(Invoke(FakeOpenSuccess));
  EXPECT_CALL(sqlite, sqlite3_prepare_v2(_, _, _, _, _))
      .WillRepeatedly(Invoke(FakePrepareOk));
  EXPECT_CALL(sqlite, sqlite3_step(_))
      .WillRepeatedly(Return(SQLITE_ROW));
  EXPECT_CALL(sqlite, sqlite3_column_int(_, _))
      .WillRepeatedly(Return(0));
  EXPECT_CALL(sqlite, sqlite3_finalize(_))
      .WillRepeatedly(Return(SQLITE_OK));
  EXPECT_CALL(sqlite, sqlite3_free(_))
      .WillRepeatedly(Return());

  /* The cached shared_ptr's deleter is sqlite3_close_v2. */
  EXPECT_CALL(sqlite, sqlite3_close_v2(_))
      .Times(1)
      .WillOnce(Return(SQLITE_OK));

  int count = -1;
  int rc = ::notification::DBManager::GetCount(
      "org.tizen.pooltest", std::string(), 5001, &count);
  ASSERT_EQ(rc, 0);

  /* Dropping the pool must close the underlying connection exactly once. */
  ::notification::detail::ResetDefaultConnectionForTest();
}

TEST_F(DbPoolTest, PoolReuse_RecoversOnOpenFailure) {
  auto& sqlite = TestFixture::GetMock<SqliteMock>();

  /* Sequence: first open fails with SQLITE_CORRUPT (no cache stored),
   * second open succeeds and should then be reused. */
  EXPECT_CALL(sqlite, sqlite3_open_v2(_, _, _, _))
      .WillOnce(Invoke(FakeOpenCorrupt))
      .WillOnce(Invoke(FakeOpenSuccess));

  EXPECT_CALL(sqlite, sqlite3_prepare_v2(_, _, _, _, _))
      .WillRepeatedly(Invoke(FakePrepareOk));
  EXPECT_CALL(sqlite, sqlite3_step(_))
      .WillRepeatedly(Return(SQLITE_ROW));
  EXPECT_CALL(sqlite, sqlite3_column_int(_, _))
      .WillRepeatedly(Return(0));
  EXPECT_CALL(sqlite, sqlite3_finalize(_))
      .WillRepeatedly(Return(SQLITE_OK));
  EXPECT_CALL(sqlite, sqlite3_free(_))
      .WillRepeatedly(Return());
  EXPECT_CALL(sqlite, sqlite3_close_v2(_))
      .WillRepeatedly(Return(SQLITE_OK));

  int count = -1;
  int rc = ::notification::DBManager::GetCount(
      "org.tizen.pooltest", std::string(), 5001, &count);
  /* First call: open returned SQLITE_CORRUPT -> ERROR_FROM_DB. */
  ASSERT_NE(rc, 0);

  /* Second call must retry sqlite3_open_v2 (not cache the failure). */
  rc = ::notification::DBManager::GetCount(
      "org.tizen.pooltest", std::string(), 5001, &count);
  ASSERT_EQ(rc, 0);
}
