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

#include <dlog.h>

#include "notification-ex/factory_manager.h"
#include "notification-ex/default_item_factory.h"
#include "notification-ex/default_action_factory.h"
#include "notification-ex/null_item.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "NOTIFICATION_EX"

using namespace std;
namespace notification {
namespace item {

FactoryManager& FactoryManager::GetInst() {
  static FactoryManager manager;
  return manager;
}

void FactoryManager::RegisterFactory(std::unique_ptr<IItemFactory> factory) {
  item_factory_ = std::move(factory);
}

void FactoryManager::RegisterFactory(std::unique_ptr<IActionFactory> factory) {
  action_factory_ = std::move(factory);
}

std::unique_ptr<AbstractItem> FactoryManager::CreateItem(int type) {
  if (item_factory_.get() == nullptr)
    item_factory_.reset(new DefaultItemFactory());

  return item_factory_->CreateItem(type);
}

std::unique_ptr<AbstractAction> FactoryManager::CreateAction(int type) {
  if (action_factory_.get() == nullptr)
    action_factory_.reset(new DefaultActionFactory());

  return action_factory_->CreateAction(type);
}

AbstractItem& FactoryManager::GetNullItem() {
  static NullItem item;
  return item;
}

}  // namespace item
}  // namespace notification
