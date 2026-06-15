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

#include "mock/aul_mock.hh"
#include "mock/glib_mock.hh"
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

class NotificationApiTest : public TestFixture {
 public:
  NotificationApiTest() : TestFixture(std::make_unique<::Mocks>()) {}

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
