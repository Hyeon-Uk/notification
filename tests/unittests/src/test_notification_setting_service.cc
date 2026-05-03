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

/*
 * Phase 1.5-E.2.  notification_setting_service.c is largely
 * LCOV-EXCL'd at source (the file uses real sqlite3 paths that
 * require a writable /opt/dbspace).  These tests defend the
 * input-validation branches that fire before notification_db_open()
 * -- safe to assert without TestFixture.
 */

#include <gtest/gtest.h>

#include "notification/include/notification_error.h"

extern "C" {
#include <notification_setting.h>
#include <notification_setting_internal.h>
#include <notification_setting_service.h>
}

namespace {

class NotificationSettingServiceTest : public ::testing::Test {};

TEST_F(NotificationSettingServiceTest,
       get_setting_by_app_id_rejects_null_app_id) {
  notification_setting_h setting = nullptr;
  EXPECT_EQ(NOTIFICATION_ERROR_INVALID_PARAMETER,
      noti_setting_service_get_setting_by_app_id(nullptr, &setting, 0));
}

TEST_F(NotificationSettingServiceTest,
       get_setting_by_app_id_rejects_null_setting) {
  EXPECT_EQ(NOTIFICATION_ERROR_INVALID_PARAMETER,
      noti_setting_service_get_setting_by_app_id("app", nullptr, 0));
}

TEST_F(NotificationSettingServiceTest,
       db_update_rejects_null_package_name) {
  EXPECT_EQ(NOTIFICATION_ERROR_INVALID_PARAMETER,
      notification_setting_db_update(nullptr, "app_id",
          0, 0, 0, 0, 0, 0));
}

TEST_F(NotificationSettingServiceTest,
       db_update_rejects_null_app_id) {
  EXPECT_EQ(NOTIFICATION_ERROR_INVALID_PARAMETER,
      notification_setting_db_update("pkg", nullptr,
          0, 0, 0, 0, 0, 0));
}

}  // namespace
