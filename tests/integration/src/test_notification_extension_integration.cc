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
#include <bundle.h>
#include <app_control.h>

#include "notification/include/notification.h"
#include "notification/include/notification_internal.h"
#include "notification/include/notification_type_internal.h"
#include "notification/include/notification_db.h"
#include "notification/include/notification_noti.h"

extern "C" void test_set_db_dir(const char* dir);

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

class NotificationExtensionIntegrationTest : public ::testing::Test {
 protected:
  void SetUp() override {
    char tmp_dir[] = "/tmp/noti_integ_extension_XXXXXX";
    if (!mkdtemp(tmp_dir))
      GTEST_SKIP() << "Failed to create temp dir";
    tmp_dir_ = tmp_dir;
    test_set_db_dir(tmp_dir_.c_str());
    db_path_ = std::string(DBPATH);

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
    if (!tmp_dir_.empty())
      rmdir(tmp_dir_.c_str());
  }

  std::string db_path_;
  std::string tmp_dir_;
};

TEST_F(NotificationExtensionIntegrationTest, EXTDATA_01_SetAndGetExtensionData) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  bundle* b = bundle_create();
  ASSERT_NE(b, nullptr);
  bundle_add_str(b, "data_key", "data_value");

  int ret = notification_set_extension_data(noti, "ext_key", b);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  notification_set_display_applist(noti,
      NOTIFICATION_DISPLAY_APP_NOTIFICATION_TRAY);

  ret = notification_noti_insert(noti);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  int group_id = 0, priv_id = 0;
  notification_get_id(noti, &group_id, &priv_id);
  ASSERT_GT(priv_id, 0);

  notification_h loaded = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(loaded, nullptr);
  ret = notification_noti_get_by_priv_id(loaded, priv_id);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  bundle* retrieved = nullptr;
  ret = notification_get_extension_data(loaded, "ext_key", &retrieved);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  ASSERT_NE(retrieved, nullptr);

  char* val = nullptr;
  bundle_get_str(retrieved, "data_key", &val);
  ASSERT_NE(val, nullptr);
  EXPECT_STREQ(val, "data_value");

  bundle_free(retrieved);
  bundle_free(b);
  notification_free(loaded);
  notification_free(noti);
}

TEST_F(NotificationExtensionIntegrationTest, EXTDATA_02_ExtensionImageSizeRoundTrip) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  int ret = notification_set_extension_image_size(noti, 256);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  notification_set_display_applist(noti,
      NOTIFICATION_DISPLAY_APP_NOTIFICATION_TRAY);

  ret = notification_noti_insert(noti);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  int group_id = 0, priv_id = 0;
  notification_get_id(noti, &group_id, &priv_id);
  ASSERT_GT(priv_id, 0);

  notification_h loaded = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(loaded, nullptr);
  ret = notification_noti_get_by_priv_id(loaded, priv_id);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  int height = 0;
  ret = notification_get_extension_image_size(loaded, &height);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(height, 256);

  notification_free(loaded);
  notification_free(noti);
}

TEST_F(NotificationExtensionIntegrationTest, EXTDATA_03_MultipleExtensionDataKeys) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  bundle* b_a = bundle_create();
  ASSERT_NE(b_a, nullptr);
  bundle_add_str(b_a, "data", "value_a");

  bundle* b_b = bundle_create();
  ASSERT_NE(b_b, nullptr);
  bundle_add_str(b_b, "data", "value_b");

  bundle* b_c = bundle_create();
  ASSERT_NE(b_c, nullptr);
  bundle_add_str(b_c, "data", "value_c");

  int ret = notification_set_extension_data(noti, "key_a", b_a);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  ret = notification_set_extension_data(noti, "key_b", b_b);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  ret = notification_set_extension_data(noti, "key_c", b_c);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  notification_set_display_applist(noti,
      NOTIFICATION_DISPLAY_APP_NOTIFICATION_TRAY);

  ret = notification_noti_insert(noti);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  int group_id = 0, priv_id = 0;
  notification_get_id(noti, &group_id, &priv_id);
  ASSERT_GT(priv_id, 0);

  notification_h loaded = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(loaded, nullptr);
  ret = notification_noti_get_by_priv_id(loaded, priv_id);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  bundle* got_a = nullptr;
  ret = notification_get_extension_data(loaded, "key_a", &got_a);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  ASSERT_NE(got_a, nullptr);
  char* val_a = nullptr;
  bundle_get_str(got_a, "data", &val_a);
  ASSERT_NE(val_a, nullptr);
  EXPECT_STREQ(val_a, "value_a");

  bundle* got_b = nullptr;
  ret = notification_get_extension_data(loaded, "key_b", &got_b);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  ASSERT_NE(got_b, nullptr);
  char* val_b = nullptr;
  bundle_get_str(got_b, "data", &val_b);
  ASSERT_NE(val_b, nullptr);
  EXPECT_STREQ(val_b, "value_b");

  bundle* got_c = nullptr;
  ret = notification_get_extension_data(loaded, "key_c", &got_c);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  ASSERT_NE(got_c, nullptr);
  char* val_c = nullptr;
  bundle_get_str(got_c, "data", &val_c);
  ASSERT_NE(val_c, nullptr);
  EXPECT_STREQ(val_c, "value_c");

  bundle_free(got_a);
  bundle_free(got_b);
  bundle_free(got_c);
  bundle_free(b_a);
  bundle_free(b_b);
  bundle_free(b_c);
  notification_free(loaded);
  notification_free(noti);
}

TEST_F(NotificationExtensionIntegrationTest,
    EXTDATA_04_ExtensionEventHandlerRoundTrip) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  app_control_h app_control = nullptr;
  int ac_ret = app_control_create(&app_control);
  if (ac_ret != APP_CONTROL_ERROR_NONE || !app_control)
    GTEST_SKIP() << "app_control_create requires app context";

  app_control_set_app_id(app_control, "com.test.handler");

  int ret = notification_set_extension_event_handler(noti,
      NOTIFICATION_EVENT_TYPE_HIDDEN_BY_TIMEOUT, app_control);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  app_control_h retrieved = nullptr;
  ret = notification_get_extension_event_handler(noti,
      NOTIFICATION_EVENT_TYPE_HIDDEN_BY_TIMEOUT, &retrieved);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  ASSERT_NE(retrieved, nullptr);

  char* app_id = nullptr;
  app_control_get_app_id(retrieved, &app_id);
  ASSERT_NE(app_id, nullptr);
  EXPECT_STREQ(app_id, "com.test.handler");

  free(app_id);
  app_control_destroy(retrieved);
  app_control_destroy(app_control);
  notification_free(noti);
}
