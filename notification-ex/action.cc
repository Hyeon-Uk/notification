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

#include <string>
#include <memory>

#include "notification-ex/action.h"
#include "notification-ex/bundle.h"
#include "notification-ex/exception.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "NOTIFICATION_EX"

namespace notification {
namespace item {

AppControlAction::AppControlAction(app_control_h control)
  : control_(control) {
}

AppControlAction::~AppControlAction() {
}

ReplyAction::ReplyAction() {
}

ReplyAction::~ReplyAction() {
}

VisibilityAction::VisibilityAction() {
}

VisibilityAction::~VisibilityAction() {
}


} //namespace item
}  //namespace notification
