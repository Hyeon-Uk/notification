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

class NotificationEventIntegrationTest : public ::testing::Test {
 protected:
  void SetUp() override {
    char tmp_dir[] = "/tmp/noti_integ_event_XXXXXX";
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

/* EVENT_01: Set event handlers for buttons 1 and 2, insert to DB, reload,
   verify the handlers are preserved. */
TEST_F(NotificationEventIntegrationTest,
    EVENT_01_SetEventHandlersAndVerifyStored) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  app_control_h ac1 = nullptr;
  int ac_ret = app_control_create(&ac1);
  if (ac_ret != APP_CONTROL_ERROR_NONE || !ac1) {
    notification_free(noti);
    GTEST_SKIP() << "app_control_create requires app context";
  }
  app_control_set_app_id(ac1, "com.test.button1");

  app_control_h ac2 = nullptr;
  ac_ret = app_control_create(&ac2);
  ASSERT_EQ(ac_ret, APP_CONTROL_ERROR_NONE);
  app_control_set_app_id(ac2, "com.test.button2");

  int ret = notification_set_event_handler(noti,
      NOTIFICATION_EVENT_TYPE_CLICK_ON_BUTTON_1, ac1);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  ret = notification_set_event_handler(noti,
      NOTIFICATION_EVENT_TYPE_CLICK_ON_BUTTON_2, ac2);
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

  app_control_h got1 = nullptr;
  ret = notification_get_event_handler(loaded,
      NOTIFICATION_EVENT_TYPE_CLICK_ON_BUTTON_1, &got1);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  ASSERT_NE(got1, nullptr);

  char* app_id1 = nullptr;
  app_control_get_app_id(got1, &app_id1);
  ASSERT_NE(app_id1, nullptr);
  EXPECT_STREQ(app_id1, "com.test.button1");

  app_control_h got2 = nullptr;
  ret = notification_get_event_handler(loaded,
      NOTIFICATION_EVENT_TYPE_CLICK_ON_BUTTON_2, &got2);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  ASSERT_NE(got2, nullptr);

  char* app_id2 = nullptr;
  app_control_get_app_id(got2, &app_id2);
  ASSERT_NE(app_id2, nullptr);
  EXPECT_STREQ(app_id2, "com.test.button2");

  free(app_id1);
  free(app_id2);
  app_control_destroy(got1);
  app_control_destroy(got2);
  app_control_destroy(ac1);
  app_control_destroy(ac2);
  notification_free(loaded);
  notification_free(noti);
}

/* EVENT_02: Extension event handlers (HIDDEN_BY_TIMEOUT, HIDDEN_BY_EXTERNAL)
   set, verify in-memory round-trip. Valid range is HIDDEN_BY_USER(100)
   through HIDDEN_BY_EXTERNAL(102). */
TEST_F(NotificationEventIntegrationTest,
    EVENT_02_ExtensionEventHandlerTypes) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  app_control_h ac_timeout = nullptr;
  int ac_ret = app_control_create(&ac_timeout);
  if (ac_ret != APP_CONTROL_ERROR_NONE || !ac_timeout) {
    notification_free(noti);
    GTEST_SKIP() << "app_control_create requires app context";
  }
  app_control_set_app_id(ac_timeout, "com.test.timeout_handler");

  int ret = notification_set_extension_event_handler(noti,
      NOTIFICATION_EVENT_TYPE_HIDDEN_BY_TIMEOUT, ac_timeout);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  app_control_h ac_external = nullptr;
  app_control_create(&ac_external);
  ASSERT_NE(ac_external, nullptr);
  app_control_set_app_id(ac_external, "com.test.external_handler");

  ret = notification_set_extension_event_handler(noti,
      NOTIFICATION_EVENT_TYPE_HIDDEN_BY_EXTERNAL, ac_external);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  app_control_h got_timeout = nullptr;
  ret = notification_get_extension_event_handler(noti,
      NOTIFICATION_EVENT_TYPE_HIDDEN_BY_TIMEOUT, &got_timeout);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  ASSERT_NE(got_timeout, nullptr);

  char* app_id = nullptr;
  app_control_get_app_id(got_timeout, &app_id);
  ASSERT_NE(app_id, nullptr);
  EXPECT_STREQ(app_id, "com.test.timeout_handler");
  free(app_id);

  app_control_h got_external = nullptr;
  ret = notification_get_extension_event_handler(noti,
      NOTIFICATION_EVENT_TYPE_HIDDEN_BY_EXTERNAL, &got_external);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  ASSERT_NE(got_external, nullptr);

  app_id = nullptr;
  app_control_get_app_id(got_external, &app_id);
  ASSERT_NE(app_id, nullptr);
  EXPECT_STREQ(app_id, "com.test.external_handler");
  free(app_id);

  app_control_destroy(got_timeout);
  app_control_destroy(got_external);
  app_control_destroy(ac_timeout);
  app_control_destroy(ac_external);
  notification_free(noti);
}

/* EVENT_02b: Buttons 7-10 event handlers — these use the regular
   notification_event_type_e enum. Verify DB round-trip. */
TEST_F(NotificationEventIntegrationTest,
    EVENT_02b_Buttons7Through10DBRoundTrip) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  const notification_event_type_e btn_types[] = {
      NOTIFICATION_EVENT_TYPE_CLICK_ON_BUTTON_7,
      NOTIFICATION_EVENT_TYPE_CLICK_ON_BUTTON_8,
      NOTIFICATION_EVENT_TYPE_CLICK_ON_BUTTON_9,
      NOTIFICATION_EVENT_TYPE_CLICK_ON_BUTTON_10
  };
  const char* btn_app_ids[] = {
      "com.test.btn7",
      "com.test.btn8",
      "com.test.btn9",
      "com.test.btn10"
  };
  const int num_btns = 4;

  app_control_h ac_arr[4] = {};
  for (int i = 0; i < num_btns; i++) {
    int ac_ret = app_control_create(&ac_arr[i]);
    if (ac_ret != APP_CONTROL_ERROR_NONE || !ac_arr[i]) {
      for (int j = 0; j < i; j++)
        app_control_destroy(ac_arr[j]);
      notification_free(noti);
      GTEST_SKIP() << "app_control_create requires app context";
    }
    app_control_set_app_id(ac_arr[i], btn_app_ids[i]);

    int ret = notification_set_event_handler(noti,
        btn_types[i], ac_arr[i]);
    EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  }

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

  for (int i = 0; i < num_btns; i++) {
    app_control_h got = nullptr;
    ret = notification_get_event_handler(loaded,
        btn_types[i], &got);
    EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
    ASSERT_NE(got, nullptr);

    char* app_id = nullptr;
    app_control_get_app_id(got, &app_id);
    ASSERT_NE(app_id, nullptr);
    EXPECT_STREQ(app_id, btn_app_ids[i]);

    free(app_id);
    app_control_destroy(got);
  }

  for (int i = 0; i < num_btns; i++)
    app_control_destroy(ac_arr[i]);
  notification_free(loaded);
  notification_free(noti);
}

/* EVENT_01b: Event handler for button 3 set in-memory, verify get
   returns the same handler without DB roundtrip. */
TEST_F(NotificationEventIntegrationTest,
    EVENT_01b_EventHandlerInMemoryRoundTrip) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  app_control_h ac = nullptr;
  int ac_ret = app_control_create(&ac);
  if (ac_ret != APP_CONTROL_ERROR_NONE || !ac) {
    notification_free(noti);
    GTEST_SKIP() << "app_control_create requires app context";
  }
  app_control_set_app_id(ac, "com.test.inmemory");

  int ret = notification_set_event_handler(noti,
      NOTIFICATION_EVENT_TYPE_CLICK_ON_BUTTON_3, ac);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  app_control_h got = nullptr;
  ret = notification_get_event_handler(noti,
      NOTIFICATION_EVENT_TYPE_CLICK_ON_BUTTON_3, &got);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  ASSERT_NE(got, nullptr);

  char* app_id = nullptr;
  app_control_get_app_id(got, &app_id);
  ASSERT_NE(app_id, nullptr);
  EXPECT_STREQ(app_id, "com.test.inmemory");

  free(app_id);
  app_control_destroy(got);
  app_control_destroy(ac);
  notification_free(noti);
}

/* EVENT_01c: Overwrite an existing event handler and verify the new one
   takes effect. */
TEST_F(NotificationEventIntegrationTest,
    EVENT_01c_OverwriteEventHandler) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  app_control_h ac_old = nullptr;
  int ac_ret = app_control_create(&ac_old);
  if (ac_ret != APP_CONTROL_ERROR_NONE || !ac_old) {
    notification_free(noti);
    GTEST_SKIP() << "app_control_create requires app context";
  }
  app_control_set_app_id(ac_old, "com.test.old_handler");

  int ret = notification_set_event_handler(noti,
      NOTIFICATION_EVENT_TYPE_CLICK_ON_BUTTON_1, ac_old);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  app_control_h ac_new = nullptr;
  app_control_create(&ac_new);
  ASSERT_NE(ac_new, nullptr);
  app_control_set_app_id(ac_new, "com.test.new_handler");

  ret = notification_set_event_handler(noti,
      NOTIFICATION_EVENT_TYPE_CLICK_ON_BUTTON_1, ac_new);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  app_control_h got = nullptr;
  ret = notification_get_event_handler(noti,
      NOTIFICATION_EVENT_TYPE_CLICK_ON_BUTTON_1, &got);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  ASSERT_NE(got, nullptr);

  char* app_id = nullptr;
  app_control_get_app_id(got, &app_id);
  ASSERT_NE(app_id, nullptr);
  EXPECT_STREQ(app_id, "com.test.new_handler");

  free(app_id);
  app_control_destroy(got);
  app_control_destroy(ac_old);
  app_control_destroy(ac_new);
  notification_free(noti);
}
