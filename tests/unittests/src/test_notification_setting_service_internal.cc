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
 * N7.5-C coverage boost — notification_setting_service.c (Mid 6 file group).
 *
 * notification_setting_service.c is a server-side helper. Most entry points
 * begin by validating their arguments before opening the notification setting
 * database. We pin those NULL/parameter rejection branches so refactors do
 * not silently relax them. The db-access body of each function is wrapped in
 * LCOV_EXCL_START/STOP because it would require either a hermetic sqlite
 * fixture or the production /opt/dbspace files; we deliberately do NOT add
 * either as that would breach v6 §N7.5-C "no heavy mocks new".
 *
 * Matrix:
 *   1. SettingService_GetByAppId_NullAppId_RejectedSync
 *      — null app_id rejected.
 *   2. SettingService_GetByAppId_NullSetting_RejectedSync
 *      — null setting out-pointer rejected.
 *   3. SettingService_GetSettingArray_NullArray_RejectedSync
 *      — null array out-pointer rejected.
 *   4. SettingService_GetSettingArray_NullCount_RejectedSync
 *      — null count out-pointer rejected.
 *   5. SettingService_LoadSystemSetting_NullArgs_RejectedSync
 *      — null system_setting out-pointer rejected.
 *   6. SettingService_GetDndAndAllowToNotify_NullAppId_RejectedSync
 *      — null app_id rejected (only this arg is currently checked).
 *   7. SettingService_LoadDndAllowException_NullExceptionPtr_RejectedSync
 *      — null exception out-pointer rejected.
 *   8. SettingService_DbUpdate_NullPackageName_RejectedSync
 *      — notification_setting_db_update null package_name rejected.
 *   9. SettingService_DbUpdate_NullAppId_RejectedSync
 *      — notification_setting_db_update null app_id rejected.
 *  10. SettingService_UpdateAppDisabled_NullAppId_RejectedSync
 *      — notification_setting_db_update_app_disabled null app_id rejected.
 *  11. SettingService_UpdatePkgDisabled_NullPkgId_RejectedSync
 *      — notification_setting_db_update_pkg_disabled null pkg_id rejected.
 */

#include <gtest/gtest.h>

#include <unistd.h>

extern "C" {
#include <notification.h>
#include <notification_internal.h>
#include <notification_setting_internal.h>
#include <notification_setting_service.h>
}

namespace {

class NotificationSettingServiceInternalTest : public ::testing::Test {};

}  // namespace

/* (1) get_setting_by_app_id null app_id rejection. */
TEST_F(NotificationSettingServiceInternalTest,
       SettingService_GetByAppId_NullAppId_RejectedSync) {
  notification_setting_h setting = nullptr;
  EXPECT_EQ(noti_setting_service_get_setting_by_app_id(nullptr, &setting,
                                                      getuid()),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* (2) get_setting_by_app_id null setting out-pointer rejection. */
TEST_F(NotificationSettingServiceInternalTest,
       SettingService_GetByAppId_NullSetting_RejectedSync) {
  EXPECT_EQ(noti_setting_service_get_setting_by_app_id("org.tizen.x", nullptr,
                                                      getuid()),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* (3) get_setting_array null array out-pointer rejection. */
TEST_F(NotificationSettingServiceInternalTest,
       SettingService_GetSettingArray_NullArray_RejectedSync) {
  int count = 0;
  EXPECT_EQ(noti_setting_get_setting_array(nullptr, &count, getuid()),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* (4) get_setting_array null count out-pointer rejection. */
TEST_F(NotificationSettingServiceInternalTest,
       SettingService_GetSettingArray_NullCount_RejectedSync) {
  notification_setting_h arr = nullptr;
  EXPECT_EQ(noti_setting_get_setting_array(&arr, nullptr, getuid()),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* (5) load_system_setting null out-pointer rejection. */
TEST_F(NotificationSettingServiceInternalTest,
       SettingService_LoadSystemSetting_NullArgs_RejectedSync) {
  EXPECT_EQ(noti_system_setting_load_system_setting(nullptr, getuid()),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* (6) get_dnd_and_allow_to_notify null app_id rejection. */
TEST_F(NotificationSettingServiceInternalTest,
       SettingService_GetDndAndAllowToNotify_NullAppId_RejectedSync) {
  int dnd = 0, except = 0, allow = 0;
  EXPECT_EQ(notification_get_dnd_and_allow_to_notify(nullptr, &dnd, &except,
                                                     &allow, getuid()),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* (7) load_dnd_allow_exception null exception out-pointer rejection. */
TEST_F(NotificationSettingServiceInternalTest,
       SettingService_LoadDndAllowException_NullExceptionPtr_RejectedSync) {
  int count = 0;
  EXPECT_EQ(notification_system_setting_load_dnd_allow_exception(
                nullptr, &count, getuid()),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* (8) db_update null package_name rejection. */
TEST_F(NotificationSettingServiceInternalTest,
       SettingService_DbUpdate_NullPackageName_RejectedSync) {
  EXPECT_EQ(notification_setting_db_update(nullptr, "org.tizen.x", 0, 0, 0, 0,
                                          0, getuid()),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* (9) db_update null app_id rejection. */
TEST_F(NotificationSettingServiceInternalTest,
       SettingService_DbUpdate_NullAppId_RejectedSync) {
  EXPECT_EQ(notification_setting_db_update("org.tizen.pkg", nullptr, 0, 0, 0, 0,
                                          0, getuid()),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* (10) update_app_disabled null app_id rejection. */
TEST_F(NotificationSettingServiceInternalTest,
       SettingService_UpdateAppDisabled_NullAppId_RejectedSync) {
  EXPECT_EQ(notification_setting_db_update_app_disabled(nullptr, true, getuid()),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* (11) update_pkg_disabled null pkg_id rejection. */
TEST_F(NotificationSettingServiceInternalTest,
       SettingService_UpdatePkgDisabled_NullPkgId_RejectedSync) {
  EXPECT_EQ(notification_setting_db_update_pkg_disabled(nullptr, true, getuid()),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}
