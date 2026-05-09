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
#include <glib.h>

#include "notification/include/notification.h"
#include "notification/include/notification_error.h"

namespace {

class NotificationErrorTest : public ::testing::Test {};

// Forward-declare the symbol so we can pin its existence as a
// reachable export. The declaration here matches its emitted
// signature in the .so; if that ever changes, this test becomes
// a link error and the lock-in catches it.
extern "C" GQuark notification_error_quark(void);

}  // namespace

TEST_F(NotificationErrorTest, error_quark_returns_nonzero_GQuark) {
  GQuark q1 = notification_error_quark();
  GQuark q2 = notification_error_quark();
  EXPECT_NE(q1, 0u);
  EXPECT_EQ(q1, q2);
}

TEST_F(NotificationErrorTest, enum_values_pinned) {
  // every dependent in the Tizen tree. Locks in the contract
  // exposed via notification_error.h.
  EXPECT_EQ(NOTIFICATION_ERROR_NONE,                TIZEN_ERROR_NONE);
  EXPECT_EQ(NOTIFICATION_ERROR_INVALID_PARAMETER,   TIZEN_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(NOTIFICATION_ERROR_OUT_OF_MEMORY,       TIZEN_ERROR_OUT_OF_MEMORY);
  EXPECT_EQ(NOTIFICATION_ERROR_IO_ERROR,            TIZEN_ERROR_IO_ERROR);
  EXPECT_EQ(NOTIFICATION_ERROR_PERMISSION_DENIED,   TIZEN_ERROR_PERMISSION_DENIED);
  EXPECT_EQ(NOTIFICATION_ERROR_INVALID_OPERATION,   TIZEN_ERROR_INVALID_OPERATION);
  EXPECT_EQ(NOTIFICATION_ERROR_FROM_DB,             TIZEN_ERROR_NOTIFICATION | 0x01);
  EXPECT_EQ(NOTIFICATION_ERROR_ALREADY_EXIST_ID,    TIZEN_ERROR_NOTIFICATION | 0x02);
  EXPECT_EQ(NOTIFICATION_ERROR_FROM_DBUS,           TIZEN_ERROR_NOTIFICATION | 0x03);
  EXPECT_EQ(NOTIFICATION_ERROR_NOT_EXIST_ID,        TIZEN_ERROR_NOTIFICATION | 0x04);
  EXPECT_EQ(NOTIFICATION_ERROR_SERVICE_NOT_READY,   TIZEN_ERROR_NOTIFICATION | 0x05);
  EXPECT_EQ(NOTIFICATION_ERROR_MAX_EXCEEDED,        TIZEN_ERROR_NOTIFICATION | 0x06);
}
