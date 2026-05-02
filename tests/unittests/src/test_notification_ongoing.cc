/*
 * Copyright (c) 2023 Samsung Electronics Co., Ltd.
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
#include "notification/include/notification_ongoing.h"

namespace {

class Mocks :
    virtual public ::testing::NiceMock<GlibMock> {};
} // namespace

using ::testing::_;
using ::testing::Invoke;
using ::testing::Return;

class NotificationOngoingTest : public TestFixture {
 public:
  NotificationOngoingTest() : TestFixture(std::make_unique<::Mocks>()) {}
  virtual void SetUp() {
  }
  virtual void TearDown() {
  }
};

void __notification_ongoing_update_cb(struct ongoing_info_s *info, void *data) {
}

TEST_F(NotificationOngoingTest, notification_ongoing_test) {
  int ret;

  ret = notification_ongoing_update_cb_set(nullptr, nullptr);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  ret = notification_ongoing_update_cb_unset();
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  // notification_ongoing.c:47-69 — three update helpers
  // (_progress / _size / _content) are file-private (no EXPORT_API)
  // and cannot be linked from this test binary. They are
  // currently stubs returning NONE; Phase 2 will revisit if they
  // gain real behavior — see F-1.6-A.
}

TEST_F(NotificationOngoingTest, ongoing_update_cb_set_with_callback_returns_none) {
  // Even passing a real callback doesn't change the stubbed return.
  EXPECT_EQ(notification_ongoing_update_cb_set(
                __notification_ongoing_update_cb, nullptr),
            NOTIFICATION_ERROR_NONE);
}
