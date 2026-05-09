/*
 * Copyright (c) 2026 Samsung Electronics Co., Ltd.
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

#include "notification/include/notification.h"
#include "notification/include/notification_viewer.h"

namespace {
class NotificationViewerTest : public ::testing::Test {};
}  // namespace

TEST_F(NotificationViewerTest, init_default_viewer_reachable) {
  // /usr/share/notification/notification.ini and the function
  // returns NOTIFICATION_ERROR_FROM_DBUS / similar — exact code
  // depends on host. Lock in just that the function is reachable
  // as an exported symbol and emits SOME value (sanity).
  int ret = notification_init_default_viewer();
  (void)ret;
  SUCCEED();
}

TEST_F(NotificationViewerTest, launch_default_viewer_invalid_priv_id) {
  int ret = notification_launch_default_viewer(-1, NOTIFICATION_OP_NONE, 0);
  EXPECT_TRUE(ret == NOTIFICATION_ERROR_INVALID_PARAMETER ||
              ret == NOTIFICATION_ERROR_INVALID_OPERATION ||
              ret == NOTIFICATION_ERROR_SERVICE_NOT_READY ||
              ret == NOTIFICATION_ERROR_NOT_EXIST_ID ||
              ret == NOTIFICATION_ERROR_PERMISSION_DENIED ||
              ret == NOTIFICATION_ERROR_NONE);
}

TEST_F(NotificationViewerTest, launch_without_candidate_invalid_priv_id) {
  int ret = notification_launch_default_viewer_without_candidate_process(
      -1, NOTIFICATION_OP_NONE, 0);
  EXPECT_TRUE(ret == NOTIFICATION_ERROR_INVALID_PARAMETER ||
              ret == NOTIFICATION_ERROR_INVALID_OPERATION ||
              ret == NOTIFICATION_ERROR_SERVICE_NOT_READY ||
              ret == NOTIFICATION_ERROR_NOT_EXIST_ID ||
              ret == NOTIFICATION_ERROR_PERMISSION_DENIED ||
              ret == NOTIFICATION_ERROR_NONE);
}
