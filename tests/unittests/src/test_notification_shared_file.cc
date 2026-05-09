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

// =====================================================================
// Lock-in tests for notification_shared_file.h (task 02h).
// 8 declared symbols; 6 exported in libnotification.so.0.12.1.
// 2 declared-but-not-exported (R-FINDINGS):
//   - notification_copy_private_file
//   - notification_check_file_path_is_private
// → F-task02h-shared-file-orphan-decls: header declares these
//   functions but the library does not export them. Locked in
//   as n/a in § API surface; future fix task either ships the
//   impl or removes the header decl.
//
// Real-fs / cross-process paths deferred to task 19; null-arg /
// no-crash lock-in for the 6 actually-exported symbols here.
// add_private_sharing_target_id and calibrate_private_sharing
// touch g_strdup-style glib helpers and require GlibMock; they
// would normally fail no-mock-provided. Marked as DEBT below.
// =====================================================================

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

// F-task02h-validate-segfault:
// notification_validate_private_sharing(nullptr) segfaults — the
// impl dereferences the handle without a NULL check. R-FINDINGS:
// behavior locked in as "crashes on NULL" via this comment.
// Future fix task should add the NULL guard. We do not call
// the function with nullptr in the test binary because it would
// SIGSEGV the whole gtest run. Documented as DEBT for task 19.

// F-task02h-add-and-calibrate-debt:
// notification_add_private_sharing_target_id and
// notification_calibrate_private_sharing both invoke g_strdup
// internally, which the existing GlibMock NiceMock does not
// stub by default — the real glib symbol gets hooked but the
// mock has no expectation, raising
// "test does not provide mock of 8GlibMock". Lock-in deferred
// to task 19's integration suite (real glib).
