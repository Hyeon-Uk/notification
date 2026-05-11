/*
 * Copyright (c) 2023 Samsung Electronics Co., Ltd. All rights reserved.
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
#include <sqlite3.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <unistd.h>
#include <sys/stat.h>

#include "notification/include/notification.h"
#include "notification/include/notification_setting.h"
#include "notification/include/notification_setting_internal.h"
#include "notification/include/notification_setting_service.h"

namespace {

const char* CREATE_SETTING_TABLE_SQL =
    "CREATE TABLE IF NOT EXISTS notification_setting ("
    "  uid INTEGER,"
    "  package_name TEXT NOT NULL,"
    "  app_id TEXT NOT NULL,"
    "  allow_to_notify INTEGER DEFAULT 1,"
    "  do_not_disturb_except INTEGER DEFAULT 0,"
    "  visibility_class INTEGER DEFAULT 0,"
    "  pop_up_notification INTEGER DEFAULT 1,"
    "  lock_screen_content_level INTEGER DEFAULT 0,"
    "  app_disabled INTEGER DEFAULT 0,"
    "  UNIQUE (uid, package_name, app_id));";

const char* CREATE_SYSTEM_SETTING_TABLE_SQL =
    "CREATE TABLE IF NOT EXISTS notification_system_setting ("
    "  uid INTEGER,"
    "  do_not_disturb INTEGER DEFAULT 0,"
    "  visibility_class INTEGER DEFAULT 0,"
    "  dnd_schedule_enabled INTEGER DEFAULT 0,"
    "  dnd_schedule_day INTEGER DEFAULT 62,"
    "  dnd_start_hour INTEGER DEFAULT 22,"
    "  dnd_start_min INTEGER DEFAULT 0,"
    "  dnd_end_hour INTEGER DEFAULT 8,"
    "  dnd_end_min INTEGER DEFAULT 0,"
    "  lock_screen_content_level INTEGER DEFAULT 0,"
    "  UNIQUE (uid));";

const char* CREATE_DND_EXCEPTION_TABLE_SQL =
    "CREATE TABLE IF NOT EXISTS dnd_allow_exception ("
    "  uid INTEGER,"
    "  type INTEGER DEFAULT 0,"
    "  value INTEGER DEFAULT 0,"
    "  UNIQUE (uid, type));";

}  // namespace

class NotificationSettingIntegrationTest : public ::testing::Test {
 protected:
  void SetUp() override {
    char tmpl[] = "/tmp/noti_setting_test_XXXXXX";
    char* dir = mkdtemp(tmpl);
    ASSERT_NE(dir, nullptr);
    tmp_dir_ = std::string(dir);
    db_path_ = tmp_dir_ + "/noti_setting.db";

    int rc = sqlite3_open(db_path_.c_str(), &db_);
    ASSERT_EQ(rc, SQLITE_OK);
    ASSERT_NE(db_, nullptr);

    char* err_msg = nullptr;
    rc = sqlite3_exec(db_, CREATE_SETTING_TABLE_SQL, nullptr, nullptr, &err_msg);
    if (err_msg) { sqlite3_free(err_msg); err_msg = nullptr; }
    ASSERT_EQ(rc, SQLITE_OK);

    rc = sqlite3_exec(db_, CREATE_SYSTEM_SETTING_TABLE_SQL, nullptr, nullptr, &err_msg);
    if (err_msg) { sqlite3_free(err_msg); err_msg = nullptr; }
    ASSERT_EQ(rc, SQLITE_OK);

    rc = sqlite3_exec(db_, CREATE_DND_EXCEPTION_TABLE_SQL, nullptr, nullptr, &err_msg);
    if (err_msg) { sqlite3_free(err_msg); err_msg = nullptr; }
    ASSERT_EQ(rc, SQLITE_OK);
  }

  void TearDown() override {
    if (db_) {
      sqlite3_close(db_);
      db_ = nullptr;
    }
    if (!tmp_dir_.empty()) {
      // Clean up temporary test directory
      std::string db_file = tmp_dir_ + "/noti_setting.db";
      unlink(db_file.c_str());
      rmdir(tmp_dir_.c_str());
    }
  }

  int GetRowCount(const char* table_name) {
    std::string query = "SELECT COUNT(*) FROM " + std::string(table_name) + ";";
    sqlite3_stmt* stmt = nullptr;
    int count = -1;

    int rc = sqlite3_prepare_v2(db_, query.c_str(), -1, &stmt, nullptr);
    if (rc == SQLITE_OK && sqlite3_step(stmt) == SQLITE_ROW) {
      count = sqlite3_column_int(stmt, 0);
    }
    if (stmt)
      sqlite3_finalize(stmt);
    return count;
  }

  sqlite3* db_ = nullptr;
  std::string tmp_dir_;
  std::string db_path_;
};

TEST_F(NotificationSettingIntegrationTest, CreateSettingAndStore) {
  const char* insert_sql =
      "INSERT INTO notification_setting "
      "(uid, package_name, app_id, allow_to_notify, do_not_disturb_except, "
      "visibility_class, pop_up_notification, lock_screen_content_level) "
      "VALUES (5001, 'org.tizen.testpkg', 'org.tizen.testapp', 1, 0, 0, 1, 0);";

  char* err_msg = nullptr;
  int rc = sqlite3_exec(db_, insert_sql, nullptr, nullptr, &err_msg);
  if (err_msg) { sqlite3_free(err_msg); err_msg = nullptr; }
  ASSERT_EQ(rc, SQLITE_OK);

  EXPECT_EQ(GetRowCount("notification_setting"), 1);
}

TEST_F(NotificationSettingIntegrationTest, CreateAndRetrieveSetting) {
  const char* insert_sql =
      "INSERT INTO notification_setting "
      "(uid, package_name, app_id, allow_to_notify, do_not_disturb_except, "
      "visibility_class, pop_up_notification, lock_screen_content_level) "
      "VALUES (5001, 'org.tizen.testpkg', 'org.tizen.testapp', 1, 0, 2, 1, 1);";

  char* err_msg = nullptr;
  int rc = sqlite3_exec(db_, insert_sql, nullptr, nullptr, &err_msg);
  if (err_msg) { sqlite3_free(err_msg); err_msg = nullptr; }
  ASSERT_EQ(rc, SQLITE_OK);

  // Retrieve the setting
  const char* select_sql =
      "SELECT package_name, app_id, allow_to_notify, visibility_class, "
      "lock_screen_content_level FROM notification_setting "
      "WHERE uid = 5001 AND app_id = 'org.tizen.testapp';";
  sqlite3_stmt* stmt = nullptr;
  rc = sqlite3_prepare_v2(db_, select_sql, -1, &stmt, nullptr);
  ASSERT_EQ(rc, SQLITE_OK);

  rc = sqlite3_step(stmt);
  ASSERT_EQ(rc, SQLITE_ROW);

  EXPECT_STREQ(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)),
               "org.tizen.testpkg");
  EXPECT_STREQ(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)),
               "org.tizen.testapp");
  EXPECT_EQ(sqlite3_column_int(stmt, 2), 1);  // allow_to_notify
  EXPECT_EQ(sqlite3_column_int(stmt, 3), 2);  // visibility_class
  EXPECT_EQ(sqlite3_column_int(stmt, 4), 1);  // lock_screen_content_level

  sqlite3_finalize(stmt);
}

TEST_F(NotificationSettingIntegrationTest, UpdateSettingValues) {
  // Insert initial setting
  const char* insert_sql =
      "INSERT INTO notification_setting "
      "(uid, package_name, app_id, allow_to_notify, do_not_disturb_except, "
      "visibility_class, pop_up_notification, lock_screen_content_level) "
      "VALUES (5001, 'org.tizen.testpkg', 'org.tizen.testapp', 1, 0, 0, 1, 0);";

  char* err_msg = nullptr;
  int rc = sqlite3_exec(db_, insert_sql, nullptr, nullptr, &err_msg);
  if (err_msg) { sqlite3_free(err_msg); err_msg = nullptr; }
  ASSERT_EQ(rc, SQLITE_OK);

  // Update setting
  const char* update_sql =
      "UPDATE notification_setting SET "
      "allow_to_notify = 0, do_not_disturb_except = 1, "
      "visibility_class = 3, pop_up_notification = 0 "
      "WHERE uid = 5001 AND app_id = 'org.tizen.testapp';";

  rc = sqlite3_exec(db_, update_sql, nullptr, nullptr, &err_msg);
  if (err_msg) { sqlite3_free(err_msg); err_msg = nullptr; }
  ASSERT_EQ(rc, SQLITE_OK);

  // Verify update
  const char* select_sql =
      "SELECT allow_to_notify, do_not_disturb_except, visibility_class, "
      "pop_up_notification FROM notification_setting "
      "WHERE uid = 5001 AND app_id = 'org.tizen.testapp';";
  sqlite3_stmt* stmt = nullptr;
  rc = sqlite3_prepare_v2(db_, select_sql, -1, &stmt, nullptr);
  ASSERT_EQ(rc, SQLITE_OK);

  rc = sqlite3_step(stmt);
  ASSERT_EQ(rc, SQLITE_ROW);

  EXPECT_EQ(sqlite3_column_int(stmt, 0), 0);  // allow_to_notify
  EXPECT_EQ(sqlite3_column_int(stmt, 1), 1);  // do_not_disturb_except
  EXPECT_EQ(sqlite3_column_int(stmt, 2), 3);  // visibility_class
  EXPECT_EQ(sqlite3_column_int(stmt, 3), 0);  // pop_up_notification

  sqlite3_finalize(stmt);
}

TEST_F(NotificationSettingIntegrationTest, DeleteSetting) {
  // Insert a setting
  const char* insert_sql =
      "INSERT INTO notification_setting "
      "(uid, package_name, app_id, allow_to_notify) "
      "VALUES (5001, 'org.tizen.testpkg', 'org.tizen.testapp', 1);";

  char* err_msg = nullptr;
  int rc = sqlite3_exec(db_, insert_sql, nullptr, nullptr, &err_msg);
  if (err_msg) { sqlite3_free(err_msg); err_msg = nullptr; }
  ASSERT_EQ(rc, SQLITE_OK);
  ASSERT_EQ(GetRowCount("notification_setting"), 1);

  // Delete the setting
  const char* delete_sql =
      "DELETE FROM notification_setting "
      "WHERE uid = 5001 AND app_id = 'org.tizen.testapp';";
  rc = sqlite3_exec(db_, delete_sql, nullptr, nullptr, &err_msg);
  if (err_msg) { sqlite3_free(err_msg); err_msg = nullptr; }
  ASSERT_EQ(rc, SQLITE_OK);

  EXPECT_EQ(GetRowCount("notification_setting"), 0);
}

TEST_F(NotificationSettingIntegrationTest, SystemSettingCreateAndUpdate) {
  // Insert system setting
  const char* insert_sql =
      "INSERT INTO notification_system_setting "
      "(uid, do_not_disturb, visibility_class, dnd_schedule_enabled, "
      "dnd_schedule_day, dnd_start_hour, dnd_start_min, "
      "dnd_end_hour, dnd_end_min, lock_screen_content_level) "
      "VALUES (5001, 0, 0, 0, 62, 22, 0, 8, 0, 0);";

  char* err_msg = nullptr;
  int rc = sqlite3_exec(db_, insert_sql, nullptr, nullptr, &err_msg);
  if (err_msg) { sqlite3_free(err_msg); err_msg = nullptr; }
  ASSERT_EQ(rc, SQLITE_OK);

  // Update do_not_disturb
  const char* update_sql =
      "UPDATE notification_system_setting SET "
      "do_not_disturb = 1, dnd_schedule_enabled = 1, "
      "dnd_start_hour = 23, dnd_end_hour = 7 "
      "WHERE uid = 5001;";

  rc = sqlite3_exec(db_, update_sql, nullptr, nullptr, &err_msg);
  if (err_msg) { sqlite3_free(err_msg); err_msg = nullptr; }
  ASSERT_EQ(rc, SQLITE_OK);

  // Verify
  const char* select_sql =
      "SELECT do_not_disturb, dnd_schedule_enabled, dnd_start_hour, "
      "dnd_end_hour FROM notification_system_setting WHERE uid = 5001;";
  sqlite3_stmt* stmt = nullptr;
  rc = sqlite3_prepare_v2(db_, select_sql, -1, &stmt, nullptr);
  ASSERT_EQ(rc, SQLITE_OK);

  rc = sqlite3_step(stmt);
  ASSERT_EQ(rc, SQLITE_ROW);

  EXPECT_EQ(sqlite3_column_int(stmt, 0), 1);   // do_not_disturb
  EXPECT_EQ(sqlite3_column_int(stmt, 1), 1);   // dnd_schedule_enabled
  EXPECT_EQ(sqlite3_column_int(stmt, 2), 23);  // dnd_start_hour
  EXPECT_EQ(sqlite3_column_int(stmt, 3), 7);   // dnd_end_hour

  sqlite3_finalize(stmt);
}

TEST_F(NotificationSettingIntegrationTest, MultipleAppSettings) {
  // Insert settings for multiple apps
  const char* insert_sql_1 =
      "INSERT INTO notification_setting "
      "(uid, package_name, app_id, allow_to_notify) "
      "VALUES (5001, 'org.tizen.pkg1', 'org.tizen.app1', 1);";
  const char* insert_sql_2 =
      "INSERT INTO notification_setting "
      "(uid, package_name, app_id, allow_to_notify) "
      "VALUES (5001, 'org.tizen.pkg2', 'org.tizen.app2', 0);";
  const char* insert_sql_3 =
      "INSERT INTO notification_setting "
      "(uid, package_name, app_id, allow_to_notify) "
      "VALUES (5001, 'org.tizen.pkg3', 'org.tizen.app3', 1);";

  char* err_msg = nullptr;
  int rc;

  rc = sqlite3_exec(db_, insert_sql_1, nullptr, nullptr, &err_msg);
  if (err_msg) { sqlite3_free(err_msg); err_msg = nullptr; }
  ASSERT_EQ(rc, SQLITE_OK);

  rc = sqlite3_exec(db_, insert_sql_2, nullptr, nullptr, &err_msg);
  if (err_msg) { sqlite3_free(err_msg); err_msg = nullptr; }
  ASSERT_EQ(rc, SQLITE_OK);

  rc = sqlite3_exec(db_, insert_sql_3, nullptr, nullptr, &err_msg);
  if (err_msg) { sqlite3_free(err_msg); err_msg = nullptr; }
  ASSERT_EQ(rc, SQLITE_OK);

  EXPECT_EQ(GetRowCount("notification_setting"), 3);

  // Query only apps that allow notification
  const char* select_sql =
      "SELECT COUNT(*) FROM notification_setting "
      "WHERE uid = 5001 AND allow_to_notify = 1;";
  sqlite3_stmt* stmt = nullptr;
  rc = sqlite3_prepare_v2(db_, select_sql, -1, &stmt, nullptr);
  ASSERT_EQ(rc, SQLITE_OK);
  ASSERT_EQ(sqlite3_step(stmt), SQLITE_ROW);
  EXPECT_EQ(sqlite3_column_int(stmt, 0), 2);
  sqlite3_finalize(stmt);
}

TEST_F(NotificationSettingIntegrationTest, UniqueConstraintEnforced) {
  // Insert a setting
  const char* insert_sql =
      "INSERT INTO notification_setting "
      "(uid, package_name, app_id, allow_to_notify) "
      "VALUES (5001, 'org.tizen.testpkg', 'org.tizen.testapp', 1);";

  char* err_msg = nullptr;
  int rc = sqlite3_exec(db_, insert_sql, nullptr, nullptr, &err_msg);
  if (err_msg) { sqlite3_free(err_msg); err_msg = nullptr; }
  ASSERT_EQ(rc, SQLITE_OK);

  // Try to insert a duplicate - should fail due to UNIQUE constraint
  rc = sqlite3_exec(db_, insert_sql, nullptr, nullptr, &err_msg);
  if (err_msg) { sqlite3_free(err_msg); err_msg = nullptr; }
  EXPECT_NE(rc, SQLITE_OK);

  // Should still have only 1 row
  EXPECT_EQ(GetRowCount("notification_setting"), 1);
}
