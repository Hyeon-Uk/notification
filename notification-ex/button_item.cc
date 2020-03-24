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
#include "notification-ex/factory_manager.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "NOTIFICATION_EX"
#define BUTTON_TITLE_KEY "__BUTTON_TITLE_KEY__"
#define BUTTON_IMAGE_KEY "__BUTTON_IMAGE_KEY__"
#define BUTTON_CONTENTS_KEY "__BUTTON_CONTENTS_KEY__"

using namespace std;
using namespace tizen_base;

namespace notification {
namespace item {

ButtonItem::ButtonItem(string title, std::shared_ptr<AbstractAction> action)
    : AbstractItem(action), impl_(new Impl(this, title)) {
}

ButtonItem::ButtonItem(string id, string title,
    std::shared_ptr<AbstractAction> action)
    : AbstractItem(id, action), impl_(new Impl(this, title)) {
}

ButtonItem::~ButtonItem() = default;
ButtonItem::Impl::~Impl() = default;

ButtonItem::Impl::Impl(ButtonItem* parent, string title)
    : title_(title), parent_(parent) {
  LOGI("ButtonItem impl created");
}

int ButtonItem::GetType() const {
  return AbstractItem::Button;
}

Bundle ButtonItem::Serialize() const {
  Bundle b;
  b = AbstractItem::Serialize();
  b.Add(BUTTON_TITLE_KEY, impl_->title_);
  if (!impl_->img_path_.empty())
    b.Add(BUTTON_IMAGE_KEY, impl_->img_path_);
  if (!impl_->contents_.empty())
    b.Add(BUTTON_CONTENTS_KEY, impl_->contents_);
  return b;
}

void ButtonItem::Deserialize(Bundle b) {
  AbstractItem::Deserialize(b);
  impl_->title_ = b.GetString(BUTTON_TITLE_KEY);
  impl_->img_path_ = b.GetString(BUTTON_IMAGE_KEY);
  impl_->contents_ = b.GetString(BUTTON_CONTENTS_KEY);
}

bool ButtonItem::IsItemTypeExist(int type) {
  if (GetType() == type)
    return true;
  return false;
}

std::string ButtonItem::GetTitle() const {
  return impl_->title_;
}

void ButtonItem::SetImgPath(string path) {
  impl_->img_path_ = path;
}

std::string ButtonItem::GetImgPath() const {
  return impl_->img_path_;
}

void ButtonItem::SetContents(std::string contents) {
  impl_->contents_ = contents;
}

std::string ButtonItem::GetContents() const {
  return impl_->contents_;
}

}  // namespace item
}  // namespace notification
