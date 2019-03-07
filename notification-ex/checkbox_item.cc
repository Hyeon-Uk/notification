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

#include "notification-ex/checkbox_item.h"
#include "notification-ex/checkbox_item_implementation.h"
#include "notification-ex/factory_manager.h"
#include "notification-ex/exception.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "NOTIFICATION_EX"
#define CHECKBOX_TITLE_KEY "__CHECKBOX_TITLE_KEY__"
#define CHECKBOX_CHECKED_KEY "__CHECKBOX_CHECKED_KEY__"

namespace notification {
namespace item {

CheckBoxItem::CheckBoxItem(std::string id, std::string title, bool checked,
  std::shared_ptr<AbstractAction> action)
  : AbstractItem(id, action), impl_(new Impl(this, title, checked)) {
}

CheckBoxItem::Impl::Impl(CheckBoxItem* parent, std::string title, bool checked)
  : parent_(parent), title_(title), checked_(checked) {
  LOGI("CheckBoxItem impl created");
}

int CheckBoxItem::GetType() const {
  return AbstractItem::CheckBox;
}

Bundle CheckBoxItem::Serialize() const {
  Bundle b;

  b = AbstractItem::Serialize();
  b.Add(CHECKBOX_TITLE_KEY, impl_->title_);
  b.Add(CHECKBOX_CHECKED_KEY, std::to_string(impl_->checked_));

  return b;
}

void CheckBoxItem::Deserialize(Bundle b) {
  AbstractItem::Deserialize(b);

  impl_->title_ = b.GetString(CHECKBOX_TITLE_KEY);
  impl_->checked_ = std::stoi(b.GetString(CHECKBOX_CHECKED_KEY));
}

AbstractItem& CheckBoxItem::FindByID(std::string id) {
  if (GetId() == id)
    return *this;

  return FactoryManager::GetInst().GetNullItem();
}

std::string CheckBoxItem::GetTitle(void) const {
  return impl_->title_;
}

bool CheckBoxItem::IsChecked(void) const {
  return impl_->checked_;
}

CheckBoxItem::~CheckBoxItem() = default;
CheckBoxItem::Impl::~Impl() = default;

}  // namespace item
}  // namespace notification_ex
