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
 * N1-T6: SqliteStmt RAII + StmtCache (LRU 16) unit tests.
 *
 * Test matrix:
 *   1. SqliteStmt_FinalizesOnScopeExit        - RAII finalize on success
 *   2. SqliteStmt_ThrowsOnPrepareFailure      - std::runtime_error thrown
 *   3. PreparedStmt_CacheHitRate              - 10 loops → 1 prepare + 9 hits
 *   4. StmtCache_Eviction                     - 17 inserts → LRU is finalized
 *   5. StmtCache_ResetBindingsBetweenReuses   - reset+clear_bindings fired
 *
 * All tests use the SqliteMock so nothing ever touches a real sqlite file.
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <sqlite3.h>

#include <cstdint>
#include <memory>
#include <string>

#include "mock/test_fixture.hh"
#include "mock/sqlite_mock.hh"

#include "notification-ex/sqlite_stmt.h"
#include "notification-ex/stmt_cache.h"

using ::testing::_;
using ::testing::AtLeast;
using ::testing::DoAll;
using ::testing::Invoke;
using ::testing::Return;
using ::testing::SaveArg;
using ::testing::SetArgPointee;
using ::testing::StrEq;

namespace {

/*
 * Each distinct sentinel below acts as the "address" of a fake sqlite
 * object the prepare mock hands out. Unit tests never dereference these;
 * only the mock SqliteMock::sqlite3_* overrides do anything with them.
 */
int g_db_sentinel = 0;

sqlite3* FakeDb() { return reinterpret_cast<sqlite3*>(&g_db_sentinel); }

/*
 * Unique stmt-pointer allocator. Every Acquire miss gets a different
 * address so the cache's internal list/map is exercised end-to-end.
 *
 * NOTE: we lean on the fact that SqliteMock finalize is a mock
 * (EXPECT_CALL can .Return(SQLITE_OK)) to free them virtually.
 */
class StmtAllocator {
 public:
  sqlite3_stmt* Next() {
    /* The sentinel array grows bounded per test, so a static array per
     * test body is safe. */
    auto p = std::make_unique<int>(nextid_++);
    sqlite3_stmt* out = reinterpret_cast<sqlite3_stmt*>(p.get());
    owned_.emplace_back(std::move(p));
    return out;
  }
  size_t Count() const { return owned_.size(); }
 private:
  int nextid_ = 1;
  std::vector<std::unique_ptr<int>> owned_;
};

int FakePrepareOk(sqlite3* /*db*/, const char* /*sql*/, int /*nbyte*/,
                  sqlite3_stmt** out_stmt, const char** /*tail*/) {
  /* Each call returns a fresh sentinel; never reused across calls. */
  static int counter = 1;
  /* Embed the counter into a static buffer to guarantee distinct
   * addresses without dynamic allocation per call. */
  static int sentinels[256] = {0};
  int idx = counter++ % 256;
  sentinels[idx] = counter;
  *out_stmt = reinterpret_cast<sqlite3_stmt*>(&sentinels[idx]);
  return SQLITE_OK;
}

int FakePrepareFail(sqlite3* /*db*/, const char* /*sql*/, int /*nbyte*/,
                    sqlite3_stmt** out_stmt, const char** /*tail*/) {
  *out_stmt = nullptr;
  return SQLITE_ERROR;
}

class Mocks : virtual public ::testing::NiceMock<SqliteMock> {};

}  // namespace

class StmtCacheTest : public TestFixture {
 public:
  StmtCacheTest() : TestFixture(std::make_unique<::Mocks>()) {}
};

/* (1) SqliteStmt RAII: finalize fires exactly once on scope exit. */
TEST_F(StmtCacheTest, SqliteStmt_FinalizesOnScopeExit) {
  auto& sqlite = TestFixture::GetMock<SqliteMock>();

  EXPECT_CALL(sqlite, sqlite3_prepare_v2(_, _, _, _, _))
      .Times(1)
      .WillOnce(Invoke(FakePrepareOk));
  EXPECT_CALL(sqlite, sqlite3_finalize(_))
      .Times(1)
      .WillOnce(Return(SQLITE_OK));

  {
    notification::SqliteStmt stmt(FakeDb(), "SELECT 1");
    ASSERT_TRUE(static_cast<bool>(stmt));
    ASSERT_NE(stmt.Get(), nullptr);
  }  /* dtor → sqlite3_finalize. */
}

/* (2) SqliteStmt throws on prepare failure and does NOT call finalize. */
TEST_F(StmtCacheTest, SqliteStmt_ThrowsOnPrepareFailure) {
  auto& sqlite = TestFixture::GetMock<SqliteMock>();

  EXPECT_CALL(sqlite, sqlite3_prepare_v2(_, _, _, _, _))
      .Times(1)
      .WillOnce(Invoke(FakePrepareFail));
  EXPECT_CALL(sqlite, sqlite3_finalize(_))
      .Times(0);

  bool threw = false;
  try {
    notification::SqliteStmt stmt(FakeDb(), "SELECT 1");
    FAIL() << "expected std::runtime_error";
  } catch (const std::runtime_error&) {
    threw = true;
  }
  EXPECT_TRUE(threw);
}

/* (3) StmtCache hit rate: 10 calls with the same SQL → 1 prepare, 9 hits.
 * Each hit resets bindings (sqlite3_reset + sqlite3_clear_bindings).
 * On destruction, the single cached stmt is finalized exactly once. */
TEST_F(StmtCacheTest, PreparedStmt_CacheHitRate) {
  auto& sqlite = TestFixture::GetMock<SqliteMock>();

  int prepare_calls = 0;
  int reset_calls = 0;
  int clear_bind_calls = 0;
  int finalize_calls = 0;

  sqlite3_stmt* kStmt = reinterpret_cast<sqlite3_stmt*>(0xabcd1234);

  EXPECT_CALL(sqlite, sqlite3_prepare_v2(_, _, _, _, _))
      .WillRepeatedly(Invoke([&](sqlite3*, const char*, int,
                                 sqlite3_stmt** out, const char**) {
        ++prepare_calls;
        *out = kStmt;
        return SQLITE_OK;
      }));
  EXPECT_CALL(sqlite, sqlite3_reset(_))
      .WillRepeatedly(Invoke([&](sqlite3_stmt*) { ++reset_calls; return SQLITE_OK; }));
  EXPECT_CALL(sqlite, sqlite3_clear_bindings(_))
      .WillRepeatedly(Invoke([&](sqlite3_stmt*) {
        ++clear_bind_calls; return SQLITE_OK;
      }));
  EXPECT_CALL(sqlite, sqlite3_finalize(_))
      .WillRepeatedly(Invoke([&](sqlite3_stmt*) {
        ++finalize_calls; return SQLITE_OK;
      }));

  {
    notification::StmtCache cache(16);
    for (int i = 0; i < 10; ++i) {
      auto handle = cache.Acquire(FakeDb(), "SELECT x FROM t WHERE a = ?");
      ASSERT_TRUE(static_cast<bool>(handle));
      ASSERT_EQ(handle.Get(), kStmt);
    }  /* handle.dtor → cache re-inserts, no finalize */

    /* Exactly one live cached entry at this point. */
    EXPECT_EQ(cache.Size(), 1u);
  }  /* cache.dtor → finalize the single cached stmt. */

  EXPECT_EQ(prepare_calls, 1);
  /* Reset + clear_bindings fire on each HIT, i.e. 9 of the 10 calls. */
  EXPECT_EQ(reset_calls, 9);
  EXPECT_EQ(clear_bind_calls, 9);
  EXPECT_EQ(finalize_calls, 1);
}

/* (4) LRU eviction: 17 distinct SQL strings into a size-16 cache → the
 * least-recently-used (the first one inserted) must be finalized. */
TEST_F(StmtCacheTest, StmtCache_Eviction) {
  auto& sqlite = TestFixture::GetMock<SqliteMock>();

  /* Hand out 17 unique fake stmt pointers across prepare calls. */
  std::vector<int> sentinels(17, 0);
  int prepare_idx = 0;
  int prepare_calls = 0;
  int finalize_calls = 0;
  sqlite3_stmt* first_stmt = nullptr;
  sqlite3_stmt* last_finalized = nullptr;

  EXPECT_CALL(sqlite, sqlite3_prepare_v2(_, _, _, _, _))
      .WillRepeatedly(Invoke([&](sqlite3*, const char*, int,
                                 sqlite3_stmt** out, const char**) {
        ++prepare_calls;
        int& slot = sentinels[prepare_idx++];
        slot = prepare_calls;
        *out = reinterpret_cast<sqlite3_stmt*>(&slot);
        return SQLITE_OK;
      }));
  EXPECT_CALL(sqlite, sqlite3_reset(_))
      .WillRepeatedly(Return(SQLITE_OK));
  EXPECT_CALL(sqlite, sqlite3_clear_bindings(_))
      .WillRepeatedly(Return(SQLITE_OK));
  EXPECT_CALL(sqlite, sqlite3_finalize(_))
      .WillRepeatedly(Invoke([&](sqlite3_stmt* s) {
        ++finalize_calls;
        last_finalized = s;
        return SQLITE_OK;
      }));

  notification::StmtCache cache(16);

  /* Insert 16 - fill the cache; none evicted yet. */
  for (int i = 0; i < 16; ++i) {
    std::string sql = "SELECT " + std::to_string(i);
    auto h = cache.Acquire(FakeDb(), sql);
    ASSERT_TRUE(static_cast<bool>(h));
    if (i == 0)
      first_stmt = h.Get();
  }
  EXPECT_EQ(prepare_calls, 16);
  EXPECT_EQ(finalize_calls, 0);
  EXPECT_EQ(cache.Size(), 16u);

  /* 17th insert forces eviction of the least-recently-used (index 0). */
  {
    auto h17 = cache.Acquire(FakeDb(), "SELECT 16");
    ASSERT_TRUE(static_cast<bool>(h17));
  }

  EXPECT_EQ(prepare_calls, 17);
  EXPECT_EQ(finalize_calls, 1);
  EXPECT_EQ(last_finalized, first_stmt)
      << "LRU eviction must finalize the oldest (first-inserted) stmt";
  EXPECT_EQ(cache.Size(), 16u);

  /* Cache destruction finalizes the remaining 16. */
  cache.Clear();
  EXPECT_EQ(finalize_calls, 17);
}

/* (5) Reset + clear_bindings fire between reuses. Also guarantees the
 * single cached stmt is returned again on the second Acquire of the same
 * SQL (i.e. a hit, not a re-prepare). */
TEST_F(StmtCacheTest, StmtCache_ResetBindingsBetweenReuses) {
  auto& sqlite = TestFixture::GetMock<SqliteMock>();

  int prepare_calls = 0;
  int reset_calls = 0;
  int clear_bind_calls = 0;
  int finalize_calls = 0;

  sqlite3_stmt* kStmt = reinterpret_cast<sqlite3_stmt*>(0x1001);

  EXPECT_CALL(sqlite, sqlite3_prepare_v2(_, _, _, _, _))
      .WillRepeatedly(Invoke([&](sqlite3*, const char* sql, int,
                                 sqlite3_stmt** out, const char**) {
        EXPECT_STREQ(sql, "INSERT INTO t VALUES (?, ?)");
        ++prepare_calls;
        *out = kStmt;
        return SQLITE_OK;
      }));
  EXPECT_CALL(sqlite, sqlite3_reset(kStmt))
      .WillRepeatedly(Invoke([&](sqlite3_stmt*) {
        ++reset_calls; return SQLITE_OK;
      }));
  EXPECT_CALL(sqlite, sqlite3_clear_bindings(kStmt))
      .WillRepeatedly(Invoke([&](sqlite3_stmt*) {
        ++clear_bind_calls; return SQLITE_OK;
      }));
  EXPECT_CALL(sqlite, sqlite3_finalize(_))
      .WillRepeatedly(Invoke([&](sqlite3_stmt*) {
        ++finalize_calls; return SQLITE_OK;
      }));

  notification::StmtCache cache(16);
  const char* kSql = "INSERT INTO t VALUES (?, ?)";

  /* First Acquire: miss → prepare, no reset/clear_bindings. */
  {
    auto h1 = cache.Acquire(FakeDb(), kSql);
    ASSERT_TRUE(static_cast<bool>(h1));
    EXPECT_EQ(h1.Get(), kStmt);
  }
  EXPECT_EQ(prepare_calls, 1);
  EXPECT_EQ(reset_calls, 0);
  EXPECT_EQ(clear_bind_calls, 0);
  EXPECT_EQ(finalize_calls, 0);

  /* Second Acquire on same SQL: hit → no prepare, reset + clear_bindings. */
  {
    auto h2 = cache.Acquire(FakeDb(), kSql);
    ASSERT_TRUE(static_cast<bool>(h2));
    EXPECT_EQ(h2.Get(), kStmt);
  }
  EXPECT_EQ(prepare_calls, 1);
  EXPECT_EQ(reset_calls, 1);
  EXPECT_EQ(clear_bind_calls, 1);
  EXPECT_EQ(finalize_calls, 0);

  /* Cache destruction finalizes the single cached stmt. */
  cache.Clear();
  EXPECT_EQ(finalize_calls, 1);
}
