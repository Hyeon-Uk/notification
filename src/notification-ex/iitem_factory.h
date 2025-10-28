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

#ifndef NOTIFICATION_EX_IITEM_FACTORY_H_
#define NOTIFICATION_EX_IITEM_FACTORY_H_

#include <string>
#include <memory>
#include <list>

#include "notification-ex/abstract_item.h"

namespace notification {
namespace item {

/**
 * @brief The interface class for ItemFactory.
 * @since_tizen 5.5
 */
class EXPORT_API IItemFactory {
 public:
  /**
   * @brief Destructor
   * @since_tizen 5.5
   */
  virtual ~IItemFactory() = default;

  /**
   * @brief Creates the AbstractItem from type
   * @since_tizen 5.5
   * @param[in] type The type of notification item
   */
  virtual std::unique_ptr<AbstractItem> CreateItem(int type) = 0;
};

}  // namespace item
}  // namespace notification

#endif  // NOTIFICATION_EX_IITEM_FACTORY_H_
