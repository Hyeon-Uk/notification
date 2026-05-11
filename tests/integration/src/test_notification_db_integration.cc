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
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <unistd.h>
#include <sys/stat.h>
#include <tzplatform_config.h>

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
  static void MkdirP(const std::string& path) {
    size_t pos = 0;
    while ((pos = path.find('/', pos + 1)) != std::string::npos)
      mkdir(path.substr(0, pos).c_str(), 0755);
    mkdir(path.c_str(), 0755);
  }

  void SetUp() override {
    db_path_ = std::string(DBPATH);
    std::string db_dir = db_path_.substr(0, db_path_.rfind('/'));
    MkdirP(db_dir);

    if (access(db_dir.c_str(), W_OK) != 0)
      GTEST_SKIP() << "DB directory not writable: " << db_dir;

    unlink(db_path_.c_str());

    sqlite3* setup_db = nullptr;
    int rc = sqlite3_open(db_path_.c_str(), &setup_db);
    ASSERT_EQ(rc, SQLITE_OK);
    ASSERT_NE(setup_db, nullptr);

    char* err_msg = nullptr;
    rc = sqlite3_exec(setup_db, CREATE_NOTI_TABLE_SQL,
        nullptr, nullptr, &err_msg);
    if (err_msg) { sqlite3_free(err_msg); err_msg = nullptr; }
    ASSERT_EQ(rc, SQLITE_OK);

    rc = sqlite3_exec(setup_db, CREATE_SETTING_TABLE_SQL,
        nullptr, nullptr, &err_msg);
    if (err_msg) { sqlite3_free(err_msg); err_msg = nullptr; }
    ASSERT_EQ(rc, SQLITE_OK);

    rc = sqlite3_exec(setup_db, CREATE_SYSTEM_SETTING_TABLE_SQL,
        nullptr, nullptr, &err_msg);
    if (err_msg) { sqlite3_free(err_msg); err_msg = nullptr; }
    ASSERT_EQ(rc, SQLITE_OK);

    sqlite3_close(setup_db);
  }

  void TearDown() override {
    if (!db_path_.empty())
      unlink(db_path_.c_str());
  }

  int GetRowCount(const char* table_name) {
    sqlite3* db = nullptr;
    int rc = sqlite3_open(db_path_.c_str(), &db);
    if (rc != SQLITE_OK) return -1;

    std::string query =
        "SELECT COUNT(*) FROM " + std::string(table_name) + ";";
    sqlite3_stmt* stmt = nullptr;
    int count = -1;

    rc = sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr);
    if (rc == SQLITE_OK && sqlite3_step(stmt) == SQLITE_ROW)
      count = sqlite3_column_int(stmt, 0);
    if (stmt) sqlite3_finalize(stmt);
    sqlite3_close(db);
    return count;
  }

  std::string db_path_;
};

TEST_F(NotificationDBIntegrationTest, DbOpenAndClose) {
  sqlite3* db = notification_db_open();
  ASSERT_NE(db, nullptr);

  int ret = notification_db_close(&db);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(db, nullptr);
}

TEST_F(NotificationDBIntegrationTest, DbCloseNullParam) {
  int ret = notification_db_close(nullptr);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationDBIntegrationTest, DbExecWithRealQuery) {
  sqlite3* db = notification_db_open();
  ASSERT_NE(db, nullptr);

  const char* insert_sql =
      "INSERT INTO noti_list (type, layout, pkg_id, caller_app_id, "
      "launch_app_id, display_applist, uid) "
      "VALUES (1, 0, 'org.tizen.test', 'org.tizen.test', "
      "'org.tizen.test', 7, 5001);";

  int num_changes = 0;
  int ret = notification_db_exec(db, insert_sql, &num_changes);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  EXPECT_EQ(GetRowCount("noti_list"), 1);
  notification_db_close(&db);
}

TEST_F(NotificationDBIntegrationTest, DbExecNullParams) {
  int ret = notification_db_exec(nullptr, "SELECT 1;", nullptr);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);

  sqlite3* db = notification_db_open();
  ASSERT_NE(db, nullptr);
  ret = notification_db_exec(db, nullptr, nullptr);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
  notification_db_close(&db);
}

TEST_F(NotificationDBIntegrationTest, NotiInsertViaLibrary) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  notification_set_text(noti, NOTIFICATION_TEXT_TYPE_TITLE,
      "Integration Test", NULL, NOTIFICATION_VARIABLE_TYPE_NONE);
  notification_set_display_applist(noti,
      NOTIFICATION_DISPLAY_APP_NOTIFICATION_TRAY);

  int ret = notification_noti_insert(noti);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  EXPECT_GE(GetRowCount("noti_list"), 1);

  notification_free(noti);
}

TEST_F(NotificationDBIntegrationTest, NotiInsertAndUpdate) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  notification_set_text(noti, NOTIFICATION_TEXT_TYPE_TITLE,
      "Before Update", NULL, NOTIFICATION_VARIABLE_TYPE_NONE);

  int ret = notification_noti_insert(noti);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  notification_set_text(noti, NOTIFICATION_TEXT_TYPE_TITLE,
      "After Update", NULL, NOTIFICATION_VARIABLE_TYPE_NONE);

  ret = notification_noti_update(noti);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  notification_free(noti);
}

TEST_F(NotificationDBIntegrationTest, NotiInsertAndDeleteAll) {
  for (int i = 0; i < 5; i++) {
    notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
    ASSERT_NE(noti, nullptr);
    notification_set_display_applist(noti,
        NOTIFICATION_DISPLAY_APP_NOTIFICATION_TRAY);

    int ret = notification_noti_insert(noti);
    EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
    notification_free(noti);
  }

  EXPECT_EQ(GetRowCount("noti_list"), 5);

  int deleted_num = 0;
  int* deleted_list = nullptr;
  int ret = notification_noti_delete_all(NOTIFICATION_TYPE_NOTI,
      nullptr, &deleted_num, &deleted_list, 5001);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  if (deleted_list) free(deleted_list);
}

TEST_F(NotificationDBIntegrationTest, NotiGetAllCount) {
  for (int i = 0; i < 3; i++) {
    notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
    ASSERT_NE(noti, nullptr);
    notification_set_display_applist(noti,
        NOTIFICATION_DISPLAY_APP_NOTIFICATION_TRAY);
    notification_noti_insert(noti);
    notification_free(noti);
  }

  int count = 0;
  int ret = notification_noti_get_all_count(
      NOTIFICATION_TYPE_NOTI, &count, 5001);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(count, 3);
}

TEST_F(NotificationDBIntegrationTest, NotiGetGroupingList) {
  for (int i = 0; i < 3; i++) {
    notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
    ASSERT_NE(noti, nullptr);
    notification_set_display_applist(noti,
        NOTIFICATION_DISPLAY_APP_NOTIFICATION_TRAY);
    notification_noti_insert(noti);
    notification_free(noti);
  }

  notification_list_h list = nullptr;
  int list_count = 0;
  int ret = notification_noti_get_grouping_list(
      NOTIFICATION_TYPE_NOTI, 1, 10, &list, &list_count, 5001);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  EXPECT_GT(list_count, 0);

  if (list) notification_free_list(list);
}

TEST_F(NotificationDBIntegrationTest, MultipleInserts) {
  const int NUM_INSERTS = 20;

  for (int i = 0; i < NUM_INSERTS; i++) {
    notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
    ASSERT_NE(noti, nullptr);

    char tag[32];
    snprintf(tag, sizeof(tag), "batch_tag_%d", i);
    notification_set_tag(noti, tag);
    notification_set_display_applist(noti,
        NOTIFICATION_DISPLAY_APP_NOTIFICATION_TRAY);

    int ret = notification_noti_insert(noti);
    EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
    notification_free(noti);
  }

  EXPECT_EQ(GetRowCount("noti_list"), NUM_INSERTS);
}
