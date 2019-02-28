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

#include "notification-ex/checkbox_item.h"
#include "notification-ex/item_inflator.h"

using namespace notification::item;

class CheckBoxItemTest : public ::testing::Test {
 public:
  CheckBoxItem* item;
  std::string id = "checkbox_id";
  std::string title = "title";
  bool isChecked = false;
  virtual void SetUp() {
    item = new CheckBoxItem(id, title, isChecked);
  }
  virtual void TearDown() {
    delete item;
  }
};

TEST_F(CheckBoxItemTest, create) {
  EXPECT_NE(CheckBoxItemTest::item, nullptr);
}

TEST_F(CheckBoxItemTest, FindByID) {
  AbstractItem& child = CheckBoxItemTest::item->FindByID(CheckBoxItemTest::id);
  CheckBoxItem& checkbox = static_cast<CheckBoxItem&>(child);
  ASSERT_EQ(checkbox.GetTitle(), CheckBoxItemTest::title);
  ASSERT_EQ(checkbox.IsChecked(), CheckBoxItemTest::isChecked);
}

TEST_F(CheckBoxItemTest, FindByIDNullItemReturn) {
  AbstractItem& child = CheckBoxItemTest::item->FindByID("not_existed_item");
  ASSERT_EQ(child.GetType(), CheckBoxItem::NullObject);
}

TEST_F(CheckBoxItemTest, SerializeDeserialize) {
  Bundle b = CheckBoxItemTest::item->Serialize();

  ItemFactory factory;
  std::shared_ptr<AbstractItem> gen_item = ItemInflator::Create(factory, b);
  ASSERT_EQ(CheckBoxItemTest::item->GetType(), gen_item.get()->GetType());

  CheckBoxItem* gen_checkbox = static_cast<CheckBoxItem*>(gen_item.get());
  ASSERT_EQ(CheckBoxItemTest::item->GetTitle(), gen_checkbox->GetTitle());
}
