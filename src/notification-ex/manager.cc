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
#include "notification-ex/event_info_internal.h"
#include "notification-ex/dbus_connection_manager.h"
#include "notification-ex/ex_util.h"
#include "notification-ex/item_info_internal.h"
#include "notification-ex/null_item.h"
#include "notification-ex/exception.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "NOTIFICATION_EX"

#define MAX_PACKAGE_STR_SIZE 512
#define NOTIFICATION_EX_MANAGER_OBJECT_PATH "/org/tizen/notification_ex_manager"
#define DATA_PROVIDER_MASTER_ID "data-provider-master"

using namespace std;
using namespace tizen_base;
using namespace notification::item;
namespace notification {

/* LCOV_EXCL_START */
Manager::Manager(unique_ptr<IEventSender> sender,
    unique_ptr<IEventListener> listener, string receiver_group)
  : impl_(new Impl(this, std::move(sender), std::move(listener), receiver_group)) {
}
Manager::~Manager() = default;

Manager::Impl::~Impl() {
  /* N4-T1: only run the matching Unregister + UnRegisterObserver pair
   * when EnsureRegistered() actually fired. Otherwise a Manager that
   * was constructed but never used would emit a stray RPC + listener
   * unhook against a peer that has not seen the registration. */
  if (!registered_.load(std::memory_order_acquire))
    return;

  listener_->UnRegisterObserver(parent_);
  EventInfo info(EventInfo::Unregister, util::GetAppId(), "", "");
  sender_->RequestReturnValue(info);
}
Manager::Impl::Impl(Manager* parent,
    unique_ptr<IEventSender> sender,
    unique_ptr<IEventListener> listener, string receiver_group)
  : sender_(std::move(sender)), listener_(std::move(listener)),
    receiver_group_(receiver_group),
    parent_(parent) {
  LOGI("impl created (lazy register)");
  /* N4-T1: do NOT call listener_->RegisterObserver(parent_) nor the
   * EventInfo::Register synchronous RPC here. Defer to the first
   * user-facing entry point via EnsureRegistered(). */
}

void Manager::Impl::EnsureRegistered() {
  std::call_once(register_once_, [this]() {
    listener_->RegisterObserver(parent_);

    /* data-provider-master is the daemon side; it must NOT register itself
     * as a Manager peer. Skip the Register RPC for that appid (mirrors the
     * pre-N4-T1 ctor behaviour). The observer hookup above still runs so
     * the daemon can dispatch incoming events. */
    if (!DBusConnectionManager::GetInst().IsDataProviderMaster(
            util::GetAppId())) {
      EventInfo info(
          EventInfo::Register, util::GetAppId(), receiver_group_, "");
      sender_->RequestReturnValue(info);
    }

    registered_.store(true, std::memory_order_release);
  });
}

void Manager::SendError(const IEventInfo& info, NotificationError error) {
  impl_->EnsureRegistered();
  list<Bundle> serialized_list {};
  IEventInfo& i = const_cast<IEventInfo&>(info);
  static_cast<IEventInfoInternal&>(i).SetError(error);
  static_cast<IEventInfoInternal&>(i).SetEventType(EventInfo::Error);
  impl_->sender_->Notify(info, serialized_list, info.GetOwner());
}

int Manager::Update(shared_ptr<item::AbstractItem> noti) {
  impl_->EnsureRegistered();
  Bundle serialized = noti->Serialize();
  EventInfo info(
      EventInfo::Update, util::GetAppId(), noti->GetChannel(), noti->GetId());
  list<Bundle> serialized_list {serialized};
  impl_->sender_->SendMessage(
      info, serialized_list, noti->GetSenderAppId());
  return info.GetRequestId();
}

int Manager::Delete(shared_ptr<item::AbstractItem> noti) {
  impl_->EnsureRegistered();
  Bundle serialized = noti->Serialize();
  EventInfo info(
      EventInfo::Delete, util::GetAppId(), noti->GetChannel(), noti->GetId());
  list<Bundle> serialized_list {serialized};
  impl_->sender_->SendMessage(
      info, serialized_list, noti->GetSenderAppId());
  return info.GetRequestId();
}

int Manager::DeleteAll() {
  impl_->EnsureRegistered();
  EventInfo info(EventInfo::DeleteAll, util::GetAppId(), "");
  impl_->sender_->RequestReturnValue(info);
  return info.GetRequestId();
}

int Manager::DeleteByChannel(string channel) {
  impl_->EnsureRegistered();
  EventInfo info(EventInfo::DeleteAll, util::GetAppId(), channel);
  impl_->sender_->RequestReturnValue(info);
  return info.GetRequestId();
}

int Manager::DeleteByAppId(string appId) {
  impl_->EnsureRegistered();
  EventInfo info(EventInfo::DeleteAll, util::GetAppId(), "", appId);
  impl_->sender_->RequestReturnValue(info);
  return info.GetRequestId();
}

int Manager::GetCount() const {
  impl_->EnsureRegistered();
  EventInfo info(EventInfo::Count, util::GetAppId(), "");
  return impl_->sender_->RequestReturnValue(info);
}

int Manager::Hide(shared_ptr<item::AbstractItem> noti) {
  impl_->EnsureRegistered();
  (reinterpret_cast<IItemInfoInternal*>(noti->GetInfo().get()))
      ->AddHideViewer(util::GetAppId());

  EventInfo info(
      EventInfo::Update, util::GetAppId(), noti->GetChannel(), noti->GetId());
  Bundle serialized = noti->Serialize();
  list<Bundle> serialized_list {serialized};
  impl_->sender_->SendMessage(
      info, serialized_list, noti->GetSenderAppId());
  return info.GetRequestId();
}

unique_ptr<item::AbstractItem> Manager::FindByRootID(string id) {
  impl_->EnsureRegistered();
  EventInfo info(EventInfo::Get, util::GetAppId(), "", id);
  list<Bundle> result = impl_->sender_->Request(info);
  Bundle b = result.front();
  unique_ptr<AbstractItem> gen_item = ItemInflator::Create(b);
  return gen_item;
}

list<unique_ptr<item::AbstractItem>> Manager::Get(string channel) {
  impl_->EnsureRegistered();
  EventInfo info(EventInfo::Get, util::GetAppId(), channel);
  list<Bundle> result = impl_->sender_->Request(info);
  list<unique_ptr<item::AbstractItem>> gen_list;
  for (auto& i : result) {
    unique_ptr<AbstractItem> gen_item = ItemInflator::Create(i);
    gen_list.push_back(std::move(gen_item));
  }
  return gen_list;
}

int Manager::SendEvent(const IEventInfo& info,
    shared_ptr<item::AbstractItem> noti) {
  LOGI("manager-sendevent");
  impl_->EnsureRegistered();
  Bundle serialized = noti->Serialize();
  Bundle serialized_info = info.Serialize();
  list<Bundle> serialized_list {serialized};
  impl_->sender_->Notify(info, serialized_list, noti->GetSenderAppId());
  return info.GetRequestId();
}

int Manager::SendEvent(const IEventInfo& info,
    list<shared_ptr<item::AbstractItem>> notiList) {
  LOGI("manager-sendevent - list");
  impl_->EnsureRegistered();
  list<Bundle> serialized_list;
  for (auto& i : notiList) {
    Bundle b = i->Serialize();
    serialized_list.push_back(b);
  }
  impl_->sender_->Notify(info, serialized_list);
  return info.GetRequestId();
}

list<Bundle> Manager::OnRequest(const IEventInfo& info) {
  list<shared_ptr<item::AbstractItem>> item_list = OnRequestEvent(info);
  list<Bundle> serialized_list;
  for (auto& i : item_list) {
    if (std::static_pointer_cast<IItemInfoInternal>(i->GetInfo())
          ->CanReceive(impl_->receiver_group_))
      serialized_list.push_back(i->Serialize());
  }
  return serialized_list;
}

int Manager::OnRequestNumber(const IEventInfo& info) {
  return OnRequestNumberEvent(info);
}

void Manager::OnEvent(const IEventInfo& info, list<Bundle> serialized) {
  shared_ptr<AbstractItem> gen_item;
  int type = info.GetEventType();
  NotificationError error =
      (static_cast<const IEventInfoInternal&>(info)).GetError();
  if (error != ERROR_NONE) {
    LOGE("Handling error event (%d)", error);
    OnError(error, info.GetRequestId());
    return;
  }

  switch (type) {
    case EventInfo::Post: {
      list<shared_ptr<item::AbstractItem>> added;
      for (auto& i : serialized) {
        gen_item = ItemInflator::Create(i);
        if (std::static_pointer_cast<IItemInfoInternal>(gen_item->GetInfo())
              ->CanReceive(impl_->receiver_group_))
          added.emplace_back(gen_item);
      }
      if (added.size() > 0)
        OnAdd(info, added);
      break;
    }
    case EventInfo::Update: {
      list<shared_ptr<item::AbstractItem>> updated;
      for (auto& i : serialized) {
        gen_item = ItemInflator::Create(i);
        if (std::static_pointer_cast<IItemInfoInternal>(gen_item->GetInfo())
              ->CanReceive(impl_->receiver_group_))
          updated.emplace_back(gen_item);
      }
      if (updated.size() > 0)
        OnUpdate(info, updated);
      break;
    }
    case EventInfo::Delete: {
      list<shared_ptr<item::AbstractItem>> deleted;
      for (auto& i : serialized) {
        gen_item = ItemInflator::Create(i);
        if (std::static_pointer_cast<IItemInfoInternal>(gen_item->GetInfo())
              ->CanReceive(impl_->receiver_group_))
          deleted.emplace_back(gen_item);
      }
      if (deleted.size() > 0)
        OnDelete(info, deleted);
      break;
    }
    case EventInfo::Get:
      break;
    case EventInfo::Error:
      break;
    case EventInfo::Count:
      break;
    case EventInfo::DeleteAll: {
      list<shared_ptr<item::AbstractItem>> deleted;
      deleted.emplace_back(new NullItem());
      OnDelete(info, deleted);
      break;
    }
    case EventInfo::Custom:
      break;
  }
}

void Manager::OnAdd(const IEventInfo& info,
      list<shared_ptr<item::AbstractItem>> addedList) {
}

void Manager::OnUpdate(const IEventInfo& info,
      list<shared_ptr<item::AbstractItem>> updatedList) {
}

void Manager::OnDelete(const IEventInfo& info,
      list<shared_ptr<item::AbstractItem>> deletedList) {
}

void Manager::OnError(NotificationError error, int requestId) {
}

list<shared_ptr<item::AbstractItem>> Manager::OnRequestEvent(
    const IEventInfo& info) {
  return list<shared_ptr<item::AbstractItem>>({});
}

int Manager::OnRequestNumberEvent(const IEventInfo& info) {
  return 0;
}

string Manager::GetPath() {
  return NOTIFICATION_EX_MANAGER_OBJECT_PATH;
}
/* LCOV_EXCL_STOP */

}  // namespace notification
