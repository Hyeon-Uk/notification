/*
 * Copyright (c) 2026 Samsung Electronics Co., Ltd.
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

#ifndef NOTIFICATION_RAII_NOTIFICATION_HANDLES_HH_
#define NOTIFICATION_RAII_NOTIFICATION_HANDLES_HH_

#include <memory>
#include <notification_setting.h>
#include <notification_setting_internal.h>

namespace notification::raii {

/* Phase 3 RAII wrappers for the notification-domain handle types
 * that are typedef'd to opaque struct pointers and follow the
 * `int <name>_free(handle)` cleanup convention.
 *
 * Replaces the explicit
 *   if (setting) notification_setting_free_notification(setting);
 *   if (system_setting) notification_system_setting_free_system_setting(system_setting);
 * cleanup lines.
 */

struct NotificationSettingDeleter {
  void operator()(notification_setting_h h) const noexcept {
    if (h != nullptr) notification_setting_free_notification(h);
  }
};
using NotificationSettingPtr = std::unique_ptr<
    std::remove_pointer_t<notification_setting_h>,
    NotificationSettingDeleter>;

struct NotificationSystemSettingDeleter {
  void operator()(notification_system_setting_h h) const noexcept {
    if (h != nullptr)
      notification_system_setting_free_system_setting(h);
  }
};
using NotificationSystemSettingPtr = std::unique_ptr<
    std::remove_pointer_t<notification_system_setting_h>,
    NotificationSystemSettingDeleter>;

}  // namespace notification::raii

#endif  // NOTIFICATION_RAII_NOTIFICATION_HANDLES_HH_
