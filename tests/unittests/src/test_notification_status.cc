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

/*
 * Phase 1.5-E.1.  notification_status.c is LCOV-EXCL'd in source
 * (legacy D-Bus status-message surface; line coverage gate is met by
 * exclusion).  These tests still defend against regression on the
 * input-validation branches that fire BEFORE any D-Bus call --
 * making them safe to assert without a TestFixture / GlibMock
 * registration.
 */

#include <gtest/gtest.h>

#include "notification/include/notification_error.h"

extern "C" {
#include <notification_status.h>
#include <notification_status_internal.h>
}

namespace {

class NotificationStatusTest : public ::testing::Test {};

TEST_F(NotificationStatusTest, cb_set_with_null_callback_returns_invalid_parameter) {
  // notification_status_monitor_message_cb_set() rejects a NULL
  // callback before touching the D-Bus connection (line 79-80).
  EXPECT_EQ(NOTIFICATION_ERROR_INVALID_PARAMETER,
      notification_status_monitor_message_cb_set(nullptr, nullptr));
}

TEST_F(NotificationStatusTest, cb_unset_on_empty_state_returns_none) {
  // notification_status_monitor_message_cb_unset() is safe to call
  // on a fresh process where the module-static `md` is
  // zero-initialized -- both `if (md.message_id)` and `if (md.conn)`
  // branches skip, so no D-Bus call happens.  Returns NONE.
  EXPECT_EQ(NOTIFICATION_ERROR_NONE,
      notification_status_monitor_message_cb_unset());
}

TEST_F(NotificationStatusTest, message_post_with_null_message_returns_invalid_parameter) {
  // notification_status_message_post() rejects a NULL message before
  // touching D-Bus (line 147-150).
  EXPECT_EQ(NOTIFICATION_ERROR_INVALID_PARAMETER,
      notification_status_message_post(nullptr));
}

}  // namespace
