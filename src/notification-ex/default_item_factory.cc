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

#include "notification-ex/default_item_factory.h"
#include "notification-ex/button_item.h"
#include "notification-ex/group_item.h"
#include "notification-ex/entry_item.h"
#include "notification-ex/text_item.h"
#include "notification-ex/exception.h"
#include "notification-ex/input_selector_item.h"
#include "notification-ex/progress_item.h"
#include "notification-ex/icon_item.h"
#include "notification-ex/image_item.h"
#include "notification-ex/checkbox_item.h"
#include "notification-ex/chat_message_item.h"
#include "notification-ex/time_item.h"
#include "notification-ex/null_item.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "NOTIFICATION_EX"

using namespace std;
namespace notification {
namespace item {

/*
 * N1-T2: Free-function implementation. This is the canonical default
 * dispatch table. Both DefaultItemFactory::CreateItem and the
 * FactoryManager shim ultimately route here when no override is set,
 * so no DefaultItemFactory instance is constructed on the hot path.
 */
unique_ptr<AbstractItem> CreateItem(int type) {
  switch (type) {
    case AbstractItem::NullObject :
      THROW(ERROR_INVALID_PARAMETER);
    case AbstractItem::Text :
      return unique_ptr<AbstractItem>(new TextItem("", ""));
    case AbstractItem::Icon :
      return unique_ptr<AbstractItem>(new IconItem(""));
    case AbstractItem::Image :
      return unique_ptr<AbstractItem>(new ImageItem(""));
    case AbstractItem::Button :
      return unique_ptr<AbstractItem>(new ButtonItem(""));
    case AbstractItem::ChatMessage :
      return unique_ptr<AbstractItem>(new ChatMessageItem("",
        nullptr, nullptr, nullptr, nullptr, ChatMessageItem::Type::user));
    case AbstractItem::CheckBox :
      return unique_ptr<AbstractItem>(new CheckBoxItem("", ""));
    case AbstractItem::InputSelector :
      return unique_ptr<AbstractItem>(new InputSelectorItem());
    case AbstractItem::Group :
      return unique_ptr<AbstractItem>(new GroupItem());
    case AbstractItem::Entry :
      return unique_ptr<AbstractItem>(new EntryItem(""));
    case AbstractItem::Progress :
      return unique_ptr<AbstractItem>(new ProgressItem(0.0, 0.0, 0.0));
    case AbstractItem::Time :
      return unique_ptr<AbstractItem>(new TimeItem());
    case AbstractItem::Custom :
      return unique_ptr<AbstractItem>(new ButtonItem(""));
  }

  return nullptr;
}

AbstractItem& GetNullItem() {
  static NullItem item;
  return item;
}

unique_ptr<AbstractItem> DefaultItemFactory::CreateItem(int type) {
  /* Delegate to the free function so behavior stays identical whether the
   * caller goes through FactoryManager or the free-function entry. */
  return ::notification::item::CreateItem(type);
}

}  // namespace item
}  // namespace notification
