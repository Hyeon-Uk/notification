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

#ifndef NOTIFICATION_EX_REPORTER_H_
#define NOTIFICATION_EX_REPORTER_H_

#include <string>
#include <list>
#include <memory>

#include "notification-ex/common.h"
#include "notification-ex/ievent_info.h"
#include "notification-ex/event_observer_interface.h"
#include "notification-ex/event_sender_interface.h"
#include "notification-ex/event_listener_interface.h"
#include "notification-ex/abstract_item.h"

#ifndef EXPORT_API
#define EXPORT_API __attribute__((visibility("default")))
#endif

namespace notification {

/* LCOV_EXCL_START */
class EXPORT_API Reporter : public IEventObserver {
 public:
  Reporter(std::unique_ptr<IEventSender> sender,
        std::unique_ptr<IEventListener> listener);
  virtual ~Reporter();

  int SendEvent(const IEventInfo& info,
      std::list<std::shared_ptr<item::AbstractItem>> notiList);
  void SendError(const IEventInfo& info, NotificationError error);
  int Post(std::shared_ptr<item::AbstractItem> noti);
  int Post(std::list<std::shared_ptr<item::AbstractItem>> notiList);
  int Update(std::shared_ptr<item::AbstractItem> noti);
  int Update(std::list<std::shared_ptr<item::AbstractItem>> notiList);
  int Delete(std::shared_ptr<item::AbstractItem> noti);
  int Delete(std::list<std::shared_ptr<item::AbstractItem>> notiList);
  int DeleteAll();
  int DeleteByChannel(std::string channel);
  std::unique_ptr<item::AbstractItem> FindByRootID(std::string id);
  std::list<std::unique_ptr<item::AbstractItem>> FindByChannel(std::string channel);
  std::list<std::unique_ptr<item::AbstractItem>> FindAll();
  int GetCount(std::string channel = "") const;
  virtual void OnEvent(const IEventInfo& info,
      std::list<std::shared_ptr<item::AbstractItem>> notiList);
  virtual std::list<std::shared_ptr<item::AbstractItem>> OnRequestEvent(
      const IEventInfo& info);
  virtual void OnError(NotificationError error, int requestId);
  void OnEvent(const IEventInfo& info,
      std::list<tizen_base::Bundle> serialized) override;
  std::list<tizen_base::Bundle> OnRequest(const IEventInfo& info) override;
  int OnRequestNumber(const IEventInfo& info) override;
  static std::string GetPath();
  virtual void OnRegister(const IEventInfo& info);

 private:
  class Impl;
  std::unique_ptr<Impl> impl_;
};
/* LCOV_EXCL_STOP */

}  // namespace notification

#endif  // NOTIFICATION_EX_REPORTER_H_
