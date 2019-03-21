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

#ifndef NOTIFICATION_EX_EVENT_INFO_INTERNAL_H_
#define NOTIFICATION_EX_EVENT_INFO_INTERNAL_H_

#include <string>
#include <list>

#include "notification-ex/common.h"
#include "notification-ex/ex_bundle.h"
#include "notification-ex/ievent_info_internal.h"

#ifndef EXPORT_API
#define EXPORT_API __attribute__((visibility("default")))
#endif

namespace notification {

class EventInfo : public IEventInfoInternal {
 public:
  EventInfo(IEventInfo::EventType type, std::string owner,
      std::string channel = "",
      std::string item_id = "",
      std::string tag = "");
  EventInfo(Bundle serialized);
  virtual ~EventInfo();
  uid_t GetUid() const override;
  void SetUid(uid_t uid) override;
  NotificationError GetError() const override;
  void SetError(NotificationError error) override;
  EventType GetEventType() const override;
  void SetEventType(EventType type) override;
  std::string GetOwner() const override;
  std::string GetChannel() const override;
  std::string GetItemId() const override;
  std::string GetTag() const override;
  int GetRequestId() const override;
  Bundle Serialize() const override;
  static std::string GetString(EventType type);

 private:
  class Impl;
  std::unique_ptr<Impl> impl_;
};

}  // namespace notification

#endif  // NOTIFICATION_EX_EVENT_INFO_INTERNAL_H_
