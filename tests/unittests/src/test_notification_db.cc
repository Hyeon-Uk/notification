/*
 * Copyright (c) 2023 Samsung Electronics Co., Ltd.
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

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <bundle.h>

#include "mock/test_fixture.hh"
#include "mock/glib_mock.hh"
#include "mock/sqlite_mock.hh"

#include "notification/include/notification.h"
#include "notification/include/notification_db.h"

namespace {

int __fake_sqlite3_open_v2(const char* arg0, sqlite3** arg1, int arg2, const char* arg3) {
  char** table = (char** )calloc(10, sizeof(char*));
  *arg1 = (sqlite3*) table;

  return SQLITE_OK;
}

const unsigned char* __fake_sqlite3_column_text(sqlite3_stmt* arg0, int arg1) {
  const unsigned char* column_text = (const unsigned char*)strdup("testcolumn");

  return column_text;
}

const unsigned char* __fake_sqlite3_column_text_bundle(sqlite3_stmt* arg0, int arg1) {
  bundle_raw *raw = nullptr;
  int len = 0;
  bundle* b = bundle_create();
  bundle_add_str(b, "testkey", "testval");
  bundle_encode(b, &raw, &len);

  const unsigned char* column_text = (const unsigned char*)raw;

  return column_text;
}

class Mocks :
    virtual public ::testing::NiceMock<GlibMock>,
    virtual public ::testing::NiceMock<SqliteMock> {};
} // namespace

using ::testing::_;
using ::testing::Invoke;
using ::testing::Return;

class NotificationDBTest : public TestFixture {
 public:
  NotificationDBTest() : TestFixture(std::make_unique<::Mocks>()) {}
  virtual void SetUp() {
  }
  virtual void TearDown() {
  }
};

/* notification_db.c null-arg validation paths */

TEST_F(NotificationDBTest, db_close_null_pointer_to_pointer_returns_invalid) {
  EXPECT_EQ(notification_db_close(nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationDBTest, db_close_null_inner_pointer_returns_invalid) {
  sqlite3* db = nullptr;
  EXPECT_EQ(notification_db_close(&db),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationDBTest, notification_db_test) {
  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_open_v2(_, _, _, _))
    .WillRepeatedly(Invoke(__fake_sqlite3_open_v2));

  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_close(_))
    .WillOnce(Return(SQLITE_OK));

  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_prepare_v2(_, _, _, _, _))
    .WillOnce(Return(SQLITE_OK));

  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_step(_))
    .WillOnce(Return(SQLITE_OK));

  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_changes(_))
    .WillOnce(Return(3));

  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_column_text(_, _))
    .WillOnce(Invoke(__fake_sqlite3_column_text))
    .WillOnce(Invoke(__fake_sqlite3_column_text_bundle));

  int ret;

  sqlite3* db = nullptr;
  db = notification_db_open();
  EXPECT_NE(db, nullptr);

  int num_changed = 0;
  ret = notification_db_exec(db, "test query", &num_changed);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(num_changed, 3);

  char* column_text = nullptr;
  sqlite3_stmt *stmt = nullptr;
  column_text = notification_db_column_text(stmt, 3);
  EXPECT_STREQ(column_text, "testcolumn");

  bundle* column_b = nullptr;
  column_b = notification_db_column_bundle(stmt, 3);
  EXPECT_NE(column_b, nullptr);

  ret = notification_db_close(&db);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);
}

TEST_F(NotificationDBTest, db_open_sqlite_permission_denied) {
  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_open_v2(_, _, _, _))
      .WillOnce(Return(SQLITE_PERM));
  sqlite3* db = notification_db_open();
  EXPECT_EQ(db, nullptr);
}

TEST_F(NotificationDBTest, db_open_sqlite_generic_error) {
  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_open_v2(_, _, _, _))
      .WillOnce(Return(SQLITE_ERROR));
  sqlite3* db = notification_db_open();
  EXPECT_EQ(db, nullptr);
}

TEST_F(NotificationDBTest, db_close_sqlite_error_returns_from_db) {
  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_close(_))
      .WillOnce(Return(SQLITE_ERROR));
  char db_buf[8] = {0};
  sqlite3* dummy = reinterpret_cast<sqlite3*>(db_buf);
  EXPECT_EQ(notification_db_close(&dummy),
            NOTIFICATION_ERROR_FROM_DB);
}

/* ============================================================== */
/* notification_db_exec — sqlite3_prepare_v2 failure path (L82)  */
/* ============================================================== */

TEST_F(NotificationDBTest, db_exec_prepare_fail_returns_from_db) {
  /* [N1] sqlite3_open_v2 succeeds, then sqlite3_prepare_v2 returns
   * SQLITE_ERROR → notification_db_exec must return
   * NOTIFICATION_ERROR_FROM_DB.
   * We use the fake open so db is a real-looking pointer, avoiding
   * a crash when sqlite3_errmsg(db) is called inside the ERR macro. */
  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_open_v2(_, _, _, _))
      .WillOnce(Invoke(__fake_sqlite3_open_v2));
  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_prepare_v2(_, _, _, _, _))
      .WillOnce(Return(SQLITE_ERROR));

  sqlite3* db = notification_db_open();
  ASSERT_NE(db, nullptr);

  int num_changed = 0;
  EXPECT_EQ(notification_db_exec(db, "SELECT 1", &num_changed),
            NOTIFICATION_ERROR_FROM_DB);

  /* Clean up the fake db (it is a heap allocation from __fake_sqlite3_open_v2). */
  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_close(_))
      .WillOnce(Return(SQLITE_OK));
  notification_db_close(&db);
}

/* ============================================================== */
/* notification_db_exec — sqlite3_step error path (L93)           */
/* ============================================================== */

TEST_F(NotificationDBTest, db_exec_step_error_returns_from_db) {
  /* [N2] sqlite3_prepare_v2 succeeds but sqlite3_step returns an error
   * code other than SQLITE_OK/SQLITE_DONE →
   * notification_db_exec must return NOTIFICATION_ERROR_FROM_DB. */
  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_open_v2(_, _, _, _))
      .WillOnce(Invoke(__fake_sqlite3_open_v2));
  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_prepare_v2(_, _, _, _, _))
      .WillOnce(Return(SQLITE_OK));
  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_step(_))
      .WillOnce(Return(SQLITE_BUSY));
  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_finalize(_))
      .WillOnce(Return(SQLITE_OK));

  sqlite3* db = notification_db_open();
  ASSERT_NE(db, nullptr);

  int num_changed = 0;
  EXPECT_EQ(notification_db_exec(db, "SELECT 1", &num_changed),
            NOTIFICATION_ERROR_FROM_DB);

  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_close(_))
      .WillOnce(Return(SQLITE_OK));
  notification_db_close(&db);
}

/* ============================================================== */
/* notification_db_exec — SQLITE_DONE path with null num_changes  */
/* ============================================================== */

/* ============================================================== */
/* notification_db_open — access() denied path (L37-38)          */
/* ============================================================== */

TEST_F(NotificationDBTest, db_open_access_denied_returns_null) {
  /* [N3] When access(database, R_OK) returns -1 (permission denied or
   * file not found), notification_db_open must return NULL without
   * calling sqlite3_open_v2. */
  EXPECT_CALL(GetMock<GlibMock>(), access(_, _))
      .WillOnce(Return(-1));
  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_open_v2(_, _, _, _))
      .Times(0);

  EXPECT_EQ(notification_db_open(), nullptr);
}

TEST_F(NotificationDBTest, db_exec_step_done_null_num_changes) {
  /* [E1] sqlite3_step returns SQLITE_DONE and num_changes is NULL →
   * sqlite3_changes must NOT be called; return NONE. */
  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_open_v2(_, _, _, _))
      .WillOnce(Invoke(__fake_sqlite3_open_v2));
  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_prepare_v2(_, _, _, _, _))
      .WillOnce(Return(SQLITE_OK));
  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_step(_))
      .WillOnce(Return(SQLITE_DONE));
  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_changes(_))
      .Times(0);
  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_finalize(_))
      .WillOnce(Return(SQLITE_OK));

  sqlite3* db = notification_db_open();
  ASSERT_NE(db, nullptr);

  EXPECT_EQ(notification_db_exec(db, "DELETE FROM x", nullptr),
            NOTIFICATION_ERROR_NONE);

  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_close(_))
      .WillOnce(Return(SQLITE_OK));
  notification_db_close(&db);
}
