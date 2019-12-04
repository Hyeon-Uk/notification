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

#include "notification-ex/group_item.h"
#include "notification-ex/button_item.h"
#include "notification-ex/text_item.h"
#include "notification-ex/item_inflator.h"
#include "unittest/mock/app_common.h"

using namespace tizen_base;
using namespace notification;
using namespace notification::item;
using namespace std;

namespace {

class GroupItemTest : public ::testing::Test {
 protected:
  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(GroupItemTest, AddChild) {
  GroupItem item("GROUP1");
  item.AddChild(std::make_shared<ButtonItem>("btn1", "test1"));
  item.AddChild(std::make_shared<ButtonItem>("btn2", "test2"));
  ASSERT_EQ(item.GetChildren().size(), 2);
}

TEST_F(GroupItemTest, RemoveChild) {
  GroupItem item("GROUP1");
  item.AddChild(std::make_shared<ButtonItem>("btn1", "test1"));
  item.AddChild(std::make_shared<ButtonItem>("btn2", "test2"));
  ASSERT_EQ(item.GetChildren().size(), 2);

  item.RemoveChild("btn1");
  ASSERT_EQ(item.GetChildren().size(), 1);
}

int __fake_app_get_name(char** app_name) {
  *app_name = strdup("unittest_appname");
  return 0;
}

TEST_F(GroupItemTest, SerializeDeserialize) {
  app_get_name_fake.custom_fake = __fake_app_get_name;

  GroupItem item("GROUP1");
  item.AddChild(std::make_shared<ButtonItem>("btn1", "test1"));
  item.AddChild(std::make_shared<ButtonItem>("btn2", "test2"));

  Bundle b = item.Serialize();
  shared_ptr<AbstractItem> gen_item = ItemInflator::Create(b);
  ASSERT_EQ(gen_item->GetType(), item.GetType());

  auto gen_group = static_pointer_cast<GroupItem>(gen_item);
  list<shared_ptr<AbstractItem>> gen_children_list = gen_group->GetChildren();
  list<shared_ptr<AbstractItem>> origin_children_list = item.GetChildren();

  list<shared_ptr<AbstractItem>>::iterator it = origin_children_list.begin();
  for (auto& i : gen_children_list) {
    ASSERT_EQ(i->GetType(), (*it)->GetType());
    if (i->GetType() == AbstractItem::Button) {
      ButtonItem* btn1 = static_cast<ButtonItem*>(i.get());
      ButtonItem* btn2 = static_cast<ButtonItem*>((*it).get());
      ASSERT_EQ(btn1->GetTitle(), btn2->GetTitle());
    }
    it++;
  }
  ASSERT_EQ(item.GetChildren().size(), 2);
}

TEST_F(GroupItemTest, FindByID) {
  GroupItem item("GROUP1");
  item.AddChild(std::make_shared<ButtonItem>("btn1", "test1"));
  item.AddChild(std::make_shared<ButtonItem>("btn2", "test2"));
  ASSERT_EQ(item.GetChildren().size(), 2);

  AbstractItem& child = item.FindByID("btn2");
  ButtonItem& btn = static_cast<ButtonItem&>(child);
  ASSERT_EQ(btn.GetTitle(), "test2");
}

TEST_F(GroupItemTest, FindByIDGroupItem) {
  GroupItem item("GROUP1");
  shared_ptr<GroupItem> gr2 = shared_ptr<GroupItem>(new GroupItem("GROUP2"));
  shared_ptr<GroupItem> gr3 = shared_ptr<GroupItem>(new GroupItem("GROUP3"));
  gr2->AddChild(std::make_shared<ButtonItem>("btn2", "test2"));
  gr2->AddChild(std::make_shared<ButtonItem>("btn3", "test3"));
  gr2->AddChild(std::make_shared<ButtonItem>("btn4", "test4"));
  gr3->AddChild(std::make_shared<ButtonItem>("btn6", "test6"));
  gr2->AddChild(gr3);

  item.AddChild(std::make_shared<ButtonItem>("btn1", "test1"));
  item.AddChild(gr2);
  item.AddChild(std::make_shared<ButtonItem>("btn5", "test5"));
  ASSERT_EQ(item.GetChildren().size(), 3);

  AbstractItem& child = item.FindByID("btn3");
  ButtonItem& btn = static_cast<ButtonItem&>(child);
  ASSERT_EQ(btn.GetTitle(), "test3");

  AbstractItem& child2 = item.FindByID("GROUP3");
  GroupItem& ret_gr = static_cast<GroupItem&>(child2);
  ASSERT_EQ(ret_gr.GetChildren().size(), 1);
}


TEST_F(GroupItemTest, IsItemTypeExist) {
  GroupItem item("GROUP1");
  shared_ptr<GroupItem> gr2 = shared_ptr<GroupItem>(new GroupItem("GROUP2"));
  shared_ptr<GroupItem> gr3 = shared_ptr<GroupItem>(new GroupItem("GROUP3"));
  gr2->AddChild(std::make_shared<ButtonItem>("btn2", "test2"));
  gr2->AddChild(std::make_shared<ButtonItem>("btn3", "test3"));
  gr2->AddChild(std::make_shared<ButtonItem>("btn4", "test4"));
  gr3->AddChild(std::make_shared<ButtonItem>("btn6", "test6"));
  gr3->AddChild(std::make_shared<TextItem>("text1", "text1"));
  gr2->AddChild(gr3);

  item.AddChild(std::make_shared<ButtonItem>("btn1", "test1"));
  item.AddChild(gr2);
  item.AddChild(std::make_shared<ButtonItem>("btn5", "test5"));
  ASSERT_EQ(item.GetChildren().size(), 3);

  ASSERT_EQ(item.IsItemTypeExist(AbstractItem::Text), true);
  ASSERT_EQ(item.IsItemTypeExist(AbstractItem::Image), false);
}

TEST_F(GroupItemTest, FindByIDNullItemReturn) {
  GroupItem item("GROUP1");
  item.AddChild(std::make_shared<ButtonItem>("btn1", "test1"));
  item.AddChild(std::make_shared<ButtonItem>("btn2", "test2"));
  ASSERT_EQ(item.GetChildren().size(), 2);

  AbstractItem& child = item.FindByID("not_exist_button");
  ASSERT_EQ(child.GetType(), AbstractItem::NullObject);
}

TEST_F(GroupItemTest, GetAppLabel) {
  app_get_name_fake.custom_fake = __fake_app_get_name;

  GroupItem item("GROUP1");
  string app_label = item.GetAppLabel();

  ASSERT_EQ(app_label, "unittest_appname");
}

TEST_F(GroupItemTest, SetAppLabel) {
  GroupItem item("GROUP1");
  item.SetAppLabel("test");
  string app_label = item.GetAppLabel();

  ASSERT_EQ(app_label, "test");
}

TEST_F(GroupItemTest, SetDirection) {
  GroupItem item("GROUP1");
  item.SetDirection(true);

  ASSERT_TRUE(item.IsVertical());
}

}  // namespace
