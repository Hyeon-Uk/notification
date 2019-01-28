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

#ifndef NOTIFICATION_EX_MANAGER_H_
#define NOTIFICATION_EX_MANAGER_H_

#include <string>
#include <list>

#include "notification-ex/abstract_item.h"

#ifndef EXPORT_API
#define EXPORT_API __attribute__((visibility("default")))
#endif

namespace notification {

class EXPORT_API Manager : public IEventObserver {
 public:
  Manager(IEventsSender* sender, IEventsListener* listener);
  virtual ~Manager();

  void Update(std::shared_ptr<item::AbstractItem> noti);
  void Remove(std::shared_ptr<item::AbstractItem> noti);
  void Hide(std::shared_ptr<item::AbstractItem> noti);
  std::shared_ptr<item::AbstractItem> FindByRootID(std::string id);
  void SendEvent(EventInfo info, std::shared_ptr<item::AbstractItem> noti);
  void OnEvent(EventInfo info, std::shared_ptr<AbstractItem> notiList) override;

 protected:
  virtual void OnAdd(std::list<std::shared_ptr<item::AbstractItem>> addedItem);
  virtual void OnUpdate(shared_ptr<item::AbstractItem> updatedItem);
  virtual void OnDelete(shared_ptr<item::AbstractItem> deletedItem);

 private:
  IEventsSender* sender_;
  IEventsListener* receiver_;
};

}  // namespace notification

#endif  // NOTIFICATION_EX_MANAGER_H_
