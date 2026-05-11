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
#include "notification/include/notification_type_internal.h"
#include "notification/include/notification_ongoing_flag.h"

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

class NotificationOngoingIntegrationTest : public ::testing::Test {
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

/* ONGOING_01 */
TEST_F(NotificationOngoingIntegrationTest, OngoingWithPercentageType) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_ONGOING);
  ASSERT_NE(noti, nullptr);

  int ret = notification_set_ongoing_value_type(noti,
      NOTIFICATION_ONGOING_VALUE_TYPE_PERCENT);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  ret = notification_set_ongoing_time(noti, 50, 100);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  notification_set_display_applist(noti,
      NOTIFICATION_DISPLAY_APP_NOTIFICATION_TRAY);

  ret = notification_noti_insert(noti);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  int group_id = 0, priv_id = 0;
  notification_get_id(noti, &group_id, &priv_id);
  ASSERT_GT(priv_id, 0);

  notification_h loaded = notification_create(NOTIFICATION_TYPE_ONGOING);
  ASSERT_NE(loaded, nullptr);
  ret = notification_noti_get_by_priv_id(loaded, priv_id);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  notification_ongoing_value_type_e type;
  ret = notification_get_ongoing_value_type(loaded, &type);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(type, NOTIFICATION_ONGOING_VALUE_TYPE_PERCENT);

  int current = 0, duration = 0;
  ret = notification_get_ongoing_time(loaded, &current, &duration);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(current, 50);
  EXPECT_EQ(duration, 100);

  notification_free(loaded);
  notification_free(noti);
}

/* ONGOING_02 */
TEST_F(NotificationOngoingIntegrationTest, UpdateProgressMultipleTimes) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_ONGOING);
  ASSERT_NE(noti, nullptr);

  notification_set_display_applist(noti,
      NOTIFICATION_DISPLAY_APP_NOTIFICATION_TRAY);

  int ret = notification_noti_insert(noti);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  int group_id = 0, priv_id = 0;
  notification_get_id(noti, &group_id, &priv_id);
  ASSERT_GT(priv_id, 0);

  /* Update to 0.25 */
  notification_set_progress(noti, 0.25);
  ret = notification_noti_update(noti);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  notification_h loaded = notification_create(NOTIFICATION_TYPE_ONGOING);
  ASSERT_NE(loaded, nullptr);
  ret = notification_noti_get_by_priv_id(loaded, priv_id);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);
  double progress = 0.0;
  notification_get_progress(loaded, &progress);
  EXPECT_DOUBLE_EQ(progress, 0.25);
  notification_free(loaded);

  /* Update to 0.75 */
  notification_set_progress(noti, 0.75);
  ret = notification_noti_update(noti);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  loaded = notification_create(NOTIFICATION_TYPE_ONGOING);
  ASSERT_NE(loaded, nullptr);
  ret = notification_noti_get_by_priv_id(loaded, priv_id);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);
  progress = 0.0;
  notification_get_progress(loaded, &progress);
  EXPECT_DOUBLE_EQ(progress, 0.75);
  notification_free(loaded);

  /* Update to 1.0 */
  notification_set_progress(noti, 1.0);
  ret = notification_noti_update(noti);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  loaded = notification_create(NOTIFICATION_TYPE_ONGOING);
  ASSERT_NE(loaded, nullptr);
  ret = notification_noti_get_by_priv_id(loaded, priv_id);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);
  progress = 0.0;
  notification_get_progress(loaded, &progress);
  EXPECT_DOUBLE_EQ(progress, 1.0);
  notification_free(loaded);

  notification_free(noti);
}

/* ONGOING_03 */
TEST_F(NotificationOngoingIntegrationTest, UpdateSizeMultipleTimes) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_ONGOING);
  ASSERT_NE(noti, nullptr);

  notification_set_display_applist(noti,
      NOTIFICATION_DISPLAY_APP_NOTIFICATION_TRAY);

  int ret = notification_noti_insert(noti);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  int group_id = 0, priv_id = 0;
  notification_get_id(noti, &group_id, &priv_id);
  ASSERT_GT(priv_id, 0);

  /* Update to 512.0 */
  notification_set_size(noti, 512.0);
  ret = notification_noti_update(noti);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  notification_h loaded = notification_create(NOTIFICATION_TYPE_ONGOING);
  ASSERT_NE(loaded, nullptr);
  ret = notification_noti_get_by_priv_id(loaded, priv_id);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);
  double size = 0.0;
  notification_get_size(loaded, &size);
  EXPECT_DOUBLE_EQ(size, 512.0);
  notification_free(loaded);

  /* Update to 2048.5 */
  notification_set_size(noti, 2048.5);
  ret = notification_noti_update(noti);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  loaded = notification_create(NOTIFICATION_TYPE_ONGOING);
  ASSERT_NE(loaded, nullptr);
  ret = notification_noti_get_by_priv_id(loaded, priv_id);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);
  size = 0.0;
  notification_get_size(loaded, &size);
  EXPECT_DOUBLE_EQ(size, 2048.5);
  notification_free(loaded);

  notification_free(noti);
}

/* ONGOING_04 */
TEST_F(NotificationOngoingIntegrationTest, OngoingFlagSetAndGet) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  int ret = notification_set_ongoing_flag(noti, true);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  bool ongoing_flag = false;
  ret = notification_get_ongoing_flag(noti, &ongoing_flag);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  EXPECT_TRUE(ongoing_flag);

  ret = notification_set_ongoing_flag(noti, false);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  ongoing_flag = true;
  ret = notification_get_ongoing_flag(noti, &ongoing_flag);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  EXPECT_FALSE(ongoing_flag);

  notification_free(noti);
}

/* ONGOING_05 */
TEST_F(NotificationOngoingIntegrationTest, OngoingWithTimeType) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_ONGOING);
  ASSERT_NE(noti, nullptr);

  int ret = notification_set_ongoing_value_type(noti,
      NOTIFICATION_ONGOING_VALUE_TYPE_TIME);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  ret = notification_set_ongoing_time(noti, 30, 120);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  notification_set_display_applist(noti,
      NOTIFICATION_DISPLAY_APP_NOTIFICATION_TRAY);

  ret = notification_noti_insert(noti);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  int group_id = 0, priv_id = 0;
  notification_get_id(noti, &group_id, &priv_id);
  ASSERT_GT(priv_id, 0);

  notification_h loaded = notification_create(NOTIFICATION_TYPE_ONGOING);
  ASSERT_NE(loaded, nullptr);
  ret = notification_noti_get_by_priv_id(loaded, priv_id);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  int current = 0, duration = 0;
  ret = notification_get_ongoing_time(loaded, &current, &duration);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(current, 30);
  EXPECT_EQ(duration, 120);

  notification_ongoing_value_type_e type;
  ret = notification_get_ongoing_value_type(loaded, &type);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(type, NOTIFICATION_ONGOING_VALUE_TYPE_TIME);

  notification_free(loaded);
  notification_free(noti);
}
