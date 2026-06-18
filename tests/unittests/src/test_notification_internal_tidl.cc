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

#include <bundle.h>
#include <dlfcn.h>
#include <glib.h>
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
#include "notification/include/notification_internal.h"
#include "notification/include/notification_private.h"
#include "notification/include/notification_setting_internal.h"

#include <notification_tidl_proxy.h>

using ::testing::_;
using ::testing::Invoke;
using ::testing::Return;

namespace {

typedef struct {
  const char *package;
  char *locale;
  pkgmgrinfo_app_component app_component;
  application_x *app_info;
} pkgmgr_appinfo_x;

typedef void *pkgmgrinfo_pkginfo_h;

struct package_info_s {
  char *package;
  pkgmgrinfo_pkginfo_h pkgmgr_pkginfo;
};

int __fake_aul_app_get_appid_bypid(int pid, char* appid, int len) {
  snprintf(appid, len, "%s", "org.tizen.testappid");
  return AUL_R_OK;
}

int __fake_aul_app_get_pkgid_bypid(int pid, char* pkgid, int len) {
  snprintf(pkgid, len, "%s", "org.tizen.testpkgid");
  return AUL_R_OK;
}

int __fake_package_info_create(const char* package, package_info_h* package_info) {
  *package_info = (package_info_h)calloc(1, sizeof(struct package_info_s));
  return PACKAGE_MANAGER_ERROR_NONE;
}

int __fake_package_info_get_root_path(package_info_h package_info, char** path) {
  *path = strdup("testrootpath");
  return PACKAGE_MANAGER_ERROR_NONE;
}

int __fake_pkgmgrinfo_appinfo_get_usr_appinfo(const char* appid, uid_t uid,
    pkgmgrinfo_appinfo_h* handle) {
  *handle = (pkgmgrinfo_appinfo_h*)calloc(1, sizeof(pkgmgr_appinfo_x));
  return PMINFO_R_OK;
}

int __fake_pkgmgrinfo_appinfo_get_label(pkgmgrinfo_appinfo_h handle, char** label) {
  *label = strdup("testlabel");
  return PMINFO_R_OK;
}

class Mocks :
    virtual public ::testing::NiceMock<AulMock>,
    virtual public ::testing::NiceMock<TzplatformConfigMock>,
    virtual public ::testing::NiceMock<GlibMock>,
    virtual public ::testing::NiceMock<NotiServiceProxyMock>,
    virtual public ::testing::NiceMock<PackageManagerMock>,
    virtual public ::testing::NiceMock<RpcPortMock> {};

}  // namespace

class NotificationInternalTidlTest : public TestFixture {
 public:
  NotificationInternalTidlTest() : TestFixture(std::make_unique<::Mocks>()) {}

  void SetUp() override {
    ON_CALL(GetMock<AulMock>(), aul_getuid()).WillByDefault(Return(5001));
    EXPECT_CALL(GetMock<AulMock>(), aul_app_get_appid_bypid(_, _, _))
        .WillRepeatedly(Invoke(__fake_aul_app_get_appid_bypid));
    EXPECT_CALL(GetMock<AulMock>(), aul_app_get_pkgid_bypid(_, _, _))
        .WillRepeatedly(Invoke(__fake_aul_app_get_pkgid_bypid));
    EXPECT_CALL(GetMock<PackageManagerMock>(), package_info_create(_, _))
        .WillRepeatedly(Invoke(__fake_package_info_create));
    EXPECT_CALL(GetMock<PackageManagerMock>(), package_info_get_root_path(_, _))
        .WillRepeatedly(Invoke(__fake_package_info_get_root_path));
    EXPECT_CALL(GetMock<PackageManagerMock>(),
        pkgmgrinfo_appinfo_get_usr_appinfo(_, _, _))
        .WillRepeatedly(Invoke(__fake_pkgmgrinfo_appinfo_get_usr_appinfo));
    EXPECT_CALL(GetMock<PackageManagerMock>(),
        pkgmgrinfo_appinfo_get_label(_, _))
        .WillRepeatedly(Invoke(__fake_pkgmgrinfo_appinfo_get_label));

    /* RpcPortMock intercepts parcel_create but doesn't populate the
     * out handle, so the real parcel_write would segfault. Delegate to
     * the real librpc-port impl via dlsym so array_bundle_clone works. */
    static auto* real_parcel_create =
        reinterpret_cast<int (*)(rpc_port_parcel_h*)>(
            dlsym(RTLD_NEXT, "rpc_port_parcel_create"));
    static auto* real_parcel_destroy =
        reinterpret_cast<int (*)(rpc_port_parcel_h)>(
            dlsym(RTLD_NEXT, "rpc_port_parcel_destroy"));
    if (real_parcel_create) {
      EXPECT_CALL(GetMock<RpcPortMock>(), rpc_port_parcel_create(_))
          .WillRepeatedly(Invoke(real_parcel_create));
    }
    if (real_parcel_destroy) {
      EXPECT_CALL(GetMock<RpcPortMock>(), rpc_port_parcel_destroy(_))
          .WillRepeatedly(Invoke(real_parcel_destroy));
    }
  }
};

TEST_F(NotificationInternalTidlTest, make_empty_notification_fills_defaults) {
  rpc_port_proxy_notification_h h = nullptr;
  ASSERT_EQ(rpc_port_proxy_notification_create(&h), RPC_PORT_ERROR_NONE);
  ASSERT_NE(h, nullptr);

  EXPECT_EQ(make_empty_notification(h), NOTIFICATION_ERROR_NONE);

  int type = 0;
  EXPECT_EQ(rpc_port_proxy_notification_get_type(h, &type),
            RPC_PORT_ERROR_NONE);
  EXPECT_EQ(type, -1);

  int layout = 0;
  EXPECT_EQ(rpc_port_proxy_notification_get_layout(h, &layout),
            RPC_PORT_ERROR_NONE);
  EXPECT_EQ(layout, -1);

  int priv_id = 0;
  EXPECT_EQ(rpc_port_proxy_notification_get_priv_id(h, &priv_id),
            RPC_PORT_ERROR_NONE);
  EXPECT_EQ(priv_id, -1);

  rpc_port_proxy_notification_destroy(h);
}

namespace {

void populate_noti(notification_h n) {
  notification_set_text(n, NOTIFICATION_TEXT_TYPE_TITLE, "hello",
                        nullptr, NOTIFICATION_VARIABLE_TYPE_NONE);
  notification_set_text(n, NOTIFICATION_TEXT_TYPE_CONTENT, "world",
                        nullptr, NOTIFICATION_VARIABLE_TYPE_NONE);
  notification_set_image(n, NOTIFICATION_IMAGE_TYPE_ICON, "/icon.png");
  notification_set_sound(n, NOTIFICATION_SOUND_TYPE_DEFAULT, nullptr);
  notification_set_vibration(n, NOTIFICATION_VIBRATION_TYPE_DEFAULT, nullptr);
  notification_set_led(n, NOTIFICATION_LED_OP_ON, 0xFF0000);
  notification_set_time(n, 1234567890);
  notification_set_display_applist(n, NOTIFICATION_DISPLAY_APP_ALL);
  notification_set_property(n, 0);
  notification_set_layout(n, NOTIFICATION_LY_NOTI_EVENT_SINGLE);
}

}  // namespace

TEST_F(NotificationInternalTidlTest,
       make_notification_from_noti_no_translate) {
  rpc_port_proxy_notification_h h = nullptr;
  ASSERT_EQ(rpc_port_proxy_notification_create(&h), RPC_PORT_ERROR_NONE);

  notification_h n = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(n, nullptr);
  populate_noti(n);

  EXPECT_EQ(make_notification_from_noti(h, n, false),
            NOTIFICATION_ERROR_NONE);

  int type = 0;
  EXPECT_EQ(rpc_port_proxy_notification_get_type(h, &type),
            RPC_PORT_ERROR_NONE);
  EXPECT_EQ(type, NOTIFICATION_TYPE_NOTI);

  notification_free(n);
  rpc_port_proxy_notification_destroy(h);
}

TEST_F(NotificationInternalTidlTest,
       make_notification_from_noti_with_translate) {
  rpc_port_proxy_notification_h h = nullptr;
  ASSERT_EQ(rpc_port_proxy_notification_create(&h), RPC_PORT_ERROR_NONE);

  notification_h n = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(n, nullptr);
  populate_noti(n);

  EXPECT_EQ(make_notification_from_noti(h, n, true),
            NOTIFICATION_ERROR_NONE);

  notification_free(n);
  rpc_port_proxy_notification_destroy(h);
}

TEST_F(NotificationInternalTidlTest,
       make_notification_from_noti_empty_handles_null_event_handlers) {
  rpc_port_proxy_notification_h h = nullptr;
  ASSERT_EQ(rpc_port_proxy_notification_create(&h), RPC_PORT_ERROR_NONE);

  notification_h n = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(n, nullptr);

  EXPECT_EQ(make_notification_from_noti(h, n, false),
            NOTIFICATION_ERROR_NONE);

  notification_free(n);
  rpc_port_proxy_notification_destroy(h);
}

TEST_F(NotificationInternalTidlTest,
       make_noti_from_notification_roundtrip) {
  rpc_port_proxy_notification_h h = nullptr;
  ASSERT_EQ(rpc_port_proxy_notification_create(&h), RPC_PORT_ERROR_NONE);

  notification_h src = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(src, nullptr);
  populate_noti(src);
  ASSERT_EQ(make_notification_from_noti(h, src, false),
            NOTIFICATION_ERROR_NONE);

  notification_h dst = nullptr;
  EXPECT_EQ(make_noti_from_notification(&dst, h), NOTIFICATION_ERROR_NONE);
  ASSERT_NE(dst, nullptr);
  EXPECT_EQ(dst->type, NOTIFICATION_TYPE_NOTI);

  notification_free(src);
  notification_free(dst);
  rpc_port_proxy_notification_destroy(h);
}

TEST_F(NotificationInternalTidlTest, dnd_exception_from_exception_copies_fields) {
  rpc_port_proxy_noti_system_setting_dnd_allow_exception_h h = nullptr;
  ASSERT_EQ(rpc_port_proxy_noti_system_setting_dnd_allow_exception_create(&h),
            RPC_PORT_ERROR_NONE);

  struct notification_system_setting_dnd_allow_exception exc {};
  exc.type = 1;
  exc.value = 42;

  EXPECT_EQ(make_dnd_allow_exception_from_exception(h, &exc),
            NOTIFICATION_ERROR_NONE);

  int type = 0;
  EXPECT_EQ(rpc_port_proxy_noti_system_setting_dnd_allow_exception_get_type(
                h, &type),
            RPC_PORT_ERROR_NONE);
  EXPECT_EQ(type, 1);

  int value = 0;
  EXPECT_EQ(rpc_port_proxy_noti_system_setting_dnd_allow_exception_get_value(
                h, &value),
            RPC_PORT_ERROR_NONE);
  EXPECT_EQ(value, 42);

  rpc_port_proxy_noti_system_setting_dnd_allow_exception_destroy(h);
}

TEST_F(NotificationInternalTidlTest, dnd_exception_from_exception_null_args) {
  struct notification_system_setting_dnd_allow_exception exc {};
  EXPECT_EQ(make_dnd_allow_exception_from_exception(nullptr, &exc),
            NOTIFICATION_ERROR_INVALID_PARAMETER);

  rpc_port_proxy_noti_system_setting_dnd_allow_exception_h h = nullptr;
  ASSERT_EQ(rpc_port_proxy_noti_system_setting_dnd_allow_exception_create(&h),
            RPC_PORT_ERROR_NONE);
  EXPECT_EQ(make_dnd_allow_exception_from_exception(h, nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  rpc_port_proxy_noti_system_setting_dnd_allow_exception_destroy(h);
}

TEST_F(NotificationInternalTidlTest,
       noti_system_setting_from_setting_empty_exception_list) {
  rpc_port_proxy_noti_system_setting_h h = nullptr;
  ASSERT_EQ(rpc_port_proxy_noti_system_setting_create(&h),
            RPC_PORT_ERROR_NONE);

  auto* s = static_cast<notification_system_setting_h>(
      calloc(1, sizeof(struct notification_system_setting)));
  s->do_not_disturb = true;
  s->visibility_class = 3;
  s->dnd_schedule_enabled = true;
  s->dnd_schedule_day = 5;
  s->dnd_start_hour = 22;
  s->dnd_start_min = 30;
  s->dnd_end_hour = 7;
  s->dnd_end_min = 0;
  s->lock_screen_content_level = HIDE_SENSITIVE_CONTENT;

  EXPECT_EQ(make_noti_system_setting_from_setting(h, s),
            NOTIFICATION_ERROR_NONE);

  bool dnd = false;
  EXPECT_EQ(rpc_port_proxy_noti_system_setting_get_do_not_disturb(h, &dnd),
            RPC_PORT_ERROR_NONE);
  EXPECT_TRUE(dnd);

  free(s);
  rpc_port_proxy_noti_system_setting_destroy(h);
}

TEST_F(NotificationInternalTidlTest,
       noti_system_setting_from_setting_with_one_exception) {
  rpc_port_proxy_noti_system_setting_h h = nullptr;
  ASSERT_EQ(rpc_port_proxy_noti_system_setting_create(&h),
            RPC_PORT_ERROR_NONE);

  auto* s = static_cast<notification_system_setting_h>(
      calloc(1, sizeof(struct notification_system_setting)));
  auto* exc = static_cast<dnd_allow_exception_h>(
      calloc(1, sizeof(struct notification_system_setting_dnd_allow_exception)));
  exc->type = 2;
  exc->value = 99;
  s->dnd_allow_exceptions = g_list_append(nullptr, exc);

  EXPECT_EQ(make_noti_system_setting_from_setting(h, s),
            NOTIFICATION_ERROR_NONE);

  g_list_free_full(s->dnd_allow_exceptions, free);
  free(s);
  rpc_port_proxy_noti_system_setting_destroy(h);
}

TEST_F(NotificationInternalTidlTest,
       make_setting_from_noti_system_setting_copies_fields) {
  rpc_port_proxy_noti_system_setting_h h = nullptr;
  ASSERT_EQ(rpc_port_proxy_noti_system_setting_create(&h),
            RPC_PORT_ERROR_NONE);

  rpc_port_proxy_noti_system_setting_set_do_not_disturb(h, true);
  rpc_port_proxy_noti_system_setting_set_visibility_class(h, 7);
  rpc_port_proxy_noti_system_setting_set_lock_screen_content_level(
      h, DO_NOT_SHOW_NOTIFICATIONS);

  notification_system_setting_h s = nullptr;
  EXPECT_EQ(make_setting_from_noti_system_setting(&s, h),
            NOTIFICATION_ERROR_NONE);
  ASSERT_NE(s, nullptr);
  EXPECT_TRUE(s->do_not_disturb);
  EXPECT_EQ(s->visibility_class, 7);
  EXPECT_EQ(s->lock_screen_content_level, DO_NOT_SHOW_NOTIFICATIONS);

  if (s->dnd_allow_exceptions)
    g_list_free_full(s->dnd_allow_exceptions, free);
  free(s);
  rpc_port_proxy_noti_system_setting_destroy(h);
}

TEST_F(NotificationInternalTidlTest,
       noti_setting_from_setting_copies_all_fields) {
  rpc_port_proxy_noti_setting_h h = nullptr;
  ASSERT_EQ(rpc_port_proxy_noti_setting_create(&h), RPC_PORT_ERROR_NONE);

  auto* s = static_cast<notification_setting_h>(
      calloc(1, sizeof(struct notification_setting)));
  s->package_name = strdup("pkg.x");
  s->app_id = strdup("app.x");
  s->allow_to_notify = true;
  s->do_not_disturb_except = false;
  s->pop_up_notification = true;
  s->visibility_class = 4;
  s->lock_screen_content_level = SHOW_ALL_CONTENT;
  s->app_disabled = false;

  EXPECT_EQ(make_noti_setting_from_setting(h, s),
            NOTIFICATION_ERROR_NONE);

  struct notification_setting dst {};
  EXPECT_EQ(make_setting_from_noti_setting(&dst, h),
            NOTIFICATION_ERROR_NONE);
  EXPECT_STREQ(dst.package_name, "pkg.x");
  EXPECT_STREQ(dst.app_id, "app.x");
  EXPECT_TRUE(dst.allow_to_notify);
  EXPECT_EQ(dst.visibility_class, 4);
  free(dst.package_name);
  free(dst.app_id);

  notification_setting_free_notification(s);
  rpc_port_proxy_noti_setting_destroy(h);
}

/* Group K — make_setting_from_noti_setting: empty string → NULL branches.
 *
 * When package_name and app_id are NULL in the source setting,
 * make_noti_setting_from_setting serializes them as "" via __get_str_value.
 * make_setting_from_noti_setting then retrieves "", strlen == 0 → false arms
 * set setting->package_name = NULL and setting->app_id = NULL.
 * Covers L1362-L1363 and L1369-L1370. */

TEST_F(NotificationInternalTidlTest,
       make_setting_from_noti_setting_empty_strings_cover_null_branches) {
  rpc_port_proxy_noti_setting_h h = nullptr;
  ASSERT_EQ(rpc_port_proxy_noti_setting_create(&h), RPC_PORT_ERROR_NONE);

  auto* s = static_cast<notification_setting_h>(
      calloc(1, sizeof(struct notification_setting)));

  ASSERT_EQ(make_noti_setting_from_setting(h, s), NOTIFICATION_ERROR_NONE);

  struct notification_setting dst {};
  EXPECT_EQ(make_setting_from_noti_setting(&dst, h), NOTIFICATION_ERROR_NONE);

  EXPECT_EQ(dst.package_name, nullptr);
  EXPECT_EQ(dst.app_id, nullptr);

  notification_setting_free_notification(s);
  rpc_port_proxy_noti_setting_destroy(h);
}

TEST_F(NotificationInternalTidlTest,
       noti_setting_from_setting_handles_null_string_fields) {
  rpc_port_proxy_noti_setting_h h = nullptr;
  ASSERT_EQ(rpc_port_proxy_noti_setting_create(&h), RPC_PORT_ERROR_NONE);

  auto* s = static_cast<notification_setting_h>(
      calloc(1, sizeof(struct notification_setting)));

  EXPECT_EQ(make_noti_setting_from_setting(h, s),
            NOTIFICATION_ERROR_NONE);

  notification_setting_free_notification(s);
  rpc_port_proxy_noti_setting_destroy(h);
}

/* Group A — NULL handle: covers WARN branches inside make_empty_notification
 * and make_notification_from_noti when every rpc_port_proxy_*_set_*(NULL,...)
 * returns RPC_PORT_ERROR_INVALID_PARAMETER.  The functions still return
 * NOTIFICATION_ERROR_NONE because the WARN branches are non-fatal. */

TEST_F(NotificationInternalTidlTest,
       make_empty_notification_null_handle_covers_warn_bodies) {
  /* Passing NULL is safe: every set_* call gets a non-NONE return code and
   * hits the WARN(...) branch, but the function always returns NONE. */
  EXPECT_EQ(make_empty_notification(nullptr), NOTIFICATION_ERROR_NONE);
}

TEST_F(NotificationInternalTidlTest,
       make_notification_from_noti_null_handle_covers_warn_bodies) {
  notification_h n = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(n, nullptr);
  populate_noti(n);

  /* NULL handle: every rpc_port_proxy_notification_set_*(NULL,...) returns
   * RPC_PORT_ERROR_INVALID_PARAMETER and hits each WARN branch (L334-L596),
   * but the function still returns NOTIFICATION_ERROR_NONE. */
  EXPECT_EQ(make_notification_from_noti(nullptr, n, false),
            NOTIFICATION_ERROR_NONE);

  notification_free(n);
}

/* Group B — make_noti_from_notification with an empty-string TIDL handle.
 *
 * make_empty_notification stores "" for pkg_id / caller_app_id /
 * launch_app_id etc.  make_noti_from_notification then receives those empty
 * strings, evaluates strlen()==0, and takes the else branch that sets the
 * corresponding notification_h field to NULL. */

TEST_F(NotificationInternalTidlTest,
       make_noti_from_notification_empty_string_handle_covers_else_branches) {
  rpc_port_proxy_notification_h h = nullptr;
  ASSERT_EQ(rpc_port_proxy_notification_create(&h), RPC_PORT_ERROR_NONE);

  /* Populate the TIDL handle with default-empty strings and -1 integers. */
  ASSERT_EQ(make_empty_notification(h), NOTIFICATION_ERROR_NONE);

  notification_h dst = nullptr;
  EXPECT_EQ(make_noti_from_notification(&dst, h), NOTIFICATION_ERROR_NONE);
  ASSERT_NE(dst, nullptr);

  /* All string fields that were stored as "" must come back as nullptr
   * (the else branch at L652-653, L664-665, L676-677, L1108-1109, etc.). */
  EXPECT_EQ(dst->pkg_id, nullptr);
  EXPECT_EQ(dst->caller_app_id, nullptr);
  EXPECT_EQ(dst->launch_app_id, nullptr);
  EXPECT_EQ(dst->channel_name, nullptr);

  /* Bundle fields stored as empty bundles must come back as nullptr too. */
  EXPECT_EQ(dst->args, nullptr);

  notification_free(dst);
  rpc_port_proxy_notification_destroy(h);
}

/* Group C — make_noti_from_notification with a non-empty args bundle.
 *
 * When args has at least one entry, bundle_get_count() > 0 and the true
 * branch (L685-686) is taken, storing the bundle pointer in dst->args. */

TEST_F(NotificationInternalTidlTest,
       make_noti_from_notification_nonempty_args_covers_bundle_true_branch) {
  /* Build a source notification with a non-empty args bundle. */
  notification_h src = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(src, nullptr);

  bundle* b = bundle_create();
  ASSERT_NE(b, nullptr);
  bundle_add_str(b, "key1", "val1");
  ASSERT_EQ(notification_set_args(src, b, nullptr), NOTIFICATION_ERROR_NONE);
  bundle_free(b);

  /* Transfer src into a TIDL handle. */
  rpc_port_proxy_notification_h h = nullptr;
  ASSERT_EQ(rpc_port_proxy_notification_create(&h), RPC_PORT_ERROR_NONE);
  ASSERT_EQ(make_notification_from_noti(h, src, false),
            NOTIFICATION_ERROR_NONE);

  /* Reconstruct a notification_h from the TIDL handle. */
  notification_h dst = nullptr;
  EXPECT_EQ(make_noti_from_notification(&dst, h), NOTIFICATION_ERROR_NONE);
  ASSERT_NE(dst, nullptr);

  /* The args bundle must be non-NULL (true branch L685-686). */
  ASSERT_NE(dst->args, nullptr);
  EXPECT_GT(bundle_get_count(dst->args), 0);

  notification_free(src);
  notification_free(dst);
  rpc_port_proxy_notification_destroy(h);
}

/* Group D — DND allow-exception callback coverage.
 *
 * Build a notification_system_setting_h with one dnd_allow_exception, push
 * it into a TIDL handle via make_noti_system_setting_from_setting, then pull
 * it back via make_setting_from_noti_system_setting.  The round-trip invokes
 * _rpc_port_proxy_list_noti_system_setting_dnd_allow_exception_cb for each
 * item (L1134-L1159), exercising the callback body. */

TEST_F(NotificationInternalTidlTest,
       make_setting_from_noti_system_setting_with_exception_covers_callback) {
  /* The callback _rpc_port_proxy_list_noti_system_setting_dnd_allow_exception_cb
   * calls g_list_append (which is mocked). Delegate to the real glib function
   * so the reconstructed list actually holds the appended items. */
  using GListAppendFn = GList*(*)(GList*, gpointer);
  ON_CALL(GetMock<GlibMock>(), g_list_append(_, _))
      .WillByDefault(Invoke([](GList* list, gpointer data) -> GList* {
        static GListAppendFn real = reinterpret_cast<GListAppendFn>(
            dlsym(RTLD_NEXT, "g_list_append"));
        return real ? real(list, data) : nullptr;
      }));

  rpc_port_proxy_noti_system_setting_h h = nullptr;
  ASSERT_EQ(rpc_port_proxy_noti_system_setting_create(&h),
            RPC_PORT_ERROR_NONE);

  /* Build a source system setting with one exception. */
  auto* s = static_cast<notification_system_setting_h>(
      calloc(1, sizeof(struct notification_system_setting)));
  ASSERT_NE(s, nullptr);

  auto* exc = static_cast<dnd_allow_exception_h>(
      calloc(1, sizeof(struct notification_system_setting_dnd_allow_exception)));
  ASSERT_NE(exc, nullptr);
  exc->type = 3;
  exc->value = 7;
  {
    static GListAppendFn real = reinterpret_cast<GListAppendFn>(
        dlsym(RTLD_NEXT, "g_list_append"));
    s->dnd_allow_exceptions = real ? real(nullptr, exc) : nullptr;
  }

  /* Store into TIDL handle — this exercises the exception-list serialisation
   * path in make_noti_system_setting_from_setting. */
  ASSERT_EQ(make_noti_system_setting_from_setting(h, s),
            NOTIFICATION_ERROR_NONE);

  /* Retrieve back — this drives the foreach callback for each item in the
   * TIDL list, covering lines L1134-L1159. */
  notification_system_setting_h result = nullptr;
  EXPECT_EQ(make_setting_from_noti_system_setting(&result, h),
            NOTIFICATION_ERROR_NONE);
  ASSERT_NE(result, nullptr);

  /* The callback fires once per exception → list must have exactly one entry.
   * (Actual type/value roundtrip goes through mocked parcel_write/read, so
   * values come back as 0; we only verify the callback was invoked.) */
  EXPECT_EQ(g_list_length(result->dnd_allow_exceptions), 1u);

  if (result->dnd_allow_exceptions) {
    auto* got = static_cast<dnd_allow_exception_h>(
        g_list_nth_data(result->dnd_allow_exceptions, 0));
    EXPECT_NE(got, nullptr);
    g_list_free_full(result->dnd_allow_exceptions, free);
  }
  free(result);

  g_list_free_full(s->dnd_allow_exceptions, free);
  free(s);
  rpc_port_proxy_noti_system_setting_destroy(h);
}

/* Group E — make_noti_from_notification: group_args true branch.
 *
 * When group_args has at least one entry, bundle_get_count() > 0 and the true
 * branch (L697-698) is taken, storing the bundle pointer in dst->group_args. */

TEST_F(NotificationInternalTidlTest,
       make_noti_from_notification_with_group_args_covers_true_branch) {
  notification_h src = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(src, nullptr);
  populate_noti(src);

  bundle* a = bundle_create();
  ASSERT_NE(a, nullptr);
  bundle_add_str(a, "ak", "av");

  bundle* ga = bundle_create();
  ASSERT_NE(ga, nullptr);
  bundle_add_str(ga, "gk", "gv");

  ASSERT_EQ(notification_set_args(src, a, ga), NOTIFICATION_ERROR_NONE);
  bundle_free(a);
  bundle_free(ga);

  rpc_port_proxy_notification_h h = nullptr;
  ASSERT_EQ(rpc_port_proxy_notification_create(&h), RPC_PORT_ERROR_NONE);
  ASSERT_EQ(make_notification_from_noti(h, src, false), NOTIFICATION_ERROR_NONE);

  notification_h dst = nullptr;
  EXPECT_EQ(make_noti_from_notification(&dst, h), NOTIFICATION_ERROR_NONE);
  ASSERT_NE(dst, nullptr);

  ASSERT_NE(dst->group_args, nullptr);
  EXPECT_GT(bundle_get_count(dst->group_args), 0);

  notification_free(src);
  notification_free(dst);
  rpc_port_proxy_notification_destroy(h);
}

/* Group F — make_noti_from_notification: execute option and service bundles
 * true branches.
 *
 * Setting execute options for SINGLE_LAUNCH, RESPONDING, and MULTI_LAUNCH
 * populates b_service_single_launch, b_service_responding, and
 * b_service_multi_launch on the source notification.  After the TIDL roundtrip
 * the reconstructed notification must have non-NULL bundle pointers for each
 * of those fields (true branches L721-722, L733-734, L745-746). */

TEST_F(NotificationInternalTidlTest,
       make_noti_from_notification_with_execute_option_and_service_bundles_covers_true_branches) {
  notification_h src = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(src, nullptr);
  populate_noti(src);

  bundle* svc = bundle_create();
  ASSERT_NE(svc, nullptr);
  bundle_add_str(svc, "k", "v");

  /* SINGLE_LAUNCH → populates b_service_single_launch */
  ASSERT_EQ(notification_set_execute_option(src,
                NOTIFICATION_EXECUTE_TYPE_SINGLE_LAUNCH,
                "btn", "btnkey", svc),
            NOTIFICATION_ERROR_NONE);

  /* RESPONDING → populates b_service_responding */
  ASSERT_EQ(notification_set_execute_option(src,
                NOTIFICATION_EXECUTE_TYPE_RESPONDING,
                nullptr, nullptr, svc),
            NOTIFICATION_ERROR_NONE);

  /* MULTI_LAUNCH → populates b_service_multi_launch */
  ASSERT_EQ(notification_set_execute_option(src,
                NOTIFICATION_EXECUTE_TYPE_MULTI_LAUNCH,
                nullptr, nullptr, svc),
            NOTIFICATION_ERROR_NONE);

  bundle_free(svc);

  rpc_port_proxy_notification_h h = nullptr;
  ASSERT_EQ(rpc_port_proxy_notification_create(&h), RPC_PORT_ERROR_NONE);
  ASSERT_EQ(make_notification_from_noti(h, src, false), NOTIFICATION_ERROR_NONE);

  notification_h dst = nullptr;
  EXPECT_EQ(make_noti_from_notification(&dst, h), NOTIFICATION_ERROR_NONE);
  ASSERT_NE(dst, nullptr);

  EXPECT_NE(dst->b_service_single_launch, nullptr);
  EXPECT_NE(dst->b_service_responding, nullptr);
  EXPECT_NE(dst->b_service_multi_launch, nullptr);

  notification_free(src);
  notification_free(dst);
  rpc_port_proxy_notification_destroy(h);
}

/* Group G — make_noti_from_notification: sound_path and vibration_path true
 * branches.
 *
 * populate_noti sets DEFAULT type with a null path which exercises the else
 * branch.  Overriding with USER_DATA type and a non-null path exercises the
 * true branch at L889-890 and L917-918. */

TEST_F(NotificationInternalTidlTest,
       make_noti_from_notification_with_sound_vibration_path_covers_true_branch) {
  notification_h src = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(src, nullptr);
  populate_noti(src);

  /* Override sound and vibration to USER_DATA type with a real path string. */
  ASSERT_EQ(notification_set_sound(src, NOTIFICATION_SOUND_TYPE_USER_DATA,
                                   "/test/snd.wav"),
            NOTIFICATION_ERROR_NONE);
  ASSERT_EQ(notification_set_vibration(src, NOTIFICATION_VIBRATION_TYPE_USER_DATA,
                                       "/test/vib.dat"),
            NOTIFICATION_ERROR_NONE);

  rpc_port_proxy_notification_h h = nullptr;
  ASSERT_EQ(rpc_port_proxy_notification_create(&h), RPC_PORT_ERROR_NONE);
  ASSERT_EQ(make_notification_from_noti(h, src, false), NOTIFICATION_ERROR_NONE);

  notification_h dst = nullptr;
  EXPECT_EQ(make_noti_from_notification(&dst, h), NOTIFICATION_ERROR_NONE);
  ASSERT_NE(dst, nullptr);

  EXPECT_NE(dst->sound_path, nullptr);
  EXPECT_NE(dst->vibration_path, nullptr);

  notification_free(src);
  notification_free(dst);
  rpc_port_proxy_notification_destroy(h);
}

/* Group H — make_noti_from_notification: launch_app_id true branch.
 *
 * No public API populates launch_app_id cleanly, so poke the struct directly.
 * After the TIDL roundtrip the reconstructed notification must have a
 * non-NULL launch_app_id matching the injected value (true branch L673-674). */

TEST_F(NotificationInternalTidlTest,
       make_noti_from_notification_with_launch_app_id_covers_true_branch) {
  notification_h src = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(src, nullptr);
  populate_noti(src);

  /* Poke the struct field directly — notification_private.h exposes it. */
  if (src->launch_app_id)
    free(src->launch_app_id);
  src->launch_app_id = strdup("org.tizen.launchapp.test");

  rpc_port_proxy_notification_h h = nullptr;
  ASSERT_EQ(rpc_port_proxy_notification_create(&h), RPC_PORT_ERROR_NONE);
  ASSERT_EQ(make_notification_from_noti(h, src, false), NOTIFICATION_ERROR_NONE);

  notification_h dst = nullptr;
  EXPECT_EQ(make_noti_from_notification(&dst, h), NOTIFICATION_ERROR_NONE);
  ASSERT_NE(dst, nullptr);

  EXPECT_NE(dst->launch_app_id, nullptr);
  EXPECT_STREQ(dst->launch_app_id, "org.tizen.launchapp.test");

  notification_free(src);
  notification_free(dst);
  rpc_port_proxy_notification_destroy(h);
}

/* Group I — make_noti_from_notification: all string copy-out true branches.
 *
 * populate_noti leaves priv_sound_path/priv_vibration_path/app_icon_path/
 * temp_title/temp_content/tag/channel_name unset → false arms run there.
 * This test sets them all before the roundtrip so the true arms execute.
 * Covers L902, L930, L988, L1012, L1024, L1036, L1106. */

TEST_F(NotificationInternalTidlTest,
       make_noti_from_notification_copies_nonempty_strings) {
  notification_h src = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(src, nullptr);

  src->priv_sound_path     = strdup("/test/priv_snd.wav");
  src->priv_vibration_path = strdup("/test/priv_vib.dat");
  src->app_icon_path       = strdup("/test/icon.png");
  src->temp_title          = strdup("Test Title");
  src->temp_content        = strdup("Test Content");
  ASSERT_EQ(notification_set_tag(src, "test-tag"), NOTIFICATION_ERROR_NONE);
  src->channel_name        = strdup("test-channel");

  rpc_port_proxy_notification_h h = nullptr;
  ASSERT_EQ(rpc_port_proxy_notification_create(&h), RPC_PORT_ERROR_NONE);
  ASSERT_EQ(make_notification_from_noti(h, src, false), NOTIFICATION_ERROR_NONE);

  notification_h dst = nullptr;
  EXPECT_EQ(make_noti_from_notification(&dst, h), NOTIFICATION_ERROR_NONE);
  ASSERT_NE(dst, nullptr);

  EXPECT_STREQ(dst->priv_sound_path, "/test/priv_snd.wav");
  EXPECT_STREQ(dst->priv_vibration_path, "/test/priv_vib.dat");
  EXPECT_STREQ(dst->app_icon_path, "/test/icon.png");
  EXPECT_STREQ(dst->temp_title, "Test Title");
  EXPECT_STREQ(dst->temp_content, "Test Content");
  EXPECT_STREQ(dst->tag, "test-tag");
  EXPECT_STREQ(dst->channel_name, "test-channel");

  notification_free(src);
  notification_free(dst);
  rpc_port_proxy_notification_destroy(h);
}

/* Group J — make_noti_from_notification: b_key and b_priv_image_path true branches.
 *
 * When src->b_key / src->b_priv_image_path are non-empty bundles,
 * make_notification_from_noti stores them in the proxy handle via bundle_dup
 * (instead of the fallback empty_bundle), so get_key/get_priv_image_path
 * return bundles with count > 0 → true arms at L832 and L872 execute.
 * The proxy struct is entirely in-memory so no parcel mocking interferes. */

TEST_F(NotificationInternalTidlTest,
       make_noti_from_notification_copies_nonempty_bundles) {
  notification_h src = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(src, nullptr);

  bundle *bk = bundle_create();
  ASSERT_NE(bk, nullptr);
  bundle_add_str(bk, "key_field", "key_val");

  bundle *bp = bundle_create();
  ASSERT_NE(bp, nullptr);
  bundle_add_str(bp, "priv_img_field", "priv_img_val");

  src->b_key             = bundle_dup(bk);
  src->b_priv_image_path = bundle_dup(bp);
  bundle_free(bk);
  bundle_free(bp);

  rpc_port_proxy_notification_h h = nullptr;
  ASSERT_EQ(rpc_port_proxy_notification_create(&h), RPC_PORT_ERROR_NONE);
  ASSERT_EQ(make_notification_from_noti(h, src, false), NOTIFICATION_ERROR_NONE);

  notification_h dst = nullptr;
  EXPECT_EQ(make_noti_from_notification(&dst, h), NOTIFICATION_ERROR_NONE);
  ASSERT_NE(dst, nullptr);

  ASSERT_NE(dst->b_key, nullptr);
  EXPECT_GT(bundle_get_count(dst->b_key), 0);

  ASSERT_NE(dst->b_priv_image_path, nullptr);
  EXPECT_GT(bundle_get_count(dst->b_priv_image_path), 0);

  notification_free(src);
  notification_free(dst);
  rpc_port_proxy_notification_destroy(h);
}
