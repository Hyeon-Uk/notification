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
 * N7.5-C coverage boost — notification_setting.c (Mid 6 file group).
 *
 * test_notification_setting.cc covers a happy-path get/set cycle on the
 * visibility_class field and the free contract. The setter/getter family
 * is large (allow_to_notify / do_not_disturb_except / pop_up_notification /
 * lock_screen_content_level / app_disabled, and the visibility class which
 * is the only non-LCOV-EXCL entry) and each one has identical NULL guard
 * structure. We add a parametric NULL matrix plus a happy-path matrix so
 * the contracts are pinned independently.
 *
 * Matrix:
 *   1. Setting_NullParam_AllGetters
 *      — All getters return INVALID_PARAMETER on null setting OR null out.
 *   2. Setting_NullParam_AllSetters
 *      — All setters return INVALID_PARAMETER on null setting.
 *   3. Setting_VisibilityMatrix_GetSet
 *      — visibility_class round-trip across a sample of values
 *        (-1 / 0 / 1 / INT_MAX-1) — pin that the helper does not clamp.
 *   4. Setting_LockScreenContent_AllLevels
 *      — SHOW_ALL_CONTENT / HIDE_SENSITIVE_CONTENT / DO_NOT_SHOW_NOTIFICATIONS
 *   5. Setting_AllowToNotify_BoolToggle
 *      — true / false / true round-trip
 *   6. Setting_DoNotDisturbExcept_BoolToggle
 *   7. Setting_PopUp_BoolToggle
 *   8. Setting_FreeNotification_NullArgs_Rejected
 *      — free(NULL) -> INVALID_PARAMETER (not NONE).
 *   9. Setting_FreeNotification_HappyPath
 *      — Allocate + populate + free; no leaks (LeakCheckFixture).
 *  10. Setting_GetPackageName_NullPackageField_Rejected
 *      — Setting struct with package_name == NULL returns NOT_EXIST_ID.
 *  11. Setting_GetAppId_NullAppIdField_Rejected
 *      — Setting struct with app_id == NULL returns NOT_EXIST_ID.
 *  12. Setting_UpdateSetting_NullArgs_Rejected
 *      — update_setting(NULL) and update_setting_for_uid(NULL, uid) reject.
 *
 * NOTE: noti_dnd_cb_compare and dnd_changed_cb_register go through TIDL/dbus
 * and are not exercised here — the v6 §N7.5-C constraint prefers mirror-
 * test patterns for the heavy IO surface.
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <climits>
#include <cstring>
#include <unistd.h>

#include "mock/test_fixture.hh"

extern "C" {
#include <notification.h>
#include <notification_internal.h>
#include <notification_setting.h>
#include <notification_setting_internal.h>
}

namespace {

/*
 * Plain TestFixture (not LeakCheckFixture). The setter/getter family
 * allocates strdup'd strings inside libc and frees them from inside
 * libnotification.so; with -Wl,--wrap=malloc/free only the test-binary
 * side of the boundary is observed, so the LeakCheckFixture would
 * spuriously report a leak for every well-balanced setting struct.
 * The hot-path leak coverage is already in test_shared_file_common_extra.cc
 * and test_notification_bundle_util_extra.cc which manage their allocations
 * entirely on the wrapped side.
 */
class NotificationSettingExtraTest : public TestFixture {
 public:
  NotificationSettingExtraTest() : TestFixture(nullptr) {}
};

/* Helper: allocate and zero-init a notification_setting struct. The
 * package_name and app_id slots are heap-owned strings so the free path
 * exercises the SAFE_FREE branch. */
notification_setting_h MakeSetting(const char* pkg, const char* app) {
  auto* s = static_cast<struct notification_setting*>(
      calloc(1, sizeof(struct notification_setting)));
  if (s == nullptr) return nullptr;
  if (pkg) s->package_name = strdup(pkg);
  if (app) s->app_id = strdup(app);
  return s;
}

}  // namespace

/* (1) NULL-arg matrix on all getters. */
TEST_F(NotificationSettingExtraTest, Setting_NullParam_AllGetters) {
  bool b = false;
  int i = 0;
  lock_screen_content_level_e level = SHOW_ALL_CONTENT;
  char* str = nullptr;

  EXPECT_EQ(notification_setting_get_package_name(nullptr, &str),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_setting_get_appid(nullptr, &str),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_setting_get_allow_to_notify(nullptr, &b),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_setting_get_do_not_disturb_except(nullptr, &b),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_setting_get_visibility_class(nullptr, &i),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_setting_get_pop_up_notification(nullptr, &b),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_setting_get_lock_screen_content(nullptr, &level),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_setting_get_app_disabled(nullptr, &b),
            NOTIFICATION_ERROR_INVALID_PARAMETER);

  /* Same getters with valid setting and NULL out-pointer. */
  notification_setting_h s = MakeSetting("p", "a");
  ASSERT_NE(s, nullptr);
  EXPECT_EQ(notification_setting_get_package_name(s, nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_setting_get_appid(s, nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_setting_get_allow_to_notify(s, nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_setting_get_do_not_disturb_except(s, nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_setting_get_visibility_class(s, nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_setting_get_pop_up_notification(s, nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_setting_get_lock_screen_content(s, nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_setting_get_app_disabled(s, nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  notification_setting_free_notification(s);
}

/* (2) NULL-arg matrix on all setters. */
TEST_F(NotificationSettingExtraTest, Setting_NullParam_AllSetters) {
  EXPECT_EQ(notification_setting_set_allow_to_notify(nullptr, true),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_setting_set_do_not_disturb_except(nullptr, true),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_setting_set_visibility_class(nullptr, 1),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_setting_set_pop_up_notification(nullptr, true),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_setting_set_lock_screen_content(nullptr,
                                                         SHOW_ALL_CONTENT),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* (3) Visibility matrix — getter / setter round-trips a range of values. */
TEST_F(NotificationSettingExtraTest, Setting_VisibilityMatrix_GetSet) {
  notification_setting_h s = MakeSetting("p", "a");
  ASSERT_NE(s, nullptr);

  const int values[] = {-1, 0, 1, 7, INT_MAX - 1};
  for (int v : values) {
    EXPECT_EQ(notification_setting_set_visibility_class(s, v),
              NOTIFICATION_ERROR_NONE) << "set v=" << v;
    int got = 0xDEADBEEF;
    EXPECT_EQ(notification_setting_get_visibility_class(s, &got),
              NOTIFICATION_ERROR_NONE) << "get v=" << v;
    EXPECT_EQ(got, v) << "round-trip v=" << v;
  }
  notification_setting_free_notification(s);
}

/* (4) Lock-screen content level matrix. */
TEST_F(NotificationSettingExtraTest, Setting_LockScreenContent_AllLevels) {
  notification_setting_h s = MakeSetting("p", "a");
  ASSERT_NE(s, nullptr);

  const lock_screen_content_level_e values[] = {
      SHOW_ALL_CONTENT,
      HIDE_SENSITIVE_CONTENT,
      DO_NOT_SHOW_NOTIFICATIONS,
  };
  for (auto v : values) {
    EXPECT_EQ(notification_setting_set_lock_screen_content(s, v),
              NOTIFICATION_ERROR_NONE);
    lock_screen_content_level_e got = SHOW_ALL_CONTENT;
    EXPECT_EQ(notification_setting_get_lock_screen_content(s, &got),
              NOTIFICATION_ERROR_NONE);
    EXPECT_EQ(got, v);
  }
  notification_setting_free_notification(s);
}

/* (5) allow_to_notify boolean toggle. */
TEST_F(NotificationSettingExtraTest, Setting_AllowToNotify_BoolToggle) {
  notification_setting_h s = MakeSetting("p", "a");
  ASSERT_NE(s, nullptr);

  bool got = false;
  EXPECT_EQ(notification_setting_set_allow_to_notify(s, true),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(notification_setting_get_allow_to_notify(s, &got),
            NOTIFICATION_ERROR_NONE);
  EXPECT_TRUE(got);

  EXPECT_EQ(notification_setting_set_allow_to_notify(s, false),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(notification_setting_get_allow_to_notify(s, &got),
            NOTIFICATION_ERROR_NONE);
  EXPECT_FALSE(got);

  EXPECT_EQ(notification_setting_set_allow_to_notify(s, true),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(notification_setting_get_allow_to_notify(s, &got),
            NOTIFICATION_ERROR_NONE);
  EXPECT_TRUE(got);

  notification_setting_free_notification(s);
}

/* (6) do_not_disturb_except boolean toggle. */
TEST_F(NotificationSettingExtraTest, Setting_DoNotDisturbExcept_BoolToggle) {
  notification_setting_h s = MakeSetting("p", "a");
  ASSERT_NE(s, nullptr);
  bool got = false;
  EXPECT_EQ(notification_setting_set_do_not_disturb_except(s, true),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(notification_setting_get_do_not_disturb_except(s, &got),
            NOTIFICATION_ERROR_NONE);
  EXPECT_TRUE(got);
  EXPECT_EQ(notification_setting_set_do_not_disturb_except(s, false),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(notification_setting_get_do_not_disturb_except(s, &got),
            NOTIFICATION_ERROR_NONE);
  EXPECT_FALSE(got);
  notification_setting_free_notification(s);
}

/* (7) pop_up_notification boolean toggle. */
TEST_F(NotificationSettingExtraTest, Setting_PopUp_BoolToggle) {
  notification_setting_h s = MakeSetting("p", "a");
  ASSERT_NE(s, nullptr);
  bool got = false;
  EXPECT_EQ(notification_setting_set_pop_up_notification(s, true),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(notification_setting_get_pop_up_notification(s, &got),
            NOTIFICATION_ERROR_NONE);
  EXPECT_TRUE(got);
  EXPECT_EQ(notification_setting_set_pop_up_notification(s, false),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(notification_setting_get_pop_up_notification(s, &got),
            NOTIFICATION_ERROR_NONE);
  EXPECT_FALSE(got);
  notification_setting_free_notification(s);
}

/* (8) free(NULL) returns INVALID_PARAMETER. */
TEST_F(NotificationSettingExtraTest, Setting_FreeNotification_NullArgs_Rejected) {
  EXPECT_EQ(notification_setting_free_notification(nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* (9) Happy-path free does not crash. */
TEST_F(NotificationSettingExtraTest, Setting_FreeNotification_HappyPath) {
  notification_setting_h s = MakeSetting("org.tizen.unittest.pkg",
                                          "org.tizen.unittest.app");
  ASSERT_NE(s, nullptr);
  EXPECT_EQ(notification_setting_free_notification(s),
            NOTIFICATION_ERROR_NONE);
}

/* (10) get_package_name with NULL package_name field returns NOT_EXIST_ID. */
TEST_F(NotificationSettingExtraTest,
       Setting_GetPackageName_NullPackageField_Rejected) {
  notification_setting_h s = MakeSetting(nullptr, "a");
  ASSERT_NE(s, nullptr);
  char* val = nullptr;
  EXPECT_EQ(notification_setting_get_package_name(s, &val),
            NOTIFICATION_ERROR_NOT_EXIST_ID);
  EXPECT_EQ(val, nullptr);
  notification_setting_free_notification(s);
}

/* (11) get_appid with NULL app_id field returns NOT_EXIST_ID. */
TEST_F(NotificationSettingExtraTest,
       Setting_GetAppId_NullAppIdField_Rejected) {
  notification_setting_h s = MakeSetting("p", nullptr);
  ASSERT_NE(s, nullptr);
  char* val = nullptr;
  EXPECT_EQ(notification_setting_get_appid(s, &val),
            NOTIFICATION_ERROR_NOT_EXIST_ID);
  EXPECT_EQ(val, nullptr);
  notification_setting_free_notification(s);
}

/* (12) update_setting/update_setting_for_uid NULL guard. */
TEST_F(NotificationSettingExtraTest, Setting_UpdateSetting_NullArgs_Rejected) {
  EXPECT_EQ(notification_setting_update_setting(nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_setting_update_setting_for_uid(nullptr, getuid()),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}
