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

#include <dlog.h>

#include <memory>
#include <vector>
#include <algorithm>

#include "notification-ex/input_selector_item.h"
#include "notification-ex/input_selector_item_implementation.h"
#include "notification-ex/item_factory.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "NOTIFICATION_EX"
#define INPUT_SELECTOR_CONTENTS_KEY "__INPUT_SELECTOR_CONTENTS_KEY__"

using namespace std;
namespace notification {
namespace item {

InputSelectorItem::InputSelectorItem(std::shared_ptr<AbstractAction> action)
  : AbstractItem(AbstractItem::InputSelector, action), impl_(new Impl(this)) {
}

InputSelectorItem::InputSelectorItem(string id, std::shared_ptr<AbstractAction> action)
  : AbstractItem(id, AbstractItem::InputSelector, action), impl_(new Impl(this)) {
}

InputSelectorItem::~InputSelectorItem() = default;
InputSelectorItem::Impl::~Impl() = default;

InputSelectorItem::Impl::Impl(InputSelectorItem* parent)
  : parent_(parent) {
  LOGI("InputSelectorItem impl created");
}

Bundle InputSelectorItem::Serialize() {
  Bundle b;
  b = AbstractItem::Serialize();
  vector<string> contents_vector {
    begin(impl_->contents_),
    end(impl_->contents_)
  };
  b.Add(INPUT_SELECTOR_CONTENTS_KEY, contents_vector);
  return b;
}

void InputSelectorItem::Deserialize(Bundle b) {
  AbstractItem::Deserialize(b);
  vector<string> contents = b.GetStringArray(INPUT_SELECTOR_CONTENTS_KEY);
  impl_->contents_ = list<string>(contents.begin(), contents.end());
}

AbstractItem& InputSelectorItem::FindByID(std::string id) {
  if (GetId() == id)
    return *this;
  return ItemFactory::GetNullItem();
}

list<string> InputSelectorItem::GetContents() const {
  return impl_->contents_;
}

void InputSelectorItem::SetContents(list<string> contents) {
  impl_->contents_ = move(contents);
}

}  // namespace item
}  // namespace notification
