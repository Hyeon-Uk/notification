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
#include <algorithm>

#include "notification-ex/item_info_internal.h"
#include "notification-ex/ex_util.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "NOTIFICATION_EX"

using namespace std;
namespace notification {
namespace item {

AbstractItem::Impl::ItemInfo::ItemInfo(AbstractItem::Impl* impl) : impl_(impl) {
}

uid_t AbstractItem::Impl::ItemInfo::GetUid() const {
  return impl_->uid_;
}

void AbstractItem::Impl::ItemInfo::SetUid(uid_t uid) {
  impl_->uid_ = uid;
}

time_t AbstractItem::Impl::ItemInfo::GetTime() const {
  return impl_->time_;
}

void AbstractItem::Impl::ItemInfo::SetTime(time_t time) {
  impl_->time_ = time;
}

int AbstractItem::Impl::ItemInfo::GetVersion() const {
  return impl_->version_;
}

void AbstractItem::Impl::ItemInfo::SetVersion(int ver) {
  impl_->version_ = ver;
}

void AbstractItem::Impl::ItemInfo::SetHideTime(int hide_time) {
  impl_->hide_time_ = hide_time;
}

int AbstractItem::Impl::ItemInfo::GetHideTime() const {
  return impl_->hide_time_;
}

void AbstractItem::Impl::ItemInfo::SetDeleteTime(int delete_time) {
  impl_->delete_time_ = delete_time;
}

int AbstractItem::Impl::ItemInfo::GetDeleteTime() const {
  return impl_->delete_time_;
}

void AbstractItem::Impl::ItemInfo::AddHideViewer(std::string appid) {
  if (find(impl_->hide_viewer_list_.begin(),
      impl_->hide_viewer_list_.end(), appid) !=
      impl_->hide_viewer_list_.end()) {
    return;
  }
  impl_->hide_viewer_list_.push_back(appid);
}

bool AbstractItem::Impl::ItemInfo::CanReceive(std::string receiver_group) const {
  if (impl_->receiver_group_list_.size() != 0 && !receiver_group.empty()) {
    list<string>::iterator iter =
      std::find(impl_->receiver_group_list_.begin(),
        impl_->receiver_group_list_.end(), receiver_group);

    if (iter == impl_->receiver_group_list_.end())
      return false;
  }

  if (impl_->hide_viewer_list_.size() == 0)
    return true;

  string app_id = util::GetAppId();
  list<string>::iterator iter =
    std::find(impl_->hide_viewer_list_.begin(),
    impl_->hide_viewer_list_.end(), app_id);

  if (iter != impl_->hide_viewer_list_.end())
    return false;

  return true;
}

std::list<std::string> AbstractItem::Impl::ItemInfo::GetHideViewerList() const {
  return impl_->hide_viewer_list_;
}

}  // namespace item
}  // namespace notification
