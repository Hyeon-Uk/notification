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

#include "notification-ex/event_info.h"
#include "notification-ex/event_info_implementation.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "NOTIFICATION_EX"
#define NOTIFICATION_EX_EVENT_TYPE_KEY "__NOTIFICATION_EX_EVENT_TYPE_KEY__"
#define NOTIFICATION_EX_EVENT_OWNER_KEY "__NOTIFICATION_EX_EVENT_OWNER_KEY__"
#define NOTIFICATION_EX_EVENT_CHANNEL_KEY "__NOTIFICATION_EX_EVENT_CHANNEL_KEY__"
#define NOTIFICATION_EX_EVENT_ITEM_ID_KEY "__NOTIFICATION_EX_EVENT_ITEM_ID_KEY__"
#define NOTIFICATION_EX_EVENT_TAG_KEY "__NOTIFICATION_EX_EVENT_TAG_KEY__"

using namespace std;
namespace notification {

EventInfo::EventInfo(EventInfo::EventType type, std::string owner,
    std::string channel, std::string item_id, std::string tag)
  : impl_(new Impl(this, type, owner, channel, item_id, tag)) {
}
EventInfo::~EventInfo() = default;
EventInfo::Impl::~Impl() = default;

EventInfo::Impl::Impl(EventInfo* parent, EventInfo::EventType type, std::string owner,
    std::string channel, std::string item_id, std::string tag)
  : type_(type), owner_(owner), channel_(channel), item_id_(item_id),
    tag_(tag), parent_(parent) {
  LOGI("EventInfo impl created");
}

EventInfo::EventInfo(Bundle serialized) {
  string event_str = serialized.GetString(NOTIFICATION_EX_EVENT_TYPE_KEY);
  impl_->type_ = static_cast<EventInfo::EventType>(strtol(event_str.c_str(), NULL, 10));
  impl_->owner_ = serialized.GetString(NOTIFICATION_EX_EVENT_OWNER_KEY);
  impl_->channel_ = serialized.GetString(NOTIFICATION_EX_EVENT_CHANNEL_KEY);
  impl_->item_id_ = serialized.GetString(NOTIFICATION_EX_EVENT_ITEM_ID_KEY);
  impl_->tag_ = serialized.GetString(NOTIFICATION_EX_EVENT_TAG_KEY);
}

string EventInfo::GetString(EventInfo::EventType type) {
  switch(type) {
  case Post:
    return "Post";
    break;
  case Update:
    return "Update";
    break;
  case Delete:
    return "Delete";
    break;
  case Get:
    return "Get";
    break;
  }
  return "";
}

Bundle EventInfo::Serialize() const {
  Bundle serialized;
  serialized.Add(NOTIFICATION_EX_EVENT_TYPE_KEY, to_string((int)impl_->type_));
  serialized.Add(NOTIFICATION_EX_EVENT_OWNER_KEY, impl_->owner_);
  serialized.Add(NOTIFICATION_EX_EVENT_CHANNEL_KEY, impl_->channel_);
  serialized.Add(NOTIFICATION_EX_EVENT_ITEM_ID_KEY, impl_->item_id_);
  serialized.Add(NOTIFICATION_EX_EVENT_TAG_KEY, impl_->tag_);

  return serialized;
}

EventInfo::EventType EventInfo::GetEventType() const {
  return impl_->type_;
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

string EventInfo::GetTag() const {
  return impl_->tag_;
}

}  // namespace notification
