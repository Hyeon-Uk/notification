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
#include "mock/glib_mock.hh"
#include "mock/noti_service_proxy_mock.hh"

#include "notification/include/notification.h"
#include "notification/include/notification_internal.h"
#include "notification/include/notification_private.h"

namespace {

rpc_port_proxy_noti_service_h kFakeServiceHandle =
    reinterpret_cast<rpc_port_proxy_noti_service_h>(0x1);

int __fake_aul_app_get_appid_bypid(int pid, char* appid, int len) {
  snprintf(appid, len, "%s", "org.tizen.testappid");
  return AUL_R_OK;
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
    virtual public ::testing::NiceMock<GlibMock>,
    virtual public ::testing::NiceMock<NotiServiceProxyMock> {};

}  // namespace

using ::testing::_;
using ::testing::Return;
using ::testing::Invoke;

class NotificationChannelTest : public TestFixture {
 public:
  NotificationChannelTest() : TestFixture(std::make_unique<::Mocks>()) {}

  void SetUp() override {
    EXPECT_CALL(GetMock<AulMock>(), aul_app_get_appid_bypid(_, _, _))
        .WillRepeatedly(Invoke(__fake_aul_app_get_appid_bypid));
    EXPECT_CALL(GetMock<GlibMock>(), g_list_append(_, _))
        .WillRepeatedly(Invoke(__fake_g_list_append));
    EXPECT_CALL(GetMock<NotiServiceProxyMock>(),
        rpc_port_proxy_noti_service_create(_, _, _, _))
        .WillRepeatedly(Invoke(__fake_rpc_port_proxy_noti_service_create));
    EXPECT_CALL(GetMock<NotiServiceProxyMock>(),
        rpc_port_proxy_noti_service_connect_sync(_))
        .WillRepeatedly(Return(RPC_PORT_ERROR_NONE));
  }
};

TEST_F(NotificationChannelTest, SetChannelNameFailsWithNullArgs) {
  EXPECT_EQ(notification_set_channel_name(nullptr, "ch"),
      NOTIFICATION_ERROR_INVALID_PARAMETER);

  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);
  EXPECT_EQ(notification_set_channel_name(noti, nullptr),
      NOTIFICATION_ERROR_INVALID_PARAMETER);
  notification_free(noti);
}

TEST_F(NotificationChannelTest, SetChannelNameOverwritesPrevious) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  ASSERT_EQ(notification_set_channel_name(noti, "first"),
      NOTIFICATION_ERROR_NONE);
  ASSERT_EQ(notification_set_channel_name(noti, "second"),
      NOTIFICATION_ERROR_NONE);

  const char *name = nullptr;
  ASSERT_EQ(notification_get_channel_name(noti, &name),
      NOTIFICATION_ERROR_NONE);
  EXPECT_STREQ(name, "second");

  notification_free(noti);
}

TEST_F(NotificationChannelTest, GetChannelNameFailsWithNullArgs) {
  EXPECT_EQ(notification_get_channel_name(nullptr, nullptr),
      NOTIFICATION_ERROR_INVALID_PARAMETER);

  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);
  EXPECT_EQ(notification_get_channel_name(noti, nullptr),
      NOTIFICATION_ERROR_INVALID_PARAMETER);
  notification_free(noti);
}

TEST_F(NotificationChannelTest, CreateWithInfoFailsWithNullArgs) {
  notification_channel_h channel = nullptr;
  EXPECT_EQ(notification_channel_create_with_info(nullptr, "ch", 1, 1,
      &channel), NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_channel_create_with_info("app", nullptr, 1, 1,
      &channel), NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationChannelTest, CreateWithInfoSetsBlockableAndBlocked) {
  notification_channel_h channel = nullptr;
  ASSERT_EQ(notification_channel_create_with_info("app", "ch", 1, 1,
      &channel), NOTIFICATION_ERROR_NONE);
  bool blockable = false;
  bool blocked = false;
  ASSERT_EQ(notification_channel_get_blockable(channel, &blockable),
      NOTIFICATION_ERROR_NONE);
  ASSERT_EQ(notification_channel_get_block(channel, &blocked),
      NOTIFICATION_ERROR_NONE);
  EXPECT_TRUE(blockable);
  EXPECT_TRUE(blocked);
  notification_channel_free(channel);

  ASSERT_EQ(notification_channel_create_with_info("app", "ch", 0, 0,
      &channel), NOTIFICATION_ERROR_NONE);
  ASSERT_EQ(notification_channel_get_blockable(channel, &blockable),
      NOTIFICATION_ERROR_NONE);
  ASSERT_EQ(notification_channel_get_block(channel, &blocked),
      NOTIFICATION_ERROR_NONE);
  EXPECT_FALSE(blockable);
  EXPECT_FALSE(blocked);
  notification_channel_free(channel);
}

TEST_F(NotificationChannelTest, CreateFailsWithNullArgs) {
  notification_channel_h channel = nullptr;
  EXPECT_EQ(notification_channel_create(nullptr, &channel),
      NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_channel_create("ch", nullptr),
      NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationChannelTest, CreateSucceeds) {
  notification_channel_h channel = nullptr;
  ASSERT_EQ(notification_channel_create("ch", &channel),
      NOTIFICATION_ERROR_NONE);
  const char *name = nullptr;
  ASSERT_EQ(notification_channel_get_name(channel, &name),
      NOTIFICATION_ERROR_NONE);
  EXPECT_STREQ(name, "ch");
  notification_channel_free(channel);
}

TEST_F(NotificationChannelTest, FreeIgnoresNull) {
  notification_channel_free(nullptr);
}

TEST_F(NotificationChannelTest, FreeReleasesChannel) {
  notification_channel_h channel = nullptr;
  ASSERT_EQ(notification_channel_create("ch", &channel),
      NOTIFICATION_ERROR_NONE);
  notification_channel_free(channel);
}

TEST_F(NotificationChannelTest, AddFailsWithNullChannel) {
  EXPECT_EQ(notification_channel_add(nullptr),
      NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationChannelTest, AddSucceeds) {
  EXPECT_CALL(GetMock<NotiServiceProxyMock>(),
      rpc_port_proxy_noti_service_invoke_channel_insert(_, _, _, _, _, _))
      .WillOnce(Return(RPC_PORT_ERROR_NONE));

  notification_channel_h channel = nullptr;
  ASSERT_EQ(notification_channel_create("ch", &channel),
      NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(notification_channel_add(channel), NOTIFICATION_ERROR_NONE);
  notification_channel_free(channel);
}

TEST_F(NotificationChannelTest, AddFailsWhenServiceInvokeFails) {
  EXPECT_CALL(GetMock<NotiServiceProxyMock>(),
      rpc_port_proxy_noti_service_invoke_channel_insert(_, _, _, _, _, _))
      .WillOnce(Return(RPC_PORT_ERROR_IO_ERROR));

  notification_channel_h channel = nullptr;
  ASSERT_EQ(notification_channel_create("ch", &channel),
      NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(notification_channel_add(channel), RPC_PORT_ERROR_IO_ERROR);
  notification_channel_free(channel);
}

TEST_F(NotificationChannelTest, RemoveFailsWithNullChannel) {
  EXPECT_EQ(notification_channel_remove(nullptr),
      NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationChannelTest, RemoveSucceeds) {
  EXPECT_CALL(GetMock<NotiServiceProxyMock>(),
      rpc_port_proxy_noti_service_invoke_channel_delete(_, _, _, _))
      .WillOnce(Return(RPC_PORT_ERROR_NONE));

  notification_channel_h channel = nullptr;
  ASSERT_EQ(notification_channel_create("ch", &channel),
      NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(notification_channel_remove(channel), NOTIFICATION_ERROR_NONE);
  notification_channel_free(channel);
}

TEST_F(NotificationChannelTest, RemoveFailsWhenServiceInvokeFails) {
  EXPECT_CALL(GetMock<NotiServiceProxyMock>(),
      rpc_port_proxy_noti_service_invoke_channel_delete(_, _, _, _))
      .WillOnce(Return(RPC_PORT_ERROR_IO_ERROR));

  notification_channel_h channel = nullptr;
  ASSERT_EQ(notification_channel_create("ch", &channel),
      NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(notification_channel_remove(channel), RPC_PORT_ERROR_IO_ERROR);
  notification_channel_free(channel);
}

TEST_F(NotificationChannelTest, UpdateFailsWithNullChannel) {
  EXPECT_EQ(notification_channel_update(nullptr),
      NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationChannelTest, UpdateSucceeds) {
  EXPECT_CALL(GetMock<NotiServiceProxyMock>(),
      rpc_port_proxy_noti_service_invoke_channel_update(_, _, _, _, _, _))
      .WillOnce(Return(RPC_PORT_ERROR_NONE));

  notification_channel_h channel = nullptr;
  ASSERT_EQ(notification_channel_create("ch", &channel),
      NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(notification_channel_update(channel), NOTIFICATION_ERROR_NONE);
  notification_channel_free(channel);
}

TEST_F(NotificationChannelTest, UpdateFailsWhenServiceInvokeFails) {
  EXPECT_CALL(GetMock<NotiServiceProxyMock>(),
      rpc_port_proxy_noti_service_invoke_channel_update(_, _, _, _, _, _))
      .WillOnce(Return(RPC_PORT_ERROR_IO_ERROR));

  notification_channel_h channel = nullptr;
  ASSERT_EQ(notification_channel_create("ch", &channel),
      NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(notification_channel_update(channel), RPC_PORT_ERROR_IO_ERROR);
  notification_channel_free(channel);
}

TEST_F(NotificationChannelTest, GetByNameFailsWithNullArgs) {
  notification_channel_h channel = nullptr;
  EXPECT_EQ(notification_channel_get_by_name(nullptr, &channel),
      NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_channel_get_by_name("ch", nullptr),
      NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationChannelTest, GetByNameSucceeds) {
  EXPECT_CALL(GetMock<NotiServiceProxyMock>(),
      rpc_port_proxy_noti_service_invoke_channel_get(_, _, _, _, _, _))
      .WillOnce(Invoke([](rpc_port_proxy_noti_service_h, const char*,
          const char*, bool *blockable, bool *is_blocked, int) {
        *blockable = true;
        *is_blocked = false;
        return static_cast<int>(RPC_PORT_ERROR_NONE);
      }));

  notification_channel_h channel = nullptr;
  ASSERT_EQ(notification_channel_get_by_name("ch", &channel),
      NOTIFICATION_ERROR_NONE);
  bool blockable = false;
  ASSERT_EQ(notification_channel_get_blockable(channel, &blockable),
      NOTIFICATION_ERROR_NONE);
  EXPECT_TRUE(blockable);
  notification_channel_free(channel);
}

TEST_F(NotificationChannelTest, GetByNameFailsWhenServiceInvokeFails) {
  EXPECT_CALL(GetMock<NotiServiceProxyMock>(),
      rpc_port_proxy_noti_service_invoke_channel_get(_, _, _, _, _, _))
      .WillOnce(Return(RPC_PORT_ERROR_IO_ERROR));

  notification_channel_h channel = nullptr;
  EXPECT_EQ(notification_channel_get_by_name("ch", &channel),
      RPC_PORT_ERROR_IO_ERROR);
}

TEST_F(NotificationChannelTest, SetGetBlockable) {
  notification_channel_h channel = nullptr;
  ASSERT_EQ(notification_channel_create("ch", &channel),
      NOTIFICATION_ERROR_NONE);

  EXPECT_EQ(notification_channel_set_blockable(nullptr, true),
      NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_channel_get_blockable(nullptr, nullptr),
      NOTIFICATION_ERROR_INVALID_PARAMETER);

  ASSERT_EQ(notification_channel_set_blockable(channel, true),
      NOTIFICATION_ERROR_NONE);
  bool blockable = false;
  ASSERT_EQ(notification_channel_get_blockable(channel, &blockable),
      NOTIFICATION_ERROR_NONE);
  EXPECT_TRUE(blockable);
  notification_channel_free(channel);
}

TEST_F(NotificationChannelTest, SetGetBlock) {
  notification_channel_h channel = nullptr;
  ASSERT_EQ(notification_channel_create("ch", &channel),
      NOTIFICATION_ERROR_NONE);

  EXPECT_EQ(notification_channel_set_block(nullptr, true),
      NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_channel_get_block(nullptr, nullptr),
      NOTIFICATION_ERROR_INVALID_PARAMETER);

  ASSERT_EQ(notification_channel_set_block(channel, true),
      NOTIFICATION_ERROR_NONE);
  bool blocked = false;
  ASSERT_EQ(notification_channel_get_block(channel, &blocked),
      NOTIFICATION_ERROR_NONE);
  EXPECT_TRUE(blocked);
  notification_channel_free(channel);
}

TEST_F(NotificationChannelTest, GetNameFailsWithNullArgs) {
  notification_channel_h channel = nullptr;
  ASSERT_EQ(notification_channel_create("ch", &channel),
      NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(notification_channel_get_name(nullptr, nullptr),
      NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_channel_get_name(channel, nullptr),
      NOTIFICATION_ERROR_INVALID_PARAMETER);
  notification_channel_free(channel);
}

TEST_F(NotificationChannelTest, CloneFailsWithNullArgs) {
  notification_channel_h channel = nullptr;
  ASSERT_EQ(notification_channel_create("ch", &channel),
      NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(notification_channel_clone(nullptr, &channel),
      NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_channel_clone(channel, nullptr),
      NOTIFICATION_ERROR_INVALID_PARAMETER);
  notification_channel_free(channel);
}

TEST_F(NotificationChannelTest, CloneCopiesFields) {
  notification_channel_h channel = nullptr;
  ASSERT_EQ(notification_channel_create_with_info("app", "ch", 1, 1,
      &channel), NOTIFICATION_ERROR_NONE);

  notification_channel_h clone = nullptr;
  ASSERT_EQ(notification_channel_clone(channel, &clone),
      NOTIFICATION_ERROR_NONE);

  const char *name = nullptr;
  ASSERT_EQ(notification_channel_get_name(clone, &name),
      NOTIFICATION_ERROR_NONE);
  EXPECT_STREQ(name, "ch");

  notification_channel_free(channel);
  notification_channel_free(clone);
}

TEST_F(NotificationChannelTest, ForeachFailsWithNullCallback) {
  EXPECT_EQ(notification_channel_foreach("app", nullptr, nullptr),
      NOTIFICATION_ERROR_INVALID_PARAMETER);
}

void __fake_channel_foreach_cb(notification_channel_h channel, void *user_data) {
}

rpc_port_proxy_list_noti_channel_h __fake_make_channel_list() {
  rpc_port_proxy_noti_channel_h channel = nullptr;
  rpc_port_proxy_noti_channel_create(&channel);
  rpc_port_proxy_noti_channel_set_app_id(channel, "org.tizen.testappid");
  rpc_port_proxy_noti_channel_set_channel_name(channel, "ch");
  rpc_port_proxy_noti_channel_set_blockable(channel, true);
  rpc_port_proxy_noti_channel_set_is_blocked(channel, false);

  rpc_port_proxy_list_noti_channel_h list = nullptr;
  rpc_port_proxy_list_noti_channel_create(&list);
  rpc_port_proxy_list_noti_channel_add(list, channel);
  rpc_port_proxy_noti_channel_destroy(channel);
  return list;
}

TEST_F(NotificationChannelTest, ForeachSucceedsWithExplicitAppId) {
  EXPECT_CALL(GetMock<NotiServiceProxyMock>(),
      rpc_port_proxy_noti_service_invoke_channel_get_list(_, _, _, _))
      .WillOnce(Invoke([](rpc_port_proxy_noti_service_h, const char*, int,
          rpc_port_proxy_list_noti_channel_h *channel_list) {
        *channel_list = __fake_make_channel_list();
        return static_cast<int>(RPC_PORT_ERROR_NONE);
      }));

  EXPECT_EQ(notification_channel_foreach("org.tizen.testappid",
      __fake_channel_foreach_cb, nullptr), NOTIFICATION_ERROR_NONE);
}

TEST_F(NotificationChannelTest, ForeachUsesCallerAppIdWhenAppIdIsNull) {
  EXPECT_CALL(GetMock<NotiServiceProxyMock>(),
      rpc_port_proxy_noti_service_invoke_channel_get_list(_, _, _, _))
      .WillOnce(Invoke([](rpc_port_proxy_noti_service_h, const char*, int,
          rpc_port_proxy_list_noti_channel_h *channel_list) {
        *channel_list = __fake_make_channel_list();
        return static_cast<int>(RPC_PORT_ERROR_NONE);
      }));

  EXPECT_EQ(notification_channel_foreach(nullptr, __fake_channel_foreach_cb,
      nullptr), NOTIFICATION_ERROR_NONE);
}

TEST_F(NotificationChannelTest, ForeachFailsWhenServiceInvokeFails) {
  EXPECT_CALL(GetMock<NotiServiceProxyMock>(),
      rpc_port_proxy_noti_service_invoke_channel_get_list(_, _, _, _))
      .WillOnce(Return(RPC_PORT_ERROR_IO_ERROR));

  EXPECT_EQ(notification_channel_foreach("org.tizen.testappid",
      __fake_channel_foreach_cb, nullptr), RPC_PORT_ERROR_IO_ERROR);
}
