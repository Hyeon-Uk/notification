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

extern "C" void test_set_db_dir(const char* dir);

class NotificationListIntegrationTest : public ::testing::Test {
 protected:
  void SetUp() override {
    notification_h test_noti = notification_create(NOTIFICATION_TYPE_NOTI);
    if (!test_noti)
      GTEST_SKIP() << "notification_create requires app context";
    notification_free(test_noti);
  }

  void TearDown() override {
  }
};

TEST_F(NotificationListIntegrationTest, AppendSingleNotification) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  notification_list_h list = nullptr;
  list = notification_list_append(list, noti);
  ASSERT_NE(list, nullptr);

  // Verify count
  int count = notification_list_get_count(list);
  EXPECT_EQ(count, 1);

  // Verify data
  notification_h data = notification_list_get_data(list);
  EXPECT_EQ(data, noti);

  notification_free_list(list);
}

TEST_F(NotificationListIntegrationTest, AppendMultipleNotifications) {
  const int NUM_NOTIS = 5;
  notification_h notis[NUM_NOTIS];
  notification_list_h list = nullptr;

  for (int i = 0; i < NUM_NOTIS; i++) {
    notis[i] = notification_create(NOTIFICATION_TYPE_NOTI);
    ASSERT_NE(notis[i], nullptr);
    list = notification_list_append(list, notis[i]);
    ASSERT_NE(list, nullptr);
  }

  int count = notification_list_get_count(list);
  EXPECT_EQ(count, NUM_NOTIS);

  notification_free_list(list);
}

TEST_F(NotificationListIntegrationTest, GetHeadAndTail) {
  notification_h noti1 = notification_create(NOTIFICATION_TYPE_NOTI);
  notification_h noti2 = notification_create(NOTIFICATION_TYPE_NOTI);
  notification_h noti3 = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti1, nullptr);
  ASSERT_NE(noti2, nullptr);
  ASSERT_NE(noti3, nullptr);

  notification_list_h list = nullptr;
  list = notification_list_append(list, noti1);
  list = notification_list_append(list, noti2);
  list = notification_list_append(list, noti3);
  ASSERT_NE(list, nullptr);

  // Get head
  notification_list_h head = notification_list_get_head(list);
  ASSERT_NE(head, nullptr);
  notification_h head_data = notification_list_get_data(head);
  EXPECT_EQ(head_data, noti1);

  // Get tail
  notification_list_h tail = notification_list_get_tail(list);
  ASSERT_NE(tail, nullptr);
  notification_h tail_data = notification_list_get_data(tail);
  EXPECT_EQ(tail_data, noti3);

  notification_free_list(list);
}

TEST_F(NotificationListIntegrationTest, NavigatePrevNext) {
  notification_h noti1 = notification_create(NOTIFICATION_TYPE_NOTI);
  notification_h noti2 = notification_create(NOTIFICATION_TYPE_NOTI);
  notification_h noti3 = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti1, nullptr);
  ASSERT_NE(noti2, nullptr);
  ASSERT_NE(noti3, nullptr);

  notification_list_h list = nullptr;
  list = notification_list_append(list, noti1);
  list = notification_list_append(list, noti2);
  list = notification_list_append(list, noti3);
  ASSERT_NE(list, nullptr);

  // Start from head, navigate forward
  notification_list_h current = notification_list_get_head(list);
  ASSERT_NE(current, nullptr);
  EXPECT_EQ(notification_list_get_data(current), noti1);

  current = notification_list_get_next(current);
  ASSERT_NE(current, nullptr);
  EXPECT_EQ(notification_list_get_data(current), noti2);

  current = notification_list_get_next(current);
  ASSERT_NE(current, nullptr);
  EXPECT_EQ(notification_list_get_data(current), noti3);

  // No more next
  notification_list_h end = notification_list_get_next(current);
  EXPECT_EQ(end, nullptr);

  // Navigate backward from tail
  current = notification_list_get_tail(list);
  ASSERT_NE(current, nullptr);
  EXPECT_EQ(notification_list_get_data(current), noti3);

  current = notification_list_get_prev(current);
  ASSERT_NE(current, nullptr);
  EXPECT_EQ(notification_list_get_data(current), noti2);

  current = notification_list_get_prev(current);
  ASSERT_NE(current, nullptr);
  EXPECT_EQ(notification_list_get_data(current), noti1);

  // No more prev
  notification_list_h begin = notification_list_get_prev(current);
  EXPECT_EQ(begin, nullptr);

  notification_free_list(list);
}

TEST_F(NotificationListIntegrationTest, RemoveFromList) {
  notification_h noti1 = notification_create(NOTIFICATION_TYPE_NOTI);
  notification_h noti2 = notification_create(NOTIFICATION_TYPE_NOTI);
  notification_h noti3 = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti1, nullptr);
  ASSERT_NE(noti2, nullptr);
  ASSERT_NE(noti3, nullptr);

  notification_list_h list = nullptr;
  list = notification_list_append(list, noti1);
  list = notification_list_append(list, noti2);
  list = notification_list_append(list, noti3);
  ASSERT_NE(list, nullptr);
  EXPECT_EQ(notification_list_get_count(list), 3);

  // Remove the middle element
  list = notification_list_remove(list, noti2);
  EXPECT_EQ(notification_list_get_count(list), 2);

  // Verify the remaining elements
  notification_list_h head = notification_list_get_head(list);
  ASSERT_NE(head, nullptr);
  EXPECT_EQ(notification_list_get_data(head), noti1);

  notification_list_h tail = notification_list_get_tail(list);
  ASSERT_NE(tail, nullptr);
  EXPECT_EQ(notification_list_get_data(tail), noti3);

  // The removed notification should be freed separately
  notification_free(noti2);
  notification_free_list(list);
}

TEST_F(NotificationListIntegrationTest, FreeListProperly) {
  notification_h noti1 = notification_create(NOTIFICATION_TYPE_NOTI);
  notification_h noti2 = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti1, nullptr);
  ASSERT_NE(noti2, nullptr);

  notification_list_h list = nullptr;
  list = notification_list_append(list, noti1);
  list = notification_list_append(list, noti2);
  ASSERT_NE(list, nullptr);

  // Free the list - this should not crash
  int ret = notification_free_list(list);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
}

TEST_F(NotificationListIntegrationTest, EmptyListOperations) {
  // Get head of null list
  notification_list_h head = notification_list_get_head(nullptr);
  EXPECT_EQ(head, nullptr);

  // Get tail of null list
  notification_list_h tail = notification_list_get_tail(nullptr);
  EXPECT_EQ(tail, nullptr);

  // Get next of null list
  notification_list_h next = notification_list_get_next(nullptr);
  EXPECT_EQ(next, nullptr);

  // Get prev of null list
  notification_list_h prev = notification_list_get_prev(nullptr);
  EXPECT_EQ(prev, nullptr);

  // Get data of null list
  notification_h data = notification_list_get_data(nullptr);
  EXPECT_EQ(data, nullptr);

  // Get count of null list
  int count = notification_list_get_count(nullptr);
  EXPECT_EQ(count, 0);
}

TEST_F(NotificationListIntegrationTest, RemoveHeadFromList) {
  notification_h noti1 = notification_create(NOTIFICATION_TYPE_NOTI);
  notification_h noti2 = notification_create(NOTIFICATION_TYPE_NOTI);
  notification_h noti3 = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti1, nullptr);
  ASSERT_NE(noti2, nullptr);
  ASSERT_NE(noti3, nullptr);

  notification_list_h list = nullptr;
  list = notification_list_append(list, noti1);
  list = notification_list_append(list, noti2);
  list = notification_list_append(list, noti3);
  ASSERT_NE(list, nullptr);

  // Remove the head element
  list = notification_list_remove(list, noti1);
  EXPECT_EQ(notification_list_get_count(list), 2);

  // New head should be noti2
  notification_list_h head = notification_list_get_head(list);
  ASSERT_NE(head, nullptr);
  EXPECT_EQ(notification_list_get_data(head), noti2);

  notification_free(noti1);
  notification_free_list(list);
}

TEST_F(NotificationListIntegrationTest, RemoveTailFromList) {
  notification_h noti1 = notification_create(NOTIFICATION_TYPE_NOTI);
  notification_h noti2 = notification_create(NOTIFICATION_TYPE_NOTI);
  notification_h noti3 = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti1, nullptr);
  ASSERT_NE(noti2, nullptr);
  ASSERT_NE(noti3, nullptr);

  notification_list_h list = nullptr;
  list = notification_list_append(list, noti1);
  list = notification_list_append(list, noti2);
  list = notification_list_append(list, noti3);
  ASSERT_NE(list, nullptr);

  // Remove the tail element
  list = notification_list_remove(list, noti3);
  EXPECT_EQ(notification_list_get_count(list), 2);

  // New tail should be noti2
  notification_list_h tail = notification_list_get_tail(list);
  ASSERT_NE(tail, nullptr);
  EXPECT_EQ(notification_list_get_data(tail), noti2);

  notification_free(noti3);
  notification_free_list(list);
}

namespace {

const char* CREATE_NOTI_TABLE_SQL_LIST =
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

class NotificationListGroupingIntegrationTest : public ::testing::Test {
 protected:
  void SetUp() override {
    char tmp_dir[] = "/tmp/noti_integ_listgrp_XXXXXX";
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
    rc = sqlite3_exec(setup_db, CREATE_NOTI_TABLE_SQL_LIST,
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

/* GroupingListOrderingNewestFirst
 * Insert 3 notifications with distinct timestamps (oldest first),
 * retrieve via notification_noti_get_grouping_list, verify the
 * returned list is ordered newest first (descending by rowid/time).
 */
TEST_F(NotificationListGroupingIntegrationTest,
    GroupingListOrderingNewestFirst) {
  int priv_ids[3] = {0};
  time_t base_time = 1000;

  for (int i = 0; i < 3; i++) {
    notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
    ASSERT_NE(noti, nullptr);
    notification_set_time(noti, base_time + i * 100);
    notification_set_display_applist(noti,
        NOTIFICATION_DISPLAY_APP_NOTIFICATION_TRAY);
    notification_set_uid(noti, 5001);

    int ret = notification_noti_insert(noti);
    ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

    int g = 0;
    notification_get_id(noti, &g, &priv_ids[i]);
    ASSERT_GT(priv_ids[i], 0);
    notification_free(noti);
  }

  notification_list_h list = nullptr;
  int list_count = 0;
  int ret = notification_noti_get_grouping_list(
      NOTIFICATION_TYPE_NOTI, 1, 10, &list, &list_count, 5001);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(list_count, 3);

  if (list && list_count == 3) {
    /* ORDER BY rowid DESC, time DESC => newest first */
    notification_list_h current = notification_list_get_head(list);
    ASSERT_NE(current, nullptr);

    notification_h first_data = notification_list_get_data(current);
    ASSERT_NE(first_data, nullptr);
    int first_priv = 0, g = 0;
    notification_get_id(first_data, &g, &first_priv);
    EXPECT_EQ(first_priv, priv_ids[2])
        << "Newest notification should be first in grouping list";

    current = notification_list_get_next(current);
    ASSERT_NE(current, nullptr);
    notification_h second_data = notification_list_get_data(current);
    ASSERT_NE(second_data, nullptr);
    int second_priv = 0;
    notification_get_id(second_data, &g, &second_priv);
    EXPECT_EQ(second_priv, priv_ids[1]);

    current = notification_list_get_next(current);
    ASSERT_NE(current, nullptr);
    notification_h third_data = notification_list_get_data(current);
    ASSERT_NE(third_data, nullptr);
    int third_priv = 0;
    notification_get_id(third_data, &g, &third_priv);
    EXPECT_EQ(third_priv, priv_ids[0])
        << "Oldest notification should be last in grouping list";
  }

  if (list) notification_free_list(list);
}
