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
 * N3-T4 A2: regression guard for the AbstractItem by-value + std::move
 * setter path.
 *
 * The setters take std::string by value and std::move() it into the
 * impl member. This single signature yields the optimal call shape
 * for every input kind while avoiding the `const char*` ambiguity
 * that a separate `&&` overload would introduce at stub.cc call sites
 * (`SetSoundPath("")`, `SetTag(c_str)`, …).
 *
 * Invariant:
 *   - When the caller passes std::move(rvalue), the source string is
 *     consumed (valid-but-empty after the call) because the by-value
 *     parameter move-constructs from the rvalue and the body then
 *     moves out of the parameter into the member.
 *   - The getter round-trip matches the original source bytes.
 *   - Lvalue callers are unaffected (they pay one copy into the
 *     parameter, same as before).
 *
 * Fixture notes:
 *   ButtonItem ctor reaches into AbstractItem which calls
 *   util::GetAppId() → aul_app_get_appid_bypid(), an extern "C" mock
 *   defined in tests/mock/aul_mock.cc. Without an AulMock fixture the
 *   MOCK_HOOK throws at ctor time, so we install the same Mocks bundle
 *   that test_noti_ex_button_item.cc uses.
 */

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <string>
#include <utility>

#include "mock/app_common_mock.hh"
#include "mock/aul_mock.hh"
#include "mock/glib_mock.hh"
#include "mock/test_fixture.hh"

#include "notification-ex/abstract_item.h"
#include "notification-ex/button_item.h"

using notification::item::AbstractItem;
using notification::item::ButtonItem;

namespace {

class Mocks :
    virtual public ::testing::NiceMock<AppCommonMock>,
    virtual public ::testing::NiceMock<AulMock>,
    virtual public ::testing::NiceMock<GlibMock> {};

class SetterMoveTest : public TestFixture {
 public:
  SetterMoveTest() : TestFixture(std::make_unique<::Mocks>()) {}
 protected:
  void SetUp() override {}
  void TearDown() override {}
};

/* Canonical above-SSO-threshold payload. libstdc++ std::string SSO
 * buffer is 15 bytes; any string with size() > 15 forces a heap
 * allocation on copy. This makes the move observable via size()
 * going to 0 on the source. */
constexpr const char *kLongBlob =
    "dynamic-heap-owning-string-value-beyond-sso-threshold-0123456789";

TEST_F(SetterMoveTest, RvalueSetIdSkipsCopy) {
  ButtonItem item("button_id_1", "Click me");
  std::string payload = kLongBlob;
  ASSERT_GT(payload.size(), 15u) << "payload must exceed libstdc++ SSO size";

  item.SetId(std::move(payload));

  /* Moved-from std::string is in a valid-but-unspecified state; the
   * standard guarantees it is valid but not its exact contents. In
   * practice libstdc++ leaves it empty, which is what we assert here
   * (a test failure here means a copy happened instead of a move). */
  // NOLINTNEXTLINE(bugprone-use-after-move)
  EXPECT_TRUE(payload.empty())
      << "rvalue SetId must have moved the payload, not copied it";

  /* Round-trip: stored value must equal the original source. */
  EXPECT_EQ(item.GetId(), std::string(kLongBlob));
}

TEST_F(SetterMoveTest, RvalueSetChannelSkipsCopy) {
  ButtonItem item("button_id_2", "Click me");
  std::string payload = kLongBlob;

  item.SetChannel(std::move(payload));

  // NOLINTNEXTLINE(bugprone-use-after-move)
  EXPECT_TRUE(payload.empty());
  EXPECT_EQ(item.GetChannel(), std::string(kLongBlob));
}

TEST_F(SetterMoveTest, RvalueSetTagSkipsCopy) {
  ButtonItem item("button_id_3", "Click me");
  std::string payload = kLongBlob;

  item.SetTag(std::move(payload));

  // NOLINTNEXTLINE(bugprone-use-after-move)
  EXPECT_TRUE(payload.empty());
  EXPECT_EQ(item.GetTag(), std::string(kLongBlob));
}

TEST_F(SetterMoveTest, RvalueSetSenderAppIdSkipsCopy) {
  ButtonItem item("button_id_4", "Click me");
  std::string payload = kLongBlob;

  item.SetSenderAppId(std::move(payload));

  // NOLINTNEXTLINE(bugprone-use-after-move)
  EXPECT_TRUE(payload.empty());
  EXPECT_EQ(item.GetSenderAppId(), std::string(kLongBlob));
}

TEST_F(SetterMoveTest, RvalueSetSoundPathSkipsCopy) {
  ButtonItem item("button_id_5", "Click me");
  std::string payload = kLongBlob;

  item.SetSoundPath(std::move(payload));

  // NOLINTNEXTLINE(bugprone-use-after-move)
  EXPECT_TRUE(payload.empty());
  EXPECT_EQ(item.GetSoundPath(), std::string(kLongBlob));
}

TEST_F(SetterMoveTest, RvalueSetVibrationPathSkipsCopy) {
  ButtonItem item("button_id_6", "Click me");
  std::string payload = kLongBlob;

  item.SetVibrationPath(std::move(payload));

  // NOLINTNEXTLINE(bugprone-use-after-move)
  EXPECT_TRUE(payload.empty());
  EXPECT_EQ(item.GetVibrationPath(), std::string(kLongBlob));
}

/* Lvalue overload MUST still work identically: the argument is not
 * disturbed after the call. */
TEST_F(SetterMoveTest, LvalueOverloadDoesNotConsume) {
  ButtonItem item("button_id_7", "Click me");
  std::string payload = kLongBlob;
  std::string before_copy = payload;

  item.SetId(payload);

  EXPECT_EQ(payload, before_copy)
      << "lvalue SetId must NOT move out of caller's string";
  EXPECT_EQ(item.GetId(), std::string(kLongBlob));
}

/* Short-string (<=15 char) stays SSO even after the lvalue setter,
 * exercising the "15자 이하 key 는 SSO 활용" path from the design doc. */
TEST_F(SetterMoveTest, ShortStringSsoPath) {
  ButtonItem item("button_id_8", "Click me");
  std::string payload = "short-key";  // 9 bytes, well within SSO
  ASSERT_LE(payload.size(), 15u);

  item.SetTag(std::move(payload));

  EXPECT_EQ(item.GetTag(), std::string("short-key"));
}

}  // namespace
