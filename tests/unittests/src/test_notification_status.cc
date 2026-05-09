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
#include "notification/include/notification_status.h"

namespace {
class NotificationStatusTest : public ::testing::Test {};
}  // namespace

TEST_F(NotificationStatusTest, message_post_null) {
  EXPECT_EQ(notification_status_message_post(nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

// notification_status_message_post("") allocates internally via
// g_strdup; the lightweight ::testing::Test fixture has no
// GlibMock so the call throws "test does not provide mock".
// Lock-in deferred to task 19's integration suite (real glib).
