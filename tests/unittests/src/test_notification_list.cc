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

TEST_F(NotificationListTest, notification_list_test) {
  EXPECT_CALL(GetMock<AulMock>(), aul_app_get_appid_bypid(_, _, _))
      .WillRepeatedly(Invoke(__fake_aul_app_get_appid_bypid));

  // EXPECT_CALL(GetMock<GlibMock>(), g_bus_get_sync(_, _, _))
  //     .WillRepeatedly(Invoke(__fake_g_bus_get_sync));

  // EXPECT_CALL(GetMock<GlibMock>(), g_dbus_connection_send_message_with_reply_sync(_, _, _, _, _, _, _))
  //     .WillRepeatedly(Invoke(__fake_g_dbus_connection_send_message_with_reply_sync));

  // EXPECT_CALL(GetMock<GlibMock>(), g_dbus_message_get_body(_))
  //     .WillRepeatedly(Invoke(__fake_g_dbus_message_get_body));

  // EXPECT_CALL(GetMock<GlibMock>(), read(_, _, _))
  //     .WillRepeatedly(Return(1));

  // EXPECT_CALL(GetMock<SocketMock>(), getsockopt(_, _, _, _, _))
  //     .WillRepeatedly(Return(0));

  // EXPECT_CALL(GetMock<GlibMock>(), g_dbus_message_new_method_call(_, _, _, _))
  //     .WillRepeatedly(Invoke(__fake_g_dbus_message_new_method_call));

  int ret;
  notification_list_h noti_list = nullptr;
  notification_h noti = nullptr;
  notification_h noti2 = nullptr;

  noti = notification_create(NOTIFICATION_TYPE_NOTI);
  EXPECT_NE(noti, nullptr);

  noti2 = notification_create(NOTIFICATION_TYPE_NOTI);
  EXPECT_NE(noti2, nullptr);

  noti_list = notification_list_append(noti_list, noti);
  EXPECT_NE(noti_list, nullptr);

  noti_list = notification_list_append(noti_list, noti2);
  EXPECT_NE(noti_list, nullptr);

  ret = notification_get_list(NOTIFICATION_TYPE_NOTI, 3, nullptr);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);

  ret = notification_get_list_by_page(NOTIFICATION_TYPE_NOTI, 1, 3, nullptr);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);

  notification_list_h list_head = nullptr;
  list_head = notification_list_get_head(nullptr);
  EXPECT_EQ(list_head, nullptr);

  list_head = notification_list_get_head(noti_list);
  EXPECT_NE(list_head, nullptr);

  notification_list_h list_tail = nullptr;
  list_tail = notification_list_get_tail(nullptr);
  EXPECT_EQ(list_tail, nullptr);

  list_tail = notification_list_get_tail(noti_list);
  EXPECT_NE(list_tail, nullptr);

  notification_list_h list_prev = nullptr;
  list_prev = notification_list_get_prev(nullptr);
  EXPECT_EQ(list_prev, nullptr);

  list_prev = notification_list_get_prev(noti_list);
  EXPECT_NE(list_prev, nullptr);

  notification_list_h list_next = nullptr;
  list_next = notification_list_get_next(nullptr);
  EXPECT_EQ(list_next, nullptr);

  list_next = notification_list_get_next(noti_list);
  EXPECT_NE(list_prev, nullptr);

  notification_h noti_data = nullptr;
  noti_data = notification_list_get_data(nullptr);
  EXPECT_EQ(noti_data, nullptr);

  noti_data = notification_list_get_data(noti_list);
  EXPECT_NE(noti_data, nullptr);

  int cnt = 0;
  cnt = notification_list_get_count(nullptr);
  EXPECT_EQ(cnt, 0);

  cnt = notification_list_get_count(noti_list);
  ASSERT_EQ(cnt, 2);

  noti_list = notification_list_remove(noti_list, noti2);
  EXPECT_NE(noti_list, nullptr);

  ret = notification_free_list(noti_list);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);
}

TEST_F(NotificationListTest, notification_list_test2) {
  int ret;

  notification_list_h get_list_detail = nullptr;
  ret = notification_get_detail_list(nullptr, 1, 1, 1, &get_list_detail);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationListTest, get_list_by_page_zero_page_returns_invalid) {
  notification_list_h list = nullptr;
  EXPECT_EQ(notification_get_list_by_page(NOTIFICATION_TYPE_NOTI,
                0, 10, &list),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationListTest, get_list_by_page_negative_page_returns_invalid) {
  notification_list_h list = nullptr;
  EXPECT_EQ(notification_get_list_by_page(NOTIFICATION_TYPE_NOTI,
                -1, 10, &list),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationListTest, get_detail_list_null_list_returns_invalid) {
  EXPECT_EQ(notification_get_detail_list("appid", 1, 1, 1, nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationListTest, free_list_null_returns_invalid) {
  EXPECT_EQ(notification_free_list(nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* ------------------------------------------------------------------ */
/* notification_list_remove — middle element                           */
/* ------------------------------------------------------------------ */

TEST_F(NotificationListTest, remove_middle_element) {
  EXPECT_CALL(GetMock<AulMock>(), aul_app_get_appid_bypid(_, _, _))
      .WillRepeatedly(Invoke(__fake_aul_app_get_appid_bypid));

  notification_h a = notification_create(NOTIFICATION_TYPE_NOTI);
  notification_h b = notification_create(NOTIFICATION_TYPE_NOTI);
  notification_h c = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(a, nullptr);
  ASSERT_NE(b, nullptr);
  ASSERT_NE(c, nullptr);

  notification_list_h list = nullptr;
  list = notification_list_append(list, a);
  list = notification_list_get_head(list);
  notification_list_h tmp = notification_list_append(list, b);
  (void)tmp;
  tmp = notification_list_append(list, c);
  (void)tmp;

  /* Rebuild the head pointer. */
  list = notification_list_get_head(list);
  ASSERT_EQ(notification_list_get_count(list), 3);

  /* Remove the middle element B. */
  list = notification_list_remove(list, b);
  ASSERT_NE(list, nullptr);
  EXPECT_EQ(notification_list_get_count(list), 2);

  /* The head should be A and the next should be C. */
  notification_list_h head = notification_list_get_head(list);
  ASSERT_NE(head, nullptr);
  EXPECT_EQ(notification_list_get_data(head), a);

  notification_list_h next = notification_list_get_next(head);
  ASSERT_NE(next, nullptr);
  EXPECT_EQ(notification_list_get_data(next), c);

  /* B is now a dangling pointer — free it separately. */
  notification_free(b);
  notification_free_list(list);
}

/* ------------------------------------------------------------------ */
/* notification_list_remove — head element                             */
/* ------------------------------------------------------------------ */

TEST_F(NotificationListTest, remove_head_element) {
  EXPECT_CALL(GetMock<AulMock>(), aul_app_get_appid_bypid(_, _, _))
      .WillRepeatedly(Invoke(__fake_aul_app_get_appid_bypid));

  notification_h a = notification_create(NOTIFICATION_TYPE_NOTI);
  notification_h b = notification_create(NOTIFICATION_TYPE_NOTI);
  notification_h c = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(a, nullptr);
  ASSERT_NE(b, nullptr);
  ASSERT_NE(c, nullptr);

  notification_list_h list = nullptr;
  list = notification_list_append(list, a);
  list = notification_list_get_head(list);
  notification_list_append(list, b);
  notification_list_append(list, c);
  list = notification_list_get_head(list);
  ASSERT_EQ(notification_list_get_count(list), 3);

  /* Remove head element A. */
  list = notification_list_remove(list, a);
  ASSERT_NE(list, nullptr);
  EXPECT_EQ(notification_list_get_count(list), 2);

  /* New head must be B and it must have no previous pointer. */
  notification_list_h head = notification_list_get_head(list);
  ASSERT_NE(head, nullptr);
  EXPECT_EQ(notification_list_get_data(head), b);
  EXPECT_EQ(notification_list_get_prev(head), nullptr);

  notification_free(a);
  notification_free_list(list);
}

/* ------------------------------------------------------------------ */
/* notification_list_append — NULL noti argument                       */
/* ------------------------------------------------------------------ */

TEST_F(NotificationListTest, append_null_noti) {
  notification_list_h result = notification_list_append(nullptr, nullptr);
  EXPECT_EQ(result, nullptr);
  EXPECT_EQ(get_last_result(), NOTIFICATION_ERROR_INVALID_PARAMETER);
}
