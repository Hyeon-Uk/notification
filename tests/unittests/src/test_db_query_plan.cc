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
 * N3-T7 Q2: query-plan regression guard for the composite covering
 * indexes added to noti_list.
 *
 * The production hot path issues three predicate shapes against
 * noti_list:
 *   - WHERE caller_app_id = ? AND uid = ? [ORDER BY insert_time DESC]
 *   - WHERE pkg_id        = ? AND uid = ? [ORDER BY insert_time DESC]
 *   - WHERE uid           = ?              ORDER BY insert_time DESC
 *
 * Without the composite (caller_app_id, uid, insert_time DESC) index,
 * SQLite resorts to a full table scan + temp-sort; the goal of the
 * index is to let the query planner answer both the equality lookup and
 * the trailing ORDER BY in a single ordered index walk, which collapses
 * N*log(N) sort work into an O(k) range scan. This test pins the
 * planner decision by parsing EXPLAIN QUERY PLAN output and asserting
 * that the composite index name appears at least once.
 *
 * Mock interposition note: the unittest binary links against
 * tests/mock/sqlite_mock.cc, which re-defines extern "C" sqlite3_*
 * symbols so every call from libnotification.so routes through the
 * gmock table. Here we want REAL SQLite — the test must see the
 * planner's genuine decision against the actual schema + indexes. We
 * therefore resolve each required sqlite3_* entry point via
 * dlsym(RTLD_NEXT, ...), the same technique used by
 * tests/mock/glib_mock.cc for g_variant_builder_* and g_object_unref.
 * This bypasses the mock layer without disturbing the production or
 * other-test call paths that still rely on the mocks.
 */

/*
 * _GNU_SOURCE is required for dlmopen (glibc-only) which gives us an
 * isolated link namespace — the only reliable way to reach libsqlite3's
 * own symbol definitions without the sqlite_mock.cc interposers getting
 * in the way. Define it before any system header pulls in dlfcn.h.
 */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <gtest/gtest.h>

#include <dlfcn.h>
#include <sqlite3.h>

#include <cstring>
#include <string>

#include "notification/src/notification_db_query.h"

namespace {

/*
 * Thin real-SQLite shim. Every entry point is resolved once from
 * libsqlite3.so.0 via an explicit dlopen() handle and cached in a
 * function-local static. The shim intentionally covers only the
 * minimal surface this test needs (open_v2, exec, prepare_v2, step,
 * column_text, finalize, close_v2) so we do not accidentally shadow
 * other production call sites.
 *
 * Why explicit dlopen instead of dlsym(RTLD_NEXT, ...): the test
 * executable links sqlite_mock.cc, which provides extern "C"
 * definitions of sqlite3_* in the main binary. Combined with
 * --export-dynamic (tests/unittests/CMakeLists.txt:48) the executable
 * exports those symbols globally, so RTLD_NEXT from within the
 * executable still resolves to the mock bodies under some glibc /
 * PIE load orderings. Opening libsqlite3 as an explicit handle and
 * calling dlsym(handle, ...) sidesteps that ambiguity and guarantees
 * we reach the library's own implementation regardless of interposer
 * presence — the pattern glib_mock.cc uses for real glib entry points
 * is equivalent in practice but relies on those symbols NOT being
 * defined in the main executable.
 */
class RealSqlite {
 public:
  static void* Handle() {
    static void* h = []() -> void* {
      /*
       * Use dlmopen(LM_ID_NEWLM, ...) so libsqlite3 is loaded into a
       * private link namespace that does NOT include the main test
       * executable's symbol table. This is necessary because the test
       * binary is built with `-Wl,--export-dynamic` (see
       * tests/unittests/CMakeLists.txt:48) and links sqlite_mock.cc
       * which provides extern "C" sqlite3_* definitions. With a
       * regular dlopen() the glibc dynamic linker, on at least some
       * Tizen build hosts, was observed to return the main-exe mock
       * symbol from `dlsym(handle, "sqlite3_open_v2")` because the
       * mock is present in the global scope. dlmopen with
       * LM_ID_NEWLM side-steps the issue entirely — the new
       * namespace only contains libsqlite3 and its dependencies, so
       * symbol resolution cannot escape back to the executable.
       *
       * RTLD_NOW forces all relocations to resolve inside the new
       * namespace (no lazy binding back to the main exe). We fall
       * back to the unversioned soname should the version-suffixed
       * one be absent on an exotic host.
       */
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

  static int OpenV2(const char* filename, sqlite3** out, int flags,
                    const char* zVfs) {
    using fp = int (*)(const char*, sqlite3**, int, const char*);
    static fp real = Resolve<fp>("sqlite3_open_v2");
    return real ? real(filename, out, flags, zVfs) : SQLITE_ERROR;
  }

  static int Exec(sqlite3* db, const char* sql,
                  int (*cb)(void*, int, char**, char**), void* ud,
                  char** errmsg) {
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

  static int Step(sqlite3_stmt* stmt) {
    using fp = int (*)(sqlite3_stmt*);
    static fp real = Resolve<fp>("sqlite3_step");
    return real ? real(stmt) : SQLITE_ERROR;
  }

  static const unsigned char* ColumnText(sqlite3_stmt* stmt, int col) {
    using fp = const unsigned char* (*)(sqlite3_stmt*, int);
    static fp real = Resolve<fp>("sqlite3_column_text");
    return real ? real(stmt, col) : nullptr;
  }

  static int Finalize(sqlite3_stmt* stmt) {
    using fp = int (*)(sqlite3_stmt*);
    static fp real = Resolve<fp>("sqlite3_finalize");
    return real ? real(stmt) : SQLITE_ERROR;
  }

  static int CloseV2(sqlite3* db) {
    using fp = int (*)(sqlite3*);
    static fp real = Resolve<fp>("sqlite3_close_v2");
    return real ? real(db) : SQLITE_ERROR;
  }
};

/*
 * Collect every text row returned by `sql` column-0 into one string,
 * separated by '\n'. EXPLAIN QUERY PLAN historically returned a table
 * with (selectid, order, from, detail); modern SQLite flattens this to
 * an (id, parent, notused, detail) shape. We only inspect the last
 * column ("detail") which is where the planner writes
 * "SEARCH TABLE noti_list USING INDEX idx_...".
 */
std::string CollectExplain(sqlite3* db, const char* sql) {
  std::string out;
  sqlite3_stmt* stmt = nullptr;
  int rc = RealSqlite::PrepareV2(db, sql, -1, &stmt, nullptr);
  EXPECT_EQ(rc, SQLITE_OK) << "prepare failed for: " << sql;
  if (rc != SQLITE_OK || !stmt)
    return out;

  while (RealSqlite::Step(stmt) == SQLITE_ROW) {
    /* Column count on EXPLAIN QUERY PLAN is 4; "detail" is the last. */
    const unsigned char* detail = RealSqlite::ColumnText(stmt, 3);
    if (detail) {
      out.append(reinterpret_cast<const char*>(detail));
      out.push_back('\n');
    }
  }
  RealSqlite::Finalize(stmt);
  return out;
}

class DbQueryPlanTest : public ::testing::Test {
 protected:
  void SetUp() override {
    /*
     * ":memory:" keeps the test self-contained and obviates filesystem
     * access (no DBPATH, no umask games, no cleanup). An in-memory DB
     * is sufficient for planner-level assertions because SQLite's
     * query-plan decision is schema-driven, not storage-driven.
     */
    ASSERT_EQ(RealSqlite::OpenV2(":memory:", &db_,
                                 SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE,
                                 nullptr),
              SQLITE_OK);

    char* errmsg = nullptr;
    /*
     * Run the exact production DDL so any future schema drift in
     * notification_db_query.h is reflected here automatically.
     */
    ASSERT_EQ(RealSqlite::Exec(db_, CREATE_NOTIFICATION_TABLE,
                               nullptr, nullptr, &errmsg),
              SQLITE_OK)
        << "CREATE_NOTIFICATION_TABLE failed: "
        << (errmsg ? errmsg : "(null)");
    if (errmsg) { /* errmsg freed via sqlite3_free in real build; on the
                   * mocked path sqlite3_free is intercepted so we leak
                   * intentionally to avoid calling through the mock.
                   * In practice errmsg is nullptr on the SQLITE_OK branch
                   * above so this arm is only hit on failure. */
    }

    ASSERT_EQ(RealSqlite::Exec(db_, CREATE_NOTIFICATION_INDEXES,
                               nullptr, nullptr, &errmsg),
              SQLITE_OK)
        << "CREATE_NOTIFICATION_INDEXES failed: "
        << (errmsg ? errmsg : "(null)");
  }

  void TearDown() override {
    if (db_)
      RealSqlite::CloseV2(db_);
  }

  sqlite3* db_ = nullptr;
};

/*
 * Primary assertion for N3-T7 Q2: the planner must pick
 * idx_noti_caller_uid_time for the (caller_app_id, uid) predicate with
 * an ORDER BY insert_time DESC trailer. If SQLite ever regresses or
 * the index definition drifts out of column-order alignment, this test
 * fires before the change lands.
 */
TEST_F(DbQueryPlanTest, UsesCompositeIndex) {
  const char* kQuery =
      "EXPLAIN QUERY PLAN "
      "SELECT priv_id FROM noti_list "
      "WHERE caller_app_id = 'com.samsung.test' AND uid = 5001 "
      "ORDER BY insert_time DESC";

  std::string plan = CollectExplain(db_, kQuery);
  ASSERT_FALSE(plan.empty()) << "EXPLAIN QUERY PLAN returned no rows";

  EXPECT_NE(plan.find("idx_noti_caller_uid_time"), std::string::npos)
      << "Planner did not pick the composite index. Plan was:\n" << plan;
  /*
   * "USING INDEX" vs "SEARCH TABLE noti_list USING INDEX ..." — the
   * exact wording varies across SQLite versions (3.8 prints "SEARCH
   * TABLE noti_list USING INDEX ...", 3.35+ sometimes prints "USING
   * COVERING INDEX ..."). We only assert the index name appears.
   */
}

/* Secondary coverage: the pkg_id + uid shape picks idx_noti_pkg_uid_time. */
TEST_F(DbQueryPlanTest, UsesPkgUidIndex) {
  const char* kQuery =
      "EXPLAIN QUERY PLAN "
      "SELECT priv_id FROM noti_list "
      "WHERE pkg_id = 'org.tizen.pkg' AND uid = 5001 "
      "ORDER BY insert_time DESC";

  std::string plan = CollectExplain(db_, kQuery);
  ASSERT_FALSE(plan.empty());

  EXPECT_NE(plan.find("idx_noti_pkg_uid_time"), std::string::npos)
      << "Planner did not pick idx_noti_pkg_uid_time. Plan was:\n" << plan;
}

/* Tertiary: uid-only + ORDER BY insert_time DESC uses idx_noti_uid_time. */
TEST_F(DbQueryPlanTest, UsesUidTimeIndex) {
  const char* kQuery =
      "EXPLAIN QUERY PLAN "
      "SELECT priv_id FROM noti_list "
      "WHERE uid = 5001 "
      "ORDER BY insert_time DESC";

  std::string plan = CollectExplain(db_, kQuery);
  ASSERT_FALSE(plan.empty());

  EXPECT_NE(plan.find("idx_noti_uid_time"), std::string::npos)
      << "Planner did not pick idx_noti_uid_time. Plan was:\n" << plan;
}

}  // namespace
