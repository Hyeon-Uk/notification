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

  EXPECT_CALL(GetMock<GlibMock>(), access(_, _))
    .WillOnce(Return(0));

  int ret;

  sqlite3* db = nullptr;
  db = notification_db_open();
  EXPECT_NE(db, nullptr);

  int num_changed = 0;
  ret = notification_db_exec(db, "test query", &num_changed);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(num_changed, 3);

  char* column_text = nullptr;
  sqlite3_stmt *stmt;
  column_text = notification_db_column_text(stmt, 3);
  EXPECT_STREQ(column_text, "testcolumn");

  bundle* column_b = nullptr;
  column_b = notification_db_column_bundle(stmt, 3);
  EXPECT_NE(column_b, nullptr);

  ret = notification_db_close(&db);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);
}
