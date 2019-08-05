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

#ifndef NOTIFICATION_EX_CHAT_MESSAGE_ITEM_H_
#define NOTIFICATION_EX_CHAT_MESSAGE_ITEM_H_

#include <time.h>

#include <string>
#include <memory>
#include <list>

#include "notification-ex/abstract_item.h"
#include "notification-ex/text_item.h"
#include "notification-ex/image_item.h"
#include "notification-ex/time_item.h"

namespace notification {
namespace item {

/**
 * @brief The class for ChatMessageItem type notification.
 * @details The class to make the chat message type notification.
 * @since_tizen 5.5
 */
class EXPORT_API ChatMessageItem : public AbstractItem {
 public:
  enum Type {
    user,
    sender,
  };

 public:
  /**
   * @brief Constructor
   * @since_tizen 5.5
   * @param[in] id The ChatMessageItem id
   * @param[in] name The name of chat message
   * @param[in] text The text of chat message
   * @param[in] image The image of chat message
   * @param[in] time The time of chat message
   * @param[in] type The type of chat message
   * @param[in] action The action for ChatMessageItem
   */
  ChatMessageItem(std::string id, std::shared_ptr<TextItem> name,
    std::shared_ptr<TextItem> text, std::shared_ptr<ImageItem> image,
    std::shared_ptr<TimeItem> time, Type type,
    std::shared_ptr<AbstractAction> action = std::shared_ptr<AbstractAction>({}));

  /**
   * @brief Destructor
   * @since_tizen 5.5
   */
  virtual ~ChatMessageItem();

  /**
   * @brief Gets the type of ChatMessageItem.
   * @since_tizen 5.5
   * @return AbstractItem::Type::ChatMessage
   */
  int GetType() const override;

  /**
   * @brief Serialize the data of ChatMessageItem.
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
   * @brief Gets the path of shared file location.
   * @since_tizen 5.5
   * @return The list of shared path.
   */
  std::list<std::string> GetSharedPath() const override;

  /**
   * @brief Gets the name data of ChatMessageItem.
   * @since_tizen 5.5
   * @return The TextItem type name data
   */
  TextItem& GetNameItem() const;

  /**
   * @brief Gets the text data of ChatMessageItem.
   * @since_tizen 5.5
   * @return The TextItem type text data
   */
  TextItem& GetTextItem() const;

  /**
   * @brief Gets the image data of ChatMessageItem.
   * @since_tizen 5.5
   * @return The ImageItem type image data
   */
  ImageItem& GetImageItem() const;

  /**
   * @brief Gets the time data of ChantMessageItem.
   * @since_tizen 5.5
   * @return The TimeItem type time data
   */
  TimeItem& GetTimeItem() const;

  /**
   * @brief Gets the type of message.
   * @since_tizen 5.5
   * @return ChatMessageItem::Type::sender or ChatMessageItem::Type::user
   */
  Type GetMessageType() const;

 private:
  class Impl;
  std::unique_ptr<Impl> impl_;
};  // class ChatMessageItem

}  // namespace item
}  // namespace notification
#endif  // NOTIFICATION_EX_CHAT_MESSAGE_ITEM_H_
