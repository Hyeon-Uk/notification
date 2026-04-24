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
#include <mutex>

#include "notification-ex/app_control_action.h"
#include "notification-ex/visibility_action.h"
#include "notification-ex/default_action_factory.h"
#include "notification-ex/exception.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "NOTIFICATION_EX"

using namespace std;
namespace notification {
namespace item {

namespace {
/* N3-T2 D8: process-level override slot (see SetActionFactoryOverride).
 * Mirrors default_item_factory.cc. */
std::unique_ptr<IActionFactory>& ActionOverrideSlot() {
  static std::unique_ptr<IActionFactory> slot;
  return slot;
}

std::mutex& ActionOverrideMutex() {
  static std::mutex m;
  return m;
}
}  // namespace

/*
 * N1-T2: Free-function implementation. Canonical default dispatch for
 * action creation. DefaultActionFactory forwards here so no
 * DefaultActionFactory instance is built on the hot path.
 *
 * N3-T2 D8: optional process-level override (see
 * SetActionFactoryOverride). When empty, this function runs its switch
 * without any singleton access.
 */
unique_ptr<AbstractAction> CreateAction(int type) {
  {
    std::lock_guard<std::mutex> lk(ActionOverrideMutex());
    if (ActionOverrideSlot()) {
      auto created = ActionOverrideSlot()->CreateAction(type);
      if (created)
        return created;
      /* Fall through to built-in dispatch on miss. */
    }
  }
  switch (type) {
    case AbstractAction::NullObject :
      THROW(ERROR_INVALID_PARAMETER);
    case AbstractAction::AppControl : {
      app_control_h control;
      app_control_create(&control);
      unique_ptr<AbstractAction> action(new AppControlAction(control));
      app_control_destroy(control);
      return action;
    }
    case AbstractAction::Visibility :
      return unique_ptr<AbstractAction>(new VisibilityAction());
    case AbstractAction::Custom :
      return nullptr;
  }

  return nullptr;
}

unique_ptr<AbstractAction> DefaultActionFactory::CreateAction(int type) {
  return ::notification::item::CreateAction(type);
}

/* N3-T2 D8: process-level override registration. Nullptr clears. */
void SetActionFactoryOverride(std::unique_ptr<IActionFactory> factory) {
  std::lock_guard<std::mutex> lk(ActionOverrideMutex());
  ActionOverrideSlot() = std::move(factory);
}

}  // namespace item
}  // namespace notification
