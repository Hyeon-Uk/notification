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
#include "mock/aul_mock.hh"
#include "mock/glib_mock.hh"
#include "mock/package_manager_mock.hh"
#include "mock/tzplatform_config_mock.hh"

#include "notification/include/notification.h"
#include "notification/include/notification_internal.h"

namespace {

int __fake_aul_app_get_appid_bypid(int pid, char* appid, int len) {
  snprintf(appid, len, "%s", "org.tizen.testappid");
  return AUL_R_OK;
}

class Mocks :
    virtual public ::testing::NiceMock<AulMock>,
    virtual public ::testing::NiceMock<GlibMock>,
    virtual public ::testing::NiceMock<PackageManagerMock>,
    virtual public ::testing::NiceMock<TzplatformConfigMock> {};
}  // namespace

using ::testing::_;
using ::testing::Invoke;

class NotificationInternalTest : public TestFixture {
 public:
  NotificationInternalTest() : TestFixture(std::make_unique<::Mocks>()) {}
};

// notification_internal.c:2442 — channel-name setter / getter
// INVALID_PARAMETER paths.
TEST_F(NotificationInternalTest, set_channel_name_rejects_null_args) {
  EXPECT_EQ(notification_set_channel_name(nullptr, "ch"),
            NOTIFICATION_ERROR_INVALID_PARAMETER);

  EXPECT_CALL(GetMock<AulMock>(), aul_app_get_appid_bypid(_, _, _))
      .WillRepeatedly(Invoke(__fake_aul_app_get_appid_bypid));
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  EXPECT_EQ(notification_set_channel_name(noti, nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);

  // Happy path: round-trip through getter.
  ASSERT_EQ(notification_set_channel_name(noti, "test-channel"),
            NOTIFICATION_ERROR_NONE);
  const char* got = nullptr;
  ASSERT_EQ(notification_get_channel_name(noti, &got),
            NOTIFICATION_ERROR_NONE);
  EXPECT_STREQ(got, "test-channel");

  notification_free(noti);
}

TEST_F(NotificationInternalTest, get_channel_name_rejects_null_args) {
  const char* got = nullptr;
  EXPECT_EQ(notification_get_channel_name(nullptr, &got),
            NOTIFICATION_ERROR_INVALID_PARAMETER);

  EXPECT_CALL(GetMock<AulMock>(), aul_app_get_appid_bypid(_, _, _))
      .WillRepeatedly(Invoke(__fake_aul_app_get_appid_bypid));
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  EXPECT_EQ(notification_get_channel_name(noti, nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);

  notification_free(noti);
}

// notification_internal.c:2504 — channel create / free.
TEST_F(NotificationInternalTest, channel_create_rejects_null_args) {
  EXPECT_CALL(GetMock<AulMock>(), aul_app_get_appid_bypid(_, _, _))
      .WillRepeatedly(Invoke(__fake_aul_app_get_appid_bypid));

  notification_channel_h ch = nullptr;
  EXPECT_EQ(notification_channel_create(nullptr, &ch),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_channel_create("name", nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);

  ASSERT_EQ(notification_channel_create("test-ch", &ch),
            NOTIFICATION_ERROR_NONE);
  ASSERT_NE(ch, nullptr);

  // notification_channel_get_name round-trip.
  const char* name = nullptr;
  ASSERT_EQ(notification_channel_get_name(ch, &name),
            NOTIFICATION_ERROR_NONE);
  EXPECT_STREQ(name, "test-ch");

  notification_channel_free(ch);
}

TEST_F(NotificationInternalTest, channel_free_null_is_safe_noop) {
  // notification_internal.c:2532 — free(NULL) early-returns; must not
  // crash.
  notification_channel_free(nullptr);
  SUCCEED();
}

// notification_internal.c:2670+ — channel block/blockable getters/setters.
TEST_F(NotificationInternalTest, channel_blockable_round_trip) {
  EXPECT_CALL(GetMock<AulMock>(), aul_app_get_appid_bypid(_, _, _))
      .WillRepeatedly(Invoke(__fake_aul_app_get_appid_bypid));

  notification_channel_h ch = nullptr;
  ASSERT_EQ(notification_channel_create("ch", &ch),
            NOTIFICATION_ERROR_NONE);

  ASSERT_EQ(notification_channel_set_blockable(ch, true),
            NOTIFICATION_ERROR_NONE);
  bool b = false;
  ASSERT_EQ(notification_channel_get_blockable(ch, &b),
            NOTIFICATION_ERROR_NONE);
  EXPECT_TRUE(b);

  ASSERT_EQ(notification_channel_set_block(ch, true),
            NOTIFICATION_ERROR_NONE);
  bool blk = false;
  ASSERT_EQ(notification_channel_get_block(ch, &blk),
            NOTIFICATION_ERROR_NONE);
  EXPECT_TRUE(blk);

  // INVALID_PARAMETER paths.
  EXPECT_EQ(notification_channel_set_blockable(nullptr, true),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_channel_get_blockable(nullptr, &b),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_channel_set_block(nullptr, true),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_channel_get_block(nullptr, &blk),
            NOTIFICATION_ERROR_INVALID_PARAMETER);

  notification_channel_free(ch);
}

TEST_F(NotificationInternalTest, channel_get_name_rejects_null_args) {
  EXPECT_CALL(GetMock<AulMock>(), aul_app_get_appid_bypid(_, _, _))
      .WillRepeatedly(Invoke(__fake_aul_app_get_appid_bypid));

  const char* name = nullptr;
  EXPECT_EQ(notification_channel_get_name(nullptr, &name),
            NOTIFICATION_ERROR_INVALID_PARAMETER);

  notification_channel_h ch = nullptr;
  ASSERT_EQ(notification_channel_create("ch", &ch),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(notification_channel_get_name(ch, nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);

  notification_channel_free(ch);
}

// notification_internal.c:2763 — channel_clone deep-copies app_id,
// channel_name, blockable, is_blocked. Source rejects NULL channel,
// NULL clone-out, AND a channel with internal NULL app_id /
// channel_name (defensive guard).
TEST_F(NotificationInternalTest, channel_clone_round_trip_and_null_guards) {
  EXPECT_CALL(GetMock<AulMock>(), aul_app_get_appid_bypid(_, _, _))
      .WillRepeatedly(Invoke(__fake_aul_app_get_appid_bypid));

  notification_channel_h orig = nullptr;
  ASSERT_EQ(notification_channel_create("ch-orig", &orig),
            NOTIFICATION_ERROR_NONE);
  ASSERT_EQ(notification_channel_set_blockable(orig, true),
            NOTIFICATION_ERROR_NONE);
  ASSERT_EQ(notification_channel_set_block(orig, true),
            NOTIFICATION_ERROR_NONE);

  notification_channel_h clone = nullptr;
  ASSERT_EQ(notification_channel_clone(orig, &clone),
            NOTIFICATION_ERROR_NONE);
  ASSERT_NE(clone, nullptr);
  EXPECT_NE(clone, orig);

  // Each scalar/string copied independently.
  const char* clone_name = nullptr;
  ASSERT_EQ(notification_channel_get_name(clone, &clone_name),
            NOTIFICATION_ERROR_NONE);
  EXPECT_STREQ(clone_name, "ch-orig");
  bool b = false;
  ASSERT_EQ(notification_channel_get_blockable(clone, &b),
            NOTIFICATION_ERROR_NONE);
  EXPECT_TRUE(b);
  bool blk = false;
  ASSERT_EQ(notification_channel_get_block(clone, &blk),
            NOTIFICATION_ERROR_NONE);
  EXPECT_TRUE(blk);

  // INVALID_PARAMETER paths.
  notification_channel_h dummy = nullptr;
  EXPECT_EQ(notification_channel_clone(nullptr, &dummy),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_channel_clone(orig, nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);

  notification_channel_free(clone);
  notification_channel_free(orig);
}

// notification_internal.c — set/get_channel_name using a channel
// already populated with non-NULL app_id (covers the strdup +
// free-old branches in set_channel_name).
// notification_internal.c:2552-2627 — channel_add / _remove / _update
// each guard against NULL channel and NULL internal app_id / channel_name.
// They then call into _notification_tidl which we don't have a mock for
// (Phase 1.5-A) — but the NULL-guard branches are reachable today.

TEST_F(NotificationInternalTest, channel_add_rejects_null_args) {
  EXPECT_EQ(notification_channel_add(nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);

  // A channel struct with NULL internal pointers also rejects.
  // Construct manually since notification_channel_create always
  // strdup's, so we can't get there via the public constructor.
  struct notification_channel { char* app_id; char* channel_name;
                                bool blockable; bool is_blocked; };
  struct notification_channel ch_no_app{nullptr,
                                        const_cast<char*>("name"),
                                        false, false};
  EXPECT_EQ(notification_channel_add(
                reinterpret_cast<notification_channel_h>(&ch_no_app)),
            NOTIFICATION_ERROR_INVALID_PARAMETER);

  struct notification_channel ch_no_name{const_cast<char*>("app"),
                                         nullptr, false, false};
  EXPECT_EQ(notification_channel_add(
                reinterpret_cast<notification_channel_h>(&ch_no_name)),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationInternalTest, channel_remove_rejects_null_args) {
  EXPECT_EQ(notification_channel_remove(nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationInternalTest, channel_update_rejects_null_args) {
  EXPECT_EQ(notification_channel_update(nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationInternalTest, channel_get_by_name_rejects_null_args) {
  notification_channel_h ch = nullptr;
  EXPECT_EQ(notification_channel_get_by_name(nullptr, &ch),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_channel_get_by_name("name", nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationInternalTest, channel_foreach_rejects_null_callback) {
  EXPECT_EQ(notification_channel_foreach(nullptr, nullptr, nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  // Non-NULL callback path requires _notification_tidl_get / proxy
  // mocks — covered in Phase 1.5-A.
}

TEST_F(NotificationInternalTest, set_channel_name_overwrites_previous) {
  EXPECT_CALL(GetMock<AulMock>(), aul_app_get_appid_bypid(_, _, _))
      .WillRepeatedly(Invoke(__fake_aul_app_get_appid_bypid));

  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  ASSERT_EQ(notification_set_channel_name(noti, "first"),
            NOTIFICATION_ERROR_NONE);
  // Second call must free the prior strdup and replace.
  ASSERT_EQ(notification_set_channel_name(noti, "second"),
            NOTIFICATION_ERROR_NONE);

  const char* got = nullptr;
  ASSERT_EQ(notification_get_channel_name(noti, &got),
            NOTIFICATION_ERROR_NONE);
  EXPECT_STREQ(got, "second");

  notification_free(noti);
}
