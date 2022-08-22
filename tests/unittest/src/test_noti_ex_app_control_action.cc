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

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "mock/app_common_mock.h"
#include "mock/test_fixture.h"

#include "notification-ex/app_control_action.h"
#include "notification-ex/action_inflator.h"

using namespace tizen_base;
using namespace notification;
using namespace notification::item;

namespace {
class Mocks :
    virtual public ::testing::NiceMock<AppCommonMock> {};
} // namespace

class AppControlActionTest : public TestFixture {
 public:
  AppControlActionTest() : TestFixture(std::make_unique<::Mocks>()) {}
  AppControlAction* action;
  app_control_h app_control;
  const char* app_id = "test_appid";
  std::string extra = "appcontrol_extra";

  virtual void SetUp() {
    app_control_create(&app_control);
    app_control_set_app_id(app_control, app_id);

    action = new AppControlAction(app_control, extra);
  }
  virtual void TearDown() {
    delete action;
    app_control_destroy(app_control);
  }
};

TEST_F(AppControlActionTest, create) {
  EXPECT_NE(AppControlActionTest::action, nullptr);
}

TEST_F(AppControlActionTest, IsLocal) {
  ASSERT_EQ(AppControlActionTest::action->IsLocal(), 1);
}

TEST_F(AppControlActionTest, GetExtra) {
  ASSERT_EQ(AppControlActionTest::action->GetExtra(),
    AppControlActionTest::extra);
}

TEST_F(AppControlActionTest, SerializeDeserialize) {
  Bundle b = AppControlActionTest::action->Serialize();

  std::shared_ptr<AbstractAction> gen_action = ActionInflator::Create(b);
  ASSERT_EQ(gen_action->GetType(), AbstractAction::AppControl);

  AppControlAction* gen_appcontrol_action =
    static_cast<AppControlAction*>(gen_action.get());

  char* app_id = nullptr;
  app_control_get_app_id(gen_appcontrol_action->GetAppControl(), &app_id);
  std::unique_ptr<char, decltype(std::free)*> ptr(app_id, std::free);
  ASSERT_STREQ(app_id, AppControlActionTest::app_id);
}

TEST_F(AppControlActionTest, AppControl) {
  app_control_h app_control;
  const char* app_id = "new_appid";

  app_control_create(&app_control);
  app_control_set_app_id(app_control, app_id);
  AppControlActionTest::action->SetAppControl(app_control);
  app_control_destroy(app_control);

  Bundle b = AppControlActionTest::action->Serialize();

  std::shared_ptr<AbstractAction> gen_action = ActionInflator::Create(b);
  AppControlAction* gen_appcontrol_action =
    static_cast<AppControlAction*>(gen_action.get());

  char* app_id_ = nullptr;
  app_control_get_app_id(gen_appcontrol_action->GetAppControl(), &app_id_);
  std::unique_ptr<char, decltype(std::free)*> ptr(app_id_, std::free);
  EXPECT_STREQ(app_id, app_id_);
}
