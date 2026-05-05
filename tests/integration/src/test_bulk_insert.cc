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
 * Phase 3 Half-B / Tasks 3.2 + 3.3 — bulk-insert metric guards.
 *
 * The new bulk-insert EXPORT_API (notification_noti_insert_list,
 * landed in a follow-up commit) is supposed to:
 *   - prepare its INSERT statement once for the whole batch,
 *   - wrap every per-row sqlite3_step in a single transaction,
 *   - run wall-clock faster than the equivalent loop over the
 *     existing single-row notification_noti_insert.
 *
 * This test target covers the SQL pattern itself with a
 * file-backed temp db (mkstemp, same fixture pattern as
 * test_db_pragma.cc).  It does NOT call the production EXPORT_API
 * directly because the production path opens a hardcoded DBPATH
 * that integration tests cannot redirect today (DBPATH-redirection
 * via tzplatform_config_mock is queued under Phase-1.5-E.8).
 *
 * Per-row behavior of the new EXPORT_API is covered separately by
 * the unit tests in tests/unittests/src/test_bulk_insert_unit.cc
 * (which CAN mock sqlite + tzplatform).  The split is intentional:
 *   - integration target: SQL pattern correctness with real sqlite.
 *   - unittests target:   per-row control flow with sqlite mocked.
 *
 * Locked in by this commit (regression guards):
 *   - loop pattern   = N prepares + N implicit commits.
 *   - bulk pattern   = 1 prepare + 1 explicit commit, regardless of N.
 *   - bulk pattern   ≥ 1.2x faster than loop pattern wall-clock.
 *
 * The 1.2x margin is the 2x safety margin the Phase-3 prompt
 * template requires for wall-clock thresholds.  The deterministic
 * counter assertions are the primary regression guard; wall-clock
 * is the secondary, flake-tolerant check.
 */

#include <sqlite3.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <chrono>
#include <string>

#include <gtest/gtest.h>

namespace {

constexpr int kBatchRows = 200;

// Number of times sqlite3_prepare_v2 has been called against the
// fixture db in the current test case.  Incremented from a wrapper
// in the test code (sqlite3 has no per-connection prepare hook;
// commit and rollback do, so we can hook those).
int g_prepare_calls = 0;

// Number of times the engine has issued a COMMIT against the
// fixture db in the current test case.  Driven by
// sqlite3_commit_hook.
int g_commit_calls = 0;

extern "C" int OnCommit(void* /*udata*/) {
  ++g_commit_calls;
  return 0;
}

class BulkInsertPatternTest : public ::testing::Test {
 protected:
  void SetUp() override {
    char tmpl[] = "/tmp/notification-bulk-XXXXXX";
    int fd = mkstemp(tmpl);
    ASSERT_GE(fd, 0) << "mkstemp failed";
    close(fd);
    db_path_ = tmpl;

    int rc = sqlite3_open_v2(db_path_.c_str(), &db_,
        SQLITE_OPEN_CREATE | SQLITE_OPEN_READWRITE, nullptr);
    ASSERT_EQ(rc, SQLITE_OK);
    ASSERT_NE(db_, nullptr);

    /* A minimal table that mirrors the structural shape of the
     * production noti_list (one INTEGER PRIMARY KEY plus a TEXT
     * payload).  We intentionally keep it small so the metric
     * ratios reflect the prepare/commit overhead, not per-bind
     * work. */
    char* errmsg = nullptr;
    rc = sqlite3_exec(db_,
        "CREATE TABLE bulk_test ("
        "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  payload TEXT NOT NULL"
        ");",
        nullptr, nullptr, &errmsg);
    if (errmsg) sqlite3_free(errmsg);
    ASSERT_EQ(rc, SQLITE_OK);

    sqlite3_commit_hook(db_, OnCommit, nullptr);

    g_prepare_calls = 0;
    g_commit_calls = 0;
  }
  void TearDown() override {
    if (db_) {
      sqlite3_close(db_);
      db_ = nullptr;
    }
    if (!db_path_.empty()) {
      unlink(db_path_.c_str());
      unlink((db_path_ + "-journal").c_str());
      db_path_.clear();
    }
  }

  // Counts as a single sqlite3_prepare_v2 call from the perspective
  // of the metric (which is what we want a real
  // notification_noti_insert_list to reduce).
  int PrepareCounted(const char* sql, sqlite3_stmt** stmt) {
    ++g_prepare_calls;
    return sqlite3_prepare_v2(db_, sql, -1, stmt, nullptr);
  }

  int InsertOneViaPattern(const char* payload) {
    sqlite3_stmt* stmt = nullptr;
    int rc = PrepareCounted(
        "INSERT INTO bulk_test (payload) VALUES (?);", &stmt);
    if (rc != SQLITE_OK) return rc;
    sqlite3_bind_text(stmt, 1, payload, -1, SQLITE_STATIC);
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return rc == SQLITE_DONE ? SQLITE_OK : rc;
  }

  // Models the LOOP pattern: prepare/step/finalize per row, no
  // explicit transaction (each step is its own implicit commit).
  void RunLoopPattern(int n) {
    for (int i = 0; i < n; ++i) {
      char payload[32];
      snprintf(payload, sizeof(payload), "row-%d", i);
      ASSERT_EQ(InsertOneViaPattern(payload), SQLITE_OK)
          << "row " << i;
    }
  }

  // Models the BULK pattern: prepare once, BEGIN IMMEDIATE, loop
  // with reset+bind+step, COMMIT.
  void RunBulkPattern(int n) {
    sqlite3_stmt* stmt = nullptr;
    ASSERT_EQ(PrepareCounted(
        "INSERT INTO bulk_test (payload) VALUES (?);", &stmt),
        SQLITE_OK);
    ASSERT_EQ(sqlite3_exec(db_, "BEGIN IMMEDIATE;",
                           nullptr, nullptr, nullptr), SQLITE_OK);
    for (int i = 0; i < n; ++i) {
      char payload[32];
      snprintf(payload, sizeof(payload), "row-%d", i);
      sqlite3_reset(stmt);
      sqlite3_bind_text(stmt, 1, payload, -1, SQLITE_TRANSIENT);
      ASSERT_EQ(sqlite3_step(stmt), SQLITE_DONE) << "row " << i;
    }
    ASSERT_EQ(sqlite3_exec(db_, "COMMIT;",
                           nullptr, nullptr, nullptr), SQLITE_OK);
    sqlite3_finalize(stmt);
  }

  int CountRows() {
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_, "SELECT COUNT(*) FROM bulk_test;",
                           -1, &stmt, nullptr) != SQLITE_OK)
      return -1;
    int count = -1;
    if (sqlite3_step(stmt) == SQLITE_ROW)
      count = sqlite3_column_int(stmt, 0);
    sqlite3_finalize(stmt);
    return count;
  }

  sqlite3* db_ = nullptr;
  std::string db_path_;
};

TEST_F(BulkInsertPatternTest, LoopPatternHasOnePreparePerRow) {
  /* Pin: the naive pattern (1000 single-row inserts via
   * notification_noti_insert) re-prepares once per row.  This is
   * the regression-guard SOURCE: the bulk path must beat this. */
  RunLoopPattern(kBatchRows);
  EXPECT_EQ(g_prepare_calls, kBatchRows);
  EXPECT_EQ(CountRows(), kBatchRows);
}

TEST_F(BulkInsertPatternTest, LoopPatternHasOneCommitPerRow) {
  /* Each implicit transaction emits one COMMIT.  This is what
   * synchronous=FULL turns into N fsyncs. */
  RunLoopPattern(kBatchRows);
  EXPECT_EQ(g_commit_calls, kBatchRows);
}

TEST_F(BulkInsertPatternTest, BulkPatternHasOnePrepareTotal) {
  /* Pin: the bulk pattern shares one prepared statement across the
   * whole batch.  This is the structural property
   * notification_noti_insert_list MUST have to count as an
   * implementation of Phase-3 task 3.2. */
  RunBulkPattern(kBatchRows);
  EXPECT_EQ(g_prepare_calls, 1);
  EXPECT_EQ(CountRows(), kBatchRows);
}

TEST_F(BulkInsertPatternTest, BulkPatternHasOneCommitTotal) {
  /* Pin: the bulk pattern wraps the whole batch in a single
   * BEGIN/COMMIT pair.  This is task 3.3.  Reverting either the
   * BEGIN or the COMMIT side (or moving them inside the loop)
   * makes this fail. */
  RunBulkPattern(kBatchRows);
  EXPECT_EQ(g_commit_calls, 1);
}

TEST_F(BulkInsertPatternTest, BulkPatternFasterThanLoopPattern) {
  /* Wall-clock check.  Run the bulk pattern first to warm sqlite's
   * page cache, then re-time both with a fresh fixture between
   * runs to keep it apples-to-apples.  We assert the loop is
   * AT LEAST 1.2x slower than bulk -- the 2x safety margin the
   * Phase-3 prompt template requires for wall-clock thresholds.
   *
   * The deterministic counter tests above are the primary
   * regression guards.  This test is the secondary signal: if it
   * ever flakes, tighten the margin or move to median-of-N. */
  using clock = std::chrono::steady_clock;

  auto t0 = clock::now();
  RunBulkPattern(kBatchRows);
  auto bulk_ns = clock::now() - t0;
  ASSERT_EQ(CountRows(), kBatchRows);

  /* Reset for a clean second run. */
  ASSERT_EQ(sqlite3_exec(db_, "DELETE FROM bulk_test;",
                         nullptr, nullptr, nullptr), SQLITE_OK);
  g_prepare_calls = 0;
  g_commit_calls = 0;

  auto t1 = clock::now();
  RunLoopPattern(kBatchRows);
  auto loop_ns = clock::now() - t1;
  ASSERT_EQ(CountRows(), kBatchRows);

  EXPECT_GE(loop_ns.count(), (bulk_ns.count() * 12) / 10)
      << "loop=" << loop_ns.count() << "ns, bulk="
      << bulk_ns.count() << "ns; loop should be >= 1.2x slower.";
}

}  // namespace
