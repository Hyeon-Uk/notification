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
 * Phase 1.5-C seed for the integration target.
 *
 * Real-libsqlite3 round-trip against a `:memory:` database.  Covers
 * notification_db_exec, the only public DB helper that takes a
 * caller-owned sqlite3* (and is therefore reachable without
 * notification_db_init's hardcoded DBPATH).  Broader integration of
 * notification_noti_insert / load / delete is deferred to 1.5-E,
 * pending DBPATH-redirection plumbing via tzplatform_config_mock.
 */

#include <sqlite3.h>

#include <gtest/gtest.h>

extern "C" {
#include <notification_db.h>
#include <notification_error.h>
}

namespace {

class DbIntegrationTest : public ::testing::Test {
 protected:
  void SetUp() override {
    int rc = sqlite3_open_v2(":memory:", &db_,
        SQLITE_OPEN_CREATE | SQLITE_OPEN_READWRITE, nullptr);
    ASSERT_EQ(rc, SQLITE_OK)
        << "sqlite3_open_v2(:memory:) failed: " << sqlite3_errstr(rc);
    ASSERT_NE(db_, nullptr);
  }
  void TearDown() override {
    if (db_) {
      sqlite3_close(db_);
      db_ = nullptr;
    }
  }
  sqlite3* db_ = nullptr;
};

TEST_F(DbIntegrationTest, OpensInMemoryDbAndAppliesMinimalSchema) {
  // Apply a minimal schema directly (the production schema macro
  // CREATE_NOTIFICATION_TABLE lives in notification_db_query.h, not
  // installed; we keep the seed self-contained with a small table).
  char* errmsg = nullptr;
  int rc = sqlite3_exec(db_,
      "CREATE TABLE seed_table (id INTEGER PRIMARY KEY, name TEXT);",
      nullptr, nullptr, &errmsg);
  if (errmsg) sqlite3_free(errmsg);
  ASSERT_EQ(rc, SQLITE_OK);
}

TEST_F(DbIntegrationTest, ExecRoundTripPersistsAndCountsChanges) {
  // Schema first.
  char* errmsg = nullptr;
  ASSERT_EQ(sqlite3_exec(db_,
      "CREATE TABLE seed_table (id INTEGER PRIMARY KEY, name TEXT);",
      nullptr, nullptr, &errmsg), SQLITE_OK);
  if (errmsg) { sqlite3_free(errmsg); errmsg = nullptr; }

  // Production helper, real sqlite under it.
  int changes = -1;
  int ret = notification_db_exec(db_,
      "INSERT INTO seed_table (name) VALUES ('alpha');",
      &changes);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(changes, 1);

  // Confirm row is actually persisted in the in-memory db.
  sqlite3_stmt* stmt = nullptr;
  ASSERT_EQ(sqlite3_prepare_v2(db_,
      "SELECT name FROM seed_table WHERE id = 1;", -1, &stmt, nullptr),
      SQLITE_OK);
  ASSERT_EQ(sqlite3_step(stmt), SQLITE_ROW);
  EXPECT_STREQ(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)),
      "alpha");
  sqlite3_finalize(stmt);
}

TEST_F(DbIntegrationTest, ExecOnInvalidQueryReturnsFromDbError) {
  int changes = 0;
  int ret = notification_db_exec(db_,
      "THIS IS NOT VALID SQL;", &changes);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_FROM_DB);
}

}  // namespace
