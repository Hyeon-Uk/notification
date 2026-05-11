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
#include "notification/include/notification_setting.h"
#include "notification/include/notification_setting_internal.h"
#include "notification/include/notification_setting_service.h"

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

}  // namespace

class NotificationUidIntegrationTest : public ::testing::Test {
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

    sqlite3_close(setup_db);

    notification_h test_noti = notification_create(NOTIFICATION_TYPE_NOTI);
    if (!test_noti)
      GTEST_SKIP() << "notification_create requires app context";
    notification_free(test_noti);
  }

  void TearDown() override {
    if (!db_path_.empty())
      unlink(db_path_.c_str());
  }

  std::string db_path_;
};

/* MULTIUSER_01: InsertAndLoadByTagForUid
 * Create noti with tag="uid_test", set uid=5000, insert.
 * Load by tag for uid=5000 and verify tag matches.
 */
TEST_F(NotificationUidIntegrationTest, MULTIUSER_01_InsertAndLoadByTagForUid) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  notification_set_tag(noti, "uid_test");
  notification_set_uid(noti, 5000);
  notification_set_display_applist(noti,
      NOTIFICATION_DISPLAY_APP_NOTIFICATION_TRAY);

  int ret = notification_noti_insert(noti);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  notification_free(noti);

  notification_h loaded = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(loaded, nullptr);

  ret = notification_noti_get_by_tag(loaded,
      nullptr, const_cast<char*>("uid_test"), 5000);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  const char* tag = nullptr;
  notification_get_tag(loaded, &tag);
  ASSERT_NE(tag, nullptr);
  EXPECT_STREQ(tag, "uid_test");

  notification_free(loaded);
}

/* MULTIUSER_02: LoadByTagForDifferentUidFails
 * Create noti with tag="uid_iso", uid=5000, insert.
 * Try to load by tag for uid=5001 — expect error.
 */
TEST_F(NotificationUidIntegrationTest,
    MULTIUSER_02_LoadByTagForDifferentUidFails) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  notification_set_tag(noti, "uid_iso");
  notification_set_uid(noti, 5000);
  notification_set_display_applist(noti,
      NOTIFICATION_DISPLAY_APP_NOTIFICATION_TRAY);

  int ret = notification_noti_insert(noti);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  notification_free(noti);

  notification_h loaded = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(loaded, nullptr);

  ret = notification_noti_get_by_tag(loaded,
      nullptr, const_cast<char*>("uid_iso"), 5001);
  EXPECT_NE(ret, NOTIFICATION_ERROR_NONE);

  notification_free(loaded);
}

/* MULTIUSER_03: DeleteAllForUidOnlyAffectsThatUid
 * Insert 3 notis with uid=5000, 2 notis with uid=5001.
 * Delete all for uid=5000. Count for 5000=0, count for 5001=2.
 */
TEST_F(NotificationUidIntegrationTest,
    MULTIUSER_03_DeleteAllForUidOnlyAffectsThatUid) {
  for (int i = 0; i < 3; i++) {
    notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
    ASSERT_NE(noti, nullptr);
    notification_set_uid(noti, 5000);
    notification_set_display_applist(noti,
        NOTIFICATION_DISPLAY_APP_NOTIFICATION_TRAY);
    int ret = notification_noti_insert(noti);
    EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
    notification_free(noti);
  }

  for (int i = 0; i < 2; i++) {
    notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
    ASSERT_NE(noti, nullptr);
    notification_set_uid(noti, 5001);
    notification_set_display_applist(noti,
        NOTIFICATION_DISPLAY_APP_NOTIFICATION_TRAY);
    int ret = notification_noti_insert(noti);
    EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
    notification_free(noti);
  }

  int deleted_num = 0;
  int* deleted_list = nullptr;
  int ret = notification_noti_delete_all(NOTIFICATION_TYPE_NOTI,
      nullptr, &deleted_num, &deleted_list, 5000);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  if (deleted_list) free(deleted_list);

  int count_5000 = 0;
  ret = notification_noti_get_all_count(NOTIFICATION_TYPE_NOTI,
      &count_5000, 5000);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(count_5000, 0);

  int count_5001 = 0;
  ret = notification_noti_get_all_count(NOTIFICATION_TYPE_NOTI,
      &count_5001, 5001);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(count_5001, 2);
}

/* MULTIUSER_04: SettingsPerUidIsolation
 * Insert setting for "com.pkg.a" uid=5000 allow=1,
 * then for "com.pkg.a" uid=5001 allow=0.
 * Verify each uid sees its own allow value.
 */
TEST_F(NotificationUidIntegrationTest,
    MULTIUSER_04_SettingsPerUidIsolation) {
  int ret = notification_setting_db_update(
      "com.pkg.a", "com.pkg.a",
      1, 0, 0, 1, 0, 5000);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  ret = notification_setting_db_update(
      "com.pkg.a", "com.pkg.a",
      0, 0, 0, 1, 0, 5001);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  notification_setting_h setting_5000 = nullptr;
  ret = noti_setting_service_get_setting_by_app_id(
      "com.pkg.a", &setting_5000, 5000);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  if (setting_5000) {
    bool allow = false;
    ret = notification_setting_get_allow_to_notify(setting_5000, &allow);
    EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
    EXPECT_TRUE(allow);
    notification_setting_free_notification(setting_5000);
  }

  notification_setting_h setting_5001 = nullptr;
  ret = noti_setting_service_get_setting_by_app_id(
      "com.pkg.a", &setting_5001, 5001);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  if (setting_5001) {
    bool allow = true;
    ret = notification_setting_get_allow_to_notify(setting_5001, &allow);
    EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
    EXPECT_FALSE(allow);
    notification_setting_free_notification(setting_5001);
  }
}
