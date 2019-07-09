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

#include "notification-ex/text_item.h"
#include "notification-ex/text_item_implementation.h"
#include "notification-ex/factory_manager.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "NOTIFICATION_EX"
#define TEXT_CONTENTS_KEY "__TEXT_CONTENTS_KEY__"
#define TEXT_HYPERLINK_KEY "__TEXT_HYPERLINK_KEY__"

using namespace tizen_base;

namespace notification {
namespace item {
TextItem::TextItem(std::string id, std::string text, std::string hyperlink,
    std::shared_ptr<AbstractAction> action)
    : AbstractItem(id, action), impl_(new Impl(this, text, hyperlink)) {
}

TextItem::Impl::Impl(TextItem* parent, std::string text, std::string hyperlink)
    : parent_(parent), text_(text), hyperlink_(hyperlink) {
  LOGI("TextItem created");
}

TextItem::~TextItem() = default;
TextItem::Impl::~Impl() = default;

int TextItem::GetType() const {
  return AbstractItem::Text;
}

Bundle TextItem::Serialize() const {
  Bundle b;
  b = AbstractItem::Serialize();

  if (!impl_->text_.empty())
    b.Add(TEXT_CONTENTS_KEY, impl_->text_);

  if (!impl_->hyperlink_.empty())
    b.Add(TEXT_HYPERLINK_KEY, impl_->hyperlink_);

  return b;
}

void TextItem::Deserialize(Bundle b) {
  AbstractItem::Deserialize(b);
  impl_->text_ = b.GetString(TEXT_CONTENTS_KEY);
  impl_->hyperlink_ = b.GetString(TEXT_HYPERLINK_KEY);
}

AbstractItem& TextItem::FindByID(std::string id) {
  if (GetId() == id)
    return *this;
  return FactoryManager::GetInst().GetNullItem();
}

bool TextItem::IsItemTypeExist(int type) {
  if (GetType() == type)
    return true;
  return false;
}

void TextItem::SetContents(std::string contents) {
  impl_->text_ = contents;
}

std::string TextItem::GetContents() const {
  return impl_->text_;
}

std::string TextItem::GetHyperLink() const {
  return impl_->hyperlink_;
}

}  // namespace item
}  // namespace notification
