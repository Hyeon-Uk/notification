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

#ifndef NOTIFICATION_EX_ITEM_INFLATOR_H_
#define NOTIFICATION_EX_ITEM_INFLATOR_H_

#include <string>
#include <memory>
#include <list>

#include "notification-ex/abstract_item.h"

namespace notification {
namespace item {

/**
 * @brief The class to create item
 * @since_tizen 5.5
 */
class EXPORT_API ItemInflator {
 public:
  /**
   * @brief Creates AbstractItem from Bundle data
   * @since_tizen 5.5
   * @param[in] b Bundle type data
   */
  static std::unique_ptr<AbstractItem> Create(tizen_base::Bundle b);
};

}  // namespace item
}  // namespace notification

#endif  // NOTIFICATION_EX_ITEM_INFLATOR_H_
