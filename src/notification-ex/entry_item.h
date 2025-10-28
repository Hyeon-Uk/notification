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

#ifndef NOTIFICATION_EX_ENTRY_ITEM_H_
#define NOTIFICATION_EX_ENTRY_ITEM_H_

#include <memory>
#include <string>

#include "notification-ex/abstract_item.h"

#ifndef EXPORT_API
#define EXPORT_API __attribute__((visibility("default")))
#endif

namespace notification {
namespace item {

/**
 * @brief The class for EntryItem type notification.
 * @details The class to make the notification with text input.
 * @since_tizen 5.5
 */
class EXPORT_API EntryItem : public AbstractItem {
 public:
  /**
   * @brief Constructor
   * @since_tizen 5.5
   * @param[in] action The action for EntryItem
   */
  EntryItem(std::shared_ptr<AbstractAction> action = std::shared_ptr<AbstractAction>({}));

  /**
   * @brief Constructor
   * @since_tizen 5.5
   * @param[in] id The EntryItem id
   * @param[in] action The action for EntryItem
   */
  EntryItem(std::string id,
      std::shared_ptr<AbstractAction> action = std::shared_ptr<AbstractAction>({}));

  /**
   * @brief Destructor
   * @since_tizen 5.5
   */
  virtual ~EntryItem();

  /**
   * @brief Serialize the data of EntryItem.
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
   * @brief Gets the type of EntryItem.
   * @since_tizen 5.5
   * @return AbstractItem::Type::Entry
   */
  int GetType() const override;

  /**
   * @brief Gets the text data of EntryItem.
   * @since_tizen 5.5
   * @return The text data
   */;
  std::string GetText() const;

  /**
   * @brief Sets the text data of EntryItem.
   * @since_tizen 5.5
   * @param[in] The text data
   */
  void SetText(std::string text);

  /**
   * @brief Gets the limit of text size.
   * @since_tizen 5.5
   * @return The limit of text size
   */
  int GetTextLimit() const;

 private:
  class Impl;
  std::unique_ptr<Impl> impl_;
};  // class EntryItem

}  // namespace item
}  // namespace notification
#endif  // NOTIFICATION_EX_ENTRY_ITEM_H_