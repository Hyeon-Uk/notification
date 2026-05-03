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
 * Phase 1.5-E.4.  notification_shared_file.c is mostly internal
 * private-sharing flows.  Of the EXPORT_API surface, only
 * notification_set_private_sharing has a clean caller-checkable
 * NULL guard at the entry point (line 1097-1098); the rest of the
 * functions return void on bad input or dereference the handle
 * directly without a guard (those would need full fixture-based
 * tests with the 1.5-D fs/security_manager mocks active).
 *
 * Broader coverage is the next 1.5-E chunk; this seed defends
 * the one cleanly testable NULL guard in the file.
 */

#include <gtest/gtest.h>

#include "notification/include/notification_error.h"

extern "C" {
#include <notification.h>
#include <notification_shared_file.h>
}

namespace {

class NotificationSharedFileTest : public ::testing::Test {};

TEST_F(NotificationSharedFileTest,
       set_private_sharing_rejects_null_noti) {
  EXPECT_EQ(NOTIFICATION_ERROR_INVALID_PARAMETER,
      notification_set_private_sharing(nullptr, 0));
}

}  // namespace
