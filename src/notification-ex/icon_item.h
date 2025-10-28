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

#ifndef NOTIFICATION_EX_ICON_ITEM_H_
#define NOTIFICATION_EX_ICON_ITEM_H_

#include <string>
#include <memory>
#include <list>

#include "notification-ex/image_item.h"

/**
 * @brief The class for IconItem type notification.
 * @details The class to make the notification with icon.
 * @since_tizen 5.5
 */
namespace notification {
namespace item {

class EXPORT_API IconItem : public ImageItem {
 public:
  /**
   * @brief Constructor
   * @since_tizen 5.5
   * @param[in] icon_path The icon path of IconItem
   * @param[in] action The action for IconItem
   */
  IconItem(std::string icon_path,
    std::shared_ptr<AbstractAction> action = std::shared_ptr<AbstractAction>({}));

  /**
   * @brief Constructor
   * @since_tizen 5.5
   * @param[in] id The IconItem id
   * @param[in] icon_path The icon path of IconItem
   * @param[in] action The action for IconItem
   */
  IconItem(std::string id, std::string icon_path,
    std::shared_ptr<AbstractAction> action = std::shared_ptr<AbstractAction>({}));

  /**
   * @brief Destructor
   * @since_tizen 5.5
   */
  virtual ~IconItem();

  /**
   * @brief Gets the type of IconItem.
   * @since_tizen 5.5
   * @return AbstractItem::Type::Icon
   */
  int GetType() const override;

 private:
  class Impl;
  std::unique_ptr<Impl> impl_;
};  // class IconItem

}  // namespace item
}  // namespace notification
#endif  // NOTIFICATION_EX_ICON_ITEM_H_
