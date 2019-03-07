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

#ifndef NOTIFICATION_EX_EVENT_INFO_H_
#define NOTIFICATION_EX_EVENT_INFO_H_

#include <string>
#include <list>

#include "notification-ex/ex_bundle.h"

#ifndef EXPORT_API
#define EXPORT_API __attribute__((visibility("default")))
#endif

namespace notification {

class EXPORT_API EventInfo {
 public:
  enum EventType {
    Post,
    Update,
    Delete,
    Get,
  };
  EventInfo(EventType type, std::string owner,
      std::string channel = "",
      std::string item_id = "",
      std::string tag = "");
  EventInfo(Bundle serialized);
  ~EventInfo();

  static std::string GetString(EventType type);
  EventType GetEventType() const;
  std::string GetOwner() const;
  std::string GetChannel() const;
  std::string GetItemId() const;
  std::string GetTag() const;
  Bundle Serialize() const;

 private:
  class Impl;
  std::unique_ptr<Impl> impl_;
};

}  // namespace notification

#endif  // NOTIFICATION_EX_EVENT_INFO_H_
