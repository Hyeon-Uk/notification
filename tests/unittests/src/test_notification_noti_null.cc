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
 * Phase 1.5-E.6.  Companion to test_notification_noti.cc which
 * uses the TestFixture+SqliteMock+AulMock plumbing for happy-path
 * coverage.  This file targets the NULL-arg early-returns that
 * fire BEFORE notification_db_open() is invoked, so no fixture is
 * needed.
 */

#include <gtest/gtest.h>

#include "notification/include/notification_error.h"

extern "C" {
#include <notification.h>
#include <notification_noti.h>
}

namespace {

class NotificationNotiNullTest : public ::testing::Test {};

TEST_F(NotificationNotiNullTest,
       insert_rejects_null_handle) {
  // notification_noti.c line 1059-1062: noti NULL guard.
  EXPECT_EQ(NOTIFICATION_ERROR_INVALID_PARAMETER,
      notification_noti_insert(nullptr));
}

TEST_F(NotificationNotiNullTest,
       get_by_priv_id_rejects_null_handle) {
  // notification_noti.c line 1142-1145: priv_id < 0 || noti == NULL.
  EXPECT_EQ(NOTIFICATION_ERROR_INVALID_PARAMETER,
      notification_noti_get_by_priv_id(nullptr, 1));
}

TEST_F(NotificationNotiNullTest,
       get_by_priv_id_rejects_negative_priv_id) {
  // Same guard, second branch.  Use a non-null dummy notification
  // pointer to bypass the noti==NULL check; the priv_id<0 branch
  // is asserted independently.
  notification_h dummy_noti = reinterpret_cast<notification_h>(0xDEADBEEF);
  EXPECT_EQ(NOTIFICATION_ERROR_INVALID_PARAMETER,
      notification_noti_get_by_priv_id(dummy_noti, -1));
}

TEST_F(NotificationNotiNullTest,
       update_rejects_null_handle) {
  // notification_noti.c line 1199-1202: noti NULL guard.
  EXPECT_EQ(NOTIFICATION_ERROR_INVALID_PARAMETER,
      notification_noti_update(nullptr));
}

// NOTE: notification_noti_check_tag(NULL) is NOT tested here -- the
// function dereferences noti->tag without a NULL guard at line
// 1962.  Park as a Phase-2 finding (R-FINDINGS): missing NULL
// guard on a public EXPORT_API entry would crash a caller that
// passes NULL.

}  // namespace
