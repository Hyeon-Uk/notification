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

#ifndef NOTIFICATION_EX_GROUP_ITEM_H_
#define NOTIFICATION_EX_GROUP_ITEM_H_

#include <string>
#include <memory>
#include <list>
#include <map>

#include "notification-ex/abstract_item.h"

namespace notification {
namespace item {

/**
 * @brief The class for GroupItem type notification.
 * @details The class to make the group of notifications.
 * @since_tizen 5.5
 */
class EXPORT_API GroupItem : public AbstractItem {
 public:
  /**
   * @brief Constructor
   * @since_tizen 5.5
   * @param[in] action The action for GroupItem
   */
  GroupItem(std::shared_ptr<AbstractAction> action = std::shared_ptr<AbstractAction>({}));

  /**
   * @brief Constructor
   * @since_tizen 5.5
   * @param[in] id The GroupItem id
   * @param[in] action The action for GroupItem
   */
  GroupItem(std::string id,
      std::shared_ptr<AbstractAction> action = std::shared_ptr<AbstractAction>({}));

  /**
   * @brief Deserialize
   * @since_tizen 5.5
   */
  virtual ~GroupItem();

 public:
  /**
   * @brief Serialize the data of GroupItem.
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
   * @brief Finds the AbstractItem using by notification item id.
   * @since_tizen 5.5
   * @param[in] id notification item id
   * @return AbstractItem object
   */
  AbstractItem& FindByID(std::string id) override;

  /**
   * @brief Finds the AbstractItem using by main type.
   * @since_tizen 5.5
   * @param[in] type The main type
   * @return AbstractItem object
   */
  AbstractItem& FindByMainType(MainType type) override;

  /**
   * @brief Checks the item type exist in this notification.
   * @since_tizen 5.5
   * @param[in] type notification item type
   * @return true if the item type exists
   */
  bool IsItemTypeExist(int type) override;

  /**
   * @brief Gets the type of GroupItem.
   * @since_tizen 5.5
   * @return AbstractItem::Type::Group
   */
  int GetType() const override;

  /**
   * @brief Gets the path of shared file location.
   * @since_tizen 5.5
   * @return The list of shared path.
   */
  std::list<std::string> GetSharedPath() const override;

  /**
   * @brief Sets the shared file path to original file path.
   * @since_tizen 5.5
   */
  void SetSharedPath() override;

  /**
   * @brief Gets the paths of shared file location.
   * @since_tizen 5.5
   * @return The list of private path and original path.
   */
  std::list<std::map<std::string, std::string>> GetPathMapList() const override;

  /**
   * @brief Sets the vertical state.
   * @details The vertical state is true, the children of GroupItem are associated vertically.
   *          And if it is false, the children are associated horizontally.
   * @since_tizen 5.5
   * @param[in] The vertical state
   */
  void SetDirection(bool vertical);

  /**
   * @brief Gets whether the vertical state is true or not.
   * @since_tizen 5.5
   * @return true if vertical is true, or false
   */
  bool IsVertical();

  /**
   * @brief Gets app label data of GroupItem.
   * @since_tizen 5.5
   * @return The app label data
   */
  std::string GetAppLabel();

  /**
   * @brief Sets app label data of GroupItem.
   * @since_tizen 5.5
   * @return The app label data
   */
  void SetAppLabel(std::string label);

  /**
   * @brief Adds child notification item.
   * @since_tizen 5.5
   * @param[in] AbstractItem object
   */
  void AddChild(std::shared_ptr<AbstractItem> child);

  /**
   * @brief Removes child notification item.
   * @since_tizen 5.5
   * @param[in] The notification id
   */
  void RemoveChild(std::string itemId);

  /**
   * @brief Removes children.
   * @since_tizen 5.5
   */
  void RemoveChildren();

  /**
   * @brief Gets the list of children item of GroupItem.
   * @since_tizen 5.5
   * @return The list of AbstractItem
   */
  std::list<std::shared_ptr<AbstractItem>> GetChildren();

 private:
  class Impl;
  std::unique_ptr<Impl> impl_;
};

}  // namespace item
}  // namespace notification

#endif  // NOTIFICATION_EX_GROUP_ITEM_H_
