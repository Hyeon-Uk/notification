/*
 * Copyright (c) 2023 Samsung Electronics Co., Ltd.
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
  *package_info  = (package_info_h)calloc(1, sizeof(struct package_info_s));
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
} // namespace

using ::testing::_;
using ::testing::Invoke;

class NotificationTest : public TestFixture {
 public:
  NotificationTest() : TestFixture(std::make_unique<::Mocks>()) {}

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
  }

  void TearDown() override {}
};

TEST_F(NotificationTest, CreateAndFree) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  int ret = notification_free(noti);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
}

TEST_F(NotificationTest, SetGetImage) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  const char* path =
      "/opt/usr/home/owner/apps_rw/org.tizen.testpkgid/data/"
      ".notification/image.png";
  int ret = notification_set_image(noti, NOTIFICATION_IMAGE_TYPE_ICON, path);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  char* image_path = nullptr;
  ret = notification_get_image(noti, NOTIFICATION_IMAGE_TYPE_ICON, &image_path);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  EXPECT_STREQ(image_path, path);

  notification_free(noti);
}

TEST_F(NotificationTest, SetGetTime) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  int ret = notification_set_time(noti, 0xFFFFFFF);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  time_t ret_time = 0;
  ret = notification_get_time(noti, &ret_time);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  EXPECT_NE(ret_time, 0);

  notification_free(noti);
}

TEST_F(NotificationTest, GetInsertTime) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  time_t ret_time = 0;
  int ret = notification_get_insert_time(noti, &ret_time);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  notification_free(noti);
}

TEST_F(NotificationTest, SetGetText) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  int ret = notification_set_text(noti, NOTIFICATION_TEXT_TYPE_TITLE,
      "testtitle", NULL, NOTIFICATION_VARIABLE_TYPE_NONE);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  char* title_text = nullptr;
  ret = notification_get_text(noti, NOTIFICATION_TEXT_TYPE_TITLE, &title_text);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  EXPECT_STREQ(title_text, "testtitle");

  notification_free(noti);
}

TEST_F(NotificationTest, SetGetTextWithFormatArgs) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  int ret = notification_set_text(noti, NOTIFICATION_TEXT_TYPE_CONTENT,
      "%f %d %s %d", NULL,
      NOTIFICATION_VARIABLE_TYPE_COUNT, NOTIFICATION_COUNT_POS_LEFT,
      NOTIFICATION_VARIABLE_TYPE_DOUBLE, 0.0,
      NOTIFICATION_VARIABLE_TYPE_COUNT, NOTIFICATION_COUNT_POS_IN,
      NOTIFICATION_VARIABLE_TYPE_STRING, "Content",
      NOTIFICATION_VARIABLE_TYPE_INT, 0,
      NOTIFICATION_VARIABLE_TYPE_COUNT, NOTIFICATION_COUNT_POS_RIGHT,
      NOTIFICATION_VARIABLE_TYPE_NONE);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  char* content_text = nullptr;
  ret = notification_get_text(noti, NOTIFICATION_TEXT_TYPE_CONTENT,
      &content_text);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  EXPECT_NE(content_text, nullptr);

  notification_free(noti);
}

TEST_F(NotificationTest, SetGetTimeToText) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  int ret = notification_set_time_to_text(noti,
      NOTIFICATION_TEXT_TYPE_TITLE, time(NULL));
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  time_t ret_time = 0;
  ret = notification_get_time_from_text(noti,
      NOTIFICATION_TEXT_TYPE_TITLE, &ret_time);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  notification_free(noti);
}

TEST_F(NotificationTest, SetGetSound) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  int ret = notification_set_sound(noti, NOTIFICATION_SOUND_TYPE_DEFAULT,
      "soundpath");
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  notification_sound_type_e sound_type = NOTIFICATION_SOUND_TYPE_NONE;
  const char* sound_path = nullptr;
  ret = notification_get_sound(noti, &sound_type, &sound_path);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  notification_free(noti);
}

TEST_F(NotificationTest, SetGetVibration) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  int ret = notification_set_vibration(noti,
      NOTIFICATION_VIBRATION_TYPE_DEFAULT, "vibpath");
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  notification_vibration_type_e vib_type = NOTIFICATION_VIBRATION_TYPE_NONE;
  const char* vib_path = nullptr;
  ret = notification_get_vibration(noti, &vib_type, &vib_path);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  notification_free(noti);
}

TEST_F(NotificationTest, SetGetLed) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  int ret = notification_set_led(noti,
      NOTIFICATION_LED_OP_ON_CUSTOM_COLOR, 256);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  notification_led_op_e op = NOTIFICATION_LED_OP_OFF;
  int led_argb = 0;
  ret = notification_get_led(noti, &op, &led_argb);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  notification_free(noti);
}

TEST_F(NotificationTest, SetGetLedTimePeriod) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  int ret = notification_set_led_time_period(noti, 100, 200);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  int on_ms = 0, off_ms = 0;
  ret = notification_get_led_time_period(noti, &on_ms, &off_ms);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  notification_free(noti);
}

TEST_F(NotificationTest, SetGetLaunchOption) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  app_control_h app_control;
  app_control_create(&app_control);
  app_control_set_app_id(app_control, "org.tizen.testapp");
  int ret = notification_set_launch_option(noti,
      NOTIFICATION_LAUNCH_OPTION_APP_CONTROL, app_control);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  app_control_destroy(app_control);

  ret = notification_get_launch_option(noti,
      NOTIFICATION_LAUNCH_OPTION_APP_CONTROL, (void *)&app_control);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  app_control_destroy(app_control);

  notification_free(noti);
}

TEST_F(NotificationTest, SetGetEventHandler) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  app_control_h app_control;
  app_control_create(&app_control);
  app_control_set_app_id(app_control, "org.tizen.testapp");
  int ret = notification_set_event_handler(noti,
      NOTIFICATION_EVENT_TYPE_CLICK_ON_BUTTON_1, app_control);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  app_control_destroy(app_control);

  ret = notification_get_event_handler(noti,
      NOTIFICATION_EVENT_TYPE_CLICK_ON_BUTTON_1, &app_control);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  app_control_destroy(app_control);

  notification_free(noti);
}

TEST_F(NotificationTest, SetGetProperty) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  int ret = notification_set_property(noti,
      NOTIFICATION_PROP_PERMANENT_DISPLAY);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  int property;
  ret = notification_get_property(noti, &property);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  notification_free(noti);
}

TEST_F(NotificationTest, SetGetDisplayApplist) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  int ret = notification_set_display_applist(noti,
      NOTIFICATION_DISPLAY_APP_NOTIFICATION_TRAY);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  int ret_applist;
  ret = notification_get_display_applist(noti, &ret_applist);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  notification_free(noti);
}

TEST_F(NotificationTest, SetGetSize) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  int ret = notification_set_size(noti, 3.3);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  double ret_size;
  ret = notification_get_size(noti, &ret_size);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  notification_free(noti);
}

TEST_F(NotificationTest, SetGetProgress) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  int ret = notification_set_progress(noti, 30.);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  double percentage;
  ret = notification_get_progress(noti, &percentage);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  notification_free(noti);
}

TEST_F(NotificationTest, SetGetLayout) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  int ret = notification_set_layout(noti,
      NOTIFICATION_LY_NOTI_EVENT_SINGLE);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  notification_ly_type_e ret_layout;
  ret = notification_get_layout(noti, &ret_layout);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  notification_free(noti);
}

TEST_F(NotificationTest, GetType) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  notification_type_e ret_type;
  int ret = notification_get_type(noti, &ret_type);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(ret_type, NOTIFICATION_TYPE_NOTI);

  notification_free(noti);
}

TEST_F(NotificationTest, SetGetTag) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  int ret = notification_set_tag(noti, "test_tag");
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  const char* ret_tag;
  ret = notification_get_tag(noti, &ret_tag);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  EXPECT_STREQ(ret_tag, "test_tag");

  notification_free(noti);
}

TEST_F(NotificationTest, GetPkgname) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  char* pkgname;
  int ret = notification_get_pkgname(noti, &pkgname);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  notification_free(noti);
}

TEST_F(NotificationTest, AddRemoveButton) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  int ret = notification_add_button(noti, NOTIFICATION_BUTTON_1);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  ret = notification_remove_button(noti, NOTIFICATION_BUTTON_1);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  notification_free(noti);
}

TEST_F(NotificationTest, SetGetAutoRemove) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  int ret = notification_set_auto_remove(noti, true);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  bool auto_remove;
  ret = notification_get_auto_remove(noti, &auto_remove);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  EXPECT_TRUE(auto_remove);

  notification_free(noti);
}

TEST_F(NotificationTest, SetTextInput) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  int ret = notification_set_text_input(noti, 1000);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  notification_free(noti);
}

TEST_F(NotificationTest, SetGetExtensionImageSize) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  int ret = notification_set_extension_image_size(noti, 20);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  int ret_height;
  ret = notification_get_extension_image_size(noti, &ret_height);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(ret_height, 20);

  notification_free(noti);
}

TEST_F(NotificationTest, Clone) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  notification_h cloned_noti = nullptr;
  int ret = notification_clone(noti, &cloned_noti);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  EXPECT_NE(cloned_noti, nullptr);

  ret = notification_free(cloned_noti);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  notification_free(noti);
}

TEST_F(NotificationTest, PostNullParam) {
  int ret = notification_post(nullptr);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationTest, UpdateNullParam) {
  int ret = notification_update(nullptr);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationTest, DeleteNullParam) {
  int ret = notification_delete(nullptr);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationTest, DeleteAllInvalidType) {
  int ret = notification_delete_all(NOTIFICATION_TYPE_NONE);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationTest, SaveAsTemplateNullNoti) {
  int ret = notification_save_as_template(nullptr, "template");
  EXPECT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationTest, SaveAsTemplateNullName) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  int ret = notification_save_as_template(noti, nullptr);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);

  notification_free(noti);
}

TEST_F(NotificationTest, CreateFromTemplateNullName) {
  notification_h noti = notification_create_from_template(nullptr);
  EXPECT_EQ(noti, nullptr);
}

TEST_F(NotificationTest, GetNotiBlockStateNullParam) {
  int ret = notification_get_noti_block_state(nullptr);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationTest, LoadByTagNullParam) {
  notification_h noti = notification_load_by_tag(nullptr);
  EXPECT_EQ(noti, nullptr);
}

TEST_F(NotificationTest, SetImageNullNoti) {
  int ret = notification_set_image(nullptr,
      NOTIFICATION_IMAGE_TYPE_ICON, "path");
  EXPECT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationTest, GetImageNullNoti) {
  char* path = nullptr;
  int ret = notification_get_image(nullptr,
      NOTIFICATION_IMAGE_TYPE_ICON, &path);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationTest, SetTextNullNoti) {
  int ret = notification_set_text(nullptr, NOTIFICATION_TEXT_TYPE_TITLE,
      "test", NULL, NOTIFICATION_VARIABLE_TYPE_NONE);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationTest, GetTextNullNoti) {
  char* text = nullptr;
  int ret = notification_get_text(nullptr,
      NOTIFICATION_TEXT_TYPE_TITLE, &text);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationTest, CloneNullNoti) {
  notification_h cloned = nullptr;
  int ret = notification_clone(nullptr, &cloned);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationTest, FreeNullNoti) {
  int ret = notification_free(nullptr);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationTest, GetTypeNullParams) {
  int ret = notification_get_type(nullptr, nullptr);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);

  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);
  ret = notification_get_type(noti, nullptr);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
  notification_free(noti);
}

TEST_F(NotificationTest, PostWithValidNoti) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  notification_set_text(noti, NOTIFICATION_TEXT_TYPE_TITLE,
      "test", NULL, NOTIFICATION_VARIABLE_TYPE_NONE);

  int ret = notification_post(noti);
  EXPECT_NE(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);

  notification_free(noti);
}

TEST_F(NotificationTest, UpdateWithValidNoti) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  int ret = notification_update(noti);
  EXPECT_NE(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);

  notification_free(noti);
}

TEST_F(NotificationTest, DeleteWithValidNoti) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  int ret = notification_delete(noti);
  EXPECT_NE(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);

  notification_free(noti);
}

TEST_F(NotificationTest, DeleteAllWithValidType) {
  int ret = notification_delete_all(NOTIFICATION_TYPE_NOTI);
  EXPECT_NE(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationTest, SetSoundNullNoti) {
  int ret = notification_set_sound(nullptr,
      NOTIFICATION_SOUND_TYPE_DEFAULT, "path");
  EXPECT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationTest, GetSoundNullNoti) {
  notification_sound_type_e type;
  const char* path;
  int ret = notification_get_sound(nullptr, &type, &path);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationTest, SetVibrationNullNoti) {
  int ret = notification_set_vibration(nullptr,
      NOTIFICATION_VIBRATION_TYPE_DEFAULT, "path");
  EXPECT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationTest, GetVibrationNullNoti) {
  notification_vibration_type_e type;
  const char* path;
  int ret = notification_get_vibration(nullptr, &type, &path);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationTest, SetLedNullNoti) {
  int ret = notification_set_led(nullptr,
      NOTIFICATION_LED_OP_ON_CUSTOM_COLOR, 256);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationTest, GetLedNullNoti) {
  notification_led_op_e op;
  int argb;
  int ret = notification_get_led(nullptr, &op, &argb);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationTest, SetPropertyNullNoti) {
  int ret = notification_set_property(nullptr, 0);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationTest, GetPropertyNullNoti) {
  int property;
  int ret = notification_get_property(nullptr, &property);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationTest, SetDisplayApplistNullNoti) {
  int ret = notification_set_display_applist(nullptr, 0);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationTest, GetDisplayApplistNullNoti) {
  int applist;
  int ret = notification_get_display_applist(nullptr, &applist);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationTest, SetLayoutNullNoti) {
  int ret = notification_set_layout(nullptr,
      NOTIFICATION_LY_NOTI_EVENT_SINGLE);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationTest, GetLayoutNullNoti) {
  notification_ly_type_e layout;
  int ret = notification_get_layout(nullptr, &layout);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationTest, SetTagNullNoti) {
  int ret = notification_set_tag(nullptr, "tag");
  EXPECT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationTest, GetTagNullNoti) {
  const char* tag;
  int ret = notification_get_tag(nullptr, &tag);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationTest, SetProgressNullNoti) {
  int ret = notification_set_progress(nullptr, 0.5);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationTest, GetProgressNullNoti) {
  double val;
  int ret = notification_get_progress(nullptr, &val);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationTest, SetSizeNullNoti) {
  int ret = notification_set_size(nullptr, 1.0);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationTest, GetSizeNullNoti) {
  double val;
  int ret = notification_get_size(nullptr, &val);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationTest, SetAutoRemoveNullNoti) {
  int ret = notification_set_auto_remove(nullptr, true);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationTest, GetAutoRemoveNullNoti) {
  bool val;
  int ret = notification_get_auto_remove(nullptr, &val);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationTest, SetExtensionImageSizeNullNoti) {
  int ret = notification_set_extension_image_size(nullptr, 10);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationTest, GetExtensionImageSizeNullNoti) {
  int height;
  int ret = notification_get_extension_image_size(nullptr, &height);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationTest, AddButtonNullNoti) {
  int ret = notification_add_button(nullptr, NOTIFICATION_BUTTON_1);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationTest, RemoveButtonNullNoti) {
  int ret = notification_remove_button(nullptr, NOTIFICATION_BUTTON_1);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationTest, SetLaunchOptionNullNoti) {
  app_control_h app_control;
  app_control_create(&app_control);
  int ret = notification_set_launch_option(nullptr,
      NOTIFICATION_LAUNCH_OPTION_APP_CONTROL, app_control);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
  app_control_destroy(app_control);
}

TEST_F(NotificationTest, GetLaunchOptionNullNoti) {
  app_control_h app_control;
  int ret = notification_get_launch_option(nullptr,
      NOTIFICATION_LAUNCH_OPTION_APP_CONTROL, (void*)&app_control);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationTest, SetEventHandlerNullNoti) {
  app_control_h app_control;
  app_control_create(&app_control);
  int ret = notification_set_event_handler(nullptr,
      NOTIFICATION_EVENT_TYPE_CLICK_ON_BUTTON_1, app_control);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
  app_control_destroy(app_control);
}

TEST_F(NotificationTest, GetEventHandlerNullNoti) {
  app_control_h app_control;
  int ret = notification_get_event_handler(nullptr,
      NOTIFICATION_EVENT_TYPE_CLICK_ON_BUTTON_1, &app_control);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}
