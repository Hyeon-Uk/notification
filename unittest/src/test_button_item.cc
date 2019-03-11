// Copyright (c) 2019 Samsung Electronics Co., Ltd All Rights Reserved
// Use of this source code is governed by a apache 2.0 license that can be
// found in the LICENSE file.

#include <gmock/gmock.h>

#include "notification-ex/button_item.h"
#include "notification-ex/item_inflator.h"

using namespace notification::item;
using namespace std;

namespace {

class ButtonItemTest : public ::testing::Test {
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
  ASSERT_EQ(gen_item.get()->GetType(), item.GetType());

  ButtonItem* gen_btn = static_cast<ButtonItem*>(gen_item.get());
  ASSERT_EQ(item.GetTitle(), gen_btn->GetTitle());
}

}  // namespace
