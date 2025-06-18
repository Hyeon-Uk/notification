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
#include "notification/include/notification_ipc.h"

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

GVariant *noti_body = nullptr;
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

const char* __fake_tzplatform_getenv(tzplatform_variable id) {
  return "/opt/usr/home/owner/apps_rw";
}

GDBusConnection* __fake_g_bus_get_sync(GBusType type, GCancellable* cancellable, GError** error) {
  GDBusConnection* conn = reinterpret_cast<GDBusConnection*>(calloc(1, 4));

  return conn;
}

GDBusMessage* __fake_g_dbus_connection_send_message_with_reply_sync(
    GDBusConnection* conn, GDBusMessage* msg, GDBusSendMessageFlags flags,
    gint timeout, volatile guint32* out_serial, GCancellable* cancellable,
    GError** error) {
  GDBusMessage* ret_msg = static_cast<GDBusMessage*>(g_object_new(G_TYPE_OBJECT, NULL));

  return ret_msg;
}

GVariant* __fake_g_dbus_message_get_body(GDBusMessage* arg0) {
  GVariant* ret_body = g_variant_new("(v)", noti_body);

  return ret_body;
}

GDBusMessage* __fake_g_dbus_message_new_method_call(const gchar* arg0,
    const gchar* arg1, const gchar* arg2, const gchar* arg3) {
  GDBusMessage* ret_msg = static_cast<GDBusMessage*>(g_object_new(G_TYPE_OBJECT, NULL));

  return ret_msg;
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

  EXPECT_CALL(GetMock<GlibMock>(), g_bus_get_sync(_, _, _))
      .WillRepeatedly(Invoke(__fake_g_bus_get_sync));

  EXPECT_CALL(GetMock<GlibMock>(), g_dbus_message_new_method_call(_, _, _, _))
      .WillRepeatedly(Invoke(__fake_g_dbus_message_new_method_call));

  EXPECT_CALL(GetMock<GlibMock>(), g_dbus_connection_send_message_with_reply_sync(_, _, _, _, _, _, _))
      .WillRepeatedly(Invoke(__fake_g_dbus_connection_send_message_with_reply_sync));

  EXPECT_CALL(GetMock<GlibMock>(), g_dbus_message_get_body(_))
      .WillRepeatedly(Invoke(__fake_g_dbus_message_get_body));

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

  noti_body = notification_ipc_make_gvariant_from_noti(noti, false);

  notification_block_state_e ret_state;
  ret = notification_get_noti_block_state(&ret_state);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  ret = notification_save_as_template(noti, "test_template");
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  notification_h template_noti = nullptr;
  template_noti = notification_create_from_template("test_template");
  EXPECT_NE(template_noti, nullptr);

  ret = notification_update(noti);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  ret = notification_post(noti);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  notification_h tag_noti;
  tag_noti = notification_load_by_tag("test_tag");
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);
  EXPECT_NE(tag_noti, nullptr);

  ret = notification_delete(noti);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  ret = notification_delete_all(NOTIFICATION_TYPE_NOTI);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);
}
