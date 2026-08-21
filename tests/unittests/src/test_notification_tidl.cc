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

#include <cstring>

#include "mock/test_fixture.hh"
#include "mock/aul_mock.hh"
#include "mock/glib_mock.hh"
#include "mock/noti_service_proxy_mock.hh"

#include "notification/include/notification.h"
#include "notification/include/notification_internal.h"
#include "notification/include/notification_private.h"
#include "notification/include/notification_setting.h"
#include "notification/include/notification_setting_internal.h"
#include "notification/include/notification_tidl.h"

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
    virtual public ::testing::NiceMock<GlibMock>,
    virtual public ::testing::NiceMock<NotiServiceProxyMock> {};

}  // namespace

using ::testing::_;
using ::testing::Return;
using ::testing::Invoke;
using ::testing::DoAll;
using ::testing::SetArgPointee;

class NotificationTidlTest : public TestFixture {
 public:
  NotificationTidlTest() : TestFixture(std::make_unique<::Mocks>()) {}

  void SetUp() override {
    ON_CALL(GetMock<AulMock>(), aul_app_get_appid_bypid(_, _, _))
        .WillByDefault(Invoke(__fake_aul_app_get_appid_bypid));
    ON_CALL(GetMock<GlibMock>(), g_list_append(_, _))
        .WillByDefault(Invoke(__fake_g_list_append));
  }
};

TEST_F(NotificationTidlTest, IsMasterReadySucceedsWhenAlreadyConnected) {
  EXPECT_EQ(notification_tidl_is_master_ready(), NOTIFICATION_ERROR_NONE);
}

TEST_F(NotificationTidlTest, RequestRefreshFailsWhenInvokeFails) {
  EXPECT_CALL(GetMock<NotiServiceProxyMock>(),
      rpc_port_proxy_noti_service_invoke_refresh_noti(_, _))
      .WillOnce(Return(RPC_PORT_ERROR_IO_ERROR));
  EXPECT_EQ(notification_tidl_request_refresh(0), RPC_PORT_ERROR_IO_ERROR);
}

TEST_F(NotificationTidlTest, RequestDeleteByDisplayApplistSucceeds) {
  EXPECT_CALL(GetMock<NotiServiceProxyMock>(),
      rpc_port_proxy_noti_service_invoke_delete_noti_by_display_applist(_, _, _))
      .WillOnce(Return(RPC_PORT_ERROR_NONE));
  EXPECT_EQ(notification_tidl_request_delete_by_display_applist(1, 0),
      NOTIFICATION_ERROR_NONE);
}

TEST_F(NotificationTidlTest, RequestDeleteByDisplayApplistFails) {
  EXPECT_CALL(GetMock<NotiServiceProxyMock>(),
      rpc_port_proxy_noti_service_invoke_delete_noti_by_display_applist(_, _, _))
      .WillOnce(Return(RPC_PORT_ERROR_IO_ERROR));
  EXPECT_EQ(notification_tidl_request_delete_by_display_applist(1, 0),
      RPC_PORT_ERROR_IO_ERROR);
}

TEST_F(NotificationTidlTest, RequestGetAllCountSucceeds) {
  EXPECT_CALL(GetMock<NotiServiceProxyMock>(),
      rpc_port_proxy_noti_service_invoke_get_all_noti_count(_, _, _, _))
      .WillOnce(DoAll(SetArgPointee<3>(5), Return(RPC_PORT_ERROR_NONE)));
  int count = 0;
  EXPECT_EQ(notification_tidl_request_get_all_count(NOTIFICATION_TYPE_NOTI, &count, 0),
      NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(count, 5);
}

TEST_F(NotificationTidlTest, RequestGetAllCountFails) {
  EXPECT_CALL(GetMock<NotiServiceProxyMock>(),
      rpc_port_proxy_noti_service_invoke_get_all_noti_count(_, _, _, _))
      .WillOnce(Return(RPC_PORT_ERROR_IO_ERROR));
  int count = 0;
  EXPECT_EQ(notification_tidl_request_get_all_count(NOTIFICATION_TYPE_NOTI, &count, 0),
      RPC_PORT_ERROR_IO_ERROR);
}

TEST_F(NotificationTidlTest, RequestGetCountSucceeds) {
  EXPECT_CALL(GetMock<NotiServiceProxyMock>(),
      rpc_port_proxy_noti_service_invoke_get_noti_count(_, _, _, _, _, _, _))
      .WillOnce(DoAll(SetArgPointee<6>(3), Return(RPC_PORT_ERROR_NONE)));
  int count = 0;
  EXPECT_EQ(notification_tidl_request_get_count(NOTIFICATION_TYPE_NOTI,
      "org.tizen.testapp", 0, 0, &count, 0), NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(count, 3);
}

TEST_F(NotificationTidlTest, RequestGetCountFails) {
  EXPECT_CALL(GetMock<NotiServiceProxyMock>(),
      rpc_port_proxy_noti_service_invoke_get_noti_count(_, _, _, _, _, _, _))
      .WillOnce(Return(RPC_PORT_ERROR_IO_ERROR));
  int count = 0;
  EXPECT_EQ(notification_tidl_request_get_count(NOTIFICATION_TYPE_NOTI,
      "org.tizen.testapp", 0, 0, &count, 0), RPC_PORT_ERROR_IO_ERROR);
}

TEST_F(NotificationTidlTest, LoadNotiByPrivIdSucceeds) {
  rpc_port_proxy_notification_h handle;
  ASSERT_EQ(rpc_port_proxy_notification_create(&handle), RPC_PORT_ERROR_NONE);
  ASSERT_EQ(make_empty_notification(handle), NOTIFICATION_ERROR_NONE);

  EXPECT_CALL(GetMock<NotiServiceProxyMock>(),
      rpc_port_proxy_noti_service_invoke_load_noti_by_priv_id(_, _, _, _, _))
      .WillOnce(DoAll(SetArgPointee<4>(handle), Return(RPC_PORT_ERROR_NONE)));

  notification_h noti = nullptr;
  EXPECT_EQ(notification_tidl_request_load_noti_by_priv_id(&noti, nullptr, 1, 0),
      NOTIFICATION_ERROR_NONE);
  ASSERT_NE(noti, nullptr);
  notification_free(noti);
}

TEST_F(NotificationTidlTest, LoadNotiByPrivIdFailsOnInvoke) {
  EXPECT_CALL(GetMock<NotiServiceProxyMock>(),
      rpc_port_proxy_noti_service_invoke_load_noti_by_priv_id(_, _, _, _, _))
      .WillOnce(Return(RPC_PORT_ERROR_IO_ERROR));
  notification_h noti = nullptr;
  EXPECT_EQ(notification_tidl_request_load_noti_by_priv_id(&noti, "org.tizen.app", 1, 0),
      RPC_PORT_ERROR_IO_ERROR);
}

TEST_F(NotificationTidlTest, LoadNotiByTagSucceeds) {
  rpc_port_proxy_notification_h handle;
  ASSERT_EQ(rpc_port_proxy_notification_create(&handle), RPC_PORT_ERROR_NONE);
  ASSERT_EQ(make_empty_notification(handle), NOTIFICATION_ERROR_NONE);

  EXPECT_CALL(GetMock<NotiServiceProxyMock>(),
      rpc_port_proxy_noti_service_invoke_load_noti_by_tag(_, _, _, _, _))
      .WillOnce(DoAll(SetArgPointee<4>(handle), Return(RPC_PORT_ERROR_NONE)));

  notification_h noti = nullptr;
  EXPECT_EQ(notification_tidl_request_load_noti_by_tag(&noti, "org.tizen.app", "tag", 0),
      NOTIFICATION_ERROR_NONE);
  ASSERT_NE(noti, nullptr);
  notification_free(noti);
}

TEST_F(NotificationTidlTest, LoadNotiByTagFailsOnInvoke) {
  EXPECT_CALL(GetMock<NotiServiceProxyMock>(),
      rpc_port_proxy_noti_service_invoke_load_noti_by_tag(_, _, _, _, _))
      .WillOnce(Return(RPC_PORT_ERROR_IO_ERROR));
  notification_h noti = nullptr;
  EXPECT_EQ(notification_tidl_request_load_noti_by_tag(&noti, "org.tizen.app", "tag", 0),
      RPC_PORT_ERROR_IO_ERROR);
}

TEST_F(NotificationTidlTest, RequestLoadSystemSettingSucceeds) {
  rpc_port_proxy_noti_system_setting_h handle;
  ASSERT_EQ(rpc_port_proxy_noti_system_setting_create(&handle), RPC_PORT_ERROR_NONE);
  rpc_port_proxy_list_noti_system_setting_dnd_allow_exception_h empty_list;
  ASSERT_EQ(rpc_port_proxy_list_noti_system_setting_dnd_allow_exception_create(&empty_list),
      RPC_PORT_ERROR_NONE);
  ASSERT_EQ(rpc_port_proxy_noti_system_setting_set_dnd_allow_exceptions(handle, empty_list),
      RPC_PORT_ERROR_NONE);
  rpc_port_proxy_list_noti_system_setting_dnd_allow_exception_destroy(empty_list);

  EXPECT_CALL(GetMock<NotiServiceProxyMock>(),
      rpc_port_proxy_noti_service_invoke_load_system_setting(_, _, _))
      .WillOnce(DoAll(SetArgPointee<2>(handle), Return(RPC_PORT_ERROR_NONE)));

  notification_system_setting_h setting = nullptr;
  EXPECT_EQ(notification_tidl_request_load_system_setting(&setting, 0),
      NOTIFICATION_ERROR_NONE);
  ASSERT_NE(setting, nullptr);
  free(setting);
}

TEST_F(NotificationTidlTest, RequestLoadSystemSettingFails) {
  EXPECT_CALL(GetMock<NotiServiceProxyMock>(),
      rpc_port_proxy_noti_service_invoke_load_system_setting(_, _, _))
      .WillOnce(Return(RPC_PORT_ERROR_IO_ERROR));
  notification_system_setting_h setting = nullptr;
  EXPECT_EQ(notification_tidl_request_load_system_setting(&setting, 0),
      RPC_PORT_ERROR_IO_ERROR);
}

TEST_F(NotificationTidlTest, GetSettingArraySucceeds) {
  rpc_port_proxy_array_noti_setting_h handle_array;
  ASSERT_EQ(rpc_port_proxy_array_noti_setting_create(&handle_array), RPC_PORT_ERROR_NONE);
  rpc_port_proxy_noti_setting_h setting_handle;
  ASSERT_EQ(rpc_port_proxy_noti_setting_create(&setting_handle), RPC_PORT_ERROR_NONE);
  rpc_port_proxy_noti_setting_h arr[1] = { setting_handle };
  ASSERT_EQ(rpc_port_proxy_array_noti_setting_set(handle_array, arr, 1), RPC_PORT_ERROR_NONE);

  EXPECT_CALL(GetMock<NotiServiceProxyMock>(),
      rpc_port_proxy_noti_service_invoke_get_setting_array(_, _, _))
      .WillOnce(DoAll(SetArgPointee<2>(handle_array), Return(RPC_PORT_ERROR_NONE)));

  notification_setting_h setting_array = nullptr;
  int count = 0;
  EXPECT_EQ(notification_tidl_request_get_setting_array(&setting_array, &count, 0),
      NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(count, 1);
  ASSERT_NE(setting_array, nullptr);
  free(setting_array);
}

TEST_F(NotificationTidlTest, GetSettingArrayFailsOnInvoke) {
  EXPECT_CALL(GetMock<NotiServiceProxyMock>(),
      rpc_port_proxy_noti_service_invoke_get_setting_array(_, _, _))
      .WillOnce(Return(RPC_PORT_ERROR_IO_ERROR));
  notification_setting_h setting_array = nullptr;
  int count = 0;
  EXPECT_EQ(notification_tidl_request_get_setting_array(&setting_array, &count, 0),
      RPC_PORT_ERROR_IO_ERROR);
}

TEST_F(NotificationTidlTest, GetSettingByAppIdSucceeds) {
  rpc_port_proxy_noti_setting_h handle;
  ASSERT_EQ(rpc_port_proxy_noti_setting_create(&handle), RPC_PORT_ERROR_NONE);
  struct notification_setting seed = {};
  ASSERT_EQ(make_noti_setting_from_setting(handle, &seed), NOTIFICATION_ERROR_NONE);

  EXPECT_CALL(GetMock<NotiServiceProxyMock>(),
      rpc_port_proxy_noti_service_invoke_get_setting_by_app_id(_, _, _, _))
      .WillOnce(DoAll(SetArgPointee<3>(handle), Return(RPC_PORT_ERROR_NONE)));

  notification_setting_h setting = nullptr;
  EXPECT_EQ(notification_tidl_request_get_setting_by_app_id("org.tizen.app", &setting, 0),
      NOTIFICATION_ERROR_NONE);
  ASSERT_NE(setting, nullptr);
  free(setting);
}

TEST_F(NotificationTidlTest, GetSettingByAppIdFailsOnInvoke) {
  EXPECT_CALL(GetMock<NotiServiceProxyMock>(),
      rpc_port_proxy_noti_service_invoke_get_setting_by_app_id(_, _, _, _))
      .WillOnce(Return(RPC_PORT_ERROR_IO_ERROR));
  notification_setting_h setting = nullptr;
  EXPECT_EQ(notification_tidl_request_get_setting_by_app_id("org.tizen.app", &setting, 0),
      RPC_PORT_ERROR_IO_ERROR);
}

TEST_F(NotificationTidlTest, UpdateSystemSettingSucceeds) {
  struct notification_system_setting setting = {};
  setting.do_not_disturb = true;

  EXPECT_CALL(GetMock<NotiServiceProxyMock>(),
      rpc_port_proxy_noti_service_invoke_update_noti_sys_setting(_, _, _))
      .WillOnce(Return(RPC_PORT_ERROR_NONE));
  EXPECT_EQ(notification_tidl_update_system_setting(&setting, 0), NOTIFICATION_ERROR_NONE);
}

TEST_F(NotificationTidlTest, UpdateSystemSettingFailsOnInvoke) {
  struct notification_system_setting setting = {};

  EXPECT_CALL(GetMock<NotiServiceProxyMock>(),
      rpc_port_proxy_noti_service_invoke_update_noti_sys_setting(_, _, _))
      .WillOnce(Return(RPC_PORT_ERROR_IO_ERROR));
  EXPECT_EQ(notification_tidl_update_system_setting(&setting, 0), RPC_PORT_ERROR_IO_ERROR);
}

TEST_F(NotificationTidlTest, LoadNotiGroupingListSucceeds) {
  rpc_port_proxy_notification_h nh;
  ASSERT_EQ(rpc_port_proxy_notification_create(&nh), RPC_PORT_ERROR_NONE);
  ASSERT_EQ(make_empty_notification(nh), NOTIFICATION_ERROR_NONE);
  rpc_port_proxy_list_notification_h list_h =
      reinterpret_cast<rpc_port_proxy_list_notification_h>(0x2);

  EXPECT_CALL(GetMock<NotiServiceProxyMock>(),
      rpc_port_proxy_noti_service_invoke_load_noti_grouping_list(_, _, _, _, _, _))
      .WillOnce(DoAll(SetArgPointee<5>(list_h), Return(RPC_PORT_ERROR_NONE)));
  EXPECT_CALL(GetMock<NotiServiceProxyMock>(),
      rpc_port_proxy_list_notification_foreach(list_h, _, _))
      .WillOnce(Invoke([nh](rpc_port_proxy_list_notification_h,
          rpc_port_proxy_list_notification_cb cb, void *user_data) {
        cb(nh, user_data);
        return RPC_PORT_ERROR_NONE;
      }));
  EXPECT_CALL(GetMock<NotiServiceProxyMock>(),
      rpc_port_proxy_list_notification_destroy(list_h))
      .WillOnce(Return(RPC_PORT_ERROR_NONE));

  notification_list_h list = nullptr;
  EXPECT_EQ(notification_tidl_request_load_noti_grouping_list(
      NOTIFICATION_TYPE_NOTI, 10, 5, &list, 0), NOTIFICATION_ERROR_NONE);
  ASSERT_NE(list, nullptr);
  notification_free_list(list);
  rpc_port_proxy_notification_destroy(nh);
}

TEST_F(NotificationTidlTest, LoadNotiGroupingListFailsOnInvoke) {
  EXPECT_CALL(GetMock<NotiServiceProxyMock>(),
      rpc_port_proxy_noti_service_invoke_load_noti_grouping_list(_, _, _, _, _, _))
      .WillOnce(Return(RPC_PORT_ERROR_IO_ERROR));
  notification_list_h list = nullptr;
  EXPECT_EQ(notification_tidl_request_load_noti_grouping_list(
      NOTIFICATION_TYPE_NOTI, 10, 5, &list, 0), RPC_PORT_ERROR_IO_ERROR);
}

TEST_F(NotificationTidlTest, LoadNotiGroupingListFailsOnForeach) {
  rpc_port_proxy_list_notification_h list_h =
      reinterpret_cast<rpc_port_proxy_list_notification_h>(0x2);
  EXPECT_CALL(GetMock<NotiServiceProxyMock>(),
      rpc_port_proxy_noti_service_invoke_load_noti_grouping_list(_, _, _, _, _, _))
      .WillOnce(DoAll(SetArgPointee<5>(list_h), Return(RPC_PORT_ERROR_NONE)));
  EXPECT_CALL(GetMock<NotiServiceProxyMock>(),
      rpc_port_proxy_list_notification_foreach(list_h, _, _))
      .WillOnce(Return(RPC_PORT_ERROR_IO_ERROR));
  EXPECT_CALL(GetMock<NotiServiceProxyMock>(),
      rpc_port_proxy_list_notification_destroy(list_h))
      .WillOnce(Return(RPC_PORT_ERROR_NONE));

  notification_list_h list = nullptr;
  EXPECT_EQ(notification_tidl_request_load_noti_grouping_list(
      NOTIFICATION_TYPE_NOTI, 10, 5, &list, 0), RPC_PORT_ERROR_IO_ERROR);
}

TEST_F(NotificationTidlTest, LoadNotiDetailListSucceeds) {
  rpc_port_proxy_notification_h nh;
  ASSERT_EQ(rpc_port_proxy_notification_create(&nh), RPC_PORT_ERROR_NONE);
  ASSERT_EQ(make_empty_notification(nh), NOTIFICATION_ERROR_NONE);
  rpc_port_proxy_list_notification_h list_h =
      reinterpret_cast<rpc_port_proxy_list_notification_h>(0x3);

  EXPECT_CALL(GetMock<NotiServiceProxyMock>(),
      rpc_port_proxy_noti_service_invoke_load_noti_detail_list(_, _, _, _, _, _, _))
      .WillOnce(DoAll(SetArgPointee<6>(list_h), Return(RPC_PORT_ERROR_NONE)));
  EXPECT_CALL(GetMock<NotiServiceProxyMock>(),
      rpc_port_proxy_list_notification_foreach(list_h, _, _))
      .WillOnce(Invoke([nh](rpc_port_proxy_list_notification_h,
          rpc_port_proxy_list_notification_cb cb, void *user_data) {
        cb(nh, user_data);
        return RPC_PORT_ERROR_NONE;
      }));
  EXPECT_CALL(GetMock<NotiServiceProxyMock>(),
      rpc_port_proxy_list_notification_destroy(list_h))
      .WillOnce(Return(RPC_PORT_ERROR_NONE));

  notification_list_h list = nullptr;
  EXPECT_EQ(notification_tidl_request_load_noti_detail_list(
      "org.tizen.app", 0, 0, 10, &list, 0), NOTIFICATION_ERROR_NONE);
  ASSERT_NE(list, nullptr);
  notification_free_list(list);
  rpc_port_proxy_notification_destroy(nh);
}

TEST_F(NotificationTidlTest, LoadNotiDetailListFailsOnInvoke) {
  EXPECT_CALL(GetMock<NotiServiceProxyMock>(),
      rpc_port_proxy_noti_service_invoke_load_noti_detail_list(_, _, _, _, _, _, _))
      .WillOnce(Return(RPC_PORT_ERROR_IO_ERROR));
  notification_list_h list = nullptr;
  EXPECT_EQ(notification_tidl_request_load_noti_detail_list(
      "org.tizen.app", 0, 0, 10, &list, 0), RPC_PORT_ERROR_IO_ERROR);
}

TEST_F(NotificationTidlTest, CheckEventReceiverSucceeds) {
  EXPECT_CALL(GetMock<NotiServiceProxyMock>(),
      rpc_port_proxy_noti_service_invoke_check_event_receiver(_, _, _))
      .WillOnce(DoAll(SetArgPointee<2>(true), Return(RPC_PORT_ERROR_NONE)));
  bool available = false;
  EXPECT_EQ(notification_tidl_check_event_receiver(1, &available), NOTIFICATION_ERROR_NONE);
  EXPECT_TRUE(available);
}

TEST_F(NotificationTidlTest, CheckEventReceiverFails) {
  EXPECT_CALL(GetMock<NotiServiceProxyMock>(),
      rpc_port_proxy_noti_service_invoke_check_event_receiver(_, _, _))
      .WillOnce(Return(RPC_PORT_ERROR_IO_ERROR));
  bool available = false;
  EXPECT_EQ(notification_tidl_check_event_receiver(1, &available), RPC_PORT_ERROR_IO_ERROR);
}

TEST_F(NotificationTidlTest, ResetEventHandlerSucceeds) {
  EXPECT_CALL(GetMock<NotiServiceProxyMock>(),
      rpc_port_proxy_noti_service_invoke_reset_event_handler(_, _))
      .WillOnce(Return(RPC_PORT_ERROR_NONE));
  notification_tidl_reset_event_handler(1);
}

TEST_F(NotificationTidlTest, ResetEventHandlerFails) {
  EXPECT_CALL(GetMock<NotiServiceProxyMock>(),
      rpc_port_proxy_noti_service_invoke_reset_event_handler(_, _))
      .WillOnce(Return(RPC_PORT_ERROR_IO_ERROR));
  notification_tidl_reset_event_handler(1);
}

TEST_F(NotificationTidlTest, RegisterDndAppSucceeds) {
  EXPECT_CALL(GetMock<NotiServiceProxyMock>(),
      rpc_port_proxy_noti_service_invoke_register_dnd_app(_, _, _))
      .WillOnce(Return(RPC_PORT_ERROR_NONE));
  EXPECT_EQ(notification_tidl_request_register_dnd_app(0, 100), NOTIFICATION_ERROR_NONE);
}

TEST_F(NotificationTidlTest, RegisterDndAppFails) {
  EXPECT_CALL(GetMock<NotiServiceProxyMock>(),
      rpc_port_proxy_noti_service_invoke_register_dnd_app(_, _, _))
      .WillOnce(Return(RPC_PORT_ERROR_IO_ERROR));
  EXPECT_EQ(notification_tidl_request_register_dnd_app(0, 100), RPC_PORT_ERROR_IO_ERROR);
}

TEST_F(NotificationTidlTest, UnregisterDndAppSucceeds) {
  EXPECT_CALL(GetMock<NotiServiceProxyMock>(),
      rpc_port_proxy_noti_service_invoke_unregister_dnd_app(_, _, _))
      .WillOnce(Return(RPC_PORT_ERROR_NONE));
  EXPECT_EQ(notification_tidl_request_unregister_dnd_app(0, 100), NOTIFICATION_ERROR_NONE);
}

TEST_F(NotificationTidlTest, UnregisterDndAppFails) {
  EXPECT_CALL(GetMock<NotiServiceProxyMock>(),
      rpc_port_proxy_noti_service_invoke_unregister_dnd_app(_, _, _))
      .WillOnce(Return(RPC_PORT_ERROR_IO_ERROR));
  EXPECT_EQ(notification_tidl_request_unregister_dnd_app(0, 100), RPC_PORT_ERROR_IO_ERROR);
}


TEST_F(NotificationTidlTest, EventMonitorInitFailsWhenCreateFails) {
  EXPECT_CALL(GetMock<NotiServiceProxyMock>(),
      rpc_port_proxy_noti_service_event_received_create(_))
      .WillOnce(Return(RPC_PORT_ERROR_IO_ERROR));
  EXPECT_EQ(notification_tidl_event_monitor_init(), RPC_PORT_ERROR_IO_ERROR);
}

TEST_F(NotificationTidlTest, EventMonitorInitFailsWhenSetCallbackFails) {
  rpc_port_proxy_noti_service_event_received_h h =
      reinterpret_cast<rpc_port_proxy_noti_service_event_received_h>(0x10);
  EXPECT_CALL(GetMock<NotiServiceProxyMock>(),
      rpc_port_proxy_noti_service_event_received_create(_))
      .WillOnce(DoAll(SetArgPointee<0>(h), Return(RPC_PORT_ERROR_NONE)));
  EXPECT_CALL(GetMock<NotiServiceProxyMock>(),
      rpc_port_proxy_noti_service_event_received_set_callback(h, _, _))
      .WillOnce(Return(RPC_PORT_ERROR_IO_ERROR));
  EXPECT_CALL(GetMock<NotiServiceProxyMock>(),
      rpc_port_proxy_noti_service_event_received_destroy(h))
      .WillOnce(Return(RPC_PORT_ERROR_NONE));
  EXPECT_EQ(notification_tidl_event_monitor_init(), RPC_PORT_ERROR_IO_ERROR);
}

TEST_F(NotificationTidlTest, EventMonitorInitFailsWhenRegisterEventFails) {
  rpc_port_proxy_noti_service_event_received_h h =
      reinterpret_cast<rpc_port_proxy_noti_service_event_received_h>(0x11);
  EXPECT_CALL(GetMock<NotiServiceProxyMock>(),
      rpc_port_proxy_noti_service_event_received_create(_))
      .WillOnce(DoAll(SetArgPointee<0>(h), Return(RPC_PORT_ERROR_NONE)));
  EXPECT_CALL(GetMock<NotiServiceProxyMock>(),
      rpc_port_proxy_noti_service_event_received_set_callback(h, _, _))
      .WillOnce(Return(RPC_PORT_ERROR_NONE));
  EXPECT_CALL(GetMock<NotiServiceProxyMock>(),
      rpc_port_proxy_noti_service_invoke_register_event(_, h))
      .WillOnce(Return(RPC_PORT_ERROR_IO_ERROR));
  EXPECT_CALL(GetMock<NotiServiceProxyMock>(),
      rpc_port_proxy_noti_service_event_received_destroy(h))
      .WillOnce(Return(RPC_PORT_ERROR_NONE));
  EXPECT_EQ(notification_tidl_event_monitor_init(), RPC_PORT_ERROR_IO_ERROR);
}

namespace {
rpc_port_proxy_noti_service_event_received_cb g_captured_event_cb = nullptr;
void *g_captured_event_cb_data = nullptr;
}  // namespace

TEST_F(NotificationTidlTest, EventMonitorInitSucceedsAndCapturesCallback) {
  rpc_port_proxy_noti_service_event_received_h h =
      reinterpret_cast<rpc_port_proxy_noti_service_event_received_h>(0x12);
  EXPECT_CALL(GetMock<NotiServiceProxyMock>(),
      rpc_port_proxy_noti_service_event_received_create(_))
      .WillOnce(DoAll(SetArgPointee<0>(h), Return(RPC_PORT_ERROR_NONE)));
  EXPECT_CALL(GetMock<NotiServiceProxyMock>(),
      rpc_port_proxy_noti_service_event_received_set_callback(h, _, _))
      .WillOnce(Invoke([](rpc_port_proxy_noti_service_event_received_h,
          rpc_port_proxy_noti_service_event_received_cb cb, void *data) {
        g_captured_event_cb = cb;
        g_captured_event_cb_data = data;
        return RPC_PORT_ERROR_NONE;
      }));
  EXPECT_CALL(GetMock<NotiServiceProxyMock>(),
      rpc_port_proxy_noti_service_invoke_register_event(_, h))
      .WillOnce(Return(RPC_PORT_ERROR_NONE));

  EXPECT_EQ(notification_tidl_event_monitor_init(), NOTIFICATION_ERROR_NONE);
  ASSERT_NE(g_captured_event_cb, nullptr);

  EXPECT_EQ(notification_tidl_event_monitor_init(), NOTIFICATION_ERROR_NONE);
}

TEST_F(NotificationTidlTest, CapturedEventCallbackDispatchesSendEvent) {
  ASSERT_NE(g_captured_event_cb, nullptr);

  rpc_port_proxy_notification_h nh;
  ASSERT_EQ(rpc_port_proxy_notification_create(&nh), RPC_PORT_ERROR_NONE);
  ASSERT_EQ(make_empty_notification(nh), NOTIFICATION_ERROR_NONE);
  ASSERT_EQ(rpc_port_proxy_notification_set_priv_id(nh, 42), RPC_PORT_ERROR_NONE);

  g_captured_event_cb(g_captured_event_cb_data, nh, "send_event", 0);

  rpc_port_proxy_notification_destroy(nh);
}

TEST_F(NotificationTidlTest, CapturedEventCallbackDispatchesDeleteNoti) {
  ASSERT_NE(g_captured_event_cb, nullptr);

  rpc_port_proxy_notification_h nh;
  ASSERT_EQ(rpc_port_proxy_notification_create(&nh), RPC_PORT_ERROR_NONE);
  ASSERT_EQ(rpc_port_proxy_notification_set_priv_id(nh, 43), RPC_PORT_ERROR_NONE);

  g_captured_event_cb(g_captured_event_cb_data, nh, "delete_noti", 0);

  rpc_port_proxy_notification_destroy(nh);
}

TEST_F(NotificationTidlTest, EventMonitorFiniFailsWhenUnregisterFails) {
  EXPECT_CALL(GetMock<NotiServiceProxyMock>(),
      rpc_port_proxy_noti_service_invoke_unregister_event(_))
      .WillOnce(Return(RPC_PORT_ERROR_IO_ERROR));
  notification_tidl_event_monitor_fini();
}

TEST_F(NotificationTidlTest, EventMonitorFiniSucceedsAndDisposes) {
  EXPECT_CALL(GetMock<NotiServiceProxyMock>(),
      rpc_port_proxy_noti_service_invoke_unregister_event(_))
      .WillOnce(Return(RPC_PORT_ERROR_NONE));
  EXPECT_CALL(GetMock<NotiServiceProxyMock>(),
      rpc_port_proxy_noti_service_event_received_dispose(_, _))
      .WillOnce(Return(RPC_PORT_ERROR_NONE));
  notification_tidl_event_monitor_fini();
}

TEST_F(NotificationTidlTest, MonitorInitFailsWhenCreateFails) {
  EXPECT_CALL(GetMock<NotiServiceProxyMock>(),
      rpc_port_proxy_noti_service_changed_create(_))
      .WillOnce(Return(RPC_PORT_ERROR_IO_ERROR));
  EXPECT_EQ(notification_tidl_monitor_init(), RPC_PORT_ERROR_IO_ERROR);
}

TEST_F(NotificationTidlTest, MonitorInitFailsWhenSetCallbackFails) {
  rpc_port_proxy_noti_service_changed_h h =
      reinterpret_cast<rpc_port_proxy_noti_service_changed_h>(0x20);
  EXPECT_CALL(GetMock<NotiServiceProxyMock>(),
      rpc_port_proxy_noti_service_changed_create(_))
      .WillOnce(DoAll(SetArgPointee<0>(h), Return(RPC_PORT_ERROR_NONE)));
  EXPECT_CALL(GetMock<NotiServiceProxyMock>(),
      rpc_port_proxy_noti_service_changed_set_callback(h, _, _))
      .WillOnce(Return(RPC_PORT_ERROR_IO_ERROR));
  EXPECT_CALL(GetMock<NotiServiceProxyMock>(),
      rpc_port_proxy_noti_service_changed_destroy(h))
      .WillOnce(Return(RPC_PORT_ERROR_NONE));
  EXPECT_EQ(notification_tidl_monitor_init(), RPC_PORT_ERROR_IO_ERROR);
}

TEST_F(NotificationTidlTest, MonitorInitFailsWhenRegisterChangedFails) {
  rpc_port_proxy_noti_service_changed_h h =
      reinterpret_cast<rpc_port_proxy_noti_service_changed_h>(0x21);
  EXPECT_CALL(GetMock<NotiServiceProxyMock>(),
      rpc_port_proxy_noti_service_changed_create(_))
      .WillOnce(DoAll(SetArgPointee<0>(h), Return(RPC_PORT_ERROR_NONE)));
  EXPECT_CALL(GetMock<NotiServiceProxyMock>(),
      rpc_port_proxy_noti_service_changed_set_callback(h, _, _))
      .WillOnce(Return(RPC_PORT_ERROR_NONE));
  EXPECT_CALL(GetMock<NotiServiceProxyMock>(),
      rpc_port_proxy_noti_service_invoke_register_changed(_, h, _))
      .WillOnce(Return(RPC_PORT_ERROR_IO_ERROR));
  EXPECT_CALL(GetMock<NotiServiceProxyMock>(),
      rpc_port_proxy_noti_service_changed_destroy(h))
      .WillOnce(Return(RPC_PORT_ERROR_NONE));
  EXPECT_EQ(notification_tidl_monitor_init(), RPC_PORT_ERROR_IO_ERROR);
}

namespace {
rpc_port_proxy_noti_service_changed_cb g_captured_changed_cb = nullptr;
void *g_captured_changed_cb_data = nullptr;
}  // namespace

TEST_F(NotificationTidlTest, MonitorInitSucceedsAndCapturesCallback) {
  rpc_port_proxy_noti_service_changed_h h =
      reinterpret_cast<rpc_port_proxy_noti_service_changed_h>(0x22);
  EXPECT_CALL(GetMock<NotiServiceProxyMock>(),
      rpc_port_proxy_noti_service_changed_create(_))
      .WillOnce(DoAll(SetArgPointee<0>(h), Return(RPC_PORT_ERROR_NONE)));
  EXPECT_CALL(GetMock<NotiServiceProxyMock>(),
      rpc_port_proxy_noti_service_changed_set_callback(h, _, _))
      .WillOnce(Invoke([](rpc_port_proxy_noti_service_changed_h,
          rpc_port_proxy_noti_service_changed_cb cb, void *data) {
        g_captured_changed_cb = cb;
        g_captured_changed_cb_data = data;
        return RPC_PORT_ERROR_NONE;
      }));
  EXPECT_CALL(GetMock<NotiServiceProxyMock>(),
      rpc_port_proxy_noti_service_invoke_register_changed(_, h, _))
      .WillOnce(Return(RPC_PORT_ERROR_NONE));
  EXPECT_CALL(GetMock<NotiServiceProxyMock>(),
      rpc_port_proxy_noti_service_invoke_register_service(_, _))
      .WillOnce(Return(RPC_PORT_ERROR_NONE));

  EXPECT_EQ(notification_tidl_monitor_init(), NOTIFICATION_ERROR_NONE);
  ASSERT_NE(g_captured_changed_cb, nullptr);

  EXPECT_EQ(notification_tidl_monitor_init(), NOTIFICATION_ERROR_NONE);
}

TEST_F(NotificationTidlTest, CapturedChangedCallbackDispatchesAddNotiNotify) {
  ASSERT_NE(g_captured_changed_cb, nullptr);

  rpc_port_proxy_notification_h nh;
  ASSERT_EQ(rpc_port_proxy_notification_create(&nh), RPC_PORT_ERROR_NONE);
  ASSERT_EQ(make_empty_notification(nh), NOTIFICATION_ERROR_NONE);
  ASSERT_EQ(rpc_port_proxy_notification_set_flags_for_property(nh, 0), RPC_PORT_ERROR_NONE);
  ASSERT_EQ(rpc_port_proxy_notification_set_priv_id(nh, 1), RPC_PORT_ERROR_NONE);
  ASSERT_EQ(rpc_port_proxy_notification_set_uid(nh, 0), RPC_PORT_ERROR_NONE);

  g_captured_changed_cb(g_captured_changed_cb_data, nh, "add_noti_notify", nullptr);

  rpc_port_proxy_notification_destroy(nh);
}

TEST_F(NotificationTidlTest, CapturedChangedCallbackDispatchesAddNotiNotifyDisabled) {
  ASSERT_NE(g_captured_changed_cb, nullptr);

  rpc_port_proxy_notification_h nh;
  ASSERT_EQ(rpc_port_proxy_notification_create(&nh), RPC_PORT_ERROR_NONE);
  ASSERT_EQ(make_empty_notification(nh), NOTIFICATION_ERROR_NONE);
  ASSERT_EQ(rpc_port_proxy_notification_set_flags_for_property(nh,
      NOTIFICATION_PROP_DISABLE_UPDATE_ON_INSERT), RPC_PORT_ERROR_NONE);
  ASSERT_EQ(rpc_port_proxy_notification_set_priv_id(nh, 2), RPC_PORT_ERROR_NONE);

  g_captured_changed_cb(g_captured_changed_cb_data, nh, "add_noti_notify", nullptr);

  rpc_port_proxy_notification_destroy(nh);
}

TEST_F(NotificationTidlTest, CapturedChangedCallbackDispatchesUpdateNotiNotify) {
  ASSERT_NE(g_captured_changed_cb, nullptr);

  rpc_port_proxy_notification_h nh;
  ASSERT_EQ(rpc_port_proxy_notification_create(&nh), RPC_PORT_ERROR_NONE);
  ASSERT_EQ(make_empty_notification(nh), NOTIFICATION_ERROR_NONE);
  ASSERT_EQ(rpc_port_proxy_notification_set_priv_id(nh, 3), RPC_PORT_ERROR_NONE);
  ASSERT_EQ(rpc_port_proxy_notification_set_uid(nh, 0), RPC_PORT_ERROR_NONE);

  g_captured_changed_cb(g_captured_changed_cb_data, nh, "update_noti_notify", nullptr);

  rpc_port_proxy_notification_destroy(nh);
}

TEST_F(NotificationTidlTest, CapturedChangedCallbackDispatchesDeleteSingleNotify) {
  ASSERT_NE(g_captured_changed_cb, nullptr);

  rpc_port_proxy_notification_h nh;
  ASSERT_EQ(rpc_port_proxy_notification_create(&nh), RPC_PORT_ERROR_NONE);
  ASSERT_EQ(rpc_port_proxy_notification_set_priv_id(nh, 4), RPC_PORT_ERROR_NONE);
  ASSERT_EQ(rpc_port_proxy_notification_set_uid(nh, 0), RPC_PORT_ERROR_NONE);
  ASSERT_EQ(rpc_port_proxy_notification_set_caller_app_id(nh, "org.tizen.app"),
      RPC_PORT_ERROR_NONE);

  g_captured_changed_cb(g_captured_changed_cb_data, nh, "delete_single_notify", nullptr);

  rpc_port_proxy_notification_destroy(nh);
}

TEST_F(NotificationTidlTest, CapturedChangedCallbackDispatchesDeleteMultipleNotify) {
  ASSERT_NE(g_captured_changed_cb, nullptr);

  rpc_port_proxy_notification_h nh;
  ASSERT_EQ(rpc_port_proxy_notification_create(&nh), RPC_PORT_ERROR_NONE);
  ASSERT_EQ(rpc_port_proxy_notification_set_uid(nh, 0), RPC_PORT_ERROR_NONE);

  rpc_port_proxy_array_int_h extra;
  ASSERT_EQ(rpc_port_proxy_array_int_create(&extra), RPC_PORT_ERROR_NONE);
  int ids[2] = { 1, 2 };
  ASSERT_EQ(rpc_port_proxy_array_int_set(extra, ids, 2), RPC_PORT_ERROR_NONE);

  g_captured_changed_cb(g_captured_changed_cb_data, nh, "delete_multiple_notify", extra);

  rpc_port_proxy_notification_destroy(nh);
  rpc_port_proxy_array_int_destroy(extra);
}

TEST_F(NotificationTidlTest, CapturedChangedCallbackDispatchesRefreshNotiNotify) {
  ASSERT_NE(g_captured_changed_cb, nullptr);

  rpc_port_proxy_notification_h nh;
  ASSERT_EQ(rpc_port_proxy_notification_create(&nh), RPC_PORT_ERROR_NONE);
  ASSERT_EQ(rpc_port_proxy_notification_set_uid(nh, 0), RPC_PORT_ERROR_NONE);

  g_captured_changed_cb(g_captured_changed_cb_data, nh, "refresh_noti_notify", nullptr);

  rpc_port_proxy_notification_destroy(nh);
}

TEST_F(NotificationTidlTest, CapturedChangedCallbackDispatchesChangeDndNotify) {
  ASSERT_NE(g_captured_changed_cb, nullptr);

  rpc_port_proxy_notification_h nh;
  ASSERT_EQ(rpc_port_proxy_notification_create(&nh), RPC_PORT_ERROR_NONE);
  ASSERT_EQ(rpc_port_proxy_notification_set_uid(nh, 0), RPC_PORT_ERROR_NONE);

  rpc_port_proxy_array_int_h extra;
  ASSERT_EQ(rpc_port_proxy_array_int_create(&extra), RPC_PORT_ERROR_NONE);
  int dnd[1] = { 1 };
  ASSERT_EQ(rpc_port_proxy_array_int_set(extra, dnd, 1), RPC_PORT_ERROR_NONE);

  g_captured_changed_cb(g_captured_changed_cb_data, nh, "change_dnd_notify", extra);

  rpc_port_proxy_notification_destroy(nh);
  rpc_port_proxy_array_int_destroy(extra);
}

TEST_F(NotificationTidlTest, CapturedChangedCallbackDispatchesDeleteByDisplayApplistNotify) {
  ASSERT_NE(g_captured_changed_cb, nullptr);

  rpc_port_proxy_notification_h nh;
  ASSERT_EQ(rpc_port_proxy_notification_create(&nh), RPC_PORT_ERROR_NONE);
  ASSERT_EQ(rpc_port_proxy_notification_set_uid(nh, 0), RPC_PORT_ERROR_NONE);

  rpc_port_proxy_array_int_h extra;
  ASSERT_EQ(rpc_port_proxy_array_int_create(&extra), RPC_PORT_ERROR_NONE);
  int ids[1] = { 7 };
  ASSERT_EQ(rpc_port_proxy_array_int_set(extra, ids, 1), RPC_PORT_ERROR_NONE);

  g_captured_changed_cb(g_captured_changed_cb_data, nh,
      "delete_by_display_applist_notify", extra);

  rpc_port_proxy_notification_destroy(nh);
  rpc_port_proxy_array_int_destroy(extra);
}

TEST_F(NotificationTidlTest, MonitorFiniFailsWhenUnregisterFails) {
  EXPECT_CALL(GetMock<NotiServiceProxyMock>(),
      rpc_port_proxy_noti_service_invoke_unregister_changed(_))
      .WillOnce(Return(RPC_PORT_ERROR_IO_ERROR));
  EXPECT_EQ(notification_tidl_monitor_fini(), RPC_PORT_ERROR_IO_ERROR);
}

TEST_F(NotificationTidlTest, MonitorFiniSucceedsAndDisposes) {
  EXPECT_CALL(GetMock<NotiServiceProxyMock>(),
      rpc_port_proxy_noti_service_invoke_unregister_changed(_))
      .WillOnce(Return(RPC_PORT_ERROR_NONE));
  EXPECT_CALL(GetMock<NotiServiceProxyMock>(),
      rpc_port_proxy_noti_service_changed_dispose(_, _))
      .WillOnce(Return(RPC_PORT_ERROR_NONE));
  EXPECT_EQ(notification_tidl_monitor_fini(), NOTIFICATION_ERROR_NONE);
}
