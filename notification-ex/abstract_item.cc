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
#include <uuid/uuid.h>

#include <memory>
#include <algorithm>

#include "notification-ex/exception.h"
#include "notification-ex/abstract_item.h"
#include "notification-ex/abstract_item_implementation.h"
#include "notification-ex/item_info_internal.h"
#include "notification-ex/ex_util.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "NOTIFICATION_EX"

#define ABSTRACT_ITEM_TYPE_KEY "__ABSTRACT_ITEM_TYPE_KEY__"
#define ABSTRACT_ITEM_ID_KEY "__ABSTRACT_ITEM_ID_KEY__"
#define ABSTRACT_ITEM_SENDER_APPID_KEY "__ABSTRACT_ITEM_SENDER_APPID_KEY__"

using namespace std;
namespace notification {
namespace item {

string AbstractItem::Impl::GenerateItemId() {
  char uuid[37];
  uuid_t u;
  uuid_generate(u);
  uuid_unparse(u, uuid);
  string id = util::GetAppId();
  return string(uuid) + ":" + id;
}

AbstractItem::AbstractItem(
    std::shared_ptr<AbstractAction> action)
  : impl_(new Impl(this, action)) {
}

AbstractItem::AbstractItem(std::string id,
    std::shared_ptr<AbstractAction> action)
  : impl_(new Impl(this, id, action)) {
}

AbstractItem::Impl::Impl(AbstractItem* parent, string id,
    std::shared_ptr<AbstractAction> action)
  : id_(id), action_(action), parent_(parent),
    info_(std::make_shared<ItemInfo>(this)) {

  if (id_.empty())
    id_ = GenerateItemId();
  sender_appid_ = util::GetAppId();
  LOGI("GroupItem created (%s)", id_.c_str());
}

AbstractItem::Impl::Impl(AbstractItem* parent,
    std::shared_ptr<AbstractAction> action)
  : action_(action), parent_(parent),
    info_(std::make_shared<ItemInfo>(this)) {
  LOGI("GroupItem created");

  id_ = GenerateItemId();
  sender_appid_ = util::GetAppId();
  LOGI("GroupItem created (%s)", id_.c_str());
}

AbstractItem::~AbstractItem() = default;

Bundle AbstractItem::Serialize() const {
  Bundle b;
  b.Add(ABSTRACT_ITEM_TYPE_KEY, to_string(GetType()));
  b.Add(ABSTRACT_ITEM_ID_KEY, impl_->id_);
  b.Add(ABSTRACT_ITEM_SENDER_APPID_KEY, GetSenderAppId().c_str());
  return b;
}

void AbstractItem::Deserialize(Bundle b) {
  string type_str = b.GetString(ABSTRACT_ITEM_TYPE_KEY);
  if (type_str.empty())
    THROW(NOTIFICATION_ERROR_IO_ERROR);

  string id_str = b.GetString(ABSTRACT_ITEM_ID_KEY);
  impl_->id_ = id_str;
  impl_->sender_appid_ = b.GetString(ABSTRACT_ITEM_SENDER_APPID_KEY);
}

string AbstractItem::GetId() const {
 return impl_->id_;
}

shared_ptr<AbstractAction> AbstractItem::GetAction() const {
  return impl_->action_;
}

shared_ptr<Style> AbstractItem::GetStyle() const {
  return impl_->style_;
}

void AbstractItem::SetStyle(shared_ptr<Style> style) {
  impl_->style_ = style;
}

void AbstractItem::SetLEDInfo(std::shared_ptr<LEDInfo> led) {
  impl_->led_ = led;
}

std::shared_ptr<LEDInfo> AbstractItem::GetLEDInfo() const {
  return impl_->led_;
}

int AbstractItem::GetType(Bundle b) {
  string type_str = b.GetString(ABSTRACT_ITEM_TYPE_KEY);
  if (type_str.empty())
    THROW(NOTIFICATION_ERROR_IO_ERROR);
  return strtol(type_str.c_str(), NULL, 10);
}

void AbstractItem::SetVisible(bool visible) {
  impl_->visible_ = visible;
}

void AbstractItem::SetEnable(bool enable) {
  impl_->enable_ = enable;
}

void AbstractItem::AddReceiver(std::string receiver_group) {
  impl_->receiver_group_list_.push_back(receiver_group);
}

void AbstractItem::RemoveReceiver(std::string receiver_group) {
  impl_->receiver_group_list_.remove(receiver_group);
}

list<string> AbstractItem::GetReceiverList() {
  return impl_->receiver_group_list_;
}

bool AbstractItem::CanReceive(std::string id) const {
  if (impl_->receiver_group_list_.size() == 0)
    return true;
  list<string>::iterator iter =
    std::find(impl_->receiver_group_list_.begin(),
      impl_->receiver_group_list_.end(), id);
  return !(iter == impl_->receiver_group_list_.end());
}

void AbstractItem::SetPolicy(AbstractItem::Policy policy) {
  impl_->policy_ = policy;
}

AbstractItem::Policy AbstractItem::GetPolicy() const {
  return impl_->policy_;
}

std::shared_ptr<IItemInfo> AbstractItem::GetInfo() const {
  return impl_->info_;
}

string AbstractItem::GetChannel() const {
  return impl_->channel_;
}

void AbstractItem::SetChannel(string channel) {
  impl_->channel_ = channel;
}

void AbstractItem::SetSoundPath(std::string path) {
  impl_->sound_path_ = path;
}

void AbstractItem::SetVibrationPath(std::string path) {
  impl_->vibration_path_ = path;
}

std::string AbstractItem::GetSoundPath() const {
  return impl_->sound_path_;
}

std::string AbstractItem::GetVibrationPath() const {
  return impl_->vibration_path_;
}

std::string AbstractItem::GetSenderAppId() const {
  return impl_->sender_appid_;
}

void AbstractItem::SetSenderAppId(std::string sender_appid) {
  impl_->sender_appid_ = sender_appid;
}

time_t AbstractItem::GetTime() {
  return impl_->time_;
}

int AbstractItem::GetUid() {
  return impl_->uid_;
}

void AbstractItem::SetUid(int uid) {
  impl_->uid_ = uid;
}

int AbstractItem::GetRequestId() {
  return impl_->request_id_;
}

}  // namespace item
}  // namespace notification
