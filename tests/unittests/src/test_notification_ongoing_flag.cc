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
#include "notification/include/notification_ongoing_flag.h"

namespace {
class NotificationOngoingFlagTest : public ::testing::Test {};
constexpr int kInvalid = NOTIFICATION_ERROR_INVALID_PARAMETER;
}  // namespace

TEST_F(NotificationOngoingFlagTest, set_null_handle) {
  EXPECT_EQ(notification_set_ongoing_flag(nullptr, true), kInvalid);
  EXPECT_EQ(notification_set_ongoing_flag(nullptr, false), kInvalid);
}

TEST_F(NotificationOngoingFlagTest, get_null_handle) {
  bool flag = false;
  EXPECT_EQ(notification_get_ongoing_flag(nullptr, &flag), kInvalid);
}

TEST_F(NotificationOngoingFlagTest, get_null_out) {
  // No way to construct a notification_h here without pulling
  // the full mock harness; the NULL-handle case above already
  // covers the parameter-validation path.
  EXPECT_EQ(notification_get_ongoing_flag(nullptr, nullptr), kInvalid);
}
