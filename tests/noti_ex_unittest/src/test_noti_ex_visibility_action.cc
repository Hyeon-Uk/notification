/*
 * Copyright (c) 2019 Samsung Electronics Co., Ltd.
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

#include <string>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "mock/app_common_mock.hh"
#include "mock/test_fixture.hh"
#include "notification-ex/visibility_action.h"
#include "notification-ex/action_inflator.h"

using namespace tizen_base;
using namespace notification;
using namespace notification::item;

namespace {
class Mocks :
    virtual public ::testing::NiceMock<AppCommonMock> {};
} // namespace

class VisibilityActionTest : public TestFixture {
 public:
  VisibilityActionTest() : TestFixture(std::make_unique<::Mocks>()) {}
  VisibilityAction* visibility;
  std::string id = "visibility_id";
  std::string extra = "visibility_extra";
  bool visible = true;

  virtual void SetUp() {
    visibility = new VisibilityAction(extra);
    visibility->SetVisibility("visibility_id", visible);
  }
  virtual void TearDown() {
    delete visibility;
  }
};

TEST_F(VisibilityActionTest, create) {
  EXPECT_NE(VisibilityActionTest::visibility, nullptr);
}

TEST_F(VisibilityActionTest, IsLocal) {
  ASSERT_EQ(VisibilityActionTest::visibility->IsLocal(), 1);
}

TEST_F(VisibilityActionTest, GetExtra) {
  ASSERT_EQ(VisibilityActionTest::visibility->GetExtra(),
    VisibilityActionTest::extra);
}

TEST_F(VisibilityActionTest, SerializeDeserialize) {
  Bundle b = VisibilityActionTest::visibility->Serialize();

  std::shared_ptr<AbstractAction> gen_action = ActionInflator::Create(b);
  ASSERT_EQ(gen_action->GetType(), AbstractAction::Visibility);

  auto gen_visibility_action =
    std::static_pointer_cast<VisibilityAction>(gen_action);
  EXPECT_NE(gen_visibility_action, nullptr);
  ASSERT_EQ(gen_visibility_action->IsLocal(), true);
  ASSERT_EQ(gen_visibility_action->GetExtra(), VisibilityActionTest::extra);
}

TEST_F(VisibilityActionTest, SetVisibility) {
  VisibilityActionTest::visibility->SetVisibility("test_id", true);
  Bundle b = VisibilityActionTest::visibility->Serialize();

  std::shared_ptr<AbstractAction> gen_action = ActionInflator::Create(b);
  ASSERT_EQ(gen_action->GetType(), AbstractAction::Visibility);

  auto gen_visibility_action =
    std::static_pointer_cast<VisibilityAction>(gen_action);
  EXPECT_NE(gen_visibility_action, nullptr);
  ASSERT_EQ(gen_visibility_action->IsLocal(), true);
  ASSERT_EQ(gen_visibility_action->GetExtra(), VisibilityActionTest::extra);
}
