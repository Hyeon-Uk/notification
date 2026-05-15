/*
 * Copyright (c) 2026 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <stdlib.h>
#include <string.h>

#include "mock/aul_mock.hh"
#include "mock/glib_mock.hh"
#include "mock/noti_service_proxy_mock.hh"
#include "mock/package_manager_mock.hh"
#include "mock/rpc_port_mock.hh"
#include "mock/test_fixture.hh"
#include "mock/tzplatform_config_mock.hh"

#include "notification/include/notification.h"
#include "notification/include/notification_setting.h"
#include "notification/include/notification_setting_internal.h"

using ::testing::_;
using ::testing::Return;

namespace {

class Mocks :
    virtual public ::testing::NiceMock<AulMock>,
    virtual public ::testing::NiceMock<TzplatformConfigMock>,
    virtual public ::testing::NiceMock<GlibMock>,
    virtual public ::testing::NiceMock<NotiServiceProxyMock>,
    virtual public ::testing::NiceMock<PackageManagerMock>,
    virtual public ::testing::NiceMock<RpcPortMock> {};

/* Construct a heap-allocated setting struct with sensible defaults.
 * struct notification_setting is a POD with malloc'd char* fields so we
 * can build one and free it ourselves without going through any DB or
 * IPC code path. notification_setting_free_notification will free the
 * char* members and the struct itself. */
notification_setting_h make_setting(const char* package_name,
                                    const char* app_id) {
  auto* s = static_cast<notification_setting_h>(
      calloc(1, sizeof(struct notification_setting)));
  if (package_name) s->package_name = strdup(package_name);
  if (app_id) s->app_id = strdup(app_id);
  s->allow_to_notify = true;
  s->do_not_disturb_except = false;
  s->pop_up_notification = true;
  s->visibility_class = 0;
  s->lock_screen_content_level = SHOW_ALL_CONTENT;
  s->app_disabled = false;
  return s;
}

}  // namespace

class NotificationSettingApiTest : public TestFixture {
 public:
  NotificationSettingApiTest() : TestFixture(std::make_unique<::Mocks>()) {}
};

/* ============================================================== */
/* get_package_name / get_appid                                   */
/* ============================================================== */

TEST_F(NotificationSettingApiTest, get_package_name_roundtrip) {
  notification_setting_h s = make_setting("my.pkg", "my.app");
  char* out = nullptr;
  EXPECT_EQ(notification_setting_get_package_name(s, &out),
            NOTIFICATION_ERROR_NONE);
  EXPECT_STREQ(out, "my.pkg");
  free(out);
  notification_setting_free_notification(s);
}

TEST_F(NotificationSettingApiTest, get_package_name_null_args) {
  notification_setting_h s = make_setting("my.pkg", "my.app");
  char* out = nullptr;
  EXPECT_EQ(notification_setting_get_package_name(nullptr, &out),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_setting_get_package_name(s, nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  notification_setting_free_notification(s);
}

TEST_F(NotificationSettingApiTest, get_appid_roundtrip) {
  notification_setting_h s = make_setting("my.pkg", "my.app");
  char* out = nullptr;
  EXPECT_EQ(notification_setting_get_appid(s, &out),
            NOTIFICATION_ERROR_NONE);
  EXPECT_STREQ(out, "my.app");
  free(out);
  notification_setting_free_notification(s);
}

TEST_F(NotificationSettingApiTest, get_appid_null_args) {
  notification_setting_h s = make_setting("my.pkg", "my.app");
  char* out = nullptr;
  EXPECT_EQ(notification_setting_get_appid(nullptr, &out),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_setting_get_appid(s, nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  notification_setting_free_notification(s);
}

/* ============================================================== */
/* allow_to_notify                                                */
/* ============================================================== */

TEST_F(NotificationSettingApiTest, set_allow_to_notify_null_setting) {
  EXPECT_EQ(notification_setting_set_allow_to_notify(nullptr, true),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* ============================================================== */
/* do_not_disturb_except                                          */
/* ============================================================== */

TEST_F(NotificationSettingApiTest, dnd_except_null_args) {
  bool v = false;
  EXPECT_EQ(notification_setting_set_do_not_disturb_except(nullptr, true),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* ============================================================== */
/* pop_up_notification                                            */
/* ============================================================== */

TEST_F(NotificationSettingApiTest, pop_up_notification_null_args) {
  bool v = false;
  EXPECT_EQ(notification_setting_set_pop_up_notification(nullptr, true),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* ============================================================== */
/* lock_screen_content                                            */
/* ============================================================== */

TEST_F(NotificationSettingApiTest, lock_screen_content_roundtrip) {
  notification_setting_h s = make_setting("p", "a");
  ASSERT_EQ(notification_setting_set_lock_screen_content(s,
                HIDE_SENSITIVE_CONTENT),
            NOTIFICATION_ERROR_NONE);
  lock_screen_content_level_e v = SHOW_ALL_CONTENT;
  EXPECT_EQ(notification_setting_get_lock_screen_content(s, &v),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(v, HIDE_SENSITIVE_CONTENT);
  notification_setting_free_notification(s);
}

TEST_F(NotificationSettingApiTest, lock_screen_content_null_args) {
  lock_screen_content_level_e v = SHOW_ALL_CONTENT;
  EXPECT_EQ(notification_setting_set_lock_screen_content(nullptr,
                HIDE_SENSITIVE_CONTENT),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_setting_get_lock_screen_content(nullptr, &v),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  notification_setting_h s = make_setting("p", "a");
  EXPECT_EQ(notification_setting_get_lock_screen_content(s, nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  notification_setting_free_notification(s);
}

/* ============================================================== */
/* visibility_class                                               */
/* ============================================================== */

TEST_F(NotificationSettingApiTest, visibility_class_roundtrip) {
  notification_setting_h s = make_setting("p", "a");
  ASSERT_EQ(notification_setting_set_visibility_class(s, 7),
            NOTIFICATION_ERROR_NONE);
  int v = 0;
  EXPECT_EQ(notification_setting_get_visibility_class(s, &v),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(v, 7);
  notification_setting_free_notification(s);
}

TEST_F(NotificationSettingApiTest, visibility_class_null_args) {
  int v = 0;
  EXPECT_EQ(notification_setting_set_visibility_class(nullptr, 1),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_setting_get_visibility_class(nullptr, &v),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  notification_setting_h s = make_setting("p", "a");
  EXPECT_EQ(notification_setting_get_visibility_class(s, nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  notification_setting_free_notification(s);
}


/* ============================================================== */
/* free_notification                                              */
/* ============================================================== */

TEST_F(NotificationSettingApiTest, free_notification_null_returns_invalid) {
  EXPECT_EQ(notification_setting_free_notification(nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationSettingApiTest, free_notification_valid_returns_none) {
  notification_setting_h s = make_setting("p", "a");
  EXPECT_EQ(notification_setting_free_notification(s),
            NOTIFICATION_ERROR_NONE);
}

/* ============================================================== */
/* IPC-backed entry points — verify NULL-parameter validation     */
/* (real call paths require server)                                */
/* ============================================================== */

TEST_F(NotificationSettingApiTest, get_setting_array_null_args) {
  int n = 0;
  notification_setting_h arr = nullptr;
  EXPECT_EQ(notification_setting_get_setting_array(nullptr, &n),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_setting_get_setting_array(&arr, nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationSettingApiTest, get_setting_by_package_name_null_args) {
  notification_setting_h s = nullptr;
  EXPECT_EQ(notification_setting_get_setting_by_package_name(nullptr, &s),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_setting_get_setting_by_package_name("p", nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* ============================================================== */
/* notification_system_setting_* on system_setting struct         */
/* ============================================================== */

TEST_F(NotificationSettingApiTest, sys_do_not_disturb_null_args) {
  bool v = false;
  EXPECT_EQ(notification_system_setting_set_do_not_disturb(nullptr, true),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationSettingApiTest, sys_dnd_schedule_enabled_null_args) {
  bool v = false;
  EXPECT_EQ(notification_system_setting_dnd_schedule_set_enabled(nullptr, true),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationSettingApiTest, sys_dnd_schedule_day_null_args) {
  EXPECT_EQ(notification_system_setting_dnd_schedule_set_day(nullptr, 1),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationSettingApiTest, sys_dnd_start_time_null_args) {
  EXPECT_EQ(notification_system_setting_dnd_schedule_set_start_time(nullptr,
                0, 0),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* ============================================================== */
/* system_setting_get/set_dnd_allow_exceptions                     */
/* ============================================================== */

TEST_F(NotificationSettingApiTest, sys_dnd_allow_exception_null_args) {
  int v;
  EXPECT_EQ(notification_system_setting_set_dnd_allow_exceptions(nullptr,
                NOTIFICATION_DND_ALLOWED_CALLS, 0),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_system_setting_get_dnd_allow_exceptions(nullptr,
                NOTIFICATION_DND_ALLOWED_CALLS, &v),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* ============================================================== */
/* register/unregister_system_setting_dnd_changed_cb — null cb     */
/* ============================================================== */

TEST_F(NotificationSettingApiTest, register_system_setting_dnd_changed_cb_null) {
  EXPECT_EQ(notification_register_system_setting_dnd_changed_cb(nullptr,
                nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationSettingApiTest, unregister_system_setting_dnd_changed_cb_null) {
  EXPECT_EQ(notification_unregister_system_setting_dnd_changed_cb(nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationSettingApiTest, register_system_setting_dnd_changed_cb_for_uid_null) {
  EXPECT_EQ(notification_register_system_setting_dnd_changed_cb_for_uid(
                nullptr, nullptr, 1000),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationSettingApiTest, unregister_system_setting_dnd_changed_cb_for_uid_null) {
  EXPECT_EQ(notification_unregister_system_setting_dnd_changed_cb_for_uid(
                nullptr, 1000),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* ============================================================== */
/* notification_setting_get_setting_array_for_uid — null args      */
/* ============================================================== */

TEST_F(NotificationSettingApiTest, get_setting_array_for_uid_null_args) {
  int n = 0;
  notification_setting_h arr = nullptr;
  EXPECT_EQ(notification_setting_get_setting_array_for_uid(nullptr, &n,
                1000),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_setting_get_setting_array_for_uid(&arr, nullptr,
                1000),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* ============================================================== */
/* notification_setting_get_setting_by_appid_for_uid — null args   */
/* ============================================================== */

TEST_F(NotificationSettingApiTest, get_setting_by_appid_for_uid_null_args) {
  notification_setting_h s = nullptr;
  EXPECT_EQ(notification_setting_get_setting_by_appid_for_uid(nullptr,
                &s, 1000),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_setting_get_setting_by_appid_for_uid("app",
                nullptr, 1000),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* ============================================================== */
/* notification_system_setting_load_system_setting_for_uid — null  */
/* ============================================================== */

TEST_F(NotificationSettingApiTest,
       system_setting_load_system_setting_for_uid_null_args) {
  EXPECT_EQ(notification_system_setting_load_system_setting_for_uid(nullptr,
                1000),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* ============================================================== */
/* notification_setting_update_setting / _for_uid — null arg       */
/* ============================================================== */

TEST_F(NotificationSettingApiTest, update_setting_null_arg) {
  EXPECT_EQ(notification_setting_update_setting(nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationSettingApiTest, update_setting_for_uid_null_arg) {
  EXPECT_EQ(notification_setting_update_setting_for_uid(nullptr, 1000),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* ============================================================== */
/* notification_setting_get_lock_screen_content / set_lock_screen   */
/* ─ extra null-arg coverage (symmetric)                            */
/* ============================================================== */

TEST_F(NotificationSettingApiTest, set_lock_screen_content_with_show_all) {
  notification_setting_h s = make_setting("p", "a");
  ASSERT_EQ(notification_setting_set_lock_screen_content(s,
                SHOW_ALL_CONTENT),
            NOTIFICATION_ERROR_NONE);
  lock_screen_content_level_e v = HIDE_SENSITIVE_CONTENT;
  EXPECT_EQ(notification_setting_get_lock_screen_content(s, &v),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(v, SHOW_ALL_CONTENT);
  notification_setting_free_notification(s);
}


/* ============================================================== */
/* visibility_class — various values stored verbatim                */
/* ============================================================== */

TEST_F(NotificationSettingApiTest, visibility_class_negative_stored) {
  notification_setting_h s = make_setting("p", "a");
  ASSERT_EQ(notification_setting_set_visibility_class(s, -1),
            NOTIFICATION_ERROR_NONE);
  int v = 0;
  EXPECT_EQ(notification_setting_get_visibility_class(s, &v),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(v, -1);
  notification_setting_free_notification(s);
}


/* ============================================================== */
/* package_name and appid string survives setter                    */
/* ============================================================== */

TEST_F(NotificationSettingApiTest, get_package_name_after_setting_replacement) {
  notification_setting_h s = make_setting("first.pkg", "first.app");
  /* Direct field replacement (not via setter — there is no setter for
   * pkg/app name; the structure carries them from the underlying
   * DB row). */
  if (s->package_name) free(s->package_name);
  s->package_name = strdup("changed.pkg");
  char* out = nullptr;
  EXPECT_EQ(notification_setting_get_package_name(s, &out),
            NOTIFICATION_ERROR_NONE);
  EXPECT_STREQ(out, "changed.pkg");
  free(out);
  notification_setting_free_notification(s);
}


/* ============================================================== */
/* visibility_class INT_MAX edge value                              */
/* ============================================================== */

TEST_F(NotificationSettingApiTest, visibility_class_int_max) {
  notification_setting_h s = make_setting("p", "a");
  ASSERT_EQ(notification_setting_set_visibility_class(s, 2147483647),
            NOTIFICATION_ERROR_NONE);
  int v = 0;
  EXPECT_EQ(notification_setting_get_visibility_class(s, &v),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(v, 2147483647);
  notification_setting_free_notification(s);
}

/* ============================================================== */
/* visibility_class zero                                            */
/* ============================================================== */

TEST_F(NotificationSettingApiTest, visibility_class_zero) {
  notification_setting_h s = make_setting("p", "a");
  ASSERT_EQ(notification_setting_set_visibility_class(s, 0),
            NOTIFICATION_ERROR_NONE);
  int v = 99;
  EXPECT_EQ(notification_setting_get_visibility_class(s, &v),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(v, 0);
  notification_setting_free_notification(s);
}

/* ============================================================== */
/* free_notification with NULL fields                              */
/* ============================================================== */

TEST_F(NotificationSettingApiTest, free_with_null_fields) {
  /* setting with NULL package/app fields — free should still succeed. */
  notification_setting_h s = make_setting(nullptr, nullptr);
  EXPECT_EQ(notification_setting_free_notification(s),
            NOTIFICATION_ERROR_NONE);
}

/* ============================================================== */
/* visibility_class default                                         */
/* ============================================================== */

TEST_F(NotificationSettingApiTest, visibility_class_default_zero) {
  notification_setting_h s = make_setting("p", "a");
  int v = 99;
  EXPECT_EQ(notification_setting_get_visibility_class(s, &v),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(v, 0);
  notification_setting_free_notification(s);
}

/* ============================================================== */
/* lock_screen_content default level                                */
/* ============================================================== */

TEST_F(NotificationSettingApiTest, lock_screen_content_default_show_all) {
  notification_setting_h s = make_setting("p", "a");
  lock_screen_content_level_e v = HIDE_SENSITIVE_CONTENT;
  EXPECT_EQ(notification_setting_get_lock_screen_content(s, &v),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(v, SHOW_ALL_CONTENT);  /* make_setting initialized */
  notification_setting_free_notification(s);
}

/* ============================================================== */
/* lock_screen_content all enum values roundtrip                   */
/* ============================================================== */

TEST_F(NotificationSettingApiTest, lock_screen_content_all_levels) {
  notification_setting_h s = make_setting("p", "a");
  lock_screen_content_level_e levels[] = {
      SHOW_ALL_CONTENT,
      HIDE_SENSITIVE_CONTENT,
      DO_NOT_SHOW_NOTIFICATIONS,
  };
  for (size_t i = 0; i < sizeof(levels) / sizeof(levels[0]); ++i) {
    ASSERT_EQ(notification_setting_set_lock_screen_content(s, levels[i]),
              NOTIFICATION_ERROR_NONE);
    lock_screen_content_level_e v = SHOW_ALL_CONTENT;
    EXPECT_EQ(notification_setting_get_lock_screen_content(s, &v),
              NOTIFICATION_ERROR_NONE);
    EXPECT_EQ(v, levels[i]);
  }
  notification_setting_free_notification(s);
}

/* ============================================================== */
/* visibility_class large negative                                  */
/* ============================================================== */

TEST_F(NotificationSettingApiTest, visibility_class_int_min) {
  notification_setting_h s = make_setting("p", "a");
  ASSERT_EQ(notification_setting_set_visibility_class(s, -2147483648),
            NOTIFICATION_ERROR_NONE);
  int v = 0;
  EXPECT_EQ(notification_setting_get_visibility_class(s, &v),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(v, -2147483648);
  notification_setting_free_notification(s);
}

/* Note: notification_setting_get_setting resolves the app_id via aul
 * before any null-arg check, so it requires an AulMock — left out of
 * this fixture to keep mock setup minimal. */

/* ============================================================== */
/* notification_system_setting_load/update_system_setting          */
/* null-arg validation                                              */
/* ============================================================== */

TEST_F(NotificationSettingApiTest, sys_load_system_setting_null_out) {
  EXPECT_EQ(notification_system_setting_load_system_setting(nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationSettingApiTest, sys_load_system_setting_for_uid_null_out) {
  EXPECT_EQ(notification_system_setting_load_system_setting_for_uid(
                nullptr, 5001),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationSettingApiTest, sys_update_system_setting_null_handle) {
  EXPECT_EQ(notification_system_setting_update_system_setting(nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationSettingApiTest, sys_update_system_setting_for_uid_null_handle) {
  EXPECT_EQ(notification_system_setting_update_system_setting_for_uid(
                nullptr, 5001),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* ============================================================== */
/* notification_setting_update_setting / free_notification         */
/* null-arg validation                                              */
/* ============================================================== */

TEST_F(NotificationSettingApiTest, update_setting_null_handle) {
  EXPECT_EQ(notification_setting_update_setting(nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationSettingApiTest, update_setting_for_uid_null_handle) {
  EXPECT_EQ(notification_setting_update_setting_for_uid(nullptr, 5001),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationSettingApiTest, free_notification_null_handle) {
  EXPECT_EQ(notification_setting_free_notification(nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* ============================================================== */
/* get_package_name / get_appid — NOT_EXIST_ID path                 */
/* ============================================================== */

TEST_F(NotificationSettingApiTest,
       get_package_name_when_unset_returns_not_exist) {
  notification_setting_h s = make_setting(nullptr, "an.app");
  char* out = nullptr;
  EXPECT_EQ(notification_setting_get_package_name(s, &out),
            NOTIFICATION_ERROR_NOT_EXIST_ID);
  notification_setting_free_notification(s);
}

TEST_F(NotificationSettingApiTest,
       get_appid_when_unset_returns_not_exist) {
  notification_setting_h s = make_setting("a.pkg", nullptr);
  char* out = nullptr;
  EXPECT_EQ(notification_setting_get_appid(s, &out),
            NOTIFICATION_ERROR_NOT_EXIST_ID);
  notification_setting_free_notification(s);
}

/* ============================================================== */
/* visibility_class boundary roundtrips                            */
/* ============================================================== */

TEST_F(NotificationSettingApiTest, visibility_class_zero_roundtrip) {
  notification_setting_h s = make_setting("p", "a");
  ASSERT_EQ(notification_setting_set_visibility_class(s, 0),
            NOTIFICATION_ERROR_NONE);
  int v = -1;
  EXPECT_EQ(notification_setting_get_visibility_class(s, &v),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(v, 0);
  notification_setting_free_notification(s);
}

TEST_F(NotificationSettingApiTest, visibility_class_int_max_roundtrip) {
  notification_setting_h s = make_setting("p", "a");
  ASSERT_EQ(notification_setting_set_visibility_class(s, 2147483647),
            NOTIFICATION_ERROR_NONE);
  int v = 0;
  EXPECT_EQ(notification_setting_get_visibility_class(s, &v),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(v, 2147483647);
  notification_setting_free_notification(s);
}

/* ============================================================== */
/* lock_screen_content all three levels — both setting + system    */
/* ============================================================== */

TEST_F(NotificationSettingApiTest,
       lock_screen_content_show_all_roundtrip_again) {
  notification_setting_h s = make_setting("p", "a");
  ASSERT_EQ(notification_setting_set_lock_screen_content(s,
                SHOW_ALL_CONTENT),
            NOTIFICATION_ERROR_NONE);
  lock_screen_content_level_e v = HIDE_SENSITIVE_CONTENT;
  EXPECT_EQ(notification_setting_get_lock_screen_content(s, &v),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(v, SHOW_ALL_CONTENT);
  notification_setting_free_notification(s);
}

TEST_F(NotificationSettingApiTest,
       lock_screen_content_hide_sensitive_roundtrip_again) {
  notification_setting_h s = make_setting("p", "a");
  ASSERT_EQ(notification_setting_set_lock_screen_content(s,
                HIDE_SENSITIVE_CONTENT),
            NOTIFICATION_ERROR_NONE);
  lock_screen_content_level_e v = SHOW_ALL_CONTENT;
  EXPECT_EQ(notification_setting_get_lock_screen_content(s, &v),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(v, HIDE_SENSITIVE_CONTENT);
  notification_setting_free_notification(s);
}

TEST_F(NotificationSettingApiTest,
       lock_screen_content_do_not_show_roundtrip_again) {
  notification_setting_h s = make_setting("p", "a");
  ASSERT_EQ(notification_setting_set_lock_screen_content(s,
                DO_NOT_SHOW_NOTIFICATIONS),
            NOTIFICATION_ERROR_NONE);
  lock_screen_content_level_e v = SHOW_ALL_CONTENT;
  EXPECT_EQ(notification_setting_get_lock_screen_content(s, &v),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(v, DO_NOT_SHOW_NOTIFICATIONS);
  notification_setting_free_notification(s);
}

namespace {
notification_system_setting_h make_sys_setting() {
  auto* s = static_cast<notification_system_setting_h>(
      calloc(1, sizeof(struct notification_system_setting)));
  return s;
}
}  // namespace

TEST_F(NotificationSettingApiTest, sys_get_visibility_class_roundtrip) {
  notification_system_setting_h s = make_sys_setting();
  s->visibility_class = 42;
  int v = 0;
  EXPECT_EQ(notification_system_setting_get_visibility_class(s, &v),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(v, 42);
  free(s);
}

TEST_F(NotificationSettingApiTest, sys_get_visibility_class_null_args) {
  notification_system_setting_h s = make_sys_setting();
  int v = 0;
  EXPECT_EQ(notification_system_setting_get_visibility_class(nullptr, &v),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_system_setting_get_visibility_class(s, nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  free(s);
}

TEST_F(NotificationSettingApiTest, sys_set_visibility_class_roundtrip) {
  notification_system_setting_h s = make_sys_setting();
  EXPECT_EQ(notification_system_setting_set_visibility_class(s, 7),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(s->visibility_class, 7);
  free(s);
}

TEST_F(NotificationSettingApiTest, sys_set_visibility_class_null_handle) {
  EXPECT_EQ(notification_system_setting_set_visibility_class(nullptr, 1),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationSettingApiTest, sys_dnd_schedule_set_end_time_roundtrip) {
  notification_system_setting_h s = make_sys_setting();
  EXPECT_EQ(notification_system_setting_dnd_schedule_set_end_time(s, 23, 45),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(s->dnd_end_hour, 23);
  EXPECT_EQ(s->dnd_end_min, 45);
  free(s);
}

TEST_F(NotificationSettingApiTest, sys_dnd_schedule_set_end_time_null_handle) {
  EXPECT_EQ(notification_system_setting_dnd_schedule_set_end_time(nullptr,
                0, 0),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationSettingApiTest, sys_get_lock_screen_content_roundtrip) {
  notification_system_setting_h s = make_sys_setting();
  s->lock_screen_content_level = HIDE_SENSITIVE_CONTENT;
  lock_screen_content_level_e v = SHOW_ALL_CONTENT;
  EXPECT_EQ(notification_system_setting_get_lock_screen_content(s, &v),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(v, HIDE_SENSITIVE_CONTENT);
  free(s);
}

TEST_F(NotificationSettingApiTest, sys_get_lock_screen_content_null_args) {
  notification_system_setting_h s = make_sys_setting();
  lock_screen_content_level_e v = SHOW_ALL_CONTENT;
  EXPECT_EQ(notification_system_setting_get_lock_screen_content(nullptr, &v),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_system_setting_get_lock_screen_content(s, nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  free(s);
}

TEST_F(NotificationSettingApiTest, sys_set_lock_screen_content_roundtrip) {
  notification_system_setting_h s = make_sys_setting();
  EXPECT_EQ(notification_system_setting_set_lock_screen_content(s,
                DO_NOT_SHOW_NOTIFICATIONS),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(s->lock_screen_content_level, DO_NOT_SHOW_NOTIFICATIONS);
  free(s);
}

TEST_F(NotificationSettingApiTest, sys_set_lock_screen_content_null_handle) {
  EXPECT_EQ(notification_system_setting_set_lock_screen_content(nullptr,
                SHOW_ALL_CONTENT),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationSettingApiTest, sys_set_do_not_disturb_roundtrip) {
  notification_system_setting_h s = make_sys_setting();
  EXPECT_EQ(notification_system_setting_set_do_not_disturb(s, true),
            NOTIFICATION_ERROR_NONE);
  EXPECT_TRUE(s->do_not_disturb);
  free(s);
}

TEST_F(NotificationSettingApiTest, sys_dnd_schedule_set_enabled_roundtrip) {
  notification_system_setting_h s = make_sys_setting();
  EXPECT_EQ(notification_system_setting_dnd_schedule_set_enabled(s, true),
            NOTIFICATION_ERROR_NONE);
  EXPECT_TRUE(s->dnd_schedule_enabled);
  free(s);
}

TEST_F(NotificationSettingApiTest, sys_dnd_schedule_set_day_roundtrip) {
  notification_system_setting_h s = make_sys_setting();
  EXPECT_EQ(notification_system_setting_dnd_schedule_set_day(s, 5),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(s->dnd_schedule_day, 5);
  free(s);
}

TEST_F(NotificationSettingApiTest, sys_dnd_schedule_set_start_time_roundtrip) {
  notification_system_setting_h s = make_sys_setting();
  EXPECT_EQ(notification_system_setting_dnd_schedule_set_start_time(s, 8, 30),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(s->dnd_start_hour, 8);
  EXPECT_EQ(s->dnd_start_min, 30);
  free(s);
}

TEST_F(NotificationSettingApiTest, set_allow_to_notify_roundtrip) {
  notification_setting_h s = make_setting("p", "a");
  EXPECT_EQ(notification_setting_set_allow_to_notify(s, false),
            NOTIFICATION_ERROR_NONE);
  EXPECT_FALSE(s->allow_to_notify);
  notification_setting_free_notification(s);
}

TEST_F(NotificationSettingApiTest, set_do_not_disturb_except_roundtrip) {
  notification_setting_h s = make_setting("p", "a");
  EXPECT_EQ(notification_setting_set_do_not_disturb_except(s, true),
            NOTIFICATION_ERROR_NONE);
  EXPECT_TRUE(s->do_not_disturb_except);
  notification_setting_free_notification(s);
}

TEST_F(NotificationSettingApiTest, set_pop_up_notification_roundtrip) {
  notification_setting_h s = make_setting("p", "a");
  EXPECT_EQ(notification_setting_set_pop_up_notification(s, false),
            NOTIFICATION_ERROR_NONE);
  EXPECT_FALSE(s->pop_up_notification);
  notification_setting_free_notification(s);
}

class NotificationSettingApiTidlFailTest : public NotificationSettingApiTest {
 public:
  void SetUp() override {
    ON_CALL(GetMock<NotiServiceProxyMock>(),
            rpc_port_proxy_noti_service_connect_sync(_))
        .WillByDefault(Return(RPC_PORT_ERROR_IO_ERROR));
  }
};

TEST_F(NotificationSettingApiTidlFailTest,
       get_setting_array_for_uid_invokes_tidl) {
  int n = 0;
  notification_setting_h arr = nullptr;
  notification_setting_get_setting_array_for_uid(&arr, &n, getuid());
}

TEST_F(NotificationSettingApiTidlFailTest,
       get_setting_by_appid_for_uid_invokes_tidl) {
  notification_setting_h out = nullptr;
  notification_setting_get_setting_by_appid_for_uid("app", &out, getuid());
}

TEST_F(NotificationSettingApiTidlFailTest,
       update_setting_for_uid_invokes_tidl) {
  notification_setting_h s = make_setting("p", "a");
  notification_setting_update_setting_for_uid(s, getuid());
  notification_setting_free_notification(s);
}

TEST_F(NotificationSettingApiTidlFailTest,
       sys_load_system_setting_for_uid_invokes_tidl) {
  notification_system_setting_h out = nullptr;
  notification_system_setting_load_system_setting_for_uid(&out, getuid());
}

TEST_F(NotificationSettingApiTidlFailTest,
       sys_update_system_setting_for_uid_invokes_tidl) {
  notification_system_setting_h s = make_sys_setting();
  notification_system_setting_update_system_setting_for_uid(s, getuid());
  free(s);
}

TEST_F(NotificationSettingApiTidlFailTest,
       get_setting_via_pid_invokes_inner_call) {
  notification_setting_h out = nullptr;
  notification_setting_get_setting(&out);
}
