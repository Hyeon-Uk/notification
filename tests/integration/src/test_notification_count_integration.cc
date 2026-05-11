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

class NotificationCountIntegrationTest : public ::testing::Test {
 protected:
  void SetUp() override {
    char tmp_dir[] = "/tmp/noti_integ_count_XXXXXX";
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
    char* pkgname = nullptr;
    notification_get_pkgname(test_noti, &pkgname);
    if (pkgname) pkgname_ = pkgname;
    notification_free(test_noti);
  }

  void TearDown() override {
    if (!db_path_.empty())
      unlink(db_path_.c_str());
    if (!tmp_dir_.empty())
      rmdir(tmp_dir_.c_str());
  }

  notification_h InsertNoti(notification_type_e type) {
    notification_h noti = notification_create(type);
    if (!noti) return nullptr;
    notification_set_display_applist(noti,
        NOTIFICATION_DISPLAY_APP_NOTIFICATION_TRAY);
    notification_set_uid(noti, 5001);
    int ret = notification_noti_insert(noti);
    if (ret != NOTIFICATION_ERROR_NONE) {
      notification_free(noti);
      return nullptr;
    }
    return noti;
  }

  std::string db_path_;
  std::string tmp_dir_;
  std::string pkgname_;
};

/* COUNT_01: InsertAndVerifyCountIncrements
 * Insert 5, count=5, insert 3 more, count=8
 */
TEST_F(NotificationCountIntegrationTest, InsertAndVerifyCountIncrements) {
  for (int i = 0; i < 5; i++) {
    notification_h noti = InsertNoti(NOTIFICATION_TYPE_NOTI);
    ASSERT_NE(noti, nullptr);
    notification_free(noti);
  }

  int count = 0;
  int ret = notification_noti_get_all_count(NOTIFICATION_TYPE_NOTI,
      &count, 5001);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(count, 5);

  for (int i = 0; i < 3; i++) {
    notification_h noti = InsertNoti(NOTIFICATION_TYPE_NOTI);
    ASSERT_NE(noti, nullptr);
    notification_free(noti);
  }

  count = 0;
  ret = notification_noti_get_all_count(NOTIFICATION_TYPE_NOTI,
      &count, 5001);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(count, 8);
}

/* COUNT_02: CountByTypeWithMixed
 * Insert 3 NOTI + 2 ONGOING, count NOTI=3, count ONGOING=2
 */
TEST_F(NotificationCountIntegrationTest, CountByTypeWithMixed) {
  for (int i = 0; i < 3; i++) {
    notification_h noti = InsertNoti(NOTIFICATION_TYPE_NOTI);
    ASSERT_NE(noti, nullptr);
    notification_free(noti);
  }

  for (int i = 0; i < 2; i++) {
    notification_h noti = InsertNoti(NOTIFICATION_TYPE_ONGOING);
    ASSERT_NE(noti, nullptr);
    notification_free(noti);
  }

  int noti_count = 0;
  int ret = notification_noti_get_all_count(NOTIFICATION_TYPE_NOTI,
      &noti_count, 5001);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(noti_count, 3);

  int ongoing_count = 0;
  ret = notification_noti_get_all_count(NOTIFICATION_TYPE_ONGOING,
      &ongoing_count, 5001);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(ongoing_count, 2);
}

/* COUNT_03: DeleteSomeAndRecount
 * Insert 10, record priv_ids, delete [0],[3],[7], count=7
 */
TEST_F(NotificationCountIntegrationTest, DeleteSomeAndRecount) {
  int priv_ids[10] = {0};

  for (int i = 0; i < 10; i++) {
    notification_h noti = InsertNoti(NOTIFICATION_TYPE_NOTI);
    ASSERT_NE(noti, nullptr);
    int group_id = 0;
    notification_get_id(noti, &group_id, &priv_ids[i]);
    notification_free(noti);
  }

  int ret = notification_noti_delete_by_priv_id(pkgname_.c_str(), priv_ids[0]);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  ret = notification_noti_delete_by_priv_id(pkgname_.c_str(), priv_ids[3]);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  ret = notification_noti_delete_by_priv_id(pkgname_.c_str(), priv_ids[7]);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  int count = 0;
  ret = notification_noti_get_all_count(NOTIFICATION_TYPE_NOTI,
      &count, 5001);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(count, 7);
}

/* COUNT_04: DeleteAllAndVerifyZero
 * Insert 5, delete_all, count=0
 */
TEST_F(NotificationCountIntegrationTest, DeleteAllAndVerifyZero) {
  for (int i = 0; i < 5; i++) {
    notification_h noti = InsertNoti(NOTIFICATION_TYPE_NOTI);
    ASSERT_NE(noti, nullptr);
    notification_free(noti);
  }

  int deleted_num = 0;
  int* deleted_list = nullptr;
  int ret = notification_noti_delete_all(NOTIFICATION_TYPE_NOTI,
      nullptr, &deleted_num, &deleted_list, 5001);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  if (deleted_list) free(deleted_list);

  int count = 0;
  ret = notification_noti_get_all_count(NOTIFICATION_TYPE_NOTI,
      &count, 5001);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(count, 0);
}

/* COUNT_05: CountByAppIdAndType
 * Insert 4, use notification_noti_get_count with nullptr app_id,
 * group_id=NOTIFICATION_GROUP_ID_NONE, priv_id=NOTIFICATION_PRIV_ID_NONE,
 * expect count=4
 */
TEST_F(NotificationCountIntegrationTest, CountByAppIdAndType) {
  for (int i = 0; i < 4; i++) {
    notification_h noti = InsertNoti(NOTIFICATION_TYPE_NOTI);
    ASSERT_NE(noti, nullptr);
    notification_free(noti);
  }

  int count = 0;
  int ret = notification_noti_get_count(NOTIFICATION_TYPE_NOTI,
      pkgname_.c_str(), NOTIFICATION_GROUP_ID_NONE, NOTIFICATION_PRIV_ID_NONE,
      &count, 5001);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(count, 4);
}
