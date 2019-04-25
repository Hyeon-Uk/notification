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

#include "notification-ex/abstract_action.h"
#include "notification-ex/abstract_action_implementation.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "NOTIFICATION_EX"
#define ABSTRACT_ACTION_TYPE_KEY "__ABSTRACT_ACTION_TYPE_KEY__"
#define ABSTRACT_ACTION_IS_LOCAL_KEY "__ABSTRACT_ACTION_IS_LOCAL_KEY__"
#define ABSTRACT_ACTION_EXTRA_KEY "__ABSTRACT_ACTION_EXTRA_KEY__"

namespace notification {
namespace item {
AbstractAction::AbstractAction(bool isLocal)
    : impl_(new Impl(this, isLocal)) {
}

AbstractAction::AbstractAction(bool isLocal, std::string extra)
    : impl_(new Impl(this, isLocal, extra)) {
}

AbstractAction::Impl::Impl(AbstractAction* parent, bool isLocal)
    : parent_(parent) , isLocal_(isLocal) {
  LOGI("Action created");
}

AbstractAction::Impl::Impl(AbstractAction* parent, bool isLocal,
    std::string extra) : parent_(parent) , isLocal_(isLocal), extra_(extra) {
  LOGI("Action created");
}

AbstractAction::~AbstractAction() = default;
AbstractAction::Impl::~Impl() = default;

int AbstractAction::GetType(Bundle b) {
  return std::stoi(b.GetString(ABSTRACT_ACTION_TYPE_KEY));
}

Bundle AbstractAction::Serialize() const {
  Bundle b;

  b.Add(ABSTRACT_ACTION_TYPE_KEY, std::to_string(GetType()));
  b.Add(ABSTRACT_ACTION_IS_LOCAL_KEY, std::to_string(impl_->isLocal_));
  if (!impl_->extra_.empty())
    b.Add(ABSTRACT_ACTION_EXTRA_KEY, impl_->extra_);
  return b;
}

void AbstractAction::Deserialize(Bundle b) {
  impl_->isLocal_ = std::stoi(b.GetString(ABSTRACT_ACTION_IS_LOCAL_KEY));
  impl_->extra_ = b.GetString(ABSTRACT_ACTION_EXTRA_KEY);
}

bool AbstractAction::IsLocal() const {
  return impl_->isLocal_;
}

void AbstractAction::Execute(std::shared_ptr<AbstractItem> item) {
}

std::string AbstractAction::GetExtra() const {
  return impl_->extra_;
}

}  // namespace item
}  // namespace notification
