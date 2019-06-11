// Copyright (c) 2019 Samsung Electronics Co., Ltd All Rights Reserved
// Use of this source code is governed by a apache 2.0 license that can be
// found in the LICENSE file.

#include <gmock/gmock.h>

#include "notification-ex/progress_item.h"
#include "notification-ex/item_inflator.h"

using namespace notification;
using namespace notification::item;
using namespace std;

namespace {

class ProgressItemTest : public ::testing::Test {
 protected:
  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(ProgressItemTest, SerializeDeserializeGetTitle) {
  ProgressItem item(1.0, 10.0, 100.0);
  item.SetDefaultUnit("byte");
  Bundle b = item.Serialize();
  shared_ptr<AbstractItem> gen_item = ItemInflator::Create(b);

  auto gen_progress = std::static_pointer_cast<ProgressItem>(gen_item);
  ASSERT_EQ(item.GetCurrent(), gen_progress->GetCurrent());
  ASSERT_EQ(item.GetMin(), gen_progress->GetMin());
  ASSERT_EQ(item.GetMax(), gen_progress->GetMax());
  ASSERT_EQ(item.GetType(), gen_progress->GetType());
  ASSERT_EQ(item.GetDefaultUnit(), gen_progress->GetDefaultUnit());

  ASSERT_EQ(gen_progress->GetMin(), 1.0);
  ASSERT_EQ(gen_progress->GetCurrent(), 10.0);
  ASSERT_EQ(gen_progress->GetMax(), 100.0);
  ASSERT_EQ(gen_progress->GetProgressType(), ProgressItem::Type::Default);
  ASSERT_EQ(gen_progress->GetDefaultUnit(), "byte");
}

TEST_F(ProgressItemTest, SetGetProgressType) {
  ProgressItem item(1.0, 10.0, 100.0);

  ASSERT_EQ(item.GetProgressType(), ProgressItem::Type::Default);

  item.SetProgressType(ProgressItem::Type::Pending);
  ASSERT_EQ(item.GetProgressType(), ProgressItem::Type::Pending);
}

TEST_F(ProgressItemTest, SetGetUnit) {
  ProgressItem item(1.0, 10.0, 100.0);
  item.SetDefaultUnit("byte");

  ASSERT_EQ(item.GetDefaultUnit(), "byte");
}

}  // namespace
