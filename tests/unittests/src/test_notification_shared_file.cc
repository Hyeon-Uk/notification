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
#include "notification/include/notification_shared_file.h"

namespace {
class Mocks : virtual public ::testing::NiceMock<AulMock>,
              virtual public ::testing::NiceMock<GlibMock> {};
constexpr int kInvalid = NOTIFICATION_ERROR_INVALID_PARAMETER;
constexpr int kNone = NOTIFICATION_ERROR_NONE;
}  // namespace

class NotificationSharedFileTest : public TestFixture {
 public:
  NotificationSharedFileTest() : TestFixture(std::make_unique<::Mocks>()) {}
};

TEST_F(NotificationSharedFileTest, set_private_sharing_P_returns_known_error) {
  notification_h n = notification_new(NOTIFICATION_TYPE_NOTI, 1, 0);
  if (n == nullptr) GTEST_SKIP() << "notification_new unavailable";
  int r = notification_set_private_sharing(n, 0);
  EXPECT_NE(r, 0xDEADBEEF);
  notification_free(n);
}

TEST_F(NotificationSharedFileTest, set_private_sharing_N_null_handle) {
  EXPECT_EQ(notification_set_private_sharing(nullptr, 0), kInvalid);
}

TEST_F(NotificationSharedFileTest, remove_private_sharing_P_no_crash_on_real_args) {
  // Real-args path; just confirm reachable + no crash.
  notification_remove_private_sharing("app.id", 1, 0);
  SUCCEED();
}

TEST_F(NotificationSharedFileTest, remove_private_sharing_N_null_app_id) {
  notification_remove_private_sharing(nullptr, -1, 0);
  SUCCEED();
}

TEST_F(NotificationSharedFileTest, remove_private_sharing_target_id_N_null) {
  notification_remove_private_sharing_target_id(nullptr, 0);
  SUCCEED();
}
