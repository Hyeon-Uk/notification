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

#include <gmock/gmock.h>

#include "mock/app_common_mock.hh"
#include "mock/glib_mock.hh"
#include "mock/test_fixture.hh"
#include "notification-ex/progress_item.h"
#include "notification-ex/item_inflator.h"


using namespace tizen_base;
using namespace notification;
using namespace notification::item;
using namespace std;

namespace {
class Mocks :
    virtual public ::testing::NiceMock<AppCommonMock>,
    virtual public ::testing::NiceMock<GlibMock> {};
} // namespace

class ProgressItemTest : public TestFixture {
 public:
  ProgressItemTest() : TestFixture(std::make_unique<::Mocks>()) {}
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
