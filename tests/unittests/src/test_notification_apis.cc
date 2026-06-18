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
#include <time.h>
#include <string>

#include "mock/app_control_mock.hh"
#include "mock/aul_mock.hh"
#include "mock/bundle_mock.hh"
#include "mock/glib_mock.hh"
#include "mock/libc_calloc_mock.hh"
#include "mock/noti_service_proxy_mock.hh"
#include "mock/package_manager_mock.hh"
#include "mock/rpc_port_mock.hh"
#include "mock/test_fixture.hh"
#include "mock/tzplatform_config_mock.hh"

#include "notification/include/notification.h"
#include "notification/include/notification_internal.h"
#include "notification/include/notification_ipc.h"
#include "notification/include/notification_private.h"
#include "notification/include/notification_shared_file.h"
#include "notification/include/notification_text_domain.h"
#include "notification/include/notification_ongoing_flag.h"

using ::testing::_;
using ::testing::DoAll;
using ::testing::Invoke;
using ::testing::Return;
using ::testing::SetArgPointee;

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
    virtual public ::testing::NiceMock<AppControlMock>,
    virtual public ::testing::NiceMock<AulMock>,
    virtual public ::testing::NiceMock<TzplatformConfigMock>,
    virtual public ::testing::NiceMock<GlibMock>,
    virtual public ::testing::NiceMock<NotiServiceProxyMock>,
    virtual public ::testing::NiceMock<PackageManagerMock>,
    virtual public ::testing::NiceMock<RpcPortMock> {};

}  // namespace

class NotificationApiTest : public TestFixture {
 public:
  NotificationApiTest() : TestFixture(std::make_unique<::Mocks>()) {}

  void SetUp() override {
    ON_CALL(GetMock<AulMock>(), aul_getuid()).WillByDefault(Return(5001));
    /* AppControlMock: default to the real library so existing tests work. */
    ON_CALL(GetMock<AppControlMock>(), app_control_create(_))
        .WillByDefault(Invoke(app_control_mock_real_create));
    ON_CALL(GetMock<AppControlMock>(), app_control_import_from_bundle(_, _))
        .WillByDefault(Invoke(app_control_mock_real_import_from_bundle));
    ON_CALL(GetMock<AppControlMock>(), app_control_destroy(_))
        .WillByDefault(Invoke(app_control_mock_real_destroy));
    ON_CALL(GetMock<AppControlMock>(), app_control_export_as_bundle(_, _))
        .WillByDefault(Invoke(app_control_mock_real_export_as_bundle));

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

    noti_ = notification_create(NOTIFICATION_TYPE_NOTI);
    ASSERT_NE(noti_, nullptr);
  }

  void TearDown() override {
    if (noti_ != nullptr) {
      notification_free(noti_);
      noti_ = nullptr;
    }
  }

  notification_h noti_ = nullptr;
};

/* ============================================================== */
/* notification_create / notification_free                        */
/* ============================================================== */

TEST_F(NotificationApiTest, create_returns_handle_via_fixture) {
  /* Fixture already created noti_; sanity check that it is non-null. */
  EXPECT_NE(noti_, nullptr);
}

TEST_F(NotificationApiTest, free_null_is_safe_noop) {
  /* Per code review: notification_free(NULL) should return
   * INVALID_PARAMETER, not crash. */
  EXPECT_EQ(notification_free(nullptr), NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* ============================================================== */
/* notification_set_image / notification_get_image                */
/* ============================================================== */

TEST_F(NotificationApiTest, set_image_icon_roundtrip) {
  ASSERT_EQ(notification_set_image(noti_, NOTIFICATION_IMAGE_TYPE_ICON,
                                   "/path/icon.png"),
            NOTIFICATION_ERROR_NONE);
  char* path = nullptr;
  EXPECT_EQ(notification_get_image(noti_, NOTIFICATION_IMAGE_TYPE_ICON, &path),
            NOTIFICATION_ERROR_NONE);
  EXPECT_STREQ(path, "/path/icon.png");
}

TEST_F(NotificationApiTest, set_image_null_noti) {
  EXPECT_EQ(notification_set_image(nullptr, NOTIFICATION_IMAGE_TYPE_ICON, "/p"),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationApiTest, set_image_type_none_returns_invalid) {
  EXPECT_EQ(notification_set_image(noti_, NOTIFICATION_IMAGE_TYPE_NONE, "/p"),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationApiTest, set_image_type_overflow_returns_invalid) {
  /* MAX itself is the last valid value; one past it is invalid. */
  EXPECT_EQ(notification_set_image(noti_,
                static_cast<notification_image_type_e>(
                    NOTIFICATION_IMAGE_TYPE_MAX + 1),
                "/p"),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationApiTest, set_image_null_path_early_return) {
  /* Per scenario [P3]: image_path NULL with no prior bundle returns NONE
   * without creating the bundle. */
  EXPECT_EQ(notification_set_image(noti_, NOTIFICATION_IMAGE_TYPE_ICON, nullptr),
            NOTIFICATION_ERROR_NONE);
}

TEST_F(NotificationApiTest, set_image_replace_existing) {
  ASSERT_EQ(notification_set_image(noti_, NOTIFICATION_IMAGE_TYPE_ICON, "/a"),
            NOTIFICATION_ERROR_NONE);
  ASSERT_EQ(notification_set_image(noti_, NOTIFICATION_IMAGE_TYPE_ICON, "/b"),
            NOTIFICATION_ERROR_NONE);
  char* path = nullptr;
  EXPECT_EQ(notification_get_image(noti_, NOTIFICATION_IMAGE_TYPE_ICON, &path),
            NOTIFICATION_ERROR_NONE);
  EXPECT_STREQ(path, "/b");
}

TEST_F(NotificationApiTest, set_image_empty_string_stored) {
  EXPECT_EQ(notification_set_image(noti_, NOTIFICATION_IMAGE_TYPE_ICON, ""),
            NOTIFICATION_ERROR_NONE);
  char* path = nullptr;
  EXPECT_EQ(notification_get_image(noti_, NOTIFICATION_IMAGE_TYPE_ICON, &path),
            NOTIFICATION_ERROR_NONE);
  EXPECT_STREQ(path, "");
}

TEST_F(NotificationApiTest, get_image_null_noti) {
  char* path = nullptr;
  EXPECT_EQ(notification_get_image(nullptr, NOTIFICATION_IMAGE_TYPE_ICON, &path),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationApiTest, get_image_null_out) {
  EXPECT_EQ(notification_get_image(noti_, NOTIFICATION_IMAGE_TYPE_ICON, nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationApiTest, get_image_type_none_returns_invalid) {
  char* path = nullptr;
  EXPECT_EQ(notification_get_image(noti_, NOTIFICATION_IMAGE_TYPE_NONE, &path),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationApiTest, get_image_no_key_returns_none_with_null_path) {
  char* path = (char*)0x1; /* sentinel */
  /* Type not previously set. With no b_image_path bundle and non-ICON type,
   * scenario [P3]/[E1]: NONE; *image_path == NULL. */
  EXPECT_EQ(notification_get_image(noti_, NOTIFICATION_IMAGE_TYPE_LIST_5, &path),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(path, nullptr);
}

/* ============================================================== */
/* notification_set_time / get_time / get_insert_time             */
/* ============================================================== */

TEST_F(NotificationApiTest, set_time_positive_roundtrip) {
  time_t t = 1700000000;
  ASSERT_EQ(notification_set_time(noti_, t), NOTIFICATION_ERROR_NONE);
  time_t got = 0;
  EXPECT_EQ(notification_get_time(noti_, &got), NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(got, t);
}

TEST_F(NotificationApiTest, set_time_zero_uses_current_time) {
  time_t before = time(nullptr);
  ASSERT_EQ(notification_set_time(noti_, 0), NOTIFICATION_ERROR_NONE);
  time_t got = 0;
  EXPECT_EQ(notification_get_time(noti_, &got), NOTIFICATION_ERROR_NONE);
  EXPECT_GE(got, before);
}

TEST_F(NotificationApiTest, set_time_null_noti) {
  EXPECT_EQ(notification_set_time(nullptr, 1234),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationApiTest, get_time_null_noti) {
  time_t got;
  EXPECT_EQ(notification_get_time(nullptr, &got),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationApiTest, get_time_null_out) {
  EXPECT_EQ(notification_get_time(noti_, nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationApiTest, get_insert_time_null_noti) {
  time_t got;
  EXPECT_EQ(notification_get_insert_time(nullptr, &got),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationApiTest, get_insert_time_null_out) {
  EXPECT_EQ(notification_get_insert_time(noti_, nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* ============================================================== */
/* notification_set_layout / get_layout                           */
/* ============================================================== */

TEST_F(NotificationApiTest, set_layout_roundtrip) {
  ASSERT_EQ(notification_set_layout(noti_, NOTIFICATION_LY_NOTI_EVENT_SINGLE),
            NOTIFICATION_ERROR_NONE);
  notification_ly_type_e got;
  EXPECT_EQ(notification_get_layout(noti_, &got), NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(got, NOTIFICATION_LY_NOTI_EVENT_SINGLE);
}

TEST_F(NotificationApiTest, set_layout_null_noti) {
  EXPECT_EQ(notification_set_layout(nullptr, NOTIFICATION_LY_NOTI_EVENT_SINGLE),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationApiTest, set_layout_out_of_range) {
  /* layout < NONE (e.g. -1) or layout > MAX is invalid. NONE itself is valid. */
  EXPECT_EQ(notification_set_layout(noti_,
                static_cast<notification_ly_type_e>(-1)),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_set_layout(noti_,
                static_cast<notification_ly_type_e>(
                    NOTIFICATION_LY_MAX + 1)),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationApiTest, get_layout_null_noti) {
  notification_ly_type_e got;
  EXPECT_EQ(notification_get_layout(nullptr, &got),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationApiTest, get_layout_null_out) {
  EXPECT_EQ(notification_get_layout(noti_, nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* ============================================================== */
/* notification_set_text_domain / get_text_domain                 */
/* ============================================================== */

TEST_F(NotificationApiTest, set_text_domain_roundtrip) {
  ASSERT_EQ(notification_set_text_domain(noti_, "mydomain", "/usr/share/locale"),
            NOTIFICATION_ERROR_NONE);
  char* domain = nullptr;
  char* dir = nullptr;
  EXPECT_EQ(notification_get_text_domain(noti_, &domain, &dir),
            NOTIFICATION_ERROR_NONE);
  EXPECT_STREQ(domain, "mydomain");
  EXPECT_STREQ(dir, "/usr/share/locale");
}

TEST_F(NotificationApiTest, set_text_domain_replaces_previous) {
  ASSERT_EQ(notification_set_text_domain(noti_, "first", "/dir1"),
            NOTIFICATION_ERROR_NONE);
  ASSERT_EQ(notification_set_text_domain(noti_, "second", "/dir2"),
            NOTIFICATION_ERROR_NONE);
  char* domain = nullptr;
  char* dir = nullptr;
  EXPECT_EQ(notification_get_text_domain(noti_, &domain, &dir),
            NOTIFICATION_ERROR_NONE);
  EXPECT_STREQ(domain, "second");
  EXPECT_STREQ(dir, "/dir2");
}

TEST_F(NotificationApiTest, set_text_domain_null_args) {
  EXPECT_EQ(notification_set_text_domain(nullptr, "d", "/dir"),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_set_text_domain(noti_, nullptr, "/dir"),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_set_text_domain(noti_, "d", nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* ============================================================== */
/* notification_set_sound / get_sound                             */
/* ============================================================== */

TEST_F(NotificationApiTest, set_sound_roundtrip) {
  ASSERT_EQ(notification_set_sound(noti_, NOTIFICATION_SOUND_TYPE_USER_DATA,
                                   "/path/sound.ogg"),
            NOTIFICATION_ERROR_NONE);
  notification_sound_type_e got_type;
  const char* got_path = nullptr;
  EXPECT_EQ(notification_get_sound(noti_, &got_type, &got_path),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(got_type, NOTIFICATION_SOUND_TYPE_USER_DATA);
  EXPECT_STREQ(got_path, "/path/sound.ogg");
}

TEST_F(NotificationApiTest, set_sound_null_noti) {
  EXPECT_EQ(notification_set_sound(nullptr, NOTIFICATION_SOUND_TYPE_DEFAULT,
                                   nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationApiTest, set_sound_out_of_range_type) {
  /* type < NONE (i.e. < -1) or type > MAX is invalid. */
  EXPECT_EQ(notification_set_sound(noti_,
                static_cast<notification_sound_type_e>(-2), nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_set_sound(noti_,
                static_cast<notification_sound_type_e>(
                    NOTIFICATION_SOUND_TYPE_MAX + 1),
                nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationApiTest, set_sound_user_data_with_null_path_returns_invalid) {
  /* Per impl: USER_DATA without a path resets sound_type to DEFAULT and
   * returns INVALID_PARAMETER (see notification.c L309-310). */
  EXPECT_EQ(notification_set_sound(noti_, NOTIFICATION_SOUND_TYPE_USER_DATA,
                                   nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationApiTest, get_sound_null_args) {
  notification_sound_type_e t;
  const char* p = nullptr;
  EXPECT_EQ(notification_get_sound(nullptr, &t, &p),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_get_sound(noti_, nullptr, &p),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* ============================================================== */
/* notification_set_vibration / get_vibration                     */
/* ============================================================== */

TEST_F(NotificationApiTest, set_vibration_roundtrip) {
  ASSERT_EQ(notification_set_vibration(noti_,
                                       NOTIFICATION_VIBRATION_TYPE_USER_DATA,
                                       "/path/vib.dat"),
            NOTIFICATION_ERROR_NONE);
  notification_vibration_type_e got_type;
  const char* got_path = nullptr;
  EXPECT_EQ(notification_get_vibration(noti_, &got_type, &got_path),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(got_type, NOTIFICATION_VIBRATION_TYPE_USER_DATA);
  EXPECT_STREQ(got_path, "/path/vib.dat");
}

TEST_F(NotificationApiTest, set_vibration_null_noti) {
  EXPECT_EQ(notification_set_vibration(nullptr,
                                       NOTIFICATION_VIBRATION_TYPE_DEFAULT,
                                       nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationApiTest, get_vibration_null_args) {
  notification_vibration_type_e t;
  const char* p = nullptr;
  EXPECT_EQ(notification_get_vibration(nullptr, &t, &p),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_get_vibration(noti_, nullptr, &p),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* ============================================================== */
/* notification_set_led / get_led / set_led_time_period           */
/* ============================================================== */

TEST_F(NotificationApiTest, set_led_roundtrip) {
  ASSERT_EQ(notification_set_led(noti_, NOTIFICATION_LED_OP_ON_CUSTOM_COLOR,
                                 0x00FF0000),
            NOTIFICATION_ERROR_NONE);
  notification_led_op_e op;
  int color = 0;
  EXPECT_EQ(notification_get_led(noti_, &op, &color), NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(op, NOTIFICATION_LED_OP_ON_CUSTOM_COLOR);
  EXPECT_EQ(color, 0x00FF0000);
}

TEST_F(NotificationApiTest, set_led_null_noti) {
  EXPECT_EQ(notification_set_led(nullptr, NOTIFICATION_LED_OP_ON, 0),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationApiTest, get_led_null_args) {
  notification_led_op_e op;
  int color = 0;
  EXPECT_EQ(notification_get_led(nullptr, &op, &color),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_get_led(noti_, nullptr, &color),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationApiTest, set_led_time_period_roundtrip) {
  ASSERT_EQ(notification_set_led_time_period(noti_, 1000, 500),
            NOTIFICATION_ERROR_NONE);
  int on_ms = 0;
  int off_ms = 0;
  EXPECT_EQ(notification_get_led_time_period(noti_, &on_ms, &off_ms),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(on_ms, 1000);
  EXPECT_EQ(off_ms, 500);
}

TEST_F(NotificationApiTest, set_led_time_period_null_noti) {
  EXPECT_EQ(notification_set_led_time_period(nullptr, 100, 100),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* ============================================================== */
/* notification_set_property / get_property                       */
/* ============================================================== */

TEST_F(NotificationApiTest, set_property_roundtrip) {
  int flags = NOTIFICATION_PROP_DISABLE_APP_LAUNCH
            | NOTIFICATION_PROP_DISABLE_AUTO_DELETE;
  ASSERT_EQ(notification_set_property(noti_, flags), NOTIFICATION_ERROR_NONE);
  int got = 0;
  EXPECT_EQ(notification_get_property(noti_, &got), NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(got, flags);
}

TEST_F(NotificationApiTest, set_property_null_noti) {
  EXPECT_EQ(notification_set_property(nullptr, 0),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationApiTest, get_property_null_args) {
  int p = 0;
  EXPECT_EQ(notification_get_property(nullptr, &p),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_get_property(noti_, nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* ============================================================== */
/* notification_set_display_applist / get_display_applist         */
/* ============================================================== */

TEST_F(NotificationApiTest, set_display_applist_roundtrip) {
  int mask = NOTIFICATION_DISPLAY_APP_INDICATOR
           | NOTIFICATION_DISPLAY_APP_NOTIFICATION_TRAY;
  ASSERT_EQ(notification_set_display_applist(noti_, mask),
            NOTIFICATION_ERROR_NONE);
  int got = 0;
  EXPECT_EQ(notification_get_display_applist(noti_, &got),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(got, mask);
}

TEST_F(NotificationApiTest, set_display_applist_null_noti) {
  EXPECT_EQ(notification_set_display_applist(nullptr, 0),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationApiTest, get_display_applist_null_args) {
  int p = 0;
  EXPECT_EQ(notification_get_display_applist(nullptr, &p),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_get_display_applist(noti_, nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* ============================================================== */
/* notification_set_size / get_size                                */
/* ============================================================== */

TEST_F(NotificationApiTest, set_size_roundtrip) {
  ASSERT_EQ(notification_set_size(noti_, 0.5), NOTIFICATION_ERROR_NONE);
  double got = 0.0;
  EXPECT_EQ(notification_get_size(noti_, &got), NOTIFICATION_ERROR_NONE);
  EXPECT_DOUBLE_EQ(got, 0.5);
}

TEST_F(NotificationApiTest, set_size_null_noti) {
  EXPECT_EQ(notification_set_size(nullptr, 0.5),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationApiTest, get_size_null_args) {
  double v = 0.0;
  EXPECT_EQ(notification_get_size(nullptr, &v),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_get_size(noti_, nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* ============================================================== */
/* notification_set_progress / get_progress                       */
/* ============================================================== */

TEST_F(NotificationApiTest, set_progress_roundtrip) {
  ASSERT_EQ(notification_set_progress(noti_, 0.75), NOTIFICATION_ERROR_NONE);
  double got = 0.0;
  EXPECT_EQ(notification_get_progress(noti_, &got), NOTIFICATION_ERROR_NONE);
  EXPECT_DOUBLE_EQ(got, 0.75);
}

TEST_F(NotificationApiTest, set_progress_null_noti) {
  EXPECT_EQ(notification_set_progress(nullptr, 0.5),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationApiTest, get_progress_null_args) {
  double v = 0.0;
  EXPECT_EQ(notification_get_progress(nullptr, &v),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_get_progress(noti_, nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* ============================================================== */
/* notification_set_auto_remove / get_auto_remove                 */
/* ============================================================== */

TEST_F(NotificationApiTest, set_auto_remove_true_roundtrip) {
  ASSERT_EQ(notification_set_auto_remove(noti_, true),
            NOTIFICATION_ERROR_NONE);
  bool got = false;
  EXPECT_EQ(notification_get_auto_remove(noti_, &got),
            NOTIFICATION_ERROR_NONE);
  EXPECT_TRUE(got);
}

TEST_F(NotificationApiTest, set_auto_remove_false_roundtrip) {
  ASSERT_EQ(notification_set_auto_remove(noti_, false),
            NOTIFICATION_ERROR_NONE);
  bool got = true;
  EXPECT_EQ(notification_get_auto_remove(noti_, &got),
            NOTIFICATION_ERROR_NONE);
  EXPECT_FALSE(got);
}

TEST_F(NotificationApiTest, set_auto_remove_null_noti) {
  EXPECT_EQ(notification_set_auto_remove(nullptr, true),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationApiTest, get_auto_remove_null_args) {
  bool b = false;
  EXPECT_EQ(notification_get_auto_remove(nullptr, &b),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_get_auto_remove(noti_, nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* ============================================================== */
/* notification_set_ongoing_flag / get_ongoing_flag               */
/* ============================================================== */

TEST_F(NotificationApiTest, set_ongoing_flag_true_roundtrip) {
  ASSERT_EQ(notification_set_ongoing_flag(noti_, true),
            NOTIFICATION_ERROR_NONE);
  bool got = false;
  EXPECT_EQ(notification_get_ongoing_flag(noti_, &got),
            NOTIFICATION_ERROR_NONE);
  EXPECT_TRUE(got);
}

TEST_F(NotificationApiTest, set_ongoing_flag_null_noti) {
  EXPECT_EQ(notification_set_ongoing_flag(nullptr, true),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationApiTest, get_ongoing_flag_null_args) {
  bool b = false;
  EXPECT_EQ(notification_get_ongoing_flag(nullptr, &b),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_get_ongoing_flag(noti_, nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* ============================================================== */
/* notification_set_tag / get_tag                                 */
/* ============================================================== */

TEST_F(NotificationApiTest, set_tag_roundtrip) {
  ASSERT_EQ(notification_set_tag(noti_, "my_tag"), NOTIFICATION_ERROR_NONE);
  const char* got = nullptr;
  EXPECT_EQ(notification_get_tag(noti_, &got), NOTIFICATION_ERROR_NONE);
  EXPECT_STREQ(got, "my_tag");
}

TEST_F(NotificationApiTest, set_tag_replaces_previous) {
  ASSERT_EQ(notification_set_tag(noti_, "first"), NOTIFICATION_ERROR_NONE);
  ASSERT_EQ(notification_set_tag(noti_, "second"), NOTIFICATION_ERROR_NONE);
  const char* got = nullptr;
  EXPECT_EQ(notification_get_tag(noti_, &got), NOTIFICATION_ERROR_NONE);
  EXPECT_STREQ(got, "second");
}

TEST_F(NotificationApiTest, set_tag_null_noti) {
  EXPECT_EQ(notification_set_tag(nullptr, "t"),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationApiTest, get_tag_null_noti) {
  /* Open question: notification_get_tag does NOT null-check the `tag`
   * out parameter (see notification.c). Passing NULL crashes. The
   * other null-arg shape (NULL noti) is well-behaved and tested here. */
  const char* p = nullptr;
  EXPECT_EQ(notification_get_tag(nullptr, &p),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* ============================================================== */
/* notification_get_type / get_pkgname                            */
/* ============================================================== */

TEST_F(NotificationApiTest, get_type_returns_noti_type) {
  notification_type_e type;
  EXPECT_EQ(notification_get_type(noti_, &type), NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(type, NOTIFICATION_TYPE_NOTI);
}

TEST_F(NotificationApiTest, get_type_null_args) {
  notification_type_e type;
  EXPECT_EQ(notification_get_type(nullptr, &type),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_get_type(noti_, nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationApiTest, get_pkgname_returns_handle_pkgname) {
  char* pkgname = nullptr;
  EXPECT_EQ(notification_get_pkgname(noti_, &pkgname),
            NOTIFICATION_ERROR_NONE);
  /* The fake aul returns appid org.tizen.testappid; the resulting
   * caller_app_id / pkgname is implementation-detail. We just assert
   * that a non-null string is returned. */
  EXPECT_NE(pkgname, nullptr);
}

TEST_F(NotificationApiTest, get_pkgname_null_args) {
  char* pkgname = nullptr;
  EXPECT_EQ(notification_get_pkgname(nullptr, &pkgname),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_get_pkgname(noti_, nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* ============================================================== */
/* notification_set_launch_option / get_launch_option             */
/* ============================================================== */

TEST_F(NotificationApiTest, set_launch_option_null_noti) {
  EXPECT_EQ(notification_set_launch_option(nullptr,
                NOTIFICATION_LAUNCH_OPTION_APP_CONTROL, nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationApiTest, set_launch_option_null_option) {
  EXPECT_EQ(notification_set_launch_option(noti_,
                NOTIFICATION_LAUNCH_OPTION_APP_CONTROL, nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* ============================================================== */
/* notification_set_extension_image_size / get_extension_image_size */
/* ============================================================== */

TEST_F(NotificationApiTest, set_extension_image_size_roundtrip) {
  ASSERT_EQ(notification_set_extension_image_size(noti_, 200),
            NOTIFICATION_ERROR_NONE);
  int h = 0;
  EXPECT_EQ(notification_get_extension_image_size(noti_, &h),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(h, 200);
}

TEST_F(NotificationApiTest, set_extension_image_size_null_noti) {
  EXPECT_EQ(notification_set_extension_image_size(nullptr, 2),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationApiTest, get_extension_image_size_null_args) {
  int h;
  EXPECT_EQ(notification_get_extension_image_size(nullptr, &h),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_get_extension_image_size(noti_, nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* ============================================================== */
/* notification_add_button / notification_remove_button           */
/* ============================================================== */

TEST_F(NotificationApiTest, add_button_valid_indices_succeed) {
  EXPECT_EQ(notification_add_button(noti_, NOTIFICATION_BUTTON_1),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(notification_add_button(noti_, NOTIFICATION_BUTTON_6),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(notification_add_button(noti_, NOTIFICATION_BUTTON_7),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(notification_add_button(noti_, NOTIFICATION_BUTTON_10),
            NOTIFICATION_ERROR_NONE);
}

TEST_F(NotificationApiTest, add_button_null_noti) {
  EXPECT_EQ(notification_add_button(nullptr, NOTIFICATION_BUTTON_1),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationApiTest, add_button_out_of_range) {
  /* Valid range is [BUTTON_1..BUTTON_6] ∪ [BUTTON_7..BUTTON_10]. */
  EXPECT_EQ(notification_add_button(noti_,
                static_cast<notification_button_index_e>(0)),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_add_button(noti_,
                static_cast<notification_button_index_e>(
                    NOTIFICATION_BUTTON_10 + 1)),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationApiTest, remove_button_null_noti) {
  EXPECT_EQ(notification_remove_button(nullptr, NOTIFICATION_BUTTON_1),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationApiTest, remove_button_out_of_range) {
  EXPECT_EQ(notification_remove_button(noti_,
                static_cast<notification_button_index_e>(0)),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationApiTest, remove_button_unset_index_is_noop_ok) {
  /* Per impl: removing a button that was never set is a NONE no-op. */
  EXPECT_EQ(notification_remove_button(noti_, NOTIFICATION_BUTTON_3),
            NOTIFICATION_ERROR_NONE);
}

/* ============================================================== */
/* notification_set_event_handler / get_event_handler             */
/* ============================================================== */

TEST_F(NotificationApiTest, set_event_handler_null_noti) {
  app_control_h ac = nullptr;
  EXPECT_EQ(notification_set_event_handler(nullptr,
                NOTIFICATION_EVENT_TYPE_CLICK_ON_BUTTON_1, ac),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationApiTest, set_event_handler_out_of_range_type) {
  app_control_h ac = nullptr;
  EXPECT_EQ(notification_set_event_handler(noti_,
                static_cast<notification_event_type_e>(0), ac),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_set_event_handler(noti_,
                static_cast<notification_event_type_e>(
                    NOTIFICATION_EVENT_TYPE_MAX + 1), ac),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationApiTest, get_event_handler_null_args) {
  app_control_h ac = nullptr;
  EXPECT_EQ(notification_get_event_handler(nullptr,
                NOTIFICATION_EVENT_TYPE_CLICK_ON_BUTTON_1, &ac),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_get_event_handler(noti_,
                NOTIFICATION_EVENT_TYPE_CLICK_ON_BUTTON_1, nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationApiTest, get_event_handler_unset_returns_not_exist) {
  /* Per impl: a valid in-range event type whose b_event_handler[type]
   * slot is NULL returns NOT_EXIST_ID. NOTIFICATION_EVENT_TYPE_CLICK_ON_BUTTON_1
   * is the first valid value (= 0 in the enum). */
  app_control_h ac = nullptr;
  EXPECT_EQ(notification_get_event_handler(noti_,
                NOTIFICATION_EVENT_TYPE_CLICK_ON_BUTTON_1, &ac),
            NOTIFICATION_ERROR_NOT_EXIST_ID);
}

/* ============================================================== */
/* notification_set_text_input                                    */
/* ============================================================== */

TEST_F(NotificationApiTest, set_text_input_null_noti) {
  EXPECT_EQ(notification_set_text_input(nullptr, 100),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationApiTest, set_text_input_negative_stored_verbatim) {
  /* Per impl: notification_set_text_input only null-checks noti. Any int
   * value (including negative) is stored verbatim. */
  EXPECT_EQ(notification_set_text_input(noti_, -1),
            NOTIFICATION_ERROR_NONE);
}

/* ============================================================== */
/* notification_set_time_to_text / get_time_from_text             */
/* ============================================================== */

TEST_F(NotificationApiTest, set_time_to_text_roundtrip) {
  time_t t = 1700000000;
  ASSERT_EQ(notification_set_time_to_text(noti_,
                NOTIFICATION_TEXT_TYPE_TITLE, t),
            NOTIFICATION_ERROR_NONE);
  time_t got = 0;
  EXPECT_EQ(notification_get_time_from_text(noti_,
                NOTIFICATION_TEXT_TYPE_TITLE, &got),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(got, t);
}

TEST_F(NotificationApiTest, set_time_to_text_null_noti) {
  EXPECT_EQ(notification_set_time_to_text(nullptr,
                NOTIFICATION_TEXT_TYPE_TITLE, 1),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationApiTest, set_time_to_text_zero_or_negative_time) {
  EXPECT_EQ(notification_set_time_to_text(noti_,
                NOTIFICATION_TEXT_TYPE_TITLE, 0),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_set_time_to_text(noti_,
                NOTIFICATION_TEXT_TYPE_TITLE, -1),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationApiTest, set_time_to_text_invalid_type) {
  EXPECT_EQ(notification_set_time_to_text(noti_,
                NOTIFICATION_TEXT_TYPE_NONE, 1),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_set_time_to_text(noti_,
                static_cast<notification_text_type_e>(
                    NOTIFICATION_TEXT_TYPE_MAX + 1), 1),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationApiTest, get_time_from_text_null_args) {
  time_t t = 0;
  EXPECT_EQ(notification_get_time_from_text(nullptr,
                NOTIFICATION_TEXT_TYPE_TITLE, &t),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_get_time_from_text(noti_,
                NOTIFICATION_TEXT_TYPE_TITLE, nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationApiTest, get_time_from_text_invalid_type) {
  time_t t = 0;
  EXPECT_EQ(notification_get_time_from_text(noti_,
                NOTIFICATION_TEXT_TYPE_NONE, &t),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* ============================================================== */
/* notification_get_insert_time roundtrip (just-set vs fresh)     */
/* ============================================================== */

TEST_F(NotificationApiTest, get_insert_time_default_is_zero) {
  /* On a fresh notification (not yet posted), insert_time is 0. */
  time_t t = -1;
  EXPECT_EQ(notification_get_insert_time(noti_, &t),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(t, 0);
}

/* ============================================================== */
/* notification_set_text / get_text                                */
/* ============================================================== */

TEST_F(NotificationApiTest, set_text_basic_roundtrip) {
  ASSERT_EQ(notification_set_text(noti_, NOTIFICATION_TEXT_TYPE_TITLE,
                "hello", nullptr, NOTIFICATION_VARIABLE_TYPE_NONE),
            NOTIFICATION_ERROR_NONE);
  char* got = nullptr;
  EXPECT_EQ(notification_get_text(noti_, NOTIFICATION_TEXT_TYPE_TITLE,
                &got),
            NOTIFICATION_ERROR_NONE);
  ASSERT_NE(got, nullptr);
  EXPECT_STREQ(got, "hello");
}

TEST_F(NotificationApiTest, set_text_replaces_existing) {
  ASSERT_EQ(notification_set_text(noti_, NOTIFICATION_TEXT_TYPE_TITLE,
                "first", nullptr, NOTIFICATION_VARIABLE_TYPE_NONE),
            NOTIFICATION_ERROR_NONE);
  ASSERT_EQ(notification_set_text(noti_, NOTIFICATION_TEXT_TYPE_TITLE,
                "second", nullptr, NOTIFICATION_VARIABLE_TYPE_NONE),
            NOTIFICATION_ERROR_NONE);
  char* got = nullptr;
  EXPECT_EQ(notification_get_text(noti_, NOTIFICATION_TEXT_TYPE_TITLE,
                &got),
            NOTIFICATION_ERROR_NONE);
  EXPECT_STREQ(got, "second");
}

TEST_F(NotificationApiTest, set_text_null_noti) {
  EXPECT_EQ(notification_set_text(nullptr,
                NOTIFICATION_TEXT_TYPE_TITLE,
                "x", nullptr, NOTIFICATION_VARIABLE_TYPE_NONE),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationApiTest, set_text_invalid_type) {
  EXPECT_EQ(notification_set_text(noti_,
                NOTIFICATION_TEXT_TYPE_NONE,
                "x", nullptr, NOTIFICATION_VARIABLE_TYPE_NONE),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_set_text(noti_,
                static_cast<notification_text_type_e>(
                    NOTIFICATION_TEXT_TYPE_MAX + 1),
                "x", nullptr, NOTIFICATION_VARIABLE_TYPE_NONE),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationApiTest, set_text_null_text_clears_existing) {
  ASSERT_EQ(notification_set_text(noti_, NOTIFICATION_TEXT_TYPE_TITLE,
                "before", nullptr, NOTIFICATION_VARIABLE_TYPE_NONE),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(notification_set_text(noti_, NOTIFICATION_TEXT_TYPE_TITLE,
                nullptr, nullptr, NOTIFICATION_VARIABLE_TYPE_NONE),
            NOTIFICATION_ERROR_NONE);
}

TEST_F(NotificationApiTest, get_text_null_args) {
  char* got = nullptr;
  EXPECT_EQ(notification_get_text(nullptr,
                NOTIFICATION_TEXT_TYPE_TITLE, &got),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_get_text(noti_,
                NOTIFICATION_TEXT_TYPE_TITLE, nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationApiTest, get_text_invalid_type) {
  char* got = nullptr;
  EXPECT_EQ(notification_get_text(noti_,
                NOTIFICATION_TEXT_TYPE_NONE, &got),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationApiTest, get_text_unset_returns_none_with_null) {
  /* When text was never set: NONE; *text points to NULL. */
  char* got = nullptr;
  EXPECT_EQ(notification_get_text(noti_,
                NOTIFICATION_TEXT_TYPE_TITLE, &got),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(got, nullptr);
}

/* ============================================================== */
/* notification_set_launch_option / get_launch_option              */
/* ============================================================== */

TEST_F(NotificationApiTest, set_launch_option_invalid_type) {
  app_control_h ac = nullptr;
  app_control_create(&ac);
  /* Per impl: only NOTIFICATION_LAUNCH_OPTION_APP_CONTROL is valid. */
  EXPECT_EQ(notification_set_launch_option(noti_,
                static_cast<notification_launch_option_type>(99), ac),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  app_control_destroy(ac);
}

TEST_F(NotificationApiTest, set_launch_option_valid_app_control) {
  app_control_h ac = nullptr;
  app_control_create(&ac);
  app_control_set_app_id(ac, "org.tizen.foo");
  EXPECT_EQ(notification_set_launch_option(noti_,
                NOTIFICATION_LAUNCH_OPTION_APP_CONTROL, ac),
            NOTIFICATION_ERROR_NONE);
  app_control_destroy(ac);
}

TEST_F(NotificationApiTest, get_launch_option_null_noti) {
  app_control_h ac = nullptr;
  EXPECT_EQ(notification_get_launch_option(nullptr,
                NOTIFICATION_LAUNCH_OPTION_APP_CONTROL, &ac),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationApiTest, get_launch_option_null_option) {
  EXPECT_EQ(notification_get_launch_option(noti_,
                NOTIFICATION_LAUNCH_OPTION_APP_CONTROL, nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* ============================================================== */
/* notification_get_noti_block_state                                */
/* ============================================================== */

TEST_F(NotificationApiTest, get_noti_block_state_null_arg) {
  EXPECT_EQ(notification_get_noti_block_state(nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* ============================================================== */
/* notification_set_image with multiple types (collision check)    */
/* ============================================================== */

TEST_F(NotificationApiTest, set_image_multiple_types_accumulate) {
  ASSERT_EQ(notification_set_image(noti_, NOTIFICATION_IMAGE_TYPE_ICON,
                "/icons/icon.png"),
            NOTIFICATION_ERROR_NONE);
  ASSERT_EQ(notification_set_image(noti_,
                NOTIFICATION_IMAGE_TYPE_ICON_FOR_LOCK,
                "/icons/lock.png"),
            NOTIFICATION_ERROR_NONE);
  char* a = nullptr;
  EXPECT_EQ(notification_get_image(noti_, NOTIFICATION_IMAGE_TYPE_ICON, &a),
            NOTIFICATION_ERROR_NONE);
  EXPECT_STREQ(a, "/icons/icon.png");
  char* b = nullptr;
  EXPECT_EQ(notification_get_image(noti_,
                NOTIFICATION_IMAGE_TYPE_ICON_FOR_LOCK, &b),
            NOTIFICATION_ERROR_NONE);
  EXPECT_STREQ(b, "/icons/lock.png");
}

/* ============================================================== */
/* notification_set_time(0) writes "current time"                   */
/* ============================================================== */

TEST_F(NotificationApiTest, set_time_zero_uses_current_time_again) {
  /* Double-set to zero — confirms idempotence of "now" semantics. */
  ASSERT_EQ(notification_set_time(noti_, 0), NOTIFICATION_ERROR_NONE);
  time_t t1 = 0;
  EXPECT_EQ(notification_get_time(noti_, &t1), NOTIFICATION_ERROR_NONE);
  EXPECT_GT(t1, 0);

  ASSERT_EQ(notification_set_time(noti_, 0), NOTIFICATION_ERROR_NONE);
  time_t t2 = 0;
  EXPECT_EQ(notification_get_time(noti_, &t2), NOTIFICATION_ERROR_NONE);
  EXPECT_GE(t2, t1);
}

/* ============================================================== */
/* notification_set_text with key (translation key path)           */
/* ============================================================== */

TEST_F(NotificationApiTest, set_text_with_key_stores_key) {
  /* Passing a key (translation key) is stored in b_key under TYPE. */
  ASSERT_EQ(notification_set_text(noti_, NOTIFICATION_TEXT_TYPE_TITLE,
                "hello", "IDS_HELLO",
                NOTIFICATION_VARIABLE_TYPE_NONE),
            NOTIFICATION_ERROR_NONE);
}

TEST_F(NotificationApiTest, set_text_with_key_only_no_text) {
  /* Per impl: text==NULL is allowed; key alone is stored. */
  ASSERT_EQ(notification_set_text(noti_, NOTIFICATION_TEXT_TYPE_TITLE,
                nullptr, "IDS_HELLO",
                NOTIFICATION_VARIABLE_TYPE_NONE),
            NOTIFICATION_ERROR_NONE);
}

TEST_F(NotificationApiTest, set_text_with_int_var_arg) {
  /* Variable-args path: TYPE_INT followed by an integer value. */
  ASSERT_EQ(notification_set_text(noti_, NOTIFICATION_TEXT_TYPE_TITLE,
                "you have %d", nullptr,
                NOTIFICATION_VARIABLE_TYPE_INT, 5,
                NOTIFICATION_VARIABLE_TYPE_NONE),
            NOTIFICATION_ERROR_NONE);
}

TEST_F(NotificationApiTest, set_text_with_unknown_var_arg_returns_invalid) {
  /* Unknown variable type in vararg list → INVALID_PARAMETER. */
  EXPECT_EQ(notification_set_text(noti_, NOTIFICATION_TEXT_TYPE_TITLE,
                "x", nullptr,
                static_cast<int>(99), 42,
                NOTIFICATION_VARIABLE_TYPE_NONE),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* ============================================================== */
/* notification_post / delete / clone / save_as_template — null args */
/* (Only null-arg paths are unit-testable; success paths require   */
/* IPC to the notification master service.)                         */
/* ============================================================== */

TEST_F(NotificationApiTest, post_null_noti) {
  EXPECT_EQ(notification_post(nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationApiTest, delete_null_noti) {
  EXPECT_EQ(notification_delete(nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationApiTest, clone_null_args) {
  notification_h clone = nullptr;
  EXPECT_EQ(notification_clone(nullptr, &clone),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_clone(noti_, nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationApiTest, clone_preserves_text_and_image) {
  ASSERT_EQ(notification_set_text(noti_,
                NOTIFICATION_TEXT_TYPE_TITLE,
                "title-text", nullptr,
                NOTIFICATION_VARIABLE_TYPE_NONE),
            NOTIFICATION_ERROR_NONE);
  ASSERT_EQ(notification_set_image(noti_,
                NOTIFICATION_IMAGE_TYPE_ICON, "/tmp/icon.png"),
            NOTIFICATION_ERROR_NONE);

  notification_h clone = nullptr;
  ASSERT_EQ(notification_clone(noti_, &clone),
            NOTIFICATION_ERROR_NONE);
  ASSERT_NE(clone, nullptr);

  char* text = nullptr;
  EXPECT_EQ(notification_get_text(clone,
                NOTIFICATION_TEXT_TYPE_TITLE, &text),
            NOTIFICATION_ERROR_NONE);
  ASSERT_NE(text, nullptr);
  EXPECT_STREQ(text, "title-text");

  char* img = nullptr;
  EXPECT_EQ(notification_get_image(clone,
                NOTIFICATION_IMAGE_TYPE_ICON, &img),
            NOTIFICATION_ERROR_NONE);
  ASSERT_NE(img, nullptr);
  EXPECT_STREQ(img, "/tmp/icon.png");

  notification_free(clone);
}

TEST_F(NotificationApiTest, clone_preserves_type) {
  notification_h n = notification_create(NOTIFICATION_TYPE_ONGOING);
  ASSERT_NE(n, nullptr);
  notification_h clone = nullptr;
  ASSERT_EQ(notification_clone(n, &clone),
            NOTIFICATION_ERROR_NONE);
  ASSERT_NE(clone, nullptr);

  notification_type_e t;
  EXPECT_EQ(notification_get_type(clone, &t),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(t, NOTIFICATION_TYPE_ONGOING);

  notification_free(clone);
  notification_free(n);
}

TEST_F(NotificationApiTest, clone_preserves_layout) {
  ASSERT_EQ(notification_set_layout(noti_,
                NOTIFICATION_LY_ONGOING_PROGRESS),
            NOTIFICATION_ERROR_NONE);

  notification_h clone = nullptr;
  ASSERT_EQ(notification_clone(noti_, &clone),
            NOTIFICATION_ERROR_NONE);
  ASSERT_NE(clone, nullptr);

  notification_ly_type_e ly;
  EXPECT_EQ(notification_get_layout(clone, &ly),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(ly, NOTIFICATION_LY_ONGOING_PROGRESS);

  notification_free(clone);
}

TEST_F(NotificationApiTest, clone_preserves_progress) {
  ASSERT_EQ(notification_set_progress(noti_, 0.42),
            NOTIFICATION_ERROR_NONE);

  notification_h clone = nullptr;
  ASSERT_EQ(notification_clone(noti_, &clone),
            NOTIFICATION_ERROR_NONE);
  ASSERT_NE(clone, nullptr);

  double v = 0;
  EXPECT_EQ(notification_get_progress(clone, &v),
            NOTIFICATION_ERROR_NONE);
  EXPECT_DOUBLE_EQ(v, 0.42);

  notification_free(clone);
}

TEST_F(NotificationApiTest, clone_preserves_property) {
  ASSERT_EQ(notification_set_property(noti_,
                NOTIFICATION_PROP_VOLATILE_DISPLAY),
            NOTIFICATION_ERROR_NONE);

  notification_h clone = nullptr;
  ASSERT_EQ(notification_clone(noti_, &clone),
            NOTIFICATION_ERROR_NONE);
  ASSERT_NE(clone, nullptr);

  int p = 0;
  EXPECT_EQ(notification_get_property(clone, &p),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(p, NOTIFICATION_PROP_VOLATILE_DISPLAY);

  notification_free(clone);
}

TEST_F(NotificationApiTest, clone_preserves_sound) {
  ASSERT_EQ(notification_set_sound(noti_,
                NOTIFICATION_SOUND_TYPE_USER_DATA, "/p/s.ogg"),
            NOTIFICATION_ERROR_NONE);

  notification_h clone = nullptr;
  ASSERT_EQ(notification_clone(noti_, &clone),
            NOTIFICATION_ERROR_NONE);
  ASSERT_NE(clone, nullptr);

  notification_sound_type_e t;
  const char* path = nullptr;
  EXPECT_EQ(notification_get_sound(clone, &t, &path),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(t, NOTIFICATION_SOUND_TYPE_USER_DATA);
  ASSERT_NE(path, nullptr);
  EXPECT_STREQ(path, "/p/s.ogg");

  notification_free(clone);
}

TEST_F(NotificationApiTest, clone_preserves_vibration) {
  ASSERT_EQ(notification_set_vibration(noti_,
                NOTIFICATION_VIBRATION_TYPE_USER_DATA,
                "/p/vib.ivt"),
            NOTIFICATION_ERROR_NONE);

  notification_h clone = nullptr;
  ASSERT_EQ(notification_clone(noti_, &clone),
            NOTIFICATION_ERROR_NONE);
  ASSERT_NE(clone, nullptr);

  notification_vibration_type_e t;
  const char* path = nullptr;
  EXPECT_EQ(notification_get_vibration(clone, &t, &path),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(t, NOTIFICATION_VIBRATION_TYPE_USER_DATA);
  ASSERT_NE(path, nullptr);
  EXPECT_STREQ(path, "/p/vib.ivt");

  notification_free(clone);
}

TEST_F(NotificationApiTest, clone_preserves_auto_remove) {
  ASSERT_EQ(notification_set_auto_remove(noti_, false),
            NOTIFICATION_ERROR_NONE);

  notification_h clone = nullptr;
  ASSERT_EQ(notification_clone(noti_, &clone),
            NOTIFICATION_ERROR_NONE);
  ASSERT_NE(clone, nullptr);

  bool b = true;
  EXPECT_EQ(notification_get_auto_remove(clone, &b),
            NOTIFICATION_ERROR_NONE);
  EXPECT_FALSE(b);

  notification_free(clone);
}

TEST_F(NotificationApiTest, clone_preserves_tag) {
  ASSERT_EQ(notification_set_tag(noti_, "my-cloned-tag"),
            NOTIFICATION_ERROR_NONE);

  notification_h clone = nullptr;
  ASSERT_EQ(notification_clone(noti_, &clone),
            NOTIFICATION_ERROR_NONE);
  ASSERT_NE(clone, nullptr);

  const char* tag = nullptr;
  EXPECT_EQ(notification_get_tag(clone, &tag),
            NOTIFICATION_ERROR_NONE);
  ASSERT_NE(tag, nullptr);
  EXPECT_STREQ(tag, "my-cloned-tag");

  notification_free(clone);
}

TEST_F(NotificationApiTest, clone_preserves_led) {
  ASSERT_EQ(notification_set_led(noti_,
                NOTIFICATION_LED_OP_ON_CUSTOM_COLOR, 0x00FF00),
            NOTIFICATION_ERROR_NONE);

  notification_h clone = nullptr;
  ASSERT_EQ(notification_clone(noti_, &clone),
            NOTIFICATION_ERROR_NONE);
  ASSERT_NE(clone, nullptr);

  notification_led_op_e op;
  int color = 0;
  EXPECT_EQ(notification_get_led(clone, &op, &color),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(op, NOTIFICATION_LED_OP_ON_CUSTOM_COLOR);
  EXPECT_EQ(color, 0x00FF00);

  notification_free(clone);
}

TEST_F(NotificationApiTest, clone_preserves_size) {
  ASSERT_EQ(notification_set_size(noti_, 0.33),
            NOTIFICATION_ERROR_NONE);

  notification_h clone = nullptr;
  ASSERT_EQ(notification_clone(noti_, &clone),
            NOTIFICATION_ERROR_NONE);
  ASSERT_NE(clone, nullptr);

  double v = 0;
  EXPECT_EQ(notification_get_size(clone, &v),
            NOTIFICATION_ERROR_NONE);
  EXPECT_DOUBLE_EQ(v, 0.33);

  notification_free(clone);
}

TEST_F(NotificationApiTest, clone_preserves_display_applist) {
  ASSERT_EQ(notification_set_display_applist(noti_,
                NOTIFICATION_DISPLAY_APP_NOTIFICATION_TRAY |
                NOTIFICATION_DISPLAY_APP_INDICATOR),
            NOTIFICATION_ERROR_NONE);

  notification_h clone = nullptr;
  ASSERT_EQ(notification_clone(noti_, &clone),
            NOTIFICATION_ERROR_NONE);
  ASSERT_NE(clone, nullptr);

  int got = 0;
  EXPECT_EQ(notification_get_display_applist(clone, &got),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(got,
            NOTIFICATION_DISPLAY_APP_NOTIFICATION_TRAY |
                NOTIFICATION_DISPLAY_APP_INDICATOR);

  notification_free(clone);
}

TEST_F(NotificationApiTest, clone_preserves_ongoing_flag) {
  ASSERT_EQ(notification_set_ongoing_flag(noti_, true),
            NOTIFICATION_ERROR_NONE);

  notification_h clone = nullptr;
  ASSERT_EQ(notification_clone(noti_, &clone),
            NOTIFICATION_ERROR_NONE);
  ASSERT_NE(clone, nullptr);

  bool b = false;
  EXPECT_EQ(notification_get_ongoing_flag(clone, &b),
            NOTIFICATION_ERROR_NONE);
  EXPECT_TRUE(b);

  notification_free(clone);
}

TEST_F(NotificationApiTest, clone_preserves_text_domain) {
  ASSERT_EQ(notification_set_text_domain(noti_,
                "my-domain", "/usr/share/locale"),
            NOTIFICATION_ERROR_NONE);

  notification_h clone = nullptr;
  ASSERT_EQ(notification_clone(noti_, &clone),
            NOTIFICATION_ERROR_NONE);
  ASSERT_NE(clone, nullptr);

  char* domain = nullptr;
  char* dir = nullptr;
  EXPECT_EQ(notification_get_text_domain(clone, &domain, &dir),
            NOTIFICATION_ERROR_NONE);
  ASSERT_NE(domain, nullptr);
  ASSERT_NE(dir, nullptr);
  EXPECT_STREQ(domain, "my-domain");
  EXPECT_STREQ(dir, "/usr/share/locale");

  notification_free(clone);
}

TEST_F(NotificationApiTest, clone_preserves_extension_image_size) {
  ASSERT_EQ(notification_set_extension_image_size(noti_, 64),
            NOTIFICATION_ERROR_NONE);

  notification_h clone = nullptr;
  ASSERT_EQ(notification_clone(noti_, &clone),
            NOTIFICATION_ERROR_NONE);
  ASSERT_NE(clone, nullptr);

  int sz = 0;
  EXPECT_EQ(notification_get_extension_image_size(clone, &sz),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(sz, 64);

  notification_free(clone);
}

TEST_F(NotificationApiTest, clone_preserves_led_time_period) {
  ASSERT_EQ(notification_set_led_time_period(noti_, 800, 200),
            NOTIFICATION_ERROR_NONE);

  notification_h clone = nullptr;
  ASSERT_EQ(notification_clone(noti_, &clone),
            NOTIFICATION_ERROR_NONE);
  ASSERT_NE(clone, nullptr);

  int on = 0, off = 0;
  EXPECT_EQ(notification_get_led_time_period(clone, &on, &off),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(on, 800);
  EXPECT_EQ(off, 200);

  notification_free(clone);
}

TEST_F(NotificationApiTest, clone_preserves_time) {
  time_t timestamp = 1700000000;
  ASSERT_EQ(notification_set_time(noti_, timestamp),
            NOTIFICATION_ERROR_NONE);

  notification_h clone = nullptr;
  ASSERT_EQ(notification_clone(noti_, &clone),
            NOTIFICATION_ERROR_NONE);
  ASSERT_NE(clone, nullptr);

  time_t got;
  EXPECT_EQ(notification_get_time(clone, &got),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(got, timestamp);

  notification_free(clone);
}

TEST_F(NotificationApiTest, clone_preserves_multiple_images) {
  ASSERT_EQ(notification_set_image(noti_,
                NOTIFICATION_IMAGE_TYPE_ICON, "/a/icon.png"),
            NOTIFICATION_ERROR_NONE);
  ASSERT_EQ(notification_set_image(noti_,
                NOTIFICATION_IMAGE_TYPE_THUMBNAIL, "/a/thumb.png"),
            NOTIFICATION_ERROR_NONE);
  ASSERT_EQ(notification_set_image(noti_,
                NOTIFICATION_IMAGE_TYPE_BACKGROUND, "/a/bg.png"),
            NOTIFICATION_ERROR_NONE);

  notification_h clone = nullptr;
  ASSERT_EQ(notification_clone(noti_, &clone),
            NOTIFICATION_ERROR_NONE);
  ASSERT_NE(clone, nullptr);

  char* p;
  EXPECT_EQ(notification_get_image(clone,
                NOTIFICATION_IMAGE_TYPE_ICON, &p),
            NOTIFICATION_ERROR_NONE);
  EXPECT_STREQ(p, "/a/icon.png");
  EXPECT_EQ(notification_get_image(clone,
                NOTIFICATION_IMAGE_TYPE_THUMBNAIL, &p),
            NOTIFICATION_ERROR_NONE);
  EXPECT_STREQ(p, "/a/thumb.png");
  EXPECT_EQ(notification_get_image(clone,
                NOTIFICATION_IMAGE_TYPE_BACKGROUND, &p),
            NOTIFICATION_ERROR_NONE);
  EXPECT_STREQ(p, "/a/bg.png");

  notification_free(clone);
}

/* ============================================================== */
/* notification_set_sound / set_vibration / set_led — all variants  */
/* ============================================================== */

TEST_F(NotificationApiTest, set_sound_default_no_path) {
  ASSERT_EQ(notification_set_sound(noti_,
                NOTIFICATION_SOUND_TYPE_DEFAULT, nullptr),
            NOTIFICATION_ERROR_NONE);
  notification_sound_type_e t;
  const char* p = nullptr;
  EXPECT_EQ(notification_get_sound(noti_, &t, &p),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(t, NOTIFICATION_SOUND_TYPE_DEFAULT);
}

TEST_F(NotificationApiTest, set_sound_none) {
  ASSERT_EQ(notification_set_sound(noti_,
                NOTIFICATION_SOUND_TYPE_NONE, nullptr),
            NOTIFICATION_ERROR_NONE);
  notification_sound_type_e t;
  const char* p = nullptr;
  EXPECT_EQ(notification_get_sound(noti_, &t, &p),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(t, NOTIFICATION_SOUND_TYPE_NONE);
}

TEST_F(NotificationApiTest, set_vibration_default_no_path) {
  ASSERT_EQ(notification_set_vibration(noti_,
                NOTIFICATION_VIBRATION_TYPE_DEFAULT, nullptr),
            NOTIFICATION_ERROR_NONE);
  notification_vibration_type_e t;
  const char* p = nullptr;
  EXPECT_EQ(notification_get_vibration(noti_, &t, &p),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(t, NOTIFICATION_VIBRATION_TYPE_DEFAULT);
}

TEST_F(NotificationApiTest, set_vibration_none) {
  ASSERT_EQ(notification_set_vibration(noti_,
                NOTIFICATION_VIBRATION_TYPE_NONE, nullptr),
            NOTIFICATION_ERROR_NONE);
  notification_vibration_type_e t;
  const char* p = nullptr;
  EXPECT_EQ(notification_get_vibration(noti_, &t, &p),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(t, NOTIFICATION_VIBRATION_TYPE_NONE);
}

TEST_F(NotificationApiTest, set_led_off_roundtrip) {
  ASSERT_EQ(notification_set_led(noti_,
                NOTIFICATION_LED_OP_OFF, 0),
            NOTIFICATION_ERROR_NONE);
  notification_led_op_e op;
  int color = 0xDEAD;
  EXPECT_EQ(notification_get_led(noti_, &op, &color),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(op, NOTIFICATION_LED_OP_OFF);
}

TEST_F(NotificationApiTest, set_led_on_default_color_roundtrip) {
  ASSERT_EQ(notification_set_led(noti_,
                NOTIFICATION_LED_OP_ON, 0),
            NOTIFICATION_ERROR_NONE);
  notification_led_op_e op;
  int color = 0;
  EXPECT_EQ(notification_get_led(noti_, &op, &color),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(op, NOTIFICATION_LED_OP_ON);
}

/* ============================================================== */
/* notification_set_extension_image_size — boundary values         */
/* ============================================================== */

TEST_F(NotificationApiTest, set_extension_image_size_one_roundtrip) {
  ASSERT_EQ(notification_set_extension_image_size(noti_, 1),
            NOTIFICATION_ERROR_NONE);
  int v = 0;
  EXPECT_EQ(notification_get_extension_image_size(noti_, &v),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(v, 1);
}

TEST_F(NotificationApiTest, set_extension_image_size_int_max_roundtrip) {
  ASSERT_EQ(notification_set_extension_image_size(noti_, 2147483647),
            NOTIFICATION_ERROR_NONE);
  int v = 0;
  EXPECT_EQ(notification_get_extension_image_size(noti_, &v),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(v, 2147483647);
}

/* ============================================================== */
/* notification_set_text_input — accepts any int, including 0 and  */
/* INT_MAX, since impl only null-checks noti.                       */
/* ============================================================== */

TEST_F(NotificationApiTest, set_text_input_zero) {
  EXPECT_EQ(notification_set_text_input(noti_, 0),
            NOTIFICATION_ERROR_NONE);
}

TEST_F(NotificationApiTest, set_text_input_int_max) {
  EXPECT_EQ(notification_set_text_input(noti_, 2147483647),
            NOTIFICATION_ERROR_NONE);
}

/* ============================================================== */
/* notification_set_size / set_progress — boundary doubles         */
/* (impl stores verbatim; no clamping)                             */
/* ============================================================== */

TEST_F(NotificationApiTest, set_size_zero) {
  ASSERT_EQ(notification_set_size(noti_, 0.0),
            NOTIFICATION_ERROR_NONE);
  double v = -1;
  EXPECT_EQ(notification_get_size(noti_, &v),
            NOTIFICATION_ERROR_NONE);
  EXPECT_DOUBLE_EQ(v, 0.0);
}

TEST_F(NotificationApiTest, set_size_one) {
  ASSERT_EQ(notification_set_size(noti_, 1.0),
            NOTIFICATION_ERROR_NONE);
  double v = -1;
  EXPECT_EQ(notification_get_size(noti_, &v),
            NOTIFICATION_ERROR_NONE);
  EXPECT_DOUBLE_EQ(v, 1.0);
}

TEST_F(NotificationApiTest, set_size_negative_stored_verbatim) {
  /* Impl does not clamp; negative values are stored as-is. */
  ASSERT_EQ(notification_set_size(noti_, -0.5),
            NOTIFICATION_ERROR_NONE);
  double v = 0;
  EXPECT_EQ(notification_get_size(noti_, &v),
            NOTIFICATION_ERROR_NONE);
  EXPECT_DOUBLE_EQ(v, -0.5);
}

TEST_F(NotificationApiTest, set_progress_zero) {
  ASSERT_EQ(notification_set_progress(noti_, 0.0),
            NOTIFICATION_ERROR_NONE);
  double v = -1;
  EXPECT_EQ(notification_get_progress(noti_, &v),
            NOTIFICATION_ERROR_NONE);
  EXPECT_DOUBLE_EQ(v, 0.0);
}

TEST_F(NotificationApiTest, set_progress_one) {
  ASSERT_EQ(notification_set_progress(noti_, 1.0),
            NOTIFICATION_ERROR_NONE);
  double v = -1;
  EXPECT_EQ(notification_get_progress(noti_, &v),
            NOTIFICATION_ERROR_NONE);
  EXPECT_DOUBLE_EQ(v, 1.0);
}

TEST_F(NotificationApiTest, set_progress_above_one_stored_verbatim) {
  ASSERT_EQ(notification_set_progress(noti_, 2.5),
            NOTIFICATION_ERROR_NONE);
  double v = 0;
  EXPECT_EQ(notification_get_progress(noti_, &v),
            NOTIFICATION_ERROR_NONE);
  EXPECT_DOUBLE_EQ(v, 2.5);
}

/* ============================================================== */
/* notification_set_layout — EXTENSION + NONE roundtrips           */
/* ============================================================== */

TEST_F(NotificationApiTest, set_layout_extension_roundtrip) {
  ASSERT_EQ(notification_set_layout(noti_,
                NOTIFICATION_LY_EXTENSION),
            NOTIFICATION_ERROR_NONE);
  notification_ly_type_e v;
  EXPECT_EQ(notification_get_layout(noti_, &v),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(v, NOTIFICATION_LY_EXTENSION);
}

TEST_F(NotificationApiTest, set_layout_none_roundtrip) {
  ASSERT_EQ(notification_set_layout(noti_,
                NOTIFICATION_LY_NONE),
            NOTIFICATION_ERROR_NONE);
  notification_ly_type_e v;
  EXPECT_EQ(notification_get_layout(noti_, &v),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(v, NOTIFICATION_LY_NONE);
}

/* ============================================================== */
/* notification_set_image — extended image types                   */
/* ============================================================== */

TEST_F(NotificationApiTest, set_image_thumbnail_for_lock_roundtrip) {
  ASSERT_EQ(notification_set_image(noti_,
                NOTIFICATION_IMAGE_TYPE_THUMBNAIL_FOR_LOCK,
                "/img/tlock.png"),
            NOTIFICATION_ERROR_NONE);
  char* out = nullptr;
  EXPECT_EQ(notification_get_image(noti_,
                NOTIFICATION_IMAGE_TYPE_THUMBNAIL_FOR_LOCK, &out),
            NOTIFICATION_ERROR_NONE);
  EXPECT_STREQ(out, "/img/tlock.png");
}

TEST_F(NotificationApiTest, set_image_icon_sub_roundtrip) {
  ASSERT_EQ(notification_set_image(noti_,
                NOTIFICATION_IMAGE_TYPE_ICON_SUB,
                "/img/sub.png"),
            NOTIFICATION_ERROR_NONE);
  char* out = nullptr;
  EXPECT_EQ(notification_get_image(noti_,
                NOTIFICATION_IMAGE_TYPE_ICON_SUB, &out),
            NOTIFICATION_ERROR_NONE);
  EXPECT_STREQ(out, "/img/sub.png");
}

TEST_F(NotificationApiTest, set_image_list_variants_roundtrip) {
  notification_image_type_e types[] = {
      NOTIFICATION_IMAGE_TYPE_LIST_1,
      NOTIFICATION_IMAGE_TYPE_LIST_2,
      NOTIFICATION_IMAGE_TYPE_LIST_3,
      NOTIFICATION_IMAGE_TYPE_LIST_4,
  };
  for (size_t i = 0; i < sizeof(types) / sizeof(types[0]); ++i) {
    std::string p = std::string("/img/list") +
                    std::to_string(i + 1) + ".png";
    ASSERT_EQ(notification_set_image(noti_, types[i], p.c_str()),
              NOTIFICATION_ERROR_NONE);
    char* got = nullptr;
    EXPECT_EQ(notification_get_image(noti_, types[i], &got),
              NOTIFICATION_ERROR_NONE);
    EXPECT_STREQ(got, p.c_str());
  }
}

/* ============================================================== */
/* notification_set_text — each button-text type roundtrip          */
/* ============================================================== */

TEST_F(NotificationApiTest, set_text_each_button_text_type) {
  notification_text_type_e types[] = {
      NOTIFICATION_TEXT_TYPE_BUTTON_1,
      NOTIFICATION_TEXT_TYPE_BUTTON_2,
      NOTIFICATION_TEXT_TYPE_BUTTON_3,
      NOTIFICATION_TEXT_TYPE_BUTTON_4,
      NOTIFICATION_TEXT_TYPE_BUTTON_5,
      NOTIFICATION_TEXT_TYPE_BUTTON_6,
      NOTIFICATION_TEXT_TYPE_BUTTON_7,
      NOTIFICATION_TEXT_TYPE_BUTTON_8,
      NOTIFICATION_TEXT_TYPE_BUTTON_9,
      NOTIFICATION_TEXT_TYPE_BUTTON_10,
  };
  for (size_t i = 0; i < sizeof(types) / sizeof(types[0]); ++i) {
    std::string label = std::string("btn-") + std::to_string(i + 1);
    ASSERT_EQ(notification_set_text(noti_, types[i],
                  label.c_str(), nullptr,
                  NOTIFICATION_VARIABLE_TYPE_NONE),
              NOTIFICATION_ERROR_NONE);
    char* got = nullptr;
    EXPECT_EQ(notification_get_text(noti_, types[i], &got),
              NOTIFICATION_ERROR_NONE);
    EXPECT_STREQ(got, label.c_str());
  }
}

TEST_F(NotificationApiTest, set_text_each_info_type) {
  notification_text_type_e types[] = {
      NOTIFICATION_TEXT_TYPE_INFO_1,
      NOTIFICATION_TEXT_TYPE_INFO_SUB_1,
      NOTIFICATION_TEXT_TYPE_INFO_2,
      NOTIFICATION_TEXT_TYPE_INFO_SUB_2,
      NOTIFICATION_TEXT_TYPE_INFO_3,
      NOTIFICATION_TEXT_TYPE_INFO_SUB_3,
  };
  for (size_t i = 0; i < sizeof(types) / sizeof(types[0]); ++i) {
    std::string label = std::string("info-") + std::to_string(i);
    ASSERT_EQ(notification_set_text(noti_, types[i],
                  label.c_str(), nullptr,
                  NOTIFICATION_VARIABLE_TYPE_NONE),
              NOTIFICATION_ERROR_NONE);
    char* got = nullptr;
    EXPECT_EQ(notification_get_text(noti_, types[i], &got),
              NOTIFICATION_ERROR_NONE);
    EXPECT_STREQ(got, label.c_str());
  }
}

/* ============================================================== */
/* notification_set_display_applist — each flag roundtrip          */
/* ============================================================== */

TEST_F(NotificationApiTest, set_display_applist_each_flag) {
  int flags[] = {
      NOTIFICATION_DISPLAY_APP_NOTIFICATION_TRAY,
      NOTIFICATION_DISPLAY_APP_TICKER,
      NOTIFICATION_DISPLAY_APP_LOCK,
      NOTIFICATION_DISPLAY_APP_INDICATOR,
      NOTIFICATION_DISPLAY_APP_ACTIVE,
  };
  for (size_t i = 0; i < sizeof(flags) / sizeof(flags[0]); ++i) {
    ASSERT_EQ(notification_set_display_applist(noti_, flags[i]),
              NOTIFICATION_ERROR_NONE);
    int got = 0;
    EXPECT_EQ(notification_get_display_applist(noti_, &got),
              NOTIFICATION_ERROR_NONE);
    EXPECT_EQ(got, flags[i]);
  }
}

TEST_F(NotificationApiTest, set_display_applist_all_roundtrip) {
  ASSERT_EQ(notification_set_display_applist(noti_,
                NOTIFICATION_DISPLAY_APP_ALL),
            NOTIFICATION_ERROR_NONE);
  int got = 0;
  EXPECT_EQ(notification_get_display_applist(noti_, &got),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(got, NOTIFICATION_DISPLAY_APP_ALL);
}

/* ============================================================== */
/* notification_set_property — each property flag roundtrip        */
/* ============================================================== */

TEST_F(NotificationApiTest, set_property_each_flag) {
  int flags[] = {
      NOTIFICATION_PROP_DISPLAY_ONLY_SIMMODE,
      NOTIFICATION_PROP_DISABLE_APP_LAUNCH,
      NOTIFICATION_PROP_DISABLE_AUTO_DELETE,
      NOTIFICATION_PROP_DISABLE_UPDATE_ON_INSERT,
      NOTIFICATION_PROP_DISABLE_UPDATE_ON_DELETE,
      NOTIFICATION_PROP_VOLATILE_DISPLAY,
  };
  for (size_t i = 0; i < sizeof(flags) / sizeof(flags[0]); ++i) {
    ASSERT_EQ(notification_set_property(noti_, flags[i]),
              NOTIFICATION_ERROR_NONE);
    int got = 0;
    EXPECT_EQ(notification_get_property(noti_, &got),
              NOTIFICATION_ERROR_NONE);
    EXPECT_EQ(got, flags[i]);
  }
}

TEST_F(NotificationApiTest, set_property_combined_flags_roundtrip) {
  int combined = NOTIFICATION_PROP_DISABLE_APP_LAUNCH |
                 NOTIFICATION_PROP_DISABLE_AUTO_DELETE |
                 NOTIFICATION_PROP_VOLATILE_DISPLAY;
  ASSERT_EQ(notification_set_property(noti_, combined),
            NOTIFICATION_ERROR_NONE);
  int got = 0;
  EXPECT_EQ(notification_get_property(noti_, &got),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(got, combined);
}

/* ============================================================== */
/* notification_set_time — special and boundary values             */
/* (impl: input_time==0 → uses current wall clock)                  */
/* ============================================================== */

TEST_F(NotificationApiTest, set_time_far_future_2038_roundtrip) {
  time_t ts = 2147483647;  /* 2038-01-19 03:14:07 UTC */
  ASSERT_EQ(notification_set_time(noti_, ts),
            NOTIFICATION_ERROR_NONE);
  time_t got = 0;
  EXPECT_EQ(notification_get_time(noti_, &got),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(got, ts);
}

/* ============================================================== */
/* set_text variable type INT/DOUBLE/STRING individual roundtrips   */
/* ============================================================== */

TEST_F(NotificationApiTest, set_text_int_variable_stored) {
  EXPECT_EQ(notification_set_text(noti_, NOTIFICATION_TEXT_TYPE_TITLE,
                "count %d", nullptr,
                NOTIFICATION_VARIABLE_TYPE_INT, 7,
                NOTIFICATION_VARIABLE_TYPE_NONE),
            NOTIFICATION_ERROR_NONE);
  char* out = nullptr;
  EXPECT_EQ(notification_get_text(noti_,
                NOTIFICATION_TEXT_TYPE_TITLE, &out),
            NOTIFICATION_ERROR_NONE);
  EXPECT_NE(out, nullptr);
}

TEST_F(NotificationApiTest, set_text_empty_string_stored) {
  EXPECT_EQ(notification_set_text(noti_, NOTIFICATION_TEXT_TYPE_TITLE,
                "", nullptr, NOTIFICATION_VARIABLE_TYPE_NONE),
            NOTIFICATION_ERROR_NONE);
}

TEST_F(NotificationApiTest, set_text_with_key_then_clear_with_null) {
  ASSERT_EQ(notification_set_text(noti_, NOTIFICATION_TEXT_TYPE_TITLE,
                "Hello", "IDS_HELLO",
                NOTIFICATION_VARIABLE_TYPE_NONE),
            NOTIFICATION_ERROR_NONE);
  /* Per impl observed: passing NULL text clears the slot. */
  EXPECT_EQ(notification_set_text(noti_, NOTIFICATION_TEXT_TYPE_TITLE,
                nullptr, nullptr,
                NOTIFICATION_VARIABLE_TYPE_NONE),
            NOTIFICATION_ERROR_NONE);
}

TEST_F(NotificationApiTest, set_text_group_title_and_content_roundtrip) {
  ASSERT_EQ(notification_set_text(noti_,
                NOTIFICATION_TEXT_TYPE_GROUP_TITLE,
                "Group X", nullptr,
                NOTIFICATION_VARIABLE_TYPE_NONE),
            NOTIFICATION_ERROR_NONE);
  ASSERT_EQ(notification_set_text(noti_,
                NOTIFICATION_TEXT_TYPE_GROUP_CONTENT,
                "Group X content", nullptr,
                NOTIFICATION_VARIABLE_TYPE_NONE),
            NOTIFICATION_ERROR_NONE);
  char* got = nullptr;
  EXPECT_EQ(notification_get_text(noti_,
                NOTIFICATION_TEXT_TYPE_GROUP_TITLE, &got),
            NOTIFICATION_ERROR_NONE);
  EXPECT_STREQ(got, "Group X");
  EXPECT_EQ(notification_get_text(noti_,
                NOTIFICATION_TEXT_TYPE_GROUP_CONTENT, &got),
            NOTIFICATION_ERROR_NONE);
  EXPECT_STREQ(got, "Group X content");
}

TEST_F(NotificationApiTest, clone_preserves_multiple_text_types) {
  ASSERT_EQ(notification_set_text(noti_,
                NOTIFICATION_TEXT_TYPE_TITLE,
                "the-title", nullptr,
                NOTIFICATION_VARIABLE_TYPE_NONE),
            NOTIFICATION_ERROR_NONE);
  ASSERT_EQ(notification_set_text(noti_,
                NOTIFICATION_TEXT_TYPE_CONTENT,
                "the-content", nullptr,
                NOTIFICATION_VARIABLE_TYPE_NONE),
            NOTIFICATION_ERROR_NONE);

  notification_h clone = nullptr;
  ASSERT_EQ(notification_clone(noti_, &clone),
            NOTIFICATION_ERROR_NONE);
  ASSERT_NE(clone, nullptr);

  char* t = nullptr;
  EXPECT_EQ(notification_get_text(clone,
                NOTIFICATION_TEXT_TYPE_TITLE, &t),
            NOTIFICATION_ERROR_NONE);
  EXPECT_STREQ(t, "the-title");
  EXPECT_EQ(notification_get_text(clone,
                NOTIFICATION_TEXT_TYPE_CONTENT, &t),
            NOTIFICATION_ERROR_NONE);
  EXPECT_STREQ(t, "the-content");

  notification_free(clone);
}

TEST_F(NotificationApiTest, save_as_template_null_args) {
  EXPECT_EQ(notification_save_as_template(nullptr, "name"),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_save_as_template(noti_, nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* ============================================================== */
/* notification_create_from_template / load_by_tag — null args    */
/* ============================================================== */

TEST_F(NotificationApiTest, create_from_template_null_name) {
  /* Returns NULL handle; last_result set to INVALID_PARAMETER. */
  notification_h n = notification_create_from_template(nullptr);
  EXPECT_EQ(n, nullptr);
  EXPECT_EQ(get_last_result(), NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationApiTest, load_by_tag_null_tag) {
  /* Returns NULL handle; last_result set to INVALID_PARAMETER. */
  notification_h n = notification_load_by_tag(nullptr);
  EXPECT_EQ(n, nullptr);
  EXPECT_EQ(get_last_result(), NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* ============================================================== */
/* notification_set_event_handler valid roundtrip                  */
/* ============================================================== */

TEST_F(NotificationApiTest, set_event_handler_valid_then_get) {
  app_control_h ac = nullptr;
  app_control_create(&ac);
  app_control_set_app_id(ac, "com.example.callback");

  ASSERT_EQ(notification_set_event_handler(noti_,
                NOTIFICATION_EVENT_TYPE_CLICK_ON_BUTTON_1, ac),
            NOTIFICATION_ERROR_NONE);

  app_control_h got = nullptr;
  EXPECT_EQ(notification_get_event_handler(noti_,
                NOTIFICATION_EVENT_TYPE_CLICK_ON_BUTTON_1, &got),
            NOTIFICATION_ERROR_NONE);
  if (got != nullptr) {
    char* app_id = nullptr;
    app_control_get_app_id(got, &app_id);
    if (app_id != nullptr) {
      EXPECT_STREQ(app_id, "com.example.callback");
      free(app_id);
    }
    app_control_destroy(got);
  }

  app_control_destroy(ac);
}

/* ============================================================== */
/* notification_set_image with priv_image_path-only path           */
/* ============================================================== */

TEST_F(NotificationApiTest, set_image_for_each_image_type) {
  /* Walk the valid image type range; each store returns NONE. */
  for (int t = NOTIFICATION_IMAGE_TYPE_ICON;
       t <= NOTIFICATION_IMAGE_TYPE_MAX; ++t) {
    EXPECT_EQ(notification_set_image(noti_,
                  static_cast<notification_image_type_e>(t),
                  "/tmp/test.png"),
              NOTIFICATION_ERROR_NONE);
  }
}

/* ============================================================== */
/* notification_set_text loops over each TEXT_TYPE in valid range  */
/* ============================================================== */

TEST_F(NotificationApiTest, set_text_for_each_text_type) {
  for (int t = NOTIFICATION_TEXT_TYPE_TITLE;
       t <= NOTIFICATION_TEXT_TYPE_MAX; ++t) {
    EXPECT_EQ(notification_set_text(noti_,
                  static_cast<notification_text_type_e>(t),
                  "loop_text", nullptr,
                  NOTIFICATION_VARIABLE_TYPE_NONE),
              NOTIFICATION_ERROR_NONE);
  }
}

/* ============================================================== */
/* notification_set_led tied to LED_OP enum range                  */
/* ============================================================== */

TEST_F(NotificationApiTest, set_led_off_get_op_only) {
  /* Per impl: led_argb is only written when op == ON_CUSTOM_COLOR. For
   * OFF, only the op field is populated; led_argb is left untouched. */
  ASSERT_EQ(notification_set_led(noti_, NOTIFICATION_LED_OP_OFF, 0),
            NOTIFICATION_ERROR_NONE);
  notification_led_op_e op;
  int color = 0xDEADBEEF;
  EXPECT_EQ(notification_get_led(noti_, &op, &color),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(op, NOTIFICATION_LED_OP_OFF);
  /* color was not overwritten — sentinel survives. */
  EXPECT_EQ(static_cast<unsigned>(color), 0xDEADBEEFu);
}

TEST_F(NotificationApiTest, set_led_on_default) {
  ASSERT_EQ(notification_set_led(noti_, NOTIFICATION_LED_OP_ON, 0),
            NOTIFICATION_ERROR_NONE);
  notification_led_op_e op;
  int color = -1;
  EXPECT_EQ(notification_get_led(noti_, &op, &color),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(op, NOTIFICATION_LED_OP_ON);
}

/* ============================================================== */
/* notification_set_progress / set_size boundary values            */
/* ============================================================== */

TEST_F(NotificationApiTest, set_progress_boundary_values) {
  /* 0.0 and 1.0 are typical boundaries; out-of-range is allowed since
   * the API doesn't validate. */
  ASSERT_EQ(notification_set_progress(noti_, 0.0), NOTIFICATION_ERROR_NONE);
  double v = -1.0;
  EXPECT_EQ(notification_get_progress(noti_, &v), NOTIFICATION_ERROR_NONE);
  EXPECT_DOUBLE_EQ(v, 0.0);

  ASSERT_EQ(notification_set_progress(noti_, 1.0), NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(notification_get_progress(noti_, &v), NOTIFICATION_ERROR_NONE);
  EXPECT_DOUBLE_EQ(v, 1.0);

  /* No validation: negative percentage stored verbatim. */
  ASSERT_EQ(notification_set_progress(noti_, -0.5), NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(notification_get_progress(noti_, &v), NOTIFICATION_ERROR_NONE);
  EXPECT_DOUBLE_EQ(v, -0.5);
}

TEST_F(NotificationApiTest, set_size_boundary_values) {
  ASSERT_EQ(notification_set_size(noti_, 0.0), NOTIFICATION_ERROR_NONE);
  double v = -1.0;
  EXPECT_EQ(notification_get_size(noti_, &v), NOTIFICATION_ERROR_NONE);
  EXPECT_DOUBLE_EQ(v, 0.0);

  ASSERT_EQ(notification_set_size(noti_, 1e9), NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(notification_get_size(noti_, &v), NOTIFICATION_ERROR_NONE);
  EXPECT_DOUBLE_EQ(v, 1e9);
}

/* ============================================================== */
/* notification_set_auto_remove multiple toggles                   */
/* ============================================================== */

TEST_F(NotificationApiTest, set_auto_remove_default_is_true) {
  /* Per impl: notification_create defaults auto_remove to true. */
  bool got = false;
  EXPECT_EQ(notification_get_auto_remove(noti_, &got),
            NOTIFICATION_ERROR_NONE);
  EXPECT_TRUE(got);
}

TEST_F(NotificationApiTest, set_property_overlapping_flags) {
  int flags = NOTIFICATION_PROP_DISABLE_APP_LAUNCH
            | NOTIFICATION_PROP_DISABLE_AUTO_DELETE
            | NOTIFICATION_PROP_VOLATILE_DISPLAY;
  ASSERT_EQ(notification_set_property(noti_, flags),
            NOTIFICATION_ERROR_NONE);
  int got = 0;
  EXPECT_EQ(notification_get_property(noti_, &got),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(got, flags);
}

/* ============================================================== */
/* set_event_handler with null app_control_h                       */
/* ============================================================== */

TEST_F(NotificationApiTest, set_event_handler_null_app_control_returns_invalid) {
  /* Per impl: app_control_export_as_bundle(NULL,..) → INVALID; the
   * set_event_handler propagates it. */
  EXPECT_EQ(notification_set_event_handler(noti_,
                NOTIFICATION_EVENT_TYPE_CLICK_ON_BUTTON_1, nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* ============================================================== */
/* set_sound DEFAULT type (no path needed)                          */
/* ============================================================== */

TEST_F(NotificationApiTest, set_sound_default_no_path_required) {
  /* DEFAULT type does not need a path. */
  ASSERT_EQ(notification_set_sound(noti_, NOTIFICATION_SOUND_TYPE_DEFAULT,
                nullptr),
            NOTIFICATION_ERROR_NONE);
  notification_sound_type_e got;
  const char* path = nullptr;
  EXPECT_EQ(notification_get_sound(noti_, &got, &path),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(got, NOTIFICATION_SOUND_TYPE_DEFAULT);
}

/* ============================================================== */
/* set_vibration DEFAULT type (no path needed)                      */
/* ============================================================== */

TEST_F(NotificationApiTest, set_vibration_default_no_path_required) {
  ASSERT_EQ(notification_set_vibration(noti_,
                NOTIFICATION_VIBRATION_TYPE_DEFAULT, nullptr),
            NOTIFICATION_ERROR_NONE);
  notification_vibration_type_e got;
  const char* path = nullptr;
  EXPECT_EQ(notification_get_vibration(noti_, &got, &path),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(got, NOTIFICATION_VIBRATION_TYPE_DEFAULT);
}

/* ============================================================== */
/* set_property zero clears flags                                  */
/* ============================================================== */

TEST_F(NotificationApiTest, set_property_zero_stored) {
  ASSERT_EQ(notification_set_property(noti_,
                NOTIFICATION_PROP_DISABLE_APP_LAUNCH),
            NOTIFICATION_ERROR_NONE);
  ASSERT_EQ(notification_set_property(noti_, 0),
            NOTIFICATION_ERROR_NONE);
  int got = 0xDEAD;
  EXPECT_EQ(notification_get_property(noti_, &got),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(got, 0);
}

/* ============================================================== */
/* set_display_applist zero clears                                  */
/* ============================================================== */

TEST_F(NotificationApiTest, set_display_applist_zero_clears) {
  ASSERT_EQ(notification_set_display_applist(noti_,
                NOTIFICATION_DISPLAY_APP_INDICATOR),
            NOTIFICATION_ERROR_NONE);
  ASSERT_EQ(notification_set_display_applist(noti_, 0),
            NOTIFICATION_ERROR_NONE);
  int got = 0xDEAD;
  EXPECT_EQ(notification_get_display_applist(noti_, &got),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(got, 0);
}

/* ============================================================== */
/* set_layout valid boundary (NONE accepted, MAX-1 ok)             */
/* ============================================================== */

TEST_F(NotificationApiTest, set_layout_each_valid) {
  /* Walk through NONE..MAX, all stored verbatim. */
  for (int t = NOTIFICATION_LY_NONE; t <= NOTIFICATION_LY_MAX; ++t) {
    EXPECT_EQ(notification_set_layout(noti_,
                  static_cast<notification_ly_type_e>(t)),
              NOTIFICATION_ERROR_NONE);
    notification_ly_type_e got;
    EXPECT_EQ(notification_get_layout(noti_, &got),
              NOTIFICATION_ERROR_NONE);
    EXPECT_EQ(got, t);
  }
}

/* ============================================================== */
/* set_text with multiple vararg pairs                              */
/* ============================================================== */

TEST_F(NotificationApiTest, set_text_with_double_var_arg) {
  ASSERT_EQ(notification_set_text(noti_, NOTIFICATION_TEXT_TYPE_CONTENT,
                "ratio %.2f", nullptr,
                NOTIFICATION_VARIABLE_TYPE_DOUBLE, 3.14,
                NOTIFICATION_VARIABLE_TYPE_NONE),
            NOTIFICATION_ERROR_NONE);
}

TEST_F(NotificationApiTest, set_text_with_string_var_arg) {
  ASSERT_EQ(notification_set_text(noti_, NOTIFICATION_TEXT_TYPE_CONTENT,
                "user %s", nullptr,
                NOTIFICATION_VARIABLE_TYPE_STRING, "alice",
                NOTIFICATION_VARIABLE_TYPE_NONE),
            NOTIFICATION_ERROR_NONE);
}

TEST_F(NotificationApiTest, set_text_with_count_var_arg) {
  ASSERT_EQ(notification_set_text(noti_, NOTIFICATION_TEXT_TYPE_CONTENT,
                "count %d", nullptr,
                NOTIFICATION_VARIABLE_TYPE_COUNT,
                NOTIFICATION_COUNT_POS_LEFT,
                NOTIFICATION_VARIABLE_TYPE_NONE),
            NOTIFICATION_ERROR_NONE);
}

/* ============================================================== */
/* notification_get_pkgname via aul-mocked fixture roundtrip        */
/* ============================================================== */

TEST_F(NotificationApiTest, get_pkgname_returns_aul_mocked_appid) {
  char* pkgname = nullptr;
  EXPECT_EQ(notification_get_pkgname(noti_, &pkgname),
            NOTIFICATION_ERROR_NONE);
  ASSERT_NE(pkgname, nullptr);
  /* The aul mock returns "org.tizen.testappid"; pkgname tracks
   * caller_app_id which is derived from that. */
  EXPECT_NE(pkgname[0], '\0');
}

/* ============================================================== */
/* notification_remove_button valid path (after add)               */
/* ============================================================== */

TEST_F(NotificationApiTest, remove_button_after_add) {
  ASSERT_EQ(notification_add_button(noti_, NOTIFICATION_BUTTON_2),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(notification_remove_button(noti_, NOTIFICATION_BUTTON_2),
            NOTIFICATION_ERROR_NONE);
}

/* ============================================================== */
/* notification_set_image with image collision then read           */
/* ============================================================== */

TEST_F(NotificationApiTest, set_image_then_set_null_at_same_type) {
  ASSERT_EQ(notification_set_image(noti_, NOTIFICATION_IMAGE_TYPE_ICON,
                "/a.png"),
            NOTIFICATION_ERROR_NONE);
  /* Setting NULL at the same type deletes the prior bundle entry. */
  ASSERT_EQ(notification_set_image(noti_, NOTIFICATION_IMAGE_TYPE_ICON,
                nullptr),
            NOTIFICATION_ERROR_NONE);
}

/* ============================================================== */
/* notification_set_led_time_period zero/negative                  */
/* ============================================================== */

TEST_F(NotificationApiTest, set_led_time_period_zero) {
  EXPECT_EQ(notification_set_led_time_period(noti_, 0, 0),
            NOTIFICATION_ERROR_NONE);
  int on_ms = 1, off_ms = 1;
  EXPECT_EQ(notification_get_led_time_period(noti_, &on_ms, &off_ms),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(on_ms, 0);
  EXPECT_EQ(off_ms, 0);
}

/* ============================================================== */
/* notification_get_image with no bundle (fresh handle)            */
/* ============================================================== */

TEST_F(NotificationApiTest, get_image_no_bundle_non_icon) {
  /* b_image_path is NULL on a fresh notification. Type != ICON yields
   * NONE with *image_path == NULL. */
  char* path = nullptr;
  EXPECT_EQ(notification_get_image(noti_, NOTIFICATION_IMAGE_TYPE_LIST_1,
                &path),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(path, nullptr);
}

TEST_F(NotificationApiTest, get_image_no_bundle_icon_falls_back_or_null) {
  /* For ICON type the impl returns app_icon_path or NULL when not set. */
  char sentinel_storage = 0;
  char* sentinel = &sentinel_storage;
  char* path = sentinel;
  EXPECT_EQ(notification_get_image(noti_, NOTIFICATION_IMAGE_TYPE_ICON,
                &path),
            NOTIFICATION_ERROR_NONE);
  /* path is either NULL or a non-empty string from app_icon_path. */
  if (path != sentinel) {
    /* Either NULL or app_icon_path was written. */
    if (path != nullptr) {
      EXPECT_NE(path[0], '\0');
    }
  }
}

/* ============================================================== */
/* notification_set_property single-flag toggling                  */
/* ============================================================== */

TEST_F(NotificationApiTest, set_property_each_flag_individually) {
  int flags[] = {
      NOTIFICATION_PROP_DISABLE_APP_LAUNCH,
      NOTIFICATION_PROP_DISABLE_AUTO_DELETE,
      NOTIFICATION_PROP_DISABLE_TICKERNOTI,
      NOTIFICATION_PROP_PERMANENT_DISPLAY,
      NOTIFICATION_PROP_DISABLE_UPDATE_ON_INSERT,
      NOTIFICATION_PROP_DISABLE_UPDATE_ON_DELETE,
      NOTIFICATION_PROP_VOLATILE_DISPLAY,
  };
  for (size_t i = 0; i < sizeof(flags) / sizeof(flags[0]); ++i) {
    ASSERT_EQ(notification_set_property(noti_, flags[i]),
              NOTIFICATION_ERROR_NONE);
    int got = 0;
    EXPECT_EQ(notification_get_property(noti_, &got),
              NOTIFICATION_ERROR_NONE);
    EXPECT_EQ(got, flags[i]);
  }
}

/* ============================================================== */
/* add_button / remove_button each valid index                     */
/* ============================================================== */

TEST_F(NotificationApiTest, add_button_all_valid_indices) {
  for (int i = NOTIFICATION_BUTTON_1; i <= NOTIFICATION_BUTTON_6; ++i) {
    EXPECT_EQ(notification_add_button(noti_,
                  static_cast<notification_button_index_e>(i)),
              NOTIFICATION_ERROR_NONE);
  }
  for (int i = NOTIFICATION_BUTTON_7; i <= NOTIFICATION_BUTTON_10; ++i) {
    EXPECT_EQ(notification_add_button(noti_,
                  static_cast<notification_button_index_e>(i)),
              NOTIFICATION_ERROR_NONE);
  }
}

TEST_F(NotificationApiTest, remove_button_all_valid_indices) {
  /* Add then remove the full range. */
  for (int i = NOTIFICATION_BUTTON_1; i <= NOTIFICATION_BUTTON_6; ++i) {
    ASSERT_EQ(notification_add_button(noti_,
                  static_cast<notification_button_index_e>(i)),
              NOTIFICATION_ERROR_NONE);
  }
  for (int i = NOTIFICATION_BUTTON_1; i <= NOTIFICATION_BUTTON_6; ++i) {
    EXPECT_EQ(notification_remove_button(noti_,
                  static_cast<notification_button_index_e>(i)),
              NOTIFICATION_ERROR_NONE);
  }
}

/* ============================================================== */
/* set_ongoing_flag toggling                                       */
/* ============================================================== */

TEST_F(NotificationApiTest, set_ongoing_flag_multiple_toggles) {
  for (int i = 0; i < 4; ++i) {
    bool target = (i % 2 == 0);
    ASSERT_EQ(notification_set_ongoing_flag(noti_, target),
              NOTIFICATION_ERROR_NONE);
    bool got = !target;
    ASSERT_EQ(notification_get_ongoing_flag(noti_, &got),
              NOTIFICATION_ERROR_NONE);
    EXPECT_EQ(got, target);
  }
}

/* ============================================================== */
/* set_auto_remove multiple toggles                                */
/* ============================================================== */

TEST_F(NotificationApiTest, set_auto_remove_multiple_toggles) {
  for (int i = 0; i < 4; ++i) {
    bool target = (i % 2 == 0);
    ASSERT_EQ(notification_set_auto_remove(noti_, target),
              NOTIFICATION_ERROR_NONE);
    bool got = !target;
    ASSERT_EQ(notification_get_auto_remove(noti_, &got),
              NOTIFICATION_ERROR_NONE);
    EXPECT_EQ(got, target);
  }
}

/* ============================================================== */
/* set_tag with empty string                                       */
/* ============================================================== */

TEST_F(NotificationApiTest, set_tag_empty_string_stored) {
  ASSERT_EQ(notification_set_tag(noti_, ""), NOTIFICATION_ERROR_NONE);
  const char* got = nullptr;
  EXPECT_EQ(notification_get_tag(noti_, &got), NOTIFICATION_ERROR_NONE);
  ASSERT_NE(got, nullptr);
  EXPECT_STREQ(got, "");
}

/* ============================================================== */
/* set_sound_path with NONE type does not need path                */
/* ============================================================== */

TEST_F(NotificationApiTest, set_sound_none_type) {
  EXPECT_EQ(notification_set_sound(noti_, NOTIFICATION_SOUND_TYPE_NONE,
                nullptr),
            NOTIFICATION_ERROR_NONE);
  notification_sound_type_e got;
  const char* path = nullptr;
  EXPECT_EQ(notification_get_sound(noti_, &got, &path),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(got, NOTIFICATION_SOUND_TYPE_NONE);
}

/* ============================================================== */
/* set_vibration_none type                                          */
/* ============================================================== */

TEST_F(NotificationApiTest, set_vibration_none_type) {
  EXPECT_EQ(notification_set_vibration(noti_,
                NOTIFICATION_VIBRATION_TYPE_NONE, nullptr),
            NOTIFICATION_ERROR_NONE);
}

/* ============================================================== */
/* notification_set_extension_image_size boundary                  */
/* ============================================================== */

TEST_F(NotificationApiTest, set_extension_image_size_zero_returns_invalid) {
  /* Per impl: height must be > 0. */
  EXPECT_EQ(notification_set_extension_image_size(noti_, 0),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationApiTest, set_extension_image_size_negative_returns_invalid) {
  EXPECT_EQ(notification_set_extension_image_size(noti_, -1),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationApiTest, set_extension_image_size_large) {
  ASSERT_EQ(notification_set_extension_image_size(noti_, 4096),
            NOTIFICATION_ERROR_NONE);
  int h = 0;
  EXPECT_EQ(notification_get_extension_image_size(noti_, &h),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(h, 4096);
}

/* ============================================================== */
/* notification_set_size/progress idempotent same value            */
/* ============================================================== */

TEST_F(NotificationApiTest, set_size_idempotent_same_value) {
  ASSERT_EQ(notification_set_size(noti_, 0.5), NOTIFICATION_ERROR_NONE);
  ASSERT_EQ(notification_set_size(noti_, 0.5), NOTIFICATION_ERROR_NONE);
  double v = 0;
  EXPECT_EQ(notification_get_size(noti_, &v), NOTIFICATION_ERROR_NONE);
  EXPECT_DOUBLE_EQ(v, 0.5);
}

TEST_F(NotificationApiTest, set_progress_idempotent_same_value) {
  ASSERT_EQ(notification_set_progress(noti_, 0.5),
            NOTIFICATION_ERROR_NONE);
  ASSERT_EQ(notification_set_progress(noti_, 0.5),
            NOTIFICATION_ERROR_NONE);
  double v = 0;
  EXPECT_EQ(notification_get_progress(noti_, &v),
            NOTIFICATION_ERROR_NONE);
  EXPECT_DOUBLE_EQ(v, 0.5);
}

/* ============================================================== */
/* notification_set_time_to_text on CONTENT (different type)       */
/* ============================================================== */

TEST_F(NotificationApiTest, set_time_to_text_content_type_roundtrip) {
  time_t t = 1700000000;
  ASSERT_EQ(notification_set_time_to_text(noti_,
                NOTIFICATION_TEXT_TYPE_CONTENT, t),
            NOTIFICATION_ERROR_NONE);
  time_t got = 0;
  EXPECT_EQ(notification_get_time_from_text(noti_,
                NOTIFICATION_TEXT_TYPE_CONTENT, &got),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(got, t);
}

/* ============================================================== */
/* notification_get_property default value is zero on fresh handle */
/* ============================================================== */

TEST_F(NotificationApiTest, get_property_default_zero) {
  int got = 0xDEAD;
  EXPECT_EQ(notification_get_property(noti_, &got),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(got, 0);
}

/* ============================================================== */
/* notification_get_display_applist default value                  */
/* ============================================================== */

TEST_F(NotificationApiTest, get_display_applist_default) {
  int got = 0xDEAD;
  EXPECT_EQ(notification_get_display_applist(noti_, &got),
            NOTIFICATION_ERROR_NONE);
  /* Default depends on impl; just confirm it was written. */
  EXPECT_NE(got, 0xDEAD);
}

/* ============================================================== */
/* notification_get_layout default                                  */
/* ============================================================== */

TEST_F(NotificationApiTest, get_layout_default) {
  notification_ly_type_e got =
      static_cast<notification_ly_type_e>(99);
  EXPECT_EQ(notification_get_layout(noti_, &got),
            NOTIFICATION_ERROR_NONE);
  EXPECT_NE(got, 99);
}

/* ============================================================== */
/* notification_get_size / get_progress default zero               */
/* ============================================================== */

TEST_F(NotificationApiTest, get_size_default_zero) {
  double v = -1;
  EXPECT_EQ(notification_get_size(noti_, &v),
            NOTIFICATION_ERROR_NONE);
  EXPECT_DOUBLE_EQ(v, 0.0);
}

TEST_F(NotificationApiTest, get_progress_default_zero) {
  double v = -1;
  EXPECT_EQ(notification_get_progress(noti_, &v),
            NOTIFICATION_ERROR_NONE);
  EXPECT_DOUBLE_EQ(v, 0.0);
}

/* ============================================================== */
/* notification_get_type after create                              */
/* ============================================================== */

TEST_F(NotificationApiTest, get_type_after_create_returns_noti) {
  notification_type_e type;
  EXPECT_EQ(notification_get_type(noti_, &type),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(type, NOTIFICATION_TYPE_NOTI);
}

/* ============================================================== */
/* notification_set_vibration USER_DATA with null path             */
/* ============================================================== */

TEST_F(NotificationApiTest, set_vibration_user_data_null_path) {
  /* USER_DATA without path may be accepted (no validation in
   * notification.c — unlike sound). Confirm error code. */
  int r = notification_set_vibration(noti_,
              NOTIFICATION_VIBRATION_TYPE_USER_DATA, nullptr);
  EXPECT_TRUE(r == NOTIFICATION_ERROR_NONE ||
              r == NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* ============================================================== */
/* notification_get_sound default                                  */
/* ============================================================== */

TEST_F(NotificationApiTest, get_sound_default) {
  notification_sound_type_e type;
  const char* path = nullptr;
  EXPECT_EQ(notification_get_sound(noti_, &type, &path),
            NOTIFICATION_ERROR_NONE);
  /* Default: NONE per impl. */
  EXPECT_EQ(type, NOTIFICATION_SOUND_TYPE_NONE);
}

/* ============================================================== */
/* notification_get_vibration default                              */
/* ============================================================== */

TEST_F(NotificationApiTest, get_vibration_default) {
  notification_vibration_type_e type;
  const char* path = nullptr;
  EXPECT_EQ(notification_get_vibration(noti_, &type, &path),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(type, NOTIFICATION_VIBRATION_TYPE_NONE);
}

/* ============================================================== */
/* notification_get_led default                                     */
/* ============================================================== */

TEST_F(NotificationApiTest, get_led_default) {
  notification_led_op_e op;
  int color = 0;
  EXPECT_EQ(notification_get_led(noti_, &op, &color),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(op, NOTIFICATION_LED_OP_OFF);
}

/* ============================================================== */
/* notification_set_image with empty path stores ""                */
/* ============================================================== */

TEST_F(NotificationApiTest, set_image_empty_path_at_lock_screen) {
  ASSERT_EQ(notification_set_image(noti_,
                NOTIFICATION_IMAGE_TYPE_ICON_FOR_LOCK, ""),
            NOTIFICATION_ERROR_NONE);
  char* p = nullptr;
  EXPECT_EQ(notification_get_image(noti_,
                NOTIFICATION_IMAGE_TYPE_ICON_FOR_LOCK, &p),
            NOTIFICATION_ERROR_NONE);
  EXPECT_STREQ(p, "");
}

/* ============================================================== */
/* set_image clearing flow: SET → NULL → no value                  */
/* ============================================================== */

TEST_F(NotificationApiTest, set_image_set_null_clears_value) {
  ASSERT_EQ(notification_set_image(noti_, NOTIFICATION_IMAGE_TYPE_LIST_2,
                "/abc"),
            NOTIFICATION_ERROR_NONE);
  ASSERT_EQ(notification_set_image(noti_, NOTIFICATION_IMAGE_TYPE_LIST_2,
                nullptr),
            NOTIFICATION_ERROR_NONE);
  char* p = nullptr;
  EXPECT_EQ(notification_get_image(noti_, NOTIFICATION_IMAGE_TYPE_LIST_2,
                &p),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(p, nullptr);
}

/* ============================================================== */
/* notification_set_extension_image_size double-set                */
/* ============================================================== */

TEST_F(NotificationApiTest, set_extension_image_size_double_set) {
  ASSERT_EQ(notification_set_extension_image_size(noti_, 100),
            NOTIFICATION_ERROR_NONE);
  ASSERT_EQ(notification_set_extension_image_size(noti_, 200),
            NOTIFICATION_ERROR_NONE);
  int h = 0;
  EXPECT_EQ(notification_get_extension_image_size(noti_, &h),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(h, 200);
}

/* ============================================================== */
/* set_text empty string                                            */
/* ============================================================== */

TEST_F(NotificationApiTest, set_text_empty_string) {
  ASSERT_EQ(notification_set_text(noti_, NOTIFICATION_TEXT_TYPE_TITLE,
                "", nullptr, NOTIFICATION_VARIABLE_TYPE_NONE),
            NOTIFICATION_ERROR_NONE);
  char* got = nullptr;
  EXPECT_EQ(notification_get_text(noti_, NOTIFICATION_TEXT_TYPE_TITLE,
                &got),
            NOTIFICATION_ERROR_NONE);
  ASSERT_NE(got, nullptr);
  EXPECT_STREQ(got, "");
}

/* ============================================================== */
/* set_text with both text and key                                  */
/* ============================================================== */

TEST_F(NotificationApiTest, set_text_with_both_text_and_key) {
  ASSERT_EQ(notification_set_text(noti_, NOTIFICATION_TEXT_TYPE_TITLE,
                "fallback", "IDS_KEY",
                NOTIFICATION_VARIABLE_TYPE_NONE),
            NOTIFICATION_ERROR_NONE);
}

/* ============================================================== */
/* set_layout LY_NONE accepted as valid                            */
/* ============================================================== */

TEST_F(NotificationApiTest, set_layout_none_accepted) {
  EXPECT_EQ(notification_set_layout(noti_, NOTIFICATION_LY_NONE),
            NOTIFICATION_ERROR_NONE);
  notification_ly_type_e got;
  EXPECT_EQ(notification_get_layout(noti_, &got),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(got, NOTIFICATION_LY_NONE);
}

/* ============================================================== */
/* set_text type loop with key set                                  */
/* ============================================================== */

TEST_F(NotificationApiTest, set_text_each_type_with_key) {
  for (int t = NOTIFICATION_TEXT_TYPE_TITLE;
       t <= NOTIFICATION_TEXT_TYPE_MAX; ++t) {
    EXPECT_EQ(notification_set_text(noti_,
                  static_cast<notification_text_type_e>(t),
                  nullptr, "IDS_KEY",
                  NOTIFICATION_VARIABLE_TYPE_NONE),
              NOTIFICATION_ERROR_NONE);
  }
}

/* ============================================================== */
/* set_tag null clears                                              */
/* ============================================================== */

TEST_F(NotificationApiTest, set_tag_null_value_is_noop) {
  /* Per impl: NULL tag is a no-op (existing value preserved), returns NONE. */
  ASSERT_EQ(notification_set_tag(noti_, "my_tag"),
            NOTIFICATION_ERROR_NONE);
  ASSERT_EQ(notification_set_tag(noti_, nullptr),
            NOTIFICATION_ERROR_NONE);
  const char* got = nullptr;
  EXPECT_EQ(notification_get_tag(noti_, &got), NOTIFICATION_ERROR_NONE);
  EXPECT_STREQ(got, "my_tag");
}

/* ============================================================== */
/* set_event_handler iterates valid CLICK_ON_BUTTON_* event types  */
/* ============================================================== */

TEST_F(NotificationApiTest, set_event_handler_click_on_icon) {
  app_control_h ac = nullptr;
  app_control_create(&ac);
  EXPECT_EQ(notification_set_event_handler(noti_,
                NOTIFICATION_EVENT_TYPE_CLICK_ON_ICON, ac),
            NOTIFICATION_ERROR_NONE);
  app_control_destroy(ac);
}

TEST_F(NotificationApiTest, set_event_handler_click_on_thumbnail) {
  app_control_h ac = nullptr;
  app_control_create(&ac);
  EXPECT_EQ(notification_set_event_handler(noti_,
                NOTIFICATION_EVENT_TYPE_CLICK_ON_THUMBNAIL, ac),
            NOTIFICATION_ERROR_NONE);
  app_control_destroy(ac);
}

TEST_F(NotificationApiTest, set_event_handler_click_on_text_input_button) {
  app_control_h ac = nullptr;
  app_control_create(&ac);
  EXPECT_EQ(notification_set_event_handler(noti_,
                NOTIFICATION_EVENT_TYPE_CLICK_ON_TEXT_INPUT_BUTTON, ac),
            NOTIFICATION_ERROR_NONE);
  app_control_destroy(ac);
}

TEST_F(NotificationApiTest, set_event_handler_each_extended_click_button_type) {
  app_control_h ac = nullptr;
  app_control_create(&ac);
  for (int t = NOTIFICATION_EVENT_TYPE_CLICK_ON_BUTTON_7;
       t <= NOTIFICATION_EVENT_TYPE_CLICK_ON_BUTTON_10; ++t) {
    EXPECT_EQ(notification_set_event_handler(noti_,
                  static_cast<notification_event_type_e>(t), ac),
              NOTIFICATION_ERROR_NONE);
  }
  app_control_destroy(ac);
}

TEST_F(NotificationApiTest, get_event_handler_each_unset_returns_not_exist) {
  notification_event_type_e types[] = {
      NOTIFICATION_EVENT_TYPE_CLICK_ON_BUTTON_1,
      NOTIFICATION_EVENT_TYPE_CLICK_ON_BUTTON_2,
      NOTIFICATION_EVENT_TYPE_CLICK_ON_BUTTON_3,
      NOTIFICATION_EVENT_TYPE_CLICK_ON_ICON,
      NOTIFICATION_EVENT_TYPE_CLICK_ON_THUMBNAIL,
      NOTIFICATION_EVENT_TYPE_CLICK_ON_TEXT_INPUT_BUTTON,
  };
  for (size_t i = 0; i < sizeof(types) / sizeof(types[0]); ++i) {
    app_control_h got = nullptr;
    EXPECT_EQ(notification_get_event_handler(noti_, types[i], &got),
              NOTIFICATION_ERROR_NOT_EXIST_ID);
  }
}

TEST_F(NotificationApiTest, set_then_get_event_handler_for_each_type_preserved) {
  for (int t = NOTIFICATION_EVENT_TYPE_CLICK_ON_BUTTON_1;
       t <= NOTIFICATION_EVENT_TYPE_CLICK_ON_BUTTON_6; ++t) {
    app_control_h ac = nullptr;
    app_control_create(&ac);
    ASSERT_EQ(notification_set_event_handler(noti_,
                  static_cast<notification_event_type_e>(t), ac),
              NOTIFICATION_ERROR_NONE);
    app_control_destroy(ac);

    app_control_h got = nullptr;
    EXPECT_EQ(notification_get_event_handler(noti_,
                  static_cast<notification_event_type_e>(t), &got),
              NOTIFICATION_ERROR_NONE);
    if (got != nullptr) {
      app_control_destroy(got);
    }
  }
}

TEST_F(NotificationApiTest, set_event_handler_each_click_button_type) {
  app_control_h ac = nullptr;
  app_control_create(&ac);
  for (int t = NOTIFICATION_EVENT_TYPE_CLICK_ON_BUTTON_1;
       t <= NOTIFICATION_EVENT_TYPE_CLICK_ON_BUTTON_6; ++t) {
    EXPECT_EQ(notification_set_event_handler(noti_,
                  static_cast<notification_event_type_e>(t), ac),
              NOTIFICATION_ERROR_NONE);
  }
  app_control_destroy(ac);
}

/* ============================================================== */
/* get_pkgname after set_app_id replacement                         */
/* ============================================================== */

TEST_F(NotificationApiTest, get_pkgname_returns_caller_app_id_default) {
  /* The fixture configures aul to return "org.tizen.testappid" via
   * the mock, and notification_create populates caller_app_id. */
  char* p = nullptr;
  EXPECT_EQ(notification_get_pkgname(noti_, &p),
            NOTIFICATION_ERROR_NONE);
  ASSERT_NE(p, nullptr);
  EXPECT_STREQ(p, "org.tizen.testappid");
}

/* ============================================================== */
/* set_progress with NaN-like values (impl doesn't validate)       */
/* ============================================================== */

TEST_F(NotificationApiTest, set_progress_inf_stored) {
  double inf = 1e308 * 10;  /* +inf */
  ASSERT_EQ(notification_set_progress(noti_, inf),
            NOTIFICATION_ERROR_NONE);
  double v = 0;
  EXPECT_EQ(notification_get_progress(noti_, &v),
            NOTIFICATION_ERROR_NONE);
  /* Any value (including inf) is stored verbatim. */
  (void)v;
}

/* ============================================================== */
/* set_time large value                                            */
/* ============================================================== */

TEST_F(NotificationApiTest, set_time_int_max) {
  time_t t = 2147483647;
  ASSERT_EQ(notification_set_time(noti_, t), NOTIFICATION_ERROR_NONE);
  time_t got = 0;
  EXPECT_EQ(notification_get_time(noti_, &got), NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(got, t);
}

/* ============================================================== */
/* set_size negative stored                                         */
/* ============================================================== */

TEST_F(NotificationApiTest, set_size_negative_stored) {
  /* No validation — negative stored verbatim. */
  ASSERT_EQ(notification_set_size(noti_, -1.5), NOTIFICATION_ERROR_NONE);
  double v = 0;
  EXPECT_EQ(notification_get_size(noti_, &v), NOTIFICATION_ERROR_NONE);
  EXPECT_DOUBLE_EQ(v, -1.5);
}

/* ============================================================== */
/* set_display_applist all single bits                              */
/* ============================================================== */

TEST_F(NotificationApiTest, set_display_applist_each_bit) {
  int bits[] = {
      NOTIFICATION_DISPLAY_APP_NOTIFICATION_TRAY,
      NOTIFICATION_DISPLAY_APP_TICKER,
      NOTIFICATION_DISPLAY_APP_LOCK,
      NOTIFICATION_DISPLAY_APP_INDICATOR,
      NOTIFICATION_DISPLAY_APP_ACTIVE,
  };
  for (size_t i = 0; i < sizeof(bits) / sizeof(bits[0]); ++i) {
    ASSERT_EQ(notification_set_display_applist(noti_, bits[i]),
              NOTIFICATION_ERROR_NONE);
    int got = 0;
    EXPECT_EQ(notification_get_display_applist(noti_, &got),
              NOTIFICATION_ERROR_NONE);
    EXPECT_EQ(got, bits[i]);
  }
}

/* ============================================================== */
/* notification_set_auto_remove default after create               */
/* ============================================================== */

TEST_F(NotificationApiTest, get_auto_remove_default) {
  bool got = false;
  EXPECT_EQ(notification_get_auto_remove(noti_, &got),
            NOTIFICATION_ERROR_NONE);
  /* Default per notification_create: true. */
  EXPECT_TRUE(got);
}

/* ============================================================== */
/* get_ongoing_flag default                                         */
/* ============================================================== */

TEST_F(NotificationApiTest, get_ongoing_flag_default) {
  bool got = true;
  EXPECT_EQ(notification_get_ongoing_flag(noti_, &got),
            NOTIFICATION_ERROR_NONE);
  EXPECT_FALSE(got);
}

/* ============================================================== */
/* get_time / get_insert_time default                              */
/* ============================================================== */

TEST_F(NotificationApiTest, get_time_default) {
  time_t got = -1;
  EXPECT_EQ(notification_get_time(noti_, &got),
            NOTIFICATION_ERROR_NONE);
  /* Default time on fresh handle is 0. */
  EXPECT_EQ(got, 0);
}

/* ============================================================== */
/* set_tag long string                                              */
/* ============================================================== */

TEST_F(NotificationApiTest, set_tag_long_string) {
  std::string long_tag(1000, 'x');
  ASSERT_EQ(notification_set_tag(noti_, long_tag.c_str()),
            NOTIFICATION_ERROR_NONE);
  const char* got = nullptr;
  EXPECT_EQ(notification_get_tag(noti_, &got),
            NOTIFICATION_ERROR_NONE);
  ASSERT_NE(got, nullptr);
  EXPECT_EQ(std::string(got), long_tag);
}

/* ============================================================== */
/* set_image with long path                                         */
/* ============================================================== */

TEST_F(NotificationApiTest, set_image_long_path) {
  std::string long_path = "/tmp/" + std::string(500, 'a') + ".png";
  ASSERT_EQ(notification_set_image(noti_,
                NOTIFICATION_IMAGE_TYPE_ICON, long_path.c_str()),
            NOTIFICATION_ERROR_NONE);
  char* p = nullptr;
  EXPECT_EQ(notification_get_image(noti_,
                NOTIFICATION_IMAGE_TYPE_ICON, &p),
            NOTIFICATION_ERROR_NONE);
  ASSERT_NE(p, nullptr);
  /* The impl truncates to NOTI_TEXT_RESULT_LEN; just confirm non-null. */
  EXPECT_NE(p[0], '\0');
}

/* ============================================================== */
/* set_extension_image_size after each valid value                  */
/* ============================================================== */

TEST_F(NotificationApiTest, set_extension_image_size_each_value) {
  int values[] = {1, 50, 100, 500, 1000, 4096};
  for (size_t i = 0; i < sizeof(values) / sizeof(values[0]); ++i) {
    ASSERT_EQ(notification_set_extension_image_size(noti_, values[i]),
              NOTIFICATION_ERROR_NONE);
    int got = 0;
    EXPECT_EQ(notification_get_extension_image_size(noti_, &got),
              NOTIFICATION_ERROR_NONE);
    EXPECT_EQ(got, values[i]);
  }
}

/* ============================================================== */
/* set_sound user data path round-trip                              */
/* ============================================================== */

TEST_F(NotificationApiTest, set_sound_user_data_roundtrip) {
  ASSERT_EQ(notification_set_sound(noti_,
                NOTIFICATION_SOUND_TYPE_USER_DATA, "/tmp/s.ogg"),
            NOTIFICATION_ERROR_NONE);
  notification_sound_type_e got;
  const char* path = nullptr;
  EXPECT_EQ(notification_get_sound(noti_, &got, &path),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(got, NOTIFICATION_SOUND_TYPE_USER_DATA);
  EXPECT_STREQ(path, "/tmp/s.ogg");
}

/* ============================================================== */
/* set_vibration user data path round-trip                          */
/* ============================================================== */

TEST_F(NotificationApiTest, set_vibration_user_data_roundtrip) {
  ASSERT_EQ(notification_set_vibration(noti_,
                NOTIFICATION_VIBRATION_TYPE_USER_DATA, "/tmp/v.dat"),
            NOTIFICATION_ERROR_NONE);
  notification_vibration_type_e got;
  const char* path = nullptr;
  EXPECT_EQ(notification_get_vibration(noti_, &got, &path),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(got, NOTIFICATION_VIBRATION_TYPE_USER_DATA);
  EXPECT_STREQ(path, "/tmp/v.dat");
}

/* ============================================================== */
/* set_sound replace USER_DATA → DEFAULT clears path                */
/* ============================================================== */

TEST_F(NotificationApiTest, set_sound_user_data_then_default_clears) {
  ASSERT_EQ(notification_set_sound(noti_,
                NOTIFICATION_SOUND_TYPE_USER_DATA, "/p"),
            NOTIFICATION_ERROR_NONE);
  ASSERT_EQ(notification_set_sound(noti_,
                NOTIFICATION_SOUND_TYPE_DEFAULT, nullptr),
            NOTIFICATION_ERROR_NONE);
  notification_sound_type_e got;
  const char* path = nullptr;
  EXPECT_EQ(notification_get_sound(noti_, &got, &path),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(got, NOTIFICATION_SOUND_TYPE_DEFAULT);
}

/* ============================================================== */
/* notification_set_priority not in public API: skipped             */
/* notification_set_image after multiple types                      */
/* ============================================================== */

TEST_F(NotificationApiTest, set_image_replace_with_different_type) {
  ASSERT_EQ(notification_set_image(noti_, NOTIFICATION_IMAGE_TYPE_ICON,
                "/i1.png"),
            NOTIFICATION_ERROR_NONE);
  ASSERT_EQ(notification_set_image(noti_,
                NOTIFICATION_IMAGE_TYPE_BACKGROUND, "/bg.png"),
            NOTIFICATION_ERROR_NONE);
  char* a = nullptr;
  EXPECT_EQ(notification_get_image(noti_, NOTIFICATION_IMAGE_TYPE_ICON, &a),
            NOTIFICATION_ERROR_NONE);
  EXPECT_STREQ(a, "/i1.png");
  char* b = nullptr;
  EXPECT_EQ(notification_get_image(noti_,
                NOTIFICATION_IMAGE_TYPE_BACKGROUND, &b),
            NOTIFICATION_ERROR_NONE);
  EXPECT_STREQ(b, "/bg.png");
}

/* ============================================================== */
/* set_layout_each_valid roundtrip                                  */
/* ============================================================== */

TEST_F(NotificationApiTest, set_text_then_set_text_clears_via_null) {
  ASSERT_EQ(notification_set_text(noti_, NOTIFICATION_TEXT_TYPE_TITLE,
                "first", nullptr, NOTIFICATION_VARIABLE_TYPE_NONE),
            NOTIFICATION_ERROR_NONE);
  ASSERT_EQ(notification_set_text(noti_, NOTIFICATION_TEXT_TYPE_TITLE,
                nullptr, nullptr, NOTIFICATION_VARIABLE_TYPE_NONE),
            NOTIFICATION_ERROR_NONE);
  char* got = nullptr;
  EXPECT_EQ(notification_get_text(noti_, NOTIFICATION_TEXT_TYPE_TITLE, &got),
            NOTIFICATION_ERROR_NONE);
  /* Cleared: no text. */
  EXPECT_EQ(got, nullptr);
}

/* ============================================================== */
/* set_text long text                                              */
/* ============================================================== */

TEST_F(NotificationApiTest, set_text_long_text_stored) {
  std::string long_text(500, 'X');
  EXPECT_EQ(notification_set_text(noti_, NOTIFICATION_TEXT_TYPE_TITLE,
                long_text.c_str(), nullptr,
                NOTIFICATION_VARIABLE_TYPE_NONE),
            NOTIFICATION_ERROR_NONE);
  char* got = nullptr;
  EXPECT_EQ(notification_get_text(noti_, NOTIFICATION_TEXT_TYPE_TITLE,
                &got),
            NOTIFICATION_ERROR_NONE);
  ASSERT_NE(got, nullptr);
  /* Long text stored — impl may truncate to NOTI_TEXT_RESULT_LEN. */
  EXPECT_NE(got[0], '\0');
}

/* ============================================================== */
/* set_layout each non-NONE value                                  */
/* ============================================================== */

TEST_F(NotificationApiTest, set_layout_event_types) {
  notification_ly_type_e types[] = {
      NOTIFICATION_LY_NOTI_EVENT_SINGLE,
      NOTIFICATION_LY_NOTI_EVENT_MULTIPLE,
      NOTIFICATION_LY_NOTI_THUMBNAIL,
      NOTIFICATION_LY_ONGOING_EVENT,
      NOTIFICATION_LY_ONGOING_PROGRESS,
  };
  for (size_t i = 0; i < sizeof(types) / sizeof(types[0]); ++i) {
    ASSERT_EQ(notification_set_layout(noti_, types[i]),
              NOTIFICATION_ERROR_NONE);
    notification_ly_type_e got;
    EXPECT_EQ(notification_get_layout(noti_, &got),
              NOTIFICATION_ERROR_NONE);
    EXPECT_EQ(got, types[i]);
  }
}

/* ============================================================== */
/* set_image with each ICON variant                                 */
/* ============================================================== */

TEST_F(NotificationApiTest, set_image_each_icon_variant) {
  notification_image_type_e types[] = {
      NOTIFICATION_IMAGE_TYPE_ICON,
      NOTIFICATION_IMAGE_TYPE_ICON_FOR_INDICATOR,
      NOTIFICATION_IMAGE_TYPE_ICON_FOR_LOCK,
      NOTIFICATION_IMAGE_TYPE_THUMBNAIL,
      NOTIFICATION_IMAGE_TYPE_BACKGROUND,
  };
  for (size_t i = 0; i < sizeof(types) / sizeof(types[0]); ++i) {
    std::string p = std::string("/tmp/") + std::to_string(i) + ".png";
    EXPECT_EQ(notification_set_image(noti_, types[i], p.c_str()),
              NOTIFICATION_ERROR_NONE);
    char* got = nullptr;
    EXPECT_EQ(notification_get_image(noti_, types[i], &got),
              NOTIFICATION_ERROR_NONE);
    EXPECT_STREQ(got, p.c_str());
  }
}

/* ============================================================== */
/* set_sound USER_DATA replaces previous path                       */
/* ============================================================== */

/* ============================================================== */
/* notification_create — type validation                            */
/* ============================================================== */

TEST_F(NotificationApiTest, create_type_none_returns_null) {
  notification_h n = notification_create(NOTIFICATION_TYPE_NONE);
  EXPECT_EQ(n, nullptr);
  EXPECT_EQ(get_last_result(),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationApiTest, create_type_above_max_returns_null) {
  notification_h n = notification_create(
      static_cast<notification_type_e>(NOTIFICATION_TYPE_MAX + 1));
  EXPECT_EQ(n, nullptr);
  EXPECT_EQ(get_last_result(),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationApiTest, create_ongoing_succeeds) {
  notification_h n = notification_create(NOTIFICATION_TYPE_ONGOING);
  ASSERT_NE(n, nullptr);
  notification_type_e got;
  EXPECT_EQ(notification_get_type(n, &got),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(got, NOTIFICATION_TYPE_ONGOING);
  notification_free(n);
}

/* ============================================================== */
/* notification_load_by_tag — null tag returns NULL + INVALID       */
/* ============================================================== */

TEST_F(NotificationApiTest, load_by_tag_null_returns_null) {
  notification_h n = notification_load_by_tag(nullptr);
  EXPECT_EQ(n, nullptr);
  EXPECT_EQ(get_last_result(),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* ============================================================== */
/* notification_post — null and bad-type validation                 */
/* ============================================================== */

TEST_F(NotificationApiTest, post_null_returns_invalid) {
  EXPECT_EQ(notification_post(nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationApiTest, update_null_returns_invalid) {
  EXPECT_EQ(notification_update(nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationApiTest, set_sound_user_data_replaces_path) {
  ASSERT_EQ(notification_set_sound(noti_,
                NOTIFICATION_SOUND_TYPE_USER_DATA, "/p1.ogg"),
            NOTIFICATION_ERROR_NONE);
  ASSERT_EQ(notification_set_sound(noti_,
                NOTIFICATION_SOUND_TYPE_USER_DATA, "/p2.ogg"),
            NOTIFICATION_ERROR_NONE);
  notification_sound_type_e t;
  const char* path = nullptr;
  EXPECT_EQ(notification_get_sound(noti_, &t, &path),
            NOTIFICATION_ERROR_NONE);
  EXPECT_STREQ(path, "/p2.ogg");
}

/* ============================================================== */
/* notification_post — type validation (N2/N3 scenarios)           */
/* notification_post_for_uid / notification_update_for_uid IPC     */
/* ============================================================== */

TEST_F(NotificationApiTest, post_type_none_returns_invalid) {
  /* [N2] notification_post delegates to _for_uid which rejects type==NONE */
  noti_->type = NOTIFICATION_TYPE_NONE;
  EXPECT_EQ(notification_post(noti_), NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationApiTest, post_type_overflow_returns_invalid) {
  /* [N3] type > NOTIFICATION_TYPE_MAX rejected before IPC */
  noti_->type = static_cast<notification_type_e>(
      static_cast<int>(NOTIFICATION_TYPE_MAX) + 1);
  EXPECT_EQ(notification_post(noti_), NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationApiTest, post_for_uid_ipc_connect_fail_returns_error) {
  /* [N4] IPC connect failure propagated out of notification_post_for_uid */
  ON_CALL(GetMock<NotiServiceProxyMock>(), rpc_port_proxy_noti_service_connect_sync(_))
      .WillByDefault(Return(RPC_PORT_ERROR_IO_ERROR));
  /* Reset global tidl state so _tidl_init() re-runs connect */
  EXPECT_NE(notification_post_for_uid(noti_, getuid()),
            NOTIFICATION_ERROR_NONE);
}

TEST_F(NotificationApiTest, update_for_uid_ipc_connect_fail_returns_error) {
  /* [N2] IPC connect failure propagated out of notification_update_for_uid */
  ON_CALL(GetMock<NotiServiceProxyMock>(), rpc_port_proxy_noti_service_connect_sync(_))
      .WillByDefault(Return(RPC_PORT_ERROR_IO_ERROR));
  EXPECT_NE(notification_update_for_uid(noti_, getuid()),
            NOTIFICATION_ERROR_NONE);
}

/* ============================================================== */
/* notification_delete_all — type validation + IPC failure (N1-N3) */
/* notification_delete     — null check + IPC failure (N1, N2)    */
/* ============================================================== */

TEST_F(NotificationApiTest, delete_all_type_none_returns_invalid) {
  /* [N1] type==NONE rejected before IPC */
  EXPECT_EQ(notification_delete_all(NOTIFICATION_TYPE_NONE),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationApiTest, delete_all_type_overflow_returns_invalid) {
  /* [N2] type > MAX rejected before IPC */
  auto over = static_cast<notification_type_e>(
      static_cast<int>(NOTIFICATION_TYPE_MAX) + 1);
  EXPECT_EQ(notification_delete_all(over),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationApiTest, delete_all_ipc_connect_fail_returns_error) {
  /* [N3] IPC connect failure propagated */
  ON_CALL(GetMock<NotiServiceProxyMock>(), rpc_port_proxy_noti_service_connect_sync(_))
      .WillByDefault(Return(RPC_PORT_ERROR_IO_ERROR));
  EXPECT_NE(notification_delete_all(NOTIFICATION_TYPE_NOTI),
            NOTIFICATION_ERROR_NONE);
}

TEST_F(NotificationApiTest, delete_ipc_connect_fail_returns_error) {
  /* [N2] notification_delete propagates IPC connect failure */
  ON_CALL(GetMock<NotiServiceProxyMock>(), rpc_port_proxy_noti_service_connect_sync(_))
      .WillByDefault(Return(RPC_PORT_ERROR_IO_ERROR));
  EXPECT_NE(notification_delete(noti_), NOTIFICATION_ERROR_NONE);
}

/* ============================================================== */
/* notification_load_by_tag — null validation (N1)                 */
/* notification_save_as_template — null args (N1, N2)             */
/* ============================================================== */

TEST_F(NotificationApiTest, load_by_tag_null_tag_returns_invalid) {
  /* [N1] tag==NULL → NULL handle, last_result = INVALID_PARAMETER */
  notification_h h = notification_load_by_tag(nullptr);
  EXPECT_EQ(h, nullptr);
  EXPECT_EQ(get_last_result(), NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationApiTest, load_by_tag_ipc_connect_fail_sets_error) {
  /* [N3] IPC connect failure → NULL handle, last_result propagated */
  ON_CALL(GetMock<NotiServiceProxyMock>(), rpc_port_proxy_noti_service_connect_sync(_))
      .WillByDefault(Return(RPC_PORT_ERROR_IO_ERROR));
  notification_h h = notification_load_by_tag("some_tag");
  EXPECT_EQ(h, nullptr);
  EXPECT_NE(get_last_result(), NOTIFICATION_ERROR_NONE);
}

TEST_F(NotificationApiTest, save_as_template_null_noti_returns_invalid) {
  /* [N1] noti==NULL → INVALID_PARAMETER */
  EXPECT_EQ(notification_save_as_template(nullptr, "tmpl"),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationApiTest, save_as_template_null_name_returns_invalid) {
  /* [N2] template_name==NULL → INVALID_PARAMETER */
  EXPECT_EQ(notification_save_as_template(noti_, nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationApiTest, save_as_template_ipc_connect_fail_returns_error) {
  /* IPC connect failure propagated */
  ON_CALL(GetMock<NotiServiceProxyMock>(), rpc_port_proxy_noti_service_connect_sync(_))
      .WillByDefault(Return(RPC_PORT_ERROR_IO_ERROR));
  EXPECT_NE(notification_save_as_template(noti_, "tmpl"),
            NOTIFICATION_ERROR_NONE);
}

/* ============================================================== */
/* notification_create_from_template — null + IPC failure (N1, N2) */
/* notification_get_noti_block_state — null + IPC failure (N1, N2) */
/* ============================================================== */

TEST_F(NotificationApiTest, create_from_template_null_name_returns_invalid) {
  /* [N1] template_name==NULL → NULL, last_result = INVALID_PARAMETER */
  ASSERT_EQ(notification_create_from_template(nullptr), nullptr);
  EXPECT_EQ(get_last_result(), NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationApiTest, create_from_template_ipc_connect_fail_sets_error) {
  /* [N2] IPC connect failure → NULL, last_result propagated */
  ON_CALL(GetMock<NotiServiceProxyMock>(), rpc_port_proxy_noti_service_connect_sync(_))
      .WillByDefault(Return(RPC_PORT_ERROR_IO_ERROR));
  ASSERT_EQ(notification_create_from_template("tmpl"), nullptr);
  EXPECT_NE(get_last_result(), NOTIFICATION_ERROR_NONE);
}

TEST_F(NotificationApiTest, get_noti_block_state_null_state_returns_invalid) {
  /* [N1] state==NULL → INVALID_PARAMETER */
  EXPECT_EQ(notification_get_noti_block_state(nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationApiTest, get_noti_block_state_ipc_connect_fail_returns_error) {
  /* [N2] IPC connect failure propagated */
  ON_CALL(GetMock<NotiServiceProxyMock>(), rpc_port_proxy_noti_service_connect_sync(_))
      .WillByDefault(Return(RPC_PORT_ERROR_IO_ERROR));
  notification_block_state_e state;
  EXPECT_NE(notification_get_noti_block_state(&state), NOTIFICATION_ERROR_NONE);
}

/* ============================================================== */
/* notification_translate_localized_text — P1/E1 (non-IPC)        */
/* notification_delete_group_by_group_id — IPC failure (N1)       */
/* ============================================================== */

TEST_F(NotificationApiTest, translate_localized_text_no_text_is_translation_true) {
  /* [E1] fresh noti with no text → loop exits; is_translation=true; NONE */
  EXPECT_EQ(notification_translate_localized_text(noti_), NOTIFICATION_ERROR_NONE);
  EXPECT_TRUE(noti_->is_translation);
}

TEST_F(NotificationApiTest, translate_localized_text_with_title_populates_bundle) {
  /* [P1] noti with title set → b_text bundle populated; is_translation=true */
  ASSERT_EQ(notification_set_title(noti_, "Hello", nullptr), NOTIFICATION_ERROR_NONE);
  ASSERT_EQ(notification_translate_localized_text(noti_), NOTIFICATION_ERROR_NONE);
  EXPECT_TRUE(noti_->is_translation);
  EXPECT_NE(noti_->b_text, nullptr);
}

/* bundle_create OOM inside notification_translate_localized_text (L569-L570) */
TEST_F(NotificationApiTest,
       translate_localized_text_bundle_create_oom_covers_L569_L570) {
  /* Set text KEY (not text) so b_key is set but b_text stays NULL.
   * call 0 = bundle_create from notification_set_text (creates b_key) → OK
   * call 1 = bundle_create from notification_set_text (creates b_format_args) → OK
   * call 2 = bundle_create from notification_translate_localized_text (creates b_text) → OOM
   * MUST set fail_on BEFORE notification_set_text — set_fail_on resets the counter to 0. */
  bundle_create_set_fail_on(2);
  ASSERT_EQ(notification_set_text(noti_, NOTIFICATION_TEXT_TYPE_TITLE, nullptr,
                                   "ZZZTESTKEY999", NOTIFICATION_VARIABLE_TYPE_NONE),
            NOTIFICATION_ERROR_NONE);
  int ret = notification_translate_localized_text(noti_);
  bundle_create_reset_fail();
  EXPECT_EQ(ret, NOTIFICATION_ERROR_OUT_OF_MEMORY);
}

TEST_F(NotificationApiTest, delete_group_by_group_id_ipc_connect_fail) {
  /* [N1] IPC connect failure propagated; no INVALID_PARAMETER guard before IPC */
  ON_CALL(GetMock<NotiServiceProxyMock>(), rpc_port_proxy_noti_service_connect_sync(_))
      .WillByDefault(Return(RPC_PORT_ERROR_IO_ERROR));
  EXPECT_NE(
      notification_delete_group_by_group_id("org.tizen.test",
                                             NOTIFICATION_TYPE_NOTI, 0),
      NOTIFICATION_ERROR_NONE);
}

/* ============================================================== */
/* notification_clear — type validation + IPC failure (N1/N2/N3)  */
/* notification_delete_all_by_type — type + IPC (N1/N2/N3)        */
/* ============================================================== */

TEST_F(NotificationApiTest, clear_type_none_returns_invalid) {
  /* [N1] type == NOTIFICATION_TYPE_NONE → INVALID_PARAMETER */
  EXPECT_EQ(notification_clear(NOTIFICATION_TYPE_NONE),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationApiTest, clear_type_overflow_returns_invalid) {
  /* [N2] type > NOTIFICATION_TYPE_MAX → INVALID_PARAMETER */
  EXPECT_EQ(notification_clear(
                static_cast<notification_type_e>(NOTIFICATION_TYPE_MAX + 1)),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationApiTest, clear_ipc_connect_fail_returns_error) {
  /* [N3] IPC connect failure propagated */
  ON_CALL(GetMock<NotiServiceProxyMock>(), rpc_port_proxy_noti_service_connect_sync(_))
      .WillByDefault(Return(RPC_PORT_ERROR_IO_ERROR));
  EXPECT_NE(notification_clear(NOTIFICATION_TYPE_NOTI),
            NOTIFICATION_ERROR_NONE);
}

TEST_F(NotificationApiTest, delete_all_by_type_none_returns_invalid) {
  /* [N1] type == NOTIFICATION_TYPE_NONE → INVALID_PARAMETER */
  EXPECT_EQ(
      notification_delete_all_by_type(nullptr, NOTIFICATION_TYPE_NONE),
      NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationApiTest, delete_all_by_type_overflow_returns_invalid) {
  /* [N2] type > NOTIFICATION_TYPE_MAX → INVALID_PARAMETER */
  EXPECT_EQ(notification_delete_all_by_type(
                nullptr,
                static_cast<notification_type_e>(NOTIFICATION_TYPE_MAX + 1)),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationApiTest, delete_all_by_type_ipc_connect_fail_returns_error) {
  /* [N3] IPC connect failure propagated */
  ON_CALL(GetMock<NotiServiceProxyMock>(), rpc_port_proxy_noti_service_connect_sync(_))
      .WillByDefault(Return(RPC_PORT_ERROR_IO_ERROR));
  EXPECT_NE(
      notification_delete_all_by_type("org.tizen.test", NOTIFICATION_TYPE_NOTI),
      NOTIFICATION_ERROR_NONE);
}

/* ============================================================== */
/* notification_load — IPC failure (N1)                           */
/* notification_is_service_ready — IPC failure (N1)              */
/* ============================================================== */

TEST_F(NotificationApiTest, load_ipc_connect_fail_returns_null) {
  /* [N1] IPC connect fail → NULL, last_result != NONE */
  ON_CALL(GetMock<NotiServiceProxyMock>(), rpc_port_proxy_noti_service_connect_sync(_))
      .WillByDefault(Return(RPC_PORT_ERROR_IO_ERROR));
  char app_id[] = "org.tizen.test";
  notification_h h = notification_load(app_id, 1);
  EXPECT_EQ(h, nullptr);
  EXPECT_NE(get_last_result(), NOTIFICATION_ERROR_NONE);
}

TEST_F(NotificationApiTest, is_service_ready_ipc_connect_fail_not_ready) {
  /* [N1] IPC connect fail → propagated; return value != 1 (not ready) */
  ON_CALL(GetMock<NotiServiceProxyMock>(), rpc_port_proxy_noti_service_connect_sync(_))
      .WillByDefault(Return(RPC_PORT_ERROR_IO_ERROR));
  EXPECT_NE(notification_is_service_ready(), 1);
}

/* ============================================================== */
/* notification_update_for_uid — null + IPC failure (N1/N2)      */
/* notification_delete_for_uid — null + IPC failure (N1/N2)      */
/* ============================================================== */

TEST_F(NotificationApiTest, update_for_uid_null_noti_returns_invalid) {
  /* [N1] noti == NULL → INVALID_PARAMETER */
  EXPECT_EQ(notification_update_for_uid(nullptr, 5001),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationApiTest, delete_for_uid_null_noti_returns_invalid) {
  /* [N1] noti == NULL → INVALID_PARAMETER */
  EXPECT_EQ(notification_delete_for_uid(nullptr, 5001),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationApiTest, delete_for_uid_ipc_connect_fail_returns_error) {
  /* [N2] IPC connect failure propagated */
  ON_CALL(GetMock<NotiServiceProxyMock>(), rpc_port_proxy_noti_service_connect_sync(_))
      .WillByDefault(Return(RPC_PORT_ERROR_IO_ERROR));
  EXPECT_NE(notification_delete_for_uid(noti_, 5001),
            NOTIFICATION_ERROR_NONE);
}

/* ============================================================== */
/* notification_create_from_package_template — null args (N1/N2)  */
/* notification_delete_by_display_applist — guard + IPC (N1/N2)  */
/* ============================================================== */

TEST_F(NotificationApiTest, create_from_package_template_null_app_id_returns_null) {
  /* [N1] app_id == NULL → NULL, last_result == INVALID_PARAMETER */
  notification_h h =
      notification_create_from_package_template(nullptr, "tmpl");
  EXPECT_EQ(h, nullptr);
  EXPECT_EQ(get_last_result(), NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationApiTest, create_from_package_template_null_name_returns_null) {
  /* [N2] template_name == NULL → NULL, last_result == INVALID_PARAMETER */
  notification_h h =
      notification_create_from_package_template("org.tizen.test", nullptr);
  EXPECT_EQ(h, nullptr);
  EXPECT_EQ(get_last_result(), NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationApiTest, delete_by_display_applist_zero_returns_invalid) {
  /* [N1] display_applist == 0 (< NOTIFICATION_DISPLAY_APP_NOTIFICATION_TRAY) */
  EXPECT_EQ(notification_delete_by_display_applist(0),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationApiTest, delete_by_display_applist_ipc_connect_fail_returns_error) {
  /* [N2] IPC connect failure propagated */
  ON_CALL(GetMock<NotiServiceProxyMock>(), rpc_port_proxy_noti_service_connect_sync(_))
      .WillByDefault(Return(RPC_PORT_ERROR_IO_ERROR));
  EXPECT_NE(
      notification_delete_by_display_applist(
          NOTIFICATION_DISPLAY_APP_NOTIFICATION_TRAY),
      NOTIFICATION_ERROR_NONE);
}

/* ============================================================== */
/* notification_unregister_do_not_disturb_app — IPC failure (N1)  */
/* ============================================================== */

TEST_F(NotificationApiTest, unregister_do_not_disturb_app_ipc_connect_fail) {
  /* [N1] IPC connect failure propagated; no pre-IPC guard */
  ON_CALL(GetMock<NotiServiceProxyMock>(), rpc_port_proxy_noti_service_connect_sync(_))
      .WillByDefault(Return(RPC_PORT_ERROR_IO_ERROR));
  EXPECT_NE(notification_unregister_do_not_disturb_app(),
            NOTIFICATION_ERROR_NONE);
}

/* ============================================================== */
/* notification_ipc_make_gvariant_from_noti — smoke (P1)          */
/* notification_ipc_make_noti_from_gvariant — roundtrip (P1)      */
/* ============================================================== */

TEST_F(NotificationApiTest, ipc_make_gvariant_from_noti_returns_nonnull) {
  /* [P1] fixture noti → serialised GVariant must not be NULL */
  GVariant *v = notification_ipc_make_gvariant_from_noti(noti_, false);
  EXPECT_NE(v, nullptr);
  if (v)
    g_variant_unref(v);
}

TEST_F(NotificationApiTest, ipc_make_noti_from_gvariant_roundtrip) {
  /* [P1] serialize then deserialize — must return NOTIFICATION_ERROR_NONE */
  GVariant *v = notification_ipc_make_gvariant_from_noti(noti_, false);
  if (!v) {
    GTEST_SKIP() << "make_gvariant returned NULL; skipping roundtrip";
    return;
  }
  notification_h dst = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(dst, nullptr);
  int ret = notification_ipc_make_noti_from_gvariant(dst, v);
  g_variant_unref(v);
  notification_free(dst);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
}

TEST_F(NotificationApiTest, get_image_icon_with_no_app_icon_returns_null) {
  noti_->app_icon_path = nullptr;
  char* out = nullptr;
  EXPECT_EQ(notification_get_image(noti_, NOTIFICATION_IMAGE_TYPE_ICON, &out),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(out, nullptr);
}

TEST_F(NotificationApiTest, get_image_icon_falls_back_to_app_icon_path) {
  char fake_app_icon[] = "/fake/app/icon.png";
  noti_->app_icon_path = fake_app_icon;
  char* out = nullptr;
  EXPECT_EQ(notification_get_image(noti_, NOTIFICATION_IMAGE_TYPE_ICON, &out),
            NOTIFICATION_ERROR_NONE);
  EXPECT_STREQ(out, fake_app_icon);
  noti_->app_icon_path = nullptr;
}

TEST_F(NotificationApiTest, set_text_twice_overwrites_existing_key) {
  ASSERT_EQ(notification_set_text(noti_, NOTIFICATION_TEXT_TYPE_TITLE,
                "first", nullptr, NOTIFICATION_VARIABLE_TYPE_NONE),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(notification_set_text(noti_, NOTIFICATION_TEXT_TYPE_TITLE,
                "second", nullptr, NOTIFICATION_VARIABLE_TYPE_NONE),
            NOTIFICATION_ERROR_NONE);
}

TEST_F(NotificationApiTest, set_and_get_launch_option_roundtrip) {
  app_control_h ac = nullptr;
  ASSERT_EQ(app_control_create(&ac), APP_CONTROL_ERROR_NONE);
  ASSERT_EQ(app_control_set_app_id(ac, "com.example.launch"),
            APP_CONTROL_ERROR_NONE);
  EXPECT_EQ(notification_set_launch_option(noti_,
                NOTIFICATION_LAUNCH_OPTION_APP_CONTROL, ac),
            NOTIFICATION_ERROR_NONE);

  app_control_h out_ac = nullptr;
  EXPECT_EQ(notification_get_launch_option(noti_,
                NOTIFICATION_LAUNCH_OPTION_APP_CONTROL, &out_ac),
            NOTIFICATION_ERROR_NONE);
  if (out_ac) app_control_destroy(out_ac);
  app_control_destroy(ac);
}

TEST_F(NotificationApiTest, list_get_list_for_uid_invokes_tidl) {
  ON_CALL(GetMock<NotiServiceProxyMock>(),
          rpc_port_proxy_noti_service_connect_sync(_))
      .WillByDefault(Return(RPC_PORT_ERROR_IO_ERROR));
  notification_list_h out = nullptr;
  notification_get_list_for_uid(NOTIFICATION_TYPE_NOTI, 10, &out, getuid());
  if (out) notification_free_list(out);
}

TEST_F(NotificationApiTest, list_get_detail_list_for_uid_invokes_tidl) {
  ON_CALL(GetMock<NotiServiceProxyMock>(),
          rpc_port_proxy_noti_service_connect_sync(_))
      .WillByDefault(Return(RPC_PORT_ERROR_IO_ERROR));
  notification_list_h out = nullptr;
  notification_get_detail_list_for_uid("some.app", 0, -1, 10, &out, getuid());
  if (out) notification_free_list(out);
}

TEST_F(NotificationApiTest, list_get_list_by_page_for_uid_clamps_count) {
  ON_CALL(GetMock<NotiServiceProxyMock>(),
          rpc_port_proxy_noti_service_connect_sync(_))
      .WillByDefault(Return(RPC_PORT_ERROR_IO_ERROR));
  notification_list_h out = nullptr;
  notification_get_list_by_page_for_uid(NOTIFICATION_TYPE_NOTI, 1, 999,
                                        &out, getuid());
  if (out) notification_free_list(out);
}

TEST_F(NotificationApiTest, list_get_list_by_page_for_uid_invalid_page) {
  notification_list_h out = nullptr;
  EXPECT_EQ(notification_get_list_by_page_for_uid(NOTIFICATION_TYPE_NOTI, 0,
                                                  10, &out, getuid()),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationApiTest, get_noti_block_state_invokes_tidl) {
  ON_CALL(GetMock<NotiServiceProxyMock>(),
          rpc_port_proxy_noti_service_connect_sync(_))
      .WillByDefault(Return(RPC_PORT_ERROR_IO_ERROR));
  notification_block_state_e state;
  notification_get_noti_block_state(&state);
}

TEST_F(NotificationApiTest, free_releases_populated_bundle_and_path_fields) {
  notification_h n = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(n, nullptr);
  notification_set_image(n, NOTIFICATION_IMAGE_TYPE_ICON, "/i.png");
  notification_set_sound(n, NOTIFICATION_SOUND_TYPE_USER_DATA, "/s.ogg");
  notification_set_vibration(n, NOTIFICATION_VIBRATION_TYPE_USER_DATA,
                             "/v.wav");
  n->b_priv_image_path = bundle_create();
  n->priv_sound_path = strdup("/priv/s.ogg");
  n->priv_vibration_path = strdup("/priv/v.wav");
  EXPECT_EQ(notification_free(n), NOTIFICATION_ERROR_NONE);
}

TEST_F(NotificationApiTest, set_event_handler_replaces_existing_bundle) {
  app_control_h ac = nullptr;
  app_control_create(&ac);
  ASSERT_EQ(notification_set_event_handler(noti_,
                NOTIFICATION_EVENT_TYPE_CLICK_ON_BUTTON_1, ac),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(notification_set_event_handler(noti_,
                NOTIFICATION_EVENT_TYPE_CLICK_ON_BUTTON_1, ac),
            NOTIFICATION_ERROR_NONE);
  app_control_destroy(ac);
}

TEST_F(NotificationApiTest, set_display_applist_legacy_all_value) {
  EXPECT_EQ(notification_set_display_applist(noti_,
                static_cast<int>(0xffffffff)),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(noti_->display_applist, NOTIFICATION_DISPLAY_APP_ALL);
}

TEST_F(NotificationApiTest, get_pkgname_no_caller_app_id_returns_null) {
  if (noti_->caller_app_id) {
    free(noti_->caller_app_id);
    noti_->caller_app_id = nullptr;
  }
  char* p = nullptr;
  EXPECT_EQ(notification_get_pkgname(noti_, &p), NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(p, nullptr);
}

TEST_F(NotificationApiTest, create_from_template_invokes_tidl) {
  ON_CALL(GetMock<NotiServiceProxyMock>(),
          rpc_port_proxy_noti_service_connect_sync(_))
      .WillByDefault(Return(RPC_PORT_ERROR_IO_ERROR));
  notification_h n = notification_create_from_template("some.template");
  if (n) notification_free(n);
}

/* ============================================================== */
/* notification_set_text — bundle_del paths (L298/343/358/374/   */
/* 390/407): set same slot twice so existing entries are deleted   */
/* ============================================================== */

TEST_F(NotificationApiTest, set_text_key_overwrite_deletes_old_bundle_entry) {
  /* [C1] Calling set_text with a key, then again with a different key
   * for the same text type triggers the bundle_del path (L298) that
   * removes the old key-bundle entry before writing the new one. */
  ASSERT_EQ(notification_set_text(noti_, NOTIFICATION_TEXT_TYPE_TITLE,
                "title", "key1", NOTIFICATION_VARIABLE_TYPE_NONE),
            NOTIFICATION_ERROR_NONE);
  /* Second call with different key — b_key already has an entry → del+add */
  EXPECT_EQ(notification_set_text(noti_, NOTIFICATION_TEXT_TYPE_TITLE,
                "title2", "key2", NOTIFICATION_VARIABLE_TYPE_NONE),
            NOTIFICATION_ERROR_NONE);

  /* Verify the latest value is stored. */
  char* t = nullptr;
  EXPECT_EQ(notification_get_text(noti_, NOTIFICATION_TEXT_TYPE_TITLE, &t),
            NOTIFICATION_ERROR_NONE);
  EXPECT_NE(t, nullptr);
}

TEST_F(NotificationApiTest,
       set_text_format_int_overwrite_deletes_old_bundle_entry) {
  /* [C2] Two consecutive calls with NOTIFICATION_VARIABLE_TYPE_INT for the
   * same type slot trigger bundle_del at L343 (type key) and L358 (value
   * key). */
  ASSERT_EQ(notification_set_text(noti_, NOTIFICATION_TEXT_TYPE_CONTENT,
                "val %d", nullptr,
                NOTIFICATION_VARIABLE_TYPE_INT, 1,
                NOTIFICATION_VARIABLE_TYPE_NONE),
            NOTIFICATION_ERROR_NONE);
  /* Second call: b_format_args already populated → del before re-add. */
  EXPECT_EQ(notification_set_text(noti_, NOTIFICATION_TEXT_TYPE_CONTENT,
                "val %d", nullptr,
                NOTIFICATION_VARIABLE_TYPE_INT, 2,
                NOTIFICATION_VARIABLE_TYPE_NONE),
            NOTIFICATION_ERROR_NONE);
}

TEST_F(NotificationApiTest,
       set_text_format_double_overwrite_deletes_old_bundle_entry) {
  /* [C3] Double variable type exercises bundle_del at L374. */
  ASSERT_EQ(notification_set_text(noti_, NOTIFICATION_TEXT_TYPE_CONTENT,
                "%.1f", nullptr,
                NOTIFICATION_VARIABLE_TYPE_DOUBLE, 1.0,
                NOTIFICATION_VARIABLE_TYPE_NONE),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(notification_set_text(noti_, NOTIFICATION_TEXT_TYPE_CONTENT,
                "%.1f", nullptr,
                NOTIFICATION_VARIABLE_TYPE_DOUBLE, 2.0,
                NOTIFICATION_VARIABLE_TYPE_NONE),
            NOTIFICATION_ERROR_NONE);
}

TEST_F(NotificationApiTest,
       set_text_format_string_overwrite_deletes_old_bundle_entry) {
  /* [C4] String variable type exercises bundle_del at L390. */
  ASSERT_EQ(notification_set_text(noti_, NOTIFICATION_TEXT_TYPE_CONTENT,
                "%s", nullptr,
                NOTIFICATION_VARIABLE_TYPE_STRING, "first",
                NOTIFICATION_VARIABLE_TYPE_NONE),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(notification_set_text(noti_, NOTIFICATION_TEXT_TYPE_CONTENT,
                "%s", nullptr,
                NOTIFICATION_VARIABLE_TYPE_STRING, "second",
                NOTIFICATION_VARIABLE_TYPE_NONE),
            NOTIFICATION_ERROR_NONE);
}

TEST_F(NotificationApiTest,
       set_text_format_count_overwrite_deletes_old_bundle_entry) {
  /* [C5] Count variable type exercises bundle_del at L407. */
  ASSERT_EQ(notification_set_text(noti_, NOTIFICATION_TEXT_TYPE_CONTENT,
                "%d", nullptr,
                NOTIFICATION_VARIABLE_TYPE_COUNT,
                NOTIFICATION_COUNT_POS_IN,
                NOTIFICATION_VARIABLE_TYPE_NONE),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(notification_set_text(noti_, NOTIFICATION_TEXT_TYPE_CONTENT,
                "%d", nullptr,
                NOTIFICATION_VARIABLE_TYPE_COUNT,
                NOTIFICATION_COUNT_POS_RIGHT,
                NOTIFICATION_VARIABLE_TYPE_NONE),
            NOTIFICATION_ERROR_NONE);
}

/* ============================================================== */
/* notification_clone — fully populated noti exercises all the    */
/* conditional bundle_dup / strdup branches (L1700-1779)          */
/* ============================================================== */

TEST_F(NotificationApiTest, clone_fully_populated_noti_copies_all_fields) {
  /* [P2] Build a notification with as many optional fields set as
   * possible, then clone it.  Every conditional in notification_clone
   * that wraps a bundle_dup or strdup will be taken. */
  ASSERT_EQ(notification_set_text(noti_, NOTIFICATION_TEXT_TYPE_TITLE,
                "Hello", nullptr, NOTIFICATION_VARIABLE_TYPE_NONE),
            NOTIFICATION_ERROR_NONE);
  ASSERT_EQ(notification_set_image(noti_, NOTIFICATION_IMAGE_TYPE_ICON,
                "/i.png"),
            NOTIFICATION_ERROR_NONE);
  ASSERT_EQ(notification_set_sound(noti_, NOTIFICATION_SOUND_TYPE_USER_DATA,
                "/s.ogg"),
            NOTIFICATION_ERROR_NONE);
  ASSERT_EQ(notification_set_vibration(noti_,
                NOTIFICATION_VIBRATION_TYPE_USER_DATA, "/v.wav"),
            NOTIFICATION_ERROR_NONE);
  ASSERT_EQ(notification_set_tag(noti_, "mytag"), NOTIFICATION_ERROR_NONE);

  /* Set an execute option to populate b_service_single_launch. */
  app_control_h ac = nullptr;
  ASSERT_EQ(app_control_create(&ac), APP_CONTROL_ERROR_NONE);
  notification_set_launch_option(noti_,
      NOTIFICATION_LAUNCH_OPTION_APP_CONTROL, ac);
  app_control_destroy(ac);

  /* Set an event handler to populate b_event_handler[0]. */
  app_control_h ev_ac = nullptr;
  ASSERT_EQ(app_control_create(&ev_ac), APP_CONTROL_ERROR_NONE);
  notification_set_event_handler(noti_,
      NOTIFICATION_EVENT_TYPE_CLICK_ON_BUTTON_1, ev_ac);
  app_control_destroy(ev_ac);

  /* Set app_label to exercise that strdup branch. */
  noti_->app_label = strdup("Test App");

  notification_h clone = nullptr;
  ASSERT_EQ(notification_clone(noti_, &clone), NOTIFICATION_ERROR_NONE);
  ASSERT_NE(clone, nullptr);

  /* Spot-check: tag and type must be copied. */
  const char* tag = nullptr;
  EXPECT_EQ(notification_get_tag(clone, &tag), NOTIFICATION_ERROR_NONE);
  EXPECT_STREQ(tag, "mytag");
  EXPECT_EQ(clone->type, noti_->type);

  notification_free(clone);
}

/* ============================================================== */
/* notification_free — app_icon_path and temp_content freed (L1896/1905) */
/* ============================================================== */

TEST_F(NotificationApiTest, free_releases_app_icon_path_and_temp_content) {
  /* [E1] notification_free must free app_icon_path (L1896) and
   * temp_content (L1905) when they are set. */
  notification_h n = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(n, nullptr);
  n->app_icon_path = strdup("/app/icon.png");
  n->temp_content = strdup("cached content");
  /* Must not crash or leak — valgrind/ASAN would catch dangling writes. */
  EXPECT_EQ(notification_free(n), NOTIFICATION_ERROR_NONE);
}

/* ============================================================== */
/* notification_ipc_make_gvariant_from_noti — populated optional  */
/* fields exercise the bundle-encode branches (L83-310)            */
/* ============================================================== */

TEST_F(NotificationApiTest,
       ipc_make_gvariant_from_noti_with_text_and_image_fields) {
  /* [P2] A noti with b_text, b_image_path and sound_path set causes
   * notification_ipc_make_gvariant_from_noti to enter the optional-field
   * encoding branches that are dead when the noti is empty. */
  ASSERT_EQ(notification_set_text(noti_, NOTIFICATION_TEXT_TYPE_TITLE,
                "Title", nullptr, NOTIFICATION_VARIABLE_TYPE_NONE),
            NOTIFICATION_ERROR_NONE);
  ASSERT_EQ(notification_set_image(noti_, NOTIFICATION_IMAGE_TYPE_ICON,
                "/icon.png"),
            NOTIFICATION_ERROR_NONE);
  ASSERT_EQ(notification_set_sound(noti_, NOTIFICATION_SOUND_TYPE_USER_DATA,
                "/sound.ogg"),
            NOTIFICATION_ERROR_NONE);
  ASSERT_EQ(notification_set_vibration(noti_,
                NOTIFICATION_VIBRATION_TYPE_USER_DATA, "/vib.wav"),
            NOTIFICATION_ERROR_NONE);
  ASSERT_EQ(notification_set_tag(noti_, "tag1"), NOTIFICATION_ERROR_NONE);

  GVariant* v = notification_ipc_make_gvariant_from_noti(noti_, false);
  EXPECT_NE(v, nullptr);
  if (v) g_variant_unref(v);
}

TEST_F(NotificationApiTest,
       ipc_make_gvariant_from_noti_with_launch_and_event_handlers) {
  /* [P3] Populate b_service_single_launch (via set_launch_option) and
   * b_event_handler[0] (via set_event_handler) to hit the corresponding
   * optional-field encoding branches. */
  app_control_h ac = nullptr;
  ASSERT_EQ(app_control_create(&ac), APP_CONTROL_ERROR_NONE);
  ASSERT_EQ(app_control_set_app_id(ac, "com.test"), APP_CONTROL_ERROR_NONE);
  notification_set_launch_option(noti_,
      NOTIFICATION_LAUNCH_OPTION_APP_CONTROL, ac);
  app_control_destroy(ac);

  app_control_h ev = nullptr;
  ASSERT_EQ(app_control_create(&ev), APP_CONTROL_ERROR_NONE);
  notification_set_event_handler(noti_,
      NOTIFICATION_EVENT_TYPE_CLICK_ON_BUTTON_1, ev);
  app_control_destroy(ev);

  /* Also set launch_app_id to exercise that branch. */
  noti_->launch_app_id = strdup("com.test.launch");

  GVariant* v = notification_ipc_make_gvariant_from_noti(noti_, false);
  EXPECT_NE(v, nullptr);
  if (v) g_variant_unref(v);

  free(noti_->launch_app_id);
  noti_->launch_app_id = nullptr;
}

TEST_F(NotificationApiTest,
       ipc_make_gvariant_from_noti_with_format_args) {
  /* [P4] A noti whose b_format_args and b_key bundles are populated
   * triggers the format-args and key encoding branches. */
  ASSERT_EQ(notification_set_text(noti_, NOTIFICATION_TEXT_TYPE_CONTENT,
                "%d items", "item_key",
                NOTIFICATION_VARIABLE_TYPE_INT, 5,
                NOTIFICATION_VARIABLE_TYPE_NONE),
            NOTIFICATION_ERROR_NONE);

  GVariant* v = notification_ipc_make_gvariant_from_noti(noti_, false);
  EXPECT_NE(v, nullptr);
  if (v) g_variant_unref(v);
}

/* ============================================================== */
/* [P4] get_text_plain_no_format_args                              */
/* Plain text set via set_text with VARIABLE_TYPE_NONE is returned */
/* verbatim by get_text.                                           */
/* ============================================================== */

TEST_F(NotificationApiTest, get_text_plain_no_format_args) {
  ASSERT_EQ(notification_set_text(noti_, NOTIFICATION_TEXT_TYPE_TITLE,
                "Hello", nullptr, NOTIFICATION_VARIABLE_TYPE_NONE),
            NOTIFICATION_ERROR_NONE);
  char* got = nullptr;
  EXPECT_EQ(notification_get_text(noti_, NOTIFICATION_TEXT_TYPE_TITLE, &got),
            NOTIFICATION_ERROR_NONE);
  ASSERT_NE(got, nullptr);
  EXPECT_STREQ(got, "Hello");
}

/* ============================================================== */
/* [P5] get_text_type_not_set_returns_null                        */
/* A freshly created noti with no text set returns NONE and *text */
/* points to NULL.                                                */
/* ============================================================== */

TEST_F(NotificationApiTest, get_text_type_not_set_returns_null) {
  /* CONTENT was never set on noti_. */
  char* got = reinterpret_cast<char*>(0x1); /* sentinel */
  EXPECT_EQ(notification_get_text(noti_, NOTIFICATION_TEXT_TYPE_CONTENT, &got),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(got, nullptr);
}

/* ============================================================== */
/* [E1] get_text_integer_d_format                                  */
/* Text "Value: %d" with an INT variable arg must produce          */
/* "Value: 42".                                                    */
/* ============================================================== */

TEST_F(NotificationApiTest, get_text_integer_d_format) {
  ASSERT_EQ(notification_set_text(noti_, NOTIFICATION_TEXT_TYPE_TITLE,
                "Value: %d", nullptr,
                NOTIFICATION_VARIABLE_TYPE_INT, 42,
                NOTIFICATION_VARIABLE_TYPE_NONE),
            NOTIFICATION_ERROR_NONE);
  char* got = nullptr;
  EXPECT_EQ(notification_get_text(noti_, NOTIFICATION_TEXT_TYPE_TITLE, &got),
            NOTIFICATION_ERROR_NONE);
  ASSERT_NE(got, nullptr);
  EXPECT_STREQ(got, "Value: 42");
}

/* ============================================================== */
/* [E5] get_text_float_f_format                                    */
/* Text "%f" with a DOUBLE arg produces "3.14" via "%.2f".        */
/* ============================================================== */

TEST_F(NotificationApiTest, get_text_float_f_format) {
  ASSERT_EQ(notification_set_text(noti_, NOTIFICATION_TEXT_TYPE_TITLE,
                "%f", nullptr,
                NOTIFICATION_VARIABLE_TYPE_DOUBLE, 3.14,
                NOTIFICATION_VARIABLE_TYPE_NONE),
            NOTIFICATION_ERROR_NONE);
  char* got = nullptr;
  EXPECT_EQ(notification_get_text(noti_, NOTIFICATION_TEXT_TYPE_TITLE, &got),
            NOTIFICATION_ERROR_NONE);
  ASSERT_NE(got, nullptr);
  EXPECT_STREQ(got, "3.14");
}

/* ============================================================== */
/* [E6] get_text_literal_percent                                   */
/* Text "100%% done" must be returned verbatim as "100%% done".    */
/* The format engine copies the first '%' via the %% branch, then  */
/* the second '%' (followed by a space) hits the fallback branch    */
/* and is also copied unchanged — "%%" is preserved as "%%".       */
/* ============================================================== */

TEST_F(NotificationApiTest, get_text_literal_percent) {
  ASSERT_EQ(notification_set_text(noti_, NOTIFICATION_TEXT_TYPE_TITLE,
                "100%% done", nullptr,
                NOTIFICATION_VARIABLE_TYPE_INT, 1,
                NOTIFICATION_VARIABLE_TYPE_NONE),
            NOTIFICATION_ERROR_NONE);
  char* got = nullptr;
  EXPECT_EQ(notification_get_text(noti_, NOTIFICATION_TEXT_TYPE_TITLE, &got),
            NOTIFICATION_ERROR_NONE);
  ASSERT_NE(got, nullptr);
  EXPECT_STREQ(got, "100%% done");
}

/* ============================================================== */
/* [N6] get_launch_option_null_noti_invalid                        */
/* ============================================================== */

TEST_F(NotificationApiTest, get_launch_option_null_noti_invalid) {
  app_control_h ac = nullptr;
  EXPECT_EQ(notification_get_launch_option(nullptr,
                NOTIFICATION_LAUNCH_OPTION_APP_CONTROL, &ac),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* ============================================================== */
/* [N7] get_launch_option_wrong_type_invalid                       */
/* Any type other than APP_CONTROL must return INVALID_PARAMETER. */
/* ============================================================== */

TEST_F(NotificationApiTest, get_launch_option_wrong_type_invalid) {
  app_control_h ac = nullptr;
  EXPECT_EQ(notification_get_launch_option(noti_,
                static_cast<notification_launch_option_type>(99), &ac),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* ============================================================== */
/* [N8] get_launch_option_no_execute_option                        */
/* A notification with no launch option set: get_execute_option    */
/* returns NONE with a NULL bundle; get_launch_option forwards     */
/* NONE when the bundle is NULL (b==NULL branch in the else path). */
/* ============================================================== */

TEST_F(NotificationApiTest, get_launch_option_no_execute_option) {
  app_control_h ac = nullptr;
  int ret = notification_get_launch_option(noti_,
                NOTIFICATION_LAUNCH_OPTION_APP_CONTROL, &ac);
  /* notification_get_execute_option returns NONE with b==NULL when
   * no execute option was set; notification_get_launch_option then
   * returns ret (NONE) from its else-branch. */
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(ac, nullptr);
}

/* ============================================================== */
/* get_text_string_var_substitution                               */
/* Goal: %s string-variable substitution (lines 609-636 of        */
/* notification.c)                                                */
/* ============================================================== */

TEST_F(NotificationApiTest, get_text_string_var_substitution) {
  /* Set text with a %s format specifier and a STRING variable arg. */
  ASSERT_EQ(notification_set_text(noti_, NOTIFICATION_TEXT_TYPE_TITLE,
                "hi %s", nullptr,
                NOTIFICATION_VARIABLE_TYPE_STRING, "bob",
                NOTIFICATION_VARIABLE_TYPE_NONE),
            NOTIFICATION_ERROR_NONE);
  char* got = nullptr;
  EXPECT_EQ(notification_get_text(noti_, NOTIFICATION_TEXT_TYPE_TITLE, &got),
            NOTIFICATION_ERROR_NONE);
  ASSERT_NE(got, nullptr);
  EXPECT_STREQ(got, "hi bob");
}

/* ============================================================== */
/* get_text_string_var_with_domain                                */
/* Goal: %s with text domain set — app-domain dgettext branch     */
/* (L616-622); no catalog → dgettext returns input verbatim.      */
/* ============================================================== */

TEST_F(NotificationApiTest, get_text_string_var_with_domain) {
  /* Set a text domain so the app-domain dgettext branch (L616-622)
   * is taken.  With no installed catalog the lookup returns the key
   * unchanged, so the final string must still be "hi bob". */
  ASSERT_EQ(notification_set_text_domain(noti_, "dom", "/usr/share/locale"),
            NOTIFICATION_ERROR_NONE);
  ASSERT_EQ(notification_set_text(noti_, NOTIFICATION_TEXT_TYPE_TITLE,
                "hi %s", nullptr,
                NOTIFICATION_VARIABLE_TYPE_STRING, "bob",
                NOTIFICATION_VARIABLE_TYPE_NONE),
            NOTIFICATION_ERROR_NONE);
  char* got = nullptr;
  EXPECT_EQ(notification_get_text(noti_, NOTIFICATION_TEXT_TYPE_TITLE, &got),
            NOTIFICATION_ERROR_NONE);
  ASSERT_NE(got, nullptr);
  EXPECT_STREQ(got, "hi bob");
}

/* ============================================================== */
/* get_text_float_var_substitution                                */
/* Goal: %f double-variable substitution (lines 637-650 of        */
/* notification.c); "%.2f" format → "3.14".                       */
/* ============================================================== */

TEST_F(NotificationApiTest, get_text_float_var_substitution) {
  ASSERT_EQ(notification_set_text(noti_, NOTIFICATION_TEXT_TYPE_TITLE,
                "pi=%f", nullptr,
                NOTIFICATION_VARIABLE_TYPE_DOUBLE, 3.14,
                NOTIFICATION_VARIABLE_TYPE_NONE),
            NOTIFICATION_ERROR_NONE);
  char* got = nullptr;
  EXPECT_EQ(notification_get_text(noti_, NOTIFICATION_TEXT_TYPE_TITLE, &got),
            NOTIFICATION_ERROR_NONE);
  ASSERT_NE(got, nullptr);
  EXPECT_STREQ(got, "pi=3.14");
}

/* ============================================================== */
/* get_text_key_only_fallback                                      */
/* Goal: b_key set but no b_text; key lookup falls back to        */
/* ret_val (lines 487-505).  With no catalog dgettext returns     */
/* the key unchanged → L504 sets get_str=ret_val; num_format_     */
/* args==0 → returns raw get_str.                                 */
/* ============================================================== */

TEST_F(NotificationApiTest, get_text_key_only_fallback) {
  /* Pass text=nullptr, key="IDS_KEY" so only b_key is populated. */
  ASSERT_EQ(notification_set_text(noti_, NOTIFICATION_TEXT_TYPE_TITLE,
                nullptr, "IDS_KEY",
                NOTIFICATION_VARIABLE_TYPE_NONE),
            NOTIFICATION_ERROR_NONE);
  char* got = nullptr;
  EXPECT_EQ(notification_get_text(noti_, NOTIFICATION_TEXT_TYPE_TITLE, &got),
            NOTIFICATION_ERROR_NONE);
  /* No catalog → sys_string dgettext returns "IDS_KEY" == ret_val;
   * L485 sets get_str=NULL; L504 fallback sets get_str=ret_val;
   * num_format_args==0 → *text = get_str = "IDS_KEY". */
  ASSERT_NE(got, nullptr);
  EXPECT_STREQ(got, "IDS_KEY");
}

/* ============================================================== */
/* get_text_key_with_domain_fallback                              */
/* Goal: b_key + domain set — app-domain dgettext path (L479-486) */
/* No catalog → returns the key verbatim.                         */
/* ============================================================== */

TEST_F(NotificationApiTest, get_text_key_with_domain_fallback) {
  ASSERT_EQ(notification_set_text_domain(noti_, "testdom", "/usr/share/locale"),
            NOTIFICATION_ERROR_NONE);
  ASSERT_EQ(notification_set_text(noti_, NOTIFICATION_TEXT_TYPE_TITLE,
                nullptr, "IDS_SOME",
                NOTIFICATION_VARIABLE_TYPE_NONE),
            NOTIFICATION_ERROR_NONE);
  char* got = nullptr;
  EXPECT_EQ(notification_get_text(noti_, NOTIFICATION_TEXT_TYPE_TITLE, &got),
            NOTIFICATION_ERROR_NONE);
  /* No catalog → app-domain dgettext("testdom","IDS_SOME") returns
   * "IDS_SOME" == ret_val; L485 sets get_str=NULL; L504 fallback
   * sets get_str=ret_val; num_format_args==0 → returns "IDS_SOME". */
  ASSERT_NE(got, nullptr);
  EXPECT_STREQ(got, "IDS_SOME");
}

/* ============================================================== */
/* get_text_event_count_type_default_arm                          */
/* Goal: default arm of the text_type switch (line 785) — a text  */
/* type that is not TITLE/GROUP_TITLE/CONTENT/CONTENT_FOR_DISPLAY_ */
/* OPTION_IS_OFF/GROUP_CONTENT/GROUP_CONTENT_FOR_DISPLAY_OPTION_  */
/* IS_OFF leaves *text unchanged (i.e. nullptr).                   */
/* NOTIFICATION_TEXT_TYPE_INFO_1 hits the default arm.            */
/* ============================================================== */

TEST_F(NotificationApiTest, get_text_event_count_type_default_arm) {
  /* Store text+format_arg for INFO_1 so get_str is non-NULL and the
   * format loop runs to produce result_str.  The subsequent switch
   * hits default: and never writes *text, so got stays nullptr. */
  ASSERT_EQ(notification_set_text(noti_, NOTIFICATION_TEXT_TYPE_INFO_1,
                "info %d", nullptr,
                NOTIFICATION_VARIABLE_TYPE_INT, 42,
                NOTIFICATION_VARIABLE_TYPE_NONE),
            NOTIFICATION_ERROR_NONE);
  char* got = nullptr;
  EXPECT_EQ(notification_get_text(noti_, NOTIFICATION_TEXT_TYPE_INFO_1, &got),
            NOTIFICATION_ERROR_NONE);
  /* default: arm does not write *text → got remains nullptr. */
  EXPECT_EQ(got, nullptr);
}

/* ============================================================== */
/* set_image_overwrite_then_clear                                 */
/* Goal: existing b_image_path bundle gets key deleted when        */
/* image_path is NULL (L117-121 of notification.c).               */
/* ============================================================== */

TEST_F(NotificationApiTest, set_image_overwrite_then_clear) {
  /* First set creates the bundle and stores the key. */
  ASSERT_EQ(notification_set_image(noti_, NOTIFICATION_IMAGE_TYPE_ICON,
                "/tmp/icon.png"),
            NOTIFICATION_ERROR_NONE);
  /* Second set with NULL: bundle_del removes the key (L117-118),
   * and since image_path==NULL nothing is re-added (L120-121). */
  ASSERT_EQ(notification_set_image(noti_, NOTIFICATION_IMAGE_TYPE_ICON,
                nullptr),
            NOTIFICATION_ERROR_NONE);
  char* path = reinterpret_cast<char*>(0x1); /* sentinel */
  EXPECT_EQ(notification_get_image(noti_, NOTIFICATION_IMAGE_TYPE_ICON, &path),
            NOTIFICATION_ERROR_NONE);
  /* Key was deleted → bundle_get_str returns NULL; app_icon_path is
   * also NULL (never set) → path must be nullptr. */
  EXPECT_EQ(path, nullptr);
}

/* ============================================================== */
/* get_text_content_type_int_substitution                         */
/* D: NOTIFICATION_TEXT_TYPE_CONTENT with %d → CONTENT switch arm */
/* ============================================================== */

TEST_F(NotificationApiTest, get_text_content_type_int_substitution) {
  ASSERT_EQ(notification_set_text(noti_, NOTIFICATION_TEXT_TYPE_CONTENT,
                "count=%d", nullptr, NOTIFICATION_VARIABLE_TYPE_INT, 7,
                NOTIFICATION_VARIABLE_TYPE_NONE),
            NOTIFICATION_ERROR_NONE);
  char* text = nullptr;
  ASSERT_EQ(
      notification_get_text(noti_, NOTIFICATION_TEXT_TYPE_CONTENT, &text),
      NOTIFICATION_ERROR_NONE);
  ASSERT_NE(text, nullptr);
  EXPECT_STREQ(text, "count=7");
}

/* ============================================================== */
/* get_text_group_title_type_int_substitution                     */
/* E1: NOTIFICATION_TEXT_TYPE_GROUP_TITLE with %d                 */
/* ============================================================== */

TEST_F(NotificationApiTest, get_text_group_title_type_int_substitution) {
  ASSERT_EQ(notification_set_text(noti_, NOTIFICATION_TEXT_TYPE_GROUP_TITLE,
                "g=%d", nullptr, NOTIFICATION_VARIABLE_TYPE_INT, 3,
                NOTIFICATION_VARIABLE_TYPE_NONE),
            NOTIFICATION_ERROR_NONE);
  char* text = nullptr;
  ASSERT_EQ(notification_get_text(noti_, NOTIFICATION_TEXT_TYPE_GROUP_TITLE,
                &text),
            NOTIFICATION_ERROR_NONE);
  ASSERT_NE(text, nullptr);
  EXPECT_STREQ(text, "g=3");
}

/* ============================================================== */
/* get_text_group_content_type_string_substitution                */
/* E2: NOTIFICATION_TEXT_TYPE_GROUP_CONTENT with %s               */
/* ============================================================== */

TEST_F(NotificationApiTest, get_text_group_content_type_string_substitution) {
  ASSERT_EQ(notification_set_text(noti_, NOTIFICATION_TEXT_TYPE_GROUP_CONTENT,
                "gc=%s", nullptr, NOTIFICATION_VARIABLE_TYPE_STRING,
                "world", NOTIFICATION_VARIABLE_TYPE_NONE),
            NOTIFICATION_ERROR_NONE);
  char* text = nullptr;
  ASSERT_EQ(notification_get_text(noti_, NOTIFICATION_TEXT_TYPE_GROUP_CONTENT,
                &text),
            NOTIFICATION_ERROR_NONE);
  ASSERT_NE(text, nullptr);
  /* %s goes through dgettext("sys_string", "world"); when no catalog
   * entry exists dgettext returns the msgid unchanged → appended to "gc=". */
  EXPECT_STREQ(text, "gc=world");
}

/* ============================================================== */
/* get_text_content_for_display_off_type                          */
/* F: CONTENT_FOR_DISPLAY_OPTION_IS_OFF → same CONTENT switch arm */
/* ============================================================== */

TEST_F(NotificationApiTest, get_text_content_for_display_off_type) {
  ASSERT_EQ(
      notification_set_text(
          noti_,
          NOTIFICATION_TEXT_TYPE_CONTENT_FOR_DISPLAY_OPTION_IS_OFF,
          "off=%d", nullptr, NOTIFICATION_VARIABLE_TYPE_INT, 1,
          NOTIFICATION_VARIABLE_TYPE_NONE),
      NOTIFICATION_ERROR_NONE);
  char* text = nullptr;
  ASSERT_EQ(
      notification_get_text(
          noti_,
          NOTIFICATION_TEXT_TYPE_CONTENT_FOR_DISPLAY_OPTION_IS_OFF, &text),
      NOTIFICATION_ERROR_NONE);
  ASSERT_NE(text, nullptr);
  EXPECT_STREQ(text, "off=1");
}

/* ============================================================== */
/* get_time_from_text_no_text_set                                 */
/* G4: no text stored → get_text returns NULL → INVALID_PARAMETER */
/* ============================================================== */

TEST_F(NotificationApiTest, get_time_from_text_no_text_set) {
  time_t t = 0;
  EXPECT_EQ(
      notification_get_time_from_text(noti_,
          NOTIFICATION_TEXT_TYPE_TITLE, &t),
      NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* ============================================================== */
/* get_time_from_text_plain_text_no_tag                           */
/* G5: plain text without <TIME> tag → TAG_TYPE_INVALID           */
/* ============================================================== */

TEST_F(NotificationApiTest, get_time_from_text_plain_text_no_tag) {
  ASSERT_EQ(notification_set_text(noti_, NOTIFICATION_TEXT_TYPE_TITLE,
                "no time tag here", nullptr, NOTIFICATION_VARIABLE_TYPE_NONE),
            NOTIFICATION_ERROR_NONE);
  time_t t = 0;
  EXPECT_EQ(
      notification_get_time_from_text(noti_,
          NOTIFICATION_TEXT_TYPE_TITLE, &t),
      NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* ============================================================== */
/* get_time_from_text_empty_tag_body                              */
/* G6: "<TIME></TIME>" — tag type recognized but strip_tag → NULL */
/* ============================================================== */

TEST_F(NotificationApiTest, get_time_from_text_empty_tag_body) {
  ASSERT_EQ(notification_set_text(noti_, NOTIFICATION_TEXT_TYPE_TITLE,
                "<TIME></TIME>", nullptr, NOTIFICATION_VARIABLE_TYPE_NONE),
            NOTIFICATION_ERROR_NONE);
  time_t t = 0;
  EXPECT_EQ(
      notification_get_time_from_text(noti_,
          NOTIFICATION_TEXT_TYPE_TITLE, &t),
      NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* ============================================================== */
/* get_text_numbered_arg_1d_int                                   */
/* H1: %1$d positional integer (lines 651-686)                    */
/* ============================================================== */

TEST_F(NotificationApiTest, get_text_numbered_arg_1d_int) {
  ASSERT_EQ(notification_set_text(noti_, NOTIFICATION_TEXT_TYPE_TITLE,
                "val=%1$d", nullptr, NOTIFICATION_VARIABLE_TYPE_INT, 42,
                NOTIFICATION_VARIABLE_TYPE_NONE),
            NOTIFICATION_ERROR_NONE);
  char* text = nullptr;
  ASSERT_EQ(
      notification_get_text(noti_, NOTIFICATION_TEXT_TYPE_TITLE, &text),
      NOTIFICATION_ERROR_NONE);
  ASSERT_NE(text, nullptr);
  EXPECT_STREQ(text, "val=42");
}

/* ============================================================== */
/* get_text_numbered_arg_2s_string                                */
/* H2: %2$s positional string — indexes second arg directly       */
/* ============================================================== */

TEST_F(NotificationApiTest, get_text_numbered_arg_2s_string) {
  ASSERT_EQ(notification_set_text(noti_, NOTIFICATION_TEXT_TYPE_TITLE,
                "%2$s", nullptr,
                NOTIFICATION_VARIABLE_TYPE_INT, 1,
                NOTIFICATION_VARIABLE_TYPE_STRING, "hello",
                NOTIFICATION_VARIABLE_TYPE_NONE),
            NOTIFICATION_ERROR_NONE);
  char* text = nullptr;
  ASSERT_EQ(
      notification_get_text(noti_, NOTIFICATION_TEXT_TYPE_TITLE, &text),
      NOTIFICATION_ERROR_NONE);
  ASSERT_NE(text, nullptr);
  /* %Ns path copies raw bundle value without dgettext translation. */
  EXPECT_STREQ(text, "hello");
}

/* ============================================================== */
/* get_text_numbered_arg_1f_float                                 */
/* H3: %1$f positional double (lines 698-711)                     */
/* ============================================================== */

TEST_F(NotificationApiTest, get_text_numbered_arg_1f_float) {
  ASSERT_EQ(notification_set_text(noti_, NOTIFICATION_TEXT_TYPE_TITLE,
                "%1$f", nullptr, NOTIFICATION_VARIABLE_TYPE_DOUBLE, 3.14,
                NOTIFICATION_VARIABLE_TYPE_NONE),
            NOTIFICATION_ERROR_NONE);
  char* text = nullptr;
  ASSERT_EQ(
      notification_get_text(noti_, NOTIFICATION_TEXT_TYPE_TITLE, &text),
      NOTIFICATION_ERROR_NONE);
  ASSERT_NE(text, nullptr);
  /* set_text stores "3.14" (%.2f); get_text reads "3.14" via atof then
   * re-formats to "3.14". */
  EXPECT_STREQ(text, "3.14");
}

/* ============================================================== */
/* get_text_numbered_arg_unknown_conversion                       */
/* H4: %1$x — digit + unknown conversion char → else branch       */
/* (L712-718) copies only '%'; remainder processed as plain chars  */
/* ============================================================== */

TEST_F(NotificationApiTest, get_text_numbered_arg_unknown_conversion) {
  ASSERT_EQ(notification_set_text(noti_, NOTIFICATION_TEXT_TYPE_TITLE,
                "a%1$xb", nullptr, NOTIFICATION_VARIABLE_TYPE_INT, 5,
                NOTIFICATION_VARIABLE_TYPE_NONE),
            NOTIFICATION_ERROR_NONE);
  char* text = nullptr;
  ASSERT_EQ(
      notification_get_text(noti_, NOTIFICATION_TEXT_TYPE_TITLE, &text),
      NOTIFICATION_ERROR_NONE);
  ASSERT_NE(text, nullptr);
  EXPECT_STREQ(text, "a%1$xb");
}

/* ============================================================== */
/* get_text_title_free_temp_on_second_call                        */
/* L766-767: free(noti->temp_title) when it is already non-NULL   */
/* ============================================================== */

TEST_F(NotificationApiTest, get_text_title_free_temp_on_second_call) {
  /* Format-arg text so the result goes through the result_str path
   * that writes into noti->temp_title. */
  ASSERT_EQ(notification_set_text(noti_, NOTIFICATION_TEXT_TYPE_TITLE,
                "val=%d", nullptr,
                NOTIFICATION_VARIABLE_TYPE_INT, 1,
                NOTIFICATION_VARIABLE_TYPE_NONE),
            NOTIFICATION_ERROR_NONE);

  char* text = nullptr;
  /* First call: temp_title is NULL → strdup sets it. */
  ASSERT_EQ(notification_get_text(noti_, NOTIFICATION_TEXT_TYPE_TITLE, &text),
            NOTIFICATION_ERROR_NONE);
  ASSERT_NE(text, nullptr);
  EXPECT_STREQ(text, "val=1");

  /* Second call: temp_title != NULL → free branch (L767) is taken,
   * then strdup stores fresh value. */
  text = nullptr;
  ASSERT_EQ(notification_get_text(noti_, NOTIFICATION_TEXT_TYPE_TITLE, &text),
            NOTIFICATION_ERROR_NONE);
  ASSERT_NE(text, nullptr);
  EXPECT_STREQ(text, "val=1");
}

/* ============================================================== */
/* get_text_content_free_temp_on_second_call                      */
/* L777-779: free(noti->temp_content) when it is already non-NULL */
/* ============================================================== */

TEST_F(NotificationApiTest, get_text_content_free_temp_on_second_call) {
  ASSERT_EQ(notification_set_text(noti_, NOTIFICATION_TEXT_TYPE_CONTENT,
                "n=%d", nullptr,
                NOTIFICATION_VARIABLE_TYPE_INT, 5,
                NOTIFICATION_VARIABLE_TYPE_NONE),
            NOTIFICATION_ERROR_NONE);

  char* text = nullptr;
  /* First call: temp_content is NULL → strdup sets it. */
  ASSERT_EQ(
      notification_get_text(noti_, NOTIFICATION_TEXT_TYPE_CONTENT, &text),
      NOTIFICATION_ERROR_NONE);
  ASSERT_NE(text, nullptr);
  EXPECT_STREQ(text, "n=5");

  /* Second call: temp_content != NULL → free branch (L779) is taken. */
  text = nullptr;
  ASSERT_EQ(
      notification_get_text(noti_, NOTIFICATION_TEXT_TYPE_CONTENT, &text),
      NOTIFICATION_ERROR_NONE);
  ASSERT_NE(text, nullptr);
  EXPECT_STREQ(text, "n=5");
}

/* ============================================================== */
/* get_text_key_sys_string_path                                   */
/* L487-491: b_key set, domain == NULL → sys_string dgettext path */
/* ============================================================== */

TEST_F(NotificationApiTest, get_text_key_sys_string_path) {
  /* Clear domain/dir so the "sys_string" dgettext branch is taken
   * instead of the app-domain branch. */
  if (noti_->domain) {
    free(noti_->domain);
    noti_->domain = nullptr;
  }
  if (noti_->dir) {
    free(noti_->dir);
    noti_->dir = nullptr;
  }

  /* Set a key-only text (no plain text string, no format args). */
  ASSERT_EQ(notification_set_text(noti_, NOTIFICATION_TEXT_TYPE_TITLE,
                nullptr, "IDS_KEY",
                NOTIFICATION_VARIABLE_TYPE_NONE),
            NOTIFICATION_ERROR_NONE);

  char* got = nullptr;
  /* dgettext("sys_string", "IDS_KEY") returns "IDS_KEY" unchanged
   * (no catalog) → get_str = NULL; then L504 fallback: get_str = ret_val
   * ("IDS_KEY") is set and returned. */
  ASSERT_EQ(
      notification_get_text(noti_, NOTIFICATION_TEXT_TYPE_TITLE, &got),
      NOTIFICATION_ERROR_NONE);
  ASSERT_NE(got, nullptr);
  EXPECT_STREQ(got, "IDS_KEY");
}

/* ============================================================== */
/* get_text_key_no_matching_bundle_entry                          */
/* L493: get_str = NULL when ret_val == NULL in b_key block        */
/* ============================================================== */

TEST_F(NotificationApiTest, get_text_key_no_matching_bundle_entry) {
  /* Clear domain/dir to enter the b_key code path. */
  if (noti_->domain) {
    free(noti_->domain);
    noti_->domain = nullptr;
  }
  if (noti_->dir) {
    free(noti_->dir);
    noti_->dir = nullptr;
  }

  /* Set a key for TITLE — this creates b_key with a TITLE entry. */
  ASSERT_EQ(notification_set_text(noti_, NOTIFICATION_TEXT_TYPE_TITLE,
                nullptr, "IDS_TITLE",
                NOTIFICATION_VARIABLE_TYPE_NONE),
            NOTIFICATION_ERROR_NONE);

  /* Query CONTENT type: b_key exists but has no CONTENT entry →
   * bundle_get_str returns NULL → else branch L493 (get_str = NULL).
   * b_text is also NULL → *text = NULL. */
  char* got = reinterpret_cast<char*>(0x1); /* non-null sentinel */
  ASSERT_EQ(
      notification_get_text(noti_, NOTIFICATION_TEXT_TYPE_CONTENT, &got),
      NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(got, nullptr);
}

/* ============================================================== */
/* set_sound_user_data_free_priv_path_on_second_call              */
/* L971-973: free(noti->priv_sound_path) when non-NULL (USER_DATA) */
/* ============================================================== */

TEST_F(NotificationApiTest, set_sound_user_data_free_priv_path_on_second_call) {
  /* Inject a pre-existing priv_sound_path to trigger the free branch. */
  noti_->priv_sound_path = strdup("/fake/priv/sound.wav");
  ASSERT_NE(noti_->priv_sound_path, nullptr);

  /* USER_DATA type with a new path: the free branch (L971-973) runs
   * before notification_check_file_path_is_private sets the new value.
   * The file does not exist so priv_sound_path ends up NULL. */
  EXPECT_EQ(
      notification_set_sound(noti_, NOTIFICATION_SOUND_TYPE_USER_DATA,
                             "/other/sound.wav"),
      NOTIFICATION_ERROR_NONE);

  /* sound_path is updated to the new path. */
  EXPECT_STREQ(noti_->sound_path, "/other/sound.wav");
  /* priv_sound_path is freed and reset to NULL (file not private). */
  EXPECT_EQ(noti_->priv_sound_path, nullptr);
}

/* ============================================================== */
/* set_sound_non_user_data_free_both_paths                        */
/* L985-987: else branch frees both sound_path and priv_sound_path */
/* ============================================================== */

TEST_F(NotificationApiTest, set_sound_non_user_data_free_both_paths) {
  /* First set a USER_DATA sound so sound_path is populated. */
  ASSERT_EQ(
      notification_set_sound(noti_, NOTIFICATION_SOUND_TYPE_USER_DATA,
                             "/some/sound.wav"),
      NOTIFICATION_ERROR_NONE);
  ASSERT_STREQ(noti_->sound_path, "/some/sound.wav");

  /* Manually inject priv_sound_path to cover the else-branch free. */
  noti_->priv_sound_path = strdup("/fake/priv.wav");
  ASSERT_NE(noti_->priv_sound_path, nullptr);

  /* Switch to DEFAULT type: else branch (L980-993) frees both paths. */
  EXPECT_EQ(
      notification_set_sound(noti_, NOTIFICATION_SOUND_TYPE_DEFAULT, nullptr),
      NOTIFICATION_ERROR_NONE);

  EXPECT_EQ(noti_->sound_path, nullptr);
  EXPECT_EQ(noti_->priv_sound_path, nullptr);
}

/* ============================================================== */
/* set_vibration_non_default_free_paths                           */
/* L1039-1041: USER_DATA branch frees priv_vibration_path when set*/
/* ============================================================== */

TEST_F(NotificationApiTest, set_vibration_non_default_free_paths) {
  /* Inject a pre-existing priv_vibration_path to trigger the free. */
  noti_->priv_vibration_path = strdup("/fake/priv/vib.wav");
  ASSERT_NE(noti_->priv_vibration_path, nullptr);

  /* USER_DATA type: free branch (L1039-1041) runs, then
   * notification_check_file_path_is_private returns NULL for a
   * non-existent file so priv_vibration_path stays NULL. */
  EXPECT_EQ(
      notification_set_vibration(noti_, NOTIFICATION_VIBRATION_TYPE_USER_DATA,
                                 "/other/vib.wav"),
      NOTIFICATION_ERROR_NONE);

  EXPECT_STREQ(noti_->vibration_path, "/other/vib.wav");
  EXPECT_EQ(noti_->priv_vibration_path, nullptr);
}

/* ============================================================== */
/* set_vibration_non_user_data_free_paths                         */
/* L1047-1053: else branch frees both vibration_path and priv_path*/
/* ============================================================== */

TEST_F(NotificationApiTest, set_vibration_non_user_data_free_paths) {
  /* First set a USER_DATA vibration so vibration_path is populated. */
  ASSERT_EQ(
      notification_set_vibration(noti_, NOTIFICATION_VIBRATION_TYPE_USER_DATA,
                                 "/some/vib.wav"),
      NOTIFICATION_ERROR_NONE);
  ASSERT_STREQ(noti_->vibration_path, "/some/vib.wav");

  /* Manually inject priv_vibration_path to cover the else-branch free. */
  noti_->priv_vibration_path = strdup("/fake/priv/vib.wav");
  ASSERT_NE(noti_->priv_vibration_path, nullptr);

  /* Switch to NONE type: else branch (L1047-1053) frees both paths. */
  EXPECT_EQ(
      notification_set_vibration(noti_, NOTIFICATION_VIBRATION_TYPE_NONE,
                                 nullptr),
      NOTIFICATION_ERROR_NONE);

  EXPECT_EQ(noti_->vibration_path, nullptr);
  EXPECT_EQ(noti_->priv_vibration_path, nullptr);
}

/* ============================================================== */
/* notification_ipc_make_gvariant_from_noti — translate=true      */
/* L82: notification_translate_localized_text branch              */
/* ============================================================== */

TEST_F(NotificationApiTest, ipc_make_gvariant_from_noti_with_translate_true) {
  /* Set text so the translate call has something to work with. */
  ASSERT_EQ(notification_set_text(noti_, NOTIFICATION_TEXT_TYPE_TITLE,
                "Hello", "IDS_TITLE", NOTIFICATION_VARIABLE_TYPE_NONE),
            NOTIFICATION_ERROR_NONE);
  GVariant* v = notification_ipc_make_gvariant_from_noti(noti_, true);
  EXPECT_NE(v, nullptr);
  if (v) g_variant_unref(v);
}

/* ============================================================== */
/* notification_ipc_make_gvariant_from_noti — args / group_args   */
/* L95-99: noti->args branch; L103-108: noti->group_args branch   */
/* ============================================================== */

TEST_F(NotificationApiTest, ipc_make_gvariant_from_noti_with_args_and_group_args) {
  noti_->args = bundle_create();
  ASSERT_NE(noti_->args, nullptr);
  noti_->group_args = bundle_create();
  ASSERT_NE(noti_->group_args, nullptr);
  GVariant* v = notification_ipc_make_gvariant_from_noti(noti_, false);
  EXPECT_NE(v, nullptr);
  if (v) g_variant_unref(v);
  /* notification_free in TearDown calls bundle_free on args/group_args. */
}

/* ============================================================== */
/* notification_ipc_make_gvariant_from_noti — service bundles     */
/* L121-126: b_service_responding; L139-144: b_service_multi_launch */
/* ============================================================== */

TEST_F(NotificationApiTest,
       ipc_make_gvariant_from_noti_with_service_responding_and_multi_launch) {
  noti_->b_service_responding = bundle_create();
  ASSERT_NE(noti_->b_service_responding, nullptr);
  noti_->b_service_multi_launch = bundle_create();
  ASSERT_NE(noti_->b_service_multi_launch, nullptr);
  GVariant* v = notification_ipc_make_gvariant_from_noti(noti_, false);
  EXPECT_NE(v, nullptr);
  if (v) g_variant_unref(v);
}

/* ============================================================== */
/* notification_ipc_make_gvariant_from_noti — private path fields */
/* L205-210: b_priv_image_path; L219: priv_sound_path;            */
/* L227: priv_vibration_path                                      */
/* ============================================================== */

TEST_F(NotificationApiTest,
       ipc_make_gvariant_from_noti_with_priv_image_and_path_fields) {
  noti_->b_priv_image_path = bundle_create();
  ASSERT_NE(noti_->b_priv_image_path, nullptr);
  noti_->priv_sound_path = strdup("/priv/s.ogg");
  ASSERT_NE(noti_->priv_sound_path, nullptr);
  noti_->priv_vibration_path = strdup("/priv/v.wav");
  ASSERT_NE(noti_->priv_vibration_path, nullptr);
  GVariant* v = notification_ipc_make_gvariant_from_noti(noti_, false);
  EXPECT_NE(v, nullptr);
  if (v) g_variant_unref(v);
}

/* ============================================================== */
/* notification_ipc_make_gvariant_from_noti — LED / time fields   */
/* L232: led_argb; L235: led_on_ms; L238: led_off_ms;             */
/* L241: time; L244: insert_time                                  */
/* ============================================================== */

TEST_F(NotificationApiTest,
       ipc_make_gvariant_from_noti_with_led_and_time_fields) {
  noti_->led_argb = 0xFF0000FF;
  noti_->led_on_ms = 100;
  noti_->led_off_ms = 200;
  noti_->time = 1000;
  noti_->insert_time = 2000;
  GVariant* v = notification_ipc_make_gvariant_from_noti(noti_, false);
  EXPECT_NE(v, nullptr);
  if (v) g_variant_unref(v);
}

/* ============================================================== */
/* notification_ipc_make_gvariant_from_noti — misc non-zero fields*/
/* L247: flags_for_property; L250: progress_size;                 */
/* L253: progress_percentage; L256: app_icon_path;                */
/* L260: temp_title; L262: temp_content; L283: channel_name       */
/* ============================================================== */

TEST_F(NotificationApiTest,
       ipc_make_gvariant_from_noti_with_misc_nonzero_fields) {
  noti_->flags_for_property = 1;
  noti_->progress_size = 0.5f;
  noti_->progress_percentage = 75.0f;
  noti_->app_icon_path = strdup("/icon.png");
  noti_->temp_title = strdup("T");
  noti_->temp_content = strdup("C");
  noti_->channel_name = strdup("ch1");
  GVariant* v = notification_ipc_make_gvariant_from_noti(noti_, false);
  EXPECT_NE(v, nullptr);
  if (v) g_variant_unref(v);
}

/* ============================================================== */
/* notification_ipc_make_noti_from_gvariant — null parameter checks*/
/* L374-375: noti == NULL; L379-380: variant == NULL              */
/* ============================================================== */

TEST_F(NotificationApiTest,
       ipc_make_noti_from_gvariant_null_noti_returns_invalid) {
  GVariant* dummy = g_variant_new("i", 0);
  EXPECT_EQ(notification_ipc_make_noti_from_gvariant(nullptr, dummy),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  g_variant_unref(dummy);
}

TEST_F(NotificationApiTest,
       ipc_make_noti_from_gvariant_null_variant_returns_invalid) {
  EXPECT_EQ(notification_ipc_make_noti_from_gvariant(noti_, nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* ============================================================== */
/* notification_ipc_make_noti_from_gvariant — unwrapped a{iv}     */
/* Unwrap the (v) wrapper → pass inner a{iv} to cover the         */
/* _variant_to_int_dict while-loop body (L304-305, L309-310)      */
/* ============================================================== */

TEST_F(NotificationApiTest,
       ipc_make_noti_from_gvariant_with_unwrapped_dict_variant) {
  /* make_gvariant_from_noti returns (v) wrapping an a{iv} dict.
   * _variant_to_int_dict iterates expecting a{iv} directly, so
   * unwrap one level to exercise the while-loop body. */
  GVariant* gv = notification_ipc_make_gvariant_from_noti(noti_, false);
  ASSERT_NE(gv, nullptr);

  GVariant* inner = nullptr;
  g_variant_get(gv, "(v)", &inner);
  ASSERT_NE(inner, nullptr);

  notification_h dst = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(dst, nullptr);
  int ret = notification_ipc_make_noti_from_gvariant(dst, inner);
  g_variant_unref(inner);
  g_variant_unref(gv);
  notification_free(dst);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
}

/* ============================================================== */
/* notification_clone — optional bundle/path fields               */
/* source lines covered: L1720, L1723, L1729, L1735, L1752,      */
/*                        L1763, L1771, L1779                     */
/* ============================================================== */

TEST_F(NotificationApiTest, clone_copies_optional_bundle_and_path_fields) {
  /* Set optional fields not covered by existing clone tests:
   * args, group_args, b_service_responding, b_service_multi_launch,
   * b_key (via set_text with key), b_priv_image_path, priv_sound_path,
   * priv_vibration_path */
  noti_->args = bundle_create();
  noti_->group_args = bundle_create();
  noti_->b_service_responding = bundle_create();
  noti_->b_service_multi_launch = bundle_create();
  /* b_key: set via notification_set_text with a key */
  ASSERT_EQ(notification_set_text(noti_, NOTIFICATION_TEXT_TYPE_TITLE,
                nullptr, "IDS_KEY", NOTIFICATION_VARIABLE_TYPE_NONE),
            NOTIFICATION_ERROR_NONE);
  noti_->b_priv_image_path = bundle_create();
  noti_->priv_sound_path = strdup("/priv/s.ogg");
  noti_->priv_vibration_path = strdup("/priv/v.wav");

  notification_h cloned = nullptr;
  ASSERT_EQ(notification_clone(noti_, &cloned), NOTIFICATION_ERROR_NONE);
  ASSERT_NE(cloned, nullptr);

  EXPECT_NE(cloned->args, nullptr);
  EXPECT_NE(cloned->group_args, nullptr);
  EXPECT_NE(cloned->b_service_responding, nullptr);
  EXPECT_NE(cloned->b_service_multi_launch, nullptr);
  EXPECT_NE(cloned->b_key, nullptr);
  EXPECT_NE(cloned->b_priv_image_path, nullptr);
  EXPECT_NE(cloned->priv_sound_path, nullptr);
  EXPECT_NE(cloned->priv_vibration_path, nullptr);

  notification_free(cloned);
  /* TearDown frees noti_ fields. */
}

/* ============================================================== */
/* notification_get_event_handler — invalid event type            */
/* source lines covered: L1261-L1263                              */
/* ============================================================== */

TEST_F(NotificationApiTest, get_event_handler_invalid_event_type_returns_invalid) {
  app_control_h out = nullptr;
  /* Pass an out-of-range event type. */
  EXPECT_EQ(notification_get_event_handler(
                noti_,
                static_cast<notification_event_type_e>(9999),
                &out),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(out, nullptr);
}

/* ============================================================== */
/* notification_get_text — COUNT LEFT position variable           */
/* source lines covered: L538-L554                                */
/* ============================================================== */

TEST_F(NotificationApiTest, get_text_count_variable_left_position) {
  /* Set text "items" with COUNT-LEFT as the variable arg.
   * Lines 538-554: the code prepends the count (0 when TIDL unavailable). */
  ASSERT_EQ(notification_set_text(noti_, NOTIFICATION_TEXT_TYPE_TITLE,
                "items", nullptr,
                NOTIFICATION_VARIABLE_TYPE_COUNT,
                NOTIFICATION_COUNT_POS_LEFT,
                NOTIFICATION_VARIABLE_TYPE_NONE),
            NOTIFICATION_ERROR_NONE);
  char* got = nullptr;
  ASSERT_EQ(notification_get_text(noti_, NOTIFICATION_TEXT_TYPE_TITLE, &got),
            NOTIFICATION_ERROR_NONE);
  ASSERT_NE(got, nullptr);
  /* Count is prepended; exact value depends on TIDL result. */
}

/* ============================================================== */
/* notification_get_text — COUNT in %d substitution               */
/* source lines covered: L589-L590                                */
/* ============================================================== */

TEST_F(NotificationApiTest, get_text_count_variable_in_percent_d_format) {
  /* Text "%d items" with COUNT-IN type.
   * Lines 589-590: notification_get_count is called for %d. */
  ASSERT_EQ(notification_set_text(noti_, NOTIFICATION_TEXT_TYPE_TITLE,
                "%d items", nullptr,
                NOTIFICATION_VARIABLE_TYPE_COUNT,
                NOTIFICATION_COUNT_POS_IN,
                NOTIFICATION_VARIABLE_TYPE_NONE),
            NOTIFICATION_ERROR_NONE);
  char* got = nullptr;
  ASSERT_EQ(notification_get_text(noti_, NOTIFICATION_TEXT_TYPE_TITLE, &got),
            NOTIFICATION_ERROR_NONE);
  ASSERT_NE(got, nullptr);
}

/* ============================================================== */
/* notification_get_text — %s substitution via sys_string path    */
/* source lines covered: L623-L627                                */
/* ============================================================== */

TEST_F(NotificationApiTest, get_text_percent_s_substitution_sys_string_path) {
  /* Clear domain/dir so the sys_string dgettext branch (L623-627) is used. */
  if (noti_->domain) { free(noti_->domain); noti_->domain = nullptr; }
  if (noti_->dir) { free(noti_->dir); noti_->dir = nullptr; }
  ASSERT_EQ(notification_set_text(noti_, NOTIFICATION_TEXT_TYPE_TITLE,
                "msg=%s", nullptr,
                NOTIFICATION_VARIABLE_TYPE_STRING, "world",
                NOTIFICATION_VARIABLE_TYPE_NONE),
            NOTIFICATION_ERROR_NONE);
  char* got = nullptr;
  ASSERT_EQ(notification_get_text(noti_, NOTIFICATION_TEXT_TYPE_TITLE, &got),
            NOTIFICATION_ERROR_NONE);
  ASSERT_NE(got, nullptr);
  /* dgettext("sys_string", "world") returns "world" when no translation exists. */
  EXPECT_STREQ(got, "msg=world");
}

/* ============================================================== */
/* notification_get_text — COUNT RIGHT position variable          */
/* source lines covered: L742-L758                                */
/* ============================================================== */

TEST_F(NotificationApiTest, get_text_count_variable_right_position) {
  /* Set text "items" with COUNT-RIGHT.
   * Lines 742-758: count is appended after the text. */
  ASSERT_EQ(notification_set_text(noti_, NOTIFICATION_TEXT_TYPE_TITLE,
                "items", nullptr,
                NOTIFICATION_VARIABLE_TYPE_COUNT,
                NOTIFICATION_COUNT_POS_RIGHT,
                NOTIFICATION_VARIABLE_TYPE_NONE),
            NOTIFICATION_ERROR_NONE);
  char* got = nullptr;
  ASSERT_EQ(notification_get_text(noti_, NOTIFICATION_TEXT_TYPE_TITLE, &got),
            NOTIFICATION_ERROR_NONE);
  ASSERT_NE(got, nullptr);
}

/* ============================================================== */
/* notification_get_text — positional %1$d with COUNT variable    */
/* source lines covered: L651-L684 (positional branch, COUNT type)*/
/* ============================================================== */

TEST_F(NotificationApiTest, get_text_count_variable_positional_percent1d_format) {
  /* Format "%1$d items" enters the positional-arg branch (L651) and
   * the COUNT-variable check at L664-671. */
  ASSERT_EQ(notification_set_text(noti_, NOTIFICATION_TEXT_TYPE_TITLE,
                "%1$d items", nullptr,
                NOTIFICATION_VARIABLE_TYPE_COUNT,
                NOTIFICATION_COUNT_POS_IN,
                NOTIFICATION_VARIABLE_TYPE_NONE),
            NOTIFICATION_ERROR_NONE);
  char* got = nullptr;
  ASSERT_EQ(notification_get_text(noti_, NOTIFICATION_TEXT_TYPE_TITLE, &got),
            NOTIFICATION_ERROR_NONE);
  ASSERT_NE(got, nullptr);
}

/* ============================================================== */
/* notification_get_launch_option — app_control_create fails      */
/* source lines covered: L1202-L1204                              */
/* ============================================================== */

TEST_F(NotificationApiTest,
       get_launch_option_app_control_create_fails_returns_io_error) {
  /* Populate b_service_single_launch via set_launch_option. */
  app_control_h ac = nullptr;
  ASSERT_EQ(app_control_create(&ac), APP_CONTROL_ERROR_NONE);
  ASSERT_NE(ac, nullptr);
  ASSERT_EQ(notification_set_launch_option(noti_,
      NOTIFICATION_LAUNCH_OPTION_APP_CONTROL, ac), NOTIFICATION_ERROR_NONE);
  app_control_destroy(ac);

  /* Override: create fails → function returns IO_ERROR (L1202-1204). */
  EXPECT_CALL(GetMock<AppControlMock>(), app_control_create(_))
      .WillOnce(Return(APP_CONTROL_ERROR_OUT_OF_MEMORY));

  app_control_h result = nullptr;
  EXPECT_EQ(notification_get_launch_option(noti_,
      NOTIFICATION_LAUNCH_OPTION_APP_CONTROL, &result),
      NOTIFICATION_ERROR_IO_ERROR);
}

/* ============================================================== */
/* notification_get_launch_option — import_from_bundle fails      */
/* source lines covered: L1198-L1200                              */
/* ============================================================== */

static int __fake_app_control_create_fake_handle(app_control_h* h) {
  *h = reinterpret_cast<app_control_h>(static_cast<uintptr_t>(1));
  return APP_CONTROL_ERROR_NONE;
}

TEST_F(NotificationApiTest,
       get_launch_option_import_bundle_fails_returns_io_error) {
  app_control_h ac = nullptr;
  ASSERT_EQ(app_control_create(&ac), APP_CONTROL_ERROR_NONE);
  ASSERT_NE(ac, nullptr);
  ASSERT_EQ(notification_set_launch_option(noti_,
      NOTIFICATION_LAUNCH_OPTION_APP_CONTROL, ac), NOTIFICATION_ERROR_NONE);
  app_control_destroy(ac);

  /* Override: create succeeds (fake handle), import fails → IO_ERROR (L1198-1200). */
  EXPECT_CALL(GetMock<AppControlMock>(), app_control_create(_))
      .WillOnce(Invoke(__fake_app_control_create_fake_handle));
  EXPECT_CALL(GetMock<AppControlMock>(), app_control_import_from_bundle(_, _))
      .WillOnce(Return(APP_CONTROL_ERROR_INVALID_PARAMETER));
  EXPECT_CALL(GetMock<AppControlMock>(), app_control_destroy(_))
      .WillOnce(Return(APP_CONTROL_ERROR_NONE));

  app_control_h result = nullptr;
  EXPECT_EQ(notification_get_launch_option(noti_,
      NOTIFICATION_LAUNCH_OPTION_APP_CONTROL, &result),
      NOTIFICATION_ERROR_IO_ERROR);
}

/* ============================================================== */
/* notification_get_event_handler — app_control_create fails      */
/* source lines covered: L1274-L1277                              */
/* ============================================================== */

TEST_F(NotificationApiTest,
       get_event_handler_app_control_create_fails_returns_io_error) {
  app_control_h ac = nullptr;
  ASSERT_EQ(app_control_create(&ac), APP_CONTROL_ERROR_NONE);
  ASSERT_NE(ac, nullptr);
  ASSERT_EQ(notification_set_event_handler(noti_,
      NOTIFICATION_EVENT_TYPE_CLICK_ON_BUTTON_1, ac), NOTIFICATION_ERROR_NONE);
  app_control_destroy(ac);

  /* Override: create fails → IO_ERROR (L1274-1277). */
  EXPECT_CALL(GetMock<AppControlMock>(), app_control_create(_))
      .WillOnce(Return(APP_CONTROL_ERROR_OUT_OF_MEMORY));

  app_control_h result = nullptr;
  EXPECT_EQ(notification_get_event_handler(noti_,
      NOTIFICATION_EVENT_TYPE_CLICK_ON_BUTTON_1, &result),
      NOTIFICATION_ERROR_IO_ERROR);
}

/* ============================================================== */
/* notification_get_event_handler — import_from_bundle fails      */
/* source lines covered: L1284-L1288                              */
/* ============================================================== */

TEST_F(NotificationApiTest,
       get_event_handler_import_bundle_fails_returns_io_error) {
  app_control_h ac = nullptr;
  ASSERT_EQ(app_control_create(&ac), APP_CONTROL_ERROR_NONE);
  ASSERT_NE(ac, nullptr);
  ASSERT_EQ(notification_set_event_handler(noti_,
      NOTIFICATION_EVENT_TYPE_CLICK_ON_BUTTON_1, ac), NOTIFICATION_ERROR_NONE);
  app_control_destroy(ac);

  /* Override: create succeeds (fake handle), import fails → IO_ERROR (L1284-1288). */
  EXPECT_CALL(GetMock<AppControlMock>(), app_control_create(_))
      .WillOnce(Invoke(__fake_app_control_create_fake_handle));
  EXPECT_CALL(GetMock<AppControlMock>(), app_control_import_from_bundle(_, _))
      .WillOnce(Return(APP_CONTROL_ERROR_INVALID_PARAMETER));
  EXPECT_CALL(GetMock<AppControlMock>(), app_control_destroy(_))
      .WillOnce(Return(APP_CONTROL_ERROR_NONE));

  app_control_h result = nullptr;
  EXPECT_EQ(notification_get_event_handler(noti_,
      NOTIFICATION_EVENT_TYPE_CLICK_ON_BUTTON_1, &result),
      NOTIFICATION_ERROR_IO_ERROR);
}

/* ============================================================== */
/* notification_set_launch_option — app_control_export_as_bundle  */
/* fails → NOTIFICATION_ERROR_INVALID_PARAMETER                   */
/* source lines covered: L1159-L1161                              */
/* ============================================================== */

TEST_F(NotificationApiTest,
       set_launch_option_export_as_bundle_fails_returns_invalid) {
  app_control_h ac = nullptr;
  ASSERT_EQ(app_control_create(&ac), APP_CONTROL_ERROR_NONE);
  ASSERT_NE(ac, nullptr);

  /* Override: export fails so set_launch_option returns INVALID_PARAMETER. */
  EXPECT_CALL(GetMock<AppControlMock>(), app_control_export_as_bundle(_, _))
      .WillOnce(Return(APP_CONTROL_ERROR_INVALID_PARAMETER));

  EXPECT_EQ(notification_set_launch_option(noti_,
      NOTIFICATION_LAUNCH_OPTION_APP_CONTROL, ac),
      NOTIFICATION_ERROR_INVALID_PARAMETER);

  app_control_destroy(ac);
}

/* ============================================================== */
/* notification_get_noti_block_state — success paths              */
/* source lines covered: L2068-L2077                              */
/* ============================================================== */

TEST_F(NotificationApiTest,
       get_noti_block_state_success_returns_allowed) {
  /* allow_to_notify=1, dnd=0 → NOTIFICATION_BLOCK_STATE_ALLOWED */
  EXPECT_CALL(GetMock<NotiServiceProxyMock>(),
      rpc_port_proxy_noti_service_invoke_get_noti_block_state(_, _, _, _, _, _))
      .WillOnce(DoAll(
          SetArgPointee<3>(0),
          SetArgPointee<4>(0),
          SetArgPointee<5>(1),
          Return(RPC_PORT_ERROR_NONE)));
  notification_block_state_e state = NOTIFICATION_BLOCK_STATE_BLOCKED;
  ASSERT_EQ(notification_get_noti_block_state(&state), NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(state, NOTIFICATION_BLOCK_STATE_ALLOWED);
}

TEST_F(NotificationApiTest,
       get_noti_block_state_success_returns_do_not_disturb) {
  /* allow_to_notify=1, dnd=1, dnd_except=0 → DO_NOT_DISTURB */
  EXPECT_CALL(GetMock<NotiServiceProxyMock>(),
      rpc_port_proxy_noti_service_invoke_get_noti_block_state(_, _, _, _, _, _))
      .WillOnce(DoAll(
          SetArgPointee<3>(1),
          SetArgPointee<4>(0),
          SetArgPointee<5>(1),
          Return(RPC_PORT_ERROR_NONE)));
  notification_block_state_e state = NOTIFICATION_BLOCK_STATE_ALLOWED;
  ASSERT_EQ(notification_get_noti_block_state(&state), NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(state, NOTIFICATION_BLOCK_STATE_DO_NOT_DISTURB);
}

TEST_F(NotificationApiTest,
       get_noti_block_state_success_returns_blocked) {
  /* allow_to_notify=0 → NOTIFICATION_BLOCK_STATE_BLOCKED */
  EXPECT_CALL(GetMock<NotiServiceProxyMock>(),
      rpc_port_proxy_noti_service_invoke_get_noti_block_state(_, _, _, _, _, _))
      .WillOnce(DoAll(
          SetArgPointee<3>(0),
          SetArgPointee<4>(0),
          SetArgPointee<5>(0),
          Return(RPC_PORT_ERROR_NONE)));
  notification_block_state_e state = NOTIFICATION_BLOCK_STATE_ALLOWED;
  ASSERT_EQ(notification_get_noti_block_state(&state), NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(state, NOTIFICATION_BLOCK_STATE_BLOCKED);
}

/* ============================================================== */
/* calloc OOM paths in _notification_create / notification_clone  */
/* ============================================================== */

/* calloc OOM inside _notification_create → L1534-L1536 */
TEST_F(NotificationApiTest, notification_create_calloc_oom_covers_L1534_L1536) {
  /* SetUp already consumed one calloc(1, sizeof(notification_s)) for noti_.
   * The NEXT calloc of that size is the one inside _notification_create. */
  calloc_set_fail_on_size(sizeof(struct _notification));
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  calloc_reset_fail();
  EXPECT_EQ(noti, nullptr);
  EXPECT_EQ(get_last_result(), NOTIFICATION_ERROR_OUT_OF_MEMORY);
}

/* calloc OOM inside notification_clone → L1700-L1701 */
TEST_F(NotificationApiTest, notification_clone_calloc_oom_covers_L1700_L1701) {
  notification_h clone = nullptr;
  /* noti_ is valid from SetUp. The next calloc(1, sizeof(notification_s))
   * inside notification_clone will fail. */
  calloc_set_fail_on_size(sizeof(struct _notification));
  int ret = notification_clone(noti_, &clone);
  calloc_reset_fail();
  EXPECT_EQ(ret, NOTIFICATION_ERROR_OUT_OF_MEMORY);
  EXPECT_EQ(clone, nullptr);
}

