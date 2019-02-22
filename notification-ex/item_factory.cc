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

#include "notification-ex/item_factory.h"
#include "notification-ex/button_item.h"
#include "notification-ex/group_item.h"
#include "notification-ex/null_item.h"
#include "notification-ex/entry_item.h"
#include "notification-ex/text_item.h"
#include "notification-ex/exception.h"
#include "notification-ex/input_selector_item.h"
#include "notification-ex/progress_item.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "NOTIFICATION_EX"

using namespace std;
namespace notification {
namespace item {

shared_ptr<AbstractItem> ItemFactory::CreateItem(AbstractItem::Type type) {
  switch (type) {
  case AbstractItem::NullObject :
    THROW(NOTIFICATION_ERROR_INVALID_PARAMETER);
  case AbstractItem::Text :
    return make_shared<TextItem>("");
  case AbstractItem::Icon :
    return make_shared<ButtonItem>("");
  case AbstractItem::Image :
    return make_shared<ButtonItem>("");
  case AbstractItem::Button :
    return make_shared<ButtonItem>("");
  case AbstractItem::ChatMessage :
    return make_shared<ButtonItem>("");
  case AbstractItem::CheckBox :
    return make_shared<ButtonItem>("");
  case AbstractItem::IconText :
    return make_shared<ButtonItem>("");
  case AbstractItem::InputSelector :
    return make_shared<InputSelectorItem>();
  case AbstractItem::Group :
    return make_shared<GroupItem>();
  case AbstractItem::Entry :
    return make_shared<EntryItem>("");
  case AbstractItem::Progress :
    return make_shared<ProgressItem>(0.0, 0.0, 0.0);
  case AbstractItem::Custom :
    return make_shared<ButtonItem>("");
  }

  return nullptr;
}

AbstractItem& ItemFactory::GetNullItem() {
  static NullItem item;
  return item;
}

}  // namespace item
}  // namespace notification
