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
#include <unistd.h>

#include <memory>

#include "notification-ex/event_info_internal.h"
#include "notification-ex/event_info_implementation.h"
#include "notification-ex/ex_util.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "NOTIFICATION_EX"
#define NOTIFICATION_EX_EVENT_TYPE_KEY "__NOTIFICATION_EX_EVENT_TYPE_KEY__"
#define NOTIFICATION_EX_EVENT_OWNER_KEY "__NOTIFICATION_EX_EVENT_OWNER_KEY__"
#define NOTIFICATION_EX_EVENT_CHANNEL_KEY "__NOTIFICATION_EX_EVENT_CHANNEL_KEY__"
#define NOTIFICATION_EX_EVENT_ITEM_ID_KEY "__NOTIFICATION_EX_EVENT_ITEM_ID_KEY__"
#define NOTIFICATION_EX_EVENT_UID_KEY "__NOTIFICATION_EX_EVENT_UID_KEY__"
#define NOTIFICATION_EX_EVENT_REQUEST_ID_KEY "__NOTIFICATION_EX_EVENT_REQUEST_ID_KEY__"
#define NOTIFICATION_EX_EVENT_ERROR_KEY "__NOTIFICATION_EX_EVENT_ERROR_KEY__"

using namespace std;
namespace notification {

EventInfo::EventInfo(int type, std::string owner, std::string channel,
    std::string item_id)
    : impl_(new Impl(this, type, owner, channel, item_id)) {
}
EventInfo::~EventInfo() = default;
EventInfo::Impl::~Impl() = default;

EventInfo::Impl::Impl(EventInfo* parent, int type, std::string owner,
    std::string channel, std::string item_id)
    : type_(type), owner_(owner), channel_(channel), item_id_(item_id),
    parent_(parent) {
  uid_ = getuid();
  request_id_ = util::GetRequestId();
  error_ = NOTIFICATION_ERROR_NONE;
  LOGI("EventInfo impl created");
}

EventInfo::EventInfo(Bundle serialized)
  : impl_(new Impl(this, EventInfo::Post, "", "", "")) {
  string event_str = serialized.GetString(NOTIFICATION_EX_EVENT_TYPE_KEY);
  impl_->type_ = (int)strtol(event_str.c_str(), NULL, 10);
  impl_->owner_ = serialized.GetString(NOTIFICATION_EX_EVENT_OWNER_KEY);
  impl_->channel_ = serialized.GetString(NOTIFICATION_EX_EVENT_CHANNEL_KEY);
  impl_->item_id_ = serialized.GetString(NOTIFICATION_EX_EVENT_ITEM_ID_KEY);
  string uid_str = serialized.GetString(NOTIFICATION_EX_EVENT_UID_KEY);
  impl_->uid_ = (uid_t)strtol(uid_str.c_str(), NULL, 10);
  string request_id_str =
      serialized.GetString(NOTIFICATION_EX_EVENT_REQUEST_ID_KEY);
  impl_->request_id_ = (int)strtol(request_id_str.c_str(), NULL, 10);
  string error_str =
      serialized.GetString(NOTIFICATION_EX_EVENT_ERROR_KEY);
  impl_->error_ = (NotificationError)strtol(error_str.c_str(), NULL, 10);
}

string EventInfo::GetString(int type) {
  switch(type) {
    case Post:
      return "Post";
    case Update:
      return "Update";
    case Delete:
      return "Delete";
    case Get:
      return "Get";
    case Error:
      return "Error";
    default:
      return "Custom" + std::to_string(Custom);
  }
}

Bundle EventInfo::Serialize() const {
  Bundle serialized;
  serialized.Add(NOTIFICATION_EX_EVENT_TYPE_KEY, to_string((int)impl_->type_));
  serialized.Add(NOTIFICATION_EX_EVENT_OWNER_KEY, impl_->owner_);
  serialized.Add(NOTIFICATION_EX_EVENT_CHANNEL_KEY, impl_->channel_);
  serialized.Add(NOTIFICATION_EX_EVENT_ITEM_ID_KEY, impl_->item_id_);
  serialized.Add(NOTIFICATION_EX_EVENT_UID_KEY, to_string((int)impl_->uid_));
  serialized.Add(
      NOTIFICATION_EX_EVENT_REQUEST_ID_KEY, to_string(impl_->request_id_));
  serialized.Add(
      NOTIFICATION_EX_EVENT_ERROR_KEY, to_string(impl_->error_));

  return serialized;
}

int EventInfo::GetEventType() const {
  return impl_->type_;
}

void EventInfo::SetEventType(int type) {
  impl_->type_ = type;
}

string EventInfo::GetOwner() const {
  return impl_->owner_;
}

string EventInfo::GetChannel() const {
  return impl_->channel_;
}

string EventInfo::GetItemId() const {
  return impl_->item_id_;
}

uid_t EventInfo::GetUid() const {
  return impl_->uid_;
}

void EventInfo::SetUid(uid_t uid) {
  impl_->uid_ = uid;
}

int EventInfo::GetRequestId() const {
  return impl_->request_id_;
}

NotificationError EventInfo::GetError() const {
  return impl_->error_;
}

void EventInfo::SetError(NotificationError error) {
  impl_->error_ = error;
}

}  // namespace notification
