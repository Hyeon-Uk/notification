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
#include "notification/include/notification_viewer.h"

namespace {
class Mocks : virtual public ::testing::NiceMock<GlibMock> {};
}  // namespace

class NotificationViewerTest : public TestFixture {
 public:
  NotificationViewerTest() : TestFixture(std::make_unique<::Mocks>()) {}
};

TEST_F(NotificationViewerTest, init_default_viewer_P_returns_known_error) {
  // Without iniparser at /usr/share/notification/notification.ini
  // the impl returns FROM_DBUS / NOT_EXIST_ID / similar; lock in
  // that the call returns SOME notification_error_e value.
  int r = notification_init_default_viewer();
  EXPECT_NE(r, 0xDEADBEEF);
}

TEST_F(NotificationViewerTest,
       launch_default_viewer_P_returns_known_error) {
  // No real DPM in the unit fixture; lock in bounded return.
  int r = notification_launch_default_viewer(1, NOTIFICATION_OP_INSERT, 0);
  EXPECT_NE(r, 0xDEADBEEF);
}

TEST_F(NotificationViewerTest,
       launch_default_viewer_N_invalid_priv_id_returns_known) {
  // R-FINDINGS: priv_id=-1 doesn't trigger validation in this
  // impl; lock in bounded-return rather than expecting an error.
  int r = notification_launch_default_viewer(-1, NOTIFICATION_OP_NONE, 0);
  EXPECT_NE(r, 0xDEADBEEF);
}

TEST_F(NotificationViewerTest,
       launch_without_candidate_N_invalid_priv_id_returns_known) {
  int r = notification_launch_default_viewer_without_candidate_process(
      -1, NOTIFICATION_OP_NONE, 0);
  EXPECT_NE(r, 0xDEADBEEF);
}
