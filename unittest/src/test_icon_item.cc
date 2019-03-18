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

#include "notification-ex/icon_item.h"
#include "notification-ex/item_inflator.h"

using namespace notification;
using namespace notification::item;

class IconItemTest : public ::testing::Test {
 public:
  IconItem* item;
  std::string id = "icon_id";
  virtual void SetUp() {
    item = new IconItem(id, "icon_path");
  }
  virtual void TearDown() {
    delete item;
  }
};

TEST_F(IconItemTest, create) {
  EXPECT_NE(IconItemTest::item, nullptr);
}

TEST_F(IconItemTest, FindByID) {
  AbstractItem& child = IconItemTest::item->FindByID(IconItemTest::id);
  IconItem& icon = static_cast<IconItem&>(child);
  ASSERT_EQ(icon.GetImagePath(), "icon_path");
}

TEST_F(IconItemTest, FindByIDNullItemReturn) {
  AbstractItem& child = IconItemTest::item->FindByID("not_existed_item");
  ASSERT_EQ(child.GetType(), IconItem::NullObject);
}

TEST_F(IconItemTest, SerializeDeserialize) {
  Bundle b = IconItemTest::item->Serialize();

  std::shared_ptr<AbstractItem> gen_item = ItemInflator::Create(b);
  ASSERT_EQ(IconItemTest::item->GetType(), gen_item->GetType());

  auto gen_icon = std::static_pointer_cast<IconItem>(gen_item);
  ASSERT_EQ(IconItemTest::item->GetImagePath(), gen_icon->GetImagePath());
}
