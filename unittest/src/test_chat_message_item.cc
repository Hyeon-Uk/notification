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

#include "notification-ex/chat_message_item.h"
#include "notification-ex/item_inflator.h"

using namespace notification;
using namespace notification::item;

class ChatMessageItemTest : public ::testing::Test {
 public:
  ChatMessageItem* item;
  ChatMessageItem::Type type = ChatMessageItem::Type::user;
  std::string id = "chatmessage_id";
  time_t current_time;
  virtual void SetUp() {
    time(&current_time);

    item = new ChatMessageItem(id,
      std::make_shared<TextItem>("name_id", "name"),
      std::make_shared<TextItem>("text_id", "text"),
      std::make_shared<TextItem>("data_id", "data"),
      std::make_shared<TimeItem>(current_time),type);
  }
  virtual void TearDown() {
    delete item;
  }
};

TEST_F(ChatMessageItemTest, create) {
  EXPECT_NE(ChatMessageItemTest::item, nullptr);
}

TEST_F(ChatMessageItemTest, FindByID) {
  AbstractItem& child = ChatMessageItemTest::item->FindByID(ChatMessageItemTest::id);
  ChatMessageItem& message = static_cast<ChatMessageItem&>(child);
  ASSERT_EQ(message.GetNameItem().GetContents(), "name");
  ASSERT_EQ(message.GetTextItem().GetContents(), "text");
  ASSERT_EQ(message.GetDataItem().GetContents(), "data");
  ASSERT_EQ(message.GetMessageType(), ChatMessageItemTest::type);
}

TEST_F(ChatMessageItemTest, FindByIDNullItemReturn) {
  AbstractItem& child = ChatMessageItemTest::item->FindByID("not_existed_item");
  ASSERT_EQ(child.GetType(), ChatMessageItem::NullObject);
}

TEST_F(ChatMessageItemTest, SerializeDeserialize) {
  Bundle b = ChatMessageItemTest::item->Serialize();

  std::shared_ptr<AbstractItem> gen_item = ItemInflator::Create(b);
  auto gen_message = std::static_pointer_cast<ChatMessageItem>(gen_item);
  ASSERT_EQ(gen_message->GetNameItem().GetContents(), "name");
  ASSERT_EQ(gen_message->GetTextItem().GetContents(), "text");
  ASSERT_EQ(gen_message->GetDataItem().GetContents(), "data");
  ASSERT_EQ(gen_message->GetTimeItem().GetTime(), ChatMessageItemTest::current_time);
  ASSERT_EQ(gen_message->GetMessageType(), ChatMessageItemTest::type);
}
