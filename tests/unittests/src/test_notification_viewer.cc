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
#include "notification/include/notification_viewer.h"

namespace {

class Mocks :
    virtual public ::testing::NiceMock<GlibMock> {};
} // namespace

using ::testing::_;
using ::testing::Invoke;
using ::testing::Return;

class NotificationViewerTest : public TestFixture {
 public:
  NotificationViewerTest() : TestFixture(std::make_unique<::Mocks>()) {}
  virtual void SetUp() {}
  virtual void TearDown() {}
};

TEST_F(NotificationViewerTest, notification_init_default_viewer_no_conf_file) {
  EXPECT_CALL(GetMock<GlibMock>(), access(_, _))
    .WillOnce(Return(-1));

  int ret = notification_init_default_viewer();
  EXPECT_EQ(ret, -1);
}

TEST_F(NotificationViewerTest, notification_launch_default_viewer_no_viewer) {
  /* When _default_viewer is NULL, it should return NOTIFICATION_ERROR_NONE */
  int ret = notification_launch_default_viewer(1,
      NOTIFICATION_OP_INSERT, 5001);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
}

TEST_F(NotificationViewerTest, notification_launch_default_viewer_without_candidate_no_viewer) {
  /* When _default_viewer is NULL, it should return NOTIFICATION_ERROR_NONE */
  int ret = notification_launch_default_viewer_without_candidate_process(1,
      NOTIFICATION_OP_INSERT, 5001);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
}

TEST_F(NotificationViewerTest, notification_launch_default_viewer_update) {
  int ret = notification_launch_default_viewer(1,
      NOTIFICATION_OP_UPDATE, 5001);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
}

TEST_F(NotificationViewerTest, notification_launch_default_viewer_delete) {
  int ret = notification_launch_default_viewer(1,
      NOTIFICATION_OP_DELETE, 5001);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
}

TEST_F(NotificationViewerTest, notification_launch_default_viewer_without_candidate_update) {
  int ret = notification_launch_default_viewer_without_candidate_process(1,
      NOTIFICATION_OP_UPDATE, 5001);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
}

TEST_F(NotificationViewerTest, notification_launch_default_viewer_without_candidate_delete) {
  int ret = notification_launch_default_viewer_without_candidate_process(1,
      NOTIFICATION_OP_DELETE, 5001);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
}
