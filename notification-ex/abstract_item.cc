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

#include "notification-ex/exception.h"
#include "notification-ex/abstract_item.h"
#include "notification-ex/abstract_item_implementation.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "NOTIFICATION_EX"

#define ABSTRACT_ITEM_TYPE_KEY "__ABSTRACT_ITEM_TYPE_KEY__"
#define ABSTRACT_ITEM_ID_KEY "__ABSTRACT_ITEM_ID_KEY__"

using namespace std;
namespace notification {
namespace item {

AbstractItem::AbstractItem(AbstractItem::Type type,
    std::shared_ptr<AbstractAction> action)
  : impl_(new Impl(this, type, action)) {
}

AbstractItem::AbstractItem(std::string id, AbstractItem::Type type,
    std::shared_ptr<AbstractAction> action)
  : impl_(new Impl(this, id, type, action)) {
}

AbstractItem::Impl::Impl(AbstractItem* parent, string id,
    AbstractItem::Type type, std::shared_ptr<AbstractAction> action)
  : id_(id), type_(type), action_(action), parent_(parent) {
  LOGI("GroupItem created");
}

AbstractItem::Impl::Impl(AbstractItem* parent,
    AbstractItem::Type type, std::shared_ptr<AbstractAction> action)
  : type_(type), action_(action), parent_(parent) {
  LOGI("GroupItem created");
}

AbstractItem::~AbstractItem() = default;

Bundle AbstractItem::Serialize() {
  Bundle b;
  b.Add(ABSTRACT_ITEM_TYPE_KEY, to_string((int)impl_->type_));
  b.Add(ABSTRACT_ITEM_ID_KEY, impl_->id_);
  return b;
}

void AbstractItem::Deserialize(Bundle b) {
  string type_str = b.GetString(ABSTRACT_ITEM_TYPE_KEY);
  if (type_str.empty())
    THROW(NOTIFICATION_ERROR_IO_ERROR);

  string id_str = b.GetString(ABSTRACT_ITEM_ID_KEY);
  impl_->id_ = id_str;
  impl_->type_ = static_cast<AbstractItem::Type>(strtol(type_str.c_str(), NULL, 10));
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

AbstractItem::Type AbstractItem::GetType() const {
  return impl_->type_;
}

AbstractItem::Type AbstractItem::GetType(Bundle b) {
  string type_str = b.GetString(ABSTRACT_ITEM_TYPE_KEY);
  if (type_str.empty())
    THROW(NOTIFICATION_ERROR_IO_ERROR);
  return static_cast<AbstractItem::Type>(strtol(type_str.c_str(), NULL, 10));
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

bool AbstractItem::CanReceive(std::string id) const {
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

int AbstractItem::GetVersion() const {
  return impl_->version_;
}

void AbstractItem::SetVersion(int ver) {
  impl_->version_ = ver;
}

int AbstractItem::GetHideTime() const {
  return impl_->hide_time_;
}

int AbstractItem::GetDeleteTime() const {
  return impl_->delete_time_;
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

}  // namespace item
}  // namespace notification
