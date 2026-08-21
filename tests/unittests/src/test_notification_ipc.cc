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
#include <gmock/gmock.h>

#include "mock/test_fixture.hh"
#include "mock/aul_mock.hh"
#include "mock/package_manager_mock.hh"
#include "mock/tzplatform_config_mock.hh"
#include "mock/glib_mock.hh"

#include "notification/include/notification.h"
#include "notification/include/notification_internal.h"
#include "notification/include/notification_ipc.h"
#include "notification/include/notification_text_domain.h"

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
    pkgmgrinfo_appinfo_h *handle) {
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
    virtual public ::testing::NiceMock<PackageManagerMock> {};

}  // namespace

using ::testing::_;
using ::testing::Invoke;

class NotificationIpcTest : public TestFixture {
 public:
  NotificationIpcTest() : TestFixture(std::make_unique<::Mocks>()) {}

  void SetUp() override {
    EXPECT_CALL(GetMock<AulMock>(), aul_app_get_appid_bypid(_, _, _))
        .WillRepeatedly(Invoke(__fake_aul_app_get_appid_bypid));
    EXPECT_CALL(GetMock<AulMock>(), aul_app_get_pkgid_bypid(_, _, _))
        .WillRepeatedly(Invoke(__fake_aul_app_get_pkgid_bypid));
    EXPECT_CALL(GetMock<PackageManagerMock>(), package_info_create(_, _))
        .WillRepeatedly(Invoke(__fake_package_info_create));
    EXPECT_CALL(GetMock<PackageManagerMock>(), package_info_get_root_path(_, _))
        .WillRepeatedly(Invoke(__fake_package_info_get_root_path));
    EXPECT_CALL(GetMock<PackageManagerMock>(), pkgmgrinfo_appinfo_get_usr_appinfo(_, _, _))
        .WillRepeatedly(Invoke(__fake_pkgmgrinfo_appinfo_get_usr_appinfo));
    EXPECT_CALL(GetMock<PackageManagerMock>(), pkgmgrinfo_appinfo_get_label(_, _))
        .WillRepeatedly(Invoke(__fake_pkgmgrinfo_appinfo_get_label));
  }
};

TEST_F(NotificationIpcTest, MakeNotiFromGvariantFailsWhenNotiIsNull) {
  GVariantBuilder builder;
  g_variant_builder_init(&builder, G_VARIANT_TYPE("a{iv}"));
  GVariant *variant = g_variant_builder_end(&builder);

  int ret = notification_ipc_make_noti_from_gvariant(nullptr, variant);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);

  g_variant_unref(variant);
}

TEST_F(NotificationIpcTest, MakeNotiFromGvariantFailsWhenVariantIsNull) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  int ret = notification_ipc_make_noti_from_gvariant(noti, nullptr);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);

  notification_free(noti);
}

TEST_F(NotificationIpcTest, MakeNotiFromGvariantHandlesEmptyPayload) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  GVariantBuilder builder;
  g_variant_builder_init(&builder, G_VARIANT_TYPE("a{iv}"));
  GVariant *variant = g_variant_builder_end(&builder);

  int ret = notification_ipc_make_noti_from_gvariant(noti, variant);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  g_variant_unref(variant);
  notification_free(noti);
}

TEST_F(NotificationIpcTest, MakeNotiFromGvariantRoundTripsFullPayload) {
  notification_h src = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(src, nullptr);

  ASSERT_EQ(notification_set_text(src, NOTIFICATION_TEXT_TYPE_TITLE,
      "title", "key", NOTIFICATION_VARIABLE_TYPE_NONE), NOTIFICATION_ERROR_NONE);
  ASSERT_EQ(notification_set_image(src, NOTIFICATION_IMAGE_TYPE_ICON,
      "/tmp/icon.png"), NOTIFICATION_ERROR_NONE);
  ASSERT_EQ(notification_set_sound(src, NOTIFICATION_SOUND_TYPE_USER_DATA,
      "/tmp/sound.wav"), NOTIFICATION_ERROR_NONE);
  ASSERT_EQ(notification_set_vibration(src, NOTIFICATION_VIBRATION_TYPE_USER_DATA,
      "/tmp/vib.dat"), NOTIFICATION_ERROR_NONE);
  ASSERT_EQ(notification_set_led(src, NOTIFICATION_LED_OP_ON_CUSTOM_COLOR, 0xff0000),
      NOTIFICATION_ERROR_NONE);
  ASSERT_EQ(notification_set_led_time_period(src, 100, 200), NOTIFICATION_ERROR_NONE);
  ASSERT_EQ(notification_set_tag(src, "tag"), NOTIFICATION_ERROR_NONE);
  ASSERT_EQ(notification_set_channel_name(src, "channel"), NOTIFICATION_ERROR_NONE);
  ASSERT_EQ(notification_set_text_domain(src, "domain", "/tmp/locale"),
      NOTIFICATION_ERROR_NONE);
  ASSERT_EQ(notification_set_size(src, 1.5), NOTIFICATION_ERROR_NONE);
  ASSERT_EQ(notification_set_progress(src, 30.0), NOTIFICATION_ERROR_NONE);

  app_control_h app_control = nullptr;
  app_control_create(&app_control);
  app_control_set_app_id(app_control, "org.tizen.testapp");
  ASSERT_EQ(notification_set_event_handler(src,
      NOTIFICATION_EVENT_TYPE_CLICK_ON_BUTTON_1, app_control),
      NOTIFICATION_ERROR_NONE);
  app_control_destroy(app_control);

  GVariant *tuple = notification_ipc_make_gvariant_from_noti(src, false);
  ASSERT_NE(tuple, nullptr);
  GVariant *wrapped = g_variant_get_child_value(tuple, 0);
  GVariant *payload = g_variant_get_variant(wrapped);

  notification_h dst = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(dst, nullptr);

  int ret = notification_ipc_make_noti_from_gvariant(dst, payload);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  char *title = nullptr;
  ASSERT_EQ(notification_get_text(dst, NOTIFICATION_TEXT_TYPE_TITLE, &title),
      NOTIFICATION_ERROR_NONE);
  EXPECT_STREQ(title, "title");

  const char *tag = nullptr;
  ASSERT_EQ(notification_get_tag(dst, &tag), NOTIFICATION_ERROR_NONE);
  EXPECT_STREQ(tag, "tag");

  const char *channel_name = nullptr;
  ASSERT_EQ(notification_get_channel_name(dst, &channel_name),
      NOTIFICATION_ERROR_NONE);
  EXPECT_STREQ(channel_name, "channel");

  g_variant_unref(payload);
  g_variant_unref(wrapped);
  g_variant_unref(tuple);
  notification_free(src);
  notification_free(dst);
}
