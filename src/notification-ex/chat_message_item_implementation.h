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

#ifndef NOTIFICATION_EX_CHAT_MESSAGE_ITEM_IMPLEMENTATION_H_
#define NOTIFICATION_EX_CHAT_MESSAGE_ITEM_IMPLEMENTATION_H_

#include <time.h>

#include <string>
#include <memory>
#include <list>

#include "notification-ex/chat_message_item.h"

namespace notification {
namespace item {

class ChatMessageItem::Impl {
 public:
  virtual ~Impl();

 private:
  friend class ChatMessageItem;
  Impl(ChatMessageItem* parent, std::shared_ptr<TextItem> name,
    std::shared_ptr<TextItem> text, std::shared_ptr<ImageItem> image,
    std::shared_ptr<TimeItem> time, Type type);

 private:
  ChatMessageItem* parent_;
  std::shared_ptr<TextItem> name_;
  std::shared_ptr<TextItem> text_;
  std::shared_ptr<ImageItem> image_;
  std::shared_ptr<TimeItem> time_;
  Type type_;
};

}  // namespace item
}  // namespace notification
#endif  // NOTIFICATION_EX_CHAT_MESSAGE_ITEM_IMPLEMENTATION_H_
