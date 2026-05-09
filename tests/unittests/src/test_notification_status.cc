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
#include "notification/include/notification_status.h"

namespace {

class Mocks : virtual public ::testing::NiceMock<GlibMock> {};

constexpr int kInvalid = NOTIFICATION_ERROR_INVALID_PARAMETER;

}  // namespace

class NotificationStatusTest : public TestFixture {
 public:
  NotificationStatusTest() : TestFixture(std::make_unique<::Mocks>()) {}
};

// notification_status_message_post with a valid string SIGSEGVs in
// the unit fixture (DPM connection path). Real-DPM round-trip is
// the only viable _P; deferred to the integration suite. Unit-test
// lock-in is _N only.

TEST_F(NotificationStatusTest, message_post_N_null_message) {
  EXPECT_EQ(notification_status_message_post(nullptr), kInvalid);
}
