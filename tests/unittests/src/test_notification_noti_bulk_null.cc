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
 * Phase 3 Half-B / Tasks 3.2 + 3.3 — input-validation guards on
 * the new notification_noti_insert_list EXPORT_API.
 *
 * These cases fire BEFORE notification_db_open() so they do not
 * need a TestFixture+SqliteMock; they are direct early-return
 * assertions, the same shape as test_notification_noti_null.cc.
 *
 * R-TDD ordering:
 *   - This file lands together with the EXPORT_API STUB in
 *     notification_noti.cc that returns
 *     NOTIFICATION_ERROR_INVALID_OPERATION unconditionally.
 *     All four tests below FAIL against that stub (they expect
 *     INVALID_PARAMETER or NONE, not INVALID_OPERATION) -- this
 *     is the deliberate red.
 *   - The next commit replaces the stub with the real
 *     implementation; the tests then go green.
 */

#include <gtest/gtest.h>

#include "notification/include/notification_error.h"

extern "C" {
#include <notification.h>
#include <notification_noti.h>
}

namespace {

class NotificationNotiBulkNullTest : public ::testing::Test {};

TEST_F(NotificationNotiBulkNullTest,
       insert_list_rejects_null_array) {
  /* notis == NULL with count > 0 must be rejected. */
  EXPECT_EQ(NOTIFICATION_ERROR_INVALID_PARAMETER,
      notification_noti_insert_list(nullptr, 5));
}

TEST_F(NotificationNotiBulkNullTest,
       insert_list_rejects_negative_count) {
  /* count < 0 is invalid regardless of array. */
  notification_h dummy_array[1] = { nullptr };
  EXPECT_EQ(NOTIFICATION_ERROR_INVALID_PARAMETER,
      notification_noti_insert_list(dummy_array, -1));
}

TEST_F(NotificationNotiBulkNullTest,
       insert_list_zero_count_is_noop_success) {
  /* count == 0 is a no-op; returns success without touching the
   * array (so a NULL array with count 0 is also a no-op). */
  EXPECT_EQ(NOTIFICATION_ERROR_NONE,
      notification_noti_insert_list(nullptr, 0));
}

TEST_F(NotificationNotiBulkNullTest,
       insert_list_rejects_null_element) {
  /* notis[k] == NULL for some k must be rejected without any
   * row being inserted.  The pre-SQL pass catches this before
   * notification_db_open() is called, so no fixture is needed. */
  notification_h notis[3] = { nullptr, nullptr, nullptr };
  EXPECT_EQ(NOTIFICATION_ERROR_INVALID_PARAMETER,
      notification_noti_insert_list(notis, 3));
}

TEST_F(NotificationNotiBulkNullTest,
       insert_list_rejects_null_in_mid_array) {
  /* Mid-array NULL must be rejected at the same early stage as
   * front-array NULL.  The naive [nullptr, nullptr, nullptr]
   * test above exits at index 0, never proving the loop walks
   * past it.  This case shows index 1 still trips the guard.
   *
   * We use a non-null dummy pointer at indices 0 and 2 so the
   * loop must iterate to index 1 before the guard fires.  The
   * dummies are only inspected by the dereferencing code that
   * runs AFTER the NULL check, which we never reach -- so a
   * sentinel pointer is safe. */
  notification_h dummy = reinterpret_cast<notification_h>(0xDEADBEEF);
  notification_h notis[3] = { dummy, nullptr, dummy };
  EXPECT_EQ(NOTIFICATION_ERROR_INVALID_PARAMETER,
      notification_noti_insert_list(notis, 3));
}

}  // namespace
