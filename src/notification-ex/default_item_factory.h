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

#ifndef NOTIFICATION_EX_DEFAULT_ITEM_FACTORY_H_
#define NOTIFICATION_EX_DEFAULT_ITEM_FACTORY_H_

#include <memory>

#include "notification-ex/iitem_factory.h"

namespace notification {
namespace item {

/**
 * @brief The class for predefined item.
 * @details The DefaultItemFactory is registered in factory manager default.
 * @since_tizen 5.5
 */
class EXPORT_API DefaultItemFactory : public IItemFactory {
 public:
  /**
   * @brief Destructor
   * @since_tizen 5.5
   */
  virtual ~DefaultItemFactory() = default;

  /**
   * @brief Creates AbstractItem
   * @since_tizen 5.5
   * @param[in] type The type of notification item.
   */
  std::unique_ptr<AbstractItem> CreateItem(int type) override;
};

/* N1-T2: Free-function entry points for default item creation.
 * These replace the FactoryManager singleton path for internal callers so
 * that no FactoryManager / DefaultItemFactory instance is materialized on
 * the hot path.
 *
 * N3-T2 D8: FactoryManager was retired. `CreateItem(int)` first consults
 * an optional process-level override (set via
 * `SetItemFactoryOverride`) and falls back to the built-in dispatch
 * table when no override is registered. This preserves the behavior
 * previously provided by `FactoryManager::RegisterFactory` without a
 * class-scope singleton — callers that need a custom factory (plugins,
 * unit tests, DPM) install the override directly.
 *
 * N2-T5: EXPORT_API needed so the unittest binary can link these without
 * being swallowed by the project-wide -fvisibility=hidden on C++ TUs. */
EXPORT_API std::unique_ptr<AbstractItem> CreateItem(int type);
EXPORT_API AbstractItem& GetNullItem();

/**
 * @brief Registers a process-level IItemFactory override.
 * @since_tizen 8.0 (N3-T2 D8)
 * @details When an override is set, `CreateItem(type)` delegates to
 *          `override->CreateItem(type)`. Passing `nullptr` clears the
 *          override and restores the default dispatch.
 *
 *          This is the migration path away from
 *          `FactoryManager::GetInst().RegisterFactory(...)`. Ownership
 *          transfers to the override slot. The override is a plain
 *          internal std::unique_ptr guarded by a mutex; there is no
 *          class singleton involved.
 */
EXPORT_API void SetItemFactoryOverride(std::unique_ptr<IItemFactory> factory);

}  // namespace item
}  // namespace notification

#endif  // NOTIFICATION_EX_DEFAULT_ITEM_FACTORY_H_
