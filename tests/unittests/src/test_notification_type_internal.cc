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
#include "notification/include/notification_internal.h"
#include "notification/include/notification_type_internal.h"

namespace {
class NotificationTypeInternalTest : public ::testing::Test {};
}

TEST_F(NotificationTypeInternalTest, NOTIFICATION_GLOBAL_UID_pinned) {
  EXPECT_EQ(NOTIFICATION_GLOBAL_UID, -1);
}

TEST_F(NotificationTypeInternalTest, res_path_type_e_pinned) {
  // SOUND lives at NOTIFICATION_IMAGE_TYPE_MAX + 1 (transitive).
  static_assert(NOTIFICATION_RES_PATH_TYPE_VIBRATION ==
                    NOTIFICATION_RES_PATH_TYPE_SOUND + 1,
                "VIBRATION must follow SOUND");
}

TEST_F(NotificationTypeInternalTest, ongoing_value_type_e_pinned) {
  EXPECT_EQ(NOTIFICATION_ONGOING_VALUE_TYPE_PERCENT, 0);
  static_assert(NOTIFICATION_ONGOING_VALUE_TYPE_PERCENT == 0,
                "PERCENT must be 0");
}
