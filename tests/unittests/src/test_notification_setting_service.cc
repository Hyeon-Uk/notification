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
#include "notification/include/notification_setting_internal.h"
#include "notification/include/notification_setting_service.h"

namespace {
class NotificationSettingServiceTest : public ::testing::Test {};
}  // namespace

TEST_F(NotificationSettingServiceTest, refresh_setting_table_reachable) {
  EXPECT_NE(reinterpret_cast<void*>(&notification_setting_refresh_setting_table),
            nullptr);
}

TEST_F(NotificationSettingServiceTest, db_update_system_setting_reachable) {
  EXPECT_NE(reinterpret_cast<void*>(&notification_setting_db_update_system_setting),
            nullptr);
}

TEST_F(NotificationSettingServiceTest, system_setting_dnd_schedule_reachable) {
  EXPECT_NE(reinterpret_cast<void*>(&notification_system_setting_dnd_schedule_get_enabled),
            nullptr);
  EXPECT_NE(reinterpret_cast<void*>(&notification_system_setting_dnd_schedule_set_enabled),
            nullptr);
}
