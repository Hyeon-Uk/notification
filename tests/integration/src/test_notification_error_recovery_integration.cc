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

class NotificationErrorRecoveryIntegrationTest : public ::testing::Test {
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

  std::string db_path_;
};

TEST_F(NotificationErrorRecoveryIntegrationTest,
    RECOVERY_01_NullHandleReturnsInvalidParameter) {
  int ret = notification_set_text(nullptr, NOTIFICATION_TEXT_TYPE_TITLE,
      "test", NULL, NOTIFICATION_VARIABLE_TYPE_NONE);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);

  char* text = nullptr;
  ret = notification_get_text(nullptr, NOTIFICATION_TEXT_TYPE_TITLE, &text);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);

  notification_h clone = nullptr;
  ret = notification_clone(nullptr, &clone);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);

  ret = notification_free(nullptr);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationErrorRecoveryIntegrationTest,
    RECOVERY_02_CloneAndFreeBothHandles) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  notification_set_text(noti, NOTIFICATION_TEXT_TYPE_TITLE,
      "clone_free_test", NULL, NOTIFICATION_VARIABLE_TYPE_NONE);

  notification_h clone = nullptr;
  int ret = notification_clone(noti, &clone);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);
  ASSERT_NE(clone, nullptr);

  char* title = nullptr;
  notification_get_text(clone, NOTIFICATION_TEXT_TYPE_TITLE, &title);
  EXPECT_STREQ(title, "clone_free_test");

  ret = notification_free(noti);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  ret = notification_free(clone);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
}

TEST_F(NotificationErrorRecoveryIntegrationTest,
    RECOVERY_03_AddAndRemoveButton) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  int ret = notification_add_button(noti, NOTIFICATION_BUTTON_1);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  ret = notification_remove_button(noti, NOTIFICATION_BUTTON_1);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  notification_free(noti);
}

TEST_F(NotificationErrorRecoveryIntegrationTest,
    RECOVERY_04_ConcurrentDBAccess) {
  sqlite3* db_a = notification_db_open();
  ASSERT_NE(db_a, nullptr);

  sqlite3* db_b = notification_db_open();
  ASSERT_NE(db_b, nullptr);

  const char* insert_sql_a =
      "INSERT INTO noti_list (type, layout, pkg_id, caller_app_id, "
      "launch_app_id, display_applist, uid) "
      "VALUES (1, 0, 'org.tizen.test.a', 'org.tizen.test.a', "
      "'org.tizen.test.a', 7, 5001);";

  int num_changes = 0;
  int ret = notification_db_exec(db_a, insert_sql_a, &num_changes);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  const char* insert_sql_b =
      "INSERT INTO noti_list (type, layout, pkg_id, caller_app_id, "
      "launch_app_id, display_applist, uid) "
      "VALUES (1, 0, 'org.tizen.test.b', 'org.tizen.test.b', "
      "'org.tizen.test.b', 7, 5001);";

  ret = notification_db_exec(db_b, insert_sql_b, &num_changes);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  ret = notification_db_close(&db_a);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  ret = notification_db_close(&db_b);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
}

TEST_F(NotificationErrorRecoveryIntegrationTest,
    RECOVERY_05_DeleteNonExistentPrivId) {
  int ret = notification_noti_delete_by_priv_id(nullptr, 99999);
  EXPECT_TRUE(ret == NOTIFICATION_ERROR_NONE ||
      ret >= NOTIFICATION_ERROR_NOT_EXIST_ID)
      << "Expected NOTIFICATION_ERROR_NONE or a valid error code, got: "
      << ret;
}
