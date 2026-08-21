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

#include <cstdlib>
#include <cstring>
#include <string>

#include "mock/test_fixture.hh"
#include "mock/aul_mock.hh"
#include "mock/package_manager_mock.hh"
#include "mock/tzplatform_config_mock.hh"
#include "mock/glib_mock.hh"
#include "mock/app_control_mock.hh"
#include "mock/noti_service_proxy_mock.hh"

#include "notification/include/notification.h"
#include "notification/include/notification_internal.h"
#include "notification/include/notification_private.h"
#include "notification/include/notification_text_domain.h"
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

notification_h cloned_noti = nullptr;

rpc_port_proxy_noti_service_h kFakeServiceHandle =
    reinterpret_cast<rpc_port_proxy_noti_service_h>(0x1);

int __fake_aul_app_get_appid_bypid(int pid, char* appid, int len) {
  snprintf(appid, len, "%s", "org.tizen.testappid");
  return AUL_R_OK;
}

int __fake_aul_app_get_appid_bypid_ok(int pid, char* appid, int len) {
  snprintf(appid, len, "%s", "org.tizen.testappid");
  return AUL_R_OK;
}

int __fake_aul_app_get_appid_bypid_fail(int pid, char* appid, int len) {
  return AUL_R_ERROR;
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

int __fake_rpc_port_proxy_noti_service_create(const char*,
    rpc_port_proxy_noti_service_callback_s*, void*,
    rpc_port_proxy_noti_service_h* handle) {
  *handle = kFakeServiceHandle;
  return RPC_PORT_ERROR_NONE;
}

GList *__fake_g_list_append(GList *list, gpointer data) {
  GList *new_list = static_cast<GList *>(g_malloc0(sizeof(GList)));
  new_list->data = data;
  if (list == nullptr)
    return new_list;

  GList *last = list;
  while (last->next)
    last = last->next;
  last->next = new_list;
  new_list->prev = last;
  return list;
}

class Mocks :
    virtual public ::testing::NiceMock<AulMock>,
    virtual public ::testing::NiceMock<TzplatformConfigMock>,
    virtual public ::testing::NiceMock<GlibMock>,
    virtual public ::testing::NiceMock<PackageManagerMock>,
    virtual public ::testing::NiceMock<AppControlMock>,
    virtual public ::testing::NiceMock<NotiServiceProxyMock> {};
} // namespace

using ::testing::_;
using ::testing::Invoke;
using ::testing::Return;
using ::testing::DoAll;
using ::testing::SetArgPointee;

class NotificationTest : public TestFixture {
 public:
  NotificationTest() : TestFixture(std::make_unique<::Mocks>()) {}

  void SetUp() override {
    EXPECT_CALL(GetMock<AulMock>(), aul_app_get_appid_bypid(_, _, _))
        .WillRepeatedly(Invoke(__fake_aul_app_get_appid_bypid_ok));
    ON_CALL(GetMock<NotiServiceProxyMock>(),
        rpc_port_proxy_noti_service_create(_, _, _, _))
        .WillByDefault(Invoke(__fake_rpc_port_proxy_noti_service_create));
    ON_CALL(GetMock<NotiServiceProxyMock>(),
        rpc_port_proxy_noti_service_connect_sync(_))
        .WillByDefault(Return(RPC_PORT_ERROR_NONE));
    ON_CALL(GetMock<AppControlMock>(), app_control_create(_))
        .WillByDefault(Invoke(app_control_mock_real_create));
    ON_CALL(GetMock<AppControlMock>(), app_control_import_from_bundle(_, _))
        .WillByDefault(Invoke(app_control_mock_real_import_from_bundle));
    ON_CALL(GetMock<AppControlMock>(), app_control_destroy(_))
        .WillByDefault(Invoke(app_control_mock_real_destroy));
    ON_CALL(GetMock<AppControlMock>(), app_control_export_as_bundle(_, _))
        .WillByDefault(Invoke(app_control_mock_real_export_as_bundle));
    ON_CALL(GetMock<GlibMock>(), g_list_append(_, _))
        .WillByDefault(Invoke(__fake_g_list_append));
  }
};

TEST_F(NotificationTest, notification_test) {
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

TEST_F(NotificationTest, GetAppIdByPidFallsBackToProcCmdline) {
  EXPECT_CALL(GetMock<AulMock>(), aul_app_get_appid_bypid(_, _, _))
      .WillOnce(Invoke(__fake_aul_app_get_appid_bypid_fail));

  char *app_id = notification_get_app_id_by_pid(getpid());
  if (app_id) {
    EXPECT_GT(strlen(app_id), 0u);
    free(app_id);
  }
}

TEST_F(NotificationTest, GetAppIdByPidReturnsNullForInvalidPid) {
  EXPECT_CALL(GetMock<AulMock>(), aul_app_get_appid_bypid(_, _, _))
      .WillOnce(Invoke(__fake_aul_app_get_appid_bypid_fail));

  char *app_id = notification_get_app_id_by_pid(-999999);
  ASSERT_EQ(app_id, nullptr);
}

TEST_F(NotificationTest, SetGetImageUpdateAndClearPath) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  int ret = notification_set_image(noti, NOTIFICATION_IMAGE_TYPE_ICON, "path1");
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  ret = notification_set_image(noti, NOTIFICATION_IMAGE_TYPE_ICON, "path2");
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  char *image_path = nullptr;
  ret = notification_get_image(noti, NOTIFICATION_IMAGE_TYPE_ICON, &image_path);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);
  ASSERT_STREQ(image_path, "path2");

  ret = notification_set_image(noti, NOTIFICATION_IMAGE_TYPE_ICON, nullptr);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  image_path = nullptr;
  ret = notification_get_image(noti, NOTIFICATION_IMAGE_TYPE_ICON, &image_path);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);
  ASSERT_EQ(image_path, nullptr);

  notification_h noti2 = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti2, nullptr);
  ret = notification_set_image(noti2, NOTIFICATION_IMAGE_TYPE_ICON, nullptr);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  ret = notification_set_image(nullptr, NOTIFICATION_IMAGE_TYPE_ICON, "x");
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
  ret = notification_set_image(noti, NOTIFICATION_IMAGE_TYPE_NONE, "x");
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
  ret = notification_get_image(nullptr, NOTIFICATION_IMAGE_TYPE_ICON, &image_path);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);

  notification_free(noti);
  notification_free(noti2);
}

TEST_F(NotificationTest, SetTextUpdatesExistingBundlesAndClears) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  int ret = notification_set_text(noti, NOTIFICATION_TEXT_TYPE_TITLE, "first",
      "key1", NOTIFICATION_VARIABLE_TYPE_NONE);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  ret = notification_set_text(noti, NOTIFICATION_TEXT_TYPE_TITLE, "second",
      "key2", NOTIFICATION_VARIABLE_TYPE_NONE);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  ret = notification_set_text(noti, NOTIFICATION_TEXT_TYPE_TITLE, nullptr,
      nullptr, NOTIFICATION_VARIABLE_TYPE_NONE);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  ret = notification_set_text(noti, NOTIFICATION_TEXT_TYPE_CONTENT, "%d", "key",
      NOTIFICATION_VARIABLE_TYPE_INT, 1, NOTIFICATION_VARIABLE_TYPE_NONE);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);
  ret = notification_set_text(noti, NOTIFICATION_TEXT_TYPE_CONTENT, "%d", "key",
      NOTIFICATION_VARIABLE_TYPE_INT, 2, NOTIFICATION_VARIABLE_TYPE_NONE);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  ret = notification_set_text(noti, NOTIFICATION_TEXT_TYPE_GROUP_TITLE, "%f", nullptr,
      NOTIFICATION_VARIABLE_TYPE_DOUBLE, 1.5, NOTIFICATION_VARIABLE_TYPE_NONE);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);
  ret = notification_set_text(noti, NOTIFICATION_TEXT_TYPE_GROUP_TITLE, "%f", nullptr,
      NOTIFICATION_VARIABLE_TYPE_DOUBLE, 2.5, NOTIFICATION_VARIABLE_TYPE_NONE);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  ret = notification_set_text(noti, NOTIFICATION_TEXT_TYPE_GROUP_CONTENT, "%s", nullptr,
      NOTIFICATION_VARIABLE_TYPE_STRING, "a", NOTIFICATION_VARIABLE_TYPE_NONE);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);
  ret = notification_set_text(noti, NOTIFICATION_TEXT_TYPE_GROUP_CONTENT, "%s", nullptr,
      NOTIFICATION_VARIABLE_TYPE_STRING, "b", NOTIFICATION_VARIABLE_TYPE_NONE);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  ret = notification_set_text(noti,
      NOTIFICATION_TEXT_TYPE_CONTENT_FOR_DISPLAY_OPTION_IS_OFF, "%d", nullptr,
      NOTIFICATION_VARIABLE_TYPE_COUNT, NOTIFICATION_COUNT_POS_LEFT,
      NOTIFICATION_VARIABLE_TYPE_NONE);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);
  ret = notification_set_text(noti,
      NOTIFICATION_TEXT_TYPE_CONTENT_FOR_DISPLAY_OPTION_IS_OFF, "%d", nullptr,
      NOTIFICATION_VARIABLE_TYPE_COUNT, NOTIFICATION_COUNT_POS_RIGHT,
      NOTIFICATION_VARIABLE_TYPE_NONE);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  ret = notification_set_text(noti,
      NOTIFICATION_TEXT_TYPE_GROUP_CONTENT_FOR_DISPLAY_OPTION_IS_OFF, "x", nullptr,
      static_cast<notification_variable_type_e>(99),
      NOTIFICATION_VARIABLE_TYPE_NONE);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);

  ret = notification_set_text(nullptr, NOTIFICATION_TEXT_TYPE_TITLE, "x", nullptr,
      NOTIFICATION_VARIABLE_TYPE_NONE);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
  ret = notification_set_text(noti, NOTIFICATION_TEXT_TYPE_NONE, "x", nullptr,
      NOTIFICATION_VARIABLE_TYPE_NONE);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);

  notification_free(noti);
}

TEST_F(NotificationTest, GetTextFormatSpecifiersAndCountPositions) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  int ret = notification_set_text(noti, NOTIFICATION_TEXT_TYPE_CONTENT,
      "%1Xd-%2Xs-%3Xf-%d-%s-%f-%%", nullptr,
      NOTIFICATION_VARIABLE_TYPE_COUNT, NOTIFICATION_COUNT_POS_LEFT,
      NOTIFICATION_VARIABLE_TYPE_INT, 7,
      NOTIFICATION_VARIABLE_TYPE_STRING, "hello",
      NOTIFICATION_VARIABLE_TYPE_DOUBLE, 1.25,
      NOTIFICATION_VARIABLE_TYPE_INT, 42,
      NOTIFICATION_VARIABLE_TYPE_STRING, "world",
      NOTIFICATION_VARIABLE_TYPE_DOUBLE, 3.5,
      NOTIFICATION_VARIABLE_TYPE_COUNT, NOTIFICATION_COUNT_POS_RIGHT,
      NOTIFICATION_VARIABLE_TYPE_NONE);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  char *text = nullptr;
  ret = notification_get_text(noti, NOTIFICATION_TEXT_TYPE_CONTENT, &text);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);
  ASSERT_NE(text, nullptr);

  ret = notification_set_text_domain(noti, "test_domain", "/tmp");
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);
  ret = notification_set_text(noti, NOTIFICATION_TEXT_TYPE_TITLE, "key_for_title",
      "translate_key", NOTIFICATION_VARIABLE_TYPE_NONE);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);
  text = nullptr;
  ret = notification_get_text(noti, NOTIFICATION_TEXT_TYPE_TITLE, &text);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  char *domain = nullptr;
  char *dir = nullptr;
  ret = notification_get_text_domain(noti, &domain, &dir);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);
  ASSERT_STREQ(domain, "test_domain");
  ASSERT_STREQ(dir, "/tmp");
  ret = notification_get_text_domain(nullptr, &domain, &dir);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);

  ret = notification_get_text(nullptr, NOTIFICATION_TEXT_TYPE_TITLE, &text);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
  ret = notification_get_text(noti, NOTIFICATION_TEXT_TYPE_NONE, &text);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);

  char *empty_text = reinterpret_cast<char *>(0x1);
  ret = notification_get_text(noti, NOTIFICATION_TEXT_TYPE_EVENT_COUNT, &empty_text);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);
  ASSERT_EQ(empty_text, nullptr);

  ret = notification_set_text_domain(noti, nullptr, "/tmp");
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);

  notification_free(noti);
}

TEST_F(NotificationTest, GetTextSkipsFormattingWhenTranslationFlagSet) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  int ret = notification_set_text(noti, NOTIFICATION_TEXT_TYPE_CONTENT, "%d", nullptr,
      NOTIFICATION_VARIABLE_TYPE_INT, 5, NOTIFICATION_VARIABLE_TYPE_NONE);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  noti->is_translation = true;
  char *text = nullptr;
  ret = notification_get_text(noti, NOTIFICATION_TEXT_TYPE_CONTENT, &text);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);
  ASSERT_STREQ(text, "%d");

  notification_free(noti);
}

TEST_F(NotificationTest, GetTextKeySetWithoutDomainUsesSysString) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  int ret = notification_set_text(noti, NOTIFICATION_TEXT_TYPE_TITLE, "title_text",
      "some_sys_key", NOTIFICATION_VARIABLE_TYPE_NONE);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  char *text = nullptr;
  ret = notification_get_text(noti, NOTIFICATION_TEXT_TYPE_TITLE, &text);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);
  ASSERT_NE(text, nullptr);

  notification_free(noti);
}

TEST_F(NotificationTest, TimeToTextInvalidParams) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  int ret = notification_set_time_to_text(noti, NOTIFICATION_TEXT_TYPE_TITLE, 0);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
  ret = notification_set_time_to_text(nullptr, NOTIFICATION_TEXT_TYPE_TITLE, 100);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
  ret = notification_set_time_to_text(noti, NOTIFICATION_TEXT_TYPE_NONE, 100);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);

  time_t ret_time = 0;
  ret = notification_get_time_from_text(nullptr, NOTIFICATION_TEXT_TYPE_TITLE, &ret_time);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
  ret = notification_get_time_from_text(noti, NOTIFICATION_TEXT_TYPE_NONE, &ret_time);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
  ret = notification_get_time_from_text(noti, NOTIFICATION_TEXT_TYPE_CONTENT, &ret_time);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);

  notification_free(noti);
}

TEST_F(NotificationTest, SoundInvalidTypeAndClearingBranches) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  int ret = notification_set_sound(noti, NOTIFICATION_SOUND_TYPE_USER_DATA, "sound1");
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  ret = notification_set_sound(noti, NOTIFICATION_SOUND_TYPE_USER_DATA, nullptr);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);

  notification_sound_type_e sound_type;
  const char *sound_path = nullptr;
  ret = notification_get_sound(noti, &sound_type, &sound_path);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  ret = notification_set_sound(nullptr, NOTIFICATION_SOUND_TYPE_DEFAULT, "x");
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
  ret = notification_set_sound(noti,
      static_cast<notification_sound_type_e>(999), "x");
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
  ret = notification_get_sound(nullptr, &sound_type, &sound_path);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);

  notification_free(noti);
}

TEST_F(NotificationTest, VibrationInvalidTypeAndClearingBranches) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  int ret = notification_set_vibration(noti, NOTIFICATION_VIBRATION_TYPE_USER_DATA, "vib1");
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  ret = notification_set_vibration(noti, NOTIFICATION_VIBRATION_TYPE_USER_DATA, nullptr);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);

  notification_vibration_type_e vib_type;
  const char *vib_path = nullptr;
  ret = notification_get_vibration(noti, &vib_type, &vib_path);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  ret = notification_set_vibration(nullptr, NOTIFICATION_VIBRATION_TYPE_DEFAULT, "x");
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
  ret = notification_set_vibration(noti,
      static_cast<notification_vibration_type_e>(999), "x");
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
  ret = notification_get_vibration(nullptr, &vib_type, &vib_path);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);

  notification_free(noti);
}

TEST_F(NotificationTest, LaunchOptionInvalidParamsAndExportFailure) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  app_control_h app_control = nullptr;
  app_control_create(&app_control);

  int ret = notification_set_launch_option(nullptr,
      NOTIFICATION_LAUNCH_OPTION_APP_CONTROL, app_control);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
  ret = notification_set_launch_option(noti,
      NOTIFICATION_LAUNCH_OPTION_APP_CONTROL, nullptr);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);

  EXPECT_CALL(GetMock<AppControlMock>(), app_control_export_as_bundle(_, _))
      .WillOnce(Return(APP_CONTROL_ERROR_INVALID_PARAMETER));
  ret = notification_set_launch_option(noti,
      NOTIFICATION_LAUNCH_OPTION_APP_CONTROL, app_control);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);

  ret = notification_get_launch_option(nullptr,
      NOTIFICATION_LAUNCH_OPTION_APP_CONTROL, &app_control);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);

  app_control_destroy(app_control);
  notification_free(noti);
}

TEST_F(NotificationTest, EventHandlerInvalidParamsAndExportFailure) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  app_control_h app_control = nullptr;
  app_control_create(&app_control);

  int ret = notification_set_event_handler(nullptr,
      NOTIFICATION_EVENT_TYPE_CLICK_ON_BUTTON_1, app_control);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
  ret = notification_set_event_handler(noti,
      static_cast<notification_event_type_e>(999), app_control);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);

  EXPECT_CALL(GetMock<AppControlMock>(), app_control_export_as_bundle(_, _))
      .WillOnce(Return(APP_CONTROL_ERROR_INVALID_PARAMETER));
  ret = notification_set_event_handler(noti,
      NOTIFICATION_EVENT_TYPE_CLICK_ON_BUTTON_1, app_control);
  ASSERT_EQ(ret, APP_CONTROL_ERROR_INVALID_PARAMETER);

  ret = notification_get_event_handler(nullptr,
      NOTIFICATION_EVENT_TYPE_CLICK_ON_BUTTON_1, &app_control);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);

  app_control_destroy(app_control);
  notification_free(noti);
}

TEST_F(NotificationTest, PropertyDisplayApplistLegacyMask) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  int ret = notification_set_property(nullptr, 0);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
  ret = notification_get_property(noti, nullptr);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);

  ret = notification_set_display_applist(nullptr, 0);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);

  ret = notification_set_display_applist(noti, 0xffffffff);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);
  int applist = 0;
  ret = notification_get_display_applist(noti, &applist);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);
  ASSERT_EQ(applist, NOTIFICATION_DISPLAY_APP_ALL);
  ret = notification_get_display_applist(noti, nullptr);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);

  ret = notification_set_size(nullptr, 1.0);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
  ret = notification_get_size(noti, nullptr);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
  ret = notification_set_progress(nullptr, 1.0);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
  ret = notification_get_progress(noti, nullptr);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);

  char *pkgname = nullptr;
  ret = notification_get_pkgname(noti, &pkgname);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);
  ret = notification_get_pkgname(nullptr, &pkgname);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);

  ret = notification_set_layout(nullptr, NOTIFICATION_LY_NOTI_EVENT_SINGLE);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
  ret = notification_get_layout(noti, nullptr);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
  ret = notification_get_type(nullptr, nullptr);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);

  notification_free(noti);
}

TEST_F(NotificationTest, FreeReleasesTempTitleContentTagAndButtons) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  int ret = notification_set_text(noti, NOTIFICATION_TEXT_TYPE_TITLE, "title", nullptr,
      NOTIFICATION_VARIABLE_TYPE_NONE);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);
  char *text = nullptr;
  ret = notification_get_text(noti, NOTIFICATION_TEXT_TYPE_TITLE, &text);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  ret = notification_set_text(noti, NOTIFICATION_TEXT_TYPE_CONTENT, "content", nullptr,
      NOTIFICATION_VARIABLE_TYPE_NONE);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);
  ret = notification_get_text(noti, NOTIFICATION_TEXT_TYPE_CONTENT, &text);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  ret = notification_set_tag(noti, "tag1");
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);
  ret = notification_set_tag(noti, "tag2");
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);
  const char *tag = nullptr;
  ret = notification_get_tag(noti, &tag);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);
  ASSERT_STREQ(tag, "tag2");
  ret = notification_get_tag(nullptr, &tag);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
  ret = notification_set_tag(nullptr, "x");
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);

  app_control_h app_control = nullptr;
  app_control_create(&app_control);
  ret = notification_set_event_handler(noti,
      NOTIFICATION_EVENT_TYPE_CLICK_ON_BUTTON_1, app_control);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);
  app_control_destroy(app_control);

  ret = notification_add_button(noti, NOTIFICATION_BUTTON_1);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);
  ret = notification_remove_button(noti, NOTIFICATION_BUTTON_1);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);
  ret = notification_remove_button(noti, NOTIFICATION_BUTTON_1);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  ret = notification_add_button(nullptr, NOTIFICATION_BUTTON_1);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
  ret = notification_add_button(noti, static_cast<notification_button_index_e>(0));
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
  ret = notification_remove_button(nullptr, NOTIFICATION_BUTTON_1);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
  ret = notification_remove_button(noti, static_cast<notification_button_index_e>(0));
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);

  ret = notification_free(noti);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  ret = notification_free(nullptr);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationTest, FreeReleasesAppIconPath) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  noti->app_icon_path = strdup("icon_path");

  int ret = notification_free(noti);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);
}

TEST_F(NotificationTest, OngoingFlagInvalidParams) {
  int ret = notification_set_ongoing_flag(nullptr, true);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
  ret = notification_get_ongoing_flag(nullptr, nullptr);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationTest, LoadByTagWithNullTagForwards) {
  notification_h noti = notification_load_by_tag(nullptr);
  ASSERT_EQ(noti, nullptr);
}

TEST_F(NotificationTest, CreateInvalidTypeReturnsNull) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NONE);
  ASSERT_EQ(noti, nullptr);
}

TEST_F(NotificationTest, GetAppIdByPidCmdlineReadReturnsEmpty) {
  EXPECT_CALL(GetMock<AulMock>(), aul_app_get_appid_bypid(_, _, _))
      .WillOnce(Invoke(__fake_aul_app_get_appid_bypid_fail));

  char *app_id = notification_get_app_id_by_pid(2);
  ASSERT_EQ(app_id, nullptr);
}

TEST_F(NotificationTest, GetAppIdByPidRejectsEmptyAulResult) {
  EXPECT_CALL(GetMock<AulMock>(), aul_app_get_appid_bypid(_, _, _))
      .WillOnce(Invoke([](int, char *appid, int) {
        appid[0] = '\0';
        return AUL_R_OK;
      }));

  char *app_id = notification_get_app_id_by_pid(getpid());
  ASSERT_EQ(app_id, nullptr);
}

TEST_F(NotificationTest, GetSoundVibrationLedWithNonNullOutParams) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  int ret = notification_set_sound(noti, NOTIFICATION_SOUND_TYPE_USER_DATA, "s1");
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);
  notification_sound_type_e sound_type;
  const char *sound_path = nullptr;
  ret = notification_get_sound(noti, &sound_type, &sound_path);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);
  ASSERT_STREQ(sound_path, "s1");

  ret = notification_set_vibration(noti, NOTIFICATION_VIBRATION_TYPE_USER_DATA, "v1");
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);
  notification_vibration_type_e vib_type;
  const char *vib_path = nullptr;
  ret = notification_get_vibration(noti, &vib_type, &vib_path);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);
  ASSERT_STREQ(vib_path, "v1");

  ret = notification_set_led(noti, NOTIFICATION_LED_OP_ON_CUSTOM_COLOR, 0x112233);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);
  notification_led_op_e led_op;
  int led_argb = 0;
  ret = notification_get_led(noti, &led_op, &led_argb);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);
  ASSERT_EQ(led_argb, 0x112233);

  ret = notification_set_led_time_period(noti, -1, 10);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
  ret = notification_set_led_time_period(noti, 10, -1);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
  ret = notification_set_led_time_period(noti, 5, 6);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  ret = notification_get_led_time_period(noti, nullptr, nullptr);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);
  int on_ms = 0, off_ms = 0;
  ret = notification_get_led_time_period(noti, &on_ms, &off_ms);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);
  ASSERT_EQ(on_ms, 5);
  ASSERT_EQ(off_ms, 6);
  ret = notification_get_led_time_period(nullptr, &on_ms, &off_ms);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);

  notification_free(noti);
}

TEST_F(NotificationTest, LaunchOptionAndEventHandlerRoundTrip) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  app_control_h app_control = nullptr;
  app_control_create(&app_control);

  int ret = notification_set_launch_option(noti,
      NOTIFICATION_LAUNCH_OPTION_APP_CONTROL, app_control);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  app_control_h got = nullptr;
  ret = notification_get_launch_option(noti,
      NOTIFICATION_LAUNCH_OPTION_APP_CONTROL, &got);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);
  ASSERT_NE(got, nullptr);
  app_control_destroy(got);

  ret = notification_set_event_handler(noti,
      NOTIFICATION_EVENT_TYPE_CLICK_ON_BUTTON_1, app_control);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);
  ret = notification_set_event_handler(noti,
      NOTIFICATION_EVENT_TYPE_CLICK_ON_BUTTON_1, app_control);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  app_control_h got_handler = nullptr;
  ret = notification_get_event_handler(noti,
      NOTIFICATION_EVENT_TYPE_CLICK_ON_BUTTON_1, &got_handler);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);
  ASSERT_NE(got_handler, nullptr);
  app_control_destroy(got_handler);

  ret = notification_get_event_handler(noti,
      static_cast<notification_event_type_e>(999), &got_handler);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);

  app_control_destroy(app_control);
  notification_free(noti);
}

TEST_F(NotificationTest, GetPkgnameWithNullCallerAppId) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  free(noti->caller_app_id);
  noti->caller_app_id = nullptr;

  char *pkgname = reinterpret_cast<char *>(0x1);
  int ret = notification_get_pkgname(noti, &pkgname);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);
  ASSERT_EQ(pkgname, nullptr);

  notification_free(noti);
}

TEST_F(NotificationTest, CreateUsesSystemAppBranchForLowUid) {
  EXPECT_CALL(GetMock<AulMock>(), aul_getuid())
      .WillOnce(Return(100));

  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);
  ASSERT_NE(noti->caller_app_id, nullptr);
  ASSERT_STREQ(noti->pkg_id, noti->caller_app_id);

  notification_free(noti);
}

TEST_F(NotificationTest, CreateReusesCachedPkgIdAndLabel) {
  notification_h noti1 = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti1, nullptr);

  notification_h noti2 = notification_create(NOTIFICATION_TYPE_ONGOING);
  ASSERT_NE(noti2, nullptr);
  EXPECT_STREQ(noti2->pkg_id, noti1->pkg_id);

  notification_free(noti1);
  notification_free(noti2);
}

TEST_F(NotificationTest, GetTextTranslationSkippedWhenKeySetAndIsTranslationTrue) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  int ret = notification_set_text(noti, NOTIFICATION_TEXT_TYPE_TITLE, "raw_text",
      "some_key", NOTIFICATION_VARIABLE_TYPE_NONE);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);
  noti->is_translation = true;

  char *text = nullptr;
  ret = notification_get_text(noti, NOTIFICATION_TEXT_TYPE_TITLE, &text);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);
  ASSERT_STREQ(text, "raw_text");

  notification_free(noti);
}

TEST_F(NotificationTest, GetTextDomainSetWithoutDirFallsBackToSysString) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  int ret = notification_set_text(noti, NOTIFICATION_TEXT_TYPE_TITLE, "title_text",
      "some_key", NOTIFICATION_VARIABLE_TYPE_NONE);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  free(noti->domain);
  noti->domain = strdup("domain_only");

  char *text = nullptr;
  ret = notification_get_text(noti, NOTIFICATION_TEXT_TYPE_TITLE, &text);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);
  ASSERT_NE(text, nullptr);

  notification_free(noti);
}

TEST_F(NotificationTest, GetTextKeyBundleExistsButHasNoEntryForType) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  int ret = notification_set_text(noti, NOTIFICATION_TEXT_TYPE_GROUP_TITLE, "group_title",
      "group_key", NOTIFICATION_VARIABLE_TYPE_NONE);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  char *text = reinterpret_cast<char *>(0x1);
  ret = notification_get_text(noti, NOTIFICATION_TEXT_TYPE_TITLE, &text);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);
  ASSERT_EQ(text, nullptr);

  notification_free(noti);
}

TEST_F(NotificationTest, GetTextWithoutFormatArgsBundleEarlyReturns) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  noti->b_text = bundle_create();
  char key[8];
  snprintf(key, sizeof(key), "%d", NOTIFICATION_TEXT_TYPE_TITLE);
  bundle_add_str(noti->b_text, key, "raw_from_bundle");

  char *text = nullptr;
  int ret = notification_get_text(noti, NOTIFICATION_TEXT_TYPE_TITLE, &text);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);
  ASSERT_STREQ(text, "raw_from_bundle");

  notification_free(noti);
}

TEST_F(NotificationTest, GetTextCountTypeInsideSimplePercentD) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  int ret = notification_set_text(noti, NOTIFICATION_TEXT_TYPE_CONTENT, "%d", nullptr,
      NOTIFICATION_VARIABLE_TYPE_COUNT, NOTIFICATION_COUNT_POS_IN,
      NOTIFICATION_VARIABLE_TYPE_NONE);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  char *text = nullptr;
  ret = notification_get_text(noti, NOTIFICATION_TEXT_TYPE_CONTENT, &text);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);
  ASSERT_NE(text, nullptr);

  notification_free(noti);
}

TEST_F(NotificationTest, GetTextCountTypeInsideIndexedPercentD) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  int ret = notification_set_text(noti, NOTIFICATION_TEXT_TYPE_CONTENT, "%1Xd", nullptr,
      NOTIFICATION_VARIABLE_TYPE_COUNT, NOTIFICATION_COUNT_POS_IN,
      NOTIFICATION_VARIABLE_TYPE_NONE);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  char *text = nullptr;
  ret = notification_get_text(noti, NOTIFICATION_TEXT_TYPE_CONTENT, &text);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);
  ASSERT_NE(text, nullptr);

  notification_free(noti);
}

TEST_F(NotificationTest, GetTextCountRightPositionAtFormatEnd) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  int ret = notification_set_text(noti, NOTIFICATION_TEXT_TYPE_CONTENT, "%f %d %s %d", nullptr,
      NOTIFICATION_VARIABLE_TYPE_COUNT, NOTIFICATION_COUNT_POS_LEFT,
      NOTIFICATION_VARIABLE_TYPE_DOUBLE, 0.0,
      NOTIFICATION_VARIABLE_TYPE_COUNT, NOTIFICATION_COUNT_POS_IN,
      NOTIFICATION_VARIABLE_TYPE_STRING, "Content",
      NOTIFICATION_VARIABLE_TYPE_INT, 0,
      NOTIFICATION_VARIABLE_TYPE_COUNT, NOTIFICATION_COUNT_POS_RIGHT,
      NOTIFICATION_VARIABLE_TYPE_NONE);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  char *text = nullptr;
  ret = notification_get_text(noti, NOTIFICATION_TEXT_TYPE_CONTENT, &text);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);
  ASSERT_NE(text, nullptr);

  notification_free(noti);
}

TEST_F(NotificationTest, GetTextBufferFullOnPlainCharacter) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  std::string long_value(4095, 'x');
  int ret = notification_set_text(noti, NOTIFICATION_TEXT_TYPE_CONTENT, "%sZ", nullptr,
      NOTIFICATION_VARIABLE_TYPE_STRING, long_value.c_str(),
      NOTIFICATION_VARIABLE_TYPE_NONE);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  char *text = nullptr;
  ret = notification_get_text(noti, NOTIFICATION_TEXT_TYPE_CONTENT, &text);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);
  ASSERT_NE(text, nullptr);

  notification_free(noti);
}

TEST_F(NotificationTest, GetTextBufferFullOnPercentPercent) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  std::string long_value(4095, 'x');
  int ret = notification_set_text(noti, NOTIFICATION_TEXT_TYPE_CONTENT, "%s%%", nullptr,
      NOTIFICATION_VARIABLE_TYPE_STRING, long_value.c_str(),
      NOTIFICATION_VARIABLE_TYPE_NONE);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  char *text = nullptr;
  ret = notification_get_text(noti, NOTIFICATION_TEXT_TYPE_CONTENT, &text);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);
  ASSERT_NE(text, nullptr);

  notification_free(noti);
}

TEST_F(NotificationTest, GetTextBufferFullOnIndexedFallbackChar) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  std::string long_value(4095, 'x');
  int ret = notification_set_text(noti, NOTIFICATION_TEXT_TYPE_CONTENT,
      "%s%1Aq%2Bq%3Cq", nullptr,
      NOTIFICATION_VARIABLE_TYPE_STRING, long_value.c_str(),
      NOTIFICATION_VARIABLE_TYPE_NONE);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  char *text = nullptr;
  ret = notification_get_text(noti, NOTIFICATION_TEXT_TYPE_CONTENT, &text);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);
  ASSERT_NE(text, nullptr);

  notification_free(noti);
}

TEST_F(NotificationTest, GetTextBufferFullOnUnknownSpecifier) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  std::string long_value(4095, 'x');
  int ret = notification_set_text(noti, NOTIFICATION_TEXT_TYPE_CONTENT, "%s%q%r%z", nullptr,
      NOTIFICATION_VARIABLE_TYPE_STRING, long_value.c_str(),
      NOTIFICATION_VARIABLE_TYPE_NONE);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  char *text = nullptr;
  ret = notification_get_text(noti, NOTIFICATION_TEXT_TYPE_CONTENT, &text);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);
  ASSERT_NE(text, nullptr);

  notification_free(noti);
}

TEST_F(NotificationTest, GetTextRepeatedCallsFreePreviousTempStrings) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  int ret = notification_set_text(noti, NOTIFICATION_TEXT_TYPE_TITLE, "%d", nullptr,
      NOTIFICATION_VARIABLE_TYPE_INT, 1, NOTIFICATION_VARIABLE_TYPE_NONE);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);
  char *text = nullptr;
  ret = notification_get_text(noti, NOTIFICATION_TEXT_TYPE_TITLE, &text);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);
  text = nullptr;
  ret = notification_get_text(noti, NOTIFICATION_TEXT_TYPE_TITLE, &text);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  ret = notification_set_text(noti, NOTIFICATION_TEXT_TYPE_CONTENT, "%d", nullptr,
      NOTIFICATION_VARIABLE_TYPE_INT, 2, NOTIFICATION_VARIABLE_TYPE_NONE);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);
  text = nullptr;
  ret = notification_get_text(noti, NOTIFICATION_TEXT_TYPE_CONTENT, &text);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);
  text = nullptr;
  ret = notification_get_text(noti, NOTIFICATION_TEXT_TYPE_CONTENT, &text);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  notification_free(noti);
}

TEST_F(NotificationTest, PostSucceeds) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  EXPECT_CALL(GetMock<NotiServiceProxyMock>(),
      rpc_port_proxy_noti_service_invoke_add_noti(_, _, _))
      .WillOnce(DoAll(SetArgPointee<2>(42), Return(RPC_PORT_ERROR_NONE)));

  EXPECT_EQ(notification_post(noti), NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(noti->priv_id, 42);

  notification_free(noti);
}

TEST_F(NotificationTest, PostFailsWhenServiceRejects) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  EXPECT_CALL(GetMock<NotiServiceProxyMock>(),
      rpc_port_proxy_noti_service_invoke_add_noti(_, _, _))
      .WillOnce(Return(RPC_PORT_ERROR_IO_ERROR));

  EXPECT_NE(notification_post(noti), NOTIFICATION_ERROR_NONE);

  notification_free(noti);
}

TEST_F(NotificationTest, UpdateSucceeds) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  EXPECT_CALL(GetMock<NotiServiceProxyMock>(),
      rpc_port_proxy_noti_service_invoke_update_noti(_, _, _))
      .WillOnce(DoAll(SetArgPointee<2>(1), Return(RPC_PORT_ERROR_NONE)));

  EXPECT_EQ(notification_update(noti), NOTIFICATION_ERROR_NONE);

  notification_free(noti);
}

TEST_F(NotificationTest, UpdateWithNullNotiRequestsRefresh) {
  EXPECT_CALL(GetMock<NotiServiceProxyMock>(),
      rpc_port_proxy_noti_service_invoke_refresh_noti(_, _))
      .WillOnce(Return(RPC_PORT_ERROR_NONE));

  EXPECT_EQ(notification_update(nullptr), NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationTest, DeleteSucceeds) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);
  noti->priv_id = 7;

  EXPECT_CALL(GetMock<NotiServiceProxyMock>(),
      rpc_port_proxy_noti_service_invoke_delete_single_noti(_, _, 7, _))
      .WillOnce(Return(RPC_PORT_ERROR_NONE));

  EXPECT_EQ(notification_delete(noti), NOTIFICATION_ERROR_NONE);

  notification_free(noti);
}

TEST_F(NotificationTest, DeleteFailsWithNullNoti) {
  EXPECT_EQ(notification_delete(nullptr), NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationTest, DeleteAllSucceeds) {
  EXPECT_CALL(GetMock<NotiServiceProxyMock>(),
      rpc_port_proxy_noti_service_invoke_delete_multiple_noti(_, _, _, _, _))
      .WillOnce(DoAll(SetArgPointee<4>(3), Return(RPC_PORT_ERROR_NONE)));

  EXPECT_EQ(notification_delete_all(NOTIFICATION_TYPE_NOTI), NOTIFICATION_ERROR_NONE);
}

TEST_F(NotificationTest, DeleteAllFailsWithInvalidType) {
  EXPECT_EQ(notification_delete_all(NOTIFICATION_TYPE_NONE),
      NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationTest, CloneCopiesPopulatedFields) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);
  noti->launch_app_id = strdup("org.tizen.launch");
  noti->args = bundle_create();
  noti->group_args = bundle_create();
  noti->b_execute_option = bundle_create();
  noti->b_service_responding = bundle_create();
  noti->b_service_single_launch = bundle_create();
  noti->b_service_multi_launch = bundle_create();
  noti->b_event_handler[0] = bundle_create();
  noti->b_text = bundle_create();
  noti->b_key = bundle_create();
  noti->b_format_args = bundle_create();
  noti->b_image_path = bundle_create();
  noti->b_priv_image_path = bundle_create();
  noti->sound_path = strdup("sound.wav");
  noti->priv_sound_path = strdup("priv_sound.wav");
  noti->vibration_path = strdup("vib.wav");
  noti->priv_vibration_path = strdup("priv_vib.wav");
  noti->tag = strdup("tag");

  notification_h clone = nullptr;
  EXPECT_EQ(notification_clone(noti, &clone), NOTIFICATION_ERROR_NONE);
  ASSERT_NE(clone, nullptr);
  EXPECT_STREQ(clone->launch_app_id, "org.tizen.launch");
  EXPECT_STREQ(clone->sound_path, "sound.wav");
  EXPECT_STREQ(clone->tag, "tag");
  EXPECT_NE(clone->args, nullptr);
  EXPECT_NE(clone->b_event_handler[0], nullptr);

  notification_free(noti);
  notification_free(clone);
}

TEST_F(NotificationTest, CloneFailsWithNullArgs) {
  notification_h clone = nullptr;
  EXPECT_EQ(notification_clone(nullptr, &clone), NOTIFICATION_ERROR_INVALID_PARAMETER);

  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);
  EXPECT_EQ(notification_clone(noti, nullptr), NOTIFICATION_ERROR_INVALID_PARAMETER);
  notification_free(noti);
}

TEST_F(NotificationTest, SetAndGetTagReplacesPreviousValue) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  EXPECT_EQ(notification_set_tag(noti, "first"), NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(notification_set_tag(noti, "second"), NOTIFICATION_ERROR_NONE);

  const char *tag = nullptr;
  EXPECT_EQ(notification_get_tag(noti, &tag), NOTIFICATION_ERROR_NONE);
  EXPECT_STREQ(tag, "second");

  notification_free(noti);
}

TEST_F(NotificationTest, SetOngoingFlagRoundTrips) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  EXPECT_EQ(notification_set_ongoing_flag(noti, true), NOTIFICATION_ERROR_NONE);
  bool flag = false;
  EXPECT_EQ(notification_get_ongoing_flag(noti, &flag), NOTIFICATION_ERROR_NONE);
  EXPECT_TRUE(flag);

  notification_free(noti);
}

TEST_F(NotificationTest, SaveAsTemplateSucceeds) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  EXPECT_CALL(GetMock<NotiServiceProxyMock>(),
      rpc_port_proxy_noti_service_invoke_save_as_template(_, _, _))
      .WillOnce(Return(RPC_PORT_ERROR_NONE));

  EXPECT_EQ(notification_save_as_template(noti, "tmpl"), NOTIFICATION_ERROR_NONE);

  notification_free(noti);
}

TEST_F(NotificationTest, SaveAsTemplateFailsWithNullArgs) {
  EXPECT_EQ(notification_save_as_template(nullptr, "tmpl"),
      NOTIFICATION_ERROR_INVALID_PARAMETER);

  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);
  EXPECT_EQ(notification_save_as_template(noti, nullptr),
      NOTIFICATION_ERROR_INVALID_PARAMETER);
  notification_free(noti);
}

TEST_F(NotificationTest, CreateFromTemplateSucceeds) {
  EXPECT_CALL(GetMock<NotiServiceProxyMock>(),
      rpc_port_proxy_noti_service_invoke_create_from_template(_, _, _))
      .WillOnce(Invoke([](rpc_port_proxy_noti_service_h, const char*,
          rpc_port_proxy_notification_h *handle) {
        rpc_port_proxy_notification_create(handle);
        make_empty_notification(*handle);
        return static_cast<int>(RPC_PORT_ERROR_NONE);
      }));

  notification_h noti = notification_create_from_template("tmpl");
  ASSERT_NE(noti, nullptr);

  notification_free(noti);
}

TEST_F(NotificationTest, CreateFromTemplateFailsWithNullArg) {
  EXPECT_EQ(notification_create_from_template(nullptr), nullptr);
}

TEST_F(NotificationTest, CreateFromTemplateFailsWhenServiceRejects) {
  EXPECT_CALL(GetMock<NotiServiceProxyMock>(),
      rpc_port_proxy_noti_service_invoke_create_from_template(_, _, _))
      .WillOnce(Return(RPC_PORT_ERROR_IO_ERROR));

  EXPECT_EQ(notification_create_from_template("tmpl"), nullptr);
}

TEST_F(NotificationTest, GetNotiBlockStateFailsWithNullState) {
  EXPECT_EQ(notification_get_noti_block_state(nullptr),
      NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationTest, GetNotiBlockStateReturnsAllowed) {
  EXPECT_CALL(GetMock<NotiServiceProxyMock>(),
      rpc_port_proxy_noti_service_invoke_get_noti_block_state(_, _, _, _, _, _))
      .WillOnce(DoAll(SetArgPointee<3>(0), SetArgPointee<4>(0),
          SetArgPointee<5>(1), Return(RPC_PORT_ERROR_NONE)));

  notification_block_state_e state;
  EXPECT_EQ(notification_get_noti_block_state(&state), NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(state, NOTIFICATION_BLOCK_STATE_ALLOWED);
}

TEST_F(NotificationTest, GetNotiBlockStateReturnsDoNotDisturb) {
  EXPECT_CALL(GetMock<NotiServiceProxyMock>(),
      rpc_port_proxy_noti_service_invoke_get_noti_block_state(_, _, _, _, _, _))
      .WillOnce(DoAll(SetArgPointee<3>(1), SetArgPointee<4>(0),
          SetArgPointee<5>(1), Return(RPC_PORT_ERROR_NONE)));

  notification_block_state_e state;
  EXPECT_EQ(notification_get_noti_block_state(&state), NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(state, NOTIFICATION_BLOCK_STATE_DO_NOT_DISTURB);
}

TEST_F(NotificationTest, GetNotiBlockStateReturnsBlocked) {
  EXPECT_CALL(GetMock<NotiServiceProxyMock>(),
      rpc_port_proxy_noti_service_invoke_get_noti_block_state(_, _, _, _, _, _))
      .WillOnce(DoAll(SetArgPointee<3>(0), SetArgPointee<4>(0),
          SetArgPointee<5>(0), Return(RPC_PORT_ERROR_NONE)));

  notification_block_state_e state;
  EXPECT_EQ(notification_get_noti_block_state(&state), NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(state, NOTIFICATION_BLOCK_STATE_BLOCKED);
}

TEST_F(NotificationTest, GetNotiBlockStateFailsWhenServiceRejects) {
  EXPECT_CALL(GetMock<NotiServiceProxyMock>(),
      rpc_port_proxy_noti_service_invoke_get_noti_block_state(_, _, _, _, _, _))
      .WillOnce(Return(RPC_PORT_ERROR_IO_ERROR));

  notification_block_state_e state;
  EXPECT_NE(notification_get_noti_block_state(&state), NOTIFICATION_ERROR_NONE);
}
