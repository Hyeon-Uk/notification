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

#include "mock/app_common_mock.h"
#include "mock/test_fixture.h"
#include "notification-ex/input_selector_item.h"
#include "notification-ex/item_inflator.h"

using namespace tizen_base;
using namespace notification;
using namespace notification::item;
using namespace std;

namespace {
class Mocks :
    virtual public ::testing::NiceMock<AppCommonMock> {};
} // namespace

class InputSelectorItemTest : public TestFixture {
 public:
  InputSelectorItemTest() : TestFixture(std::make_unique<::Mocks>()) {}
 protected:
  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(InputSelectorItemTest, SerializeDeserialize) {
  InputSelectorItem item;
  list<string> contents;
  contents.push_back("AA");
  contents.push_back("BB");
  contents.push_back("CC");
  item.SetContents(contents);

  Bundle b = item.Serialize();
  shared_ptr<AbstractItem> gen_item = ItemInflator::Create(b);
  ASSERT_EQ(gen_item->GetType(), item.GetType());

  auto gen_input = static_pointer_cast<InputSelectorItem>(gen_item);

  list<string> l1 = item.GetContents();
  list<string> l2 = gen_input->GetContents();
  list<string>::iterator it1 = l1.begin();
  list<string>::iterator it2 = l2.begin();
  while(it1 != l1.end() && it2 != l2.end()) {
    ASSERT_EQ((*it1), (*it2));
    it1++;
    it2++;
  }
}
