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

#include "notification-ex/item_info_internal.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "NOTIFICATION_EX"

using namespace std;
namespace notification {
namespace item {

AbstractItem::Impl::ItemInfo::ItemInfo(AbstractItem::Impl* impl) : impl_(impl) {}

int AbstractItem::Impl::ItemInfo::GetVersion() const {
  return impl_->version_;
}

void AbstractItem::Impl::ItemInfo::SetVersion(int ver) {
  impl_->version_ = ver;
}

int AbstractItem::Impl::ItemInfo::GetHideTime() const {
  return impl_->hide_time_;
}

int AbstractItem::Impl::ItemInfo::GetDeleteTime() const {
  return impl_->delete_time_;
}

}  // namespace item
}  // namespace notification
