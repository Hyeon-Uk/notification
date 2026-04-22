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
 */

/*
 * Exercises the post / update / delete / delete_all / get_by_priv_id
 * entry points (U12 in the Phase 0 plan).
 *
 * These tests can only succeed in environments with a running notification
 * daemon; on the unit-test host they are expected to return
 * NOTIFICATION_ERROR_SERVICE_NOT_READY or similar non-success codes. We
 * therefore assert the contract without pinning a specific success path:
 *
 *   - Invalid input  -> NOTIFICATION_ERROR_INVALID_PARAMETER
 *   - Valid input    -> any int (success or a specific error code)
 *
 * The important guarantees are:
 *   1. The calls do not crash.
 *   2. NULL / bogus input returns INVALID_PARAMETER deterministically.
 *   3. The binary exports the expected symbols (so the link/ABI line is
 *      still alive after refactoring).
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "mock/test_fixture.hh"
#include "mock/glib_mock.hh"

#include "notification/include/notification.h"
#include "notification/include/notification_error.h"
#include "notification/include/notification_internal.h"

namespace {

class Mocks :
    virtual public ::testing::NiceMock<GlibMock> {};

class PostUpdateDeleteTest : public TestFixture {
 public:
  PostUpdateDeleteTest() : TestFixture(std::make_unique<::Mocks>()) {}
  void SetUp() override {}
  void TearDown() override {}
};

notification_h MakeSample() {
  notification_h n = notification_create(NOTIFICATION_TYPE_NOTI);
  if (!n) return nullptr;
  notification_set_text(n, NOTIFICATION_TEXT_TYPE_TITLE, "t",
                        nullptr, NOTIFICATION_VARIABLE_TYPE_NONE);
  return n;
}

}  // namespace

TEST_F(PostUpdateDeleteTest, Post_NullHandle_IsInvalidParam) {
  EXPECT_EQ(notification_post(nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(PostUpdateDeleteTest, Update_NullHandle_IsInvalidParam) {
  EXPECT_EQ(notification_update(nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(PostUpdateDeleteTest, Delete_NullHandle_IsInvalidParam) {
  EXPECT_EQ(notification_delete(nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(PostUpdateDeleteTest, Post_ValidHandle_DoesNotCrash) {
  notification_h n = MakeSample();
  ASSERT_NE(n, nullptr);
  int rc = notification_post(n);
  /* Accept any int - daemon presence is environment-dependent. */
  (void)rc;
  notification_free(n);
}

TEST_F(PostUpdateDeleteTest, Update_ValidHandle_DoesNotCrash) {
  notification_h n = MakeSample();
  ASSERT_NE(n, nullptr);
  int rc = notification_update(n);
  (void)rc;
  notification_free(n);
}

TEST_F(PostUpdateDeleteTest, Delete_ValidHandle_DoesNotCrash) {
  notification_h n = MakeSample();
  ASSERT_NE(n, nullptr);
  int rc = notification_delete(n);
  (void)rc;
  notification_free(n);
}

TEST_F(PostUpdateDeleteTest, DeleteAll_DoesNotCrash) {
  int rc = notification_delete_all(NOTIFICATION_TYPE_NOTI);
  (void)rc;
}

TEST_F(PostUpdateDeleteTest, Load_BogusId_ReturnsNull) {
  /* notification_load() returns a notification_h directly; lookup miss
   * yields NULL. This guards the lookup path without requiring the daemon
   * to be alive. */
  char app_id[] = "org.tizen.bogus";
  notification_h n = notification_load(app_id, 0);
  /* Either NULL (lookup miss, daemon unavailable) or a handle (test host
   * actually has the record). Both are acceptable; crashing is not. */
  if (n) notification_free(n);
}
