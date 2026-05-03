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
 * Phase 1.5-E.5 -- additional NULL-arg coverage on
 * notification_internal.c entries that aren't yet covered by
 * test_notification.cc / test_notification_internal.cc (the
 * channel-API and public-handle-handle cohorts).  This file
 * targets the deferred-task and changed-cb registration entries.
 */

#include <gtest/gtest.h>

#include "notification/include/notification_error.h"

extern "C" {
#include <notification.h>
#include <notification_internal.h>
}

namespace {

class NotificationInternalExtraTest : public ::testing::Test {};

TEST_F(NotificationInternalExtraTest,
       add_deferred_task_rejects_null_callback) {
  // notification_internal.c line 268-269: callback NULL guard.
  EXPECT_EQ(NOTIFICATION_ERROR_INVALID_PARAMETER,
      notification_add_deferred_task(nullptr, nullptr));
}

TEST_F(NotificationInternalExtraTest,
       del_deferred_task_rejects_null_callback) {
  // notification_internal.c line 277-278: callback NULL guard.
  EXPECT_EQ(NOTIFICATION_ERROR_INVALID_PARAMETER,
      notification_del_deferred_task(nullptr));
}

TEST_F(NotificationInternalExtraTest,
       resister_changed_cb_for_uid_rejects_null_callback) {
  // notification_internal.c line 291-292: callback NULL guard.
  EXPECT_EQ(NOTIFICATION_ERROR_INVALID_PARAMETER,
      notification_resister_changed_cb_for_uid(nullptr, nullptr, 0));
}

TEST_F(NotificationInternalExtraTest,
       unresister_changed_cb_for_uid_rejects_null_callback) {
  // notification_internal.c line 354-355: callback NULL guard.
  EXPECT_EQ(NOTIFICATION_ERROR_INVALID_PARAMETER,
      notification_unresister_changed_cb_for_uid(nullptr, 0));
}

}  // namespace
