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

#ifndef NOTIFICATION_EX_INPUT_SELECTOR_ITEM_H_
#define NOTIFICATION_EX_INPUT_SELECTOR_ITEM_H_

#include <string>
#include <memory>
#include <list>

#include "notification-ex/abstract_item.h"

namespace notification {
namespace item {

/**
 * @brief The class for InputSelectorItem type notification.
 * @details The class to make the notification with selector.
 * @since_tizen 5.5
 */
class EXPORT_API InputSelectorItem : public AbstractItem {
 public:
  /**
   * @brief Constructor
   * @since_tizen 5.5
   * @param[in] action The action for InputSelectorItem
   */
  InputSelectorItem(std::shared_ptr<AbstractAction> action = std::shared_ptr<AbstractAction>({}));

  /**
   * @brief Constructor
   * @since_tizen 5.5
   * @param[in] id The InputSelectorItem id
   * @param[in] action The action for InputSelectorItem
   */
  InputSelectorItem(std::string id,
      std::shared_ptr<AbstractAction> action = std::shared_ptr<AbstractAction>({}));

  /**
   * @brief Destructor
   * @since_tizen 5.5
   */
  virtual ~InputSelectorItem();

 public:
  /**
   * @brief Serialize the data of InputSelectorItem.
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
   * @brief Gets the type of InputSelectorItem.
   * @since_tizen 5.5
   * @return AbstractItem::Type::InputSelector
   */
  int GetType() const override;

  /**
   * @brief Gets the contents of InputSelectorItem.
   * @since_tizen 5.5
   * @return The list of string
   */
  std::list<std::string> GetContents() const;

  /**
   * @brief Sets the contents of InputSelectorItem.
   * @since_tizen 5.5
   * @param[in] contents The list of string
   */
  void SetContents(std::list<std::string> contents);

 private:
  class Impl;
  std::unique_ptr<Impl> impl_;
};

}  // namespace item
}  // namespace notification

#endif  // NOTIFICATION_EX_INPUT_SELECTOR_ITEM_H_
