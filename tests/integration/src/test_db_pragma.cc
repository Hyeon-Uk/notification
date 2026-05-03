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
 * Phase 3 Half-B / Task 3.8 — pragma regression guard.
 *
 * The production schema bootstrap (CREATE_NOTIFICATION_TABLE in
 * src/notification/src/notification_db_query.h) sets
 * `PRAGMA journal_mode` and `PRAGMA synchronous` as part of the
 * initial sqlite3_exec.  Those values are durability-load-bearing
 * for the notification daemon: relaxing them changes how aggressively
 * sqlite fsyncs to disk on commit.
 *
 * This test pins the current values so any future change has to
 * land alongside an explicit update here, surfacing the durability
 * trade-off in code review.
 *
 * Current values (locked in):
 *   journal_mode = PERSIST   (rollback journal kept on disk)
 *   synchronous  = 2 (FULL)  (sqlite always fsyncs at commit)
 *
 * Modern alternative (proposed but NOT applied yet):
 *   journal_mode = WAL       (write-ahead log)
 *   synchronous  = 1 (NORMAL)
 * which is ~30-50% faster on writes but relaxes durability slightly
 * (a power-loss can lose the very last commit).  Decision pending
 * user signoff on the durability change.
 */

#include <sqlite3.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <string>

#include <gtest/gtest.h>

#include "notification_db_query.h"  // CREATE_NOTIFICATION_TABLE

namespace {

class DbPragmaTest : public ::testing::Test {
 protected:
  void SetUp() override {
    /* Use a file-backed temp db rather than `:memory:` because
     * sqlite silently forces journal_mode=memory on in-memory
     * databases regardless of the PRAGMA request -- which would
     * make the journal_mode regression guard misfire. */
    char tmpl[] = "/tmp/notification-pragma-XXXXXX";
    int fd = mkstemp(tmpl);
    ASSERT_GE(fd, 0) << "mkstemp failed";
    close(fd);
    db_path_ = tmpl;

    int rc = sqlite3_open_v2(db_path_.c_str(), &db_,
        SQLITE_OPEN_CREATE | SQLITE_OPEN_READWRITE, nullptr);
    ASSERT_EQ(rc, SQLITE_OK);
    ASSERT_NE(db_, nullptr);

    // Apply the production schema bootstrap once.  Captures pragmas
    // + table creation in a single sqlite3_exec call, just like
    // notification_db_init does.
    char* errmsg = nullptr;
    rc = sqlite3_exec(db_, CREATE_NOTIFICATION_TABLE, nullptr, nullptr,
                      &errmsg);
    if (errmsg) {
      ADD_FAILURE() << "sqlite3_exec(CREATE_NOTIFICATION_TABLE) failed: "
                    << errmsg;
      sqlite3_free(errmsg);
    }
    ASSERT_EQ(rc, SQLITE_OK);
  }
  void TearDown() override {
    if (db_) {
      sqlite3_close(db_);
      db_ = nullptr;
    }
    if (!db_path_.empty()) {
      unlink(db_path_.c_str());
      /* PERSIST journal also leaves a *-journal file on disk; clean it
       * too so the test leaves /tmp tidy. */
      std::string journal = db_path_ + "-journal";
      unlink(journal.c_str());
    }
  }

  // Read back a string-valued pragma (`PRAGMA journal_mode`) into a
  // gtest-comparable std::string.  Empty string on failure.
  std::string ReadStringPragma(const char* name) {
    sqlite3_stmt* stmt = nullptr;
    char buf[64] = {0,};
    snprintf(buf, sizeof(buf), "PRAGMA %s", name);
    if (sqlite3_prepare_v2(db_, buf, -1, &stmt, nullptr) != SQLITE_OK)
      return {};
    std::string result;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
      const unsigned char* col = sqlite3_column_text(stmt, 0);
      if (col) result = reinterpret_cast<const char*>(col);
    }
    sqlite3_finalize(stmt);
    return result;
  }

  // Read back an integer-valued pragma (`PRAGMA synchronous`) into
  // an int.  -1 on failure.
  int ReadIntPragma(const char* name) {
    sqlite3_stmt* stmt = nullptr;
    char buf[64] = {0,};
    snprintf(buf, sizeof(buf), "PRAGMA %s", name);
    if (sqlite3_prepare_v2(db_, buf, -1, &stmt, nullptr) != SQLITE_OK)
      return -1;
    int result = -1;
    if (sqlite3_step(stmt) == SQLITE_ROW)
      result = sqlite3_column_int(stmt, 0);
    sqlite3_finalize(stmt);
    return result;
  }

  sqlite3* db_ = nullptr;
  std::string db_path_;
};

TEST_F(DbPragmaTest, JournalModeIsPersist) {
  // Locked in by Half-B/3.8a (this commit).  If you're changing
  // this to "wal", land it together with the matching change in
  // notification_db_query.h::CREATE_NOTIFICATION_TABLE and confirm
  // the durability trade-off has been signed off.
  EXPECT_EQ(ReadStringPragma("journal_mode"), "persist");
}

TEST_F(DbPragmaTest, SynchronousIsFull) {
  // 0 = OFF, 1 = NORMAL, 2 = FULL, 3 = EXTRA.  Locked at FULL
  // (strongest durability).  See JournalModeIsPersist comment for
  // the change protocol.
  EXPECT_EQ(ReadIntPragma("synchronous"), 2);
}

TEST_F(DbPragmaTest, NotiListTableExistsAfterBootstrap) {
  // Sanity check that the schema bootstrap actually created the
  // expected production table -- confirms the test is exercising
  // the real CREATE_NOTIFICATION_TABLE macro and not silently
  // failing.
  sqlite3_stmt* stmt = nullptr;
  ASSERT_EQ(sqlite3_prepare_v2(db_,
      "SELECT name FROM sqlite_master WHERE type='table' AND name='noti_list';",
      -1, &stmt, nullptr), SQLITE_OK);
  ASSERT_EQ(sqlite3_step(stmt), SQLITE_ROW);
  EXPECT_STREQ(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)),
               "noti_list");
  sqlite3_finalize(stmt);
}

}  // namespace
