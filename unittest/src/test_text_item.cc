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

using namespace notification;
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
  TextItem item("text_id", "contents", "hyperlink");

  AbstractItem& child = item.FindByID("text_id");
  TextItem& btn = static_cast<TextItem&>(child);
  ASSERT_EQ(btn.GetContents(), "contents");
}

TEST_F(TexttItemTest, FindByIDNullItemReturn) {
  TextItem item("text_id", "contents", "hyperlink");

  AbstractItem& child = item.FindByID("not_existed_item");
  ASSERT_EQ(child.GetType(), TextItem::NullObject);
}

TEST_F(TexttItemTest, SerializeDeserializeGetContents) {
  TextItem item("text_id", "contents");
  Bundle b = item.Serialize();
  std::shared_ptr<AbstractItem> gen_item = ItemInflator::Create(b);
  ASSERT_EQ(gen_item->GetType(), item.GetType());

  auto gen_text = std::static_pointer_cast<TextItem>(gen_item);
  ASSERT_EQ(item.GetContents(), gen_text->GetContents());
}

TEST_F(TexttItemTest, SetContentsGetContents) {
  TextItem item("text_id", "contents");
  ASSERT_EQ(item.GetContents(), "contents");

  item.SetContents("changed");
  ASSERT_EQ(item.GetContents(), "changed");
}

TEST_F(TexttItemTest, GetHyperLink) {
  TextItem item("text_id", "contents");
  ASSERT_TRUE(item.GetHyperLink().empty());

  TextItem item2("text_id", "contents", "hyperlink2");
  ASSERT_EQ(item2.GetHyperLink(), "hyperlink2");
}

}  // namespace