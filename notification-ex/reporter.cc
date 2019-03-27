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
#include <glib.h>
#include <unistd.h>

#include <list>

#include "notification-ex/reporter.h"
#include "notification-ex/reporter_implementation.h"
#include "notification-ex/event_info_internal.h"
#include "notification-ex/item_inflator.h"
#include "notification-ex/dbus_connection_manager.h"
#include "notification-ex/ex_util.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "NOTIFICATION_EX"
#define MAX_PACKAGE_STR_SIZE 512
#define NOTIFICATION_EX_REPORTER_OBJECT_PATH "/org/tizen/notification_ex_reporter"

using namespace std;
using namespace notification::item;
namespace notification {

Reporter::Reporter(
    unique_ptr<IEventSender> sender, unique_ptr<IEventListener> listener)
  : impl_(new Impl(this, move(sender), move(listener))) {
}
Reporter::~Reporter() = default;

Reporter::Impl::~Impl() {
  listener_->UnRegisterObserver(parent_);
}
Reporter::Impl::Impl(Reporter* parent,
    unique_ptr<IEventSender> sender, unique_ptr<IEventListener> listener)
  : sender_(move(sender)), listener_(move(listener)), parent_(parent) {
  LOGI("impl created");
  listener_->RegisterObserver(parent_);
}

int Reporter::Impl::SendNotify(shared_ptr<item::AbstractItem> noti,
    IEventInfo::EventType type) {
  Bundle serialized = noti->Serialize();
  EventInfo info(type, util::GetAppId(), noti->GetChannel());
  list<Bundle> serialized_list {serialized};
  sender_->Notify(info, serialized_list);
  return info.GetRequestId();
}

void Reporter::SendError(const IEventInfo& info, NotificationError error) {
  list<Bundle> serialized_list {};
  IEventInfo& i = const_cast<IEventInfo&>(info);
  static_cast<IEventInfoInternal&>(i).SetError(error);
  static_cast<IEventInfoInternal&>(i).SetEventType(EventInfo::Error);
  impl_->sender_->Notify(info, serialized_list, info.GetOwner());
}

int Reporter::Post(std::shared_ptr<item::AbstractItem> noti) {
  LOGI("Post noti");
  return impl_->SendNotify(noti, EventInfo::Post);
}

int Reporter::Post(std::list<std::shared_ptr<AbstractItem>> notiList) {
  EventInfo info(EventInfo::Post, util::GetAppId(), "");
  list<Bundle> serialized_list;
  for (auto& i : notiList) {
    Bundle b = i->Serialize();
    serialized_list.push_back(b);
  }
  impl_->sender_->Notify(info, serialized_list);
  return info.GetRequestId();
}

int Reporter::Update(std::shared_ptr<AbstractItem> noti) {
  return impl_->SendNotify(noti, EventInfo::Update);
}

int Reporter::Delete(std::shared_ptr<AbstractItem> noti) {
  return impl_->SendNotify(noti, EventInfo::Delete);
}

int Reporter::DeleteAll() {
  //TODO
  return -1;
}

std::shared_ptr<AbstractItem> Reporter::FindByRootID(std::string id) {
  Bundle serialized;
  EventInfo info(EventInfo::Get, util::GetAppId(), "", id);
  list<Bundle> result = impl_->sender_->Request(info);
  if (result.size() == 0) {
    LOGE("Fail to get noti");
    return shared_ptr<item::AbstractItem>({});
  }
  Bundle b = result.front();
  shared_ptr<AbstractItem> gen_item = ItemInflator::Create(b);
  return gen_item;
}

int Reporter::SendEvent(const IEventInfo& info,
    shared_ptr<item::AbstractItem> noti) {
  Bundle serialized = noti->Serialize();
  list<Bundle> serialized_list {serialized};
  impl_->sender_->Notify(info, serialized_list);
  return info.GetRequestId();
}

void Reporter::OnEvent(const IEventInfo& info, list<Bundle> serialized) {
  NotificationError error =
      (static_cast<const IEventInfoInternal&>(info)).GetError();
  if (info.GetEventType() == EventInfo::Error) {
    OnError(error, info.GetRequestId());
    return;
  }

  list<shared_ptr<item::AbstractItem>> item_list;
  for (auto& i : serialized) {
    shared_ptr<AbstractItem> gen_item = ItemInflator::Create(i);
    item_list.emplace_back(gen_item);
  }
  OnEvent(info, item_list);
}

list<shared_ptr<item::AbstractItem>> Reporter::OnRequestEvent(const IEventInfo& info) {
  return list<shared_ptr<item::AbstractItem>>({});
}

list<Bundle> Reporter::OnRequest(const IEventInfo& info) {
  list<shared_ptr<item::AbstractItem>> item_list = OnRequestEvent(info);
  list<Bundle> serialized_list;
  for (auto& i : item_list) {
    serialized_list.push_back(i->Serialize());
  }
  return serialized_list;
}

void Reporter::OnEvent(
    const IEventInfo& info, list<shared_ptr<item::AbstractItem>> notiList) {
}

void Reporter::OnError(NotificationError error, int requestId) {
}

string Reporter::GetPath() {
  return NOTIFICATION_EX_REPORTER_OBJECT_PATH;
}

}  // namespace notification
