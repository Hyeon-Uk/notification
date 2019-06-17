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

#include "api/notification_ex_app_control_action.h"
#include "api/notification_ex_button.h"
#include "api/notification_ex_chat_message.h"
#include "api/notification_ex_checkbox.h"
#include "api/notification_ex_entry.h"
#include "api/notification_ex_event_info.h"
#include "api/notification_ex_group.h"
#include "api/notification_ex_image.h"
#include "api/notification_ex_input_selector.h"
#include "api/notification_ex_item.h"
#include "api/notification_ex_manager.h"
#include "api/notification_ex_progress.h"
#include "api/notification_ex_reporter.h"
#include "api/notification_ex_text.h"
#include "api/notification_ex_time.h"
#include "api/notification_ex_visibility_action.h"
#include "notification-ex/reporter.h"
#include "notification-ex/app_control_action.h"
#include "notification-ex/button_item.h"
#include "notification-ex/chat_message_item.h"
#include "notification-ex/checkbox_item.h"
#include "notification-ex/entry_item.h"
#include "notification-ex/group_item.h"
#include "notification-ex/input_selector_item.h"
#include "notification-ex/abstract_item.h"
#include "notification-ex/progress_item.h"
#include "notification-ex/time_item.h"
#include "notification-ex/visibility_action.h"
#include "notification-ex/ex_bundle.h"
#include "notification-ex/event_info_internal.h"
#include "notification-ex/manager.h"
#include "notification-ex/dbus_sender.h"
#include "notification-ex/dbus_event_listener.h"
#include "notification-ex/exception.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "NOTIFICATION_EX"

#ifdef EXPORT_API
#undef EXPORT_API
#endif
#define EXPORT_API __attribute__((visibility("default")))

using namespace std;
using namespace notification::item;
using namespace notification;

namespace {

class Handle {
 public:
  explicit Handle(item::AbstractItem* ref) : ref_(ref) { }
  explicit Handle(std::shared_ptr<item::AbstractItem> ptr)
      : ref_(nullptr), ptr_(move(ptr)) { }
  virtual ~Handle() = default;
  item::AbstractItem* Get() const {
    if (ptr_ == nullptr)
      return ref_;
    return ptr_.get();
  }

  bool IsValidType(int type) const {
    return (Get()->GetType() == type
        || Get()->GetType() >= AbstractItem::Custom);
  }

  std::shared_ptr<item::AbstractItem> GetPtr() const {
    if (ptr_ == nullptr)
      return std::shared_ptr<item::AbstractItem>({});
    return ptr_;
  }

 private:
  item::AbstractItem* ref_;
  std::shared_ptr<item::AbstractItem> ptr_;
};

class ManagerCallbackInfo {
 public:
  ManagerCallbackInfo(noti_ex_manager_events_s cb, void* user_data)
    : user_data_(user_data) {
    cb_.added = cb.added;
    cb_.updated = cb.updated;
    cb_.deleted = cb.deleted;
    cb_.error = cb.error;
  }

  void InvokeAdded(Manager* manager, const IEventInfo& info,
      list<shared_ptr<AbstractItem>> addedItem) {
    if (cb_.added == nullptr)
      return;
    noti_ex_item_h* added_item =
        (noti_ex_item_h*)calloc(addedItem.size(), sizeof(noti_ex_item_h));
    if (added_item == nullptr) {
      LOGE("Out of memory");
      return;
    }

    int idx = 0;
    for (auto& i : addedItem) {
      added_item[idx++] =
          static_cast<noti_ex_item_h>(new Handle(shared_ptr<AbstractItem>(i)));
    }

    IEventInfo* c_info = const_cast<IEventInfo*>(&info);
    cb_.added(static_cast<noti_ex_manager_h>(manager),
        static_cast<noti_ex_event_info_h>(c_info), added_item,
        addedItem.size(), user_data_);
    free(added_item);
  }

  void InvokeUpdated(Manager* manager, const IEventInfo& info,
      shared_ptr<item::AbstractItem> updatedItem) {
    if (cb_.updated == nullptr)
      return;
    IEventInfo* c_info = const_cast<IEventInfo*>(&info);
    cb_.updated(static_cast<noti_ex_manager_h>(manager),
        static_cast<noti_ex_event_info_h>(c_info),
        static_cast<noti_ex_item_h>(new Handle(updatedItem)), user_data_);
  }

  void InvokeDeleted(Manager* manager, const IEventInfo& info,
      shared_ptr<item::AbstractItem> deletedItem) {
    if (cb_.deleted == nullptr)
      return;
    IEventInfo* c_info = const_cast<IEventInfo*>(&info);
    cb_.deleted(static_cast<noti_ex_manager_h>(manager),
        static_cast<noti_ex_event_info_h>(c_info),
        static_cast<noti_ex_item_h>(
            new Handle(deletedItem)), user_data_);
  }

  void InvokeError(Manager* manager, NotificationError error, int requestId) {
    if (cb_.error == nullptr)
      return;
    cb_.error(static_cast<noti_ex_manager_h>(manager),
        static_cast<noti_ex_error_e>(error), requestId, user_data_);
  }

 private:
  noti_ex_manager_events_s cb_;
  void* user_data_;
};

class ManagerStub : public Manager {
 public:
  ManagerStub(std::unique_ptr<IEventSender> sender,
      std::unique_ptr<IEventListener> listener, std::string receiver_group = "")
    : Manager(move(sender), move(listener), receiver_group) {
  }

  void OnAdd(const IEventInfo& info,
      list<shared_ptr<AbstractItem>> addedItem) override {
    cb_->InvokeAdded(this, info, addedItem);
  }

  void OnUpdate(const IEventInfo& info,
      std::shared_ptr<item::AbstractItem> updatedItem) override {
    cb_->InvokeUpdated(this, info, updatedItem);
  }

  void OnDelete(const IEventInfo& info,
      shared_ptr<item::AbstractItem> deletedItem) override {
    cb_->InvokeDeleted(this, info, deletedItem);
  }

  void OnError(NotificationError error, int requestId) override {
    cb_->InvokeError(this, error, requestId);
  }

  int SetManagerCallbackInfo(unique_ptr<ManagerCallbackInfo> ci) {
    cb_ = move(ci);
    return NOTI_EX_ERROR_NONE;
  }

  int ClearManagerCallbackInfo() {
    cb_.reset();
    return NOTI_EX_ERROR_NONE;
  }

 private:
  unique_ptr<ManagerCallbackInfo> cb_;
};


class ReporterCallbackInfo {
 public:
  ReporterCallbackInfo(noti_ex_reporter_events_s cb, void* user_data)
    : user_data_(user_data) {
    cb_.event = cb.event;
    cb_.error = cb.error;
  }

  void InvokeEvent(Reporter* reporter, const IEventInfo& info,
      list<shared_ptr<AbstractItem>> notiList) {
    if (cb_.event == nullptr)
      return;
    noti_ex_item_h* noti_list =
        (noti_ex_item_h*)calloc(notiList.size(), sizeof(noti_ex_item_h));
    if (noti_list == nullptr) {
      LOGE("Out of memory");
      return;
    }

    int idx = 0;
    for (auto& i : notiList) {
      noti_list[idx++] =
          static_cast<noti_ex_item_h>(new Handle(i));
    }

    IEventInfo* c_info = const_cast<IEventInfo*>(&info);
    cb_.event(static_cast<noti_ex_reporter_h>(reporter),
        static_cast<noti_ex_event_info_h>(c_info), noti_list,
        notiList.size(), user_data_);
    free(noti_list);
  }

  void InvokeError(Reporter* reporter, NotificationError error, int requestId) {
    if (cb_.error == nullptr)
      return;
    cb_.error(static_cast<noti_ex_reporter_h>(reporter),
        static_cast<noti_ex_error_e>(error), requestId, user_data_);
  }

 private:
  noti_ex_reporter_events_s cb_;
  void* user_data_;
};

class ReporterStub : public Reporter {
 public:
  ReporterStub(std::unique_ptr<IEventSender> sender,
      std::unique_ptr<IEventListener> listener)
    : Reporter(move(sender), move(listener)) {
  }

  void OnEvent(const IEventInfo& info,
      std::list<std::shared_ptr<item::AbstractItem>> notiList) override {
    cb_->InvokeEvent(this, info, notiList);
  }

  void OnError(NotificationError error, int requestId) override {
    cb_->InvokeError(this, error, requestId);
  }

  int SetReporterCallbackInfo(unique_ptr<ReporterCallbackInfo> ci) {
    cb_ = move(ci);
    return NOTI_EX_ERROR_NONE;
  }

  int ClearReporterCallbackInfo() {
    cb_.reset();
    return NOTI_EX_ERROR_NONE;
  }

 private:
  unique_ptr<ReporterCallbackInfo> cb_;
};

}  // namespace

extern "C" EXPORT_API int noti_ex_action_app_control_create(
    noti_ex_action_h *handle, app_control_h app_control,
    const char *extra) {
  if (handle == nullptr || app_control == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  AppControlAction* p;

  if (extra)
    p = new (std::nothrow) AppControlAction(app_control, extra);
  else
    p = new (std::nothrow) AppControlAction(app_control);

  if (p == nullptr) {
    LOGE("Out-of-memory");
    return NOTI_EX_ERROR_OUT_OF_MEMORY;
  }

  *handle = p;

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_action_app_control_set(
    noti_ex_action_h handle, app_control_h app_control) {
  if (handle == nullptr || app_control == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  AppControlAction* p = static_cast<AppControlAction*>(handle);
  p->SetAppControl(app_control);

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_action_app_control_get(
    noti_ex_action_h handle, app_control_h *app_control) {
  if (handle == nullptr || app_control == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  AppControlAction* p = static_cast<AppControlAction*>(handle);
  *app_control = p->GetAppControl();

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_item_button_create(noti_ex_item_h *handle,
    const char *id, const char *title) {
  ButtonItem* p;

  if (handle == nullptr || title == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  if (id)
    p = new (std::nothrow) ButtonItem(id, title);
  else
    p = new (std::nothrow) ButtonItem(title);

  if (p == nullptr) {
    LOGE("Out-of-memory");
    return NOTI_EX_ERROR_OUT_OF_MEMORY;
  }
  *handle = new Handle(shared_ptr<AbstractItem>(p));

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_item_button_get_title(noti_ex_item_h handle,
    char **title) {
  if (handle == nullptr || title == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  Handle* sp = static_cast<Handle*>(handle);
  if (!sp->IsValidType(AbstractItem::Button)) {
    LOGE("Invalid handle type");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }
  ButtonItem* p = static_cast<ButtonItem*>(sp->Get());
  if (!p->GetTitle().empty()) {
    *title = strdup(p->GetTitle().c_str());
    if (*title == nullptr) {
      LOGE("Out-of-memory");
      return NOTI_EX_ERROR_OUT_OF_MEMORY;
    }
  }

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_item_chat_message_create(
    noti_ex_item_h *handle, const char *id, noti_ex_item_h name,
    noti_ex_item_h text, noti_ex_item_h image, noti_ex_item_h time,
    noti_ex_item_chat_message_type_e message_type) {
  if (handle == nullptr || message_type > NOTI_EX_ITEM_CHAT_MESSAGE_TYPE_SENDER) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  auto* p = new (std::nothrow) ChatMessageItem(id,
          dynamic_pointer_cast<TextItem>(static_cast<Handle*>(name)->GetPtr()),
          dynamic_pointer_cast<TextItem>(static_cast<Handle*>(text)->GetPtr()),
          dynamic_pointer_cast<ImageItem>(static_cast<Handle*>(image)->GetPtr()),
          dynamic_pointer_cast<TimeItem>(static_cast<Handle*>(time)->GetPtr()),
          static_cast<ChatMessageItem::Type>(message_type));
  if (p == nullptr) {
    LOGE("Out-of-memory");
    return NOTI_EX_ERROR_OUT_OF_MEMORY;
  }

  *handle = new Handle(shared_ptr<AbstractItem>(p));

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_item_chat_message_get_name(
    noti_ex_item_h handle, noti_ex_item_h *name) {
  if (handle == nullptr || name == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }
  Handle* h = static_cast<Handle*>(handle);
  if (!h->IsValidType(AbstractItem::ChatMessage)) {
    LOGE("Invalid handle type");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }
  ChatMessageItem* p = static_cast<ChatMessageItem*>(h->Get());
  *name = new Handle(&(p->GetNameItem()));

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_item_chat_message_get_text(
    noti_ex_item_h handle, noti_ex_item_h *text) {
  if (handle == nullptr || text == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  Handle* h = static_cast<Handle*>(handle);
  if (!h->IsValidType(AbstractItem::ChatMessage)) {
    LOGE("Invalid handle type");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }
  ChatMessageItem* p = static_cast<ChatMessageItem*>(h->Get());
  *text = new Handle(&(p->GetTextItem()));

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_item_chat_message_get_image(
    noti_ex_item_h handle, noti_ex_item_h *image) {
  if (handle == nullptr || image == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  Handle* h = static_cast<Handle*>(handle);
  if (!h->IsValidType(AbstractItem::ChatMessage)) {
    LOGE("Invalid handle type");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }
  ChatMessageItem* p = static_cast<ChatMessageItem*>(h->Get());
  *image =  new Handle(&(p->GetImageItem()));

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_item_chat_message_get_time(
    noti_ex_item_h handle, noti_ex_item_h *time) {
  if (handle == nullptr || time == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  Handle* h = static_cast<Handle*>(handle);
  if (!h->IsValidType(AbstractItem::ChatMessage)) {
    LOGE("Invalid handle type");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }
  ChatMessageItem* p = static_cast<ChatMessageItem*>(h->Get());
  *time = new Handle(&(p->GetTimeItem()));

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_item_chat_message_get_message_type(
    noti_ex_item_h handle, noti_ex_item_chat_message_type_e *message_type) {
  if (handle == nullptr || message_type == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  Handle* h = static_cast<Handle*>(handle);
  if (!h->IsValidType(AbstractItem::ChatMessage)) {
    LOGE("Invalid handle type");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }
  ChatMessageItem* p = static_cast<ChatMessageItem*>(h->Get());
  *message_type = (noti_ex_item_chat_message_type_e)(p->GetMessageType());

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_item_checkbox_create(noti_ex_item_h *handle,
    const char *id, const char *title, bool checked) {
  CheckBoxItem* p;

  if (handle == nullptr || title == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  p = new (std::nothrow) CheckBoxItem(id, title, checked);
  if (p == nullptr) {
    LOGE("Out-of-memory");
    return NOTI_EX_ERROR_OUT_OF_MEMORY;
  }

  *handle = new Handle(p);

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_item_checkbox_get_title(noti_ex_item_h handle,
    char **title) {
  if (handle == nullptr || title == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }
  Handle* h = static_cast<Handle*>(handle);
  if (!h->IsValidType(AbstractItem::CheckBox)) {
    LOGE("Invalid handle type");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }
  CheckBoxItem* p = static_cast<CheckBoxItem*>(h->Get());
  if (!p->GetTitle().empty()) {
    *title = strdup(p->GetTitle().c_str());
    if (*title == nullptr) {
        LOGE("Out-of-memory");
        return NOTI_EX_ERROR_OUT_OF_MEMORY;
    }
  }

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_item_checkbox_is_checked(noti_ex_item_h handle,
    bool *checked) {
  if (handle == nullptr || checked == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }
  Handle* h = static_cast<Handle*>(handle);
  if (!h->IsValidType(AbstractItem::CheckBox)) {
    LOGE("Invalid handle type");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }
  CheckBoxItem* p = static_cast<CheckBoxItem*>(h->Get());
  *checked = p->IsChecked();

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_item_entry_create(noti_ex_item_h *handle,
    const char *id) {
  EntryItem* p;

  if (handle == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  p = new (std::nothrow) EntryItem(id);
  if (p == nullptr) {
    LOGE("Out-of-memory");
    return NOTI_EX_ERROR_OUT_OF_MEMORY;
  }

  *handle = new Handle(p);

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_item_entry_get_text(noti_ex_item_h handle,
    char **text) {
  if (handle == nullptr || text == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  Handle* h = static_cast<Handle*>(handle);
  if (!h->IsValidType(AbstractItem::Entry)) {
    LOGE("Invalid handle type");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }
  EntryItem* p = static_cast<EntryItem*>(h->Get());
  if (!p->GetText().empty()) {
    *text = strdup(p->GetText().c_str());
    if (*text == nullptr) {
        LOGE("Out-of-memory");
        return NOTI_EX_ERROR_OUT_OF_MEMORY;
    }
  }

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_item_entry_set_text(noti_ex_item_h handle,
    const char *text) {
  if (handle == nullptr || text == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }
  Handle* h = static_cast<Handle*>(handle);
  if (!h->IsValidType(AbstractItem::Entry)) {
    LOGE("Invalid handle type");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }
  EntryItem* p = static_cast<EntryItem*>(h->Get());
  p->SetText(std::string(text));

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_event_info_clone(noti_ex_event_info_h handle,
		noti_ex_event_info_h* cloned_handle) {
  if (handle == nullptr || cloned_handle == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  Bundle cloned = static_cast<EventInfo*>(handle)->Serialize();
  EventInfo* info = new EventInfo(cloned);
  *cloned_handle = info;
  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_event_info_destroy(
    noti_ex_event_info_h handle) {
  if (handle == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }
  EventInfo* info = static_cast<EventInfo*>(handle);
  delete info;
  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_event_info_get_event_type(
    noti_ex_event_info_h handle, noti_ex_event_info_type_e *event_type) {
  if (handle == nullptr || event_type == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }
  EventInfo* info = static_cast<EventInfo*>(handle);
  *event_type = static_cast<noti_ex_event_info_type_e>(info->GetEventType());

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_event_info_get_owner(
    noti_ex_event_info_h handle, char **owner) {
  if (handle == nullptr || owner == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }
  EventInfo* info = static_cast<EventInfo*>(handle);
  *owner = strdup(info->GetOwner().c_str());
  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_event_info_get_channel(
    noti_ex_event_info_h handle, char **channel) {
  if (handle == nullptr || channel == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }
  EventInfo* info = static_cast<EventInfo*>(handle);
  *channel = strdup(info->GetChannel().c_str());
  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_event_info_get_item_id(
    noti_ex_event_info_h handle, char **item_id) {
  if (handle == nullptr || item_id == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }
  EventInfo* info = static_cast<EventInfo*>(handle);
  *item_id = strdup(info->GetItemId().c_str());
  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_event_info_get_request_id(
    noti_ex_event_info_h handle, int *req_id) {
  if (handle == nullptr || req_id == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }
  EventInfo* info = static_cast<EventInfo*>(handle);
  *req_id = info->GetRequestId();
  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_item_group_create(noti_ex_item_h *handle,
    const char *id) {
  GroupItem* p;

  if (handle == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  if (id)
    p = new (std::nothrow) GroupItem(id);
  else
    p = new (std::nothrow) GroupItem();

  if (p == nullptr) {
    LOGE("Out-of-memory");
    return NOTI_EX_ERROR_OUT_OF_MEMORY;
  }

  *handle = new Handle(shared_ptr<AbstractItem>(p));

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_item_group_set_direction(noti_ex_item_h handle,
    bool vertical) {
  if (handle == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }
  Handle* h = static_cast<Handle*>(handle);
  if (!h->IsValidType(AbstractItem::Group)) {
    LOGE("Invalid handle type");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }
  GroupItem* p = static_cast<GroupItem*>(h->Get());
  p->SetDirection(vertical);

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_item_group_is_vertical(noti_ex_item_h handle,
    bool *vertical) {
  if (handle == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }
  Handle* h = static_cast<Handle*>(handle);
  if (!h->IsValidType(AbstractItem::Group)) {
    LOGE("Invalid handle type");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }
  GroupItem* p = static_cast<GroupItem*>(h->Get());
  *vertical = p->IsVertical();

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_item_group_get_app_label(noti_ex_item_h handle,
    char **label) {
  if (handle == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }
  Handle* h = static_cast<Handle*>(handle);
  if (!h->IsValidType(AbstractItem::Group)) {
    LOGE("Invalid handle type");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }
  GroupItem* p = static_cast<GroupItem*>(h->Get());
  if (!p->GetAppLabel().empty()) {
    *label = strdup(p->GetAppLabel().c_str());
    if (*label == nullptr) {
      LOGE("Out-of-memory");
      return NOTI_EX_ERROR_OUT_OF_MEMORY;
    }
  }

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_item_group_add_child(noti_ex_item_h handle,
    noti_ex_item_h child) {
  if (handle == nullptr || child == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }
  Handle* h = static_cast<Handle*>(handle);
  if (!h->IsValidType(AbstractItem::Group)) {
    LOGE("Invalid handle type");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }
  auto p = static_cast<GroupItem*>(h->Get());
  p->AddChild((static_cast<Handle*>(child))->GetPtr());

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_item_group_remove_child(noti_ex_item_h handle,
    const char *item_id) {
  if (handle == nullptr || item_id == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }
  Handle* h = static_cast<Handle*>(handle);
  if (!h->IsValidType(AbstractItem::Group)) {
    LOGE("Invalid handle type");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }
  GroupItem* p = static_cast<GroupItem*>(h->Get());
  p->RemoveChild(std::string(item_id));

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_item_group_foreach_child(noti_ex_item_h handle,
    noti_ex_item_group_foreach_child_cb callback, void *data) {
  if (handle == nullptr || callback == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  Handle* h = static_cast<Handle*>(handle);
  if (!h->IsValidType(AbstractItem::Group)) {
    LOGE("Invalid handle type");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }
  GroupItem* p = static_cast<GroupItem*>(h->Get());
  list<shared_ptr<AbstractItem>> children = p->GetChildren();
  LOGI("Retrive (%zd)", children.size());
  for (auto i : children) {
    int ret = callback(
        static_cast<noti_ex_item_h>(new Handle(i)), data);
    if (ret != NOTI_EX_ERROR_NONE) {
      LOGW("callback return (%d) stop foreach", ret);
      break;
    }
  }

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_item_image_create(noti_ex_item_h *handle,
    const char *id, const char *image_path) {
  ImageItem* p;

  if (handle == nullptr  || image_path == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  if (id)
    p = new (std::nothrow) ImageItem(id, image_path);
  else
    p = new (std::nothrow) ImageItem(image_path);

  if (p == nullptr) {
    LOGE("Out-of-memory");
    return NOTI_EX_ERROR_OUT_OF_MEMORY;
  }

  *handle = new Handle(shared_ptr<AbstractItem>(p));

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_item_image_get_image_path(
    noti_ex_item_h handle, char **image_path) {
  if (handle == nullptr || image_path == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }
  Handle* h = static_cast<Handle*>(handle);
  if (!h->IsValidType(AbstractItem::Image)) {
    LOGE("Invalid handle type");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }
  ImageItem* p = static_cast<ImageItem*>(h->Get());
  if (!p->GetImagePath().empty()) {
    *image_path = strdup(p->GetImagePath().c_str());
    if (*image_path == nullptr) {
      LOGE("Out-of-memory");
      return NOTI_EX_ERROR_OUT_OF_MEMORY;
    }
  }

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_item_input_selector_create(
    noti_ex_item_h *handle, const char *id) {
  InputSelectorItem* p;

  if (handle == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  if (id)
    p = new (std::nothrow) InputSelectorItem(id);
  else
    p = new (std::nothrow) InputSelectorItem();

  if (p == nullptr) {
    LOGE("Out-of-memory");
    return NOTI_EX_ERROR_OUT_OF_MEMORY;
  }

  *handle = new Handle(p);

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_item_input_selector_get_contents(
    noti_ex_item_h handle, char ***contents_list, int *count) {
  if (handle == nullptr || contents_list == nullptr || count == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  Handle* h = static_cast<Handle*>(handle);
  if (!h->IsValidType(AbstractItem::InputSelector)) {
    LOGE("Invalid handle type");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }
  InputSelectorItem* p = static_cast<InputSelectorItem*>(h->Get());
  list<string> contents = p->GetContents();
  char **list = (char**)calloc(contents.size(), sizeof(char*));
  int idx = 0;
  for (auto& i : contents) {
    list[idx++] = strdup(i.c_str());
  }
  *count = contents.size();
  *contents_list = list;

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_item_input_selector_set_contents(
    noti_ex_item_h handle, const char **contents, int count) {
  if (handle == nullptr || contents == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  list<string> new_contents;
  Handle* h = static_cast<Handle*>(handle);
  if (!h->IsValidType(AbstractItem::InputSelector)) {
    LOGE("Invalid handle type");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }
  InputSelectorItem* p = static_cast<InputSelectorItem*>(h->Get());
  for (int i = 0; i < count; i++) {
    new_contents.push_back(contents[i]);
  }
  p->SetContents(move(new_contents));

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_color_create(noti_ex_color_h *handle,
    unsigned char a, unsigned char r, unsigned char g, unsigned char b) {
  if (handle == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  auto* ptr = new (std::nothrow) shared_ptr<Color>(
      new (std::nothrow) Color(a, r, g, b));
  if (ptr == nullptr || ptr->get() == nullptr) {
    LOGE("Out-of-memory");
    return NOTI_EX_ERROR_OUT_OF_MEMORY;
  }

  *handle = ptr;

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_color_destroy(noti_ex_color_h handle) {
  if (handle == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  shared_ptr<Color>* p = static_cast<shared_ptr<Color>*>(handle);
  delete p;

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_color_get_alpha(noti_ex_color_h handle,
    unsigned char *val) {
  if (handle == nullptr || val == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  shared_ptr<Color>* p = static_cast<shared_ptr<Color>*>(handle);
  *val = (*p)->GetAVal();

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_color_get_red(noti_ex_color_h handle,
    unsigned char *val) {
  if (handle == nullptr || val == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  shared_ptr<Color>* p = static_cast<shared_ptr<Color>*>(handle);
  *val = (*p)->GetRVal();

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_color_get_green(noti_ex_color_h handle,
    unsigned char *val) {
  if (handle == nullptr || val == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  shared_ptr<Color>* p = static_cast<shared_ptr<Color>*>(handle);
  *val = (*p)->GetGVal();

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_color_get_blue(noti_ex_color_h handle,
    unsigned char *val) {
  if (handle == nullptr || val == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  shared_ptr<Color>* p = static_cast<shared_ptr<Color>*>(handle);
  *val = (*p)->GetBVal();

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_padding_create(noti_ex_padding_h *handle,
    int left, int top, int right, int bottom) {
  if (handle == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  auto* p = new (std::nothrow) Padding(left, top, right, bottom);
  if (p == nullptr) {
    LOGE("Out-of-memory");
    return NOTI_EX_ERROR_OUT_OF_MEMORY;
  }

  auto* ptr = new (std::nothrow) shared_ptr<Padding>(
      new (std::nothrow) Padding(left, top, right, bottom));
  if (ptr == nullptr || ptr->get() == nullptr) {
    LOGE("Out-of-memory");
    return NOTI_EX_ERROR_OUT_OF_MEMORY;
  }

  *handle = ptr;

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_padding_destroy(noti_ex_padding_h handle) {
  if (handle == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  shared_ptr<Padding>* p = static_cast<shared_ptr<Padding>*>(handle);
  delete p;

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_padding_get_left(noti_ex_padding_h handle,
    int *val) {
  if (handle == nullptr || val == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  shared_ptr<Padding>* p = static_cast<shared_ptr<Padding>*>(handle);
  *val = (*p)->GetLeft();

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_padding_get_top(noti_ex_padding_h handle,
    int *val) {
  if (handle == nullptr || val == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  shared_ptr<Padding>* p = static_cast<shared_ptr<Padding>*>(handle);
  *val = (*p)->GetTop();

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_padding_get_right(noti_ex_padding_h handle,
    int *val) {
  if (handle == nullptr || val == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  shared_ptr<Padding>* p = static_cast<shared_ptr<Padding>*>(handle);
  *val = (*p)->GetRight();

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_padding_get_bottom(noti_ex_padding_h handle,
    int *val) {
  if (handle == nullptr || val == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  shared_ptr<Padding>* p = static_cast<shared_ptr<Padding>*>(handle);
  *val = (*p)->GetBottom();

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_geometry_create(noti_ex_geometry_h *handle,
    int x, int y, int w, int h) {
  if (handle == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  auto* ptr = new (std::nothrow) shared_ptr<Geometry>(
      new (std::nothrow) Geometry(x, y, w, h));
  if (ptr == nullptr || ptr->get() == nullptr) {
    LOGE("Out-of-memory");
    return NOTI_EX_ERROR_OUT_OF_MEMORY;
  }

  *handle = ptr;

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_geometry_destroy(noti_ex_geometry_h handle) {
  if (handle == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  shared_ptr<Geometry>* p = static_cast<shared_ptr<Geometry>*>(handle);
  delete p;

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_geometry_get_x(noti_ex_geometry_h handle,
    int *val) {
  if (handle == nullptr || val == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  shared_ptr<Geometry>* p = static_cast<shared_ptr<Geometry>*>(handle);
  *val = (*p)->GetX();

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_geometry_get_y(noti_ex_geometry_h handle,
    int *val) {
  if (handle == nullptr || val == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  shared_ptr<Geometry>* p = static_cast<shared_ptr<Geometry>*>(handle);
  *val = (*p)->GetY();

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_geometry_get_width(noti_ex_geometry_h handle,
    int *val) {
  if (handle == nullptr || val == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  shared_ptr<Geometry>* p = static_cast<shared_ptr<Geometry>*>(handle);
  *val = (*p)->GetWidth();

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_geometry_get_height(noti_ex_geometry_h handle,
    int *val) {
  if (handle == nullptr || val == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  shared_ptr<Geometry>* p = static_cast<shared_ptr<Geometry>*>(handle);
  *val = (*p)->GetHeight();

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_style_create(noti_ex_style_h *handle,
    noti_ex_color_h color,
    noti_ex_padding_h padding,
    noti_ex_geometry_h geometry) {
  if (handle == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  shared_ptr<Color> col = (color == nullptr) ?
      nullptr : *(static_cast<shared_ptr<Color>*>(color));
  shared_ptr<Padding> padd = (padding == nullptr) ?
      nullptr : *(static_cast<shared_ptr<Padding>*>(padding));
  shared_ptr<Geometry> geo = (geometry == nullptr) ?
      nullptr : *(static_cast<shared_ptr<Geometry>*>(geometry));

  auto* ptr = new (std::nothrow) shared_ptr<Style>(
      new (std::nothrow) Style(col, padd, geo));
  if (ptr == nullptr || ptr->get() == nullptr) {
    LOGE("Out-of-memory");
    return NOTI_EX_ERROR_OUT_OF_MEMORY;
  }

  *handle = ptr;

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_style_destroy(noti_ex_style_h handle) {
  if (handle == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  shared_ptr<Style>* p = static_cast<shared_ptr<Style>*>(handle);
  delete p;

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_style_get_padding(noti_ex_style_h handle,
    noti_ex_padding_h *padding) {
  if (handle == nullptr || padding == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  shared_ptr<Style>* p = static_cast<shared_ptr<Style>*>(handle);
  if ((*p)->GetPadding() == nullptr) {
    LOGW("Padding info is null");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  shared_ptr<Padding>* padd = new (std::nothrow) shared_ptr<Padding>(
      new (std::nothrow) Padding(*((*p)->GetPadding())));
  if (padd == nullptr || padd->get() == nullptr) {
    LOGE("Out-of-memory");
    return NOTI_EX_ERROR_OUT_OF_MEMORY;
  }

  *padding = padd;

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_style_get_color(noti_ex_style_h handle,
    noti_ex_color_h *color) {
  if (handle == nullptr || color == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  shared_ptr<Style>* p = static_cast<shared_ptr<Style>*>(handle);
  if ((*p)->GetColor() == nullptr) {
    LOGW("Color info is null");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  shared_ptr<Color>* col = new (std::nothrow) shared_ptr<Color>(
      new (std::nothrow) Color(*((*p)->GetColor())));
  if (col == nullptr || col->get() == nullptr) {
    LOGE("Out-of-memory");
    return NOTI_EX_ERROR_OUT_OF_MEMORY;
  }

  *color = col;

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_style_get_geometry(noti_ex_style_h handle,
    noti_ex_geometry_h *geometry) {
  if (handle == nullptr || geometry == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  shared_ptr<Style>* p = static_cast<shared_ptr<Style>*>(handle);
  if ((*p)->GetGeometry() == nullptr) {
    LOGW("Geometry info is null");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  shared_ptr<Geometry>* geo = new (std::nothrow) shared_ptr<Geometry>(
      new (std::nothrow) Geometry(*((*p)->GetGeometry())));
  if (geo == nullptr || geo->get() == nullptr) {
    LOGE("Out-of-memory");
    return NOTI_EX_ERROR_OUT_OF_MEMORY;
  }

  *geometry = geo;

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_led_info_create(noti_ex_led_info_h *handle,
    noti_ex_color_h color) {
  if (handle == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  shared_ptr<Color>* color_ = static_cast<shared_ptr<Color>*>(color);
  auto* p = new (std::nothrow) LEDInfo(*color_);
  if (p == nullptr) {
    LOGE("Out-of-memory");
    return NOTI_EX_ERROR_OUT_OF_MEMORY;
  }

  *handle = p;

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_led_info_destroy(noti_ex_led_info_h handle) {
  if (handle == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  LEDInfo* p = static_cast<LEDInfo*>(handle);
  p->~LEDInfo();

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_led_info_set_on_period(
    noti_ex_led_info_h handle, int ms) {
  if (handle == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  LEDInfo* p = static_cast<LEDInfo*>(handle);
  p->SetOnPeriod(ms);

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_led_info_get_on_period(
    noti_ex_led_info_h handle, int *ms) {
  if (handle == nullptr || ms == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  LEDInfo* p = static_cast<LEDInfo*>(handle);
  *ms = p->GetOnPeriod();

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_led_info_set_off_period(
    noti_ex_led_info_h handle, int ms) {
  if (handle == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  LEDInfo* p = static_cast<LEDInfo*>(handle);
  p->SetOffPeriod(ms);

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_led_info_get_off_period(
    noti_ex_led_info_h handle, int *ms) {
  if (handle == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  LEDInfo* p = static_cast<LEDInfo*>(handle);
  *ms = p->GetOffPeriod();

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_led_info_get_color(
    noti_ex_led_info_h handle, noti_ex_color_h *color) {
  if (handle == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  LEDInfo* p = static_cast<LEDInfo*>(handle);
  if (p->GetColor() == nullptr) {
    LOGE("Color is null");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  shared_ptr<Color>* col = new (std::nothrow) shared_ptr<Color>(
      new (std::nothrow) Color(*(p->GetColor())));
  if (col == nullptr || col->get() == nullptr) {
    LOGE("Out-of-memory");
    return NOTI_EX_ERROR_OUT_OF_MEMORY;
  }

  *color = col;

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_action_destroy(noti_ex_action_h handle) {
  if (handle == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  AbstractAction* p = static_cast<AbstractAction*>(handle);
  p->~AbstractAction();

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_action_get_type(noti_ex_action_h handle,
    int *type) {
  if (handle == nullptr || type == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  AbstractAction* p = static_cast<AbstractAction*>(handle);
  *type = p->GetType();

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_action_is_local(noti_ex_action_h handle,
    bool *local) {
  if (handle == nullptr || local == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  AbstractAction* p = static_cast<AbstractAction*>(handle);
  *local = p->IsLocal();

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_action_execute(noti_ex_action_h handle,
    noti_ex_item_h item) {
  if (handle == nullptr || item == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }
  AbstractAction* p = static_cast<AbstractAction*>(handle);
  Handle* ih = static_cast<Handle*>(item);
  p->Execute(ih->GetPtr());

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_action_get_extra(noti_ex_action_h handle,
    char **extra) {
  if (handle == nullptr || extra == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  AbstractAction* p = static_cast<AbstractAction*>(handle);
  if (!p->GetExtra().empty()) {
    *extra = strdup(p->GetExtra().c_str());
    if (*extra == nullptr) {
      LOGE("Out-of-memory");
      return NOTI_EX_ERROR_OUT_OF_MEMORY;
    }
  }

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_item_info_get_hide_time(
    noti_ex_item_info_h handle, int *hide_time) {
  if (handle == nullptr || hide_time == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }
  IItemInfo* p = static_cast<IItemInfo*>(handle);
  *hide_time = p->GetHideTime();
  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_item_info_set_hide_time(
    noti_ex_item_info_h handle, int hide_time) {
  if (handle == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }
  IItemInfo* p = static_cast<IItemInfo*>(handle);
  p->SetHideTime(hide_time);
  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_item_info_get_delete_time(
    noti_ex_item_info_h handle, int *delete_time) {
  if (handle == nullptr || delete_time == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }
  IItemInfo* p = static_cast<IItemInfo*>(handle);
  *delete_time = p->GetDeleteTime();
  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_item_info_set_delete_time(
    noti_ex_item_info_h handle, int delete_time) {
  if (handle == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }
  IItemInfo* p = static_cast<IItemInfo*>(handle);
  p->SetDeleteTime(delete_time);
  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_item_info_get_time(
    noti_ex_item_info_h handle, time_t *time) {
  if (handle == nullptr || time == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  IItemInfo* p = static_cast<IItemInfo*>(handle);
  *time = p->GetTime();
  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_item_destroy(noti_ex_item_h handle) {
  if (handle == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  Handle* h = static_cast<Handle*>(handle);
  delete h;
  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_item_find_by_id(noti_ex_item_h handle,
    const char *id, noti_ex_item_h *item) {
  if (handle == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  Handle* p = static_cast<Handle*>(handle);
  AbstractItem& find_item = p->Get()->FindByID(string(id));
  *item = new Handle(&find_item);
  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_item_get_type(noti_ex_item_h handle,
    int *type) {
  if (handle == nullptr || type == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  Handle* h = static_cast<Handle*>(handle);
  AbstractItem* p = h->Get();
  *type = p->GetType();
  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_item_get_shared_paths(noti_ex_item_h handle,
    char ***path, int *count) {
  if (handle == nullptr || path == nullptr || count == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }
  Handle* p = static_cast<Handle*>(handle);
  list<string> shared_path = p->Get()->GetSharedPath();
  *path = (char**)calloc(shared_path.size(), sizeof(char*));
  int idx = 0;
  for (auto& i : shared_path) {
    *path[idx++] = strdup(i.c_str());
  }
  *count = shared_path.size();
  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_item_get_id(noti_ex_item_h handle,
    char **id) {
  if (handle == nullptr || id == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }
  Handle* h = static_cast<Handle*>(handle);
  AbstractItem* p = h->Get();
  *id = strdup(p->GetId().c_str());
  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_item_set_id(noti_ex_item_h handle,
    const char *id) {
  if (handle == nullptr || id == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }
  Handle* p = static_cast<Handle*>(handle);
  p->Get()->SetId(id);
  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_item_get_action(noti_ex_item_h handle,
    noti_ex_action_h *action) {
  if (handle == nullptr || action == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }
  Handle* p = static_cast<Handle*>(handle);
  if (p->Get()->GetAction() == nullptr) {
    *action = nullptr;
    return NOTI_EX_ERROR_NONE;
  }
  *action = static_cast<noti_ex_action_h>(p->Get()->GetAction().get());

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_item_set_action(noti_ex_item_h handle,
    noti_ex_action_h action) {
  if (handle == nullptr || action == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  Handle* p = static_cast<Handle*>(handle);
  AbstractAction* a = static_cast<AbstractAction*>(action);
  p->Get()->SetAction(shared_ptr<AbstractAction>(a));
  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_item_get_style(noti_ex_item_h handle,
    noti_ex_style_h *style) {
  if (handle == nullptr || style == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  Handle* p = static_cast<Handle*>(handle);
  shared_ptr<Style> s = p->Get()->GetStyle();
  if (s.get() == nullptr) {
    LOGE("Style is null");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  auto* ptr = new (std::nothrow) shared_ptr<Style>(new (std::nothrow) Style(*s));
  if (ptr == nullptr || ptr->get() == nullptr) {
    LOGE("Out of memory");
    return NOTI_EX_ERROR_OUT_OF_MEMORY;
  }

  *style = ptr;
  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_item_set_style(noti_ex_item_h handle,
    noti_ex_style_h style) {
  if (handle == nullptr || style == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  Handle* p = static_cast<Handle*>(handle);
  shared_ptr<Style>* s = static_cast<shared_ptr<Style>*>(style);
  p->Get()->SetStyle(*s);
  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_item_set_visible(noti_ex_item_h handle,
    bool visible) {
  if (handle == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  Handle* p = static_cast<Handle*>(handle);
  p->Get()->SetVisible(visible);
  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_item_get_visible(noti_ex_item_h handle,
    bool *visible) {
  if (handle == nullptr || visible == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  Handle* p = static_cast<Handle*>(handle);
  *visible = p->Get()->GetVisible();
  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_item_set_enable(noti_ex_item_h handle,
    bool enable) {
  if (handle == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  Handle* p = static_cast<Handle*>(handle);
  p->Get()->SetEnable(enable);
  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_item_get_enable(noti_ex_item_h handle,
    bool *enable) {
  if (handle == nullptr || enable == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  Handle* p = static_cast<Handle*>(handle);
  *enable = p->Get()->GetEnable();
  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_item_add_receiver(noti_ex_item_h handle,
    const char *receiver_group) {
  if (handle == nullptr || receiver_group == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  Handle* p = static_cast<Handle*>(handle);
  p->Get()->AddReceiver(receiver_group);
  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_item_remove_receiver(noti_ex_item_h handle,
    const char *receiver_group) {
  if (handle == nullptr || receiver_group == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  Handle* p = static_cast<Handle*>(handle);
  p->Get()->RemoveReceiver(receiver_group);
  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_item_get_receiver_list(noti_ex_item_h handle,
    char ***receiver_list, int *count) {
  if (handle == nullptr || receiver_list == nullptr || count == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  Handle* p = static_cast<Handle*>(handle);
  list<string> receivers = p->Get()->GetReceiverList();
  *receiver_list = (char**)calloc(receivers.size(), sizeof(char*));
  int idx = 0;
  for (auto& i : receivers) {
    *receiver_list[idx++] = strdup(i.c_str());
  }
  *count = receivers.size();
  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_item_set_policy(noti_ex_item_h handle,
    int policy) {
  if (handle == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  Handle* p = static_cast<Handle*>(handle);
  p->Get()->SetPolicy(policy);
  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_item_get_policy(noti_ex_item_h handle,
    int *policy) {
  if (handle == nullptr || policy == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  Handle* p = static_cast<Handle*>(handle);
  *policy = p->Get()->GetPolicy();
  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_item_get_channel(noti_ex_item_h handle,
    char **channel) {
  if (handle == nullptr || channel == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  Handle* p = static_cast<Handle*>(handle);
  if (!p->Get()->GetChannel().empty())
    *channel = strdup(p->Get()->GetChannel().c_str());
  else
    *channel = nullptr;

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_item_set_channel(noti_ex_item_h handle,
    const char *channel) {
  if (handle == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  Handle* p = static_cast<Handle*>(handle);
  p->Get()->SetChannel(channel);
  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_item_set_led_info(noti_ex_item_h handle,
    noti_ex_led_info_h led) {
  if (handle == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  Handle* p = static_cast<Handle*>(handle);
  LEDInfo* led_info = static_cast<LEDInfo*>(led);
  p->Get()->SetLEDInfo(shared_ptr<LEDInfo>(led_info));
  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_item_get_led_info(noti_ex_item_h handle,
    noti_ex_led_info_h *led) {
  if (handle == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  Handle* p = static_cast<Handle*>(handle);
  if (p->Get()->GetLEDInfo() != nullptr)
    *led = static_cast<noti_ex_led_info_h>(p->Get()->GetLEDInfo().get());
  else
    *led = nullptr;
  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_item_set_sound_path(noti_ex_item_h handle,
    const char *path) {
  if (handle == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  Handle* p = static_cast<Handle*>(handle);
  if (path == nullptr)
    p->Get()->SetSoundPath("");
  else
    p->Get()->SetSoundPath(path);
  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_item_set_vibration_path(noti_ex_item_h handle,
    const char *path) {
  if (handle == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  Handle* p = static_cast<Handle*>(handle);
  if (path == nullptr)
    p->Get()->SetVibrationPath("");
  else
    p->Get()->SetVibrationPath(path);
  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_item_get_sound_path(noti_ex_item_h handle,
    char **path) {
  if (handle == nullptr || path == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  Handle* p = static_cast<Handle*>(handle);
  if (p->Get()->GetSoundPath().empty())
    *path = nullptr;
  else
    *path = strdup(p->Get()->GetSoundPath().c_str());
  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_item_get_vibration_path(noti_ex_item_h handle,
    char **path) {
  if (handle == nullptr || path == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  Handle* p = static_cast<Handle*>(handle);
  if (p->Get()->GetVibrationPath().empty())
    *path = nullptr;
  else
    *path = strdup(p->Get()->GetVibrationPath().c_str());
  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_item_get_info(noti_ex_item_h handle,
    noti_ex_item_info_h *info) {
  if (handle == nullptr || info == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  Handle* p = static_cast<Handle*>(handle);
  if (p->Get()->GetInfo() == nullptr)
    *info = nullptr;
  else
    *info = static_cast<noti_ex_item_info_h>(p->Get()->GetInfo().get());
  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_item_get_sender_app_id(noti_ex_item_h handle,
    char **id) {
  if (handle == nullptr || id == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  Handle* p = static_cast<Handle*>(handle);
  if (p->Get()->GetSenderAppId().empty())
    *id = nullptr;
  else
    *id = strdup(p->Get()->GetSenderAppId().c_str());
  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_item_get_tag(noti_ex_item_h handle,
    char **tag) {
  if (handle == nullptr || tag == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  Handle* p = static_cast<Handle*>(handle);
  if (p->Get()->GetTag().empty())
    *tag = nullptr;
  else
    *tag = strdup(p->Get()->GetTag().c_str());
  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_item_set_tag(noti_ex_item_h handle,
    const char *tag) {
  if (handle == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  Handle* p = static_cast<Handle*>(handle);
  if (tag == nullptr)
    p->Get()->SetTag("");
  else
    p->Get()->SetTag(tag);
  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_manager_create(noti_ex_manager_h *handle,
    const char *receiver_group, noti_ex_manager_events_s event_callbacks,
    void *data) {
  if (handle == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  string receiver_group_str = "";
  if (receiver_group)
    receiver_group_str = string(receiver_group);

  ManagerStub* stub = new (std::nothrow) ManagerStub(
      unique_ptr<DBusSender>(new DBusSender(Reporter::GetPath())),
      unique_ptr<DBusEventListener>(new DBusEventListener(Manager::GetPath())),
      receiver_group_str);
  if (stub == nullptr) {
    LOGE("Fail to create manager");
    return NOTI_EX_ERROR_IO_ERROR;
  }
  stub->SetManagerCallbackInfo(unique_ptr<ManagerCallbackInfo>(
      new ManagerCallbackInfo(event_callbacks, data)));
  *handle = static_cast<noti_ex_manager_h>(stub);

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_manager_destroy(noti_ex_manager_h handle) {
  if (handle == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }
  ManagerStub* stub = static_cast<ManagerStub*>(handle);
  delete stub;
  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_manager_get(noti_ex_manager_h handle,
    noti_ex_item_h **items, int *count) {
  if (handle == nullptr || items == nullptr || count == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  try {
    ManagerStub* stub = static_cast<ManagerStub*>(handle);
    list<unique_ptr<item::AbstractItem>> item_list = stub->Get();
    if (item_list.size() == 0) {
      *items = nullptr;
      *count = 0;
      return NOTI_EX_ERROR_NONE;
    }
    *items = (noti_ex_item_h*)calloc(item_list.size(), sizeof(noti_ex_item_h));
    if (*items == nullptr) {
      LOGE("Fail to create items");
      return NOTI_EX_ERROR_OUT_OF_MEMORY;
    }

    int idx = 0;
    for (auto& i : item_list) {
      *items[idx++] = new Handle(move(i));
    }
    *count = item_list.size();
  } catch (Exception &ex) {
    LOGE("%s %d", ex.what(), ex.GetErrorCode());
    return NOTI_EX_ERROR_IO_ERROR;
  }
  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_manager_update(noti_ex_manager_h handle,
    noti_ex_item_h noti, int *request_id) {
  if (handle == nullptr || noti == nullptr || request_id == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }
  try {
    ManagerStub* stub = static_cast<ManagerStub*>(handle);
    Handle* sp = static_cast<Handle*>(noti);
    if (sp->GetPtr().get() == nullptr) {
      LOGE("Invalid noti reference can not be sended");
      return NOTI_EX_ERROR_INVALID_PARAMETER;
    } else {
      *request_id = stub->Update(sp->GetPtr());
    }
  } catch (Exception &ex) {
    LOGE("%s %d", ex.what(), ex.GetErrorCode());
    return NOTI_EX_ERROR_IO_ERROR;
  }
  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_manager_delete(noti_ex_manager_h handle,
    noti_ex_item_h noti, int *request_id) {
  if (handle == nullptr || noti == nullptr || request_id == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }
  try {
    ManagerStub* stub = static_cast<ManagerStub*>(handle);
    Handle* item = static_cast<Handle*>(noti);
    if (item->GetPtr().get() == nullptr) {
      LOGE("Invalid noti reference can not be sended");
      return NOTI_EX_ERROR_INVALID_PARAMETER;
    } else {
      *request_id = stub->Delete(item->GetPtr());
    }
  } catch (Exception &ex) {
    LOGE("%s %d", ex.what(), ex.GetErrorCode());
    return NOTI_EX_ERROR_IO_ERROR;
  }
  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_manager_delete_all(noti_ex_manager_h handle,
    int *request_id) {
  if (handle == nullptr || request_id == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }
  try {
    ManagerStub* stub = static_cast<ManagerStub*>(handle);
    *request_id = stub->DeleteAll();
  } catch (Exception &ex) {
    LOGE("%s %d", ex.what(), ex.GetErrorCode());
    return NOTI_EX_ERROR_IO_ERROR;
  }
  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_manager_hide(noti_ex_manager_h handle,
    noti_ex_item_h noti, int *request_id) {
  if (handle == nullptr || noti == nullptr || request_id == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }
  try {
    ManagerStub* stub = static_cast<ManagerStub*>(handle);
    Handle* item = static_cast<Handle*>(noti);
    if (item->GetPtr().get() == nullptr) {
      LOGE("Invalid noti reference can not be sended");
      return NOTI_EX_ERROR_INVALID_PARAMETER;
    } else {
      *request_id = stub->Hide(item->GetPtr());
    }
  } catch (Exception &ex) {
    LOGE("%s %d", ex.what(), ex.GetErrorCode());
    return NOTI_EX_ERROR_IO_ERROR;
  }
  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_manager_find_by_root_id(
    noti_ex_manager_h handle, const char *root_id, noti_ex_item_h *item) {
  if (handle == nullptr || root_id == nullptr || item == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }
  try {
    ManagerStub* stub = static_cast<ManagerStub*>(handle);
    *item = new Handle(stub->FindByRootID(root_id));
  } catch (Exception &ex) {
    LOGE("%s %d", ex.what(), ex.GetErrorCode());
    return NOTI_EX_ERROR_IO_ERROR;
  }
  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_manager_send_error(noti_ex_manager_h handle,
    noti_ex_event_info_h info, noti_ex_error_e error) {
  if (handle == nullptr || info == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }
  try {
    ManagerStub* stub = static_cast<ManagerStub*>(handle);
    IEventInfo* c_info = static_cast<IEventInfo*>(info);
    stub->SendError(static_cast<const IEventInfo&>(*c_info),
        static_cast<NotificationError>(error));
  } catch (Exception &ex) {
    LOGE("%s %d", ex.what(), ex.GetErrorCode());
    return NOTI_EX_ERROR_IO_ERROR;
  }
  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_manager_get_notification_count(
    noti_ex_manager_h handle, int *cnt) {

  if (handle == nullptr || cnt == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }
  try {
    ManagerStub* stub = static_cast<ManagerStub*>(handle);
    *cnt = stub->GetCount();
  } catch (Exception &ex) {
    LOGE("%s %d", ex.what(), ex.GetErrorCode());
    return NOTI_EX_ERROR_IO_ERROR;
  }
  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_item_progress_create(noti_ex_item_h *handle,
    const char *id, float min, float current, float max) {
  ProgressItem* p;

  if (handle == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  if (id)
    p = new (std::nothrow) ProgressItem(id, min, current, max);
  else
    p = new (std::nothrow) ProgressItem(min, current, max);

  if (p == nullptr) {
    LOGE("Out-of-memory");
    return NOTI_EX_ERROR_OUT_OF_MEMORY;
  }

  *handle = new Handle(shared_ptr<AbstractItem>(p));

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_item_progress_get_current(
    noti_ex_item_h handle, float *current) {
  if (handle == nullptr || current == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  Handle *h = static_cast<Handle*>(handle);
  if (!h->IsValidType(AbstractItem::Progress)) {
    LOGE("Invalid handle type");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }
  ProgressItem* p = static_cast<ProgressItem*>(h->Get());
  *current = p->GetCurrent();

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_item_progress_set_current(
    noti_ex_item_h handle, float current) {
  if (handle == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  Handle *h = static_cast<Handle*>(handle);
  if (!h->IsValidType(AbstractItem::Progress)) {
    LOGE("Invalid handle type");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }
  ProgressItem* p = static_cast<ProgressItem*>(h->Get());
  p->SetCurrent(current);

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_item_progress_get_min(noti_ex_item_h handle,
    float *min) {
  if (handle == nullptr || min == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  Handle *h = static_cast<Handle*>(handle);
  if (!h->IsValidType(AbstractItem::Progress)) {
    LOGE("Invalid handle type");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }
  ProgressItem* p = static_cast<ProgressItem*>(h->Get());
  *min = p->GetMin();

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_item_progress_get_max(noti_ex_item_h handle,
    float *max) {
  if (handle == nullptr || max == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  Handle *h = static_cast<Handle*>(handle);
  if (!h->IsValidType(AbstractItem::Progress)) {
    LOGE("Invalid handle type");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }
  ProgressItem* p = static_cast<ProgressItem*>(h->Get());
  *max = p->GetMax();

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_reporter_create(noti_ex_reporter_h *handle,
    noti_ex_reporter_events_s event_callbacks, void *data) {
  if (handle == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  ReporterStub* stub = new (std::nothrow) ReporterStub(
      unique_ptr<DBusSender>(new DBusSender(Manager::GetPath())),
      unique_ptr<DBusEventListener>(new DBusEventListener(Reporter::GetPath())));
  if (stub == nullptr) {
    LOGE("Fail to create manager");
    return NOTI_EX_ERROR_IO_ERROR;
  }
  stub->SetReporterCallbackInfo(unique_ptr<ReporterCallbackInfo>(
      new ReporterCallbackInfo(event_callbacks, data)));

  *handle = static_cast<noti_ex_reporter_h>(stub);

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_reporter_destroy(noti_ex_reporter_h handle) {
  if (handle == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }
  ReporterStub* stub = static_cast<ReporterStub*>(handle);
  delete stub;
  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_reporter_send_error(noti_ex_reporter_h handle,
    noti_ex_event_info_h info, noti_ex_error_e error) {
  if (handle == nullptr || info == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }
  try {
    ReporterStub* stub = static_cast<ReporterStub*>(handle);
    IEventInfo* c_info = static_cast<IEventInfo*>(info);
    stub->SendError(static_cast<const IEventInfo&>(*c_info),
        static_cast<NotificationError>(error));
  } catch (Exception &ex) {
    LOGE("%s %d", ex.what(), ex.GetErrorCode());
    return NOTI_EX_ERROR_IO_ERROR;
  }
  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_reporter_post(noti_ex_reporter_h handle,
    noti_ex_item_h noti, int *request_id) {
  if (handle == nullptr || noti == nullptr || request_id == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }
  try {
    ReporterStub* stub = static_cast<ReporterStub*>(handle);
    Handle* h = static_cast<Handle*>(noti);
    if (h->GetPtr().get() == nullptr) {
      LOGE("Invalid noti reference can not be sended");
      return NOTI_EX_ERROR_INVALID_PARAMETER;
    } else {
      *request_id = stub->Post(h->GetPtr());
    }
  } catch (Exception &ex) {
    LOGE("%s %d", ex.what(), ex.GetErrorCode());
    return NOTI_EX_ERROR_IO_ERROR;
  }
  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_reporter_post_list(noti_ex_reporter_h handle,
    noti_ex_item_h *noti_list, int count, int *request_id) {

  if (handle == nullptr || noti_list == nullptr || request_id == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }
  try {
    ReporterStub* stub = static_cast<ReporterStub*>(handle);
    list<shared_ptr<item::AbstractItem>> notiList;
    for (int i = 0; i < count; i++) {
      Handle* item = static_cast<Handle*>(noti_list[i]);
      notiList.push_back(item->GetPtr());
    }
    *request_id = stub->Post(notiList);
  } catch (Exception &ex) {
    LOGE("%s %d", ex.what(), ex.GetErrorCode());
    return NOTI_EX_ERROR_IO_ERROR;
  }
  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_reporter_update(noti_ex_reporter_h handle,
    noti_ex_item_h noti, int *request_id) {
  if (handle == nullptr || noti == nullptr || request_id == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }
  try {
    ReporterStub* stub = static_cast<ReporterStub*>(handle);
    Handle* item = static_cast<Handle*>(noti);
    if (item->GetPtr().get() == nullptr) {
      LOGE("Invalid noti reference can not be sended");
      return NOTI_EX_ERROR_INVALID_PARAMETER;
    } else {
      *request_id = stub->Update(item->GetPtr());
    }
  } catch (Exception &ex) {
    LOGE("%s %d", ex.what(), ex.GetErrorCode());
    return NOTI_EX_ERROR_IO_ERROR;
  }
  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_reporter_delete(noti_ex_reporter_h handle,
    noti_ex_item_h noti, int *request_id) {
  if (handle == nullptr || noti == nullptr || request_id == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }
  try {
    ReporterStub* stub = static_cast<ReporterStub*>(handle);
    Handle* item = static_cast<Handle*>(noti);
    if (item->GetPtr().get() == nullptr) {
      LOGE("Invalid noti reference can not be sended");
      return NOTI_EX_ERROR_INVALID_PARAMETER;
    } else {
      *request_id = stub->Delete(item->GetPtr());
    }
  } catch (Exception &ex) {
    LOGE("%s %d", ex.what(), ex.GetErrorCode());
    return NOTI_EX_ERROR_IO_ERROR;
  }
  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_reporter_delete_all(
    noti_ex_reporter_h handle, int *request_id) {
  if (handle == nullptr || request_id == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }
  try {
    ReporterStub* stub = static_cast<ReporterStub*>(handle);
    *request_id = stub->DeleteAll();
  } catch (Exception &ex) {
    LOGE("%s %d", ex.what(), ex.GetErrorCode());
    return NOTI_EX_ERROR_IO_ERROR;
  }
  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_reporter_find_by_root_id(
    noti_ex_reporter_h handle, const char *root_id, noti_ex_item_h *item) {
  if (handle == nullptr || root_id == nullptr || item == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }
  try {
    ReporterStub* stub = static_cast<ReporterStub*>(handle);
    *item = new Handle(stub->FindByRootID(root_id));
  } catch (Exception &ex) {
    LOGE("%s %d", ex.what(), ex.GetErrorCode());
    return NOTI_EX_ERROR_IO_ERROR;
  }
  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_item_text_create(noti_ex_item_h *handle,
    const char *id, const char *text, const char *hyperlink) {
  if (handle == nullptr || text == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  TextItem* p;

  if (hyperlink)
    p = new (std::nothrow) TextItem(id, std::string(text),
                std::string(hyperlink));
  else
    p = new (std::nothrow) TextItem(id, std::string(text));

  if (p == nullptr) {
    LOGE("Out-of-memory");
    return NOTI_EX_ERROR_OUT_OF_MEMORY;
  }

  *handle = new Handle(shared_ptr<AbstractItem>(p));

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_item_text_set_contents(noti_ex_item_h handle,
    const char *contents) {
  if (handle == nullptr || contents == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  Handle* p = static_cast<Handle*>(handle);
  if (!p->IsValidType(AbstractItem::Text)) {
    LOGE("Invalid handle type");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }
  TextItem* ti = static_cast<TextItem*>(p->Get());
  ti->SetContents(std::string(contents));

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_item_text_get_contents(noti_ex_item_h handle,
    char **contents) {
  if (handle == nullptr || contents == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  Handle* p = static_cast<Handle*>(handle);
  if (!p->IsValidType(AbstractItem::Text)) {
    LOGE("Invalid handle type");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }
  TextItem* ti = static_cast<TextItem*>(p->Get());
  if (!ti->GetContents().empty()) {
    *contents = strdup(ti->GetContents().c_str());
    if (*contents == nullptr) {
      LOGE("Out-of-memory");
      return NOTI_EX_ERROR_OUT_OF_MEMORY;
    }
  }

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_item_text_get_hyperlink(
    noti_ex_item_h handle, char **hyper_link) {
  if (handle == nullptr || hyper_link == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  Handle* p = static_cast<Handle*>(handle);
  if (!p->IsValidType(AbstractItem::Text)) {
    LOGE("Invalid handle type");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }
  TextItem* ti = static_cast<TextItem*>(p->Get());
  if (!ti->GetHyperLink().empty()) {
    *hyper_link = strdup(ti->GetHyperLink().c_str());
    if (*hyper_link == nullptr) {
      LOGE("Out-of-memory");
      return NOTI_EX_ERROR_OUT_OF_MEMORY;
    }
  }

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_item_time_create(noti_ex_item_h *handle,
    const char *id, time_t time) {
  TimeItem* p;

  if (handle == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  if (time) {
    if (id)
      p = new (std::nothrow) TimeItem(id, time);
    else
      p = new (std::nothrow) TimeItem(time);
  } else {
      p = new (std::nothrow) TimeItem();
  }

  if (p == nullptr) {
    LOGE("Out-of-memory");
    return NOTI_EX_ERROR_OUT_OF_MEMORY;
  }

  *handle = new Handle(shared_ptr<AbstractItem>(p));

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_item_time_get_time(noti_ex_item_h handle,
    time_t *time) {
  if (handle == nullptr || time == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }
  Handle* h = static_cast<Handle*>(handle);
  if (!h->IsValidType(AbstractItem::Time)) {
    LOGE("Invalid handle type");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }
  TimeItem* p = static_cast<TimeItem*>(h->Get());
  *time = p->GetTime();

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_action_visibility_create(
    noti_ex_action_h *handle, const char *extra) {
  if (handle == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  string extra_str = "";
  if (extra != NULL)
    extra_str = string(extra);

  auto* p = new (std::nothrow) VisibilityAction(extra_str);
  if (p == nullptr) {
    LOGE("Out-of-memory");
    return NOTI_EX_ERROR_OUT_OF_MEMORY;
  }

  *handle = p;

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_action_visibility_set(noti_ex_action_h handle,
    const char *id, bool visible) {
  if (handle == nullptr || id == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  VisibilityAction* p = static_cast<VisibilityAction*>(handle);
  p->SetVisibility(id, visible);

  return NOTI_EX_ERROR_NONE;
}
