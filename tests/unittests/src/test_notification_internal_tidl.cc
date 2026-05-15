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

  /* A handle produced by the real IPC deserializer always has its
   * dnd_allow_exceptions list created (see __rpc_port_proxy_noti_system_
   * setting_from in the generated proxy). A handle built from _create()
   * alone leaves the list nullptr, a state production never receives, so
   * give the fixture a realistic (empty) list to match. */
  rpc_port_proxy_list_noti_system_setting_dnd_allow_exception_h exc_list =
      nullptr;
  ASSERT_EQ(rpc_port_proxy_list_noti_system_setting_dnd_allow_exception_create(
                &exc_list),
            RPC_PORT_ERROR_NONE);
  rpc_port_proxy_noti_system_setting_set_dnd_allow_exceptions(h, exc_list);
  rpc_port_proxy_list_noti_system_setting_dnd_allow_exception_destroy(exc_list);

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
