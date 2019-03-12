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

#include <memory>

#include "notification-ex/factory_manager.h"
#include "notification-ex/action_inflator.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "NOTIFICATION_EX"

namespace notification {
namespace item {

std::shared_ptr<AbstractAction> ActionInflator::Create(Bundle b) {
  std::shared_ptr<AbstractAction> action =
      FactoryManager::GetInst().CreateAction(AbstractAction::GetType(b));
  action.get()->Deserialize(b);
  return action;
}

}  // namespace item
}  // namespace notification

