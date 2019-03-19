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

#include "notification-ex/icon_item.h"
#include "notification-ex/icon_item_implementation.h"
#include "notification-ex/abstract_item.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "NOTIFICATION_EX"

namespace notification {
namespace item {

IconItem::IconItem(std::string iconPath, std::shared_ptr<AbstractAction> action)
    : ImageItem(iconPath, action) , impl_(new Impl(this, iconPath)) {
}

IconItem::IconItem(std::string id, std::string iconPath,
    std::shared_ptr<AbstractAction> action)
    : ImageItem(id, iconPath, action), impl_(new Impl(this, iconPath)) {
}

IconItem::Impl::Impl(IconItem* parent, std::string iconPath)
    : parent_(parent), iconPath_(iconPath) {
  LOGI("IconItem impl created");
}

IconItem::~IconItem() = default;
IconItem::Impl::~Impl() = default;

int IconItem::GetType() const {
  return AbstractItem::Icon;
}

}  // namespace item
}  // namespace notification_ex

