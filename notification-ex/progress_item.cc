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

#include "notification-ex/exception.h"
#include "notification-ex/progress_item.h"
#include "notification-ex/progress_item_implementation.h"
#include "notification-ex/factory_manager.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "NOTIFICATION_EX"
#define PROGRESS_MIN_KEY "__PROGRESS_MIN_KEY__"
#define PROGRESS_CURRENT_KEY "__PROGRESS_CURRENT_KEY__"
#define PROGRESS_MAX_KEY "__PROGRESS_MAX_KEY__"

using namespace std;
namespace notification {
namespace item {

ProgressItem::ProgressItem(float min_val, float current, float max_val,
    std::shared_ptr<AbstractAction> action)
  : AbstractItem(action),
    impl_(new Impl(min_val, current, max_val, this)) {
}

ProgressItem::ProgressItem(string id, float min_val, float current, float max_val,
    std::shared_ptr<AbstractAction> action)
  : AbstractItem(id, action),
  impl_(new Impl(min_val, current, max_val, this)) {
}

ProgressItem::~ProgressItem() = default;
ProgressItem::Impl::~Impl() = default;

ProgressItem::Impl::Impl(float min_val, float current, float max_val, ProgressItem* parent)
  : min_(min_val), current_(current), max_(max_val), parent_(parent) {
  if (min_val > current || max_val < current)
    THROW(NOTIFICATION_ERROR_INVALID_PARAMETER);
  LOGI("ProgressItem impl created");
}

int ProgressItem::GetType() const {
  return AbstractItem::Progress;
}

Bundle ProgressItem::Serialize() const {
  Bundle b;
  b = AbstractItem::Serialize();
  b.Add(PROGRESS_MIN_KEY, to_string(impl_->min_));
  b.Add(PROGRESS_CURRENT_KEY, to_string(impl_->current_));
  b.Add(PROGRESS_MAX_KEY, to_string(impl_->max_));
  return b;
}

void ProgressItem::Deserialize(Bundle b) {
  AbstractItem::Deserialize(b);
  impl_->min_ = strtof(b.GetString(PROGRESS_MIN_KEY).c_str(), 0);
  impl_->current_ = strtof(b.GetString(PROGRESS_CURRENT_KEY).c_str(), 0);
  impl_->max_ = strtof(b.GetString(PROGRESS_MAX_KEY).c_str(), 0);
}

AbstractItem& ProgressItem::FindByID(std::string id) {
  if (GetId() == id)
    return *this;
  return FactoryManager::GetInst().GetNullItem();
}

float ProgressItem::GetCurrent() const {
  return impl_->current_;
}

void ProgressItem::SetCurrent(float current) {
  impl_->current_ = current;
}

float ProgressItem::GetMin() const {
  return impl_->min_;
}

float ProgressItem::GetMax() const {
  return impl_->max_;
}

}  // namespace item
}  // namespace notification
