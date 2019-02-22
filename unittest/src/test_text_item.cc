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

#include "notification-ex/text_item.h"
#include "notification-ex/item_inflator.h"

using namespace notification::item;

namespace {

class TexttItemTest : public ::testing::Test {
 public:
  virtual void SetUp() {
  }
  virtual void TearDown() {
  }
};

TEST_F(TexttItemTest, FindByID) {
  TextItem item("text_id", "default");

  AbstractItem& child = item.FindByID("text_id");
  TextItem& btn = static_cast<TextItem&>(child);
  ASSERT_EQ(btn.GetContents(), "default");
}

TEST_F(TexttItemTest, FindByIDNullItemReturn) {
  TextItem item("text_id", "default");

  AbstractItem& child = item.FindByID("not_existed_item");
  ASSERT_EQ(child.GetType(), TextItem::NullObject);
}

TEST_F(TexttItemTest, SerializeDeserializeGetContents) {
  TextItem item("text_id", "default");
  Bundle b = item.Serialize();
  ItemFactory factory;
  std::shared_ptr<AbstractItem> gen_item = ItemInflator::Create(factory, b);
  ASSERT_EQ(gen_item.get()->GetType(), item.GetType());

  TextItem* gen_text = static_cast<TextItem*>(gen_item.get());
  ASSERT_EQ(item.GetContents(), gen_text->GetContents());
}

TEST_F(TexttItemTest, SetContentsGetContents) {
  TextItem item("text_id", "default");
  item.SetContents("test");

  ASSERT_EQ(item.GetContents(), "test");
}

TEST_F(TexttItemTest, GetHyperLink) {
  TextItem item("text_id", "default");

  ASSERT_EQ(item.GetHyperLink(), "");
}

}  // namespace