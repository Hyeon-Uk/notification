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
 * Format-string + SQL-injection audit, follow-up to the integer-overflow
 * pass (commit 74aefde).  One production-reachable defect found in the
 * EXPORT_API surface of the notification-ex client library:
 *
 *   FS-1  src/notification-ex/db_manager.cc::
 *         DBManagerImpl::GetNotificationList(uid_t uid, string channel)
 *
 *   Pre-fix the overload built `query_str` by string-concatenating the
 *   caller-supplied `channel` directly into the SQL ("AND channel = '"
 *   + channel + "'") and then handed the result to
 *   `sqlite3_mprintf(query_str.c_str(), uid)` — i.e. the dynamic string
 *   was used as the FORMAT argument.  Two real defects:
 *
 *     1. SQL injection — any single-quote in `channel` escapes the
 *        WHERE clause (`' OR '1'='1' --`).
 *     2. Format-string injection — any `%`-specifier (`%n` / `%s` /
 *        `%x`) in `channel` survived into sqlite3_mprintf's printf-like
 *        substitution and read past the single bound argument via
 *        va_arg.  CWE-134.
 *
 *   Sister overloads (uid + app_id + root_id, uid + app_id + priv_id)
 *   already used prepared statements; the fix folds this overload onto
 *   the same shape via four static SQL templates picked by the
 *   (with_channel, with_simmode) flag pair, with `?` bindings for both
 *   the integer uid / policy-mask and the text channel.  Eliminates
 *   both injection classes in one shape.
 *
 * Test direction: pin the post-fix contract by intercepting
 * sqlite3_prepare_v2 / sqlite3_bind_text via SqliteMock and asserting
 *
 *   (a) the SQL string passed to prepare_v2 is one of the four static
 *       templates and contains NEITHER the literal channel value NOR
 *       any %-format specifier;
 *   (b) the channel value reaches sqlite3_bind_text verbatim (so the
 *       caller's data is interpreted purely as data, never as code);
 *   (c) hostile channel inputs (`%n`, `' OR '1'='1' --`, NUL-laden)
 *       drive the same path — the bound parameter is opaque to SQLite
 *       beyond the `?` slot.
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <sqlite3.h>

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
using ::testing::SetArgPointee;

namespace {

int FmtInjFakeOpenSuccess(const char* /*path*/, sqlite3** out_db,
                          int /*flags*/, const char* /*vfs*/) {
  static int sentinel;
  *out_db = reinterpret_cast<sqlite3*>(&sentinel);
  return SQLITE_OK;
}

int FmtInjFakePrepareOk(sqlite3* /*db*/, const char* /*sql*/, int /*nbyte*/,
                        sqlite3_stmt** out_stmt, const char** /*tail*/) {
  static int stmt_sentinel;
  *out_stmt = reinterpret_cast<sqlite3_stmt*>(&stmt_sentinel);
  return SQLITE_OK;
}

class FmtInjMocks :
    virtual public ::testing::NiceMock<SqliteMock> {};

}  // namespace

class NotificationFmtInjAuditTest : public TestFixture {
 public:
  NotificationFmtInjAuditTest()
      : TestFixture(std::make_unique<::FmtInjMocks>()) {}

  void SetUp() override {
    /* Cold pool — keep open counts deterministic across tests. */
    ::notification::detail::ResetDefaultConnectionForTest();

    auto& sqlite = TestFixture::GetMock<SqliteMock>();
    /* Every test below funnels through the same scaffolding: open OK,
     * prepare OK, no rows, finalize / close OK.  Per-test
     * EXPECT_CALLs add specific bind-arg assertions on top. */
    ON_CALL(sqlite, sqlite3_open_v2(_, _, _, _))
        .WillByDefault(Invoke(FmtInjFakeOpenSuccess));
    ON_CALL(sqlite, sqlite3_prepare_v2(_, _, _, _, _))
        .WillByDefault(Invoke(FmtInjFakePrepareOk));
    ON_CALL(sqlite, sqlite3_step(_))
        .WillByDefault(Return(SQLITE_DONE));
    ON_CALL(sqlite, sqlite3_finalize(_))
        .WillByDefault(Return(SQLITE_OK));
    ON_CALL(sqlite, sqlite3_close_v2(_))
        .WillByDefault(Return(SQLITE_OK));
    ON_CALL(sqlite, sqlite3_bind_int(_, _, _))
        .WillByDefault(Return(SQLITE_OK));
    ON_CALL(sqlite, sqlite3_bind_text(_, _, _, _, _))
        .WillByDefault(Return(SQLITE_OK));
  }

  void TearDown() override {
    ::notification::detail::ResetDefaultConnectionForTest();
  }
};

/* ------------------------------------------------------------------ */
/* Contract pin: SQL string is a static template, not concatenated     */
/* ------------------------------------------------------------------ */

TEST_F(NotificationFmtInjAuditTest, FS1_PreparedSqlHasNoChannelLiteral) {
  auto& sqlite = TestFixture::GetMock<SqliteMock>();

  /* Capture the SQL passed to prepare_v2; the channel value must NEVER
   * appear inside it.  If post-fix regresses to the old concat shape
   * the literal "weirdchannel" would re-appear in the captured string. */
  const char* captured_sql = nullptr;
  EXPECT_CALL(sqlite, sqlite3_prepare_v2(_, _, _, _, _))
      .WillRepeatedly(DoAll(SaveArg<1>(&captured_sql),
                            Invoke(FmtInjFakePrepareOk)));

  ::notification::DBManager::GetNotificationList(
      static_cast<uid_t>(5001), std::string("weirdchannel"));

  ASSERT_NE(captured_sql, nullptr);
  EXPECT_EQ(strstr(captured_sql, "weirdchannel"), nullptr)
      << "post-fix: channel value must not be inlined into the SQL string";
  EXPECT_NE(strstr(captured_sql, "channel = ?"), nullptr)
      << "post-fix: channel must be a `?` placeholder bound separately";
  EXPECT_EQ(strstr(captured_sql, "%d"), nullptr)
      << "post-fix: prepared SQL must not retain any %d format specifier";
  EXPECT_EQ(strstr(captured_sql, "%s"), nullptr);
}

TEST_F(NotificationFmtInjAuditTest, FS1_EmptyChannelUsesPlainTemplate) {
  auto& sqlite = TestFixture::GetMock<SqliteMock>();

  const char* captured_sql = nullptr;
  EXPECT_CALL(sqlite, sqlite3_prepare_v2(_, _, _, _, _))
      .WillRepeatedly(DoAll(SaveArg<1>(&captured_sql),
                            Invoke(FmtInjFakePrepareOk)));

  ::notification::DBManager::GetNotificationList(
      static_cast<uid_t>(5001), std::string());

  ASSERT_NE(captured_sql, nullptr);
  EXPECT_EQ(strstr(captured_sql, "channel"), nullptr)
      << "empty channel must select the channel-free template";
  EXPECT_NE(strstr(captured_sql, "uid = ?"), nullptr);
}

/* ------------------------------------------------------------------ */
/* Contract pin: channel reaches bind_text verbatim (data, not code)   */
/* ------------------------------------------------------------------ */

TEST_F(NotificationFmtInjAuditTest, FS1_ChannelReachesBindTextVerbatim) {
  auto& sqlite = TestFixture::GetMock<SqliteMock>();

  std::string captured_bind;
  ON_CALL(sqlite, sqlite3_bind_text(_, 2, _, _, _))
      .WillByDefault(Invoke([&captured_bind](
          sqlite3_stmt*, int, const char* s, int /*n*/,
          void(*)(void*)) {
        if (s) captured_bind = s;
        return SQLITE_OK;
      }));

  ::notification::DBManager::GetNotificationList(
      static_cast<uid_t>(5001), std::string("legit_channel"));

  EXPECT_EQ(captured_bind, "legit_channel");
}

/* ------------------------------------------------------------------ */
/* Hostile inputs: SQL injection + format-string injection             */
/* ------------------------------------------------------------------ */

TEST_F(NotificationFmtInjAuditTest, FS1_SqlInjectionPayloadStaysInBind) {
  /* Pre-fix: this payload would dissolve the WHERE clause into
   *   AND channel = '' OR '1'='1' --'
   * Post-fix: it must reach sqlite3_bind_text byte-for-byte and the
   * SQL string itself must remain a static template. */
  auto& sqlite = TestFixture::GetMock<SqliteMock>();

  const char* captured_sql = nullptr;
  std::string captured_bind;
  EXPECT_CALL(sqlite, sqlite3_prepare_v2(_, _, _, _, _))
      .WillRepeatedly(DoAll(SaveArg<1>(&captured_sql),
                            Invoke(FmtInjFakePrepareOk)));
  ON_CALL(sqlite, sqlite3_bind_text(_, 2, _, _, _))
      .WillByDefault(Invoke([&captured_bind](
          sqlite3_stmt*, int, const char* s, int /*n*/,
          void(*)(void*)) {
        if (s) captured_bind = s;
        return SQLITE_OK;
      }));

  const std::string payload = "' OR '1'='1' --";
  ::notification::DBManager::GetNotificationList(
      static_cast<uid_t>(5001), payload);

  ASSERT_NE(captured_sql, nullptr);
  EXPECT_EQ(strstr(captured_sql, "OR"), nullptr)
      << "SQL injection payload must not appear in the prepared SQL";
  EXPECT_EQ(strstr(captured_sql, "'1'='1'"), nullptr);
  EXPECT_EQ(captured_bind, payload)
      << "channel payload reaches bind_text verbatim — opaque to SQLite";
}

TEST_F(NotificationFmtInjAuditTest,
       FS1_FormatSpecifierPayloadStaysInBind) {
  /* Pre-fix: %n / %s / %x in `channel` survived into sqlite3_mprintf's
   * vararg loop and would have read past the single bound `uid`.
   * Post-fix: the prepared SQL is one of four static templates with
   * no %-specifiers, and the payload reaches bind_text byte-for-byte. */
  auto& sqlite = TestFixture::GetMock<SqliteMock>();

  const char* captured_sql = nullptr;
  std::string captured_bind;
  EXPECT_CALL(sqlite, sqlite3_prepare_v2(_, _, _, _, _))
      .WillRepeatedly(DoAll(SaveArg<1>(&captured_sql),
                            Invoke(FmtInjFakePrepareOk)));
  ON_CALL(sqlite, sqlite3_bind_text(_, 2, _, _, _))
      .WillByDefault(Invoke([&captured_bind](
          sqlite3_stmt*, int, const char* s, int /*n*/,
          void(*)(void*)) {
        if (s) captured_bind = s;
        return SQLITE_OK;
      }));

  const std::string payload = "%n%n%n%s%x";
  ::notification::DBManager::GetNotificationList(
      static_cast<uid_t>(5001), payload);

  ASSERT_NE(captured_sql, nullptr);
  EXPECT_EQ(strstr(captured_sql, "%n"), nullptr)
      << "format specifier from caller must not reach the prepared SQL";
  EXPECT_EQ(strstr(captured_sql, "%s"), nullptr);
  EXPECT_EQ(strstr(captured_sql, "%x"), nullptr);
  EXPECT_EQ(captured_bind, payload)
      << "format-specifier payload reaches bind_text verbatim";
}
