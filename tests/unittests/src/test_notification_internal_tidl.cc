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

#include <cstdlib>
#include <cstring>

#include "mock/test_fixture.hh"
#include "mock/aul_mock.hh"
#include "mock/glib_mock.hh"

#include "notification/include/notification_private.h"
#include "notification/include/notification_setting_internal.h"
#include "notification/include/notification_tidl_proxy.h"

namespace {

int __fake_aul_app_get_appid_bypid(int pid, char* appid, int len) {
  snprintf(appid, len, "%s", "org.tizen.testappid");
  return AUL_R_OK;
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
    virtual public ::testing::NiceMock<GlibMock> {};
}  // namespace

using ::testing::_;
using ::testing::Invoke;

class NotificationInternalTidlTest : public TestFixture {
 public:
  NotificationInternalTidlTest() : TestFixture(std::make_unique<::Mocks>()) {}

  void SetUp() override {
    EXPECT_CALL(GetMock<AulMock>(), aul_app_get_appid_bypid(_, _, _))
        .WillRepeatedly(Invoke(__fake_aul_app_get_appid_bypid));
    EXPECT_CALL(GetMock<GlibMock>(), g_list_append(_, _))
        .WillRepeatedly(Invoke(__fake_g_list_append));
  }
};

TEST_F(NotificationInternalTidlTest, MakeEmptyNotificationSucceeds) {
  rpc_port_proxy_notification_h handle;
  ASSERT_EQ(rpc_port_proxy_notification_create(&handle), RPC_PORT_ERROR_NONE);

  EXPECT_EQ(make_empty_notification(handle), NOTIFICATION_ERROR_NONE);

  rpc_port_proxy_notification_destroy(handle);
}

TEST_F(NotificationInternalTidlTest, MakeNotificationFromNotiWithoutTranslate) {
  rpc_port_proxy_notification_h handle;
  ASSERT_EQ(rpc_port_proxy_notification_create(&handle), RPC_PORT_ERROR_NONE);

  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);
  notification_set_text(noti, NOTIFICATION_TEXT_TYPE_TITLE, "title",
      nullptr, NOTIFICATION_VARIABLE_TYPE_NONE);
  noti->pkg_id = strdup("org.tizen.testpkg");
  noti->channel_name = strdup("ch");

  EXPECT_EQ(make_notification_from_noti(handle, noti, false),
      NOTIFICATION_ERROR_NONE);

  notification_free(noti);
  rpc_port_proxy_notification_destroy(handle);
}

TEST_F(NotificationInternalTidlTest, MakeNotificationFromNotiWithTranslate) {
  rpc_port_proxy_notification_h handle;
  ASSERT_EQ(rpc_port_proxy_notification_create(&handle), RPC_PORT_ERROR_NONE);

  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  EXPECT_EQ(make_notification_from_noti(handle, noti, true),
      NOTIFICATION_ERROR_NONE);

  notification_free(noti);
  rpc_port_proxy_notification_destroy(handle);
}

TEST_F(NotificationInternalTidlTest, MakeNotiFromNotificationWithEmptyValues) {
  rpc_port_proxy_notification_h handle;
  ASSERT_EQ(rpc_port_proxy_notification_create(&handle), RPC_PORT_ERROR_NONE);
  ASSERT_EQ(make_empty_notification(handle), NOTIFICATION_ERROR_NONE);

  notification_h noti = nullptr;
  EXPECT_EQ(make_noti_from_notification(&noti, handle), NOTIFICATION_ERROR_NONE);
  ASSERT_NE(noti, nullptr);
  EXPECT_EQ(noti->pkg_id, nullptr);

  notification_free(noti);
  rpc_port_proxy_notification_destroy(handle);
}

TEST_F(NotificationInternalTidlTest, MakeNotiFromNotificationWithPopulatedValues) {
  rpc_port_proxy_notification_h handle;
  ASSERT_EQ(rpc_port_proxy_notification_create(&handle), RPC_PORT_ERROR_NONE);

  notification_h src = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(src, nullptr);
  src->pkg_id = strdup("org.tizen.testpkg");
  src->caller_app_id = strdup("org.tizen.caller");
  notification_set_text(src, NOTIFICATION_TEXT_TYPE_TITLE, "title",
      nullptr, NOTIFICATION_VARIABLE_TYPE_NONE);
  ASSERT_EQ(make_notification_from_noti(handle, src, false),
      NOTIFICATION_ERROR_NONE);
  notification_free(src);

  notification_h noti = nullptr;
  EXPECT_EQ(make_noti_from_notification(&noti, handle), NOTIFICATION_ERROR_NONE);
  ASSERT_NE(noti, nullptr);
  EXPECT_STREQ(noti->pkg_id, "org.tizen.testpkg");
  EXPECT_STREQ(noti->caller_app_id, "org.tizen.caller");
  EXPECT_NE(noti->b_text, nullptr);

  notification_free(noti);
  rpc_port_proxy_notification_destroy(handle);
}

TEST_F(NotificationInternalTidlTest, MakeDndAllowExceptionFromExceptionRejectsNull) {
  struct notification_system_setting_dnd_allow_exception exception = {1, 2};

  EXPECT_EQ(make_dnd_allow_exception_from_exception(nullptr, &exception),
      NOTIFICATION_ERROR_INVALID_PARAMETER);

  rpc_port_proxy_noti_system_setting_dnd_allow_exception_h handle;
  ASSERT_EQ(rpc_port_proxy_noti_system_setting_dnd_allow_exception_create(&handle),
      RPC_PORT_ERROR_NONE);
  EXPECT_EQ(make_dnd_allow_exception_from_exception(handle, nullptr),
      NOTIFICATION_ERROR_INVALID_PARAMETER);
  rpc_port_proxy_noti_system_setting_dnd_allow_exception_destroy(handle);
}

TEST_F(NotificationInternalTidlTest, SystemSettingRoundTripWithoutDndAllowExceptions) {
  rpc_port_proxy_noti_system_setting_h handle;
  ASSERT_EQ(rpc_port_proxy_noti_system_setting_create(&handle), RPC_PORT_ERROR_NONE);

  rpc_port_proxy_list_noti_system_setting_dnd_allow_exception_h empty_list;
  ASSERT_EQ(rpc_port_proxy_list_noti_system_setting_dnd_allow_exception_create(&empty_list),
      RPC_PORT_ERROR_NONE);
  ASSERT_EQ(rpc_port_proxy_noti_system_setting_set_dnd_allow_exceptions(handle, empty_list),
      RPC_PORT_ERROR_NONE);
  rpc_port_proxy_list_noti_system_setting_dnd_allow_exception_destroy(empty_list);

  struct notification_system_setting setting = {};
  setting.do_not_disturb = true;
  setting.visibility_class = 1;
  setting.lock_screen_content_level = HIDE_SENSITIVE_CONTENT;

  EXPECT_EQ(make_noti_system_setting_from_setting(handle, &setting),
      NOTIFICATION_ERROR_NONE);

  notification_system_setting_h out = nullptr;
  EXPECT_EQ(make_setting_from_noti_system_setting(&out, handle),
      NOTIFICATION_ERROR_NONE);
  ASSERT_NE(out, nullptr);
  EXPECT_EQ(out->do_not_disturb, true);
  EXPECT_EQ(g_list_length(out->dnd_allow_exceptions), 0u);

  free(out);
  rpc_port_proxy_noti_system_setting_destroy(handle);
}

TEST_F(NotificationInternalTidlTest, SystemSettingRoundTripWithDndAllowExceptions) {
  rpc_port_proxy_noti_system_setting_h handle;
  ASSERT_EQ(rpc_port_proxy_noti_system_setting_create(&handle), RPC_PORT_ERROR_NONE);

  struct notification_system_setting setting = {};
  struct notification_system_setting_dnd_allow_exception exception = {3, 4};
  setting.dnd_allow_exceptions = g_list_append(nullptr, &exception);

  EXPECT_EQ(make_noti_system_setting_from_setting(handle, &setting),
      NOTIFICATION_ERROR_NONE);
  g_list_free(setting.dnd_allow_exceptions);

  notification_system_setting_h out = nullptr;
  EXPECT_EQ(make_setting_from_noti_system_setting(&out, handle),
      NOTIFICATION_ERROR_NONE);
  ASSERT_NE(out, nullptr);
  ASSERT_EQ(g_list_length(out->dnd_allow_exceptions), 1u);

  dnd_allow_exception_h got =
      static_cast<dnd_allow_exception_h>(out->dnd_allow_exceptions->data);
  EXPECT_EQ(got->type, 3);
  EXPECT_EQ(got->value, 4);

  g_list_free_full(out->dnd_allow_exceptions, free);
  free(out);
  rpc_port_proxy_noti_system_setting_destroy(handle);
}

TEST_F(NotificationInternalTidlTest, SettingRoundTripWithEmptyValues) {
  rpc_port_proxy_noti_setting_h handle;
  ASSERT_EQ(rpc_port_proxy_noti_setting_create(&handle), RPC_PORT_ERROR_NONE);

  struct notification_setting setting = {};
  EXPECT_EQ(make_noti_setting_from_setting(handle, &setting), NOTIFICATION_ERROR_NONE);

  struct notification_setting out = {};
  EXPECT_EQ(make_setting_from_noti_setting(&out, handle), NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(out.package_name, nullptr);
  EXPECT_EQ(out.app_id, nullptr);

  rpc_port_proxy_noti_setting_destroy(handle);
}

TEST_F(NotificationInternalTidlTest, SettingRoundTripWithPopulatedValues) {
  rpc_port_proxy_noti_setting_h handle;
  ASSERT_EQ(rpc_port_proxy_noti_setting_create(&handle), RPC_PORT_ERROR_NONE);

  struct notification_setting setting = {};
  setting.package_name = strdup("org.tizen.testpkg");
  setting.app_id = strdup("org.tizen.testappid");
  setting.allow_to_notify = true;
  setting.app_disabled = true;

  EXPECT_EQ(make_noti_setting_from_setting(handle, &setting), NOTIFICATION_ERROR_NONE);
  free(setting.package_name);
  free(setting.app_id);

  struct notification_setting out = {};
  EXPECT_EQ(make_setting_from_noti_setting(&out, handle), NOTIFICATION_ERROR_NONE);
  EXPECT_STREQ(out.package_name, "org.tizen.testpkg");
  EXPECT_STREQ(out.app_id, "org.tizen.testappid");
  EXPECT_EQ(out.allow_to_notify, true);
  EXPECT_EQ(out.app_disabled, true);

  free(out.package_name);
  free(out.app_id);
  rpc_port_proxy_noti_setting_destroy(handle);
}
