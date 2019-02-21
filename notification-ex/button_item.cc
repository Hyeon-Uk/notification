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

#include "notification-ex/button_item.h"
#include "notification-ex/button_item_implementation.h"
#include "notification-ex/item_factory.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "NOTIFICATION_EX"
#define BUTTON_TITLE_KEY "__BUTTON_TITLE_KEY__"

using namespace std;
namespace notification {
namespace item {

ButtonItem::ButtonItem(string title,
      std::shared_ptr<AbstractAction> action)
  : AbstractItem(AbstractItem::Button, action), impl_(new Impl(this, title)) {
}

ButtonItem::ButtonItem(string id, string title,
      std::shared_ptr<AbstractAction> action)
  : AbstractItem(id, AbstractItem::Button, action), impl_(new Impl(this, title)) {
}

ButtonItem::~ButtonItem() = default;
ButtonItem::Impl::~Impl() = default;

ButtonItem::Impl::Impl(ButtonItem* parent, string title)
  : title_(title), parent_(parent) {
  LOGI("ButtonItem impl created");
}

Bundle ButtonItem::Serialize() {
  Bundle b;
  b = AbstractItem::Serialize();
  b.Add(BUTTON_TITLE_KEY, impl_->title_);
  return b;
}

void ButtonItem::Deserialize(Bundle b) {
  AbstractItem::Deserialize(b);
  impl_->title_ = b.GetString(BUTTON_TITLE_KEY);
}

AbstractItem& ButtonItem::FindByID(std::string id) {
  if (GetId() == id)
    return *this;
  return ItemFactory::GetNullItem();
}

std::string ButtonItem::GetTitle() const {
  return impl_->title_;
}

}  // namespace item
}  // namespace notification
