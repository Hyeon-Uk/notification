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
#include <cstdlib>
#include <cstring>
#include <string>

#include "notification/include/notification.h"
#include "notification/include/notification_list.h"

class NotificationListIntegrationTest : public ::testing::Test {
 protected:
  void SetUp() override {
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
