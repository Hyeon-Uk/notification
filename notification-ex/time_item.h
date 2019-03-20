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

#ifndef NOTIFICATION_EX_TIME_ITEM_H_
#define NOTIFICATION_EX_TIME_ITEM_H_

#include <time.h>

#include <string>
#include <memory>
#include <list>

#include "notification-ex/abstract_item.h"

namespace notification {
namespace item {

class EXPORT_API TimeItem : public AbstractItem {
 public:
  TimeItem(
    std::shared_ptr<AbstractAction> action = std::shared_ptr<AbstractAction>({}));
  TimeItem(time_t time,
    std::shared_ptr<AbstractAction> action = std::shared_ptr<AbstractAction>({}));
  TimeItem(std::string id, time_t time,
    std::shared_ptr<AbstractAction> action = std::shared_ptr<AbstractAction>({}));

  virtual ~TimeItem();
  int GetType() const override;
  Bundle Serialize() const override;
  void Deserialize(Bundle b) override;
  AbstractItem& FindByID(std::string id) override;
  time_t GetTime() const;

 private:
  class Impl;
  std::unique_ptr<Impl> impl_;
};  // class TimeItem

}  // namespace item
}  // namespace notification
#endif  // NOTIFICATION_EX_TIME_ITEM_H_
