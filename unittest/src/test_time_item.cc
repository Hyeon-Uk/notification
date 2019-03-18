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

#include "notification-ex/time_item.h"
#include "notification-ex/item_inflator.h"

using namespace notification;
using namespace notification::item;

class TimeItemTest : public ::testing::Test {
 public:
  TimeItem* item;
  time_t current_time;
  std::string id = "time_id";
  virtual void SetUp() {
    time(&current_time);
    item = new TimeItem(id, current_time);
  }
  virtual void TearDown() {
    delete item;
  }
};

TEST_F(TimeItemTest, create) {
  EXPECT_NE(TimeItemTest::item, nullptr);
}

TEST_F(TimeItemTest, FindByID) {
  AbstractItem& child = TimeItemTest::item->FindByID(TimeItemTest::id);
  TimeItem& time_ = static_cast<TimeItem&>(child);
  ASSERT_EQ(time_.GetTime(), TimeItemTest::current_time);
}

TEST_F(TimeItemTest, FindByIDNullItemReturn) {
  AbstractItem& child = TimeItemTest::item->FindByID("not_existed_item");
  ASSERT_EQ(child.GetType(), TimeItem::NullObject);
}

TEST_F(TimeItemTest, SerializeDeserialize) {
  Bundle b = TimeItemTest::item->Serialize();

  std::shared_ptr<AbstractItem> gen_item = ItemInflator::Create(b);
  ASSERT_EQ(TimeItemTest::item->GetType(), gen_item->GetType());

  auto gen_time = std::static_pointer_cast<TimeItem>(gen_item);
  ASSERT_EQ(item->GetTime(), gen_time->GetTime());
}
