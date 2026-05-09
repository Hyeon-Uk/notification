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
#include <gmock/gmock.h>

#include "mock/test_fixture.hh"
#include "mock/aul_mock.hh"
#include "mock/glib_mock.hh"

#include "notification/include/notification.h"
#include "notification/include/notification_internal.h"
#include "notification/include/notification_ongoing_flag.h"

namespace {
class Mocks : virtual public ::testing::NiceMock<AulMock>,
              virtual public ::testing::NiceMock<GlibMock> {};
constexpr int kInvalid = NOTIFICATION_ERROR_INVALID_PARAMETER;
constexpr int kNone = NOTIFICATION_ERROR_NONE;
}  // namespace

class NotificationOngoingFlagTest : public TestFixture {
 public:
  NotificationOngoingFlagTest() : TestFixture(std::make_unique<::Mocks>()) {}
};

TEST_F(NotificationOngoingFlagTest, set_get_ongoing_flag_P) {
  notification_h n = notification_new(NOTIFICATION_TYPE_NOTI, 1, 0);
  if (n == nullptr) GTEST_SKIP() << "notification_new unavailable";
  EXPECT_EQ(notification_set_ongoing_flag(n, true), kNone);
  bool flag = false;
  EXPECT_EQ(notification_get_ongoing_flag(n, &flag), kNone);
  EXPECT_TRUE(flag);
  EXPECT_EQ(notification_set_ongoing_flag(n, false), kNone);
  EXPECT_EQ(notification_get_ongoing_flag(n, &flag), kNone);
  EXPECT_FALSE(flag);
  notification_free(n);
}

TEST_F(NotificationOngoingFlagTest, set_ongoing_flag_N_null_handle) {
  EXPECT_EQ(notification_set_ongoing_flag(nullptr, true), kInvalid);
}

TEST_F(NotificationOngoingFlagTest, get_ongoing_flag_N_null_handle) {
  bool flag = false;
  EXPECT_EQ(notification_get_ongoing_flag(nullptr, &flag), kInvalid);
}
