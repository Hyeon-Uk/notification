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

#include "notification-ex/iitem_info.h"

#ifndef EXPORT_API
#define EXPORT_API __attribute__((visibility("default")))
#endif

namespace notification {
namespace item {

class EXPORT_API IItemInfoInternal : public IItemInfo {
 public:
  virtual ~IItemInfoInternal() = default;
  virtual void SetHideTime(int hide_time) = 0;
  virtual int GetHideTime() const = 0;
  virtual void SetDeleteTime(int delete_time)  = 0;
  virtual int GetDeleteTime() const = 0;
  virtual void AddHideViewer(std::string appid) = 0;
  virtual void RemoveHideViewer(std::string appid) = 0;
  virtual std::list<std::string> GetHideViewerList() const = 0;
};

}  // namespace item
}  // namespace notification
#endif  // NOTIFICATION_EX_IITEM_INFO_INTERNAL_H_
