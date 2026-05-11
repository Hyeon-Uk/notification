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
#include "notification/include/notification_list.h"

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

class NotificationLifecycleIntegrationTest : public ::testing::Test {
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

  bool RowExistsByPrivId(int priv_id) {
    sqlite3* db = nullptr;
    int rc = sqlite3_open(db_path_.c_str(), &db);
    if (rc != SQLITE_OK) return false;

    std::string query =
        "SELECT COUNT(*) FROM noti_list WHERE priv_id = " +
        std::to_string(priv_id) + ";";
    sqlite3_stmt* stmt = nullptr;
    bool exists = false;

    rc = sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr);
    if (rc == SQLITE_OK && sqlite3_step(stmt) == SQLITE_ROW)
      exists = sqlite3_column_int(stmt, 0) > 0;
    if (stmt) sqlite3_finalize(stmt);
    sqlite3_close(db);
    return exists;
  }

  std::string db_path_;
};

TEST_F(NotificationLifecycleIntegrationTest, FullCreateInsertUpdateDeleteCycle) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  notification_set_text(noti, NOTIFICATION_TEXT_TYPE_TITLE,
      "Original Title", NULL, NOTIFICATION_VARIABLE_TYPE_NONE);
  notification_set_display_applist(noti,
      NOTIFICATION_DISPLAY_APP_NOTIFICATION_TRAY);

  int ret = notification_noti_insert(noti);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  int group_id = 0, priv_id = 0;
  notification_get_id(noti, &group_id, &priv_id);
  EXPECT_GT(priv_id, 0);
  EXPECT_TRUE(RowExistsByPrivId(priv_id));

  notification_set_text(noti, NOTIFICATION_TEXT_TYPE_TITLE,
      "Updated Title", NULL, NOTIFICATION_VARIABLE_TYPE_NONE);
  ret = notification_noti_update(noti);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  notification_h loaded = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(loaded, nullptr);
  ret = notification_noti_get_by_priv_id(loaded, priv_id);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  char* title = nullptr;
  notification_get_text(loaded, NOTIFICATION_TEXT_TYPE_TITLE, &title);
  EXPECT_STREQ(title, "Updated Title");
  notification_free(loaded);

  ret = notification_noti_delete_by_priv_id(nullptr, priv_id);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  EXPECT_FALSE(RowExistsByPrivId(priv_id));

  notification_free(noti);
}

TEST_F(NotificationLifecycleIntegrationTest, InsertAndLoadByPrivIdVerifiesFields) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  notification_set_text(noti, NOTIFICATION_TEXT_TYPE_TITLE,
      "priv_test", NULL, NOTIFICATION_VARIABLE_TYPE_NONE);
  notification_set_text(noti, NOTIFICATION_TEXT_TYPE_CONTENT,
      "content_body", NULL, NOTIFICATION_VARIABLE_TYPE_NONE);
  notification_set_sound(noti, NOTIFICATION_SOUND_TYPE_DEFAULT, NULL);
  notification_set_vibration(noti, NOTIFICATION_VIBRATION_TYPE_DEFAULT, NULL);
  notification_set_led(noti, NOTIFICATION_LED_OP_ON, 0);
  notification_set_led_time_period(noti, 500, 500);
  notification_set_display_applist(noti,
      NOTIFICATION_DISPLAY_APP_NOTIFICATION_TRAY);

  int ret = notification_noti_insert(noti);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  int group_id = 0, priv_id = 0;
  notification_get_id(noti, &group_id, &priv_id);
  ASSERT_GT(priv_id, 0);

  notification_h loaded = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(loaded, nullptr);
  ret = notification_noti_get_by_priv_id(loaded, priv_id);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  char* title = nullptr;
  notification_get_text(loaded, NOTIFICATION_TEXT_TYPE_TITLE, &title);
  EXPECT_STREQ(title, "priv_test");

  char* content = nullptr;
  notification_get_text(loaded, NOTIFICATION_TEXT_TYPE_CONTENT, &content);
  EXPECT_STREQ(content, "content_body");

  notification_sound_type_e snd_type;
  const char* snd_path = nullptr;
  notification_get_sound(loaded, &snd_type, &snd_path);
  EXPECT_EQ(snd_type, NOTIFICATION_SOUND_TYPE_DEFAULT);

  notification_vibration_type_e vib_type;
  const char* vib_path = nullptr;
  notification_get_vibration(loaded, &vib_type, &vib_path);
  EXPECT_EQ(vib_type, NOTIFICATION_VIBRATION_TYPE_DEFAULT);

  notification_led_op_e led_op;
  int led_argb = 0;
  notification_get_led(loaded, &led_op, &led_argb);
  EXPECT_EQ(led_op, NOTIFICATION_LED_OP_ON);

  int on_ms = 0, off_ms = 0;
  notification_get_led_time_period(loaded, &on_ms, &off_ms);
  EXPECT_EQ(on_ms, 500);
  EXPECT_EQ(off_ms, 500);

  notification_free(loaded);
  notification_free(noti);
}

TEST_F(NotificationLifecycleIntegrationTest, ClonePreservesAllFields) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  notification_set_text(noti, NOTIFICATION_TEXT_TYPE_TITLE,
      "clone_title", NULL, NOTIFICATION_VARIABLE_TYPE_NONE);
  notification_set_text(noti, NOTIFICATION_TEXT_TYPE_CONTENT,
      "clone_content", NULL, NOTIFICATION_VARIABLE_TYPE_NONE);
  notification_set_layout(noti, NOTIFICATION_LY_NOTI_EVENT_SINGLE);
  notification_set_sound(noti, NOTIFICATION_SOUND_TYPE_USER_DATA,
      "/opt/usr/share/test.wav");
  notification_set_led(noti, NOTIFICATION_LED_OP_ON_CUSTOM_COLOR, 0xFF0000);
  notification_set_led_time_period(noti, 300, 700);
  notification_set_tag(noti, "clone_tag");
  notification_set_auto_remove(noti, false);
  notification_set_display_applist(noti,
      NOTIFICATION_DISPLAY_APP_NOTIFICATION_TRAY |
      NOTIFICATION_DISPLAY_APP_TICKER);

  notification_h clone = nullptr;
  int ret = notification_clone(noti, &clone);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);
  ASSERT_NE(clone, nullptr);

  char* title = nullptr;
  notification_get_text(clone, NOTIFICATION_TEXT_TYPE_TITLE, &title);
  EXPECT_STREQ(title, "clone_title");

  char* content = nullptr;
  notification_get_text(clone, NOTIFICATION_TEXT_TYPE_CONTENT, &content);
  EXPECT_STREQ(content, "clone_content");

  notification_ly_type_e layout;
  notification_get_layout(clone, &layout);
  EXPECT_EQ(layout, NOTIFICATION_LY_NOTI_EVENT_SINGLE);

  notification_sound_type_e snd_type;
  const char* snd_path = nullptr;
  notification_get_sound(clone, &snd_type, &snd_path);
  EXPECT_EQ(snd_type, NOTIFICATION_SOUND_TYPE_USER_DATA);
  EXPECT_STREQ(snd_path, "/opt/usr/share/test.wav");

  notification_led_op_e led_op;
  int led_argb = 0;
  notification_get_led(clone, &led_op, &led_argb);
  EXPECT_EQ(led_op, NOTIFICATION_LED_OP_ON_CUSTOM_COLOR);
  EXPECT_EQ(led_argb, 0xFF0000);

  int on_ms = 0, off_ms = 0;
  notification_get_led_time_period(clone, &on_ms, &off_ms);
  EXPECT_EQ(on_ms, 300);
  EXPECT_EQ(off_ms, 700);

  const char* tag = nullptr;
  notification_get_tag(clone, &tag);
  EXPECT_STREQ(tag, "clone_tag");

  bool auto_rm = true;
  notification_get_auto_remove(clone, &auto_rm);
  EXPECT_FALSE(auto_rm);

  int applist = 0;
  notification_get_display_applist(clone, &applist);
  EXPECT_EQ(applist,
      NOTIFICATION_DISPLAY_APP_NOTIFICATION_TRAY |
      NOTIFICATION_DISPLAY_APP_TICKER);

  notification_free(clone);
  notification_free(noti);
}

TEST_F(NotificationLifecycleIntegrationTest, DeleteAllByTypeSelectiveRemoval) {
  for (int i = 0; i < 3; i++) {
    notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
    ASSERT_NE(noti, nullptr);
    notification_set_display_applist(noti,
        NOTIFICATION_DISPLAY_APP_NOTIFICATION_TRAY);
    EXPECT_EQ(notification_noti_insert(noti), NOTIFICATION_ERROR_NONE);
    notification_free(noti);
  }

  for (int i = 0; i < 2; i++) {
    notification_h noti = notification_create(NOTIFICATION_TYPE_ONGOING);
    ASSERT_NE(noti, nullptr);
    notification_set_display_applist(noti,
        NOTIFICATION_DISPLAY_APP_NOTIFICATION_TRAY);
    EXPECT_EQ(notification_noti_insert(noti), NOTIFICATION_ERROR_NONE);
    notification_free(noti);
  }

  EXPECT_EQ(GetRowCount("noti_list"), 5);

  int deleted_num = 0;
  int* deleted_list = nullptr;
  int ret = notification_noti_delete_all(NOTIFICATION_TYPE_NOTI,
      nullptr, &deleted_num, &deleted_list, 5001);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  if (deleted_list) free(deleted_list);

  int noti_count = 0;
  notification_noti_get_all_count(NOTIFICATION_TYPE_NOTI, &noti_count, 5001);
  EXPECT_EQ(noti_count, 0);

  int ongoing_count = 0;
  notification_noti_get_all_count(NOTIFICATION_TYPE_ONGOING,
      &ongoing_count, 5001);
  EXPECT_EQ(ongoing_count, 2);
}

TEST_F(NotificationLifecycleIntegrationTest, DeleteNotiLeavesOngoingIntact) {
  notification_h noti_handle = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti_handle, nullptr);
  notification_set_display_applist(noti_handle,
      NOTIFICATION_DISPLAY_APP_NOTIFICATION_TRAY);
  EXPECT_EQ(notification_noti_insert(noti_handle), NOTIFICATION_ERROR_NONE);
  int g = 0, noti_priv = 0;
  notification_get_id(noti_handle, &g, &noti_priv);
  notification_free(noti_handle);

  notification_h ongoing_handle = notification_create(NOTIFICATION_TYPE_ONGOING);
  ASSERT_NE(ongoing_handle, nullptr);
  notification_set_display_applist(ongoing_handle,
      NOTIFICATION_DISPLAY_APP_NOTIFICATION_TRAY);
  EXPECT_EQ(notification_noti_insert(ongoing_handle), NOTIFICATION_ERROR_NONE);
  int ongoing_priv = 0;
  notification_get_id(ongoing_handle, &g, &ongoing_priv);
  notification_free(ongoing_handle);

  int deleted_num = 0;
  int* deleted_list = nullptr;
  notification_noti_delete_all(NOTIFICATION_TYPE_NOTI,
      nullptr, &deleted_num, &deleted_list, 5001);
  if (deleted_list) free(deleted_list);

  notification_h check_ongoing = notification_create(NOTIFICATION_TYPE_ONGOING);
  ASSERT_NE(check_ongoing, nullptr);
  int ret = notification_noti_get_by_priv_id(check_ongoing, ongoing_priv);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  notification_free(check_ongoing);

  EXPECT_FALSE(RowExistsByPrivId(noti_priv));
}

TEST_F(NotificationLifecycleIntegrationTest, UpdateOngoingProgressSizeContent) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_ONGOING);
  ASSERT_NE(noti, nullptr);
  notification_set_display_applist(noti,
      NOTIFICATION_DISPLAY_APP_NOTIFICATION_TRAY);
  EXPECT_EQ(notification_noti_insert(noti), NOTIFICATION_ERROR_NONE);

  int g = 0, priv_id = 0;
  notification_get_id(noti, &g, &priv_id);
  ASSERT_GT(priv_id, 0);

  notification_set_progress(noti, 0.5);
  EXPECT_EQ(notification_noti_update(noti), NOTIFICATION_ERROR_NONE);

  notification_h loaded = notification_create(NOTIFICATION_TYPE_ONGOING);
  ASSERT_NE(loaded, nullptr);
  EXPECT_EQ(notification_noti_get_by_priv_id(loaded, priv_id),
      NOTIFICATION_ERROR_NONE);
  double progress = 0.0;
  notification_get_progress(loaded, &progress);
  EXPECT_DOUBLE_EQ(progress, 0.5);
  notification_free(loaded);

  notification_set_size(noti, 1024.0);
  EXPECT_EQ(notification_noti_update(noti), NOTIFICATION_ERROR_NONE);

  loaded = notification_create(NOTIFICATION_TYPE_ONGOING);
  ASSERT_NE(loaded, nullptr);
  EXPECT_EQ(notification_noti_get_by_priv_id(loaded, priv_id),
      NOTIFICATION_ERROR_NONE);
  double size = 0.0;
  notification_get_size(loaded, &size);
  EXPECT_DOUBLE_EQ(size, 1024.0);
  notification_free(loaded);

  notification_free(noti);
}

TEST_F(NotificationLifecycleIntegrationTest, FreeHandleDoesNotDeleteFromDB) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  notification_set_text(noti, NOTIFICATION_TEXT_TYPE_TITLE,
      "persist_test", NULL, NOTIFICATION_VARIABLE_TYPE_NONE);
  notification_set_display_applist(noti,
      NOTIFICATION_DISPLAY_APP_NOTIFICATION_TRAY);

  EXPECT_EQ(notification_noti_insert(noti), NOTIFICATION_ERROR_NONE);
  int g = 0, priv_id = 0;
  notification_get_id(noti, &g, &priv_id);
  ASSERT_GT(priv_id, 0);

  notification_free(noti);

  EXPECT_TRUE(RowExistsByPrivId(priv_id));

  notification_h loaded = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(loaded, nullptr);
  int ret = notification_noti_get_by_priv_id(loaded, priv_id);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  char* title = nullptr;
  notification_get_text(loaded, NOTIFICATION_TEXT_TYPE_TITLE, &title);
  EXPECT_STREQ(title, "persist_test");

  notification_free(loaded);
}
