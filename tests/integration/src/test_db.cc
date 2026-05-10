#include <gtest/gtest.h>
#include <sqlite3.h>

#include <fstream>
#include <string>

#include <notification_db.h>
#include <notification_error.h>

#include "integ_harness.hh"

namespace {

class IntegDb : public ::testing::Test {
 protected:
  integ::TmpDb tmp_;
};

TEST_F(IntegDb, P_DB_1_init_creates_db_file_under_seam_path) {
  int rc = notification_db_init();
  EXPECT_EQ(rc, NOTIFICATION_ERROR_NONE);

  const char* p = getenv("NOTIFICATION_INTEGTEST_DB_PATH");
  ASSERT_NE(p, nullptr);
  EXPECT_EQ(access(p, R_OK | W_OK), 0);
}

TEST_F(IntegDb, P_DB_2_open_returns_handle_after_init) {
  ASSERT_EQ(notification_db_init(), NOTIFICATION_ERROR_NONE);
  sqlite3* db = notification_db_open();
  ASSERT_NE(db, nullptr);
  EXPECT_EQ(notification_db_close(&db), NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(db, nullptr);
}

TEST_F(IntegDb, P_DB_3_exec_ddl_returns_none_on_done) {
  // notification_db_exec is for non-row-returning statements:
  // it only accepts SQLITE_OK or SQLITE_DONE from sqlite3_step.
  // A SELECT returns SQLITE_ROW and is rejected with FROM_DB
  // (today's contract -- pinned by N-DB-5 below). DDL returns
  // SQLITE_DONE so it's the correct happy-path probe.
  ASSERT_EQ(notification_db_init(), NOTIFICATION_ERROR_NONE);
  sqlite3* db = notification_db_open();
  ASSERT_NE(db, nullptr);
  int changes = -1;
  int rc = notification_db_exec(
      db, "CREATE TABLE IF NOT EXISTS integ_db_exec_probe (id INT)",
      &changes);
  EXPECT_EQ(rc, NOTIFICATION_ERROR_NONE);
  notification_db_close(&db);
}

TEST_F(IntegDb, P_DB_4_repeat_init_is_idempotent) {
  EXPECT_EQ(notification_db_init(), NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(notification_db_init(), NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(notification_db_init(), NOTIFICATION_ERROR_NONE);
}

TEST_F(IntegDb, P_DB_5_column_text_decodes_a_row) {
  ASSERT_EQ(notification_db_init(), NOTIFICATION_ERROR_NONE);
  sqlite3* db = notification_db_open();
  ASSERT_NE(db, nullptr);
  sqlite3_stmt* stmt = nullptr;
  ASSERT_EQ(sqlite3_prepare_v2(db, "SELECT 'hello'", -1, &stmt, nullptr),
            SQLITE_OK);
  ASSERT_EQ(sqlite3_step(stmt), SQLITE_ROW);
  char* text = notification_db_column_text(stmt, 0);
  ASSERT_NE(text, nullptr);
  EXPECT_STREQ(text, "hello");
  free(text);
  sqlite3_finalize(stmt);
  notification_db_close(&db);
}

TEST_F(IntegDb, N_DB_1_close_null_db_returns_invalid_parameter) {
  EXPECT_EQ(notification_db_close(nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  sqlite3* db = nullptr;
  EXPECT_EQ(notification_db_close(&db),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(IntegDb, N_DB_2_exec_null_db_returns_invalid_parameter) {
  EXPECT_EQ(notification_db_exec(nullptr, "SELECT 1", nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(IntegDb, N_DB_3_exec_null_query_returns_invalid_parameter) {
  ASSERT_EQ(notification_db_init(), NOTIFICATION_ERROR_NONE);
  sqlite3* db = notification_db_open();
  ASSERT_NE(db, nullptr);
  EXPECT_EQ(notification_db_exec(db, nullptr, nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  notification_db_close(&db);
}

TEST_F(IntegDb, N_DB_4_exec_malformed_query_returns_from_db) {
  ASSERT_EQ(notification_db_init(), NOTIFICATION_ERROR_NONE);
  sqlite3* db = notification_db_open();
  ASSERT_NE(db, nullptr);
  EXPECT_EQ(notification_db_exec(db, "NOT A SQL STATEMENT", nullptr),
            NOTIFICATION_ERROR_FROM_DB);
  notification_db_close(&db);
}

TEST_F(IntegDb, N_DB_5_exec_select_yielding_rows_returns_from_db) {
  // Lock-in finding: notification_db_exec only accepts statements
  // whose first sqlite3_step returns SQLITE_OK or SQLITE_DONE.
  // SELECT 1 returns SQLITE_ROW on the first step, which the
  // current impl maps to NOTIFICATION_ERROR_FROM_DB. Pin this
  // before any Phase 1 migration that rewrites the helper.
  ASSERT_EQ(notification_db_init(), NOTIFICATION_ERROR_NONE);
  sqlite3* db = notification_db_open();
  ASSERT_NE(db, nullptr);
  EXPECT_EQ(notification_db_exec(db, "SELECT 1", nullptr),
            NOTIFICATION_ERROR_FROM_DB);
  notification_db_close(&db);
}

TEST_F(IntegDb, E_DB_1_open_without_init_returns_null) {
  // Fresh tmpdir; no init() called -> file doesn't exist -> open
  // must fail-safe with NULL.
  sqlite3* db = notification_db_open();
  EXPECT_EQ(db, nullptr);
}

TEST_F(IntegDb, E_DB_2_column_text_empty_string_returns_null) {
  ASSERT_EQ(notification_db_init(), NOTIFICATION_ERROR_NONE);
  sqlite3* db = notification_db_open();
  ASSERT_NE(db, nullptr);
  sqlite3_stmt* stmt = nullptr;
  ASSERT_EQ(sqlite3_prepare_v2(db, "SELECT ''", -1, &stmt, nullptr),
            SQLITE_OK);
  ASSERT_EQ(sqlite3_step(stmt), SQLITE_ROW);
  EXPECT_EQ(notification_db_column_text(stmt, 0), nullptr);
  sqlite3_finalize(stmt);
  notification_db_close(&db);
}

TEST_F(IntegDb, E_DB_3_column_bundle_empty_returns_null) {
  ASSERT_EQ(notification_db_init(), NOTIFICATION_ERROR_NONE);
  sqlite3* db = notification_db_open();
  ASSERT_NE(db, nullptr);
  sqlite3_stmt* stmt = nullptr;
  ASSERT_EQ(sqlite3_prepare_v2(db, "SELECT ''", -1, &stmt, nullptr),
            SQLITE_OK);
  ASSERT_EQ(sqlite3_step(stmt), SQLITE_ROW);
  EXPECT_EQ(notification_db_column_bundle(stmt, 0), nullptr);
  sqlite3_finalize(stmt);
  notification_db_close(&db);
}

TEST_F(IntegDb, C_DB_1_corrupt_file_recovers_via_init) {
  // Plant a non-sqlite blob at the seam path, then call init.
  // The PRAGMA integrity_check path triggers __recover_corrupted_db
  // which unlinks + recreates. Lock-in: the recovery path returns
  // NOTIFICATION_ERROR_NONE on success (today's behavior).
  const char* p = getenv("NOTIFICATION_INTEGTEST_DB_PATH");
  ASSERT_NE(p, nullptr);
  {
    std::ofstream f(p, std::ios::binary | std::ios::trunc);
    f << "not a sqlite database, just garbage bytes";
  }
  int rc = notification_db_init();
  EXPECT_EQ(rc, NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(access(p, R_OK | W_OK), 0);
}

TEST_F(IntegDb, C_DB_2_close_double_call_second_returns_invalid_parameter) {
  ASSERT_EQ(notification_db_init(), NOTIFICATION_ERROR_NONE);
  sqlite3* db = notification_db_open();
  ASSERT_NE(db, nullptr);
  EXPECT_EQ(notification_db_close(&db), NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(db, nullptr);
  EXPECT_EQ(notification_db_close(&db),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(IntegDb, C_DB_3_seam_redirects_path) {
  // Confirms _notification_db_resolve_path() is reading the seam env
  // var: open() before init() returns NULL (file absent at the
  // tmpdir seam), then init() creates the file at exactly that path.
  const char* p = getenv("NOTIFICATION_INTEGTEST_DB_PATH");
  ASSERT_NE(p, nullptr);
  EXPECT_NE(access(p, F_OK), 0) << "tmpdir DB should not exist yet";

  ASSERT_EQ(notification_db_init(), NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(access(p, F_OK), 0) << "init must create at seam path";

  // The production default DBPATH must not have been touched -- but
  // we can only assert "seam path used" indirectly here, via the
  // file-presence at the tmpdir.
}

}  // namespace
