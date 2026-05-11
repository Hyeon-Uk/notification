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
#include <tzplatform_config.h>

#include "notification/include/notification.h"
#include "notification/include/notification_internal.h"
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

}  // namespace

class NotificationTagIntegrationTest : public ::testing::Test {
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

    sqlite3_close(setup_db);
  }

  void TearDown() override {
    if (!db_path_.empty())
      unlink(db_path_.c_str());
  }

  int GetTagCount(const char* tag) {
    sqlite3* db = nullptr;
    int rc = sqlite3_open(db_path_.c_str(), &db);
    if (rc != SQLITE_OK) return -1;

    const char* query = "SELECT COUNT(*) FROM noti_list WHERE tag = ?;";
    sqlite3_stmt* stmt = nullptr;
    int count = -1;

    rc = sqlite3_prepare_v2(db, query, -1, &stmt, nullptr);
    if (rc == SQLITE_OK) {
      sqlite3_bind_text(stmt, 1, tag, -1, SQLITE_STATIC);
      if (sqlite3_step(stmt) == SQLITE_ROW)
        count = sqlite3_column_int(stmt, 0);
    }
    if (stmt) sqlite3_finalize(stmt);
    sqlite3_close(db);
    return count;
  }

  std::string db_path_;
};

/* TAG_01: Basic tag post and load */
TEST_F(NotificationTagIntegrationTest, TAG_01_BasicTagPostAndLoad) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  notification_set_tag(noti, "unique_tag");
  notification_set_text(noti, NOTIFICATION_TEXT_TYPE_TITLE,
      "tagged_noti", NULL, NOTIFICATION_VARIABLE_TYPE_NONE);
  notification_set_display_applist(noti,
      NOTIFICATION_DISPLAY_APP_NOTIFICATION_TRAY);

  int ret = notification_noti_insert(noti);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  notification_free(noti);

  notification_h loaded = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(loaded, nullptr);

  ret = notification_noti_get_by_tag(loaded,
      nullptr, const_cast<char*>("unique_tag"), 5001);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  char* title = nullptr;
  notification_get_text(loaded, NOTIFICATION_TEXT_TYPE_TITLE, &title);
  ASSERT_NE(title, nullptr);
  EXPECT_STREQ(title, "tagged_noti");

  notification_free(loaded);
}

/* TAG_02: Same tag triggers update-or-insert */
TEST_F(NotificationTagIntegrationTest, TAG_02_SameTagUpdateOrInsert) {
  notification_h noti_a = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti_a, nullptr);

  notification_set_tag(noti_a, "dup_tag");
  notification_set_text(noti_a, NOTIFICATION_TEXT_TYPE_TITLE,
      "first", NULL, NOTIFICATION_VARIABLE_TYPE_NONE);
  notification_set_display_applist(noti_a,
      NOTIFICATION_DISPLAY_APP_NOTIFICATION_TRAY);

  int ret = notification_noti_insert(noti_a);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  ret = notification_noti_check_tag(noti_a);
  notification_free(noti_a);

  notification_h noti_b = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti_b, nullptr);

  notification_set_tag(noti_b, "dup_tag");
  notification_set_text(noti_b, NOTIFICATION_TEXT_TYPE_TITLE,
      "second", NULL, NOTIFICATION_VARIABLE_TYPE_NONE);
  notification_set_display_applist(noti_b,
      NOTIFICATION_DISPLAY_APP_NOTIFICATION_TRAY);

  ret = notification_noti_check_tag(noti_b);
  ret = notification_noti_insert(noti_b);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  notification_free(noti_b);

  EXPECT_LE(GetTagCount("dup_tag"), 2);
}

/* TAG_03: Delete then load by tag fails */
TEST_F(NotificationTagIntegrationTest, TAG_03_DeleteThenLoadByTagFails) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  notification_set_tag(noti, "del_tag");
  notification_set_text(noti, NOTIFICATION_TEXT_TYPE_TITLE,
      "to_delete", NULL, NOTIFICATION_VARIABLE_TYPE_NONE);
  notification_set_display_applist(noti,
      NOTIFICATION_DISPLAY_APP_NOTIFICATION_TRAY);

  int ret = notification_noti_insert(noti);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  int group_id = 0;
  int priv_id = 0;
  notification_get_id(noti, &group_id, &priv_id);
  EXPECT_GT(priv_id, 0);
  notification_free(noti);

  ret = notification_noti_delete_by_priv_id(nullptr, priv_id);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  notification_h loaded = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(loaded, nullptr);

  ret = notification_noti_get_by_tag(loaded,
      nullptr, const_cast<char*>("del_tag"), 5001);
  EXPECT_NE(ret, NOTIFICATION_ERROR_NONE);

  notification_free(loaded);
}

/* TAG_04: Tag with special characters */
TEST_F(NotificationTagIntegrationTest, TAG_04_TagWithSpecialCharacters) {
  /* Sub-test A: tag with spaces */
  notification_h noti_spaces = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti_spaces, nullptr);

  notification_set_tag(noti_spaces, " spaces here ");
  notification_set_text(noti_spaces, NOTIFICATION_TEXT_TYPE_TITLE,
      "spaced_tag", NULL, NOTIFICATION_VARIABLE_TYPE_NONE);
  notification_set_display_applist(noti_spaces,
      NOTIFICATION_DISPLAY_APP_NOTIFICATION_TRAY);

  int ret = notification_noti_insert(noti_spaces);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  notification_free(noti_spaces);

  notification_h loaded_spaces = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(loaded_spaces, nullptr);

  ret = notification_noti_get_by_tag(loaded_spaces,
      nullptr, const_cast<char*>(" spaces here "), 5001);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  const char* ret_tag = nullptr;
  notification_get_tag(loaded_spaces, &ret_tag);
  ASSERT_NE(ret_tag, nullptr);
  EXPECT_STREQ(ret_tag, " spaces here ");
  notification_free(loaded_spaces);

  /* Sub-test B: tag with unicode (Korean characters) */
  notification_h noti_unicode = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti_unicode, nullptr);

  notification_set_tag(noti_unicode, "tag_\xed\x95\x9c\xea\xb8\x80");
  notification_set_text(noti_unicode, NOTIFICATION_TEXT_TYPE_TITLE,
      "unicode_tag", NULL, NOTIFICATION_VARIABLE_TYPE_NONE);
  notification_set_display_applist(noti_unicode,
      NOTIFICATION_DISPLAY_APP_NOTIFICATION_TRAY);

  ret = notification_noti_insert(noti_unicode);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  notification_free(noti_unicode);

  notification_h loaded_unicode = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(loaded_unicode, nullptr);

  ret = notification_noti_get_by_tag(loaded_unicode,
      nullptr, const_cast<char*>("tag_\xed\x95\x9c\xea\xb8\x80"), 5001);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  const char* ret_tag_unicode = nullptr;
  notification_get_tag(loaded_unicode, &ret_tag_unicode);
  ASSERT_NE(ret_tag_unicode, nullptr);
  EXPECT_STREQ(ret_tag_unicode, "tag_\xed\x95\x9c\xea\xb8\x80");
  notification_free(loaded_unicode);
}
