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

#include "notification-ex/visibility_action.h"

using namespace notification;
using namespace notification::item;

class VisibilityActionTest : public ::testing::Test {
 public:
  VisibilityAction* visibility;
  std::string id = "visibility_id";
  bool visible = true;

  virtual void SetUp() {
    visibility = new VisibilityAction();
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

TEST_F(VisibilityActionTest, SerializeDeserialize) {
  Bundle b = VisibilityActionTest::visibility->Serialize();
  VisibilityActionTest::visibility->Deserialize(b);
}

TEST_F(VisibilityActionTest, SetVisibility) {
  VisibilityActionTest::visibility->SetVisibility("test_id", true);
}
