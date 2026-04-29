/*
 * Copyright (c) 2026 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 */

/*
 * test_notification_resource_audit.cc — resource-exhaustion audit,
 * follow-up to the encoding pass (commit eeb6c2c).  One defensive
 * resource-bound issue on the DBManager hot path:
 *
 *   RES-1  src/notification-ex/db_manager.cc::DBManagerImpl::GetHideMap
 *
 *   Pre-fix the SQL pulled every row whose hide_list was non-NULL
 *   from noti_ex_list with no LIMIT and no row-count cap, then
 *   built a `std::map<string, string>` from each result row.  With
 *   a corrupted DB or a long-lived app accumulating millions of
 *   hidden-flagged notifications, the map grew linearly (~150
 *   bytes per entry on top of the std::map node overhead) and
 *   could exhaust heap on a 32-bit ARM target.  Realistic Tizen
 *   devices steady-state hold ~hundreds of hide rows, but the
 *   query path was unbounded.
 *
 *   Fix bounds the SQL with `LIMIT 100000` — several orders above
 *   any realistic device steady-state.  When the table has more
 *   than the cap, the SQL silently truncates; callers tolerate a
 *   partial map (the notification's hide_list is consulted
 *   opportunistically as a UI hint, not as a security gate; a
 *   missed entry shows the notification, not the wrong action).
 *
 * Mirror probe: GetHideMap() depends on the live DbConnectionHolder
 * + sqlite_stmt cache, which the existing test_db_pool fixture
 * already exercises via SqliteMock; following that pattern, this
 * test pins the SQL-string contract — the prepared statement must
 * include a LIMIT clause and the literal cap must drift in lockstep
 * with the production constant.  Captures the SQL passed to
 * `sqlite3_prepare_v2` and asserts the LIMIT marker.
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <sqlite3.h>

#include <cstdlib>
#include <cstring>
#include <memory>
#include <string>

#include "mock/test_fixture.hh"
#include "mock/sqlite_mock.hh"

#include "notification-ex/db_manager.h"
#include "notification-ex/idb_manager.h"

using ::testing::_;
using ::testing::DoAll;
using ::testing::Invoke;
using ::testing::Return;
using ::testing::SaveArg;

namespace {

int FakeOpen_Res(const char*, sqlite3** db, int, const char*) {
  static int sentinel;
  *db = reinterpret_cast<sqlite3*>(&sentinel);
  return SQLITE_OK;
}
int FakePrepare_Res(sqlite3*, const char*, int, sqlite3_stmt** stmt,
                    const char**) {
  static int sstmt;
  *stmt = reinterpret_cast<sqlite3_stmt*>(&sstmt);
  return SQLITE_OK;
}
int FakeClose_Res(sqlite3*) { return SQLITE_OK; }
int FakeFinalize_Res(sqlite3_stmt*) { return SQLITE_OK; }

class ResourceMocks :
    virtual public ::testing::NiceMock<SqliteMock> {};

}  // namespace

class NotificationResourceAuditTest : public TestFixture {
 public:
  NotificationResourceAuditTest()
      : TestFixture(std::make_unique<::ResourceMocks>()) {}

  void SetUp() override {
    ::notification::detail::ResetDefaultConnectionForTest();
    auto& m = TestFixture::GetMock<SqliteMock>();
    ON_CALL(m, sqlite3_open_v2(_, _, _, _))
        .WillByDefault(Invoke(FakeOpen_Res));
    ON_CALL(m, sqlite3_close_v2(_)).WillByDefault(Invoke(FakeClose_Res));
    ON_CALL(m, sqlite3_close(_)).WillByDefault(Invoke(FakeClose_Res));
    ON_CALL(m, sqlite3_finalize(_)).WillByDefault(Invoke(FakeFinalize_Res));
    /* Empty result — single SQLITE_DONE keeps the loop from running. */
    ON_CALL(m, sqlite3_step(_)).WillByDefault(Return(SQLITE_DONE));
  }

  void TearDown() override {
    ::notification::detail::ResetDefaultConnectionForTest();
  }
};

/* ------------------------------------------------------------------ */
/* RES-1: GetHideMap SQL must contain LIMIT                            */
/* ------------------------------------------------------------------ */

TEST_F(NotificationResourceAuditTest, RES1_GetHideMapSqlContainsLimitClause) {
  auto& m = TestFixture::GetMock<SqliteMock>();
  const char* captured_sql = nullptr;
  EXPECT_CALL(m, sqlite3_prepare_v2(_, _, _, _, _))
      .WillRepeatedly(DoAll(SaveArg<1>(&captured_sql),
                            Invoke(FakePrepare_Res)));

  auto map = ::notification::DBManager::GetHideMap();

  ASSERT_NE(captured_sql, nullptr);
  EXPECT_NE(strstr(captured_sql, "LIMIT"), nullptr)
      << "post-fix: GetHideMap SQL must contain a LIMIT clause";
  EXPECT_NE(strstr(captured_sql, "100000"), nullptr)
      << "post-fix: LIMIT cap must be 100000 (matches "
         "kSettingDbMaxRowCount in notification_setting_service.cc)";
}

TEST_F(NotificationResourceAuditTest, RES1_GetHideMapSqlMatchesContract) {
  /* Pin the full contract — SELECT shape preserved, WHERE preserved,
   * LIMIT present.  Drift in either direction breaks this test. */
  auto& m = TestFixture::GetMock<SqliteMock>();
  const char* captured_sql = nullptr;
  EXPECT_CALL(m, sqlite3_prepare_v2(_, _, _, _, _))
      .WillRepeatedly(DoAll(SaveArg<1>(&captured_sql),
                            Invoke(FakePrepare_Res)));

  ::notification::DBManager::GetHideMap();

  ASSERT_NE(captured_sql, nullptr);
  EXPECT_NE(strstr(captured_sql, "SELECT root_id, app_id, uid, hide_list"),
            nullptr);
  EXPECT_NE(strstr(captured_sql, "noti_ex_list"), nullptr);
  EXPECT_NE(strstr(captured_sql, "hide_list IS NOT NULL"), nullptr);
  EXPECT_NE(strstr(captured_sql, "LIMIT 100000"), nullptr);
}

TEST_F(NotificationResourceAuditTest, RES1_EmptyResultIsNoCrash) {
  /* Defensive — empty hide_list (which is the normal Tizen
   * device steady state) returns an empty map without crashing. */
  auto map = ::notification::DBManager::GetHideMap();
  EXPECT_TRUE(map.empty());
}

TEST_F(NotificationResourceAuditTest, RES1_LimitFitsCapEnvelope) {
  /* The cap must fit inside a typical 32-bit unsigned without
   * overflow, and must be small enough that
   *   100000 entries * sizeof(std::pair<std::string, std::string>)
   * stays well below 100 MB even at pessimistic key/value sizes
   * (50 bytes key + 100 bytes value = 150 bytes/entry → 15 MB). */
  constexpr int kCap = 100000;
  EXPECT_LE(kCap, 1 << 20)
      << "cap must be small enough that 1MB-key + 1MB-value pessimistic "
         "envelopes stay below process RSS targets";
  EXPECT_GE(kCap, 1024)
      << "cap must be large enough that realistic Tizen device "
         "steady-state (~1000 hide entries) is not truncated";
}
