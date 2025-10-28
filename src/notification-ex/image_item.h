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

#ifndef NOTIFICATION_EX_IMAGE_ITEM_H_
#define NOTIFICATION_EX_IMAGE_ITEM_H_

#include <string>
#include <memory>
#include <list>
#include <map>

#include "notification-ex/abstract_item.h"

namespace notification {
namespace item {

/**
 * @brief The class for ImageItem type notification.
 * @details The class to make the notification with image.
 * @since_tizen 5.5
 */
class EXPORT_API ImageItem : public AbstractItem {
 public:
  /**
   * @brief Constructor
   * @since_tizen 5.5
   * @param[in] image_path The image path of ImageItem
   * @param[in] action The action for ImageItem
   */
  ImageItem(std::string image_path,
    std::shared_ptr<AbstractAction> action = std::shared_ptr<AbstractAction>({}));

  /**
   * @brief Constructor
   * @since_tizen 5.5
   * @param[in] id The ImageItem id
   * @param[in] image_path The image path of ImageItem
   * @param[in] action The action for ImageItem
   */
  ImageItem(std::string id, std::string image_path,
    std::shared_ptr<AbstractAction> action = std::shared_ptr<AbstractAction>({}));

  /**
   * @brief Destructor
   * @since_tizen 5.5
   */
  virtual ~ImageItem();

  /**
   * @brief Gets the type of ImageItem.
   * @since_tizen 5.5
   * @return AbstractItem::Type::Image
   */
  int GetType() const override;

  /**
   * @brief Serialize the data of ImageItem.
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
   * @brief Gets the path of image.
   * @since_tizen 5.5
   * @return The path of image
   */
  std::string GetImagePath() const;

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

  std::list<std::map<std::string, std::string>> GetPathMapList(void) const override;

 private:
  class Impl;
  std::unique_ptr<Impl> impl_;
  void UpdatePrivatePath();
};  //class ImageItem

}  // namespace item
}  // namespace notification
#endif  // NOTIFICATION_EX_IMAGE_ITEM_H_
