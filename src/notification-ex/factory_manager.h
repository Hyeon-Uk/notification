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

#ifndef NOTIFICATION_EX_FACTORY_MANAGER_H_
#define NOTIFICATION_EX_FACTORY_MANAGER_H_

/*
 * N3-T2 D8 (2026-04): FactoryManager body removed.
 *
 * Phase 1 (N1-T2) converted this class into a deprecation shim that
 * forwarded to the free functions `::notification::item::CreateItem` /
 * `CreateAction` / `GetNullItem` (see `default_item_factory.h` /
 * `default_action_factory.h`). Phase 3 now retires the shim entirely:
 *
 *   1. The three internal call sites (`action_inflator.cc`,
 *      `item_inflator.cc`, `group_item.cc`) call the free functions
 *      directly.
 *   2. No `FactoryManager` symbol is exported from libnotification-ex.so
 *      (`nm --dynamic | grep FactoryManager` == 0).
 *   3. The file is kept as an empty header so that legacy consumers
 *      still listing `#include "notification-ex/factory_manager.h"` in
 *      their build continue to compile during the Phase 4 cleanup
 *      window. Any code that still referenced the class body has been
 *      migrated to the free functions exposed by the two headers
 *      above, plus the `SetItemFactoryOverride` /
 *      `SetActionFactoryOverride` test seams.
 *
 * If a downstream caller needs to register a custom factory (the old
 * `FactoryManager::GetInst().RegisterFactory(...)` use case), call
 * `::notification::item::SetItemFactoryOverride(std::move(factory))`
 * or `::notification::item::SetActionFactoryOverride(...)` declared in
 * default_item_factory.h / default_action_factory.h.
 */

namespace notification {
namespace item {
/* intentionally empty - symbols live in default_item_factory.h /
 * default_action_factory.h */
}  // namespace item
}  // namespace notification

#endif  // NOTIFICATION_EX_FACTORY_MANAGER_H_
