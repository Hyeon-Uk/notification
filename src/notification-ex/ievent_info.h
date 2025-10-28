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

#ifndef NOTIFICATION_EX_IEVENT_INFO_H_
#define NOTIFICATION_EX_IEVENT_INFO_H_

#include <bundle_cpp.h>

#include <string>

namespace notification {

#ifndef EXPORT_API
#define EXPORT_API __attribute__((visibility("default")))
#endif

/* LCOV_EXCL_START */
class EXPORT_API IEventInfo {
 public:
  enum EventType {
    Post,
    Update,
    Delete,
    Get,
    Error,
    DeleteAll,
    Count,
    Register,
    Unregister,
    Custom = 100
  };
  virtual ~IEventInfo() = default;
  virtual int GetEventType() const = 0;
  virtual std::string GetOwner() const = 0;
  virtual std::string GetChannel() const = 0;
  virtual std::string GetItemId() const = 0;
  virtual int GetRequestId() const = 0;
  virtual tizen_base::Bundle Serialize() const = 0;
};
/* LCOV_EXCL_STOP */

}  // namespace notification
#endif  // NOTIFICATION_EX_IEVENT_INFO_H_
