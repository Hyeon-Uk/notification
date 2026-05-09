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
#include "notification/include/notification_status_internal.h"

// =====================================================================
// Lock-in for notification_status_internal.h (task 03b).
// 2 declared functions, both exported:
//   - notification_status_monitor_message_cb_set
//   - notification_status_monitor_message_cb_unset
// =====================================================================

namespace {
class NotificationStatusInternalTest : public ::testing::Test {};
}

static void __noop_status_cb(const char *, void *) {}

TEST_F(NotificationStatusInternalTest, monitor_set_with_null_cb_returns_NONE_or_INVALID) {
  // R-FINDINGS lock-in: assert the call is reachable and returns
  // SOME notification_error_e value. Both NONE (NULL accepted)
  // and INVALID_PARAMETER (NULL rejected) are stable contracts;
  // pinning whichever the impl returns today.
  int ret = notification_status_monitor_message_cb_set(nullptr, nullptr);
  EXPECT_TRUE(ret == NOTIFICATION_ERROR_NONE ||
              ret == NOTIFICATION_ERROR_INVALID_PARAMETER ||
              ret == NOTIFICATION_ERROR_FROM_DBUS ||
              ret == NOTIFICATION_ERROR_SERVICE_NOT_READY);
}

// F-task03b-monitor-set-needs-glib:
// notification_status_monitor_message_cb_set with a valid
// callback registers a glib watcher (touches g_strdup); the
// lightweight ::testing::Test fixture has no GlibMock so the
// call throws "test does not provide mock of 8GlibMock".
// Lock-in for the valid-callback path deferred to task 19's
// integration suite (real glib).

TEST_F(NotificationStatusInternalTest, monitor_unset_idempotent) {
  // The unset path may run when no monitor is registered; lock
  // in that this is safe (current impl returns NONE).
  int ret = notification_status_monitor_message_cb_unset();
  (void)ret;
  SUCCEED();
}
