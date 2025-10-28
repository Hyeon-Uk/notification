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

#ifndef NOTIFICATION_EX_CHECKBOX_ITEM_IMPLEMENTATION_H_
#define NOTIFICATION_EX_CHECKBOX_ITEM_IMPLEMENTATION_H_

#include <string>
#include <memory>
#include <list>

#include "notification-ex/checkbox_item.h"

namespace notification {
namespace item {

class CheckBoxItem::Impl {
 public:
  virtual ~Impl();

 private:
  friend class CheckBoxItem;
  Impl(CheckBoxItem* parent, std::string title, bool checked);

 private:
  CheckBoxItem* parent_;
  std::string title_;
  bool checked_;
};

}  // namespace item
}  // namespace notification
#endif  // NOTIFICATION_EX_CHECKBOX_ITEM_IMPLEMENTATION_H_
