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

#include "notification-ex/icon_text_item.h"
#include "notification-ex/item_inflator.h"

using namespace notification::item;

class IconTextItemTest : public ::testing::Test {
 public:
  IconTextItem* item;
  std::string id = "icontext_id";
  virtual void SetUp() {
    item = new IconTextItem(id,
      std::make_shared<IconItem>("icon_id", "icon_path"),
      std::make_shared<TextItem>("text_id", "title"));
  }
  virtual void TearDown() {
    delete item;
  }
};

TEST_F(IconTextItemTest, create) {
  EXPECT_NE(IconTextItemTest::item, nullptr);
}

TEST_F(IconTextItemTest, FindByID) {
  AbstractItem& child = IconTextItemTest::item->FindByID(IconTextItemTest::id);
  IconTextItem& icontext = static_cast<IconTextItem&>(child);
  ASSERT_EQ(icontext.GetIconItem().GetImagePath(), "icon_path");
  ASSERT_EQ(icontext.GetTextItem().GetContents(), "title");
}

TEST_F(IconTextItemTest, FindByIDNullItemReturn) {
  AbstractItem& child = IconTextItemTest::item->FindByID("not_existed_item");
  ASSERT_EQ(child.GetType(), IconItem::NullObject);
}

TEST_F(IconTextItemTest, SerializeDeserialize) {
  Bundle b = IconTextItemTest::item->Serialize();

  ItemFactory factory;
  std::shared_ptr<AbstractItem> gen_item = ItemInflator::Create(factory, b);
  IconTextItem* gen_icon = static_cast<IconTextItem*>(gen_item.get());
  ASSERT_EQ(IconTextItemTest::item->GetIconItem().GetImagePath(), gen_icon->GetIconItem().GetImagePath());
  ASSERT_EQ(IconTextItemTest::item->GetTextItem().GetContents(), gen_icon->GetTextItem().GetContents());
}
