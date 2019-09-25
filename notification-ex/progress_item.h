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

#ifndef NOTIFICATION_EX_PROGRESS_ITEM_H_
#define NOTIFICATION_EX_PROGRESS_ITEM_H_

#include <string>
#include <memory>
#include <list>

#include "notification-ex/abstract_item.h"

namespace notification {
namespace item {

/**
 * @brief The class for ProgressItem type notification.
 * @details
 * @since_tizen 5.5
 */
class EXPORT_API ProgressItem : public AbstractItem {
 public:
  /**
   * @brief Constructor
   * @since_tizen 5.5
   * @param[in] min The minimum value of ProgressItem
   * @param[in] current The current value ProgressItem
   * @param[in] max The maximum value of ProgressItem
   * @param[in] action The action for ProgressItem
   */
  ProgressItem(float min, float current, float max,
      std::shared_ptr<AbstractAction> action = std::shared_ptr<AbstractAction>({}));

  /**
   * @brief Constructor
   * @since_tizen 5.5
   * @param[in] id The ProgressItem id
   * @param[in] min The minimum value of ProgressItem
   * @param[in] current The current value ProgressItem
   * @param[in] max The maximum value of ProgressItem
   * @param[in] action The action for ProgressItem
   */
  ProgressItem(std::string id, float min, float current, float max,
      std::shared_ptr<AbstractAction> action = std::shared_ptr<AbstractAction>({}));

  /**
   * @brief Destructor
   * @since_tizen 5.5
   */
  virtual ~ProgressItem();

 public:
  enum Type {
    Default,
    Time,
    Percent,
    Pending,
  };

  /**
   * @brief Serialize the data of ProgressItem.
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
   * @brief Gets the type of ProgressItem.
   * @since_tizen 5.5
   * @return AbstractItem::Type::Progress
   */
  int GetType() const override;

  /**
   * @brief Gets the current value of progress.
   * @since_tizen 5.5
   * @return The current value of progress
   */
  float GetCurrent() const;

  /**
   * @brief Sets the current value of progress.
   * @since_tizen 5.5
   * @param[in] current The current value of progress
   */
  void SetCurrent(float current);

  /**
   * @brief Gets the minimum value of progress.
   * @since_tizen 5.5
   * @return The minimum value of progress
   */
  float GetMin() const;

  /**
   * @brief Gets the maximum value of progress.
   * @since_tizen 5.5
   * @return The maximum value of progress
   */
  float GetMax() const;

  /**
   * @brief Sets the type of progress.
   * @since_tizen 5.5
   * @param[in] type The type of progress
   */
  void SetProgressType(ProgressItem::Type type);

  /**
   * @brief Gets the type of progress.
   * @since_tizen 5.5
   * @return The type of progress
   */
  Type GetProgressType() const;

   /**
   * @brief Sets the default unit of progress.
   * @details the unit is valid when the progress type is default.
   * @since_tizen 5.5
   * @param[in] unit The unit of progress
   */
  void SetDefaultUnit(std::string unit);

  /**
   * @brief Gets the default unit of progress.
   * @details the unit is valid when the progress type is default.
   * @since_tizen 5.5
   * @return The type of progress
   */
  std::string GetDefaultUnit() const;

 private:
  class Impl;
  std::unique_ptr<Impl> impl_;
};

}  // namespace item
}  // namespace notification

#endif  // NOTIFICATION_EX_PROGRESS_ITEM_H_
