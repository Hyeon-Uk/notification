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
#include "notification-ex/item_info_internal.h"
#include "notification-ex/shared_file.h"
#include "notification-ex/exception.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "NOTIFICATION_EX"
#define MAX_PACKAGE_STR_SIZE 512
#define NOTIFICATION_EX_REPORTER_OBJECT_PATH "/org/tizen/notification_ex_reporter"

using namespace std;
using namespace tizen_base;
using namespace notification::item;
namespace notification {

/* LCOV_EXCL_START */
Reporter::Reporter(
    unique_ptr<IEventSender> sender, unique_ptr<IEventListener> listener)
  : impl_(new Impl(this, std::move(sender), std::move(listener))) {
}
Reporter::~Reporter() = default;

Reporter::Impl::~Impl() {
  /* N4-T1: only unregister if EnsureRegistered() ever actually fired.
   * registered_ is set inside the call_once lambda with release order;
   * acquire-load here pairs with that release. Without this guard, a
   * Reporter destroyed without ever being used would still call
   * UnRegisterObserver on an observer the listener never saw. */
  if (registered_.load(std::memory_order_acquire))
    listener_->UnRegisterObserver(parent_);
}
Reporter::Impl::Impl(Reporter* parent,
    unique_ptr<IEventSender> sender, unique_ptr<IEventListener> listener)
  : sender_(std::move(sender)), listener_(std::move(listener)), parent_(parent) {
  LOGI("impl created (lazy register)");
  /* N4-T1: do NOT call listener_->RegisterObserver(parent_) here. The
   * DBusEventListener implementation pulls in g_bus_get_sync +
   * g_bus_own_name_on_connection (~20-50 ms cold start). Defer to the
   * first user-facing entry point via EnsureRegistered(). */
}

void Reporter::Impl::EnsureRegistered() {
  std::call_once(register_once_, [this]() {
    listener_->RegisterObserver(parent_);
    registered_.store(true, std::memory_order_release);
  });
}

void Reporter::SendError(const IEventInfo& info, NotificationError error) {
  impl_->EnsureRegistered();
  list<Bundle> serialized_list {};
  IEventInfo& i = const_cast<IEventInfo&>(info);
  static_cast<IEventInfoInternal&>(i).SetError(error);
  static_cast<IEventInfoInternal&>(i).SetEventType(EventInfo::Error);
  impl_->sender_->Notify(info, serialized_list, info.GetOwner());
}

int Reporter::Post(std::shared_ptr<item::AbstractItem> noti) {
  LOGI("Post noti");
  impl_->EnsureRegistered();
  static_pointer_cast<IItemInfoInternal>(noti->GetInfo())->SetTime(time(NULL));
  Bundle serialized = noti->Serialize();

  EventInfo info(
      EventInfo::Post, util::GetAppId(), noti->GetChannel(), noti->GetId());
  list<Bundle> serialized_list {serialized};
  SharedFile::CopyPrivateFile(noti);

  impl_->sender_->SendMessage(info, serialized_list);
  return info.GetRequestId();
}

int Reporter::Post(std::list<std::shared_ptr<AbstractItem>> notiList) {
  LOGI("Post noti list");
  impl_->EnsureRegistered();
  EventInfo info(EventInfo::Post, util::GetAppId(), "");
  list<Bundle> serialized_list;
  for (auto& i : notiList) {
    static_pointer_cast<IItemInfoInternal>(i->GetInfo())->SetTime(time(NULL));
    Bundle b = i->Serialize();
    serialized_list.push_back(b);
    SharedFile::CopyPrivateFile(i);
  }

  impl_->sender_->SendMessage(info, serialized_list);
  return info.GetRequestId();
}

int Reporter::Update(std::shared_ptr<AbstractItem> noti) {
  impl_->EnsureRegistered();
  static_pointer_cast<IItemInfoInternal>(noti->GetInfo())->SetTime(time(NULL));
  Bundle serialized = noti->Serialize();

  EventInfo info(
      EventInfo::Update, util::GetAppId(), noti->GetChannel(), noti->GetId());
  list<Bundle> serialized_list {serialized};
  SharedFile::CopyPrivateFile(noti);

  impl_->sender_->SendMessage(info, serialized_list);
  return info.GetRequestId();
}

int Reporter::Update(
    std::list<std::shared_ptr<AbstractItem>> notiList) {
  impl_->EnsureRegistered();
  EventInfo info(EventInfo::Update, util::GetAppId(), "");
  list<Bundle> serialized_list;
  for (auto& i : notiList) {
    static_pointer_cast<IItemInfoInternal>(i->GetInfo())->SetTime(time(NULL));
    Bundle b = i->Serialize();
    serialized_list.push_back(b);
    SharedFile::CopyPrivateFile(i);
  }

  impl_->sender_->SendMessage(info, serialized_list);
  return info.GetRequestId();
}

int Reporter::Delete(std::shared_ptr<AbstractItem> noti) {
  impl_->EnsureRegistered();
  Bundle serialized = noti->Serialize();
  EventInfo info(
      EventInfo::Delete, util::GetAppId(), noti->GetChannel(), noti->GetId());
  list<Bundle> serialized_list {serialized};
  impl_->sender_->SendMessage(
      info, serialized_list, noti->GetSenderAppId());
  return info.GetRequestId();
}

int Reporter::Delete(
    std::list<std::shared_ptr<AbstractItem>> notiList) {
  impl_->EnsureRegistered();
  EventInfo info(EventInfo::Delete, util::GetAppId(), "");
  list<Bundle> serialized_list;
  for (auto& i : notiList) {
    static_pointer_cast<IItemInfoInternal>(i->GetInfo())->SetTime(time(NULL));
    Bundle b = i->Serialize();
    serialized_list.push_back(b);
    SharedFile::CopyPrivateFile(i);
  }

  impl_->sender_->SendMessage(info, serialized_list);
  return info.GetRequestId();
}

int Reporter::DeleteAll() {
  impl_->EnsureRegistered();
  EventInfo info(EventInfo::DeleteAll, util::GetAppId(), "");
  impl_->sender_->RequestReturnValue(info);
  return info.GetRequestId();
}

int Reporter::DeleteByChannel(string channel) {
  impl_->EnsureRegistered();
  EventInfo info(EventInfo::DeleteAll, util::GetAppId(), channel);
  impl_->sender_->RequestReturnValue(info);
  return info.GetRequestId();
}

std::unique_ptr<AbstractItem> Reporter::FindByRootID(std::string id) {
  impl_->EnsureRegistered();
  EventInfo info(EventInfo::Get, util::GetAppId(), "", id);
  list<Bundle> result = impl_->sender_->Request(info);
  Bundle b = result.front();
  unique_ptr<AbstractItem> gen_item = ItemInflator::Create(b);
  return gen_item;
}

list<unique_ptr<AbstractItem>> Reporter::FindByChannel(string channel) {
  impl_->EnsureRegistered();
  EventInfo info(EventInfo::Get, util::GetAppId(), channel, "");
  list<Bundle> result = impl_->sender_->Request(info);
  list<unique_ptr<AbstractItem>> gen_item_list;
  for (auto& i : result)
    gen_item_list.push_back(ItemInflator::Create(i));

  return gen_item_list;
}

list<unique_ptr<AbstractItem>> Reporter::FindAll() {
  impl_->EnsureRegistered();
  EventInfo info(EventInfo::Get, util::GetAppId(), "", "");
  list<Bundle> result = impl_->sender_->Request(info);
  list<unique_ptr<AbstractItem>> gen_item_list;
  for (auto& i : result)
    gen_item_list.push_back(ItemInflator::Create(i));

  return gen_item_list;
}

int Reporter::GetCount(string channel) const {
  impl_->EnsureRegistered();
  EventInfo info(EventInfo::Count, util::GetAppId(), channel);
  return impl_->sender_->RequestReturnValue(info);
}

int Reporter::SendEvent(const IEventInfo& info,
    std::list<std::shared_ptr<item::AbstractItem>> notiList) {
  impl_->EnsureRegistered();
  list<Bundle> serialized_list;
  for (auto& i : notiList) {
    Bundle b = i->Serialize();
    serialized_list.push_back(b);
  }

  impl_->sender_->Notify(info, serialized_list);
  return info.GetRequestId();
}

void Reporter::OnEvent(const IEventInfo& info, list<Bundle> serialized) {
  NotificationError error =
      (static_cast<const IEventInfoInternal&>(info)).GetError();
  list<shared_ptr<item::AbstractItem>> item_list;
  if (info.GetEventType() == EventInfo::Error) {
    OnError(error, info.GetRequestId());
    return;
  } else if (info.GetEventType() == EventInfo::DeleteAll) {
    OnEvent(info, item_list);
    return;
  }

  for (auto& i : serialized) {
    shared_ptr<AbstractItem> gen_item = ItemInflator::Create(i);
    item_list.emplace_back(gen_item);
  }
  OnEvent(info, item_list);
}

list<shared_ptr<item::AbstractItem>> Reporter::OnRequestEvent(
      const IEventInfo& info) {
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

int Reporter::OnRequestNumber(const IEventInfo& info) {
  return 0;
}

void Reporter::OnEvent(
    const IEventInfo& info, list<shared_ptr<item::AbstractItem>> notiList) {
}

void Reporter::OnError(NotificationError error, int requestId) {
}

string Reporter::GetPath() {
  return NOTIFICATION_EX_REPORTER_OBJECT_PATH;
}

void Reporter::OnRegister(const IEventInfo& info) {
}
/* LCOV_EXCL_STOP */

}  // namespace notification
