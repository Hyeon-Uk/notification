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

#include "notification-ex/item_inflator.h"
#include "notification-ex/manager.h"
#include "notification-ex/manager_implementation.h"
#include "notification-ex/event_info.h"
#include "notification-ex/dbus_connection_manager.h"
#include "notification-ex/ex_util.h"
#include "notification-ex/item_info_internal.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "NOTIFICATION_EX"

#define MAX_PACKAGE_STR_SIZE 512
#define NOTIFICATION_EX_MANAGER_OBJECT_PATH "/org/tizen/notification_ex_manager"

using namespace std;
using namespace notification::item;
namespace notification {

Manager::Manager(unique_ptr<IEventSender> sender,
    unique_ptr<IEventListener> listener, string receiver_group)
  : impl_(new Impl(this, move(sender), move(listener), receiver_group)) {
}
Manager::~Manager() = default;

Manager::Impl::~Impl() {
  listener_->UnRegisterObserver(parent_);
}
Manager::Impl::Impl(Manager* parent,
    unique_ptr<IEventSender> sender,
    unique_ptr<IEventListener> listener, string receiver_group)
  : sender_(move(sender)), listener_(move(listener)),
    receiver_group_(receiver_group),
    parent_(parent) {
  LOGI("impl created");
  listener_->RegisterObserver(parent_);
}

void Manager::Impl::SendNotify(shared_ptr<item::AbstractItem> noti,
    EventInfo::EventType type) {
  Bundle serialized = noti->Serialize();
  EventInfo info(type, util::GetAppId(), noti->GetChannel());
  list<Bundle> serialized_list {serialized};

  /* Reply to Sender */
  sender_->Notify(info, serialized_list, noti->GetSenderAppId());
}

void Manager::Update(shared_ptr<item::AbstractItem> noti) {
  impl_->SendNotify(noti, EventInfo::Update);
}

void Manager::Remove(shared_ptr<item::AbstractItem> noti) {
  impl_->SendNotify(noti, EventInfo::Delete);
}

void Manager::Hide(shared_ptr<item::AbstractItem> noti) {
  ((IItemInfoInternal*)noti->GetInfo().get())->AddHideViewer(util::GetAppId());
  impl_->SendNotify(noti, EventInfo::Update);
}

shared_ptr<item::AbstractItem> Manager::FindByRootID(string id) {
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

list<shared_ptr<item::AbstractItem>> Manager::Get() {
  EventInfo info(EventInfo::Get, util::GetAppId(), "");
  list<Bundle> result = impl_->sender_->Request(info);
  list<shared_ptr<item::AbstractItem>> gen_list;
  for (auto& i : result) {
    shared_ptr<AbstractItem> gen_item = ItemInflator::Create(i);
    gen_list.emplace_back(gen_item);
  }
  return gen_list;
}

void Manager::SendEvent(const EventInfo& info,
    shared_ptr<item::AbstractItem> noti) {
  Bundle serialized = noti->Serialize();
  Bundle serialized_info = info.Serialize();
  list<Bundle> serialized_list {serialized};
  impl_->sender_->Notify(info, serialized_list, noti->GetSenderAppId());
}

list<Bundle> Manager::OnRequest(const EventInfo& info) {
  list<shared_ptr<item::AbstractItem>> item_list = OnRequestEvent(info);
  list<Bundle> serialized_list;
  for (auto& i : item_list) {
    serialized_list.push_back(i->Serialize());
  }
  return serialized_list;
}

void Manager::OnEvent(const EventInfo& info, list<Bundle> serialized) {
  shared_ptr<AbstractItem> gen_item;
  const EventInfo::EventType type = info.GetEventType();
  switch(type) {
    case EventInfo::Post: {
      list<shared_ptr<item::AbstractItem>> added;
      for (auto& i : serialized) {
        gen_item = ItemInflator::Create(i);
        if (gen_item->CanReceive(impl_->receiver_group_))
          added.emplace_back(gen_item);
      }
      if (added.size() > 0)
        OnAdd(info, added);
      break;
    }
    case EventInfo::Update: {
      for (auto& i : serialized) {
        gen_item = ItemInflator::Create(i);
        if (gen_item->CanReceive(impl_->receiver_group_))
          OnUpdate(info, gen_item);
      }
      break;
    }
    case EventInfo::Delete: {
      for (auto& i : serialized) {
        gen_item = ItemInflator::Create(i);
        if (gen_item->CanReceive(impl_->receiver_group_))
          OnDelete(info, gen_item);
      }
      break;
    }
    case EventInfo::Get:
      break;
  }
}

void Manager::OnAdd(const EventInfo& info,
      list<shared_ptr<item::AbstractItem>> addedItem) {
}

void Manager::OnUpdate(const EventInfo& info,
      shared_ptr<item::AbstractItem> updatedItem) {
}

void Manager::OnDelete(const EventInfo& info,
      shared_ptr<item::AbstractItem> deletedItem) {
}

list<shared_ptr<item::AbstractItem>> Manager::OnRequestEvent(const EventInfo& info) {
  return list<shared_ptr<item::AbstractItem>>({});
}

string Manager::GetPath() {
  return NOTIFICATION_EX_MANAGER_OBJECT_PATH;
}

}  // nampace notification
