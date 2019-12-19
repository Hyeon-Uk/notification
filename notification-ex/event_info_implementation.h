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

#ifndef NOTIFICATION_EX_EVENT_INFO_IMPLEMENTATION_H_
#define NOTIFICATION_EX_EVENT_INFO_IMPLEMENTATION_H_

#include <string>
#include <memory>
#include <list>

#include <sys/types.h>

#include "notification-ex/dbus_sender.h"
#include "notification-ex/ievent_info.h"
#include "notification-ex/common.h"

namespace notification {

class EventInfo::Impl {
 public:
  virtual ~Impl();

 private:
  friend class EventInfo;
  Impl(EventInfo* parent, int type, std::string owner, std::string channel,
    std::string item_id);

 private:
  int type_;
  std::string owner_;
  std::string validated_owner_;
  std::string channel_;
  std::string item_id_;
  uid_t uid_;
  uid_t validated_uid_;
  int request_id_;
  NotificationError error_;
  EventInfo* parent_;
};

}  // namespace notification
#endif  // NOTIFICATION_EX_EVENT_INFO_IMPLEMENTATION_H_
