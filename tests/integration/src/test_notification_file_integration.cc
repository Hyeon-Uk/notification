/*
 * Copyright (c) 2023 Samsung Electronics Co., Ltd. All rights reserved.
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
#include <cstdlib>

#include "notification/include/notification.h"
#include "notification/include/notification_internal.h"
#include "notification/include/notification_shared_file.h"

class NotificationFileIntegrationTest : public ::testing::Test {
 protected:
  void SetUp() override {}
  void TearDown() override {}
};

/* FILE_06: Validate private sharing — test with a notification that has
   no sharing set (should handle gracefully). */
TEST_F(NotificationFileIntegrationTest, FILE_06_ValidatePrivateSharingNoSharing) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  if (!noti)
    GTEST_SKIP() << "notification_create requires app context";

  bool valid = notification_validate_private_sharing(noti);
  EXPECT_TRUE(valid);

  notification_free(noti);
}
