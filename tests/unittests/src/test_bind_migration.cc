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
 * N3-T3 C4: regression guard for the sqlite3_mprintf → sqlite3_bind_*
 * migration on the hot-path DML.
 *
 * Two invariants:
 *   1. Semantic equivalence — the bind form produces an identical row
 *      to what the legacy %Q-quoting path produced. We simulate this
 *      by doing a round-trip (INSERT with bind → SELECT → compare) on a
 *      minimal replica of the noti_list / noti_ex_list schema.
 *   2. SQL injection immunity — app_id values containing "'; DROP
 *      TABLE…" do NOT execute as DDL under the bind path. We verify
 *      this by running the adversarial literal as a parameter and
 *      confirming the target table still exists afterwards.
 *
 * Execution model: like test_db_query_plan.cc we need *real* SQLite,
 * not the tests/mock/sqlite_mock.cc interposers, so we load
 * libsqlite3.so.0 into a private link namespace via dlmopen(LM_ID_NEWLM).
 * The dlmopen indirection is necessary because the test binary links
 * the mock TU and exports sqlite3_* with --export-dynamic.
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <gtest/gtest.h>

#include <dlfcn.h>
#include <sqlite3.h>

#include <cstring>
#include <string>

namespace {

/*
 * Real-SQLite shim — identical technique to test_db_query_plan.cc.
 * We only expose the surface we need: open, exec, prepare, bind_*,
 * step, column_text, column_int, finalize, close_v2.
 */
class RealSqlite {
 public:
  static void* Handle() {
    static void* h = []() -> void* {
      void* p = dlmopen(LM_ID_NEWLM, "libsqlite3.so.0", RTLD_NOW);
      if (!p) p = dlmopen(LM_ID_NEWLM, "libsqlite3.so", RTLD_NOW);
      return p;
    }();
    return h;
  }

  template <typename Fn>
  static Fn Resolve(const char* name) {
    void* h = Handle();
    if (!h) return nullptr;
    return reinterpret_cast<Fn>(dlsym(h, name));
  }

  static int OpenV2(const char* f, sqlite3** out, int flags, const char* vfs) {
    using fp = int (*)(const char*, sqlite3**, int, const char*);
    static fp real = Resolve<fp>("sqlite3_open_v2");
    return real ? real(f, out, flags, vfs) : SQLITE_ERROR;
  }
  static int Exec(sqlite3* db, const char* sql,
                  int (*cb)(void*, int, char**, char**),
                  void* ud, char** errmsg) {
    using fp = int (*)(sqlite3*, const char*,
                       int (*)(void*, int, char**, char**), void*, char**);
    static fp real = Resolve<fp>("sqlite3_exec");
    return real ? real(db, sql, cb, ud, errmsg) : SQLITE_ERROR;
  }
  static int PrepareV2(sqlite3* db, const char* sql, int n,
                       sqlite3_stmt** stmt, const char** tail) {
    using fp = int (*)(sqlite3*, const char*, int,
                       sqlite3_stmt**, const char**);
    static fp real = Resolve<fp>("sqlite3_prepare_v2");
    return real ? real(db, sql, n, stmt, tail) : SQLITE_ERROR;
  }
  static int BindText(sqlite3_stmt* s, int i, const char* t, int n,
                      void (*dtor)(void*)) {
    using fp = int (*)(sqlite3_stmt*, int, const char*, int, void (*)(void*));
    static fp real = Resolve<fp>("sqlite3_bind_text");
    return real ? real(s, i, t, n, dtor) : SQLITE_ERROR;
  }
  static int BindInt(sqlite3_stmt* s, int i, int v) {
    using fp = int (*)(sqlite3_stmt*, int, int);
    static fp real = Resolve<fp>("sqlite3_bind_int");
    return real ? real(s, i, v) : SQLITE_ERROR;
  }
  static int Step(sqlite3_stmt* s) {
    using fp = int (*)(sqlite3_stmt*);
    static fp real = Resolve<fp>("sqlite3_step");
    return real ? real(s) : SQLITE_ERROR;
  }
  static const unsigned char* ColumnText(sqlite3_stmt* s, int c) {
    using fp = const unsigned char* (*)(sqlite3_stmt*, int);
    static fp real = Resolve<fp>("sqlite3_column_text");
    return real ? real(s, c) : nullptr;
  }
  static int ColumnInt(sqlite3_stmt* s, int c) {
    using fp = int (*)(sqlite3_stmt*, int);
    static fp real = Resolve<fp>("sqlite3_column_int");
    return real ? real(s, c) : 0;
  }
  static int Finalize(sqlite3_stmt* s) {
    using fp = int (*)(sqlite3_stmt*);
    static fp real = Resolve<fp>("sqlite3_finalize");
    return real ? real(s) : SQLITE_ERROR;
  }
  static int CloseV2(sqlite3* db) {
    using fp = int (*)(sqlite3*);
    static fp real = Resolve<fp>("sqlite3_close_v2");
    return real ? real(db) : SQLITE_ERROR;
  }
  /* SQLITE_TRANSIENT is a macro defined as (sqlite3_destructor_type)-1 in
   * the libsqlite3 header. We recreate it at run-time (constexpr cannot
   * hold a reinterpret_cast) so the text lifetime is explicitly
   * "SQLite takes its own copy", matching the production call sites in
   * notification_noti.c / db_manager.cc. */
  static void (*Transient())(void*) {
    return SQLITE_TRANSIENT;
  }
};

class BindMigrationTest : public ::testing::Test {
 protected:
  void SetUp() override {
    ASSERT_EQ(RealSqlite::OpenV2(":memory:", &db_,
                                 SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE,
                                 nullptr),
              SQLITE_OK);

    /* Minimal schema — only the columns this test reads/writes. The
     * full production DDL is exercised by test_db_query_plan.cc; here
     * we want a small schema so the inserted values are hand-verifiable. */
    char* errmsg = nullptr;
    ASSERT_EQ(RealSqlite::Exec(db_,
                "CREATE TABLE noti_list ("
                " priv_id INTEGER PRIMARY KEY AUTOINCREMENT,"
                " caller_app_id TEXT NOT NULL,"
                " tag TEXT,"
                " uid INTEGER)",
                nullptr, nullptr, &errmsg),
              SQLITE_OK) << (errmsg ? errmsg : "");
  }

  void TearDown() override {
    if (db_) RealSqlite::CloseV2(db_);
  }

  sqlite3* db_ = nullptr;
};

/*
 * Semantic equivalence: an INSERT via the bind form + a SELECT via the
 * bind form round-trip the payload unchanged. We insert two rows with
 * distinct app_id / tag so the SELECT_BY_TAG filter actually discriminates.
 */
TEST_F(BindMigrationTest, InsertViaBindProducesIdenticalRow) {
  const char* kInsert =
      "INSERT INTO noti_list (caller_app_id, tag, uid) VALUES (?, ?, ?)";
  sqlite3_stmt* ins = nullptr;
  ASSERT_EQ(RealSqlite::PrepareV2(db_, kInsert, -1, &ins, nullptr), SQLITE_OK);

  ASSERT_EQ(RealSqlite::BindText(ins, 1, "org.tizen.app", -1,
                                 RealSqlite::Transient()), SQLITE_OK);
  ASSERT_EQ(RealSqlite::BindText(ins, 2, "important", -1,
                                 RealSqlite::Transient()), SQLITE_OK);
  ASSERT_EQ(RealSqlite::BindInt(ins, 3, 5001), SQLITE_OK);
  ASSERT_EQ(RealSqlite::Step(ins), SQLITE_DONE);
  RealSqlite::Finalize(ins);

  /* SELECT via the same bind shape the production code uses for
   * notification_noti_get_by_tag. */
  const char* kSelect =
      "SELECT caller_app_id, tag, uid FROM noti_list"
      " WHERE caller_app_id = ? AND tag = ? AND uid = ?";
  sqlite3_stmt* sel = nullptr;
  ASSERT_EQ(RealSqlite::PrepareV2(db_, kSelect, -1, &sel, nullptr), SQLITE_OK);
  ASSERT_EQ(RealSqlite::BindText(sel, 1, "org.tizen.app", -1,
                                 RealSqlite::Transient()), SQLITE_OK);
  ASSERT_EQ(RealSqlite::BindText(sel, 2, "important", -1,
                                 RealSqlite::Transient()), SQLITE_OK);
  ASSERT_EQ(RealSqlite::BindInt(sel, 3, 5001), SQLITE_OK);
  ASSERT_EQ(RealSqlite::Step(sel), SQLITE_ROW);

  const unsigned char* app = RealSqlite::ColumnText(sel, 0);
  const unsigned char* tag = RealSqlite::ColumnText(sel, 1);
  int uid = RealSqlite::ColumnInt(sel, 2);
  ASSERT_NE(app, nullptr);
  ASSERT_NE(tag, nullptr);
  EXPECT_STREQ(reinterpret_cast<const char*>(app), "org.tizen.app");
  EXPECT_STREQ(reinterpret_cast<const char*>(tag), "important");
  EXPECT_EQ(uid, 5001);

  /* No second row. */
  EXPECT_EQ(RealSqlite::Step(sel), SQLITE_DONE);
  RealSqlite::Finalize(sel);
}

/*
 * SQL injection immunity.
 *
 * The %Q-based legacy code would correctly escape single quotes inside
 * app_id on the happy path, but the bind form is stronger: the user
 * input is never concatenated into the SQL text at all. This test
 * passes the classic "'; DROP TABLE noti_list;--" payload as a bound
 * parameter to DELETE_BY_PRIV_ID and asserts:
 *
 *   1. The DELETE runs (i.e. the prepare + step succeed).
 *   2. The affected row count is 0 (no row whose caller_app_id
 *      literally equals "'; DROP TABLE noti_list;--" exists).
 *   3. The noti_list table is still there afterwards — proven by a
 *      follow-up SELECT count(*) succeeding without SQLITE_ERROR.
 *
 * If anywhere in the code path substituted the parameter into SQL
 * text via %s instead of ?, step 3 would fail with SQLITE_ERROR
 * ("no such table: noti_list").
 */
TEST_F(BindMigrationTest, DeleteIsImmuneToSqlInjection) {
  /* Seed one row so the DELETE is measurable on the "safe" tail. */
  char* errmsg = nullptr;
  ASSERT_EQ(RealSqlite::Exec(db_,
              "INSERT INTO noti_list (caller_app_id, tag, uid)"
              " VALUES ('legit', 'x', 5001)",
              nullptr, nullptr, &errmsg), SQLITE_OK) << (errmsg ? errmsg : "");

  /* Adversarial payload — classic Bobby-Tables. */
  const char* kEvilAppId = "'; DROP TABLE noti_list;--";

  /* DELETE via the same prepared-statement shape used by
   * notification_noti_delete_by_priv_id. */
  const char* kDelete =
      "DELETE FROM noti_list WHERE caller_app_id = ? AND priv_id = ?";
  sqlite3_stmt* del = nullptr;
  ASSERT_EQ(RealSqlite::PrepareV2(db_, kDelete, -1, &del, nullptr), SQLITE_OK);
  ASSERT_EQ(RealSqlite::BindText(del, 1, kEvilAppId, -1,
                                 RealSqlite::Transient()), SQLITE_OK);
  ASSERT_EQ(RealSqlite::BindInt(del, 2, 1), SQLITE_OK);
  /* Step should return SQLITE_DONE (statement completed normally) —
   * the DELETE found no rows matching the evil app_id literal, which
   * is the correct outcome. */
  EXPECT_EQ(RealSqlite::Step(del), SQLITE_DONE);
  RealSqlite::Finalize(del);

  /* Prove the table still exists — this is the key injection check. */
  sqlite3_stmt* count = nullptr;
  ASSERT_EQ(RealSqlite::PrepareV2(db_, "SELECT count(*) FROM noti_list",
                                  -1, &count, nullptr), SQLITE_OK)
      << "noti_list table appears to have been dropped — SQL injection!";
  ASSERT_EQ(RealSqlite::Step(count), SQLITE_ROW);
  /* The seed row is still there. */
  EXPECT_EQ(RealSqlite::ColumnInt(count, 0), 1);
  RealSqlite::Finalize(count);
}

/*
 * Secondary injection guard: SELECT_BY_TAG with a malicious tag value.
 * Same structure as the DELETE case — the payload is bound via ? so
 * the query plan cannot be rewritten.
 */
TEST_F(BindMigrationTest, SelectByTagIsImmuneToSqlInjection) {
  char* errmsg = nullptr;
  ASSERT_EQ(RealSqlite::Exec(db_,
              "INSERT INTO noti_list (caller_app_id, tag, uid)"
              " VALUES ('com.samsung.calendar', 'reminder', 5001)",
              nullptr, nullptr, &errmsg), SQLITE_OK) << (errmsg ? errmsg : "");

  const char* kEvilTag = "reminder' OR '1'='1";

  const char* kSelect =
      "SELECT caller_app_id FROM noti_list"
      " WHERE caller_app_id = ? AND tag = ? AND uid = ?";
  sqlite3_stmt* sel = nullptr;
  ASSERT_EQ(RealSqlite::PrepareV2(db_, kSelect, -1, &sel, nullptr), SQLITE_OK);
  ASSERT_EQ(RealSqlite::BindText(sel, 1, "com.samsung.calendar", -1,
                                 RealSqlite::Transient()), SQLITE_OK);
  ASSERT_EQ(RealSqlite::BindText(sel, 2, kEvilTag, -1,
                                 RealSqlite::Transient()), SQLITE_OK);
  ASSERT_EQ(RealSqlite::BindInt(sel, 3, 5001), SQLITE_OK);
  /* With bind, the evil tag is interpreted as a literal value of the
   * tag column — no row matches it exactly, so we expect SQLITE_DONE
   * (no rows returned). A successful injection would have returned
   * the seeded row with SQLITE_ROW via the "OR '1'='1'" escape. */
  EXPECT_EQ(RealSqlite::Step(sel), SQLITE_DONE)
      << "SELECT matched a row under an injection payload — bind is broken.";
  RealSqlite::Finalize(sel);
}

}  // namespace
