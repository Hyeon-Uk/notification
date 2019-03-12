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

class EXPORT_API FactoryManager {
 public:
  static FactoryManager& GetInst();
  void RegisterFactory(std::unique_ptr<IItemFactory> factory);
  void RegisterFactory(std::unique_ptr<IActionFactory> factory);
  std::unique_ptr<AbstractItem> CreateItem(AbstractItem::Type type);
  std::unique_ptr<AbstractAction> CreateAction(AbstractAction::Type type);
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
