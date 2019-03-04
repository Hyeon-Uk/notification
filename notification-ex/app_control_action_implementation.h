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

#ifndef NOTIFICATION_EX_APP_CONTROL_ACTION_IMPLEMENTATION_H_
#define NOTIFICATION_EX_APP_CONTROL_ACTION_IMPLEMENTATION_H_

#include "notification-ex/app_control_action.h"

namespace notification {
namespace item {

class AppControlAction::Impl {
 public:
  virtual ~Impl();

 private:
  friend class AppControlAction;
  Impl(AppControlAction* parent);

 private:
  AppControlAction* parent_;

  app_control_h control_ = nullptr;
};

}  // namespace item
}  // namespace notification

#endif  // NOTIFICATION_EX_APP_CONTROL_ACTION_IMPLEMENTATION_H_

