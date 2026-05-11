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

#include <sys/types.h>

#include "mock/test_fixture.hh"
#include "mock/glib_mock.hh"
#include "mock/aul_mock.hh"
#include "mock/package_manager_mock.hh"
#include "mock/socket_mock.hh"

#include "notification/include/notification.h"
#include "notification/include/notification_list.h"

namespace {

// typedef struct {
// 	const char *package;
// 	char *locale;
// 	pkgmgrinfo_app_component app_component;
// 	application_x *app_info;
// } pkgmgr_appinfo_x;

int __fake_aul_app_get_appid_bypid(int pid, char* appid, int len) {
  snprintf(appid, len, "%s", "org.tizen.testappid");
  return AUL_R_OK;
}

// GDBusConnection* __fake_g_bus_get_sync(GBusType type, GCancellable* cancellable, GError** error) {
//   GDBusConnection* conn = reinterpret_cast<GDBusConnection*>(calloc(1, 4));

//   return conn;
// }

// GDBusMessage* __fake_g_dbus_connection_send_message_with_reply_sync(
//     GDBusConnection* conn, GDBusMessage* msg, GDBusSendMessageFlags flags,
//     gint timeout, volatile guint32* out_serial, GCancellable* cancellable,
//     GError** error) {
//   GDBusMessage* ret_msg = static_cast<GDBusMessage*>(g_object_new(G_TYPE_OBJECT, NULL));

//   return ret_msg;
// }

// GVariant* __fake_g_dbus_message_get_body(GDBusMessage* arg0) {
//   GVariant* ret_body = g_variant_new("(i)", 1);

//   return ret_body;
// }

// GDBusMessage* __fake_g_dbus_message_new_method_call(const gchar* arg0,
//     const gchar* arg1, const gchar* arg2, const gchar* arg3) {
//   GDBusMessage* ret_msg = static_cast<GDBusMessage*>(g_object_new(G_TYPE_OBJECT, NULL));

//   return ret_msg;
// }

// int __fake_rpc_port_proxy_noti_service_invoke_load_noti_grouping_list(
//     rpc_port_proxy_noti_service_h h, int type, int count, int count_per_page, int uid,
//     rpc_port_proxy_list_notification_h *noti_list) {
//   return RPC_PORT_ERROR_NONE;
// }

// int __fake_rpc_port_proxy_list_notification_foreach(
//     rpc_port_proxy_list_notification_h h,
//     rpc_port_proxy_list_notification_cb callback, void *user_data) {
//   return RPC_PORT_ERROR_NONE;
// }

class Mocks :
    virtual public ::testing::NiceMock<AulMock>,
    virtual public ::testing::NiceMock<PackageManagerMock>,
    virtual public ::testing::NiceMock<SocketMock>,
    virtual public ::testing::NiceMock<GlibMock> {};
} // namespace

using ::testing::_;
using ::testing::Invoke;
using ::testing::Return;

class NotificationListTest : public TestFixture {
 public:
  NotificationListTest() : TestFixture(std::make_unique<::Mocks>()) {}
  virtual void SetUp() {
  }
  virtual void TearDown() {
  }
};

TEST_F(NotificationListTest, AppendAndCount) {
  EXPECT_CALL(GetMock<AulMock>(), aul_app_get_appid_bypid(_, _, _))
      .WillRepeatedly(Invoke(__fake_aul_app_get_appid_bypid));

  notification_list_h noti_list = nullptr;
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  notification_h noti2 = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti2, nullptr);

  noti_list = notification_list_append(noti_list, noti);
  ASSERT_NE(noti_list, nullptr);

  noti_list = notification_list_append(noti_list, noti2);
  ASSERT_NE(noti_list, nullptr);

  int cnt = notification_list_get_count(noti_list);
  EXPECT_EQ(cnt, 2);

  int ret = notification_free_list(noti_list);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
}

TEST_F(NotificationListTest, GetHeadAndTail) {
  EXPECT_CALL(GetMock<AulMock>(), aul_app_get_appid_bypid(_, _, _))
      .WillRepeatedly(Invoke(__fake_aul_app_get_appid_bypid));

  notification_list_h noti_list = nullptr;
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);
  notification_h noti2 = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti2, nullptr);

  noti_list = notification_list_append(noti_list, noti);
  noti_list = notification_list_append(noti_list, noti2);
  ASSERT_NE(noti_list, nullptr);

  EXPECT_EQ(notification_list_get_head(nullptr), nullptr);
  EXPECT_EQ(notification_list_get_tail(nullptr), nullptr);

  notification_list_h head = notification_list_get_head(noti_list);
  EXPECT_NE(head, nullptr);

  notification_list_h tail = notification_list_get_tail(noti_list);
  EXPECT_NE(tail, nullptr);
  EXPECT_NE(head, tail);

  notification_free_list(head);
}

TEST_F(NotificationListTest, GetPrevAndNext) {
  EXPECT_CALL(GetMock<AulMock>(), aul_app_get_appid_bypid(_, _, _))
      .WillRepeatedly(Invoke(__fake_aul_app_get_appid_bypid));

  notification_list_h noti_list = nullptr;
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);
  notification_h noti2 = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti2, nullptr);

  noti_list = notification_list_append(noti_list, noti);
  noti_list = notification_list_append(noti_list, noti2);
  ASSERT_NE(noti_list, nullptr);

  EXPECT_EQ(notification_list_get_prev(nullptr), nullptr);
  EXPECT_EQ(notification_list_get_next(nullptr), nullptr);

  notification_list_h head = notification_list_get_head(noti_list);
  ASSERT_NE(head, nullptr);

  /* From head, get_next should return the second node */
  notification_list_h next = notification_list_get_next(head);
  EXPECT_NE(next, nullptr);

  /* From tail (noti_list), get_next should be null */
  notification_list_h tail_next = notification_list_get_next(noti_list);
  EXPECT_EQ(tail_next, nullptr);

  /* From tail, get_prev should return the first node */
  notification_list_h prev = notification_list_get_prev(noti_list);
  EXPECT_NE(prev, nullptr);

  notification_free_list(head);
}

TEST_F(NotificationListTest, GetData) {
  EXPECT_CALL(GetMock<AulMock>(), aul_app_get_appid_bypid(_, _, _))
      .WillRepeatedly(Invoke(__fake_aul_app_get_appid_bypid));

  EXPECT_EQ(notification_list_get_data(nullptr), nullptr);

  notification_list_h noti_list = nullptr;
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  noti_list = notification_list_append(noti_list, noti);
  ASSERT_NE(noti_list, nullptr);

  notification_h data = notification_list_get_data(noti_list);
  EXPECT_NE(data, nullptr);

  notification_free_list(noti_list);
}

TEST_F(NotificationListTest, GetCountNull) {
  EXPECT_EQ(notification_list_get_count(nullptr), 0);
}

TEST_F(NotificationListTest, RemoveFromList) {
  EXPECT_CALL(GetMock<AulMock>(), aul_app_get_appid_bypid(_, _, _))
      .WillRepeatedly(Invoke(__fake_aul_app_get_appid_bypid));

  notification_list_h noti_list = nullptr;
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);
  notification_h noti2 = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti2, nullptr);

  noti_list = notification_list_append(noti_list, noti);
  noti_list = notification_list_append(noti_list, noti2);

  noti_list = notification_list_remove(noti_list, noti2);
  EXPECT_NE(noti_list, nullptr);
  EXPECT_EQ(notification_list_get_count(noti_list), 1);

  int ret = notification_free_list(noti_list);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  notification_free(noti2);
}

TEST_F(NotificationListTest, GetListNullParam) {
  int ret = notification_get_list(NOTIFICATION_TYPE_NOTI, 3, nullptr);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationListTest, GetListByPageNullParam) {
  int ret = notification_get_list_by_page(NOTIFICATION_TYPE_NOTI, 1, 3, nullptr);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationListTest, GetDetailListNullAppId) {
  notification_list_h get_list_detail = nullptr;
  int ret = notification_get_detail_list(nullptr, 1, 1, 1, &get_list_detail);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}
