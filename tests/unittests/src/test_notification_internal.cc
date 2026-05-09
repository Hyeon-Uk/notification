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

#include <gtest/gtest.h>

#include "notification/include/notification.h"
#include "notification/include/notification_internal.h"

// notification_internal.h exposes ~109 internal-API symbols
// consumed by data-provider-master and notification-init.
// Symbol-presence is enforced by the ABI snapshot
// (tests/abi/snapshot-0.12.1.txt). Behavioral lock-in for the
// individual symbols requires real DB / glib / rpc-port and
// lives in the integration suite. Below: smoke-level
// reachability for representative families.

namespace {
class NotificationInternalTest : public ::testing::Test {};
}  // namespace

TEST_F(NotificationInternalTest, channel_family_symbols_reachable) {
  EXPECT_NE(reinterpret_cast<void*>(&notification_channel_add),     nullptr);
  EXPECT_NE(reinterpret_cast<void*>(&notification_channel_clone),   nullptr);
  EXPECT_NE(reinterpret_cast<void*>(&notification_channel_create),  nullptr);
  EXPECT_NE(reinterpret_cast<void*>(&notification_channel_foreach), nullptr);
  EXPECT_NE(reinterpret_cast<void*>(&notification_channel_free),    nullptr);
  EXPECT_NE(reinterpret_cast<void*>(&notification_channel_get_block),
            nullptr);
  EXPECT_NE(reinterpret_cast<void*>(&notification_channel_get_blockable),
            nullptr);
  EXPECT_NE(reinterpret_cast<void*>(&notification_channel_get_by_name),
            nullptr);
  EXPECT_NE(reinterpret_cast<void*>(&notification_channel_get_name),
            nullptr);
  EXPECT_NE(reinterpret_cast<void*>(&notification_channel_remove),  nullptr);
  EXPECT_NE(reinterpret_cast<void*>(&notification_channel_set_block),
            nullptr);
  EXPECT_NE(reinterpret_cast<void*>(&notification_channel_set_blockable),
            nullptr);
  EXPECT_NE(reinterpret_cast<void*>(&notification_channel_update),  nullptr);
}

TEST_F(NotificationInternalTest, delete_family_symbols_reachable) {
  EXPECT_NE(reinterpret_cast<void*>(&notification_delete_all),
            nullptr);
  EXPECT_NE(reinterpret_cast<void*>(&notification_delete_all_by_type),
            nullptr);
  EXPECT_NE(reinterpret_cast<void*>(&notification_delete_all_for_uid),
            nullptr);
  EXPECT_NE(reinterpret_cast<void*>(&notification_delete_by_priv_id),
            nullptr);
  EXPECT_NE(reinterpret_cast<void*>(&notification_delete_for_uid),
            nullptr);
  EXPECT_NE(reinterpret_cast<void*>(&notification_delete_group_by_priv_id),
            nullptr);
  EXPECT_NE(reinterpret_cast<void*>(&notification_delete_group_by_group_id),
            nullptr);
  EXPECT_NE(reinterpret_cast<void*>(&notification_delete_by_display_applist),
            nullptr);
}

TEST_F(NotificationInternalTest, post_update_load_family_symbols_reachable) {
  EXPECT_NE(reinterpret_cast<void*>(&notification_create_from_template),
            nullptr);
  EXPECT_NE(reinterpret_cast<void*>(&notification_create_from_package_template),
            nullptr);
  EXPECT_NE(reinterpret_cast<void*>(&notification_clear),
            nullptr);
  EXPECT_NE(reinterpret_cast<void*>(&notification_check_event_receiver_available),
            nullptr);
}

TEST_F(NotificationInternalTest, deferred_task_family_symbols_reachable) {
  EXPECT_NE(reinterpret_cast<void*>(&notification_add_deferred_task),
            nullptr);
  EXPECT_NE(reinterpret_cast<void*>(&notification_del_deferred_task),
            nullptr);
}
