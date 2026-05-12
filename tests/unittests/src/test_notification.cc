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
#include "notification/include/notification_ongoing_flag.h"

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
  EXPECT_EQ(ret, NOTIFICATION_ERROR_IO_ERROR);

  notification_free(noti);
}

TEST_F(NotificationTest, UpdateWithValidNoti) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  int ret = notification_update(noti);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_IO_ERROR);

  notification_free(noti);
}

TEST_F(NotificationTest, DeleteWithValidNoti) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  int ret = notification_delete(noti);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_IO_ERROR);

  notification_free(noti);
}

TEST_F(NotificationTest, DeleteAllWithValidType) {
  int ret = notification_delete_all(NOTIFICATION_TYPE_NOTI);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_IO_ERROR);
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

/* IMG_SET_P_02..P_05: Set/get image for multiple image types */
TEST_F(NotificationTest, SetGetImageThumbnail) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  const char* path = "/opt/usr/data/thumb.png";
  int ret = notification_set_image(noti, NOTIFICATION_IMAGE_TYPE_THUMBNAIL, path);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  char* out = nullptr;
  ret = notification_get_image(noti, NOTIFICATION_IMAGE_TYPE_THUMBNAIL, &out);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  EXPECT_STREQ(out, path);

  notification_free(noti);
}

TEST_F(NotificationTest, SetGetImageSubIcon) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  const char* path = "/opt/usr/data/sub.png";
  int ret = notification_set_image(noti, NOTIFICATION_IMAGE_TYPE_ICON_SUB, path);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  char* out = nullptr;
  ret = notification_get_image(noti, NOTIFICATION_IMAGE_TYPE_ICON_SUB, &out);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  EXPECT_STREQ(out, path);

  notification_free(noti);
}

TEST_F(NotificationTest, SetGetImageBackground) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  const char* path = "/opt/usr/data/bg.png";
  int ret = notification_set_image(noti, NOTIFICATION_IMAGE_TYPE_BACKGROUND, path);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  char* out = nullptr;
  ret = notification_get_image(noti, NOTIFICATION_IMAGE_TYPE_BACKGROUND, &out);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  EXPECT_STREQ(out, path);

  notification_free(noti);
}

TEST_F(NotificationTest, SetGetImageIndicatorIcon) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  const char* path = "/opt/usr/data/indicator.png";
  int ret = notification_set_image(noti, NOTIFICATION_IMAGE_TYPE_ICON_FOR_INDICATOR, path);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  char* out = nullptr;
  ret = notification_get_image(noti, NOTIFICATION_IMAGE_TYPE_ICON_FOR_INDICATOR, &out);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  EXPECT_STREQ(out, path);

  notification_free(noti);
}

TEST_F(NotificationTest, SetGetImageLockIcon) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  const char* path = "/opt/usr/data/lock.png";
  int ret = notification_set_image(noti, NOTIFICATION_IMAGE_TYPE_ICON_FOR_LOCK, path);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  char* out = nullptr;
  ret = notification_get_image(noti, NOTIFICATION_IMAGE_TYPE_ICON_FOR_LOCK, &out);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  EXPECT_STREQ(out, path);

  notification_free(noti);
}

TEST_F(NotificationTest, SetGetImageList1) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  const char* path = "/opt/usr/data/list1.png";
  int ret = notification_set_image(noti, NOTIFICATION_IMAGE_TYPE_LIST_1, path);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  char* out = nullptr;
  ret = notification_get_image(noti, NOTIFICATION_IMAGE_TYPE_LIST_1, &out);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  EXPECT_STREQ(out, path);

  notification_free(noti);
}

TEST_F(NotificationTest, SetGetImageList2Through5) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  const notification_image_type_e types[] = {
      NOTIFICATION_IMAGE_TYPE_LIST_2, NOTIFICATION_IMAGE_TYPE_LIST_3,
      NOTIFICATION_IMAGE_TYPE_LIST_4, NOTIFICATION_IMAGE_TYPE_LIST_5};
  for (auto t : types) {
    int ret = notification_set_image(noti, t, "/opt/usr/data/list.png");
    EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

    char* out = nullptr;
    ret = notification_get_image(noti, t, &out);
    EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
    EXPECT_STREQ(out, "/opt/usr/data/list.png");
  }

  notification_free(noti);
}

TEST_F(NotificationTest, SetGetImageButtonTypes) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  const notification_image_type_e types[] = {
      NOTIFICATION_IMAGE_TYPE_BUTTON_1, NOTIFICATION_IMAGE_TYPE_BUTTON_2,
      NOTIFICATION_IMAGE_TYPE_BUTTON_3, NOTIFICATION_IMAGE_TYPE_BUTTON_4,
      NOTIFICATION_IMAGE_TYPE_BUTTON_5, NOTIFICATION_IMAGE_TYPE_BUTTON_6,
      NOTIFICATION_IMAGE_TYPE_BUTTON_7, NOTIFICATION_IMAGE_TYPE_BUTTON_8,
      NOTIFICATION_IMAGE_TYPE_BUTTON_9, NOTIFICATION_IMAGE_TYPE_BUTTON_10};
  for (auto t : types) {
    int ret = notification_set_image(noti, t, "/opt/usr/data/btn.png");
    EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

    char* out = nullptr;
    ret = notification_get_image(noti, t, &out);
    EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
    EXPECT_NE(out, nullptr);
  }

  notification_free(noti);
}

TEST_F(NotificationTest, SetGetImageExtension) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  int ret = notification_set_image(noti, NOTIFICATION_IMAGE_TYPE_EXTENSION,
      "/opt/usr/data/ext.png");
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  char* out = nullptr;
  ret = notification_get_image(noti, NOTIFICATION_IMAGE_TYPE_EXTENSION, &out);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  EXPECT_STREQ(out, "/opt/usr/data/ext.png");

  notification_free(noti);
}

TEST_F(NotificationTest, SetImageOverwrite) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  notification_set_image(noti, NOTIFICATION_IMAGE_TYPE_ICON, "/first.png");
  notification_set_image(noti, NOTIFICATION_IMAGE_TYPE_ICON, "/second.png");

  char* out = nullptr;
  notification_get_image(noti, NOTIFICATION_IMAGE_TYPE_ICON, &out);
  EXPECT_STREQ(out, "/second.png");

  notification_free(noti);
}

/* TXT_SET: Various text types */
TEST_F(NotificationTest, SetGetTextContent) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  int ret = notification_set_text(noti, NOTIFICATION_TEXT_TYPE_CONTENT,
      "content text", NULL, NOTIFICATION_VARIABLE_TYPE_NONE);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  char* text = nullptr;
  ret = notification_get_text(noti, NOTIFICATION_TEXT_TYPE_CONTENT, &text);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  EXPECT_STREQ(text, "content text");

  notification_free(noti);
}

TEST_F(NotificationTest, SetGetTextInfo1) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  int ret = notification_set_text(noti, NOTIFICATION_TEXT_TYPE_INFO_1,
      "info1", NULL, NOTIFICATION_VARIABLE_TYPE_NONE);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  char* text = nullptr;
  ret = notification_get_text(noti, NOTIFICATION_TEXT_TYPE_INFO_1, &text);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  EXPECT_STREQ(text, "info1");

  notification_free(noti);
}

TEST_F(NotificationTest, SetGetTextInfoSub1) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  int ret = notification_set_text(noti, NOTIFICATION_TEXT_TYPE_INFO_SUB_1,
      "sub info", NULL, NOTIFICATION_VARIABLE_TYPE_NONE);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  char* text = nullptr;
  ret = notification_get_text(noti, NOTIFICATION_TEXT_TYPE_INFO_SUB_1, &text);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  EXPECT_STREQ(text, "sub info");

  notification_free(noti);
}

TEST_F(NotificationTest, SetGetTextMultipleTypes) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  const notification_text_type_e types[] = {
      NOTIFICATION_TEXT_TYPE_INFO_2, NOTIFICATION_TEXT_TYPE_INFO_SUB_2,
      NOTIFICATION_TEXT_TYPE_INFO_3, NOTIFICATION_TEXT_TYPE_INFO_SUB_3,
      NOTIFICATION_TEXT_TYPE_GROUP_TITLE, NOTIFICATION_TEXT_TYPE_GROUP_CONTENT,
      NOTIFICATION_TEXT_TYPE_BUTTON_1, NOTIFICATION_TEXT_TYPE_BUTTON_2,
      NOTIFICATION_TEXT_TYPE_BUTTON_3, NOTIFICATION_TEXT_TYPE_BUTTON_4,
      NOTIFICATION_TEXT_TYPE_BUTTON_5, NOTIFICATION_TEXT_TYPE_BUTTON_6,
      NOTIFICATION_TEXT_TYPE_BUTTON_7, NOTIFICATION_TEXT_TYPE_BUTTON_8,
      NOTIFICATION_TEXT_TYPE_BUTTON_9, NOTIFICATION_TEXT_TYPE_BUTTON_10,
      NOTIFICATION_TEXT_TYPE_CONTENT_EXTENSION};

  for (auto t : types) {
    int ret = notification_set_text(noti, t, "test", NULL,
        NOTIFICATION_VARIABLE_TYPE_NONE);
    EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

    char* text = nullptr;
    ret = notification_get_text(noti, t, &text);
    EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
    EXPECT_NE(text, nullptr);
  }

  notification_free(noti);
}

TEST_F(NotificationTest, SetGetTextInputPlaceholder) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  int ret = notification_set_text(noti, NOTIFICATION_TEXT_TYPE_TEXT_INPUT_PLACEHOLDER,
      "Type here...", NULL, NOTIFICATION_VARIABLE_TYPE_NONE);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  char* text = nullptr;
  ret = notification_get_text(noti, NOTIFICATION_TEXT_TYPE_TEXT_INPUT_PLACEHOLDER, &text);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  EXPECT_STREQ(text, "Type here...");

  notification_free(noti);
}

/* T2T: Time to text for content type */
TEST_F(NotificationTest, SetGetTimeToTextContent) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  time_t now = time(NULL);
  int ret = notification_set_time_to_text(noti,
      NOTIFICATION_TEXT_TYPE_CONTENT, now);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  time_t out = 0;
  ret = notification_get_time_from_text(noti,
      NOTIFICATION_TEXT_TYPE_CONTENT, &out);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(out, now);

  notification_free(noti);
}

TEST_F(NotificationTest, SetTimeToTextNullNoti) {
  int ret = notification_set_time_to_text(nullptr,
      NOTIFICATION_TEXT_TYPE_TITLE, time(NULL));
  EXPECT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationTest, GetTimeFromTextNullNoti) {
  time_t out;
  int ret = notification_get_time_from_text(nullptr,
      NOTIFICATION_TEXT_TYPE_TITLE, &out);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* SND_SET_P_02/P_03: Sound USER_DATA type */
TEST_F(NotificationTest, SetGetSoundUserData) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  int ret = notification_set_sound(noti, NOTIFICATION_SOUND_TYPE_USER_DATA,
      "/opt/usr/data/sound.wav");
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  notification_sound_type_e type;
  const char* path = nullptr;
  ret = notification_get_sound(noti, &type, &path);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(type, NOTIFICATION_SOUND_TYPE_USER_DATA);
  EXPECT_STREQ(path, "/opt/usr/data/sound.wav");

  notification_free(noti);
}

TEST_F(NotificationTest, SetGetSoundNone) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  int ret = notification_set_sound(noti, NOTIFICATION_SOUND_TYPE_NONE, NULL);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  notification_sound_type_e type;
  const char* path = nullptr;
  ret = notification_get_sound(noti, &type, &path);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(type, NOTIFICATION_SOUND_TYPE_NONE);

  notification_free(noti);
}

/* VIB_SET_P_02: Vibration USER_DATA type */
TEST_F(NotificationTest, SetGetVibrationUserData) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  int ret = notification_set_vibration(noti, NOTIFICATION_VIBRATION_TYPE_USER_DATA,
      "/opt/usr/data/vib.ivt");
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  notification_vibration_type_e type;
  const char* path = nullptr;
  ret = notification_get_vibration(noti, &type, &path);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(type, NOTIFICATION_VIBRATION_TYPE_USER_DATA);
  EXPECT_STREQ(path, "/opt/usr/data/vib.ivt");

  notification_free(noti);
}

TEST_F(NotificationTest, SetGetVibrationNone) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  int ret = notification_set_vibration(noti, NOTIFICATION_VIBRATION_TYPE_NONE, NULL);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  notification_vibration_type_e type;
  const char* path = nullptr;
  ret = notification_get_vibration(noti, &type, &path);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(type, NOTIFICATION_VIBRATION_TYPE_NONE);

  notification_free(noti);
}

/* LED_SET_P_02: LED with custom color and time period verification */
TEST_F(NotificationTest, SetGetLedCustomColor) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  int ret = notification_set_led(noti,
      NOTIFICATION_LED_OP_ON_CUSTOM_COLOR, 0xFF00FF);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  notification_led_op_e op;
  int argb = 0;
  ret = notification_get_led(noti, &op, &argb);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(op, NOTIFICATION_LED_OP_ON_CUSTOM_COLOR);
  EXPECT_EQ(argb, 0xFF00FF);

  notification_free(noti);
}

TEST_F(NotificationTest, SetGetLedOff) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  int ret = notification_set_led(noti, NOTIFICATION_LED_OP_OFF, 0);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  notification_led_op_e op;
  int argb = 0;
  ret = notification_get_led(noti, &op, &argb);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(op, NOTIFICATION_LED_OP_OFF);

  notification_free(noti);
}

TEST_F(NotificationTest, SetGetLedTimePeriodValues) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  int ret = notification_set_led_time_period(noti, 500, 1000);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  int on_ms = 0, off_ms = 0;
  ret = notification_get_led_time_period(noti, &on_ms, &off_ms);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(on_ms, 500);
  EXPECT_EQ(off_ms, 1000);

  notification_free(noti);
}

TEST_F(NotificationTest, SetLedTimePeriodNullNoti) {
  int ret = notification_set_led_time_period(nullptr, 100, 200);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationTest, GetLedTimePeriodNullNoti) {
  int on_ms, off_ms;
  int ret = notification_get_led_time_period(nullptr, &on_ms, &off_ms);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* LY_SET: All layout types */
TEST_F(NotificationTest, SetGetLayoutAllTypes) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  const notification_ly_type_e layouts[] = {
      NOTIFICATION_LY_NONE,
      NOTIFICATION_LY_NOTI_EVENT_SINGLE,
      NOTIFICATION_LY_NOTI_EVENT_MULTIPLE,
      NOTIFICATION_LY_NOTI_THUMBNAIL,
      NOTIFICATION_LY_ONGOING_EVENT,
      NOTIFICATION_LY_ONGOING_PROGRESS,
      NOTIFICATION_LY_EXTENSION};

  for (auto ly : layouts) {
    int ret = notification_set_layout(noti, ly);
    EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

    notification_ly_type_e out;
    ret = notification_get_layout(noti, &out);
    EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
    EXPECT_EQ(out, ly);
  }

  notification_free(noti);
}

/* EVT: All event handler types */
TEST_F(NotificationTest, SetGetEventHandlerAllTypes) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  const notification_event_type_e events[] = {
      NOTIFICATION_EVENT_TYPE_CLICK_ON_BUTTON_1,
      NOTIFICATION_EVENT_TYPE_CLICK_ON_BUTTON_2,
      NOTIFICATION_EVENT_TYPE_CLICK_ON_BUTTON_3,
      NOTIFICATION_EVENT_TYPE_CLICK_ON_BUTTON_4,
      NOTIFICATION_EVENT_TYPE_CLICK_ON_BUTTON_5,
      NOTIFICATION_EVENT_TYPE_CLICK_ON_BUTTON_6,
      NOTIFICATION_EVENT_TYPE_CLICK_ON_ICON,
      NOTIFICATION_EVENT_TYPE_CLICK_ON_THUMBNAIL,
      NOTIFICATION_EVENT_TYPE_CLICK_ON_TEXT_INPUT_BUTTON,
      NOTIFICATION_EVENT_TYPE_CLICK_ON_BUTTON_7,
      NOTIFICATION_EVENT_TYPE_CLICK_ON_BUTTON_8,
      NOTIFICATION_EVENT_TYPE_CLICK_ON_BUTTON_9,
      NOTIFICATION_EVENT_TYPE_CLICK_ON_BUTTON_10};

  for (auto ev : events) {
    app_control_h ac;
    app_control_create(&ac);
    app_control_set_app_id(ac, "org.tizen.testapp");
    int ret = notification_set_event_handler(noti, ev, ac);
    EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
    app_control_destroy(ac);

    app_control_h out;
    ret = notification_get_event_handler(noti, ev, &out);
    EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
    app_control_destroy(out);
  }

  notification_free(noti);
}

/* PROP: Multiple property flags */
TEST_F(NotificationTest, SetGetPropertyMultipleFlags) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  int flags = NOTIFICATION_PROP_PERMANENT_DISPLAY |
              NOTIFICATION_PROP_DISABLE_AUTO_DELETE;
  int ret = notification_set_property(noti, flags);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  int out = 0;
  ret = notification_get_property(noti, &out);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(out, flags);

  notification_free(noti);
}

/* DISP_SET_P_02: Multiple display targets */
TEST_F(NotificationTest, SetGetDisplayApplistMultiple) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  int applist = NOTIFICATION_DISPLAY_APP_NOTIFICATION_TRAY |
                NOTIFICATION_DISPLAY_APP_INDICATOR |
                NOTIFICATION_DISPLAY_APP_LOCK;
  int ret = notification_set_display_applist(noti, applist);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  int out = 0;
  ret = notification_get_display_applist(noti, &out);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(out, applist);

  notification_free(noti);
}

/* PROG_SET_P_02: Progress bounds */
TEST_F(NotificationTest, SetGetProgressZero) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  int ret = notification_set_progress(noti, 0.0);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  double val = -1;
  ret = notification_get_progress(noti, &val);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  EXPECT_DOUBLE_EQ(val, 0.0);

  notification_free(noti);
}

TEST_F(NotificationTest, SetGetProgressFull) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  int ret = notification_set_progress(noti, 1.0);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  double val = 0;
  ret = notification_get_progress(noti, &val);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  EXPECT_DOUBLE_EQ(val, 1.0);

  notification_free(noti);
}

TEST_F(NotificationTest, SetGetSizeValues) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  int ret = notification_set_size(noti, 0.0);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  double val = -1;
  ret = notification_get_size(noti, &val);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  EXPECT_DOUBLE_EQ(val, 0.0);

  ret = notification_set_size(noti, 1024.5);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  ret = notification_get_size(noti, &val);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  EXPECT_DOUBLE_EQ(val, 1024.5);

  notification_free(noti);
}

/* CLONE_P_01: Clone with full field verification */
TEST_F(NotificationTest, ClonePreservesAllFields) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  notification_set_text(noti, NOTIFICATION_TEXT_TYPE_TITLE,
      "clone_title", NULL, NOTIFICATION_VARIABLE_TYPE_NONE);
  notification_set_text(noti, NOTIFICATION_TEXT_TYPE_CONTENT,
      "clone_content", NULL, NOTIFICATION_VARIABLE_TYPE_NONE);
  notification_set_image(noti, NOTIFICATION_IMAGE_TYPE_ICON,
      "/opt/usr/data/clone.png");
  notification_set_sound(noti, NOTIFICATION_SOUND_TYPE_USER_DATA,
      "/opt/usr/data/clone.wav");
  notification_set_vibration(noti, NOTIFICATION_VIBRATION_TYPE_DEFAULT, NULL);
  notification_set_led(noti, NOTIFICATION_LED_OP_ON_CUSTOM_COLOR, 0xABCDEF);
  notification_set_led_time_period(noti, 300, 600);
  notification_set_property(noti, NOTIFICATION_PROP_PERMANENT_DISPLAY);
  notification_set_layout(noti, NOTIFICATION_LY_NOTI_EVENT_MULTIPLE);
  notification_set_tag(noti, "clone_tag");
  notification_set_display_applist(noti,
      NOTIFICATION_DISPLAY_APP_NOTIFICATION_TRAY);
  notification_set_progress(noti, 0.75);
  notification_set_size(noti, 512.0);
  notification_set_auto_remove(noti, false);
  notification_set_extension_image_size(noti, 50);

  notification_h cloned = nullptr;
  int ret = notification_clone(noti, &cloned);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);
  ASSERT_NE(cloned, nullptr);

  char* title = nullptr;
  notification_get_text(cloned, NOTIFICATION_TEXT_TYPE_TITLE, &title);
  EXPECT_STREQ(title, "clone_title");

  char* content = nullptr;
  notification_get_text(cloned, NOTIFICATION_TEXT_TYPE_CONTENT, &content);
  EXPECT_STREQ(content, "clone_content");

  char* icon = nullptr;
  notification_get_image(cloned, NOTIFICATION_IMAGE_TYPE_ICON, &icon);
  EXPECT_STREQ(icon, "/opt/usr/data/clone.png");

  notification_sound_type_e snd_type;
  const char* snd_path = nullptr;
  notification_get_sound(cloned, &snd_type, &snd_path);
  EXPECT_EQ(snd_type, NOTIFICATION_SOUND_TYPE_USER_DATA);

  notification_led_op_e led_op;
  int led_argb = 0;
  notification_get_led(cloned, &led_op, &led_argb);
  EXPECT_EQ(led_op, NOTIFICATION_LED_OP_ON_CUSTOM_COLOR);
  EXPECT_EQ(led_argb, 0xABCDEF);

  int on_ms = 0, off_ms = 0;
  notification_get_led_time_period(cloned, &on_ms, &off_ms);
  EXPECT_EQ(on_ms, 300);
  EXPECT_EQ(off_ms, 600);

  int prop = 0;
  notification_get_property(cloned, &prop);
  EXPECT_EQ(prop, NOTIFICATION_PROP_PERMANENT_DISPLAY);

  notification_ly_type_e ly;
  notification_get_layout(cloned, &ly);
  EXPECT_EQ(ly, NOTIFICATION_LY_NOTI_EVENT_MULTIPLE);

  const char* tag = nullptr;
  notification_get_tag(cloned, &tag);
  EXPECT_STREQ(tag, "clone_tag");

  double prog = 0;
  notification_get_progress(cloned, &prog);
  EXPECT_DOUBLE_EQ(prog, 0.75);

  double sz = 0;
  notification_get_size(cloned, &sz);
  EXPECT_DOUBLE_EQ(sz, 512.0);

  bool auto_rm = true;
  notification_get_auto_remove(cloned, &auto_rm);
  EXPECT_FALSE(auto_rm);

  int ext_sz = 0;
  notification_get_extension_image_size(cloned, &ext_sz);
  EXPECT_EQ(ext_sz, 50);

  notification_free(cloned);
  notification_free(noti);
}

/* TAG: Set then overwrite */
TEST_F(NotificationTest, SetTagOverwrite) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  notification_set_tag(noti, "mytag");
  int ret = notification_set_tag(noti, "newtag");
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  const char* tag = nullptr;
  ret = notification_get_tag(noti, &tag);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  EXPECT_STREQ(tag, "newtag");

  notification_free(noti);
}

/* BTN: All button indices */
TEST_F(NotificationTest, AddRemoveButtonAllIndices) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  const notification_button_index_e buttons[] = {
      NOTIFICATION_BUTTON_1, NOTIFICATION_BUTTON_2, NOTIFICATION_BUTTON_3,
      NOTIFICATION_BUTTON_4, NOTIFICATION_BUTTON_5, NOTIFICATION_BUTTON_6,
      NOTIFICATION_BUTTON_7, NOTIFICATION_BUTTON_8, NOTIFICATION_BUTTON_9,
      NOTIFICATION_BUTTON_10};
  for (auto btn : buttons) {
    int ret = notification_add_button(noti, btn);
    EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  }
  for (auto btn : buttons) {
    int ret = notification_remove_button(noti, btn);
    EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  }

  notification_free(noti);
}

/* CREATE: Type ONGOING */
TEST_F(NotificationTest, CreateOngoing) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_ONGOING);
  ASSERT_NE(noti, nullptr);

  notification_type_e type;
  int ret = notification_get_type(noti, &type);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(type, NOTIFICATION_TYPE_ONGOING);

  notification_free(noti);
}

/* AutoRemove: false then true */
TEST_F(NotificationTest, SetAutoRemoveFalseThenTrue) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  notification_set_auto_remove(noti, false);
  bool val = true;
  notification_get_auto_remove(noti, &val);
  EXPECT_FALSE(val);

  notification_set_auto_remove(noti, true);
  notification_get_auto_remove(noti, &val);
  EXPECT_TRUE(val);

  notification_free(noti);
}

/* GetTimeNullOutput */
TEST_F(NotificationTest, GetTimeNullOutput) {
  int ret = notification_get_time(nullptr, nullptr);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationTest, GetInsertTimeNullOutput) {
  int ret = notification_get_insert_time(nullptr, nullptr);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* SetTime DoNotShow */
TEST_F(NotificationTest, SetTimeDoNotShow) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  int ret = notification_set_time(noti, NOTIFICATION_DO_NOT_SHOW_TIME_STAMP);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  time_t out = 0;
  ret = notification_get_time(noti, &out);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(out, NOTIFICATION_DO_NOT_SHOW_TIME_STAMP);

  notification_free(noti);
}

/* SetTextInput value */
TEST_F(NotificationTest, SetTextInputMaxLength) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  int ret = notification_set_text_input(noti, 500);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  notification_free(noti);
}

TEST_F(NotificationTest, SetTextInputNullNoti) {
  int ret = notification_set_text_input(nullptr, 100);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationTest, SetTimeNullNoti) {
  int ret = notification_set_time(nullptr, 0);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationTest, GetTimeNullNoti) {
  time_t t;
  int ret = notification_get_time(nullptr, &t);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationTest, GetInsertTimeNullNoti) {
  time_t t;
  int ret = notification_get_insert_time(nullptr, &t);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationTest, GetPkgnameNullNoti) {
  char* pkg;
  int ret = notification_get_pkgname(nullptr, &pkg);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* 1. GetImageNullOutputPointer */
TEST_F(NotificationTest, GetImageNullOutputPointer) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  int ret = notification_get_image(noti, NOTIFICATION_IMAGE_TYPE_ICON, NULL);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);

  notification_free(noti);
}

/* 2. GetTimeNullRetTimePointer */
TEST_F(NotificationTest, GetTimeNullRetTimePointer) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  int ret = notification_get_time(noti, NULL);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);

  notification_free(noti);
}

/* 3. GetTextNullOutputPointer */
TEST_F(NotificationTest, GetTextNullOutputPointer) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  int ret = notification_get_text(noti, NOTIFICATION_TEXT_TYPE_TITLE, NULL);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);

  notification_free(noti);
}

/* 4. GetSoundNullTypeOutput */
TEST_F(NotificationTest, GetSoundNullTypeOutput) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  int ret = notification_get_sound(noti, NULL, NULL);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);

  notification_free(noti);
}

/* 5. GetVibrationNullTypeOutput */
TEST_F(NotificationTest, GetVibrationNullTypeOutput) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  int ret = notification_get_vibration(noti, NULL, NULL);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);

  notification_free(noti);
}

/* 6. GetLedNullOperationOutput */
TEST_F(NotificationTest, GetLedNullOperationOutput) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  int ret = notification_get_led(noti, NULL, NULL);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);

  notification_free(noti);
}

/* 7. GetLedTimePeriodNullOutput — both on_ms and off_ms are optional;
 *    the function only guards against noti == NULL, so NULL,NULL returns NONE. */
TEST_F(NotificationTest, GetLedTimePeriodNullOutput) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  int ret = notification_get_led_time_period(noti, NULL, NULL);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  notification_free(noti);
}

/* 8. GetLaunchOptionNullOutput */
TEST_F(NotificationTest, GetLaunchOptionNullOutput) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  int ret = notification_get_launch_option(noti,
      NOTIFICATION_LAUNCH_OPTION_APP_CONTROL, NULL);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);

  notification_free(noti);
}

/* 9. GetEventHandlerNullOutput */
TEST_F(NotificationTest, GetEventHandlerNullOutput) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  int ret = notification_get_event_handler(noti,
      NOTIFICATION_EVENT_TYPE_CLICK_ON_BUTTON_1, NULL);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);

  notification_free(noti);
}

/* 10. GetPropertyNullOutput */
TEST_F(NotificationTest, GetPropertyNullOutput) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  int ret = notification_get_property(noti, NULL);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);

  notification_free(noti);
}

/* 11. GetDisplayApplistNullOutput */
TEST_F(NotificationTest, GetDisplayApplistNullOutput) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  int ret = notification_get_display_applist(noti, NULL);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);

  notification_free(noti);
}

/* 12. GetSizeNullOutput */
TEST_F(NotificationTest, GetSizeNullOutput) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  int ret = notification_get_size(noti, NULL);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);

  notification_free(noti);
}

/* 13. GetProgressNullOutput */
TEST_F(NotificationTest, GetProgressNullOutput) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  int ret = notification_get_progress(noti, NULL);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);

  notification_free(noti);
}

/* 14. GetPkgnameNullOutput */
TEST_F(NotificationTest, GetPkgnameNullOutput) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  int ret = notification_get_pkgname(noti, NULL);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);

  notification_free(noti);
}

/* 15. GetLayoutNullOutput */
TEST_F(NotificationTest, GetLayoutNullOutput) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  int ret = notification_get_layout(noti, NULL);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);

  notification_free(noti);
}

/* 16. GetTagNullOutput — notification_get_tag only checks noti == NULL;
 *     passing NULL tag pointer would dereference it, so we verify the
 *     function succeeds when tag was never set (returns NULL value). */
TEST_F(NotificationTest, GetTagNullOutput) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  const char* tag = reinterpret_cast<const char*>(0xDEAD);
  int ret = notification_get_tag(noti, &tag);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(tag, nullptr);

  notification_free(noti);
}

/* 17. GetAutoRemoveNullOutput */
TEST_F(NotificationTest, GetAutoRemoveNullOutput) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  int ret = notification_get_auto_remove(noti, NULL);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);

  notification_free(noti);
}

/* 18. GetExtensionImageSizeNullOutputHeight */
TEST_F(NotificationTest, GetExtensionImageSizeNullOutputHeight) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  int ret = notification_get_extension_image_size(noti, NULL);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);

  notification_free(noti);
}

/* 19. SetImageNullPathClearsExisting */
TEST_F(NotificationTest, SetImageNullPathClearsExisting) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  notification_set_image(noti, NOTIFICATION_IMAGE_TYPE_ICON, "/test.png");
  notification_set_image(noti, NOTIFICATION_IMAGE_TYPE_ICON, NULL);

  char* path = reinterpret_cast<char*>(0xDEAD);
  notification_get_image(noti, NOTIFICATION_IMAGE_TYPE_ICON, &path);
  EXPECT_EQ(path, nullptr);

  notification_free(noti);
}

/* 20. GetImageWhenNoneSet */
TEST_F(NotificationTest, GetImageWhenNoneSet) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  char* path = reinterpret_cast<char*>(0xDEAD);
  int ret = notification_get_image(noti, NOTIFICATION_IMAGE_TYPE_ICON, &path);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  /* path may be NULL or point to app_icon_path fallback */

  notification_free(noti);
}

/* 21. SetTextOverwrite */
TEST_F(NotificationTest, SetTextOverwrite) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  notification_set_text(noti, NOTIFICATION_TEXT_TYPE_TITLE,
      "first", NULL, NOTIFICATION_VARIABLE_TYPE_NONE);
  int ret = notification_set_text(noti, NOTIFICATION_TEXT_TYPE_TITLE,
      "second", NULL, NOTIFICATION_VARIABLE_TYPE_NONE);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  char* text = nullptr;
  ret = notification_get_text(noti, NOTIFICATION_TEXT_TYPE_TITLE, &text);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  EXPECT_STREQ(text, "second");

  notification_free(noti);
}

/* 22. SetTextNullClears */
TEST_F(NotificationTest, SetTextNullClears) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  notification_set_text(noti, NOTIFICATION_TEXT_TYPE_TITLE,
      "hello", NULL, NOTIFICATION_VARIABLE_TYPE_NONE);
  int ret = notification_set_text(noti, NOTIFICATION_TEXT_TYPE_TITLE,
      NULL, NULL, NOTIFICATION_VARIABLE_TYPE_NONE);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  char* text = nullptr;
  ret = notification_get_text(noti, NOTIFICATION_TEXT_TYPE_TITLE, &text);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(text, nullptr);

  notification_free(noti);
}

/* 23. SetTagNullDoesNotCrash — implementation keeps old tag on NULL set */
TEST_F(NotificationTest, SetTagNullDoesNotCrash) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  int ret = notification_set_tag(noti, "test");
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  ret = notification_set_tag(noti, NULL);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  const char* tag = nullptr;
  ret = notification_get_tag(noti, &tag);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  notification_free(noti);
}

/* 24. CreateInvalidTypeNegative */
TEST_F(NotificationTest, CreateInvalidTypeNegative) {
  notification_h noti = notification_create((notification_type_e)-1);
  EXPECT_EQ(noti, nullptr);
}

/* 25. CreateInvalidTypeOverMax */
TEST_F(NotificationTest, CreateInvalidTypeOverMax) {
  notification_h noti = notification_create((notification_type_e)999);
  EXPECT_EQ(noti, nullptr);
}

/* 26. SetOngoingFlagTrue */
TEST_F(NotificationTest, SetOngoingFlagTrue) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  int ret = notification_set_ongoing_flag(noti, true);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  bool flag = false;
  ret = notification_get_ongoing_flag(noti, &flag);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  EXPECT_TRUE(flag);

  notification_free(noti);
}

/* 27. SetOngoingFlagFalse */
TEST_F(NotificationTest, SetOngoingFlagFalse) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  int ret = notification_set_ongoing_flag(noti, false);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  bool flag = true;
  ret = notification_get_ongoing_flag(noti, &flag);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  EXPECT_FALSE(flag);

  notification_free(noti);
}

/* 28. SetOngoingFlagNullNoti */
TEST_F(NotificationTest, SetOngoingFlagNullNoti) {
  int ret = notification_set_ongoing_flag(NULL, true);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* 29. GetOngoingFlagNullNoti */
TEST_F(NotificationTest, GetOngoingFlagNullNoti) {
  bool flag = false;
  int ret = notification_get_ongoing_flag(NULL, &flag);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* 30. GetOngoingFlagNullOutput */
TEST_F(NotificationTest, GetOngoingFlagNullOutput) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  int ret = notification_get_ongoing_flag(noti, NULL);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);

  notification_free(noti);
}

/* 31. SetTimeToCurrent */
TEST_F(NotificationTest, SetTimeToCurrent) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  int ret = notification_set_time(noti, 0);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  time_t out = 0;
  ret = notification_get_time(noti, &out);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  EXPECT_NE(out, 0);

  notification_free(noti);
}

/* 32. SetTimeNegative */
TEST_F(NotificationTest, SetTimeNegative) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  int ret = notification_set_time(noti, (time_t)-1);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  notification_free(noti);
}

/* 33. SetSizeNegative */
TEST_F(NotificationTest, SetSizeNegative) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  int ret = notification_set_size(noti, -1.0);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  double val = 0.0;
  ret = notification_get_size(noti, &val);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  EXPECT_DOUBLE_EQ(val, -1.0);

  notification_free(noti);
}

/* 34. SetProgressOverOne */
TEST_F(NotificationTest, SetProgressOverOne) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  int ret = notification_set_progress(noti, 1.5);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  double val = 0.0;
  ret = notification_get_progress(noti, &val);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  EXPECT_DOUBLE_EQ(val, 1.5);

  notification_free(noti);
}

/* 35. SetProgressNegative */
TEST_F(NotificationTest, SetProgressNegative) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  int ret = notification_set_progress(noti, -0.1);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  double val = 0.0;
  ret = notification_get_progress(noti, &val);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  EXPECT_DOUBLE_EQ(val, -0.1);

  notification_free(noti);
}

/* 36. SetLedTimePeriodZero */
TEST_F(NotificationTest, SetLedTimePeriodZero) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  int ret = notification_set_led_time_period(noti, 0, 0);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  int on_ms = -1, off_ms = -1;
  ret = notification_get_led_time_period(noti, &on_ms, &off_ms);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(on_ms, 0);
  EXPECT_EQ(off_ms, 0);

  notification_free(noti);
}

/* 37. SetTextEmptyString */
TEST_F(NotificationTest, SetTextEmptyString) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  int ret = notification_set_text(noti, NOTIFICATION_TEXT_TYPE_TITLE,
      "", NULL, NOTIFICATION_VARIABLE_TYPE_NONE);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  char* text = nullptr;
  ret = notification_get_text(noti, NOTIFICATION_TEXT_TYPE_TITLE, &text);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  EXPECT_STREQ(text, "");

  notification_free(noti);
}

/* 38. SetExtensionImageSizeZero — height=0 is rejected as invalid */
TEST_F(NotificationTest, SetExtensionImageSizeZero) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  int ret = notification_set_extension_image_size(noti, 0);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);

  notification_free(noti);
}

/* 39. AddButtonInvalidIndex */
TEST_F(NotificationTest, AddButtonInvalidIndex) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  int ret = notification_add_button(noti, (notification_button_index_e)0);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);

  ret = notification_add_button(noti, (notification_button_index_e)99);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);

  notification_free(noti);
}

/* 40. RemoveButtonNotAdded — remove_button only checks validity of index
 *     and noti handle; removing an index that was not added returns NONE. */
TEST_F(NotificationTest, RemoveButtonNotAdded) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  int ret = notification_remove_button(noti, NOTIFICATION_BUTTON_1);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  notification_free(noti);
}
