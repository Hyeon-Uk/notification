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

/**
 * @brief The class for TimeItem type notification.
 * @details The class to make the notification with time.
 * @since_tizen 5.5
 */
class EXPORT_API TimeItem : public AbstractItem {
 public:
  /**
   * @brief Constructor
   * @since_tizen 5.5
   * @param[in] action The action for TimeItem
   */
  TimeItem(
    std::shared_ptr<AbstractAction> action = std::shared_ptr<AbstractAction>({}));

  /**
   * @brief Constructor
   * @since_tizen 5.5
   * @param[in] time The time data of TimeItem
   * @param[in] action The action for TimeItem
   */
  TimeItem(time_t time,
    std::shared_ptr<AbstractAction> action = std::shared_ptr<AbstractAction>({}));

  /**
   * @brief Constructor
   * @since_tizen 5.5
   * @param[in] id The TimeItem id
   * @param[in] time The time data of TimeItem
   * @param[in] action The action for TimeItem
   */
  TimeItem(std::string id, time_t time,
    std::shared_ptr<AbstractAction> action = std::shared_ptr<AbstractAction>({}));

  /**
   * @brief Destructor
   * @since_tizen 5.5
   */
  virtual ~TimeItem();

  /**
   * @brief Gets the type of TimeItem.
   * @since_tizen 5.5
   * @return AbstractItem::Type::Time
   */
  int GetType() const override;

  /**
   * @brief Serialize the data of TimeItem.
   * @since_tizen 5.5
   * @return Bundle type data
   */
  tizen_base::Bundle Serialize() const override;

  /**
   * @brief Deserialize the serialized data.
   * @since_tizen 5.5
   * @param[in] b The serialized Bundle data
   */
  void Deserialize(tizen_base::Bundle b) override;

  /**
   * @brief Checks the item type exist in this notification.
   * @since_tizen 5.5
   * @param[in] type notification item type
   * @return true if the item type exists
   */
  bool IsItemTypeExist(int type) override;

  /**
   * @brief Gets the time state of TimeItem.
   * @since_tizen 5.5
   * @return The time state of TimeItem.
   */
  time_t GetTime() const;
  void SetTime(time_t time);

 private:
  class Impl;
  std::unique_ptr<Impl> impl_;
};  // class TimeItem

}  // namespace item
}  // namespace notification
#endif  // NOTIFICATION_EX_TIME_ITEM_H_
