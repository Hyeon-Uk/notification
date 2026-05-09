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
#include "notification/include/notification_type.h"

// =====================================================================
// Lock-in for notification_type.h (task 02k).
// Header is typedef/enum-only — no exported function symbols.
// Pin a sample of the public-enum numeric values so any future
// schema-style change is caught (R-ABI-STABLE).
// =====================================================================

namespace {
class NotificationTypeTest : public ::testing::Test {};
}  // namespace

TEST_F(NotificationTypeTest, notification_type_e_values_pinned) {
  // R-ABI-STABLE: per src/notification/include/notification_type.h
  // NONE = -1, NOTI = 0, ONGOING follows (= 1).
  EXPECT_EQ(NOTIFICATION_TYPE_NONE,    -1);
  EXPECT_EQ(NOTIFICATION_TYPE_NOTI,     0);
  EXPECT_EQ(NOTIFICATION_TYPE_ONGOING,  1);
}

TEST_F(NotificationTypeTest, notification_image_type_e_pinned) {
  EXPECT_EQ(NOTIFICATION_IMAGE_TYPE_NONE, -1);
  EXPECT_EQ(NOTIFICATION_IMAGE_TYPE_ICON,  0);
  static_assert(NOTIFICATION_IMAGE_TYPE_ICON > NOTIFICATION_IMAGE_TYPE_NONE,
                "ICON must come after NONE");
}

TEST_F(NotificationTypeTest, notification_sound_type_e_pinned) {
  EXPECT_EQ(NOTIFICATION_SOUND_TYPE_NONE,      -1);
  EXPECT_EQ(NOTIFICATION_SOUND_TYPE_DEFAULT,    0);
  EXPECT_EQ(NOTIFICATION_SOUND_TYPE_USER_DATA,  1);
}

TEST_F(NotificationTypeTest, notification_vibration_type_e_pinned) {
  EXPECT_EQ(NOTIFICATION_VIBRATION_TYPE_NONE,      -1);
  EXPECT_EQ(NOTIFICATION_VIBRATION_TYPE_DEFAULT,    0);
  EXPECT_EQ(NOTIFICATION_VIBRATION_TYPE_USER_DATA,  1);
}

TEST_F(NotificationTypeTest, notification_op_type_e_pinned) {
  EXPECT_EQ(NOTIFICATION_OP_NONE,        0);
  EXPECT_EQ(NOTIFICATION_OP_INSERT,      1);
  EXPECT_EQ(NOTIFICATION_OP_UPDATE,      2);
  EXPECT_EQ(NOTIFICATION_OP_DELETE,      3);
  EXPECT_EQ(NOTIFICATION_OP_DELETE_ALL,  4);
}
