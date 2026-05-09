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
#include "mock/glib_mock.hh"

#include "notification/include/notification.h"
#include "notification/include/notification_shared_file.h"

namespace {
class Mocks : virtual public ::testing::NiceMock<GlibMock> {};
}  // namespace

class NotificationSharedFileTest : public TestFixture {
 public:
  NotificationSharedFileTest() : TestFixture(std::make_unique<::Mocks>()) {}
};

namespace { constexpr int kInvalid = NOTIFICATION_ERROR_INVALID_PARAMETER; }

TEST_F(NotificationSharedFileTest, set_private_sharing_null_handle) {
  EXPECT_EQ(notification_set_private_sharing(nullptr, 0), kInvalid);
}

TEST_F(NotificationSharedFileTest, remove_private_sharing_null) {
  notification_remove_private_sharing(nullptr, -1, 0);
  SUCCEED();
}

TEST_F(NotificationSharedFileTest, remove_private_sharing_target_id_null) {
  notification_remove_private_sharing_target_id(nullptr, 0);
  SUCCEED();
}

// notification_validate_private_sharing(nullptr) segfaults — the
// behavior locked in as "crashes on NULL" via this comment.
// Future fix task should add the NULL guard. We do not call
// the function with nullptr in the test binary because it would
// SIGSEGV the whole gtest run. Documented as DEBT for task 19.

// notification_add_private_sharing_target_id and
// notification_calibrate_private_sharing both invoke g_strdup
// internally, which the existing GlibMock NiceMock does not
// stub by default — the real glib symbol gets hooked but the
// mock has no expectation, raising
// "test does not provide mock of 8GlibMock". Lock-in deferred
// to task 19's integration suite (real glib).
