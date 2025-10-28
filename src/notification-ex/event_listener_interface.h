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

#ifndef NOTIFICATION_EX_EVENT_LISTENER_INTERFACE_H_
#define NOTIFICATION_EX_EVENT_LISTENER_INTERFACE_H_

#include <list>

#include "notification-ex/event_observer_interface.h"
#include "notification-ex/ievent_info.h"

#ifndef EXPORT_API
#define EXPORT_API __attribute__((visibility("default")))
#endif

namespace notification {

/* LCOV_EXCL_START */
class EXPORT_API IEventListener {
 public:
  virtual ~IEventListener() = default;
  virtual void RegisterObserver(IEventObserver* observer) = 0;
  virtual void UnRegisterObserver(IEventObserver* observer) = 0;
  virtual void NotifyObserver(const IEventInfo& info,
      std::list<tizen_base::Bundle> serialized) = 0;
  virtual std::list<tizen_base::Bundle> NotifyObserver(const IEventInfo& info) = 0;
  virtual int NotifyNumberRequest(const IEventInfo& info) = 0;
};
/* LCOV_EXCL_STOP */

}  // namespace notification

#endif  // NOTIFICATION_EX_EVENT_LISTENER_INTERFACE_H_
