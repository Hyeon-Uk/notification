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

#ifndef NOTIFICATION_EX_ITEM_INFO_INTERNAL_H_
#define NOTIFICATION_EX_ITEM_INFO_INTERNAL_H_

#include <string>
#include <list>

#include "notification-ex/abstract_item_implementation.h"
#include "notification-ex/iitem_info_internal.h"

namespace notification {
namespace item {

class AbstractItem::Impl::ItemInfo : public IItemInfoInternal {
 public:
  ItemInfo(AbstractItem::Impl* impl);
  virtual ~ItemInfo() = default;
  int64_t GetPrivateId() const override;
  void SetPrivateId(int64_t private_id) override;
  uid_t GetUid() const override;
  void SetUid(uid_t uid) override;
  time_t GetTime() const override;
  void SetTime(time_t time) override;
  int GetVersion() const override;
  void SetVersion(int ver) override;
  void SetHideTime(int hide_time) override;
  int GetHideTime() const override;
  void SetDeleteTime(int delete_time) override;
  int GetDeleteTime() const override;
  void AddHideViewer(std::string appid) override;
  std::list<std::string> GetHideViewerList() const override;
  bool CanReceive(std::string receiver_group) const override;

 private:
  AbstractItem::Impl* impl_;
};

}  // namespace item
}  // namespace notification
#endif  // NOTIFICATION_EX_ITEM_INFO_INTERNAL_H_
