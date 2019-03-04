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

#include "notification-ex/app_control_action.h"

using namespace notification;
using namespace notification::item;

class AppControlActionTest : public ::testing::Test {
 public:
  AppControlAction* action;
  app_control_h app_control;
  const char* app_id = "test_appid";

  virtual void SetUp() {
    app_control_create(&app_control);
    app_control_set_app_id(app_control, app_id);

    action = new AppControlAction(app_control);
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

TEST_F(AppControlActionTest, SerializeDeserialize) {
  char* app_id = NULL;

  Bundle b = AppControlActionTest::action->Serialize();
  AppControlActionTest::action->Deserialize(b);

  app_control_get_app_id(AppControlActionTest::action->GetAppControl(), &app_id);
  ASSERT_STREQ(AppControlActionTest::app_id, app_id);
}

TEST_F(AppControlActionTest, AppControl) {
  const char* app_id = "new_appid";
  char* app_id_ = NULL;
  app_control_h app_control;
  app_control_h app_control_;

  app_control_create(&app_control);
  app_control_set_app_id(app_control, app_id);

  AppControlActionTest::action->SetAppControl(app_control);
  app_control_ = AppControlActionTest::action->GetAppControl();
  app_control_get_app_id(app_control_, &app_id_);

  EXPECT_STREQ(app_id, app_id_);
}
