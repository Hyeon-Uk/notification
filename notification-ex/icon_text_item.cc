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

#include "notification-ex/icon_text_item.h"
#include "notification-ex/icon_text_item_implementation.h"
#include "notification-ex/factory_manager.h"
#include "notification-ex/exception.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "NOTIFICATION_EX"
#define ICONTEXT_PATH_KEY "__ICONTEXT_PATH_KEY__"
#define ICONTEXT_TITLE_KEY "__ICONTEXT_TITLE_KEY__"

namespace notification {
namespace item {

IconTextItem::IconTextItem(std::string id, std::shared_ptr<IconItem> icon,
    std::shared_ptr<TextItem> text, std::shared_ptr<AbstractAction> action)
    : AbstractItem(id, action), impl_(new Impl(this, icon, text)) {
}

IconTextItem::Impl::Impl(IconTextItem* parent, std::shared_ptr<IconItem> icon,
    std::shared_ptr<TextItem> text)
    : parent_(parent), icon_(icon), text_(text) {
  LOGI("IconTextItem impl created");
}

int IconTextItem::GetType() const {
  return AbstractItem::IconText;
}

Bundle IconTextItem::Serialize() const {
  Bundle b;
  b = AbstractItem::Serialize();

  b.Add(ICONTEXT_PATH_KEY,
    reinterpret_cast<char*>(impl_->icon_->Serialize().ToRaw().first.get()));
  b.Add(ICONTEXT_TITLE_KEY,
    reinterpret_cast<char*>(impl_->text_->Serialize().ToRaw().first.get()));
  return b;
}

void IconTextItem::Deserialize(Bundle b) {
  AbstractItem::Deserialize(b);

  std::shared_ptr<AbstractItem> icon =
      FactoryManager::GetInst().CreateItem(AbstractItem::Icon);
  icon.get()->Deserialize(Bundle(b.GetString(ICONTEXT_PATH_KEY)));
  impl_->icon_ = std::static_pointer_cast<IconItem>(icon);

  std::shared_ptr<AbstractItem> text =
    FactoryManager::GetInst().CreateItem(AbstractItem::Text);
  text.get()->Deserialize(Bundle(b.GetString(ICONTEXT_TITLE_KEY)));
  impl_->text_ = std::static_pointer_cast<TextItem>(text);
}

AbstractItem& IconTextItem::FindByID(std::string id) {
  if (GetId() == id)
    return *this;

  return FactoryManager::GetInst().GetNullItem();
}

bool IconTextItem::IsItemTypeExist(int type) {
  if (GetType() == type)
    return true;
  return false;
}

IconItem& IconTextItem::GetIconItem() const {
  return *(impl_->icon_);
}

TextItem& IconTextItem::GetTextItem() const {
  return *(impl_->text_);
}

IconTextItem::~IconTextItem() = default;
IconTextItem::Impl::~Impl() = default;

}  // namespace item
}  // namespace notification
