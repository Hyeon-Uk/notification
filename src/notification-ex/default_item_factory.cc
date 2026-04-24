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
#include <mutex>

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

namespace {
/* N3-T2 D8: process-level override slot. Replaces the
 * FactoryManager::item_factory_ member without a class singleton. The
 * mutex only serializes the register call against concurrent first
 * access; steady-state CreateItem reads are cheap (single pointer load
 * under a short lock; contention is not a real workload - plugins and
 * tests install the override once at init and never mutate the slot
 * again).
 *
 * Hidden visibility: the slot is strictly internal to this TU. The
 * public API surface is the two free functions CreateItem /
 * SetItemFactoryOverride declared in default_item_factory.h. */
std::unique_ptr<IItemFactory>& ItemOverrideSlot() {
  static std::unique_ptr<IItemFactory> slot;
  return slot;
}

std::mutex& ItemOverrideMutex() {
  static std::mutex m;
  return m;
}
}  // namespace

/*
 * N1-T2: Free-function implementation. This is the canonical default
 * dispatch table. DefaultItemFactory::CreateItem routes here, so no
 * DefaultItemFactory instance is constructed on the hot path.
 *
 * N3-T2 D8: Optional process-level override slot (see
 * SetItemFactoryOverride). When empty — the default — this function
 * jumps straight into the switch below without any singleton access.
 */
unique_ptr<AbstractItem> CreateItem(int type) {
  /* Override check first. The lock is local and cheap, and the
   * override is rarely set, so this path is a pointer-null test in
   * practice. */
  {
    std::lock_guard<std::mutex> lk(ItemOverrideMutex());
    if (ItemOverrideSlot()) {
      auto created = ItemOverrideSlot()->CreateItem(type);
      if (created)
        return created;
      /* Override returned nullptr (type not handled). Fall through to
       * the built-in dispatch so mixed factories can compose. This
       * matches the prior FactoryManager shim which, if the override
       * missed, called the free function below. */
    }
  }
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
   * caller goes through the class entry (still held for external plugin
   * API surface) or the free-function entry. */
  return ::notification::item::CreateItem(type);
}

/* N3-T2 D8: process-level override registration. Nullptr clears. */
void SetItemFactoryOverride(std::unique_ptr<IItemFactory> factory) {
  std::lock_guard<std::mutex> lk(ItemOverrideMutex());
  ItemOverrideSlot() = std::move(factory);
}

}  // namespace item
}  // namespace notification
