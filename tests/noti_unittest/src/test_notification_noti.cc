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

#include "mock/test_fixture.hh"
#include "mock/aul_mock.hh"
#include "mock/package_manager_mock.hh"
#include "mock/glib_mock.hh"
#include "mock/sqlite_mock.hh"

#include "notification/include/notification.h"
#include "notification/include/notification_noti.h"

namespace {

notification_h noti = nullptr;

typedef struct {
  const char *package;
  char *locale;
  pkgmgrinfo_app_component app_component;
  application_x *app_info;
} pkgmgr_appinfo_x;

int __fake_aul_app_get_appid_bypid(int pid, char* appid, int len) {
  snprintf(appid, len, "%s", "org.tizen.testappid");
  return AUL_R_OK;
}

int __fake_sqlite3_open_v2(const char* arg0, sqlite3** arg1, int arg2, const char* arg3) {
  char** table = (char** )calloc(10, sizeof(char*));
  *arg1 = (sqlite3*) table;

  return SQLITE_OK;
}

int __fake_sqlite3_prepare_v2(sqlite3* arg0, const char* arg1,
    int arg2, sqlite3_stmt** arg3, const char** arg4) {
  sqlite3_stmt* stmt = (sqlite3_stmt*)malloc(sizeof(sqlite3_stmt*));
  *arg3 = stmt;

  return SQLITE_OK;
};

extern "C" char* sqlite3_mprintf(const char*,...) {
  return (char*)strdup("test_query");
}

class Mocks :
    virtual public ::testing::NiceMock<AulMock>,
    virtual public ::testing::NiceMock<GlibMock>,
    virtual public ::testing::NiceMock<SqliteMock>,
    virtual public ::testing::NiceMock<PackageManagerMock> {};
} // namespace

using ::testing::_;
using ::testing::Invoke;
using ::testing::Return;

class NotificationNotiTest : public TestFixture {
 public:
  NotificationNotiTest() : TestFixture(std::make_unique<::Mocks>()) {}
  virtual void SetUp() {
  }
  virtual void TearDown() {
  }
};

TEST_F(NotificationNotiTest, notification_noti_test) {
  EXPECT_CALL(GetMock<AulMock>(), aul_app_get_appid_bypid(_, _, _))
      .WillRepeatedly(Invoke(__fake_aul_app_get_appid_bypid));

  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_open_v2(_, _, _, _))
      .WillRepeatedly(Invoke(__fake_sqlite3_open_v2));

  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_prepare_v2(_, _, _, _, _))
      .WillRepeatedly(Invoke(__fake_sqlite3_prepare_v2));

  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_get_table(_, _, _, _, _, _))
      .WillRepeatedly(Return(1));

  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_bind_int(_, _, _))
      .WillRepeatedly(Return(SQLITE_OK));

  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_bind_double(_, _, _))
      .WillRepeatedly(Return(SQLITE_OK));

  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_bind_text(_, _, _, _, _))
      .WillRepeatedly(Return(SQLITE_OK));

  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_step(_))
      .WillOnce(Return(SQLITE_OK))
      .WillOnce(Return(SQLITE_ROW))
      .WillOnce(Return(SQLITE_OK))
      .WillOnce(Return(SQLITE_ROW))
      .WillRepeatedly(Return(SQLITE_OK));

  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_finalize(_))
      .WillRepeatedly(Return(SQLITE_OK));

  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_free(_))
      .WillRepeatedly(Return());

  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_column_int(_, _))
      .WillRepeatedly(Return(1));

  int ret;

  noti = notification_create(NOTIFICATION_TYPE_NOTI);
  EXPECT_NE(noti, nullptr);

  ret = notification_noti_insert(noti);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  ret = notification_noti_update(noti);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  int deleted_num;
  int* deleted_list;
  ret = notification_noti_delete_all(NOTIFICATION_TYPE_NOTI, "org.tizen.testappid",
      &deleted_num, &deleted_list, 5001);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);
}

TEST_F(NotificationNotiTest, notification_noti_test2) {
  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_open_v2(_, _, _, _))
      .WillOnce(Invoke(__fake_sqlite3_open_v2));

  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_prepare_v2(_, _, _, _, _))
      .WillOnce(Invoke(__fake_sqlite3_prepare_v2));

  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_step(_))
      .WillOnce(Return(SQLITE_ROW));

  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_finalize(_))
      .WillOnce(Return(SQLITE_OK));

  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_free(_))
      .WillRepeatedly(Return());

  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_column_int(_, _))
      .WillRepeatedly(Return(1));

  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_column_double(_, _))
      .WillRepeatedly(Return(1.0));

  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_column_text(_, _))
      .WillRepeatedly(Return(nullptr));

  int ret;
  ret = notification_noti_get_by_tag(noti, "org.tizen.testappid", "test_tag", 5001);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);
}

TEST_F(NotificationNotiTest, notification_noti_test3) {
  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_open_v2(_, _, _, _))
      .WillOnce(Invoke(__fake_sqlite3_open_v2));

  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_prepare_v2(_, _, _, _, _))
      .WillOnce(Invoke(__fake_sqlite3_prepare_v2));

  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_step(_))
      .WillOnce(Return(SQLITE_OK));

  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_finalize(_))
      .WillOnce(Return(SQLITE_OK));

  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_free(_))
      .WillOnce(Return());

  int ret;

  int num_changes;
  ret = notification_noti_delete_by_priv_id_get_changes("org.tizen.testappid", 1, &num_changes, 5001);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);
}


TEST_F(NotificationNotiTest, notification_noti_test4) {
  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_open_v2(_, _, _, _))
      .WillRepeatedly(Invoke(__fake_sqlite3_open_v2));

  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_prepare_v2(_, _, _, _, _))
      .WillRepeatedly(Invoke(__fake_sqlite3_prepare_v2));

  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_step(_))
    .WillOnce(Return(SQLITE_ROW))
    .WillOnce(Return(SQLITE_ROW))
    .WillRepeatedly(Return(SQLITE_OK));

  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_finalize(_))
      .WillRepeatedly(Return(SQLITE_OK));

  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_free(_))
      .WillRepeatedly(Return());

  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_column_int(_, _))
      .WillRepeatedly(Return(1));

  int ret;

  int deleted_num;
  notification_deleted_list_info_s *deleted_list;
  ret = notification_noti_delete_by_display_applist(NOTIFICATION_DISPLAY_APP_INDICATOR,
      &deleted_num, &deleted_list, 5001);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);
}

TEST_F(NotificationNotiTest, notification_noti_test5) {
  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_open_v2(_, _, _, _))
      .WillOnce(Invoke(__fake_sqlite3_open_v2));

  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_prepare_v2(_, _, _, _, _))
      .WillOnce(Invoke(__fake_sqlite3_prepare_v2));

  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_step(_))
      .WillOnce(Return(SQLITE_ROW));

  int ret;
  int count;
  ret = notification_noti_get_all_count(NOTIFICATION_TYPE_NOTI, &count, 5001);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);
}

TEST_F(NotificationNotiTest, notification_noti_test6) {
  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_open_v2(_, _, _, _))
      .WillOnce(Invoke(__fake_sqlite3_open_v2));

  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_prepare_v2(_, _, _, _, _))
      .WillOnce(Invoke(__fake_sqlite3_prepare_v2));

  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_step(_))
      .WillOnce(Return(SQLITE_ROW));

  int ret;
  int count;
  notification_list_h group_list;
  int list_count;
  ret = notification_noti_get_grouping_list(NOTIFICATION_TYPE_NOTI, 1,1, &group_list, &list_count, 5001);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);
}

TEST_F(NotificationNotiTest, notification_noti_test7) {
  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_open_v2(_, _, _, _))
      .WillRepeatedly(Invoke(__fake_sqlite3_open_v2));

  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_prepare_v2(_, _, _, _, _))
      .WillRepeatedly(Invoke(__fake_sqlite3_prepare_v2));

  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_step(_))
      .WillRepeatedly(Return(SQLITE_ROW));

  int ret;
  notification_list_h detail_list;
  ret = notification_noti_get_detail_list("org.tizen.testappid", 1, 1, 1, &detail_list, 5001);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);
}

TEST_F(NotificationNotiTest, notification_noti_test8) {
  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_open_v2(_, _, _, _))
      .WillRepeatedly(Invoke(__fake_sqlite3_open_v2));

  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_prepare_v2(_, _, _, _, _))
      .WillRepeatedly(Invoke(__fake_sqlite3_prepare_v2));

  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_step(_))
      .WillRepeatedly(Return(SQLITE_ROW));

  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_column_int(_, _))
      .WillRepeatedly(Return(1));

  int ret;
  ret = notification_set_tag(noti, "test_tag");
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  ret = notification_noti_check_tag(noti);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_ALREADY_EXIST_ID);
}

TEST_F(NotificationNotiTest, notification_noti_test9) {
  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_open_v2(_, _, _, _))
      .WillOnce(Invoke(__fake_sqlite3_open_v2));

  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_prepare_v2(_, _, _, _, _))
      .WillOnce(Invoke(__fake_sqlite3_prepare_v2));

  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_step(_))
      .WillOnce(Return(SQLITE_OK));

  int ret;
  int count;
  ret = notification_noti_check_count_for_template(noti, &count);
  ASSERT_EQ(ret, 0);
}

TEST_F(NotificationNotiTest, notification_noti_test10) {
  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_open_v2(_, _, _, _))
      .WillOnce(Invoke(__fake_sqlite3_open_v2));

  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_prepare_v2(_, _, _, _, _))
      .WillOnce(Invoke(__fake_sqlite3_prepare_v2));

  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_step(_))
      .WillOnce(Return(SQLITE_OK));

  int ret;
  ret = notification_noti_add_template(noti, "test_template");
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);
}

TEST_F(NotificationNotiTest, notification_noti_test11) {
  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_open_v2(_, _, _, _))
      .WillOnce(Invoke(__fake_sqlite3_open_v2));

  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_prepare_v2(_, _, _, _, _))
      .WillOnce(Invoke(__fake_sqlite3_prepare_v2));

  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_step(_))
      .WillOnce(Return(SQLITE_ROW));

  int ret;
  ret = notification_noti_get_package_template(noti, "org.tizen.testappid", "test_template");
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);
}

TEST_F(NotificationNotiTest, notification_noti_test12) {
  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_open_v2(_, _, _, _))
      .WillOnce(Invoke(__fake_sqlite3_open_v2));

  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_prepare_v2(_, _, _, _, _))
      .WillOnce(Invoke(__fake_sqlite3_prepare_v2));

  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_step(_))
      .WillOnce(Return(SQLITE_OK));

  int ret;
  ret = notification_noti_delete_template("org.tizen.testpkgid");
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);
}

TEST_F(NotificationNotiTest, notification_noti_test13) {
  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_open_v2(_, _, _, _))
      .WillOnce(Invoke(__fake_sqlite3_open_v2));

  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_prepare_v2(_, _, _, _, _))
      .WillRepeatedly(Invoke(__fake_sqlite3_prepare_v2));

  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_step(_))
      .WillRepeatedly(Return(SQLITE_ROW));

  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_column_int(_, _))
      .WillRepeatedly(Return(101));

  EXPECT_CALL(GetMock<GlibMock>(), g_list_append(_, _))
      .WillOnce(Return(nullptr));

  int ret;
  GList* list;
  ret = notification_noti_check_limit(noti, 5001, &list);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);
}
