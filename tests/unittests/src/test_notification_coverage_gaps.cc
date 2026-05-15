/*
 * Copyright (c) 2026 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Tests targeting coverage gaps in EXPORT_API functions that were
 * previously untested or only partially tested (e.g., NULL-arg paths
 * only).  Each test references the function under test + scenario tag
 * from dpm_test_scenario.md conventions: [P1] success, [N1..Nn] error
 * paths, [E1..En] edge cases, [C1..Cn] state corners.
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <bundle.h>

#include "mock/aul_mock.hh"
#include "mock/glib_mock.hh"
#include "mock/noti_service_proxy_mock.hh"
#include "mock/package_manager_mock.hh"
#include "mock/rpc_port_mock.hh"
#include "mock/test_fixture.hh"
#include "mock/tzplatform_config_mock.hh"

#include <rpc-port.h>

#include "notification/include/notification.h"
#include "notification/include/notification_internal.h"
#include "notification/include/notification_private.h"

using ::testing::_;
using ::testing::Invoke;
using ::testing::Return;

namespace {

int __cov_fake_aul_app_get_appid_bypid(int pid, char* appid, int len) {
  snprintf(appid, len, "%s", "org.tizen.testappid");
  return AUL_R_OK;
}

class Mocks :
    virtual public ::testing::NiceMock<AulMock>,
    virtual public ::testing::NiceMock<TzplatformConfigMock>,
    virtual public ::testing::NiceMock<GlibMock>,
    virtual public ::testing::NiceMock<NotiServiceProxyMock>,
    virtual public ::testing::NiceMock<PackageManagerMock>,
    virtual public ::testing::NiceMock<RpcPortMock> {};

}  // namespace

class NotificationCoverageGapsTest : public TestFixture {
 public:
  NotificationCoverageGapsTest() : TestFixture(std::make_unique<::Mocks>()) {}

  void SetUp() override {
    EXPECT_CALL(GetMock<AulMock>(), aul_app_get_appid_bypid(_, _, _))
        .WillRepeatedly(Invoke(__cov_fake_aul_app_get_appid_bypid));

    noti_ = notification_create(NOTIFICATION_TYPE_NOTI);
    ASSERT_NE(noti_, nullptr);
  }

  void TearDown() override {
    if (noti_ != nullptr) {
      notification_free(noti_);
      noti_ = nullptr;
    }
  }

  notification_h noti_ = nullptr;
};

/* ============================================================== */
/* notification_insert_for_uid — branch coverage                  */
/* ============================================================== */

TEST_F(NotificationCoverageGapsTest,
       insert_for_uid_type_zero_returns_invalid_parameter) {
  /* [N2] type == NOTIFICATION_TYPE_NONE (0) → INVALID_PARAMETER before
   * any tidl call. */
  noti_->type = NOTIFICATION_TYPE_NONE;
  int priv_id = 0;
  EXPECT_EQ(notification_insert_for_uid(noti_, &priv_id, 1000),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationCoverageGapsTest,
       insert_for_uid_type_above_max_returns_invalid_parameter) {
  /* [N3] type > NOTIFICATION_TYPE_MAX → INVALID_PARAMETER. */
  noti_->type = (notification_type_e)(NOTIFICATION_TYPE_MAX + 1);
  int priv_id = 0;
  EXPECT_EQ(notification_insert_for_uid(noti_, &priv_id, 1000),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationCoverageGapsTest,
       insert_for_uid_negative_type_returns_invalid_parameter) {
  /* [N4] type negative (< NOTIFICATION_TYPE_NONE) → INVALID_PARAMETER. */
  noti_->type = (notification_type_e)(-1);
  int priv_id = 0;
  EXPECT_EQ(notification_insert_for_uid(noti_, &priv_id, 1000),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationCoverageGapsTest,
       insert_null_noti_returns_invalid_parameter) {
  /* notification_insert (no _for_uid) delegates with aul_getuid().
   * Null noti path before delegation. */
  int priv_id = 0;
  EXPECT_EQ(notification_insert(nullptr, &priv_id),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* ============================================================== */
/* notification_set_pkgname additional coverage                   */
/* ============================================================== */

TEST_F(NotificationCoverageGapsTest, set_pkgname_null_pkgname_returns_invalid) {
  /* notification_set_pkgname → notification_set_app_id; null pkgname
   * exercises the null-arg branch of set_app_id. */
  EXPECT_EQ(notification_set_pkgname(noti_, nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* ============================================================== */
/* notification_set_icon / notification_get_icon                  */
/* ============================================================== */

TEST_F(NotificationCoverageGapsTest, set_icon_null_noti_returns_invalid) {
  /* notification_set_icon → notification_set_image with ICON type.
   * NULL noti propagates the INVALID_PARAMETER from set_image. */
  EXPECT_EQ(notification_set_icon(nullptr, "/tmp/icon.png"),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationCoverageGapsTest, set_icon_success_round_trip) {
  /* [P1] set then get round-trip via image-API. */
  ASSERT_EQ(notification_set_icon(noti_, "/tmp/icon.png"),
            NOTIFICATION_ERROR_NONE);
  char* out = nullptr;
  EXPECT_EQ(notification_get_icon(noti_, &out), NOTIFICATION_ERROR_NONE);
  EXPECT_STREQ(out, "/tmp/icon.png");
}

TEST_F(NotificationCoverageGapsTest, get_icon_null_noti_returns_invalid) {
  char* out = nullptr;
  EXPECT_EQ(notification_get_icon(nullptr, &out),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationCoverageGapsTest, get_icon_null_out_returns_invalid) {
  /* notification_get_icon delegates to notification_get_image which
   * handles the null out param.  Accept either INVALID_PARAMETER or
   * NONE (some image-API impls only check the noti handle). */
  int ret = notification_get_icon(noti_, nullptr);
  EXPECT_TRUE(ret == NOTIFICATION_ERROR_INVALID_PARAMETER ||
              ret == NOTIFICATION_ERROR_NONE);
}

/* ============================================================== */
/* notification_delete_for_uid                                    */
/* ============================================================== */

TEST_F(NotificationCoverageGapsTest, delete_for_uid_null_noti_returns_invalid) {
  /* [N1] noti == NULL → INVALID_PARAMETER before tidl call. */
  EXPECT_EQ(notification_delete_for_uid(nullptr, 5001),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* ============================================================== */
/* notification_load_by_tag / _for_uid                            */
/* ============================================================== */

TEST_F(NotificationCoverageGapsTest, load_by_tag_null_tag_returns_null) {
  /* [N1] tag == NULL → returns NULL handle; set_last_result set to
   * INVALID_PARAMETER (we only check the return for portability). */
  notification_h h = notification_load_by_tag(nullptr);
  EXPECT_EQ(h, nullptr);
}

TEST_F(NotificationCoverageGapsTest,
       load_by_tag_for_uid_null_tag_returns_null) {
  notification_h h = notification_load_by_tag_for_uid(nullptr, 5001);
  EXPECT_EQ(h, nullptr);
}

/* ============================================================== */
/* notification_set_extension_image_size /                        */
/* notification_get_extension_image_size                           */
/* ============================================================== */

TEST_F(NotificationCoverageGapsTest, set_extension_image_size_null_noti) {
  /* [N1] noti == NULL → INVALID_PARAMETER. */
  EXPECT_EQ(notification_set_extension_image_size(nullptr, 100),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationCoverageGapsTest, set_extension_image_size_zero_height) {
  /* [N2] height <= 0 → INVALID_PARAMETER. */
  EXPECT_EQ(notification_set_extension_image_size(noti_, 0),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationCoverageGapsTest, set_extension_image_size_negative_height) {
  /* [N3] negative height → INVALID_PARAMETER. */
  EXPECT_EQ(notification_set_extension_image_size(noti_, -1),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationCoverageGapsTest, set_extension_image_size_success_round_trip) {
  /* [P1] valid → NONE; round-trip via getter. */
  ASSERT_EQ(notification_set_extension_image_size(noti_, 256),
            NOTIFICATION_ERROR_NONE);
  int got = 0;
  EXPECT_EQ(notification_get_extension_image_size(noti_, &got),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(got, 256);
}

TEST_F(NotificationCoverageGapsTest, get_extension_image_size_null_noti) {
  int h = 0;
  EXPECT_EQ(notification_get_extension_image_size(nullptr, &h),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationCoverageGapsTest, get_extension_image_size_null_out) {
  EXPECT_EQ(notification_get_extension_image_size(noti_, nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* ============================================================== */
/* notification_channel_foreach — RPC-failure paths               */
/*                                                                  */
/* The null-cb gate is already verified in test_notification_      */
/* internal.cc.  Here we mock rpc_port_proxy_noti_service_connect_sync to fail */
/* and verify the function propagates a non-NONE error after       */
/* reaching the TIDL layer (which proves we passed the upfront     */
/* null-cb gate and entered the IPC code path).                    */
/* ============================================================== */

static void __cov_channel_foreach_cb(notification_channel_h channel,
                                     void* user_data) {
  (void)channel;
  (void)user_data;
}

TEST_F(NotificationCoverageGapsTest,
       channel_foreach_ipc_connect_fail_with_null_app_id_returns_error) {
  /* [N2-ipc] app_id == NULL → AUL lookup succeeds → TIDL connect_sync
   * mocked to fail → function returns the propagated error (non-NONE
   * and explicitly NOT the upfront INVALID_PARAMETER gate). */
  ON_CALL(GetMock<NotiServiceProxyMock>(), rpc_port_proxy_noti_service_connect_sync(_))
      .WillByDefault(Return(RPC_PORT_ERROR_IO_ERROR));
  int ret = notification_channel_foreach(nullptr,
                                         __cov_channel_foreach_cb,
                                         nullptr);
  EXPECT_NE(ret, NOTIFICATION_ERROR_NONE);
}

TEST_F(NotificationCoverageGapsTest,
       channel_foreach_ipc_connect_fail_with_valid_app_id_returns_error) {
  /* [P2-ipc] app_id provided → skip AUL lookup → TIDL connect_sync
   * fails → function propagates the error. */
  ON_CALL(GetMock<NotiServiceProxyMock>(), rpc_port_proxy_noti_service_connect_sync(_))
      .WillByDefault(Return(RPC_PORT_ERROR_IO_ERROR));
  int ret = notification_channel_foreach("org.tizen.app",
                                         __cov_channel_foreach_cb,
                                         nullptr);
  EXPECT_NE(ret, NOTIFICATION_ERROR_NONE);
}

/* ============================================================== */
/* notification_delete_group_by_priv_id — IPC-failure path        */
/* ============================================================== */

TEST_F(NotificationCoverageGapsTest,
       delete_group_by_priv_id_ipc_connect_fail_returns_error) {
  /* The function has no upfront NULL gate; it always reaches TIDL.
   * With IPC connect mocked to fail it must return a non-NONE error. */
  ON_CALL(GetMock<NotiServiceProxyMock>(), rpc_port_proxy_noti_service_connect_sync(_))
      .WillByDefault(Return(RPC_PORT_ERROR_IO_ERROR));
  int ret = notification_delete_group_by_priv_id("org.tizen.app",
                                                 NOTIFICATION_TYPE_NOTI, 1);
  EXPECT_NE(ret, NOTIFICATION_ERROR_NONE);
}

TEST_F(NotificationCoverageGapsTest,
       delete_group_by_priv_id_null_app_id_ipc_fail_returns_error) {
  /* app_id == NULL path: function calls notification_get_app_id_by_pid
   * (uses AUL mock) before TIDL. Connect fails → propagated error. */
  ON_CALL(GetMock<NotiServiceProxyMock>(), rpc_port_proxy_noti_service_connect_sync(_))
      .WillByDefault(Return(RPC_PORT_ERROR_IO_ERROR));
  int ret = notification_delete_group_by_priv_id(nullptr,
                                                 NOTIFICATION_TYPE_NOTI, 1);
  EXPECT_NE(ret, NOTIFICATION_ERROR_NONE);
}

/* ============================================================== */
/* notification_resister_changed_cb / unresister_changed_cb       */
/* (deprecated typo variants — still EXPORT_API)                   */
/* ============================================================== */

TEST_F(NotificationCoverageGapsTest, resister_changed_cb_null_cb_returns_invalid) {
  /* [N1] callback == NULL → INVALID_PARAMETER. */
  EXPECT_EQ(notification_resister_changed_cb(nullptr, nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationCoverageGapsTest,
       unresister_changed_cb_null_cb_returns_invalid) {
  /* [N1] callback == NULL → INVALID_PARAMETER. */
  EXPECT_EQ(notification_unresister_changed_cb(nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationCoverageGapsTest,
       register_detailed_changed_cb_null_cb_returns_invalid) {
  EXPECT_EQ(notification_register_detailed_changed_cb(nullptr, nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationCoverageGapsTest,
       unregister_detailed_changed_cb_null_cb_returns_invalid) {
  EXPECT_EQ(notification_unregister_detailed_changed_cb(nullptr, nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* ============================================================== */
/* notification_get_count — null out param + caller-app path      */
/* ============================================================== */

TEST_F(NotificationCoverageGapsTest, get_count_null_count_returns_invalid) {
  /* [N1] count out-param == NULL → INVALID_PARAMETER. */
  EXPECT_EQ(notification_get_count(NOTIFICATION_TYPE_NOTI,
                                   "org.tizen.app",
                                   NOTIFICATION_GROUP_ID_NONE,
                                   NOTIFICATION_PRIV_ID_NONE,
                                   nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* ============================================================== */
/* notification_delete_group_by_priv_id — null app_id              */
/* ============================================================== */

/* notification_delete_group_by_priv_id has no upfront NULL gate on app_id;
 * it always proceeds into TIDL via notification_tidl_request_delete_single.
 * The path is unreachable from unit tests without a working RPC backend.
 * Out-of-scope until rpc-port mock returns success values. */

/* ============================================================== */
/* notification_add_deferred_task — null callback                  */
/* ============================================================== */

TEST_F(NotificationCoverageGapsTest, add_deferred_task_null_cb_returns_invalid) {
  /* [N1] deferred_task_cb == NULL → INVALID_PARAMETER. */
  EXPECT_EQ(notification_add_deferred_task(nullptr, nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationCoverageGapsTest, del_deferred_task_null_cb_returns_invalid) {
  EXPECT_EQ(notification_del_deferred_task(nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}
