// Copyright (c) 2019 Samsung Electronics Co., Ltd All Rights Reserved
// Use of this source code is governed by a apache 2.0 license that can be
// found in the LICENSE file.

#include <gmock/gmock.h>

#include "notification-ex/input_selector_item.h"
#include "notification-ex/item_inflator.h"

using namespace notification;
using namespace notification::item;
using namespace std;

namespace {

class InputSelectorItemTest : public ::testing::Test {
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
  ASSERT_EQ(gen_item.get()->GetType(), item.GetType());

  InputSelectorItem* gen_input = static_cast<InputSelectorItem*>(gen_item.get());

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

}  // namespace
