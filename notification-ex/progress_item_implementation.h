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

#ifndef NOTIFICATION_EX_PROGRESS_ITEM_IMPLEMENTATION_H_
#define NOTIFICATION_EX_PROGRESS_ITEM_IMPLEMENTATION_H_

#include <string>
#include <memory>
#include <list>

#include "notification-ex/progress_item.h"

namespace notification {
namespace item {

class ProgressItem::Impl {
 public:
  virtual ~Impl();

 private:
  Impl(float min_val, float current, float max_val,
    ProgressItem* parent);

 private:
  friend class ProgressItem;

  float min_;
  float current_;
  float max_;
  ProgressItem* parent_;
  Type type_;
  std::string unit_;
};

}  // namespace item
}  // namespace notification

#endif  // NOTIFICATION_EX_PROGRESS_ITEM_IMPLEMENTATION_H_
