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

extern "C" void test_set_db_dir(const char* dir);
extern "C" void tidl_stub_reset(void);
extern "C" bool tidl_stub_plt_active(void);
extern "C" int  tidl_stub_insert_called;
extern "C" int  tidl_stub_update_called;
extern "C" int  tidl_stub_delete_called;
extern "C" int  tidl_stub_delete_all_called;
extern "C" int  tidl_stub_save_template_called;
extern "C" int  tidl_stub_create_template_called;
extern "C" int  tidl_stub_next_insert_ret;
extern "C" int  tidl_stub_next_update_ret;
extern "C" int  tidl_stub_next_delete_ret;
extern "C" int  tidl_stub_next_delete_all_ret;
extern "C" int  tidl_stub_next_save_template_ret;
extern "C" int  tidl_stub_next_create_template_ret;

#define REQUIRE_PLT() \
  do { \
    if (!plt_active_) \
      GTEST_SKIP() << "PLT interposition not active " \
                       "(TIDL symbols have hidden visibility)"; \
  } while (0)

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

class NotificationPostIntegrationTest : public ::testing::Test {
 protected:
  void SetUp() override {
    tidl_stub_reset();
    plt_active_ = tidl_stub_plt_active();

    char tmp_dir[] = "/tmp/noti_integ_post_XXXXXX";
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

  bool plt_active_ = false;
  std::string tmp_dir_;
  std::string db_path_;
};

// --- TIDL-path tests (require PLT interposition) ---

TEST_F(NotificationPostIntegrationTest, PostValidNotiCallsInsertStub) {
  REQUIRE_PLT();

  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);
  notification_set_display_applist(noti,
      NOTIFICATION_DISPLAY_APP_NOTIFICATION_TRAY);

  int ret = notification_post(noti);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(tidl_stub_insert_called, 1);

  int gid = 0, pid = 0;
  notification_get_id(noti, &gid, &pid);
  EXPECT_EQ(pid, 9001);

  notification_free(noti);
}

TEST_F(NotificationPostIntegrationTest, PostWhenStubReturnsIoError) {
  REQUIRE_PLT();
  tidl_stub_next_insert_ret = NOTIFICATION_ERROR_IO_ERROR;

  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);
  notification_set_display_applist(noti,
      NOTIFICATION_DISPLAY_APP_NOTIFICATION_TRAY);

  int ret = notification_post(noti);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_IO_ERROR);
  EXPECT_EQ(tidl_stub_insert_called, 1);

  notification_free(noti);
}

TEST_F(NotificationPostIntegrationTest, UpdateValidNotiCallsUpdateStub) {
  REQUIRE_PLT();

  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);
  notification_set_display_applist(noti,
      NOTIFICATION_DISPLAY_APP_NOTIFICATION_TRAY);

  int ret = notification_update(noti);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(tidl_stub_update_called, 1);

  notification_free(noti);
}

TEST_F(NotificationPostIntegrationTest, UpdateWhenStubReturnsIoError) {
  REQUIRE_PLT();
  tidl_stub_next_update_ret = NOTIFICATION_ERROR_IO_ERROR;

  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);
  notification_set_display_applist(noti,
      NOTIFICATION_DISPLAY_APP_NOTIFICATION_TRAY);

  int ret = notification_update(noti);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_IO_ERROR);
  EXPECT_EQ(tidl_stub_update_called, 1);

  notification_free(noti);
}

TEST_F(NotificationPostIntegrationTest, DeleteValidNotiCallsDeleteStub) {
  REQUIRE_PLT();

  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);
  notification_set_display_applist(noti,
      NOTIFICATION_DISPLAY_APP_NOTIFICATION_TRAY);

  int ret = notification_post(noti);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);
  tidl_stub_reset();

  ret = notification_delete(noti);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(tidl_stub_delete_called, 1);

  notification_free(noti);
}

TEST_F(NotificationPostIntegrationTest, DeleteWhenStubReturnsIoError) {
  REQUIRE_PLT();

  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);
  notification_set_display_applist(noti,
      NOTIFICATION_DISPLAY_APP_NOTIFICATION_TRAY);

  int ret = notification_post(noti);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  tidl_stub_reset();
  tidl_stub_next_delete_ret = NOTIFICATION_ERROR_IO_ERROR;

  ret = notification_delete(noti);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_IO_ERROR);
  EXPECT_EQ(tidl_stub_delete_called, 1);

  notification_free(noti);
}

TEST_F(NotificationPostIntegrationTest, DeleteAllValidTypeCallsStub) {
  REQUIRE_PLT();

  int ret = notification_delete_all(NOTIFICATION_TYPE_NOTI);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(tidl_stub_delete_all_called, 1);
}

TEST_F(NotificationPostIntegrationTest, DeleteAllWhenStubReturnsIoError) {
  REQUIRE_PLT();
  tidl_stub_next_delete_all_ret = NOTIFICATION_ERROR_IO_ERROR;

  int ret = notification_delete_all(NOTIFICATION_TYPE_NOTI);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_IO_ERROR);
  EXPECT_EQ(tidl_stub_delete_all_called, 1);
}

TEST_F(NotificationPostIntegrationTest, SaveTemplateValidArgsCallsStub) {
  REQUIRE_PLT();

  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  int ret = notification_save_as_template(noti, "my_template");
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(tidl_stub_save_template_called, 1);

  notification_free(noti);
}

TEST_F(NotificationPostIntegrationTest, CreateFromTemplateValidNameCallsStub) {
  REQUIRE_PLT();

  notification_h noti = notification_create_from_template("my_template");
  EXPECT_EQ(tidl_stub_create_template_called, 1);
  if (noti)
    notification_free(noti);
}

// --- Parameter validation tests (no PLT needed, always pass) ---

TEST_F(NotificationPostIntegrationTest, PostNullNotiReturnsInvalidParam) {
  int ret = notification_post(nullptr);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationPostIntegrationTest, UpdateNullNotiReturnsInvalidParam) {
  int ret = notification_update(nullptr);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationPostIntegrationTest, DeleteNullNotiReturnsInvalidParam) {
  int ret = notification_delete(nullptr);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationPostIntegrationTest, DeleteAllInvalidTypeReturnsError) {
  int ret = notification_delete_all((notification_type_e)-1);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationPostIntegrationTest, SaveTemplateNullNotiReturnsError) {
  int ret = notification_save_as_template(nullptr, "test_template");
  EXPECT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationPostIntegrationTest, SaveTemplateNullNameReturnsError) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  int ret = notification_save_as_template(noti, nullptr);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);

  notification_free(noti);
}

TEST_F(NotificationPostIntegrationTest, CreateFromTemplateNullNameReturnsNull) {
  notification_h noti = notification_create_from_template(nullptr);
  EXPECT_EQ(noti, nullptr);
}
