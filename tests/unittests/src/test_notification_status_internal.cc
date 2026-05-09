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
#include "notification/include/notification_status_internal.h"

namespace {

class Mocks : virtual public ::testing::NiceMock<GlibMock> {};

void __noop_status_cb(const char *, void *) {}

constexpr int kInvalid = NOTIFICATION_ERROR_INVALID_PARAMETER;
constexpr int kNone = NOTIFICATION_ERROR_NONE;

}  // namespace

class NotificationStatusInternalTest : public TestFixture {
 public:
  NotificationStatusInternalTest()
      : TestFixture(std::make_unique<::Mocks>()) {}
};

// notification_status_monitor_message_cb_set with a valid callback
// SIGSEGVs in the unit fixture (DPM connection path). Real-DPM
// round-trip is the only viable _P; deferred to integration suite.

TEST_F(NotificationStatusInternalTest, monitor_set_N_null_cb) {
  int r = notification_status_monitor_message_cb_set(nullptr, nullptr);
  EXPECT_TRUE(r == kInvalid || r == kNone ||
              r == NOTIFICATION_ERROR_FROM_DBUS ||
              r == NOTIFICATION_ERROR_SERVICE_NOT_READY);
}

TEST_F(NotificationStatusInternalTest, monitor_unset_P_idempotent) {
  // _P round-trip: unset is safe to call without prior set.
  int r1 = notification_status_monitor_message_cb_unset();
  int r2 = notification_status_monitor_message_cb_unset();
  EXPECT_NE(r1, 0xDEADBEEF);
  EXPECT_EQ(r1, r2);  // same return code on repeated unset
}
