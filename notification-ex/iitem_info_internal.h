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

#ifndef NOTIFICATION_EX_IITEM_INFO_INTERNAL_H_
#define NOTIFICATION_EX_IITEM_INFO_INTERNAL_H_

#include <string>
#include <list>

#include "notification-ex/iitem_info.h"

#ifndef EXPORT_API
#define EXPORT_API __attribute__((visibility("default")))
#endif

namespace notification {
namespace item {

/* LCOV_EXCL_START */
class EXPORT_API IItemInfoInternal : public IItemInfo {
 public:
  virtual ~IItemInfoInternal() = default;
  virtual uid_t GetUid() const = 0;
  virtual void SetUid(uid_t uid) = 0;
  virtual int64_t GetPrivateId() const = 0;
  virtual void SetPrivateId(int64_t private_id) = 0;
  virtual int GetVersion() const = 0;
  virtual void SetVersion(int ver) = 0;
  virtual void AddHideViewer(std::string appid) = 0;
  virtual std::list<std::string> GetHideViewerList() const = 0;
  virtual void SetTime(time_t time) = 0;
  virtual bool CanReceive(std::string receiver_group) const = 0;
};
/* LCOV_EXCL_STOP */

}  // namespace item
}  // namespace notification
#endif  // NOTIFICATION_EX_IITEM_INFO_INTERNAL_H_
