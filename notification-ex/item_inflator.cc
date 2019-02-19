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
#include "notification-ex/item_inflator.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "NOTIFICATION_EX"

using namespace std;
namespace notification {
namespace item {

shared_ptr<AbstractItem> ItemInflator::Create(ItemFactory factory, Bundle b) {
  shared_ptr<AbstractItem> item = factory.CreateItem(AbstractItem::GetType(b));
  item.get()->Deserialize(b);
  return item;
}

}  // namespace item
}  // namespace notification
