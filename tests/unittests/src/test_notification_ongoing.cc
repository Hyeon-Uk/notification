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

TEST_F(NotificationOngoingTest, happy_path_combined) {
  int ret;

  ret = notification_ongoing_update_cb_set(nullptr, nullptr);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  ret = notification_ongoing_update_cb_unset();
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  // ret = notification_ongoing_update_progress("org.tizen.caller_appid", 1, 10.0);
  // ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  // ret = notification_ongoing_update_size("org.tizen.caller_appid", 1, 10.0);
  // ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  // ret = notification_ongoing_update_content("org.tizen.caller_appid", 1, "test_content");
  // ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);
}

TEST_F(NotificationOngoingTest, cb_set_with_null_callback_returns_NONE) {
  // returns NONE without registering anything. Lock in this
  // behavior so a refactor that adds NULL-validation is caught.
  EXPECT_EQ(notification_ongoing_update_cb_set(nullptr, nullptr),
            NOTIFICATION_ERROR_NONE);
}

TEST_F(NotificationOngoingTest, cb_unset_idempotent) {
  EXPECT_EQ(notification_ongoing_update_cb_unset(),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(notification_ongoing_update_cb_unset(),
            NOTIFICATION_ERROR_NONE);
}
