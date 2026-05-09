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
#include "notification/include/notification_text_domain.h"

// =====================================================================
// Lock-in tests for notification_text_domain.h (task 02j).
// 2 exported symbols: notification_set_text_domain /
// notification_get_text_domain. Null-arg lock-in here; full
// gettext round-trip deferred to task 19.
// =====================================================================

namespace {
class NotificationTextDomainTest : public ::testing::Test {};
constexpr int kInvalid = NOTIFICATION_ERROR_INVALID_PARAMETER;
}  // namespace

TEST_F(NotificationTextDomainTest, set_null_handle) {
  EXPECT_EQ(notification_set_text_domain(nullptr, "test", "/usr/share/locale"),
            kInvalid);
}

TEST_F(NotificationTextDomainTest, set_null_domain) {
  EXPECT_EQ(notification_set_text_domain(nullptr, nullptr, "/usr/share/locale"),
            kInvalid);
}

TEST_F(NotificationTextDomainTest, get_null_handle) {
  const char* domain = nullptr;
  const char* dir = nullptr;
  EXPECT_EQ(notification_get_text_domain(nullptr, (char**)&domain, (char**)&dir),
            kInvalid);
}

TEST_F(NotificationTextDomainTest, get_null_outs) {
  EXPECT_EQ(notification_get_text_domain(nullptr, nullptr, nullptr), kInvalid);
}
