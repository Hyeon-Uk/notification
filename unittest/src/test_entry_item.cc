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

#include "notification-ex/entry_item.h"
#include "notification-ex/item_inflator.h"

using namespace notification::item;

namespace {

class EntryItemTest : public ::testing::Test {
 public:
  virtual void SetUp() {}
  virtual void TearDown() {}
};

TEST_F(EntryItemTest, FindByID) {
  EntryItem item("entry_id");

  AbstractItem& child = item.FindByID("entry_id");
  EntryItem& btn = static_cast<EntryItem&>(child);
  ASSERT_EQ(btn.GetId(), "entry_id");
}

TEST_F(EntryItemTest, FindByIDNullItemReturn) {
  EntryItem item("entry_id");

  AbstractItem& child = item.FindByID("not_existed_item");
  ASSERT_EQ(child.GetType(), EntryItem::NullObject);
}

TEST_F(EntryItemTest, SerializeDeserialize) {
  EntryItem item("entry_id");
  Bundle b = item.Serialize();
  std::shared_ptr<AbstractItem> gen_item = ItemInflator::Create(b);
  ASSERT_EQ(gen_item.get()->GetType(), item.GetType());

  EntryItem* gen_effect = static_cast<EntryItem*>(gen_item.get());
  ASSERT_EQ(item.GetId(), gen_effect->GetId());
}

TEST_F(EntryItemTest, SetTextGetText) {
  EntryItem item("entry_id");
  item.SetText("test");

  ASSERT_EQ(item.GetText(), "test");
}

TEST_F(EntryItemTest, GetTextLimit) {
  EntryItem item("entry_id");

  ASSERT_EQ(item.GetTextLimit(), 160);
}

}  // namespace