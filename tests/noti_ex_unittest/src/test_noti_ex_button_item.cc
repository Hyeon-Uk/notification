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

#include <gmock/gmock.h>

#include "mock/app_common_mock.hh"
#include "mock/test_fixture.hh"
#include "notification-ex/button_item.h"
#include "notification-ex/item_inflator.h"

using namespace tizen_base;
using namespace notification;
using namespace notification::item;
using namespace std;

namespace {
class Mocks :
    virtual public ::testing::NiceMock<AppCommonMock> {};
} // namespace

class ButtonItemTest : public TestFixture {
 public:
  ButtonItemTest() : TestFixture(std::make_unique<::Mocks>()) {}
 protected:
  void SetUp() override {}
  void TearDown() override {}
};


TEST_F(ButtonItemTest, FindByID) {
  ButtonItem item("btn_id", "title");

  AbstractItem& child = item.FindByID("btn_id");
  ButtonItem& btn = static_cast<ButtonItem&>(child);
  ASSERT_EQ(btn.GetTitle(), "title");
}

TEST_F(ButtonItemTest, FindByIDNullItemReturn) {
  ButtonItem item("btn_id", "title");

  AbstractItem& child = item.FindByID("not_exist_button");
  ASSERT_EQ(child.GetType(), AbstractItem::NullObject);
}

TEST_F(ButtonItemTest, SerializeDeserializeGetTitle) {
  ButtonItem item("title");
  Bundle b = item.Serialize();
  shared_ptr<AbstractItem> gen_item = ItemInflator::Create(b);
  ASSERT_EQ(gen_item->GetType(), item.GetType());

  auto gen_btn = std::static_pointer_cast<ButtonItem>(gen_item);
  ASSERT_EQ(item.GetTitle(), gen_btn->GetTitle());
}

TEST_F(ButtonItemTest, GetSetPath) {
  ButtonItem item("btn_id", "title");

  item.SetImgPath("testpath");
  std::string path = item.GetImgPath();
  ASSERT_STREQ(path.c_str(), "testpath");
}

TEST_F(ButtonItemTest, GetSeContent) {
  ButtonItem item("btn_id", "title");

  item.SetContents("testcontents");
  std::string content = item.GetContents();
  ASSERT_STREQ(content.c_str(), "testcontents");
}
