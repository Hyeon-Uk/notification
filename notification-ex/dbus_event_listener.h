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

#ifndef NOTIFICATION_EX_DBUS_EVENT_LISTENER_H_
#define NOTIFICATION_EX_DBUS_EVENT_LISTENER_H_

#include <string>
#include <list>

#include "notification-ex/event_listener_interface.h"

namespace notification {

class EXPORT_API DBusEventListener : public IEventListener {
 public:
  DBusEventListener(std::string path);
  ~DBusEventListener();

  void RegisterObserver(IEventObserver* observer) override;
  void UnRegisterObserver(IEventObserver* observer) override;
  void NotifyObserver(
      const IEventInfo& info, std::list<tizen_base::Bundle> serialized) override;
  std::list<tizen_base::Bundle> NotifyObserver(const IEventInfo& info) override;
  int NotifyNumberRequest(const IEventInfo& info) override;

 private:
  class Impl;
  std::unique_ptr<Impl> impl_;
};

}  // namespace notification

#endif  // NOTIFICATION_EX_DBUS_EVENT_LISTENER_H_
