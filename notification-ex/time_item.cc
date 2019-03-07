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

#include "notification-ex/time_item.h"
#include "notification-ex/time_item_implementation.h"
#include "notification-ex/item_factory.h"
#include "notification-ex/exception.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "NOTIFICATION_EX"
#define TIME_KEY "__TIME_KEY__"

namespace notification {
namespace item {

TimeItem::TimeItem(std::shared_ptr<AbstractAction> action)
  : AbstractItem(AbstractItem::Time, action), impl_(new Impl(this, 0)) {
  time_t current_time;
  time(&current_time);

  impl_->time_ = current_time;
}

TimeItem::TimeItem(time_t time, std::shared_ptr<AbstractAction> action)
  : AbstractItem(AbstractItem::Time, action), impl_(new Impl(this, time)) {
}

TimeItem::TimeItem(std::string id, time_t time,
  std::shared_ptr<AbstractAction> action)
  : AbstractItem(id, AbstractItem::Time, action), impl_(new Impl(this, time)) {
}

TimeItem::Impl::Impl(TimeItem* parent, time_t time)
  : parent_(parent), time_(time) {
  LOGI("TimeItem impl created");
}

Bundle TimeItem::Serialize() {
  Bundle b;
  struct tm* timeinfo;
  char buf[80] = {0,};

  b = AbstractItem::Serialize();

  //timt_t to tm
  timeinfo = localtime(&impl_->time_);
  //tm to str
  strftime (buf, sizeof(buf), "%s", timeinfo);
  b.Add(TIME_KEY, std::string(buf));

  return b;
}

void TimeItem::Deserialize(Bundle b) {
  std::string time_s;
  struct tm timeinfo;

  AbstractItem::Deserialize(b);
  time_s = b.GetString(TIME_KEY);

  //str to tm
  strptime(time_s.c_str(), "%s", &timeinfo);
  //tm to time_t
  impl_->time_ = mktime(&timeinfo);
}

AbstractItem& TimeItem::FindByID(std::string id) {
  if (GetId() == id)
    return *this;

  return ItemFactory::GetNullItem();
}

time_t TimeItem::GetTime() const {
  return impl_->time_;
}

TimeItem::~TimeItem() = default;
TimeItem::Impl::~Impl() = default;

}  // namespace item
}  // namespace notification_ex
