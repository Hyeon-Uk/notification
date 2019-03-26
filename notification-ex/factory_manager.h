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

#ifndef NOTIFICATION_EX_FACTORY_MANAGER_H_
#define NOTIFICATION_EX_FACTORY_MANAGER_H_

#include <string>
#include <memory>
#include <list>

#include "notification-ex/iitem_factory.h"
#include "notification-ex/iaction_factory.h"

namespace notification {
namespace item {

/**
 * @brief The class for FactoryManager.
 * @details The class to manage creation of action and item.
 * @since_tizen 5.5
 */
class EXPORT_API FactoryManager {
 public:
  /**
   * @brief Gets the instance of FactoryManager
   * @since_tizen 5.5
   * @return The FactoryManager instance
   */
  static FactoryManager& GetInst();

  /**
   * @brief Resgisters the IItemFactory
   * @since_tizen 5.5
   * @param[in] factory The IItemFactory for noitfication item
   */
  void RegisterFactory(std::unique_ptr<IItemFactory> factory);

  /**
   * @brief Registers the IActionFactory
   * @since_tizen 5.5
   * @param[in] factory The IActionFactory for noitfication action
   */
  void RegisterFactory(std::unique_ptr<IActionFactory> factory);

  /**
   * @brief Creates the notification item from type.
   * @since_tizen 5.5
   * @return AbstractItem object
   */
  std::unique_ptr<AbstractItem> CreateItem(int type);

  /**
   * @brief Creates the notification action from type.
   * @since_tizen 5.5
   * @return AbstractAction object
   */
  std::unique_ptr<AbstractAction> CreateAction(int type);

  /**
   * @brief Gets NullItem
   * @since_tizen 5.5
   * @return NullItem object
   */
  AbstractItem& GetNullItem();

 private:
  FactoryManager() {}

 private:
  std::unique_ptr<IItemFactory> item_factory_;
  std::unique_ptr<IActionFactory> action_factory_;
};

}  // namespace item
}  // namespace notification

#endif  // NOTIFICATION_EX_FACTORY_MANAGER_H_
