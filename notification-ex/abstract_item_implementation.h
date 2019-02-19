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

#ifndef NOTIFICATION_EX_ABSTRACT_ITEM_IMPLEMENTATION_H_
#define NOTIFICATION_EX_ABSTRACT_ITEM_IMPLEMENTATION_H_

#include <string>
#include <memory>
#include <list>

#include "notification-ex/abstract_item.h"

namespace notification {
namespace item {

class AbstractItem::Impl {
 public:
  virtual ~Impl() = default;

 private:
  Impl(AbstractItem* parent);
  Impl(AbstractItem* parent, AbstractItem::Type type,
      std::shared_ptr<AbstractAction> action = std::shared_ptr<AbstractAction>({}));
  Impl(AbstractItem* parent, std::string id, AbstractItem::Type type,
      std::shared_ptr<AbstractAction> action = std::shared_ptr<AbstractAction>({}));

 private:
  friend class AbstractItem;

  std::string channel_;
  std::string id_;
  std::shared_ptr<LEDInfo> led_ = nullptr;
  Policy policy_ = None;
  std::shared_ptr<Style> style_ = nullptr;
  AbstractItem::Type type_;
  bool visible_ = true;
  bool enable_ = true;
  int version_ = 1;
  int hide_time_ = 0;
  int delete_time_ = 0;
  std::list<std::string> receiver_group_list_;
  std::string can_receive_;
  std::shared_ptr<AbstractAction> action_;
  AbstractItem* parent_;
};

}  // namespace item
}  // namespace notification

#endif  // NOTIFICATION_EX_ABSTRACT_ITEM_IMPLEMENTATION_H_
