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

#include "notification-ex/entry_item.h"
#include "notification-ex/entry_item_implementation.h"
#include "notification-ex/factory_manager.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "NOTIFICATION_EX"
#define ENTRY_TEXT_KEY "__ENTRY_TEXT_KEY__"
#define ENTRY_LIMIT_KEY "__ENTRY_LIMIT_KEY__"

namespace notification {
namespace item {

EntryItem::EntryItem(std::shared_ptr<AbstractAction> action)
  : AbstractItem(AbstractItem::Type::Entry), impl_(new Impl(this)) {
}

EntryItem::EntryItem(std::string id, std::shared_ptr<AbstractAction> action)
  : AbstractItem(id, AbstractItem::Type::Entry), impl_(new Impl(this)) {
}

EntryItem::Impl::Impl(EntryItem* parent)
  : parent_(parent) {
  LOGI("EntryItem created");
}

EntryItem::~EntryItem() = default;
EntryItem::Impl::~Impl() = default;

Bundle EntryItem::Serialize() {
  Bundle b;
  b = AbstractItem::Serialize();

  if (!impl_->text_.empty())
    b.Add(ENTRY_TEXT_KEY, impl_->text_);

  b.Add(ENTRY_LIMIT_KEY, std::to_string(impl_->limit_));

  return b;
}

void EntryItem::Deserialize(Bundle b) {
  AbstractItem::Deserialize(b);
  impl_->text_ = b.GetString(ENTRY_TEXT_KEY);
  impl_->limit_ = std::stoi(b.GetString(ENTRY_LIMIT_KEY));
}

AbstractItem& EntryItem::FindByID(std::string id) {
  if (GetId() == id)
    return *this;
  return FactoryManager::GetInst().GetNullItem();
}

std::string EntryItem::GetText() const {
  return impl_->text_;
}

void EntryItem::SetText(std::string text) {
  impl_->text_ = text;
}

int EntryItem::GetTextLimit() const {
  return impl_->limit_;
}

}  // namespace item
}  // namespace notification
