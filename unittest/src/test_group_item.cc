// Copyright (c) 2019 Samsung Electronics Co., Ltd All Rights Reserved
// Use of this source code is governed by a apache 2.0 license that can be
// found in the LICENSE file.

#include <gmock/gmock.h>

#include "notification-ex/group_item.h"
#include "notification-ex/button_item.h"
#include "notification-ex/item_inflator.h"
#include "unittest/mock/app_common.h"

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

TEST_F(GroupItemTest, SerializeDeserialize) {
  GroupItem item("GROUP1");
  item.AddChild(std::make_shared<ButtonItem>("btn1", "test1"));
  item.AddChild(std::make_shared<ButtonItem>("btn2", "test2"));

  Bundle b = item.Serialize();
  ItemFactory factory;
  shared_ptr<AbstractItem> gen_item = ItemInflator::Create(factory, b);
  ASSERT_EQ(gen_item.get()->GetType(), item.GetType());

  GroupItem* gen_group = static_cast<GroupItem*>(gen_item.get());
  list<shared_ptr<AbstractItem>> gen_children_list = gen_group->GetChildren();
  list<shared_ptr<AbstractItem>> origin_children_list = item.GetChildren();

  list<shared_ptr<AbstractItem>>::iterator it = origin_children_list.begin();
  for (auto& i : gen_children_list) {
    ASSERT_EQ(i.get()->GetType(), (*it).get()->GetType());
    if (i.get()->GetType() == AbstractItem::Button) {
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

TEST_F(GroupItemTest, FindByIDNullItemReturn) {
  GroupItem item("GROUP1");
  item.AddChild(std::make_shared<ButtonItem>("btn1", "test1"));
  item.AddChild(std::make_shared<ButtonItem>("btn2", "test2"));
  ASSERT_EQ(item.GetChildren().size(), 2);

  AbstractItem& child = item.FindByID("not_exist_button");
  ASSERT_EQ(child.GetType(), AbstractItem::NullObject);
}

int __fake_app_get_name(char** app_name) {
  *app_name = (char*)"unittest_appname";
  return 0;
}

TEST_F(GroupItemTest, GetAppLabel) {
  app_get_name_fake.custom_fake = __fake_app_get_name;

  GroupItem item("GROUP1");
  string app_label = item.GetAppLabel();

  ASSERT_EQ(app_label, "unittest_appname");
}

TEST_F(GroupItemTest, SetDirection) {
  GroupItem item("GROUP1");
  item.SetDirection(true);

  ASSERT_TRUE(item.IsVertical());
}

}  // namespace
