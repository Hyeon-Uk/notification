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

#ifndef NOTIFICATION_EX_IEVENT_INFO_INTERNAL_H_
#define NOTIFICATION_EX_IEVENT_INFO_INTERNAL_H_

#include "notification-ex/common.h"
#include "notification-ex/ievent_info.h"

namespace notification {

/* LCOV_EXCL_START */
class IEventInfoInternal : public IEventInfo {
 public:
  virtual ~IEventInfoInternal() = default;
  virtual uid_t GetUid() const = 0;
  virtual void SetUid(uid_t uid) = 0;
  virtual uid_t GetValidatedUid() const = 0;
  virtual void SetValidatedUid(uid_t uid) = 0;
  virtual void SetValidatedOwner(std::string owner) = 0;
  virtual std::string GetValidatedOwner() const = 0;
  virtual NotificationError GetError() const = 0;
  virtual void SetError(NotificationError error) = 0;
  virtual void SetEventType(int type) = 0;
};
/* LCOV_EXCL_STOP */

}  // namespace notification
#endif  // NOTIFICATION_EX_IEVENT_INFO_INTERNAL_H_
