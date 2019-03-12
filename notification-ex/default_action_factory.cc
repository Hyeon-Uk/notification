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

unique_ptr<AbstractAction> DefaultActionFactory::CreateAction(
    AbstractAction::Type type) {

  switch (type) {
  case AbstractAction::NullObject :
    THROW(NOTIFICATION_ERROR_INVALID_PARAMETER);
  case AbstractAction::AppControl :
    app_control_h control;
    app_control_create(&control);
    return unique_ptr<AbstractAction>(new AppControlAction(control));
  case AbstractAction::Visibility :
    return unique_ptr<AbstractAction>(new VisibilityAction());
  case AbstractAction::Custom :
    return nullptr;
  }

  return nullptr;
}

}  // namespace item
}  // namespace notification
