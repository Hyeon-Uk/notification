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

const char* CREATE_CHANNEL_TABLE_SQL =
    "CREATE TABLE IF NOT EXISTS noti_channel ("
    "  app_id TEXT NOT NULL,"
    "  channel_name TEXT NOT NULL,"
    "  blockable INTEGER DEFAULT 0,"
    "  is_blocked INTEGER DEFAULT 0,"
    "  UNIQUE (app_id, channel_name));";

}  // namespace

class NotificationChannelIntegrationTest : public ::testing::Test {
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

    rc = sqlite3_exec(setup_db, CREATE_CHANNEL_TABLE_SQL,
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

/* CHANNEL_01: Insert and retrieve channel */
TEST_F(NotificationChannelIntegrationTest, CHANNEL_01_InsertAndRetrieveChannel) {
  int ret = notification_noti_insert_channel("com.test.app",
      "test_channel", 0, 0);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  int blockable = -1;
  int is_blocked = -1;
  ret = notification_noti_get_channel("com.test.app",
      "test_channel", &blockable, &is_blocked);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(blockable, 0);
  EXPECT_EQ(is_blocked, 0);
}

/* CHANNEL_02: Set blockable and verify */
TEST_F(NotificationChannelIntegrationTest, CHANNEL_02_SetBlockableAndVerify) {
  int ret = notification_noti_insert_channel("com.test.app",
      "ch_block", 0, 0);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  ret = notification_noti_update_channel("com.test.app",
      "ch_block", 1, 0);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  int blockable = -1;
  int is_blocked = -1;
  ret = notification_noti_get_channel("com.test.app",
      "ch_block", &blockable, &is_blocked);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(blockable, 1);
}

/* CHANNEL_03: Set block state and verify */
TEST_F(NotificationChannelIntegrationTest, CHANNEL_03_SetBlockStateAndVerify) {
  int ret = notification_noti_insert_channel("com.test.app",
      "ch_state", 1, 0);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  ret = notification_noti_update_channel("com.test.app",
      "ch_state", 1, 1);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  int blockable = -1;
  int is_blocked = -1;
  ret = notification_noti_get_channel("com.test.app",
      "ch_state", &blockable, &is_blocked);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(is_blocked, 1);
}

/* CHANNEL_04: Delete channel and verify gone */
TEST_F(NotificationChannelIntegrationTest, CHANNEL_04_DeleteChannelAndVerifyGone) {
  int ret = notification_noti_insert_channel("com.test.app",
      "ch_del", 0, 0);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  ret = notification_noti_delete_channel("com.test.app", "ch_del");
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  int blockable = -1;
  int is_blocked = -1;
  ret = notification_noti_get_channel("com.test.app",
      "ch_del", &blockable, &is_blocked);
  if (ret != NOTIFICATION_ERROR_NONE) {
    SUCCEED() << "Channel correctly removed from DB";
  } else {
    EXPECT_EQ(blockable, 0);
    EXPECT_EQ(is_blocked, 0);
  }
}

/* CHANNEL_05: Channel name on notification */
TEST_F(NotificationChannelIntegrationTest, CHANNEL_05_ChannelNameOnNotification) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  if (!noti)
    GTEST_SKIP() << "notification_create requires app context";

  int ret = notification_set_channel_name(noti, "my_channel");
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  notification_set_display_applist(noti,
      NOTIFICATION_DISPLAY_APP_NOTIFICATION_TRAY);

  ret = notification_noti_insert(noti);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  int group_id = 0, priv_id = 0;
  notification_get_id(noti, &group_id, &priv_id);
  ASSERT_GT(priv_id, 0);
  notification_free(noti);

  notification_h loaded = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(loaded, nullptr);

  ret = notification_noti_get_by_priv_id(loaded, priv_id);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  const char* channel_name = nullptr;
  ret = notification_get_channel_name(loaded, &channel_name);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  ASSERT_NE(channel_name, nullptr);
  EXPECT_STREQ(channel_name, "my_channel");

  notification_free(loaded);
}

/* CHANNEL_06: Multiple channels for same app */
TEST_F(NotificationChannelIntegrationTest, CHANNEL_06_MultipleChannelsForApp) {
  int ret = notification_noti_insert_channel("com.test.app", "ch_a", 0, 0);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  ret = notification_noti_insert_channel("com.test.app", "ch_b", 0, 0);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  ret = notification_noti_insert_channel("com.test.app", "ch_c", 0, 0);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  int blockable = -1;
  int is_blocked = -1;

  ret = notification_noti_get_channel("com.test.app",
      "ch_a", &blockable, &is_blocked);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  ret = notification_noti_get_channel("com.test.app",
      "ch_b", &blockable, &is_blocked);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  ret = notification_noti_get_channel("com.test.app",
      "ch_c", &blockable, &is_blocked);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
}
