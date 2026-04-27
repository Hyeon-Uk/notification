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

/*
 * N7.5-A: Internal API Coverage for src/notification/src/notification_ongoing.c
 *
 * The production file is a placeholder — every API logs a "not
 * working now" warning and returns NOTIFICATION_ERROR_NONE. This
 * test pins that contract so that any future re-enablement
 * surfaces as a behaviour change in CI rather than silently
 * starting to do work and breaking callers that depend on the
 * NOTIFICATION_ERROR_NONE / no-op semantics today.
 *
 * Behaviours covered (no production change — coverage only):
 *   1. notification_ongoing_update_cb_set returns NONE for
 *      callback==NULL, callback!=NULL, and arbitrary user_data.
 *   2. notification_ongoing_update_cb_unset returns NONE.
 *   3. notification_ongoing_update_progress accepts boundary
 *      progress values (0.0, 50.0, 100.0, -1.0, 101.0) without
 *      touching real DBus or DB layers (mock GLib injected).
 *   4. notification_ongoing_update_size handles NULL caller_app_id,
 *      negative size, and a representative positive size.
 *   5. notification_ongoing_update_content handles NULL caller and
 *      NULL content without crashing.
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

class NotificationOngoingInternalTest : public TestFixture {
 public:
  NotificationOngoingInternalTest()
      : TestFixture(std::make_unique<::Mocks>()) {}
  void SetUp() override {}
  void TearDown() override {}
};

void __ongoing_cb_for_test(struct ongoing_info_s *info, void *data) {
  (void)info;
  (void)data;
}

}  // namespace

TEST_F(NotificationOngoingInternalTest, UpdateCbSet_NullCallback_ReturnsNone) {
  /* Production path is a stub WARN+return-NONE — both NULL and
   * non-NULL callback must yield NOTIFICATION_ERROR_NONE so that
   * callers receive a stable "feature disabled" signal instead of
   * a hard error. */
  ASSERT_EQ(notification_ongoing_update_cb_set(nullptr, nullptr),
            NOTIFICATION_ERROR_NONE);
}

TEST_F(NotificationOngoingInternalTest, UpdateCbSet_RealCallback_ReturnsNone) {
  int user_data = 42;
  ASSERT_EQ(notification_ongoing_update_cb_set(__ongoing_cb_for_test,
                                               &user_data),
            NOTIFICATION_ERROR_NONE);
}

TEST_F(NotificationOngoingInternalTest, UpdateCbUnset_ReturnsNone) {
  ASSERT_EQ(notification_ongoing_update_cb_unset(),
            NOTIFICATION_ERROR_NONE);
}

TEST_F(NotificationOngoingInternalTest,
       UpdateCbUnset_AfterSet_StillReturnsNone) {
  /* Pin idempotence — set/unset/unset all return NONE. */
  ASSERT_EQ(notification_ongoing_update_cb_set(__ongoing_cb_for_test, nullptr),
            NOTIFICATION_ERROR_NONE);
  ASSERT_EQ(notification_ongoing_update_cb_unset(),
            NOTIFICATION_ERROR_NONE);
  ASSERT_EQ(notification_ongoing_update_cb_unset(),
            NOTIFICATION_ERROR_NONE);
}

TEST_F(NotificationOngoingInternalTest, UpdateProgress_BoundaryValues) {
  const double values[] = {0.0, 50.0, 100.0, -1.0, 101.0};
  for (double v : values) {
    EXPECT_EQ(notification_ongoing_update_progress("org.tizen.test", 1, v),
              NOTIFICATION_ERROR_NONE) << "progress=" << v;
  }
}

TEST_F(NotificationOngoingInternalTest, UpdateProgress_NullCallerAppId) {
  /* Stub does not dereference caller_app_id; pin that contract. */
  ASSERT_EQ(notification_ongoing_update_progress(nullptr, 0, 0.0),
            NOTIFICATION_ERROR_NONE);
}

TEST_F(NotificationOngoingInternalTest, UpdateSize_VariousValues) {
  /* All three branches taken: 0, positive, negative. */
  EXPECT_EQ(notification_ongoing_update_size("org.tizen.test", 1, 0.0),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(notification_ongoing_update_size("org.tizen.test", 1, 1024.0),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(notification_ongoing_update_size("org.tizen.test", 1, -1.0),
            NOTIFICATION_ERROR_NONE);
}

TEST_F(NotificationOngoingInternalTest, UpdateSize_NullCallerAppId) {
  ASSERT_EQ(notification_ongoing_update_size(nullptr, 0, 0.0),
            NOTIFICATION_ERROR_NONE);
}

TEST_F(NotificationOngoingInternalTest, UpdateContent_RealString) {
  ASSERT_EQ(notification_ongoing_update_content("org.tizen.test", 1,
                                                "test content"),
            NOTIFICATION_ERROR_NONE);
}

TEST_F(NotificationOngoingInternalTest, UpdateContent_NullArgs) {
  /* Stub doesn't dereference; pin no-crash contract. */
  EXPECT_EQ(notification_ongoing_update_content(nullptr, 0, nullptr),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(notification_ongoing_update_content("org.tizen.test", 1, nullptr),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(notification_ongoing_update_content(nullptr, 0, "x"),
            NOTIFICATION_ERROR_NONE);
}
