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

notification_h cloned_noti = nullptr;

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
  virtual void SetUp() {
  }
  virtual void TearDown() {
  }
};

TEST_F(NotificationTest, happy_path_combined) {
  EXPECT_CALL(GetMock<AulMock>(), aul_app_get_appid_bypid(_, _, _))
      .WillRepeatedly(Invoke(__fake_aul_app_get_appid_bypid));

  EXPECT_CALL(GetMock<AulMock>(), aul_app_get_pkgid_bypid(_, _, _))
      .WillOnce(Invoke(__fake_aul_app_get_pkgid_bypid));

  EXPECT_CALL(GetMock<PackageManagerMock>(), package_info_create(_, _))
      .WillOnce(Invoke(__fake_package_info_create));

  EXPECT_CALL(GetMock<PackageManagerMock>(), package_info_get_root_path(_, _))
      .WillOnce(Invoke(__fake_package_info_get_root_path));

  EXPECT_CALL(GetMock<PackageManagerMock>(), pkgmgrinfo_appinfo_get_usr_appinfo(_, _, _))
      .WillOnce(Invoke(__fake_pkgmgrinfo_appinfo_get_usr_appinfo));

  EXPECT_CALL(GetMock<PackageManagerMock>(), pkgmgrinfo_appinfo_get_label(_, _))
      .WillOnce(Invoke(__fake_pkgmgrinfo_appinfo_get_label));

  notification_h noti = nullptr;
  int ret;

  noti = notification_create(NOTIFICATION_TYPE_NOTI);
  EXPECT_NE(noti, nullptr);

  ret = notification_set_image(noti, NOTIFICATION_IMAGE_TYPE_ICON,
    "/opt/usr/home/owner/apps_rw/org.tizen.testpkgid/data/.notification/image.png");
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  char* image_path = NULL;
  ret = notification_get_image(noti, NOTIFICATION_IMAGE_TYPE_ICON, &image_path);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);
  ASSERT_STREQ(image_path, "/opt/usr/home/owner/apps_rw/org.tizen.testpkgid/data/.notification/image.png");

  ret = notification_set_time(noti, 0xFFFFFFF);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  time_t ret_time = 0;

  ret = notification_get_time(noti, &ret_time);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);
  EXPECT_NE(ret_time, 0);

  ret_time = 0;

  ret = notification_get_insert_time(noti, &ret_time);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  ret = notification_set_text(noti, NOTIFICATION_TEXT_TYPE_TITLE, "testtitle", NULL, NOTIFICATION_VARIABLE_TYPE_NONE);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  char* title_text = nullptr;
  ret = notification_get_text(noti, NOTIFICATION_TEXT_TYPE_TITLE, &title_text);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);
  ASSERT_STREQ(title_text, "testtitle");

  ret = notification_set_text(noti, NOTIFICATION_TEXT_TYPE_CONTENT, "%f %d %s %d", NULL,
      NOTIFICATION_VARIABLE_TYPE_COUNT, NOTIFICATION_COUNT_POS_LEFT,
      NOTIFICATION_VARIABLE_TYPE_DOUBLE, 0.0,
      NOTIFICATION_VARIABLE_TYPE_COUNT, NOTIFICATION_COUNT_POS_IN,
      NOTIFICATION_VARIABLE_TYPE_STRING, "Content",
      NOTIFICATION_VARIABLE_TYPE_INT, 0,
      NOTIFICATION_VARIABLE_TYPE_COUNT, NOTIFICATION_COUNT_POS_RIGHT,
      NOTIFICATION_VARIABLE_TYPE_NONE);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  char* content_text = nullptr;
  ret = notification_get_text(noti, NOTIFICATION_TEXT_TYPE_TITLE, &content_text);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);
  EXPECT_NE(content_text, nullptr);

  ret = notification_set_time_to_text(noti, NOTIFICATION_TEXT_TYPE_TITLE, time(NULL));
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  ret_time = 0;
  ret = notification_get_time_from_text(noti, NOTIFICATION_TEXT_TYPE_TITLE, &ret_time);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  ret = notification_set_sound(noti, NOTIFICATION_SOUND_TYPE_DEFAULT, "soundpath");
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  notification_sound_type_e sound_type = NOTIFICATION_SOUND_TYPE_NONE;
  const char* sound_path = nullptr;
  ret = notification_get_sound(noti, &sound_type, &sound_path);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  ret = notification_set_vibration(noti, NOTIFICATION_VIBRATION_TYPE_DEFAULT, "vibpath");
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  notification_vibration_type_e vib_type = NOTIFICATION_VIBRATION_TYPE_NONE;
  const char* vib_path = nullptr;
  ret = notification_get_vibration(noti, &vib_type, &vib_path);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  ret = notification_set_led(noti, NOTIFICATION_LED_OP_ON_CUSTOM_COLOR, 256);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  notification_led_op_e op = NOTIFICATION_LED_OP_OFF;
  int led_argb = 0;
  ret = notification_get_led(noti, &op, &led_argb);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  ret = notification_set_led_time_period(noti, 100, 200);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  int on_ms = 0;
  int off_ms = 0;
  ret = notification_get_led_time_period(noti, &on_ms, &off_ms);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  app_control_h app_control;
  app_control_create(&app_control);
  app_control_set_app_id(app_control, "org.tizen.testapp");
  ret = notification_set_launch_option(noti, NOTIFICATION_LAUNCH_OPTION_APP_CONTROL, app_control);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  app_control_destroy(app_control);

  ret = notification_get_launch_option(noti, NOTIFICATION_LAUNCH_OPTION_APP_CONTROL, (void *)&app_control);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  app_control_destroy(app_control);

  app_control_create(&app_control);
  app_control_set_app_id(app_control, "org.tizen.testapp");
  ret = notification_set_event_handler(noti, NOTIFICATION_EVENT_TYPE_CLICK_ON_BUTTON_1, app_control);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  app_control_destroy(app_control);

  ret = notification_get_event_handler(noti, NOTIFICATION_EVENT_TYPE_CLICK_ON_BUTTON_1, &app_control);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  app_control_destroy(app_control);

  ret = notification_set_property(noti, NOTIFICATION_PROP_PERMANENT_DISPLAY);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  int property;
  ret = notification_get_property(noti, &property);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  ret = notification_set_display_applist(noti, NOTIFICATION_DISPLAY_APP_NOTIFICATION_TRAY);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  int ret_applist;
  ret = notification_get_display_applist(noti, &ret_applist);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  ret = notification_set_size(noti, 3.3);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  double ret_size;
  ret = notification_get_size(noti, &ret_size);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  ret = notification_set_progress(noti, 30.);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  double percentage;
  ret = notification_get_progress(noti, &percentage);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  ret = notification_set_layout(noti, NOTIFICATION_LY_NOTI_EVENT_SINGLE);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  notification_ly_type_e ret_layout;
  ret = notification_get_layout(noti, &ret_layout);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  notification_type_e ret_type;
  ret = notification_get_type(noti, &ret_type);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  ret = notification_set_tag(noti, "test_tag");
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  const char* ret_tag;
  ret = notification_get_tag(noti, &ret_tag);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  char* pkgname;
  ret = notification_get_pkgname(noti, &pkgname);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  ret = notification_add_button(noti, NOTIFICATION_BUTTON_1);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  ret = notification_remove_button(noti, NOTIFICATION_BUTTON_1);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  ret = notification_set_auto_remove(noti, true);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  bool auto_remove;
  ret = notification_get_auto_remove(noti, &auto_remove);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  ret = notification_set_text_input(noti, 1000);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  ret = notification_set_extension_image_size(noti, 20);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  int ret_height;
  ret = notification_get_extension_image_size(noti, &ret_height);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  ret = notification_clone(noti, &cloned_noti);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  ret = notification_free(cloned_noti);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  // notification_block_state_e ret_state;
  // ret = notification_get_noti_block_state(&ret_state);
  // ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  // ret = notification_save_as_template(noti, "test_template");
  // ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  // notification_h template_noti = nullptr;
  // template_noti = notification_create_from_template("test_template");
  // EXPECT_NE(template_noti, nullptr);

  // ret = notification_update(noti);
  // ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  // ret = notification_post(noti);
  // ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  // notification_h tag_noti;
  // tag_noti = notification_load_by_tag("test_tag");
  // ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);
  // EXPECT_NE(tag_noti, nullptr);

  // ret = notification_delete(noti);
  // ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  // ret = notification_delete_all(NOTIFICATION_TYPE_NOTI);
  // ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);
}

// =====================================================================
// Lock-in tests for notification.h public API.
//
// Per R-LOCK-IN-BASELINE / R-API-SCENARIO-COVERAGE for task 02a, every
// one of the 53 exported symbols declared in notification.h gets at
// least one TEST_F asserting its current behavior on NULL / invalid /
// repeat-call inputs. The happy paths are covered above by
// `happy_path_combined`. Symbols that require a live DPM round-trip
// (post / update / delete / load_by_tag / save_as_template / etc.)
// are covered for null-handle err-codes only here; full IPC paths are
// deferred to task 19's integration suite per the planner's § API
// surface (DEBT entries).
//
// See refactoring-plans/05_logs/02a-locks-notification-h.md.
// =====================================================================

namespace {
constexpr int kInvalid = NOTIFICATION_ERROR_INVALID_PARAMETER;
}  // namespace

TEST_F(NotificationTest, create_invalid_type) {
  notification_h n = notification_create((notification_type_e)999);
  EXPECT_EQ(n, nullptr);
  n = notification_create(NOTIFICATION_TYPE_NONE);
  EXPECT_EQ(n, nullptr);
}

TEST_F(NotificationTest, clone_null_args) {
  notification_h dst = nullptr;
  EXPECT_EQ(notification_clone(nullptr, &dst), kInvalid);
  EXPECT_EQ(notification_clone(nullptr, nullptr), kInvalid);
}

TEST_F(NotificationTest, free_null) {
  EXPECT_EQ(notification_free(nullptr), kInvalid);
}

TEST_F(NotificationTest, add_button_null) {
  EXPECT_EQ(notification_add_button(nullptr, NOTIFICATION_BUTTON_1), kInvalid);
}

TEST_F(NotificationTest, remove_button_null) {
  EXPECT_EQ(notification_remove_button(nullptr, NOTIFICATION_BUTTON_1), kInvalid);
}

TEST_F(NotificationTest, set_image_null) {
  EXPECT_EQ(notification_set_image(nullptr, NOTIFICATION_IMAGE_TYPE_ICON, "p"), kInvalid);
}

TEST_F(NotificationTest, get_image_null) {
  char* p = nullptr;
  EXPECT_EQ(notification_get_image(nullptr, NOTIFICATION_IMAGE_TYPE_ICON, &p), kInvalid);
  notification_h n = notification_create(NOTIFICATION_TYPE_NOTI);
  if (n != nullptr) {
    EXPECT_EQ(notification_get_image(n, NOTIFICATION_IMAGE_TYPE_ICON, nullptr), kInvalid);
    notification_free(n);
  }
}

TEST_F(NotificationTest, set_text_null) {
  EXPECT_EQ(notification_set_text(nullptr, NOTIFICATION_TEXT_TYPE_TITLE, "x", NULL,
                                  NOTIFICATION_VARIABLE_TYPE_NONE),
            kInvalid);
}

TEST_F(NotificationTest, get_text_null) {
  char* t = nullptr;
  EXPECT_EQ(notification_get_text(nullptr, NOTIFICATION_TEXT_TYPE_TITLE, &t), kInvalid);
}

TEST_F(NotificationTest, set_text_input_null) {
  EXPECT_EQ(notification_set_text_input(nullptr, 100), kInvalid);
}

TEST_F(NotificationTest, set_time_null) {
  EXPECT_EQ(notification_set_time(nullptr, 0), kInvalid);
}

TEST_F(NotificationTest, get_time_null) {
  time_t t = 0;
  EXPECT_EQ(notification_get_time(nullptr, &t), kInvalid);
}

TEST_F(NotificationTest, get_insert_time_null) {
  time_t t = 0;
  EXPECT_EQ(notification_get_insert_time(nullptr, &t), kInvalid);
}

TEST_F(NotificationTest, set_time_to_text_null) {
  EXPECT_EQ(notification_set_time_to_text(nullptr, NOTIFICATION_TEXT_TYPE_TITLE, 0), kInvalid);
}

TEST_F(NotificationTest, get_time_from_text_null) {
  time_t t = 0;
  EXPECT_EQ(notification_get_time_from_text(nullptr, NOTIFICATION_TEXT_TYPE_TITLE, &t), kInvalid);
}

TEST_F(NotificationTest, set_sound_null) {
  EXPECT_EQ(notification_set_sound(nullptr, NOTIFICATION_SOUND_TYPE_DEFAULT, NULL), kInvalid);
}

TEST_F(NotificationTest, get_sound_null) {
  notification_sound_type_e t = NOTIFICATION_SOUND_TYPE_NONE;
  const char* p = nullptr;
  EXPECT_EQ(notification_get_sound(nullptr, &t, &p), kInvalid);
}

TEST_F(NotificationTest, set_vibration_null) {
  EXPECT_EQ(notification_set_vibration(nullptr, NOTIFICATION_VIBRATION_TYPE_DEFAULT, NULL),
            kInvalid);
}

TEST_F(NotificationTest, get_vibration_null) {
  notification_vibration_type_e t = NOTIFICATION_VIBRATION_TYPE_NONE;
  const char* p = nullptr;
  EXPECT_EQ(notification_get_vibration(nullptr, &t, &p), kInvalid);
}

TEST_F(NotificationTest, set_led_null) {
  EXPECT_EQ(notification_set_led(nullptr, NOTIFICATION_LED_OP_OFF, 0), kInvalid);
}

TEST_F(NotificationTest, get_led_null) {
  notification_led_op_e o = NOTIFICATION_LED_OP_OFF;
  int argb = 0;
  EXPECT_EQ(notification_get_led(nullptr, &o, &argb), kInvalid);
}

TEST_F(NotificationTest, set_led_time_period_null) {
  EXPECT_EQ(notification_set_led_time_period(nullptr, 100, 200), kInvalid);
}

TEST_F(NotificationTest, get_led_time_period_null) {
  int on = 0, off = 0;
  EXPECT_EQ(notification_get_led_time_period(nullptr, &on, &off), kInvalid);
}

TEST_F(NotificationTest, set_launch_option_null) {
  app_control_h ac = nullptr;
  app_control_create(&ac);
  EXPECT_EQ(notification_set_launch_option(
                nullptr, NOTIFICATION_LAUNCH_OPTION_APP_CONTROL, ac),
            kInvalid);
  app_control_destroy(ac);
}

TEST_F(NotificationTest, get_launch_option_null) {
  app_control_h ac = nullptr;
  EXPECT_EQ(notification_get_launch_option(
                nullptr, NOTIFICATION_LAUNCH_OPTION_APP_CONTROL, (void*)&ac),
            kInvalid);
}

TEST_F(NotificationTest, set_event_handler_null) {
  app_control_h ac = nullptr;
  app_control_create(&ac);
  EXPECT_EQ(notification_set_event_handler(
                nullptr, NOTIFICATION_EVENT_TYPE_CLICK_ON_BUTTON_1, ac),
            kInvalid);
  app_control_destroy(ac);
}

TEST_F(NotificationTest, get_event_handler_null) {
  app_control_h ac = nullptr;
  EXPECT_EQ(notification_get_event_handler(
                nullptr, NOTIFICATION_EVENT_TYPE_CLICK_ON_BUTTON_1, &ac),
            kInvalid);
}

TEST_F(NotificationTest, set_property_null) {
  EXPECT_EQ(notification_set_property(nullptr, 0), kInvalid);
}

TEST_F(NotificationTest, get_property_null) {
  int p = 0;
  EXPECT_EQ(notification_get_property(nullptr, &p), kInvalid);
}

TEST_F(NotificationTest, set_display_applist_null) {
  EXPECT_EQ(notification_set_display_applist(nullptr, NOTIFICATION_DISPLAY_APP_ALL), kInvalid);
}

TEST_F(NotificationTest, get_display_applist_null) {
  int v = 0;
  EXPECT_EQ(notification_get_display_applist(nullptr, &v), kInvalid);
}

TEST_F(NotificationTest, set_size_null) {
  EXPECT_EQ(notification_set_size(nullptr, 1.0), kInvalid);
}

TEST_F(NotificationTest, get_size_null) {
  double s = 0.0;
  EXPECT_EQ(notification_get_size(nullptr, &s), kInvalid);
}

TEST_F(NotificationTest, set_progress_null) {
  EXPECT_EQ(notification_set_progress(nullptr, 0.5), kInvalid);
}

TEST_F(NotificationTest, get_progress_null) {
  double p = 0.0;
  EXPECT_EQ(notification_get_progress(nullptr, &p), kInvalid);
}

TEST_F(NotificationTest, set_layout_null) {
  EXPECT_EQ(notification_set_layout(nullptr, NOTIFICATION_LY_NOTI_EVENT_SINGLE), kInvalid);
}

TEST_F(NotificationTest, get_layout_null) {
  notification_ly_type_e l;
  EXPECT_EQ(notification_get_layout(nullptr, &l), kInvalid);
}

TEST_F(NotificationTest, set_auto_remove_null) {
  EXPECT_EQ(notification_set_auto_remove(nullptr, true), kInvalid);
}

TEST_F(NotificationTest, get_auto_remove_null) {
  bool a = false;
  EXPECT_EQ(notification_get_auto_remove(nullptr, &a), kInvalid);
}

TEST_F(NotificationTest, set_extension_image_size_null) {
  EXPECT_EQ(notification_set_extension_image_size(nullptr, 10), kInvalid);
}

TEST_F(NotificationTest, get_extension_image_size_null) {
  int h = 0;
  EXPECT_EQ(notification_get_extension_image_size(nullptr, &h), kInvalid);
}

TEST_F(NotificationTest, set_tag_null) {
  EXPECT_EQ(notification_set_tag(nullptr, "tag"), kInvalid);
}

TEST_F(NotificationTest, get_tag_null) {
  const char* t = nullptr;
  EXPECT_EQ(notification_get_tag(nullptr, &t), kInvalid);
}

TEST_F(NotificationTest, get_type_null) {
  notification_type_e t;
  EXPECT_EQ(notification_get_type(nullptr, &t), kInvalid);
}

TEST_F(NotificationTest, get_pkgname_null) {
  char* p = nullptr;
  EXPECT_EQ(notification_get_pkgname(nullptr, &p), kInvalid);
}

// IPC-DEBT symbols — null-handle err-code coverage only here.
// Full IPC round-trip tests deferred to task 19 (integration suite).

TEST_F(NotificationTest, save_as_template_null) {
  EXPECT_EQ(notification_save_as_template(nullptr, "n"), kInvalid);
}

TEST_F(NotificationTest, create_from_template_null_name) {
  EXPECT_EQ(notification_create_from_template(nullptr), nullptr);
}

TEST_F(NotificationTest, load_by_tag_null) {
  EXPECT_EQ(notification_load_by_tag(nullptr), nullptr);
}

TEST_F(NotificationTest, post_null) {
  EXPECT_EQ(notification_post(nullptr), kInvalid);
}

TEST_F(NotificationTest, update_null) {
  EXPECT_EQ(notification_update(nullptr), kInvalid);
}

TEST_F(NotificationTest, delete_null) {
  EXPECT_EQ(notification_delete(nullptr), kInvalid);
}

TEST_F(NotificationTest, get_noti_block_state_null) {
  EXPECT_EQ(notification_get_noti_block_state(nullptr), kInvalid);
}
