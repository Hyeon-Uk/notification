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
#include "notification/include/notification_db.h"
#include "notification/include/notification_noti.h"

namespace {

const char* CREATE_NOTI_TABLE_SQL =
    "CREATE TABLE IF NOT EXISTS noti_list ("
    "  type INTEGER NOT NULL,"
    "  layout INTEGER NOT NULL DEFAULT 0,"
    "  pkg_id TEXT NOT NULL,"
    "  caller_app_id TEXT NOT NULL,"
    "  launch_app_id TEXT,"
    "  app_label TEXT,"
    "  image_path TEXT,"
    "  priv_image_path TEXT,"
    "  group_id INTEGER DEFAULT 0,"
    "  internal_group_id INTEGER DEFAULT 0,"
    "  priv_id INTEGER PRIMARY KEY AUTOINCREMENT,"
    "  title_key TEXT,"
    "  b_text TEXT,"
    "  b_key TEXT,"
    "  tag TEXT,"
    "  b_format_args TEXT,"
    "  num_format_args INTEGER DEFAULT 0,"
    "  text_domain TEXT,"
    "  text_dir TEXT,"
    "  time INTEGER DEFAULT 0,"
    "  insert_time INTEGER DEFAULT 0,"
    "  args TEXT,"
    "  group_args TEXT,"
    "  b_execute_option TEXT,"
    "  b_service_responding TEXT,"
    "  b_service_single_launch TEXT,"
    "  b_service_multi_launch TEXT,"
    "  b_event_handler_click_on_button_1 TEXT,"
    "  b_event_handler_click_on_button_2 TEXT,"
    "  b_event_handler_click_on_button_3 TEXT,"
    "  b_event_handler_click_on_button_4 TEXT,"
    "  b_event_handler_click_on_button_5 TEXT,"
    "  b_event_handler_click_on_button_6 TEXT,"
    "  b_event_handler_click_on_icon TEXT,"
    "  b_event_handler_click_on_thumbnail TEXT,"
    "  b_event_handler_click_on_text_input_button TEXT,"
    "  b_event_handler_click_on_button_7 TEXT,"
    "  b_event_handler_click_on_button_8 TEXT,"
    "  b_event_handler_click_on_button_9 TEXT,"
    "  b_event_handler_click_on_button_10 TEXT,"
    "  sound_type INTEGER DEFAULT 0,"
    "  sound_path TEXT,"
    "  priv_sound_path TEXT,"
    "  vibration_type INTEGER DEFAULT 0,"
    "  vibration_path TEXT,"
    "  priv_vibration_path TEXT,"
    "  led_operation INTEGER DEFAULT 0,"
    "  led_argb INTEGER DEFAULT 0,"
    "  led_on_ms INTEGER DEFAULT -1,"
    "  led_off_ms INTEGER DEFAULT -1,"
    "  flags_for_property INTEGER DEFAULT 0,"
    "  flag_simmode INTEGER DEFAULT 0,"
    "  display_applist INTEGER,"
    "  progress_size DOUBLE DEFAULT 0,"
    "  progress_percentage DOUBLE DEFAULT 0,"
    "  ongoing_flag INTEGER DEFAULT 0,"
    "  ongoing_value_type INTEGER DEFAULT 0,"
    "  ongoing_current INTEGER DEFAULT 0,"
    "  ongoing_duration INTEGER DEFAULT 0,"
    "  auto_remove INTEGER DEFAULT 1,"
    "  default_button_index INTEGER DEFAULT 0,"
    "  hide_timeout INTEGER DEFAULT 0,"
    "  delete_timeout INTEGER DEFAULT 0,"
    "  text_input_max_length INTEGER DEFAULT 0,"
    "  event_flag INTEGER DEFAULT 0,"
    "  extension_image_size INTEGER DEFAULT 0,"
    "  uid INTEGER,"
    "  check_box INTEGER DEFAULT 0,"
    "  check_box_value INTEGER DEFAULT 0,"
    "  channel_name TEXT);";

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

}  // namespace

class NotificationDBIntegrationTest : public ::testing::Test {
 protected:
  void SetUp() override {
    char tmpl[] = "/tmp/noti_db_test_XXXXXX";
    char* dir = mkdtemp(tmpl);
    ASSERT_NE(dir, nullptr);
    tmp_dir_ = std::string(dir);
    db_path_ = tmp_dir_ + "/noti.db";

    int rc = sqlite3_open(db_path_.c_str(), &db_);
    ASSERT_EQ(rc, SQLITE_OK);
    ASSERT_NE(db_, nullptr);

    // Create the schema tables
    char* err_msg = nullptr;
    rc = sqlite3_exec(db_, CREATE_NOTI_TABLE_SQL, nullptr, nullptr, &err_msg);
    if (err_msg) {
      sqlite3_free(err_msg);
      err_msg = nullptr;
    }
    ASSERT_EQ(rc, SQLITE_OK);

    rc = sqlite3_exec(db_, CREATE_SETTING_TABLE_SQL, nullptr, nullptr, &err_msg);
    if (err_msg) {
      sqlite3_free(err_msg);
      err_msg = nullptr;
    }
    ASSERT_EQ(rc, SQLITE_OK);

    rc = sqlite3_exec(db_, CREATE_SYSTEM_SETTING_TABLE_SQL, nullptr, nullptr, &err_msg);
    if (err_msg) {
      sqlite3_free(err_msg);
      err_msg = nullptr;
    }
    ASSERT_EQ(rc, SQLITE_OK);
  }

  void TearDown() override {
    if (db_) {
      sqlite3_close(db_);
      db_ = nullptr;
    }
    if (!tmp_dir_.empty()) {
      // Clean up temporary test directory
      std::string db_file = tmp_dir_ + "/noti.db";
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

TEST_F(NotificationDBIntegrationTest, OpenAndCloseDB) {
  // Verify the DB was opened successfully in SetUp
  ASSERT_NE(db_, nullptr);

  // Verify we can execute a simple query
  sqlite3_stmt* stmt = nullptr;
  int rc = sqlite3_prepare_v2(db_, "SELECT 1;", -1, &stmt, nullptr);
  ASSERT_EQ(rc, SQLITE_OK);
  ASSERT_NE(stmt, nullptr);

  rc = sqlite3_step(stmt);
  EXPECT_EQ(rc, SQLITE_ROW);
  EXPECT_EQ(sqlite3_column_int(stmt, 0), 1);

  sqlite3_finalize(stmt);
}

TEST_F(NotificationDBIntegrationTest, ExecWithRealSQL) {
  // Insert a notification record directly via SQL
  const char* insert_sql =
      "INSERT INTO noti_list (type, layout, pkg_id, caller_app_id, "
      "launch_app_id, display_applist, uid) "
      "VALUES (1, 0, 'org.tizen.test', 'org.tizen.test', "
      "'org.tizen.test', 7, 5001);";

  char* err_msg = nullptr;
  int rc = sqlite3_exec(db_, insert_sql, nullptr, nullptr, &err_msg);
  if (err_msg) {
    sqlite3_free(err_msg);
    err_msg = nullptr;
  }
  ASSERT_EQ(rc, SQLITE_OK);

  EXPECT_EQ(GetRowCount("noti_list"), 1);
}

TEST_F(NotificationDBIntegrationTest, InsertAndQueryNotification) {
  // Insert multiple notification records
  const char* insert_sql_fmt =
      "INSERT INTO noti_list (type, layout, pkg_id, caller_app_id, "
      "launch_app_id, tag, display_applist, uid) "
      "VALUES (%d, 0, 'org.tizen.test', 'org.tizen.test', "
      "'org.tizen.test', '%s', 7, 5001);";

  char query[512];
  char* err_msg = nullptr;

  snprintf(query, sizeof(query), insert_sql_fmt, 1, "tag_one");
  int rc = sqlite3_exec(db_, query, nullptr, nullptr, &err_msg);
  if (err_msg) { sqlite3_free(err_msg); err_msg = nullptr; }
  ASSERT_EQ(rc, SQLITE_OK);

  snprintf(query, sizeof(query), insert_sql_fmt, 2, "tag_two");
  rc = sqlite3_exec(db_, query, nullptr, nullptr, &err_msg);
  if (err_msg) { sqlite3_free(err_msg); err_msg = nullptr; }
  ASSERT_EQ(rc, SQLITE_OK);

  EXPECT_EQ(GetRowCount("noti_list"), 2);

  // Query by tag
  const char* select_sql =
      "SELECT priv_id, type, tag FROM noti_list WHERE tag = 'tag_one';";
  sqlite3_stmt* stmt = nullptr;
  rc = sqlite3_prepare_v2(db_, select_sql, -1, &stmt, nullptr);
  ASSERT_EQ(rc, SQLITE_OK);

  rc = sqlite3_step(stmt);
  ASSERT_EQ(rc, SQLITE_ROW);
  EXPECT_EQ(sqlite3_column_int(stmt, 1), 1);  // type
  EXPECT_STREQ(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)), "tag_one");

  sqlite3_finalize(stmt);
}

TEST_F(NotificationDBIntegrationTest, DeleteNotification) {
  // Insert a record
  const char* insert_sql =
      "INSERT INTO noti_list (type, layout, pkg_id, caller_app_id, "
      "launch_app_id, display_applist, uid) "
      "VALUES (1, 0, 'org.tizen.test', 'org.tizen.test', "
      "'org.tizen.test', 7, 5001);";

  char* err_msg = nullptr;
  int rc = sqlite3_exec(db_, insert_sql, nullptr, nullptr, &err_msg);
  if (err_msg) { sqlite3_free(err_msg); err_msg = nullptr; }
  ASSERT_EQ(rc, SQLITE_OK);
  ASSERT_EQ(GetRowCount("noti_list"), 1);

  // Get the priv_id
  int priv_id = static_cast<int>(sqlite3_last_insert_rowid(db_));
  ASSERT_GT(priv_id, 0);

  // Delete by priv_id
  char delete_sql[256];
  snprintf(delete_sql, sizeof(delete_sql),
           "DELETE FROM noti_list WHERE priv_id = %d;", priv_id);
  rc = sqlite3_exec(db_, delete_sql, nullptr, nullptr, &err_msg);
  if (err_msg) { sqlite3_free(err_msg); err_msg = nullptr; }
  ASSERT_EQ(rc, SQLITE_OK);

  EXPECT_EQ(GetRowCount("noti_list"), 0);
}

TEST_F(NotificationDBIntegrationTest, MultipleConcurrentInserts) {
  const int NUM_INSERTS = 50;
  char query[512];
  char* err_msg = nullptr;

  // Begin transaction for batch insert
  int rc = sqlite3_exec(db_, "BEGIN TRANSACTION;", nullptr, nullptr, &err_msg);
  if (err_msg) { sqlite3_free(err_msg); err_msg = nullptr; }
  ASSERT_EQ(rc, SQLITE_OK);

  for (int i = 0; i < NUM_INSERTS; i++) {
    snprintf(query, sizeof(query),
             "INSERT INTO noti_list (type, layout, pkg_id, caller_app_id, "
             "launch_app_id, tag, display_applist, uid) "
             "VALUES (1, 0, 'org.tizen.test', 'org.tizen.test', "
             "'org.tizen.test', 'tag_%d', 7, 5001);", i);
    rc = sqlite3_exec(db_, query, nullptr, nullptr, &err_msg);
    if (err_msg) { sqlite3_free(err_msg); err_msg = nullptr; }
    ASSERT_EQ(rc, SQLITE_OK);
  }

  rc = sqlite3_exec(db_, "COMMIT;", nullptr, nullptr, &err_msg);
  if (err_msg) { sqlite3_free(err_msg); err_msg = nullptr; }
  ASSERT_EQ(rc, SQLITE_OK);

  EXPECT_EQ(GetRowCount("noti_list"), NUM_INSERTS);

  // Verify we can query individual records
  sqlite3_stmt* stmt = nullptr;
  rc = sqlite3_prepare_v2(db_,
      "SELECT COUNT(*) FROM noti_list WHERE tag LIKE 'tag_%';",
      -1, &stmt, nullptr);
  ASSERT_EQ(rc, SQLITE_OK);
  ASSERT_EQ(sqlite3_step(stmt), SQLITE_ROW);
  EXPECT_EQ(sqlite3_column_int(stmt, 0), NUM_INSERTS);
  sqlite3_finalize(stmt);
}

TEST_F(NotificationDBIntegrationTest, UpdateNotification) {
  // Insert a record
  const char* insert_sql =
      "INSERT INTO noti_list (type, layout, pkg_id, caller_app_id, "
      "launch_app_id, tag, display_applist, uid) "
      "VALUES (1, 0, 'org.tizen.test', 'org.tizen.test', "
      "'org.tizen.test', 'update_test', 7, 5001);";

  char* err_msg = nullptr;
  int rc = sqlite3_exec(db_, insert_sql, nullptr, nullptr, &err_msg);
  if (err_msg) { sqlite3_free(err_msg); err_msg = nullptr; }
  ASSERT_EQ(rc, SQLITE_OK);

  int priv_id = static_cast<int>(sqlite3_last_insert_rowid(db_));

  // Update the record
  char update_sql[512];
  snprintf(update_sql, sizeof(update_sql),
           "UPDATE noti_list SET type = 2, tag = 'updated_tag' "
           "WHERE priv_id = %d;", priv_id);
  rc = sqlite3_exec(db_, update_sql, nullptr, nullptr, &err_msg);
  if (err_msg) { sqlite3_free(err_msg); err_msg = nullptr; }
  ASSERT_EQ(rc, SQLITE_OK);

  // Verify update
  char select_sql[256];
  snprintf(select_sql, sizeof(select_sql),
           "SELECT type, tag FROM noti_list WHERE priv_id = %d;", priv_id);
  sqlite3_stmt* stmt = nullptr;
  rc = sqlite3_prepare_v2(db_, select_sql, -1, &stmt, nullptr);
  ASSERT_EQ(rc, SQLITE_OK);
  ASSERT_EQ(sqlite3_step(stmt), SQLITE_ROW);
  EXPECT_EQ(sqlite3_column_int(stmt, 0), 2);
  EXPECT_STREQ(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)),
               "updated_tag");
  sqlite3_finalize(stmt);
}

TEST_F(NotificationDBIntegrationTest, InsertAndDeleteAll) {
  char query[512];
  char* err_msg = nullptr;

  // Insert several records
  for (int i = 0; i < 10; i++) {
    snprintf(query, sizeof(query),
             "INSERT INTO noti_list (type, layout, pkg_id, caller_app_id, "
             "launch_app_id, display_applist, uid) "
             "VALUES (1, 0, 'org.tizen.test', 'org.tizen.test', "
             "'org.tizen.test', 7, 5001);");
    int rc = sqlite3_exec(db_, query, nullptr, nullptr, &err_msg);
    if (err_msg) { sqlite3_free(err_msg); err_msg = nullptr; }
    ASSERT_EQ(rc, SQLITE_OK);
  }
  ASSERT_EQ(GetRowCount("noti_list"), 10);

  // Delete all by caller_app_id
  const char* delete_sql =
      "DELETE FROM noti_list WHERE caller_app_id = 'org.tizen.test';";
  int rc = sqlite3_exec(db_, delete_sql, nullptr, nullptr, &err_msg);
  if (err_msg) { sqlite3_free(err_msg); err_msg = nullptr; }
  ASSERT_EQ(rc, SQLITE_OK);

  EXPECT_EQ(GetRowCount("noti_list"), 0);
}
