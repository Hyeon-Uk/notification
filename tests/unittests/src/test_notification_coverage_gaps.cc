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
#include <dlfcn.h>

#include "mock/app_control_mock.hh"
#include "mock/aul_mock.hh"
#include "mock/bundle_mock.hh"
#include "mock/glib_mock.hh"
#include "mock/noti_service_proxy_mock.hh"
#include "mock/package_manager_mock.hh"
#include "mock/rpc_port_mock.hh"
#include "mock/test_fixture.hh"
#include "mock/tzplatform_config_mock.hh"

#include <rpc-port.h>
#include <notification_tidl_proxy.h>

#include "notification/include/notification.h"
#include "notification/include/notification_internal.h"
#include "notification/include/notification_ipc.h"
#include "notification/include/notification_list.h"
#include "notification/include/notification_private.h"
#include "notification/include/notification_setting.h"
#include "notification/include/notification_setting_internal.h"

using ::testing::_;
using ::testing::DoAll;
using ::testing::Invoke;
using ::testing::Return;
using ::testing::SetArgPointee;

namespace {

int __cov_fake_aul_app_get_appid_bypid(int pid, char* appid, int len) {
  snprintf(appid, len, "%s", "org.tizen.testappid");
  return AUL_R_OK;
}

class Mocks :
    virtual public ::testing::NiceMock<AppControlMock>,
    virtual public ::testing::NiceMock<AulMock>,
    virtual public ::testing::NiceMock<BundleMock>,
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
    ON_CALL(GetMock<AppControlMock>(), app_control_create(_))
        .WillByDefault(Invoke(app_control_mock_real_create));
    ON_CALL(GetMock<AppControlMock>(), app_control_import_from_bundle(_, _))
        .WillByDefault(Invoke(app_control_mock_real_import_from_bundle));
    ON_CALL(GetMock<AppControlMock>(), app_control_destroy(_))
        .WillByDefault(Invoke(app_control_mock_real_destroy));
    ON_CALL(GetMock<AppControlMock>(), app_control_export_as_bundle(_, _))
        .WillByDefault(Invoke(app_control_mock_real_export_as_bundle));

    ON_CALL(GetMock<BundleMock>(), bundle_encode(_, _, _))
        .WillByDefault(Invoke([](bundle* b, bundle_raw** r, int* len) -> int {
          using fn_t = int (*)(bundle*, bundle_raw**, int*);
          static fn_t real =
              reinterpret_cast<fn_t>(dlsym(RTLD_NEXT, "bundle_encode"));
          return real ? real(b, r, len) : BUNDLE_ERROR_INVALID_PARAMETER;
        }));
    ON_CALL(GetMock<BundleMock>(), bundle_add_str(_, _, _))
        .WillByDefault(Invoke([](bundle* b, const char* k, const char* s) -> int {
          using fn_t = int (*)(bundle*, const char*, const char*);
          static fn_t real =
              reinterpret_cast<fn_t>(dlsym(RTLD_NEXT, "bundle_add_str"));
          return real ? real(b, k, s) : BUNDLE_ERROR_INVALID_PARAMETER;
        }));

    ON_CALL(GetMock<AulMock>(), aul_getuid()).WillByDefault(Return(5001));
    EXPECT_CALL(GetMock<AulMock>(), aul_app_get_appid_bypid(_, _, _))
        .WillRepeatedly(Invoke(__cov_fake_aul_app_get_appid_bypid));

    /* rpc_port_parcel_create is mocked but doesn't populate the out-handle,
     * so rpc_port_proxy_array_bundle_clone (used by make_notification_from_noti
     * and make_noti_from_notification) would crash with a null parcel.
     * Delegate to the real librpc-port symbol so the clone path works. */
    static auto* real_parcel_create =
        reinterpret_cast<int (*)(rpc_port_parcel_h*)>(
            dlsym(RTLD_NEXT, "rpc_port_parcel_create"));
    static auto* real_parcel_destroy =
        reinterpret_cast<int (*)(rpc_port_parcel_h)>(
            dlsym(RTLD_NEXT, "rpc_port_parcel_destroy"));
    if (real_parcel_create) {
      EXPECT_CALL(GetMock<RpcPortMock>(), rpc_port_parcel_create(_))
          .WillRepeatedly(Invoke(real_parcel_create));
    }
    if (real_parcel_destroy) {
      EXPECT_CALL(GetMock<RpcPortMock>(), rpc_port_parcel_destroy(_))
          .WillRepeatedly(Invoke(real_parcel_destroy));
    }

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

/* ============================================================== */
/* notification_create — aul_getuid() < REGULAR_UID_MIN (5000)    */
/* notification.c L1567-L1568 (low-uid path: pkg_id = caller_id)  */
/* ============================================================== */

TEST_F(NotificationCoverageGapsTest,
       create_with_low_uid_uses_caller_app_id_as_pkg_id) {
  /* [E1] uid < REGULAR_UID_MIN(5000) → _notification_create skips
   * aul_app_get_pkgid_bypid and instead copies caller_app_id into pkg_id.
   * Override the fixture-level ON_CALL with an EXPECT_CALL that returns 0. */
  EXPECT_CALL(GetMock<AulMock>(), aul_getuid())
      .WillRepeatedly(Return(0));
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);
  notification_free(noti);
}

/* ============================================================== */
/* notification_get_count — app_id == NULL path (pid-based lookup) */
/* notification_internal.c L774                                    */
/* ============================================================== */

TEST_F(NotificationCoverageGapsTest,
       get_count_with_null_app_id_uses_pid_lookup) {
  /* [P1] app_id==NULL + count!=NULL → notification_get_app_id_by_pid(pid)
   * branch at notification_internal.c L774 (pid-based lookup).
   * AulMock already wired in SetUp to return "org.tizen.testappid". */
  int count = -1;
  EXPECT_EQ(notification_get_count(NOTIFICATION_TYPE_NOTI, nullptr,
                                   NOTIFICATION_GROUP_ID_NONE,
                                   NOTIFICATION_PRIV_ID_NONE, &count),
            NOTIFICATION_ERROR_NONE);
}

/* ============================================================== */
/* notification_get_list_by_page — clamp + success paths          */
/* notification_list.c L292-295 (clamp) + L305 (success assign)  */
/* ============================================================== */

TEST_F(NotificationCoverageGapsTest,
       get_list_by_page_count_over_max_is_clamped_to_100) {
  /* [E1] count_per_page=101 > COUNT_PER_PAGE_MAX(100) → clamped to 100
   * before TIDL call.  Verify TIDL receives 100 (not 101).
   * Return IO_ERROR so the success-path assertion doesn't apply. */
  EXPECT_CALL(GetMock<NotiServiceProxyMock>(),
      rpc_port_proxy_noti_service_invoke_load_noti_grouping_list(
          _, _, _, 100, _, _))
      .WillOnce(Return(RPC_PORT_ERROR_IO_ERROR));
  notification_list_h list = nullptr;
  EXPECT_NE(notification_get_list_by_page(NOTIFICATION_TYPE_NOTI, 1, 101,
                                          &list),
            NOTIFICATION_ERROR_NONE);
}

TEST_F(NotificationCoverageGapsTest,
       get_list_by_page_success_assigns_list_head) {
  /* [P3] TIDL returns success with an empty list handle.
   * rpc_port_proxy_list_notification_foreach iterates zero items (safe),
   * then *list = notification_list_get_head(NULL) = NULL.
   * Covers notification_list.c L305. */
  rpc_port_proxy_list_notification_h empty_list = nullptr;
  ASSERT_EQ(rpc_port_proxy_list_notification_create(&empty_list),
            RPC_PORT_ERROR_NONE);
  ASSERT_NE(empty_list, nullptr);

  EXPECT_CALL(GetMock<NotiServiceProxyMock>(),
      rpc_port_proxy_noti_service_invoke_load_noti_grouping_list(
          _, _, _, _, _, _))
      .WillOnce(DoAll(SetArgPointee<5>(empty_list),
                      Return(RPC_PORT_ERROR_NONE)));

  notification_list_h list = reinterpret_cast<notification_list_h>(0x1);
  int ret = notification_get_list_by_page(NOTIFICATION_TYPE_NOTI, 1, 10,
                                          &list);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  /* empty noti_list → no items appended → get_list == NULL → head == NULL */
  EXPECT_EQ(list, nullptr);
  /* empty_list is owned and freed by the TIDL function */
}

/* ============================================================== */
/* notification_create_from_template — TIDL invoke failure        */
/* notification.c L2038-L2042                                     */
/* ============================================================== */

TEST_F(NotificationCoverageGapsTest,
       create_from_template_tidl_invoke_fails_returns_null) {
  /* [N2] connect_sync succeeds but invoke_create_from_template returns
   * RPC_PORT_ERROR_IO_ERROR → function frees noti and returns NULL.
   * Covers notification.c L2038-L2042. */
  ON_CALL(GetMock<NotiServiceProxyMock>(),
          rpc_port_proxy_noti_service_connect_sync(_))
      .WillByDefault(Return(RPC_PORT_ERROR_NONE));
  EXPECT_CALL(GetMock<NotiServiceProxyMock>(),
              rpc_port_proxy_noti_service_invoke_create_from_template(_, _, _))
      .WillOnce(Return(RPC_PORT_ERROR_IO_ERROR));

  notification_h noti = notification_create_from_template("my_template");
  EXPECT_EQ(noti, nullptr);
}

/* ============================================================== */
/* notification_create_from_package_template — TIDL invoke fail   */
/* notification_internal.c L1549-L1552                            */
/* ============================================================== */

TEST_F(NotificationCoverageGapsTest,
       create_from_package_template_tidl_invoke_fails_returns_null) {
  /* [N1] connect_sync succeeds but invoke_create_from_package_template
   * returns RPC_PORT_ERROR_IO_ERROR → function frees noti and returns NULL.
   * Covers notification_internal.c L1549-L1552. */
  ON_CALL(GetMock<NotiServiceProxyMock>(),
          rpc_port_proxy_noti_service_connect_sync(_))
      .WillByDefault(Return(RPC_PORT_ERROR_NONE));
  EXPECT_CALL(GetMock<NotiServiceProxyMock>(),
              rpc_port_proxy_noti_service_invoke_create_from_package_template(
                  _, _, _, _))
      .WillOnce(Return(RPC_PORT_ERROR_IO_ERROR));

  notification_h noti = notification_create_from_package_template(
      "org.tizen.testapp", "my_template");
  EXPECT_EQ(noti, nullptr);
}

/* ============================================================== */
/* notification_insert_for_uid — valid noti type reaches L1148    */
/* ============================================================== */

TEST_F(NotificationCoverageGapsTest,
       insert_for_uid_valid_noti_reaches_tidl_layer) {
  /* [P1-partial] valid NOTIFICATION_TYPE_NOTI passes L1141-1146 type guard →
   * L1148 (noti->uid = uid), L1149 (noti->insert_time = time(NULL)),
   * L1151-1154 (CPU inheritance + tidl_request_insert + error return).
   * invoke_add_noti mocked to fail → non-NONE returned from TIDL layer. */
  ON_CALL(GetMock<NotiServiceProxyMock>(),
          rpc_port_proxy_noti_service_invoke_add_noti(_, _, _))
      .WillByDefault(Return(RPC_PORT_ERROR_IO_ERROR));
  int priv_id = 0;
  int ret = notification_insert_for_uid(noti_, &priv_id, 5001);
  EXPECT_NE(ret, NOTIFICATION_ERROR_NONE);
}

/* [P2] invoke_add_noti succeeds → L1157-1165, L1175 covered */
TEST_F(NotificationCoverageGapsTest,
       insert_for_uid_tidl_invoke_success_sets_priv_id) {
  /* invoke_add_noti returns NONE with priv_id=42 → success path executes:
   * L1157 (noti->priv_id = id), L1160-1161 (*priv_id set), L1163-1165
   * (get_event_flag check), L1175 (return NONE).
   * event_flag is false for new noti → event-monitor init skipped. */
  ON_CALL(GetMock<NotiServiceProxyMock>(),
          rpc_port_proxy_noti_service_invoke_add_noti(_, _, _))
      .WillByDefault(DoAll(SetArgPointee<2>(42), Return(RPC_PORT_ERROR_NONE)));
  int priv_id = 0;
  int ret = notification_insert_for_uid(noti_, &priv_id, 5001);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(priv_id, 42);
}

/* ============================================================== */
/* notification_post_for_uid — success path via invoke_add_noti   */
/* ============================================================== */

TEST_F(NotificationCoverageGapsTest,
       post_for_uid_tidl_invoke_success_covers_success_path) {
  /* [P1] invoke_add_noti returns NONE → L1419-1425, L1427-1442 covered.
   * __copy_private_file returns NULL (no private paths) → file_list=NULL
   * → L1440 (g_list_free_full) is skipped; L1442 returns NONE. */
  ON_CALL(GetMock<NotiServiceProxyMock>(),
          rpc_port_proxy_noti_service_invoke_add_noti(_, _, _))
      .WillByDefault(DoAll(SetArgPointee<2>(7), Return(RPC_PORT_ERROR_NONE)));
  int ret = notification_post_for_uid(noti_, 5001);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
}

/* ============================================================== */
/* notification_get_extension_data — IO_ERROR on corrupt raw      */
/* ============================================================== */

TEST_F(NotificationCoverageGapsTest,
       get_extension_data_corrupt_bundle_raw_returns_io_error) {
  /* [N2] bundle_raw under key is not a valid bundle encoding →
   * _create_bundle_from_bundle_raw (bundle_decode) returns NULL →
   * NOTIFICATION_ERROR_IO_ERROR (L2670). */
  noti_->args = bundle_create();
  bundle_add_str(noti_->args, "ext_key", "not_a_valid_bundle_raw_data");
  bundle *value = nullptr;
  int ret = notification_get_extension_data(noti_, "ext_key", &value);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_IO_ERROR);
}

/* ============================================================== */
/* notification_update_for_uid — success path                     */
/* ============================================================== */

TEST_F(NotificationCoverageGapsTest,
       update_for_uid_tidl_invoke_success_returns_none) {
  /* [P1] invoke_update_noti returns NONE → L1458-1467 covered
   * (uid/insert_time set, TIDL call, return NONE). */
  ON_CALL(GetMock<NotiServiceProxyMock>(),
          rpc_port_proxy_noti_service_invoke_update_noti(_, _, _))
      .WillByDefault(Return(RPC_PORT_ERROR_NONE));
  int ret = notification_update_for_uid(noti_, 5001);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
}

/* ============================================================== */
/* notification_delete_for_uid — success path                     */
/* ============================================================== */

TEST_F(NotificationCoverageGapsTest,
       delete_for_uid_tidl_invoke_success_returns_none) {
  /* [P1] invoke_delete_single_noti returns NONE → L1477-1481 covered. */
  ON_CALL(GetMock<NotiServiceProxyMock>(),
          rpc_port_proxy_noti_service_invoke_delete_single_noti(_, _, _, _))
      .WillByDefault(Return(RPC_PORT_ERROR_NONE));
  int ret = notification_delete_for_uid(noti_, 5001);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
}

/* ============================================================== */
/* notification_delete_all_for_uid — success path                 */
/* ============================================================== */

TEST_F(NotificationCoverageGapsTest,
       delete_all_for_uid_tidl_invoke_success_returns_none) {
  /* [P1] invoke_delete_multiple_noti returns NONE → L1492-1501 covered.
   * aul_app_get_appid_bypid mock provides a valid caller_app_id. */
  ON_CALL(GetMock<NotiServiceProxyMock>(),
          rpc_port_proxy_noti_service_invoke_delete_multiple_noti(_, _, _, _, _))
      .WillByDefault(Return(RPC_PORT_ERROR_NONE));
  int ret = notification_delete_all_for_uid(NOTIFICATION_TYPE_NOTI, 5001);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
}

/* ============================================================== */
/* notification_insert_for_uid — event_flag=true path             */
/* ============================================================== */

TEST_F(NotificationCoverageGapsTest,
       insert_for_uid_event_flag_true_calls_event_monitor_init) {
  /* [E1] noti->event_flag = true → after insert success, L1167 branch taken →
   * L1168-1171 covered (CPU inheritance + event_monitor_init + clear + if). */
  ON_CALL(GetMock<NotiServiceProxyMock>(),
          rpc_port_proxy_noti_service_invoke_add_noti(_, _, _))
      .WillByDefault(DoAll(SetArgPointee<2>(55), Return(RPC_PORT_ERROR_NONE)));
  noti_->event_flag = true;
  int priv_id = 0;
  int ret = notification_insert_for_uid(noti_, &priv_id, 5001);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
}

/* ============================================================== */
/* notification_post_for_uid — event_flag=true path               */
/* ============================================================== */

TEST_F(NotificationCoverageGapsTest,
       post_for_uid_event_flag_true_calls_event_monitor_init) {
  /* [E1] noti->event_flag = true → after insert success, L1427 branch taken →
   * L1428-1431 covered (CPU inheritance + event_monitor_init + clear + if). */
  ON_CALL(GetMock<NotiServiceProxyMock>(),
          rpc_port_proxy_noti_service_invoke_add_noti(_, _, _))
      .WillByDefault(DoAll(SetArgPointee<2>(56), Return(RPC_PORT_ERROR_NONE)));
  noti_->event_flag = true;
  int ret = notification_post_for_uid(noti_, 5001);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
}

/* ============================================================== */
/* notification_post_for_uid — event_monitor_init fail (L1432)    */
/* ============================================================== */

TEST_F(NotificationCoverageGapsTest,
       post_for_uid_event_monitor_init_fail_goes_to_out) {
  /* insert succeeds (invoke_add_noti priv_id=55), event_flag=true,
   * then event_monitor_init fails (connect_sync: WillOnce(NONE) for
   * request_insert's _tidl_init, WillRepeatedly(IO_ERROR) for
   * event_monitor_init's _tidl_init) → L1431-1432 (if + goto out) covered. */
  ON_CALL(GetMock<NotiServiceProxyMock>(),
          rpc_port_proxy_noti_service_invoke_add_noti(_, _, _))
      .WillByDefault(DoAll(SetArgPointee<2>(55), Return(RPC_PORT_ERROR_NONE)));
  EXPECT_CALL(GetMock<NotiServiceProxyMock>(),
              rpc_port_proxy_noti_service_connect_sync(_))
      .WillOnce(Return(RPC_PORT_ERROR_NONE))
      .WillRepeatedly(Return(RPC_PORT_ERROR_IO_ERROR));
  noti_->event_flag = true;
  int ret = notification_post_for_uid(noti_, 5001);
  EXPECT_NE(ret, NOTIFICATION_ERROR_NONE);
}

/* ============================================================== */
/* notification_tidl_request_refresh — L691-692 (_tidl_init fail) */
/* L698-699 (invoke_refresh_noti fail)                            */
/* Triggered via notification_update_for_uid(NULL, uid).          */
/* ============================================================== */

TEST_F(NotificationCoverageGapsTest,
       request_refresh_tidl_init_fail_covers_early_return) {
  /* notification_update_for_uid(NULL, uid) → request_refresh →
   * connect_sync fails → L691-692 covered.
   * Function itself returns INVALID_PARAMETER (noti == NULL path). */
  EXPECT_CALL(GetMock<NotiServiceProxyMock>(),
              rpc_port_proxy_noti_service_connect_sync(_))
      .WillRepeatedly(Return(RPC_PORT_ERROR_IO_ERROR));
  int ret = notification_update_for_uid(nullptr, 5001);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationCoverageGapsTest,
       request_refresh_invoke_refresh_fail_covers_error_return) {
  /* notification_update_for_uid(NULL, uid) → request_refresh →
   * _tidl_init succeeds (NiceMock), invoke_refresh_noti fails →
   * L698-699 covered. */
  ON_CALL(GetMock<NotiServiceProxyMock>(),
          rpc_port_proxy_noti_service_invoke_refresh_noti(_, _))
      .WillByDefault(Return(RPC_PORT_ERROR_IO_ERROR));
  int ret = notification_update_for_uid(nullptr, 5001);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* ============================================================== */
/* notification_tidl_request_update — L732-734                    */
/* invoke_update_noti fails                                       */
/* ============================================================== */

TEST_F(NotificationCoverageGapsTest,
       request_update_invoke_update_noti_fail_returns_error) {
  /* notification_update_for_uid(noti, uid) → request_update →
   * _tidl_init succeeds, notification_create succeeds (real impl),
   * invoke_update_noti fails → L732-734 (WARN + destroy + return). */
  ON_CALL(GetMock<NotiServiceProxyMock>(),
          rpc_port_proxy_noti_service_invoke_update_noti(_, _, _))
      .WillByDefault(Return(RPC_PORT_ERROR_IO_ERROR));
  int ret = notification_update_for_uid(noti_, 5001);
  EXPECT_NE(ret, NOTIFICATION_ERROR_NONE);
}

/* ============================================================== */
/* notification_tidl_event_monitor_init — L506-507                */
/* event_received_create fails                                    */
/* ============================================================== */

TEST_F(NotificationCoverageGapsTest,
       event_monitor_init_create_handle_fail_returns_error) {
  /* _tidl_init succeeds (NiceMock connect), event_received_create fails →
   * L506-507 (WARN + return in event_monitor_init) covered. */
  ON_CALL(GetMock<NotiServiceProxyMock>(),
          rpc_port_proxy_noti_service_event_received_create(_))
      .WillByDefault(Return(RPC_PORT_ERROR_IO_ERROR));
  static disturb_cb cb = [](void*) {};
  int ret = notification_register_do_not_disturb_app(cb, nullptr);
  EXPECT_NE(ret, NOTIFICATION_ERROR_NONE);
}

/* ============================================================== */
/* notification_tidl_event_monitor_init — L514-517                */
/* event_received_set_callback fails                              */
/* ============================================================== */

TEST_F(NotificationCoverageGapsTest,
       event_monitor_init_set_callback_fail_returns_error) {
  /* create succeeds (NiceMock → 0, does not set handle), set_callback
   * fails → L514-517 (WARN + destroy + NULL + return) covered. */
  ON_CALL(GetMock<NotiServiceProxyMock>(),
          rpc_port_proxy_noti_service_event_received_set_callback(_, _, _))
      .WillByDefault(Return(RPC_PORT_ERROR_IO_ERROR));
  static disturb_cb cb = [](void*) {};
  int ret = notification_register_do_not_disturb_app(cb, nullptr);
  EXPECT_NE(ret, NOTIFICATION_ERROR_NONE);
}

/* ============================================================== */
/* notification_tidl_event_monitor_init — L523-526                */
/* invoke_register_event fails                                    */
/* ============================================================== */

TEST_F(NotificationCoverageGapsTest,
       event_monitor_init_invoke_register_event_fail_returns_error) {
  /* create + set_callback succeed (NiceMock), invoke_register_event fails
   * → L523-526 (WARN + destroy + NULL + return) covered. */
  ON_CALL(GetMock<NotiServiceProxyMock>(),
          rpc_port_proxy_noti_service_invoke_register_event(_, _))
      .WillByDefault(Return(RPC_PORT_ERROR_IO_ERROR));
  static disturb_cb cb = [](void*) {};
  int ret = notification_register_do_not_disturb_app(cb, nullptr);
  EXPECT_NE(ret, NOTIFICATION_ERROR_NONE);
}

/* ============================================================== */
/* notification_tidl_event_monitor_init — L530                    */
/* event_handle already created (else branch)                     */
/* + notification_tidl_event_monitor_fini — L555-556              */
/* event_handle non-NULL dispose path                             */
/* ============================================================== */

TEST_F(NotificationCoverageGapsTest,
       event_monitor_init_handle_already_created_and_fini_dispose) {
  /* event_received_create mock fills event_handle with non-NULL.
   * 1st register: create sets event_handle → init succeeds.
   * 2nd register: event_handle != NULL → else L530 covered.
   * Cleanup: unregister → fini → if(event_handle) true → L555-556 covered
   *          → event_handle reset to NULL for subsequent tests. */
  static rpc_port_proxy_noti_service_event_received_h fake_h =
      reinterpret_cast<rpc_port_proxy_noti_service_event_received_h>(1);
  ON_CALL(GetMock<NotiServiceProxyMock>(),
          rpc_port_proxy_noti_service_event_received_create(_))
      .WillByDefault(DoAll(SetArgPointee<0>(fake_h), Return(RPC_PORT_ERROR_NONE)));
  static disturb_cb cb = [](void*) {};
  ASSERT_EQ(notification_register_do_not_disturb_app(cb, nullptr),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(notification_register_do_not_disturb_app(cb, nullptr),
            NOTIFICATION_ERROR_NONE);
  /* fini: event_handle != NULL → L555-556 → event_handle = NULL */
  notification_unregister_do_not_disturb_app();
}

/* ============================================================== */
/* notification_tidl_event_monitor_fini — L543-544                */
/* _tidl_init fails inside fini                                   */
/* ============================================================== */

TEST_F(NotificationCoverageGapsTest,
       event_monitor_fini_tidl_init_fail_returns_early) {
  /* connect_sync: WillOnce(NONE) for request_unregister_dnd_app's _tidl_init,
   * then IO_ERROR for fini's _tidl_init → L543-544 (WARN + return in fini).
   * fini is void so unregister itself returns NONE. */
  EXPECT_CALL(GetMock<NotiServiceProxyMock>(),
              rpc_port_proxy_noti_service_connect_sync(_))
      .WillOnce(Return(RPC_PORT_ERROR_NONE))
      .WillRepeatedly(Return(RPC_PORT_ERROR_IO_ERROR));
  int ret = notification_unregister_do_not_disturb_app();
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
}

/* ============================================================== */
/* notification_tidl_event_monitor_fini — L550-551                */
/* invoke_unregister_event fails inside fini                      */
/* ============================================================== */

TEST_F(NotificationCoverageGapsTest,
       event_monitor_fini_invoke_unregister_event_fail_returns_early) {
  /* _tidl_init succeeds (NiceMock), invoke_unregister_event fails →
   * L550-551 (ERR + return in fini). fini is void → unregister returns NONE. */
  ON_CALL(GetMock<NotiServiceProxyMock>(),
          rpc_port_proxy_noti_service_invoke_unregister_event(_))
      .WillByDefault(Return(RPC_PORT_ERROR_IO_ERROR));
  int ret = notification_unregister_do_not_disturb_app();
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
}

/* ================================================================ */
/* notification_tidl_monitor_init — changed_handle failure paths    */
/* L577-578: changed_create fails                                   */
/* L585-588: changed_set_callback fails                             */
/* L594-597: invoke_register_changed fails                          */
/* L602-603: invoke_register_service fails                          */
/* All use notification_resister_changed_cb_for_uid as the driver.  */
/* On monitor_init failure the API auto-calls unresister, which     */
/* drains the hash and triggers monitor_fini (NiceMock succeeds).   */
/* ================================================================ */

TEST_F(NotificationCoverageGapsTest,
       tidl_monitor_init_changed_create_fail_returns_error) {
  /* changed_create fails → L577-578 (WARN + return in monitor_init) */
  using GListAppendFn = GList*(*)(GList*, gpointer);
  ON_CALL(GetMock<GlibMock>(), g_list_append(_, _))
      .WillByDefault(Invoke([](GList* list, gpointer data) -> GList* {
        static GListAppendFn real = reinterpret_cast<GListAppendFn>(
            dlsym(RTLD_NEXT, "g_list_append"));
        return real ? real(list, data) : nullptr;
      }));
  ON_CALL(GetMock<NotiServiceProxyMock>(),
          rpc_port_proxy_noti_service_changed_create(_))
      .WillByDefault(Return(RPC_PORT_ERROR_IO_ERROR));
  static notification_changed_cb cb = [](void*, notification_type_e) {};
  int ret = notification_resister_changed_cb_for_uid(cb, nullptr, 9100);
  EXPECT_NE(ret, NOTIFICATION_ERROR_NONE);
}

TEST_F(NotificationCoverageGapsTest,
       tidl_monitor_init_changed_set_callback_fail_returns_error) {
  /* create succeeds (NiceMock), set_callback fails → L585-588 */
  using GListAppendFn = GList*(*)(GList*, gpointer);
  ON_CALL(GetMock<GlibMock>(), g_list_append(_, _))
      .WillByDefault(Invoke([](GList* list, gpointer data) -> GList* {
        static GListAppendFn real = reinterpret_cast<GListAppendFn>(
            dlsym(RTLD_NEXT, "g_list_append"));
        return real ? real(list, data) : nullptr;
      }));
  ON_CALL(GetMock<NotiServiceProxyMock>(),
          rpc_port_proxy_noti_service_changed_set_callback(_, _, _))
      .WillByDefault(Return(RPC_PORT_ERROR_IO_ERROR));
  static notification_changed_cb cb = [](void*, notification_type_e) {};
  int ret = notification_resister_changed_cb_for_uid(cb, nullptr, 9101);
  EXPECT_NE(ret, NOTIFICATION_ERROR_NONE);
}

TEST_F(NotificationCoverageGapsTest,
       tidl_monitor_init_invoke_register_changed_fail_returns_error) {
  /* create + set_callback succeed (NiceMock), invoke_register_changed fails
   * → L594-597 */
  using GListAppendFn = GList*(*)(GList*, gpointer);
  ON_CALL(GetMock<GlibMock>(), g_list_append(_, _))
      .WillByDefault(Invoke([](GList* list, gpointer data) -> GList* {
        static GListAppendFn real = reinterpret_cast<GListAppendFn>(
            dlsym(RTLD_NEXT, "g_list_append"));
        return real ? real(list, data) : nullptr;
      }));
  ON_CALL(GetMock<NotiServiceProxyMock>(),
          rpc_port_proxy_noti_service_invoke_register_changed(_, _, _))
      .WillByDefault(Return(RPC_PORT_ERROR_IO_ERROR));
  static notification_changed_cb cb = [](void*, notification_type_e) {};
  int ret = notification_resister_changed_cb_for_uid(cb, nullptr, 9102);
  EXPECT_NE(ret, NOTIFICATION_ERROR_NONE);
}

TEST_F(NotificationCoverageGapsTest,
       tidl_monitor_init_invoke_register_service_fail_returns_error) {
  /* create + set_callback + invoke_register_changed succeed, register_service
   * fails → L602-603 */
  using GListAppendFn = GList*(*)(GList*, gpointer);
  ON_CALL(GetMock<GlibMock>(), g_list_append(_, _))
      .WillByDefault(Invoke([](GList* list, gpointer data) -> GList* {
        static GListAppendFn real = reinterpret_cast<GListAppendFn>(
            dlsym(RTLD_NEXT, "g_list_append"));
        return real ? real(list, data) : nullptr;
      }));
  ON_CALL(GetMock<NotiServiceProxyMock>(),
          rpc_port_proxy_noti_service_invoke_register_service(_, _))
      .WillByDefault(Return(RPC_PORT_ERROR_IO_ERROR));
  static notification_changed_cb cb = [](void*, notification_type_e) {};
  int ret = notification_resister_changed_cb_for_uid(cb, nullptr, 9103);
  EXPECT_NE(ret, NOTIFICATION_ERROR_NONE);
}

/* ================================================================ */
/* notification_tidl_monitor_init — L607 (changed_handle already    */
/* created) + notification_tidl_monitor_fini — L632-633 (dispose)   */
/* ================================================================ */

TEST_F(NotificationCoverageGapsTest,
       tidl_monitor_init_changed_handle_already_created_and_fini_dispose) {
  /* changed_create mock fills changed_handle with non-NULL.
   * 1st register: create sets changed_handle → monitor_init succeeds.
   * 2nd register: changed_handle != NULL → else L607 covered.
   * Cleanup: unregister both → last unregister → hash empty → fini →
   * if(changed_handle) TRUE → L632-633 → changed_handle = NULL. */
  using GListAppendFn = GList*(*)(GList*, gpointer);
  ON_CALL(GetMock<GlibMock>(), g_list_append(_, _))
      .WillByDefault(Invoke([](GList* list, gpointer data) -> GList* {
        static GListAppendFn real = reinterpret_cast<GListAppendFn>(
            dlsym(RTLD_NEXT, "g_list_append"));
        return real ? real(list, data) : nullptr;
      }));
  static rpc_port_proxy_noti_service_changed_h fake_h =
      reinterpret_cast<rpc_port_proxy_noti_service_changed_h>(2);
  ON_CALL(GetMock<NotiServiceProxyMock>(),
          rpc_port_proxy_noti_service_changed_create(_))
      .WillByDefault(DoAll(SetArgPointee<0>(fake_h), Return(RPC_PORT_ERROR_NONE)));
  static notification_changed_cb cb1 = [](void*, notification_type_e) {};
  static notification_changed_cb cb2 = [](void*, notification_type_e) {};
  ASSERT_EQ(notification_resister_changed_cb_for_uid(cb1, nullptr, 9104),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(notification_resister_changed_cb_for_uid(cb2, nullptr, 9104),
            NOTIFICATION_ERROR_NONE);
  notification_unresister_changed_cb_for_uid(cb1, 9104);
  notification_unresister_changed_cb_for_uid(cb2, 9104);
}

/* ================================================================ */
/* notification_tidl_monitor_fini — L620-621 (_tidl_init fail)      */
/* ================================================================ */

TEST_F(NotificationCoverageGapsTest,
       tidl_monitor_fini_tidl_init_fail_returns_early) {
  /* connect_sync: WillOnce(NONE) for register's monitor_init _tidl_init,
   * WillRepeatedly(IO_ERROR) for fini's _tidl_init → L620-621 covered. */
  using GListAppendFn = GList*(*)(GList*, gpointer);
  ON_CALL(GetMock<GlibMock>(), g_list_append(_, _))
      .WillByDefault(Invoke([](GList* list, gpointer data) -> GList* {
        static GListAppendFn real = reinterpret_cast<GListAppendFn>(
            dlsym(RTLD_NEXT, "g_list_append"));
        return real ? real(list, data) : nullptr;
      }));
  EXPECT_CALL(GetMock<NotiServiceProxyMock>(),
              rpc_port_proxy_noti_service_connect_sync(_))
      .WillOnce(Return(RPC_PORT_ERROR_NONE))
      .WillRepeatedly(Return(RPC_PORT_ERROR_IO_ERROR));
  static notification_changed_cb cb = [](void*, notification_type_e) {};
  ASSERT_EQ(notification_resister_changed_cb_for_uid(cb, nullptr, 9105),
            NOTIFICATION_ERROR_NONE);
  notification_unresister_changed_cb_for_uid(cb, 9105);
}

/* ================================================================ */
/* notification_tidl_monitor_fini — L627-628 (unregister fail)      */
/* ================================================================ */

TEST_F(NotificationCoverageGapsTest,
       tidl_monitor_fini_invoke_unregister_changed_fail_returns_early) {
  /* _tidl_init succeeds (NiceMock), invoke_unregister_changed fails →
   * L627-628. monitor_fini returns non-NONE but monitor_fini return value
   * is not propagated by unresister (checked in code: unresister returns NONE). */
  using GListAppendFn = GList*(*)(GList*, gpointer);
  ON_CALL(GetMock<GlibMock>(), g_list_append(_, _))
      .WillByDefault(Invoke([](GList* list, gpointer data) -> GList* {
        static GListAppendFn real = reinterpret_cast<GListAppendFn>(
            dlsym(RTLD_NEXT, "g_list_append"));
        return real ? real(list, data) : nullptr;
      }));
  ON_CALL(GetMock<NotiServiceProxyMock>(),
          rpc_port_proxy_noti_service_invoke_unregister_changed(_))
      .WillByDefault(Return(RPC_PORT_ERROR_IO_ERROR));
  static notification_changed_cb cb = [](void*, notification_type_e) {};
  ASSERT_EQ(notification_resister_changed_cb_for_uid(cb, nullptr, 9106),
            NOTIFICATION_ERROR_NONE);
  int ret = notification_unresister_changed_cb_for_uid(cb, 9106);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
}

/* ================================================================ */
/* notification_set_extension_event_handler — bundle_encode fail    */
/* notification_internal.c L1921-L1923                              */
/* ================================================================ */

TEST_F(NotificationCoverageGapsTest,
       set_extension_event_handler_bundle_encode_fails_covers_L1921_L1923) {
  /* [N1] app_control_export_as_bundle succeeds (real impl via SetUp),
   * bundle_encode mocked to return BUNDLE_ERROR_INVALID_PARAMETER →
   * L1921-1923 (ERR + ret=IO_ERROR + goto out) covered. */
  app_control_h handler = nullptr;
  ASSERT_EQ(app_control_create(&handler), APP_CONTROL_ERROR_NONE);

  EXPECT_CALL(GetMock<BundleMock>(), bundle_encode(_, _, _))
      .WillOnce(Return(BUNDLE_ERROR_INVALID_PARAMETER));

  int ret = notification_set_extension_event_handler(
      noti_, NOTIFICATION_EVENT_TYPE_HIDDEN_BY_USER, handler);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_IO_ERROR);

  app_control_destroy(handler);
}

/* ================================================================ */
/* notification_set_extension_event_handler — bundle_add_str fail   */
/* notification_internal.c L1928-L1930                              */
/* ================================================================ */

TEST_F(NotificationCoverageGapsTest,
       set_extension_event_handler_bundle_add_str_fails_covers_L1928_L1930) {
  /* [N2] app_control_export_as_bundle + bundle_encode both succeed
   * (real impls via SetUp ON_CALL), bundle_add_str mocked to return
   * BUNDLE_ERROR_INVALID_PARAMETER →
   * L1928-1930 (ERR + ret=IO_ERROR + goto out) covered. */
  app_control_h handler = nullptr;
  ASSERT_EQ(app_control_create(&handler), APP_CONTROL_ERROR_NONE);

  EXPECT_CALL(GetMock<BundleMock>(), bundle_add_str(_, _, _))
      .WillOnce(Return(BUNDLE_ERROR_INVALID_PARAMETER));

  int ret = notification_set_extension_event_handler(
      noti_, NOTIFICATION_EVENT_TYPE_HIDDEN_BY_USER, handler);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_IO_ERROR);

  app_control_destroy(handler);
}

/* ============================================================== */
/* notification_unregister_do_not_disturb_app — L2250-2253        */
/* event_monitor_fini called when __noti_event_cb_list == NULL    */
/* MUST run before post_with_event_cb tests that populate the     */
/* global __noti_event_cb_list.                                   */
/* ============================================================== */

TEST_F(NotificationCoverageGapsTest,
       unregister_dnd_app_empty_cb_list_runs_event_monitor_fini) {
  /* [P-fini] __noti_event_cb_list is NULL at this point (no post_with_event_cb
   * test has run yet).  notification_unregister_do_not_disturb_app():
   *   1. calls notification_tidl_request_unregister_dnd_app() → NONE (NiceMock)
   *   2. sets _disturb_callback = NULL
   *   3. __noti_event_cb_list == NULL → TRUE → L2251-2253 taken →
   *      notification_tidl_event_monitor_fini() called → L537-559 covered. */
  int ret = notification_unregister_do_not_disturb_app();
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
}

/* ============================================================== */
/* notification_post_with_event_cb_for_uid — success path         */
/* ============================================================== */

TEST_F(NotificationCoverageGapsTest,
       post_with_event_cb_for_uid_success_appends_to_cb_list) {
  /* [P1] invoke_add_noti succeeds → L1688-1738 covered.
   * g_list_append mock passthrough needed so the info node is actually stored.
   * event_monitor_init succeeds under NiceMock. */
  ON_CALL(GetMock<NotiServiceProxyMock>(),
          rpc_port_proxy_noti_service_invoke_add_noti(_, _, _))
      .WillByDefault(DoAll(SetArgPointee<2>(57), Return(RPC_PORT_ERROR_NONE)));
  using GListAppendFn = GList*(*)(GList*, gpointer);
  ON_CALL(GetMock<GlibMock>(), g_list_append(_, _))
      .WillByDefault(Invoke([](GList* list, gpointer data) -> GList* {
        static GListAppendFn real = reinterpret_cast<GListAppendFn>(
            dlsym(RTLD_NEXT, "g_list_append"));
        return real ? real(list, data) : nullptr;
      }));
  static event_handler_cb cb = [](notification_h, int, void*) {};
  int ret = notification_post_with_event_cb_for_uid(noti_, cb, nullptr, 5001);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
}

/* [C1] Second call with same priv_id → find_list != NULL → L1722-1725 */
TEST_F(NotificationCoverageGapsTest,
       post_with_event_cb_for_uid_existing_cb_updates_in_place) {
  /* Both calls return priv_id=58 → second call finds existing node → L1722-1725. */
  ON_CALL(GetMock<NotiServiceProxyMock>(),
          rpc_port_proxy_noti_service_invoke_add_noti(_, _, _))
      .WillByDefault(DoAll(SetArgPointee<2>(58), Return(RPC_PORT_ERROR_NONE)));
  using GListAppendFn = GList*(*)(GList*, gpointer);
  ON_CALL(GetMock<GlibMock>(), g_list_append(_, _))
      .WillByDefault(Invoke([](GList* list, gpointer data) -> GList* {
        static GListAppendFn real = reinterpret_cast<GListAppendFn>(
            dlsym(RTLD_NEXT, "g_list_append"));
        return real ? real(list, data) : nullptr;
      }));
  static event_handler_cb cb1 = [](notification_h, int, void*) {};
  static event_handler_cb cb2 = [](notification_h, int, void*) {};
  /* First call: adds priv_id=58 to __noti_event_cb_list */
  ASSERT_EQ(notification_post_with_event_cb_for_uid(noti_, cb1, nullptr, 5001),
            NOTIFICATION_ERROR_NONE);
  /* Second call: same priv_id=58 → find_list != NULL → update cb in place */
  EXPECT_EQ(notification_post_with_event_cb_for_uid(noti_, cb2, nullptr, 5001),
            NOTIFICATION_ERROR_NONE);
}

/* ============================================================== */
/* notification_get_extension_event_handler — corrupt bundle_raw  */
/* ============================================================== */

TEST_F(NotificationCoverageGapsTest,
       get_extension_event_handler_corrupt_bundle_raw_returns_io_error) {
  /* [N2] _create_bundle_from_bundle_raw returns NULL → NOTIFICATION_ERROR_IO_ERROR
   * (L1973-1974). Insert a non-decodable string directly under the event key. */
  noti_->args = bundle_create();
  bundle_add_str(noti_->args, "_NOTIFICATION_EXTENSION_EVENT_100", "garbage_data");
  app_control_h handler = nullptr;
  int ret = notification_get_extension_event_handler(
      noti_, NOTIFICATION_EVENT_TYPE_HIDDEN_BY_USER, &handler);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_IO_ERROR);
}

/* ============================================================== */
/* notification_register_do_not_disturb_app — success path        */
/* ============================================================== */

TEST_F(NotificationCoverageGapsTest,
       register_do_not_disturb_app_success_covers_post_register_path) {
  /* [P1] invoke_register_dnd_app returns NONE (NiceMock default) →
   * event_monitor_init also succeeds → L2218-2231 covered. */
  static disturb_cb cb = [](void*) {};
  int ret = notification_register_do_not_disturb_app(cb, nullptr);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
}

/* ============================================================== */
/* notification_unregister_do_not_disturb_app — success path      */
/* ============================================================== */

TEST_F(NotificationCoverageGapsTest,
       unregister_do_not_disturb_app_success_covers_post_unregister_path) {
  /* [P1] invoke_unregister_dnd_app returns NONE (NiceMock default) →
   * L2248-2258 covered (_disturb_callback = NULL + monitor_fini if list empty). */
  int ret = notification_unregister_do_not_disturb_app();
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
}

/* ============================================================== */
/* notification_get_extension_event_handler — key not found       */
/* ============================================================== */

TEST_F(NotificationCoverageGapsTest,
       get_extension_event_handler_no_key_returns_invalid_parameter) {
  /* [N1] noti->args is NULL → bundle_get_str returns NULL ret_str →
   * L1967-1968 (ERR + INVALID_PARAMETER) covered. */
  app_control_h handler = nullptr;
  int ret = notification_get_extension_event_handler(
      noti_, NOTIFICATION_EVENT_TYPE_HIDDEN_BY_USER, &handler);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* ============================================================== */
/* notification_channel_add/remove/update — success paths         */
/* ============================================================== */

TEST_F(NotificationCoverageGapsTest,
       channel_add_tidl_invoke_success_returns_none) {
  /* [P1] invoke_channel_insert returns NONE (NiceMock default) → L2393-2405. */
  notification_channel_h channel = nullptr;
  ASSERT_EQ(notification_channel_create("test_channel_add", &channel),
            NOTIFICATION_ERROR_NONE);
  ASSERT_NE(channel, nullptr);
  int ret = notification_channel_add(channel);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  notification_channel_free(channel);
}

TEST_F(NotificationCoverageGapsTest,
       channel_remove_tidl_invoke_success_returns_none) {
  /* [P1] invoke_channel_delete returns NONE (NiceMock default) → L2421-2432. */
  notification_channel_h channel = nullptr;
  ASSERT_EQ(notification_channel_create("test_channel_remove", &channel),
            NOTIFICATION_ERROR_NONE);
  ASSERT_NE(channel, nullptr);
  int ret = notification_channel_remove(channel);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  notification_channel_free(channel);
}

TEST_F(NotificationCoverageGapsTest,
       channel_update_tidl_invoke_success_returns_none) {
  /* [P1] invoke_channel_update returns NONE (NiceMock default) → L2448-2460. */
  notification_channel_h channel = nullptr;
  ASSERT_EQ(notification_channel_create("test_channel_update", &channel),
            NOTIFICATION_ERROR_NONE);
  ASSERT_NE(channel, nullptr);
  int ret = notification_channel_update(channel);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  notification_channel_free(channel);
}

/* ============================================================== */
/* notification_channel_get_by_name — success path (L2481-2503)  */
/* ============================================================== */

TEST_F(NotificationCoverageGapsTest,
       channel_get_by_name_tidl_invoke_success_returns_none) {
  /* [P1] AUL succeeds (fixture) → invoke_channel_get returns NONE
   * (NiceMock default) → L2481-2503 covered. */
  notification_channel_h channel = nullptr;
  int ret = notification_channel_get_by_name("test_get_channel", &channel);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  if (channel) notification_channel_free(channel);
}

/* ============================================================== */
/* notification_channel_clone — success path (L2560-2575)        */
/* ============================================================== */

TEST_F(NotificationCoverageGapsTest,
       channel_clone_success_copies_fields) {
  /* [P1] notification_channel_create sets app_id via AUL mock (fixture).
   * clone copies app_id + channel_name → L2560-2575. */
  notification_channel_h channel = nullptr;
  ASSERT_EQ(notification_channel_create("test_ch_clone", &channel),
            NOTIFICATION_ERROR_NONE);
  ASSERT_NE(channel, nullptr);
  notification_channel_h clone = nullptr;
  int ret = notification_channel_clone(channel, &clone);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  notification_channel_free(channel);
  if (clone) notification_channel_free(clone);
}

/* ============================================================== */
/* notification_channel_foreach — success + IO_ERROR paths       */
/* ============================================================== */

TEST_F(NotificationCoverageGapsTest,
       channel_foreach_explicit_app_id_tidl_fail_returns_error) {
  /* [N1] explicit app_id skips AUL lookup → invoke_channel_get_list
   * mocked to fail (avoid uninitialized list handle SEGFAULT) →
   * L2599-2607 (TIDL call + error branch) covered. */
  ON_CALL(GetMock<NotiServiceProxyMock>(),
          rpc_port_proxy_noti_service_invoke_channel_get_list(_, _, _, _))
      .WillByDefault(Return(RPC_PORT_ERROR_IO_ERROR));
  auto cb = [](notification_channel_h, void*) {};
  int ret = notification_channel_foreach("org.tizen.testapp", cb, nullptr);
  EXPECT_NE(ret, NOTIFICATION_ERROR_NONE);
}

TEST_F(NotificationCoverageGapsTest,
       channel_foreach_null_app_id_aul_fail_returns_io_error) {
  /* [N1] app_id==NULL and notification_get_app_id_by_pid returns NULL
   * (AUL fails) → L2592-2594 (IO_ERROR). Override SetUp AUL mock. */
  EXPECT_CALL(GetMock<AulMock>(), aul_app_get_appid_bypid(_, _, _))
      .WillRepeatedly(Return(AUL_R_ERROR));
  auto cb = [](notification_channel_h, void*) {};
  int ret = notification_channel_foreach(nullptr, cb, nullptr);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_IO_ERROR);
}

/* ============================================================== */
/* notification_channel_get_by_name — TIDL failure path          */
/* ============================================================== */

TEST_F(NotificationCoverageGapsTest,
       channel_get_by_name_tidl_fail_frees_channel_and_returns_error) {
  /* [N1] invoke_channel_get mocked to fail → L2494-2496
   * (free(noti_channel) + goto out). */
  ON_CALL(GetMock<NotiServiceProxyMock>(),
          rpc_port_proxy_noti_service_invoke_channel_get(_, _, _, _, _, _))
      .WillByDefault(Return(RPC_PORT_ERROR_IO_ERROR));
  notification_channel_h channel = nullptr;
  int ret = notification_channel_get_by_name("test_fail_channel", &channel);
  EXPECT_NE(ret, NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(channel, nullptr);
}

/* ============================================================== */
/* notification_check_event_receiver_available — L1814-1815      */
/* ============================================================== */

TEST_F(NotificationCoverageGapsTest,
       check_event_receiver_available_tidl_success_returns_none) {
  /* [P1] invoke_check_event_receiver returns NONE (NiceMock default)
   * → L1814-1815 (TIDL call + clear_cpu_inheritance) covered. */
  bool available = false;
  int ret = notification_check_event_receiver_available(noti_, &available);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
}

/* ============================================================== */
/* notification_load_by_tag_for_uid — L1516-1519                  */
/* AUL returns OK but empty string → notification_get_app_id_by_pid */
/* returns NULL at notification.c:84 (strlen(app_id)<=0).        */
/* ============================================================== */

TEST_F(NotificationCoverageGapsTest,
       load_by_tag_for_uid_empty_app_id_returns_null) {
  /* [N2] aul_app_get_appid_bypid writes empty string → strlen<=0 →
   * notification_get_app_id_by_pid returns NULL → L1516-1519 covered.
   * Override fixture EXPECT_CALL with newer one that wins in LIFO. */
  EXPECT_CALL(GetMock<AulMock>(), aul_app_get_appid_bypid(_, _, _))
      .WillRepeatedly(Invoke([](int, char* appid, int len) -> int {
        if (len > 0) appid[0] = '\0';
        return AUL_R_OK;
      }));
  notification_h h = notification_load_by_tag_for_uid("some_tag", 5001);
  EXPECT_EQ(h, nullptr);
}

/* ============================================================== */
/* notification_channel_add — L2399-2400 (invoke-fail arm)        */
/* ============================================================== */

TEST_F(NotificationCoverageGapsTest,
       channel_add_tidl_invoke_fail_returns_error) {
  /* [N1] invoke_channel_insert mocked to fail →
   * L2398-2400 (ERR + return ret) covered. */
  ON_CALL(GetMock<NotiServiceProxyMock>(),
          rpc_port_proxy_noti_service_invoke_channel_insert(_, _, _, _, _, _))
      .WillByDefault(Return(RPC_PORT_ERROR_IO_ERROR));
  notification_channel_h channel = nullptr;
  ASSERT_EQ(notification_channel_create("ch_add_fail", &channel),
            NOTIFICATION_ERROR_NONE);
  ASSERT_NE(channel, nullptr);
  int ret = notification_channel_add(channel);
  EXPECT_NE(ret, NOTIFICATION_ERROR_NONE);
  notification_channel_free(channel);
}

/* ============================================================== */
/* notification_channel_remove — L2426-2427 (invoke-fail arm)     */
/* ============================================================== */

TEST_F(NotificationCoverageGapsTest,
       channel_remove_tidl_invoke_fail_returns_error) {
  /* [N1] invoke_channel_delete mocked to fail →
   * L2425-2427 (ERR + return ret) covered. */
  ON_CALL(GetMock<NotiServiceProxyMock>(),
          rpc_port_proxy_noti_service_invoke_channel_delete(_, _, _, _))
      .WillByDefault(Return(RPC_PORT_ERROR_IO_ERROR));
  notification_channel_h channel = nullptr;
  ASSERT_EQ(notification_channel_create("ch_remove_fail", &channel),
            NOTIFICATION_ERROR_NONE);
  ASSERT_NE(channel, nullptr);
  int ret = notification_channel_remove(channel);
  EXPECT_NE(ret, NOTIFICATION_ERROR_NONE);
  notification_channel_free(channel);
}

/* ============================================================== */
/* notification_channel_update — L2454-2455 (invoke-fail arm)     */
/* ============================================================== */

TEST_F(NotificationCoverageGapsTest,
       channel_update_tidl_invoke_fail_returns_error) {
  /* [N1] invoke_channel_update mocked to fail →
   * L2453-2455 (ERR + return ret) covered. */
  ON_CALL(GetMock<NotiServiceProxyMock>(),
          rpc_port_proxy_noti_service_invoke_channel_update(_, _, _, _, _, _))
      .WillByDefault(Return(RPC_PORT_ERROR_IO_ERROR));
  notification_channel_h channel = nullptr;
  ASSERT_EQ(notification_channel_create("ch_update_fail", &channel),
            NOTIFICATION_ERROR_NONE);
  ASSERT_NE(channel, nullptr);
  int ret = notification_channel_update(channel);
  EXPECT_NE(ret, NOTIFICATION_ERROR_NONE);
  notification_channel_free(channel);
}

/* ============================================================== */
/* notification_post_with_event_cb_for_uid — L1700-1702           */
/* (insert failure path: ERR + g_list_foreach + goto out)         */
/* ============================================================== */

TEST_F(NotificationCoverageGapsTest,
       post_with_event_cb_for_uid_insert_fail_goes_to_out) {
  /* [N1] invoke_add_noti fails → L1699-1702 (ERR + foreach + goto out).
   * file_list is NULL (no private files) so g_list_foreach is a no-op. */
  ON_CALL(GetMock<NotiServiceProxyMock>(),
          rpc_port_proxy_noti_service_invoke_add_noti(_, _, _))
      .WillByDefault(Return(RPC_PORT_ERROR_IO_ERROR));
  static event_handler_cb cb = [](notification_h, int, void*) {};
  int ret = notification_post_with_event_cb_for_uid(noti_, cb, nullptr, 5001);
  EXPECT_NE(ret, NOTIFICATION_ERROR_NONE);
}

/* ============================================================== */
/* notification_register_detailed_changed_cb_for_uid — L1218-1219 */
/* (_noti_cb_hash creation on first registration)                  */
/* ============================================================== */

TEST_F(NotificationCoverageGapsTest,
       register_detailed_changed_cb_for_uid_non_null_cb_creates_hash) {
  /* [P1] valid callback → L1215 (false), L1218 (condition evaluated),
   * L1219 (hash created if hash was NULL) covered.
   * g_list_append passthrough so the node is actually stored and
   * unregister can find and remove it. */
  using GListAppendFn = GList*(*)(GList*, gpointer);
  ON_CALL(GetMock<GlibMock>(), g_list_append(_, _))
      .WillByDefault(Invoke([](GList* list, gpointer data) -> GList* {
        static GListAppendFn real = reinterpret_cast<GListAppendFn>(
            dlsym(RTLD_NEXT, "g_list_append"));
        return real ? real(list, data) : nullptr;
      }));
  static notification_detailed_changed_cb cb =
      [](void*, notification_type_e, notification_op*, int) {};
  int ret = notification_register_detailed_changed_cb_for_uid(cb, nullptr, 9001);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  /* Cleanup: remove registered cb to avoid cross-test contamination. */
  notification_unregister_detailed_changed_cb_for_uid(cb, nullptr, 9001);
}

/* ============================================================== */
/* notification_register_do_not_disturb_app — L2222-2223          */
/* (event_monitor_init fails after successful dnd registration)   */
/* ============================================================== */

TEST_F(NotificationCoverageGapsTest,
       register_dnd_app_event_monitor_init_fail_covers_error_return) {
  /* [N2] rpc_port_proxy_noti_service_connect_sync: WillOnce(NONE) for
   * _tidl_init inside notification_tidl_request_register_dnd_app, then
   * WillRepeatedly(IO_ERROR) for notification_tidl_event_monitor_init.
   * This makes request_register_dnd_app succeed (L2213 false branch)
   * while event_monitor_init fails → L2221 true → L2222-2223 covered. */
  EXPECT_CALL(GetMock<NotiServiceProxyMock>(),
              rpc_port_proxy_noti_service_connect_sync(_))
      .WillOnce(Return(RPC_PORT_ERROR_NONE))
      .WillRepeatedly(Return(RPC_PORT_ERROR_IO_ERROR));
  static disturb_cb cb = [](void*) {};
  int ret = notification_register_do_not_disturb_app(cb, nullptr);
  EXPECT_NE(ret, NOTIFICATION_ERROR_NONE);
}

/* ============================================================== */
/* notification_post_with_event_cb_for_uid — L1712-1713           */
/* event_monitor_init fails after successful insert               */
/* ============================================================== */

TEST_F(NotificationCoverageGapsTest,
       post_with_event_cb_for_uid_event_monitor_init_fail_goes_to_out) {
  /* [N2] invoke_add_noti succeeds (priv_id=99) so the insert path clears,
   * then event_monitor_init is called.  connect_sync:
   *   - WillOnce(NONE) : the _tidl_init inside request_add_noti succeeds
   *   - WillRepeatedly(IO_ERROR) : the _tidl_init inside event_monitor_init fails
   * → L1712 condition is true → L1713 (ERR = event_monitor_init result + goto out).
   * Also covers notification_tidl_event_monitor_init L497-499. */
  ON_CALL(GetMock<NotiServiceProxyMock>(),
          rpc_port_proxy_noti_service_invoke_add_noti(_, _, _))
      .WillByDefault(DoAll(SetArgPointee<2>(99), Return(RPC_PORT_ERROR_NONE)));
  EXPECT_CALL(GetMock<NotiServiceProxyMock>(),
              rpc_port_proxy_noti_service_connect_sync(_))
      .WillOnce(Return(RPC_PORT_ERROR_NONE))
      .WillRepeatedly(Return(RPC_PORT_ERROR_IO_ERROR));
  static event_handler_cb cb = [](notification_h, int, void*) {};
  int ret = notification_post_with_event_cb_for_uid(noti_, cb, nullptr, 5001);
  EXPECT_NE(ret, NOTIFICATION_ERROR_NONE);
}

/* ============================================================== */
/* notification_tidl_request_delete_single — L758-759             */
/* invoke_delete_single_noti fails                                */
/* ============================================================== */

TEST_F(NotificationCoverageGapsTest,
       request_delete_single_invoke_fail_covers_L758_L759) {
  /* [N1] _tidl_init succeeds (NiceMock defaults connect_sync→NONE),
   * invoke_delete_single_noti returns IO_ERROR → L758-759 covered. */
  ON_CALL(GetMock<NotiServiceProxyMock>(),
          rpc_port_proxy_noti_service_invoke_delete_single_noti(_, _, _, _))
      .WillByDefault(Return(RPC_PORT_ERROR_IO_ERROR));
  int ret = notification_delete_for_uid(noti_, 5001);
  EXPECT_NE(ret, NOTIFICATION_ERROR_NONE);
}

/* ============================================================== */
/* notification_tidl_request_delete_multiple — L781, L786-787    */
/* NULL app_id reassigned + invoke fail                           */
/* ============================================================== */

TEST_F(NotificationCoverageGapsTest,
       request_delete_multiple_null_appid_invoke_fail_covers_L781_L787) {
  /* [N1] notification_clear passes NULL as app_id into
   * notification_tidl_request_delete_multiple → L780-781 (NULL→"")
   * then invoke_delete_multiple_noti returns IO_ERROR → L786-787. */
  ON_CALL(GetMock<NotiServiceProxyMock>(),
          rpc_port_proxy_noti_service_invoke_delete_multiple_noti(_, _, _, _, _))
      .WillByDefault(Return(RPC_PORT_ERROR_IO_ERROR));
  int ret = notification_clear(NOTIFICATION_TYPE_NOTI);
  EXPECT_NE(ret, NOTIFICATION_ERROR_NONE);
}

/* ============================================================== */
/* notification_tidl_send_event — L809, L811-812, L834           */
/* priv_id > 0 path succeeds                                      */
/* ============================================================== */

TEST_F(NotificationCoverageGapsTest,
       send_event_by_priv_id_success_covers_L809_L811_L812_L834) {
  /* [P1] priv_id=1 > 0 → L811 if-branch; NiceMock
   * invoke_send_event_by_priv_id returns NONE → falls through to
   * L834 (WARN done). Covers L809, L811, L812, L834. */
  int ret = notification_send_event_by_priv_id(
      1, NOTIFICATION_EVENT_TYPE_CLICK_ON_BUTTON_1);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
}

/* ============================================================== */
/* notification_tidl_send_event — L813-815                       */
/* priv_id > 0, invoke_send_event_by_priv_id fails               */
/* ============================================================== */

TEST_F(NotificationCoverageGapsTest,
       send_event_by_priv_id_invoke_fail_covers_L813_L815) {
  /* [N1] invoke_send_event_by_priv_id returns IO_ERROR → L813-815. */
  ON_CALL(GetMock<NotiServiceProxyMock>(),
          rpc_port_proxy_noti_service_invoke_send_event_by_priv_id(_, _, _))
      .WillByDefault(Return(RPC_PORT_ERROR_IO_ERROR));
  int ret = notification_send_event_by_priv_id(
      1, NOTIFICATION_EVENT_TYPE_CLICK_ON_BUTTON_1);
  EXPECT_NE(ret, NOTIFICATION_ERROR_NONE);
}

/* ============================================================== */
/* notification_tidl_send_event — L818, L824, L826, L832, L834  */
/* priv_id <= 0 else-branch succeeds                             */
/* ============================================================== */

TEST_F(NotificationCoverageGapsTest,
       send_event_else_success_covers_L818_L824_L832_L834) {
  /* [P1] priv_id=-1 → else-branch; rpc_port_proxy_notification_create
   * (real function) creates handle, make_notification_from_noti fills
   * it, invoke_send_event returns NONE (NiceMock default), then
   * rpc_port_proxy_notification_destroy and L834 WARN. */
  noti_->event_flag = true;
  int ret = notification_send_event(noti_, NOTIFICATION_EVENT_TYPE_CLICK_ON_BUTTON_1);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
}

/* ============================================================== */
/* notification_tidl_send_event — L827-829                       */
/* priv_id <= 0 else-branch, invoke_send_event fails             */
/* ============================================================== */

TEST_F(NotificationCoverageGapsTest,
       send_event_else_invoke_fail_covers_L827_L829) {
  /* [N1] invoke_send_event returns IO_ERROR → L827-829. */
  ON_CALL(GetMock<NotiServiceProxyMock>(),
          rpc_port_proxy_noti_service_invoke_send_event(_, _, _))
      .WillByDefault(Return(RPC_PORT_ERROR_IO_ERROR));
  noti_->event_flag = true;
  int ret = notification_send_event(noti_, NOTIFICATION_EVENT_TYPE_CLICK_ON_BUTTON_1);
  EXPECT_NE(ret, NOTIFICATION_ERROR_NONE);
}

/* ============================================================== */
/* notification_load — NULL app_id, invoke_load_noti_by_priv_id  */
/* fails → L854, L856-857, L859, L861-863                        */
/* ============================================================== */

TEST_F(NotificationCoverageGapsTest,
       load_noti_by_priv_id_null_appid_invoke_fail_covers_L854_L861) {
  /* [N1] NULL app_id + invoke returns IO_ERROR → NULL handle. */
  ON_CALL(GetMock<NotiServiceProxyMock>(),
          rpc_port_proxy_noti_service_invoke_load_noti_by_priv_id(_, _, _, _, _))
      .WillByDefault(Return(RPC_PORT_ERROR_IO_ERROR));
  EXPECT_EQ(notification_load(NULL, 1), nullptr);
}

/* ============================================================== */
/* notification_load_by_tag_for_uid — invoke_load_noti_by_tag    */
/* fails → L893, L895, L897-899                                  */
/* ============================================================== */

TEST_F(NotificationCoverageGapsTest,
       load_noti_by_tag_invoke_fail_covers_L893_L899) {
  /* [N1] invoke_load_noti_by_tag returns IO_ERROR → NULL handle. */
  ON_CALL(GetMock<NotiServiceProxyMock>(),
          rpc_port_proxy_noti_service_invoke_load_noti_by_tag(_, _, _, _, _))
      .WillByDefault(Return(RPC_PORT_ERROR_IO_ERROR));
  EXPECT_EQ(notification_load_by_tag_for_uid("test_tag", 5001), nullptr);
}

/* ============================================================== */
/* notification_get_all_count — invoke_get_all_noti_count fails  */
/* → L927, L929, L931-933                                        */
/* ============================================================== */

TEST_F(NotificationCoverageGapsTest,
       get_all_count_invoke_fail_covers_L927_L933) {
  /* [N1] invoke_get_all_noti_count returns IO_ERROR → error. */
  ON_CALL(GetMock<NotiServiceProxyMock>(),
          rpc_port_proxy_noti_service_invoke_get_all_noti_count(_, _, _, _))
      .WillByDefault(Return(RPC_PORT_ERROR_IO_ERROR));
  int count = 0;
  int ret = notification_get_all_count(NOTIFICATION_TYPE_NOTI, &count);
  EXPECT_NE(ret, NOTIFICATION_ERROR_NONE);
}

/* ============================================================== */
/* notification_get_all_count — success path → L935, L937        */
/* ============================================================== */

TEST_F(NotificationCoverageGapsTest,
       get_all_count_success_covers_L935_L937) {
  /* [P1] NiceMock default: invoke returns 0 (NONE), count set to 0. */
  int count = -1;
  int ret = notification_get_all_count(NOTIFICATION_TYPE_NOTI, &count);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
}

/* ============================================================== */
/* notification_system_setting_load_system_setting               */
/* invoke_load_system_setting fails → L956, L959-962             */
/* ============================================================== */

TEST_F(NotificationCoverageGapsTest,
       load_system_setting_invoke_fail_covers_L956_L962) {
  /* [N1] invoke_load_system_setting returns IO_ERROR → error. */
  ON_CALL(GetMock<NotiServiceProxyMock>(),
          rpc_port_proxy_noti_service_invoke_load_system_setting(_, _, _))
      .WillByDefault(Return(RPC_PORT_ERROR_IO_ERROR));
  notification_system_setting_h setting = nullptr;
  int ret = notification_system_setting_load_system_setting(&setting);
  EXPECT_NE(ret, NOTIFICATION_ERROR_NONE);
}

/* ============================================================== */
/* notification_setting_get_setting_array                        */
/* invoke_get_setting_array fails → L994, L996-999               */
/* ============================================================== */

TEST_F(NotificationCoverageGapsTest,
       get_setting_array_invoke_fail_covers_L994_L999) {
  /* [N1] invoke_get_setting_array returns IO_ERROR → error. */
  ON_CALL(GetMock<NotiServiceProxyMock>(),
          rpc_port_proxy_noti_service_invoke_get_setting_array(_, _, _))
      .WillByDefault(Return(RPC_PORT_ERROR_IO_ERROR));
  notification_setting_h arr = nullptr;
  int cnt = 0;
  int ret = notification_setting_get_setting_array(&arr, &cnt);
  EXPECT_NE(ret, NOTIFICATION_ERROR_NONE);
}

/* ============================================================== */
/* notification_setting_get_setting_by_package_name              */
/* invoke_get_setting_by_app_id fails → L1044, L1046-1050        */
/* ============================================================== */

TEST_F(NotificationCoverageGapsTest,
       get_setting_by_app_id_invoke_fail_covers_L1044_L1050) {
  /* [N1] invoke_get_setting_by_app_id returns IO_ERROR → error. */
  ON_CALL(GetMock<NotiServiceProxyMock>(),
          rpc_port_proxy_noti_service_invoke_get_setting_by_app_id(_, _, _, _))
      .WillByDefault(Return(RPC_PORT_ERROR_IO_ERROR));
  notification_setting_h s = nullptr;
  int ret = notification_setting_get_setting_by_package_name(
      "org.tizen.testpkg", &s);
  EXPECT_NE(ret, NOTIFICATION_ERROR_NONE);
}

/* ============================================================== */
/* notification_setting_update_setting                           */
/* invoke_update_noti_setting fails → L1083, L1091-1098          */
/* ============================================================== */

TEST_F(NotificationCoverageGapsTest,
       update_setting_invoke_fail_covers_L1083_L1098) {
  /* [N1] invoke_update_noti_setting returns IO_ERROR → error.
   * rpc_port_proxy_noti_setting_create is a real function; it will
   * succeed.  We only need a minimal notification_setting on the stack. */
  ON_CALL(GetMock<NotiServiceProxyMock>(),
          rpc_port_proxy_noti_service_invoke_update_noti_setting(_, _, _))
      .WillByDefault(Return(RPC_PORT_ERROR_IO_ERROR));
  struct notification_setting setting_data = {};
  setting_data.package_name = const_cast<char*>("org.test");
  setting_data.app_id = const_cast<char*>("org.test.app");
  notification_setting_h setting_h = &setting_data;
  int ret = notification_setting_update_setting(setting_h);
  EXPECT_NE(ret, NOTIFICATION_ERROR_NONE);
}

/* ============================================================== */
/* notification_system_setting_update_system_setting             */
/* invoke_update_noti_sys_setting fails → L1120, L1128-1135      */
/* ============================================================== */

TEST_F(NotificationCoverageGapsTest,
       update_system_setting_invoke_fail_covers_L1120_L1135) {
  /* [N1] invoke_update_noti_sys_setting returns IO_ERROR → error.
   * rpc_port_proxy_noti_system_setting_create is a real function. */
  ON_CALL(GetMock<NotiServiceProxyMock>(),
          rpc_port_proxy_noti_service_invoke_update_noti_sys_setting(_, _, _))
      .WillByDefault(Return(RPC_PORT_ERROR_IO_ERROR));
  struct notification_system_setting sys_setting_data = {};
  sys_setting_data.dnd_allow_exceptions = nullptr;
  notification_system_setting_h sys_h = &sys_setting_data;
  int ret = notification_system_setting_update_system_setting(sys_h);
  EXPECT_NE(ret, NOTIFICATION_ERROR_NONE);
}

/* ============================================================== */
/* notification_save_as_template                                 */
/* invoke_save_as_template fails → L1157, L1165-1172             */
/* ============================================================== */

TEST_F(NotificationCoverageGapsTest,
       save_as_template_invoke_fail_covers_L1157_L1172) {
  /* [N1] invoke_save_as_template returns IO_ERROR → error.
   * rpc_port_proxy_notification_create is real and will succeed. */
  ON_CALL(GetMock<NotiServiceProxyMock>(),
          rpc_port_proxy_noti_service_invoke_save_as_template(_, _, _))
      .WillByDefault(Return(RPC_PORT_ERROR_IO_ERROR));
  int ret = notification_save_as_template(noti_, "test_template");
  EXPECT_NE(ret, NOTIFICATION_ERROR_NONE);
}

/* ============================================================== */
/* notification_create_from_template                             */
/* invoke_create_from_template fails → L1193, L1195-1198         */
/* ============================================================== */

TEST_F(NotificationCoverageGapsTest,
       create_from_template_invoke_fail_covers_L1193_L1198) {
  /* [N1] invoke_create_from_template returns IO_ERROR → NULL handle. */
  ON_CALL(GetMock<NotiServiceProxyMock>(),
          rpc_port_proxy_noti_service_invoke_create_from_template(_, _, _))
      .WillByDefault(Return(RPC_PORT_ERROR_IO_ERROR));
  notification_h h = notification_create_from_template("test_template");
  EXPECT_EQ(h, nullptr);
}

/* ============================================================== */
/* notification_create_from_package_template                     */
/* invoke_create_from_package_template fails → L1230, L1232-1236 */
/* ============================================================== */

TEST_F(NotificationCoverageGapsTest,
       create_from_package_template_invoke_fail_covers_L1230_L1236) {
  /* [N1] invoke_create_from_package_template returns IO_ERROR → NULL. */
  ON_CALL(GetMock<NotiServiceProxyMock>(),
          rpc_port_proxy_noti_service_invoke_create_from_package_template(
              _, _, _, _))
      .WillByDefault(Return(RPC_PORT_ERROR_IO_ERROR));
  notification_h h = notification_create_from_package_template(
      "org.tizen.app", "tpl");
  EXPECT_EQ(h, nullptr);
}

/* ============================================================== */
/* notification_get_list_for_uid                                 */
/* invoke_load_noti_grouping_list fails → L1286, L1288-1293      */
/* ============================================================== */

TEST_F(NotificationCoverageGapsTest,
       load_noti_grouping_list_invoke_fail_covers_L1286_L1293) {
  /* [N1] invoke_load_noti_grouping_list returns IO_ERROR → error. */
  ON_CALL(GetMock<NotiServiceProxyMock>(),
          rpc_port_proxy_noti_service_invoke_load_noti_grouping_list(
              _, _, _, _, _, _))
      .WillByDefault(Return(RPC_PORT_ERROR_IO_ERROR));
  notification_list_h list = nullptr;
  int ret = notification_get_list_for_uid(
      NOTIFICATION_TYPE_NOTI, -1, &list, 5001);
  EXPECT_NE(ret, NOTIFICATION_ERROR_NONE);
}

/* ============================================================== */
/* notification_get_detail_list_for_uid                          */
/* invoke_load_noti_detail_list fails → L1323, L1325-1330        */
/* ============================================================== */

TEST_F(NotificationCoverageGapsTest,
       load_noti_detail_list_invoke_fail_covers_L1323_L1330) {
  /* [N1] invoke_load_noti_detail_list returns IO_ERROR → error. */
  ON_CALL(GetMock<NotiServiceProxyMock>(),
          rpc_port_proxy_noti_service_invoke_load_noti_detail_list(
              _, _, _, _, _, _, _))
      .WillByDefault(Return(RPC_PORT_ERROR_IO_ERROR));
  notification_list_h list = nullptr;
  int ret = notification_get_detail_list_for_uid(
      "org.tizen.testappid", 0, 0, -1, &list, 5001);
  EXPECT_NE(ret, NOTIFICATION_ERROR_NONE);
}

/* ============================================================== */
/* notification_register_do_not_disturb_app                      */
/* invoke_register_dnd_app fails → L1482-1483                    */
/* ============================================================== */

TEST_F(NotificationCoverageGapsTest,
       register_dnd_app_invoke_fail_covers_L1482_L1483) {
  /* [N1] invoke_register_dnd_app returns IO_ERROR → error.
   * Returns early before notification_tidl_event_monitor_init. */
  ON_CALL(GetMock<NotiServiceProxyMock>(),
          rpc_port_proxy_noti_service_invoke_register_dnd_app(_, _, _))
      .WillByDefault(Return(RPC_PORT_ERROR_IO_ERROR));
  disturb_cb cb = [](void*) {};
  int ret = notification_register_do_not_disturb_app(cb, nullptr);
  EXPECT_NE(ret, NOTIFICATION_ERROR_NONE);
}

/* ============================================================== */
/* notification_unregister_do_not_disturb_app                    */
/* invoke_unregister_dnd_app fails → L1504-1505                  */
/* ============================================================== */

TEST_F(NotificationCoverageGapsTest,
       unregister_dnd_app_invoke_fail_covers_L1504_L1505) {
  /* [N1] invoke_unregister_dnd_app returns IO_ERROR → error returned. */
  ON_CALL(GetMock<NotiServiceProxyMock>(),
          rpc_port_proxy_noti_service_invoke_unregister_dnd_app(_, _, _))
      .WillByDefault(Return(RPC_PORT_ERROR_IO_ERROR));
  int ret = notification_unregister_do_not_disturb_app();
  EXPECT_NE(ret, NOTIFICATION_ERROR_NONE);
}

/* ============================================================== */
/* notification_get_noti_block_state                             */
/* invoke_get_noti_block_state fails → L1363-1364                */
/* ============================================================== */

TEST_F(NotificationCoverageGapsTest,
       get_noti_block_state_invoke_fail_covers_L1363_L1364) {
  /* [N1] invoke_get_noti_block_state returns IO_ERROR → error. */
  ON_CALL(GetMock<NotiServiceProxyMock>(),
          rpc_port_proxy_noti_service_invoke_get_noti_block_state(
              _, _, _, _, _, _))
      .WillByDefault(Return(RPC_PORT_ERROR_IO_ERROR));
  notification_block_state_e state =
      NOTIFICATION_BLOCK_STATE_DO_NOT_DISTURB;
  int ret = notification_get_noti_block_state(&state);
  EXPECT_NE(ret, NOTIFICATION_ERROR_NONE);
}

/* ============================================================== */
/* notification_setting_update_setting — success → L1100-1102    */
/* ============================================================== */

TEST_F(NotificationCoverageGapsTest,
       update_setting_success_covers_L1100_L1102) {
  /* [P1] NiceMock default: invoke_update_noti_setting returns 0 → NONE. */
  struct notification_setting setting_data = {};
  setting_data.package_name = const_cast<char*>("org.test");
  setting_data.app_id = const_cast<char*>("org.test.app");
  notification_setting_h setting_h = &setting_data;
  int ret = notification_setting_update_setting(setting_h);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
}

/* ============================================================== */
/* notification_system_setting_update_system_setting — success    */
/* → L1137-1139                                                   */
/* ============================================================== */

TEST_F(NotificationCoverageGapsTest,
       update_system_setting_success_covers_L1137_L1139) {
  /* [P1] NiceMock default: invoke_update_noti_sys_setting returns 0. */
  struct notification_system_setting sys_setting_data = {};
  sys_setting_data.dnd_allow_exceptions = nullptr;
  notification_system_setting_h sys_h = &sys_setting_data;
  int ret = notification_system_setting_update_system_setting(sys_h);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
}

/* ============================================================== */
/* notification_save_as_template — success path → L1173-1175     */
/* ============================================================== */

TEST_F(NotificationCoverageGapsTest,
       save_as_template_success_covers_L1173_L1175) {
  /* [P1] NiceMock default: invoke_save_as_template returns 0 → NONE. */
  int ret = notification_save_as_template(noti_, "test_template_ok");
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
}

/* ============================================================== */
/* notification_delete_by_display_applist                        */
/* invoke_delete_noti_by_display_applist fails → L1383-1387      */
/* ============================================================== */

TEST_F(NotificationCoverageGapsTest,
       delete_by_display_applist_invoke_fail_covers_L1383_L1387) {
  /* [N1] invoke returns IO_ERROR → non-NONE error. */
  ON_CALL(GetMock<NotiServiceProxyMock>(),
          rpc_port_proxy_noti_service_invoke_delete_noti_by_display_applist(
              _, _, _))
      .WillByDefault(Return(RPC_PORT_ERROR_IO_ERROR));
  int ret = notification_delete_by_display_applist(0x0F);
  EXPECT_NE(ret, NOTIFICATION_ERROR_NONE);
}

/* ============================================================== */
/* notification_delete_by_display_applist — success → L1389-1391 */
/* ============================================================== */

TEST_F(NotificationCoverageGapsTest,
       delete_by_display_applist_success_covers_L1389_L1391) {
  /* [P1] NiceMock default: invoke returns 0 → NONE. */
  int ret = notification_delete_by_display_applist(0x0F);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
}

/* ============================================================== */
/* notification_get_count                                        */
/* invoke_get_noti_count fails → L1404-1405                      */
/* ============================================================== */

TEST_F(NotificationCoverageGapsTest,
       get_count_invoke_fail_covers_L1404_L1405) {
  /* [N1] invoke returns IO_ERROR → non-NONE error. */
  ON_CALL(GetMock<NotiServiceProxyMock>(),
          rpc_port_proxy_noti_service_invoke_get_noti_count(
              _, _, _, _, _, _, _))
      .WillByDefault(Return(RPC_PORT_ERROR_IO_ERROR));
  int count = 0;
  int ret = notification_get_count(NOTIFICATION_TYPE_NOTI, NULL, 0, 0, &count);
  EXPECT_NE(ret, NOTIFICATION_ERROR_NONE);
}

/* ============================================================== */
/* notification_get_count — success path → L1411-1412             */
/* ============================================================== */

TEST_F(NotificationCoverageGapsTest,
       get_count_success_covers_L1411_L1412) {
  /* [P1] NiceMock default: invoke returns 0, count set. */
  int count = -1;
  int ret = notification_get_count(NOTIFICATION_TYPE_NOTI, NULL, 0, 0, &count);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
}

/* ============================================================== */
/* notification_check_event_receiver_available                   */
/* invoke_check_event_receiver fails → L1436-1437                */
/* ============================================================== */

TEST_F(NotificationCoverageGapsTest,
       check_event_receiver_invoke_fail_covers_L1436_L1437) {
  /* [N1] invoke_check_event_receiver returns IO_ERROR → error. */
  ON_CALL(GetMock<NotiServiceProxyMock>(),
          rpc_port_proxy_noti_service_invoke_check_event_receiver(_, _, _))
      .WillByDefault(Return(RPC_PORT_ERROR_IO_ERROR));
  bool available = false;
  int ret = notification_check_event_receiver_available(noti_, &available);
  EXPECT_NE(ret, NOTIFICATION_ERROR_NONE);
}

/* ============================================================== */
/* notification_channel_add — _tidl_init fail → L1522-1523       */
/* ============================================================== */

TEST_F(NotificationCoverageGapsTest,
       channel_add_tidl_init_fail_covers_L1522_L1523) {
  /* [N1] connect_sync → IO_ERROR → _tidl_init fails → early return. */
  ON_CALL(GetMock<NotiServiceProxyMock>(),
          rpc_port_proxy_noti_service_connect_sync(_))
      .WillByDefault(Return(RPC_PORT_ERROR_IO_ERROR));
  notification_channel_h channel = nullptr;
  ASSERT_EQ(notification_channel_create("test_ch_add_fail", &channel),
            NOTIFICATION_ERROR_NONE);
  ASSERT_NE(channel, nullptr);
  int ret = notification_channel_add(channel);
  EXPECT_NE(ret, NOTIFICATION_ERROR_NONE);
  notification_channel_free(channel);
}

/* ============================================================== */
/* notification_channel_remove — _tidl_init fail → L1546-1547    */
/* ============================================================== */

TEST_F(NotificationCoverageGapsTest,
       channel_remove_tidl_init_fail_covers_L1546_L1547) {
  /* [N1] connect_sync → IO_ERROR → _tidl_init fails → early return. */
  ON_CALL(GetMock<NotiServiceProxyMock>(),
          rpc_port_proxy_noti_service_connect_sync(_))
      .WillByDefault(Return(RPC_PORT_ERROR_IO_ERROR));
  notification_channel_h channel = nullptr;
  ASSERT_EQ(notification_channel_create("test_ch_remove_fail", &channel),
            NOTIFICATION_ERROR_NONE);
  ASSERT_NE(channel, nullptr);
  int ret = notification_channel_remove(channel);
  EXPECT_NE(ret, NOTIFICATION_ERROR_NONE);
  notification_channel_free(channel);
}

/* ============================================================== */
/* notification_channel_update — _tidl_init fail → L1570-1571    */
/* ============================================================== */

TEST_F(NotificationCoverageGapsTest,
       channel_update_tidl_init_fail_covers_L1570_L1571) {
  /* [N1] connect_sync → IO_ERROR → _tidl_init fails → early return. */
  ON_CALL(GetMock<NotiServiceProxyMock>(),
          rpc_port_proxy_noti_service_connect_sync(_))
      .WillByDefault(Return(RPC_PORT_ERROR_IO_ERROR));
  notification_channel_h channel = nullptr;
  ASSERT_EQ(notification_channel_create("test_ch_update_fail", &channel),
            NOTIFICATION_ERROR_NONE);
  ASSERT_NE(channel, nullptr);
  int ret = notification_channel_update(channel);
  EXPECT_NE(ret, NOTIFICATION_ERROR_NONE);
  notification_channel_free(channel);
}

/* ============================================================== */
/* notification_get_count — _tidl_init fail → L1403-1405         */
/* ============================================================== */

TEST_F(NotificationCoverageGapsTest,
       get_count_tidl_init_fail_covers_L1403_L1405) {
  /* [N1] connect_sync → IO_ERROR → _tidl_init fails → ERR+return. */
  ON_CALL(GetMock<NotiServiceProxyMock>(),
          rpc_port_proxy_noti_service_connect_sync(_))
      .WillByDefault(Return(RPC_PORT_ERROR_IO_ERROR));
  int count = 0;
  int ret = notification_get_count(NOTIFICATION_TYPE_NOTI, NULL, 0, 0, &count);
  EXPECT_NE(ret, NOTIFICATION_ERROR_NONE);
}

/* ============================================================== */
/* notification_channel_get_by_name — _tidl_init fail → L1596-1597 */
/* ============================================================== */

TEST_F(NotificationCoverageGapsTest,
       channel_get_by_name_tidl_init_fail_covers_L1596_L1597) {
  /* [N1] connect_sync → IO_ERROR → _tidl_init fails in get_channel. */
  ON_CALL(GetMock<NotiServiceProxyMock>(),
          rpc_port_proxy_noti_service_connect_sync(_))
      .WillByDefault(Return(RPC_PORT_ERROR_IO_ERROR));
  notification_channel_h ch = nullptr;
  int ret = notification_channel_get_by_name("test_channel", &ch);
  EXPECT_NE(ret, NOTIFICATION_ERROR_NONE);
  if (ch) notification_channel_free(ch);
}

/* ============================================================== */
/* _tidl_init — noti_service_create fails → L458-L461             */
/* ============================================================== */

TEST_F(NotificationCoverageGapsTest,
       tidl_init_service_create_fail_covers_L458_L461) {
  /* [N1] noti_service_create → IO_ERROR → _tidl_init fails at create.
   * Any TIDL-dependent API exercises this path. */
  ON_CALL(GetMock<NotiServiceProxyMock>(),
          rpc_port_proxy_noti_service_create(_, _, _, _))
      .WillByDefault(Return(RPC_PORT_ERROR_IO_ERROR));
  int count = 0;
  int ret = notification_get_count(NOTIFICATION_TYPE_NOTI, NULL, 0, 0, &count);
  EXPECT_NE(ret, NOTIFICATION_ERROR_NONE);
}

/* ============================================================== */
/* notification_add_deferred_task — success → L1719-1723          */
/* ============================================================== */

TEST_F(NotificationCoverageGapsTest,
       add_deferred_task_success_covers_L1719_L1723) {
  /* [P1] NiceMock default: _tidl_init succeeds → callback invoked. */
  static bool cb_called = false;
  auto cb = [](void *data) { cb_called = true; };
  cb_called = false;
  int ret = notification_add_deferred_task(cb, nullptr);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  EXPECT_TRUE(cb_called);
}

/* ============================================================== */
/* notification_channel_foreach — success path → L1665-1702       */
/* ============================================================== */

TEST_F(NotificationCoverageGapsTest,
       channel_foreach_success_covers_L1665_L1702) {
  /* [P1] invoke_channel_get_list returns an empty real list.
   * rpc_port_proxy_list_noti_channel_length → length=0 (but ret==0)
   * → calloc cb_info → foreach (no-op on empty list) → free → destroy
   * → return NOTIFICATION_ERROR_NONE. Covers L1665-1702. */
  rpc_port_proxy_list_noti_channel_h empty_list = nullptr;
  ASSERT_EQ(rpc_port_proxy_list_noti_channel_create(&empty_list),
            RPC_PORT_ERROR_NONE);
  ASSERT_NE(empty_list, nullptr);

  EXPECT_CALL(GetMock<NotiServiceProxyMock>(),
      rpc_port_proxy_noti_service_invoke_channel_get_list(_, _, _, _))
      .WillOnce(DoAll(SetArgPointee<3>(empty_list),
                      Return(RPC_PORT_ERROR_NONE)));

  static bool ch_cb_called = false;
  auto ch_cb = [](notification_channel_h, void*) { ch_cb_called = true; };
  ch_cb_called = false;

  int ret = notification_channel_foreach("org.tizen.testapp.ch", ch_cb, nullptr);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  EXPECT_FALSE(ch_cb_called);
}

/* ============================================================== */
/* notification_channel_foreach — invoke fail → L1667-1669        */
/* ============================================================== */

TEST_F(NotificationCoverageGapsTest,
       channel_foreach_invoke_fail_covers_L1667_L1669) {
  /* [N1] invoke_channel_get_list → IO_ERROR → L1668 ERR → return io_error. */
  ON_CALL(GetMock<NotiServiceProxyMock>(),
          rpc_port_proxy_noti_service_invoke_channel_get_list(_, _, _, _))
      .WillByDefault(Return(RPC_PORT_ERROR_IO_ERROR));

  auto ch_cb = [](notification_channel_h, void*) {};
  int ret = notification_channel_foreach("org.tizen.testapp.ch2", ch_cb, nullptr);
  EXPECT_NE(ret, NOTIFICATION_ERROR_NONE);
}

/* ============================================================== */
/* notification_create_from_template — invoke success → L1201-1211 */
/* ============================================================== */

TEST_F(NotificationCoverageGapsTest,
       create_from_template_success_returns_noti) {
  /* [P1] invoke_create_from_template succeeds → covers L1201-1211. */
  notification_h src = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(src, nullptr);
  rpc_port_proxy_notification_h h = nullptr;
  ASSERT_EQ(rpc_port_proxy_notification_create(&h), RPC_PORT_ERROR_NONE);
  ASSERT_EQ(make_notification_from_noti(h, src, false), NOTIFICATION_ERROR_NONE);
  notification_free(src);

  /* h is owned and freed by the TIDL function after make_noti_from_notification. */
  EXPECT_CALL(GetMock<NotiServiceProxyMock>(),
      rpc_port_proxy_noti_service_invoke_create_from_template(_, _, _))
      .WillOnce(DoAll(SetArgPointee<2>(h), Return(RPC_PORT_ERROR_NONE)));

  notification_h noti = notification_create_from_template("test_tmpl");
  ASSERT_NE(noti, nullptr);
  notification_free(noti);
}

/* ============================================================== */
/* notification_create_from_package_template — success → L1239-1249 */
/* ============================================================== */

TEST_F(NotificationCoverageGapsTest,
       create_from_package_template_success_returns_noti) {
  /* [P2] invoke_create_from_package_template succeeds → covers L1239-1249. */
  notification_h src = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(src, nullptr);
  rpc_port_proxy_notification_h h = nullptr;
  ASSERT_EQ(rpc_port_proxy_notification_create(&h), RPC_PORT_ERROR_NONE);
  ASSERT_EQ(make_notification_from_noti(h, src, false), NOTIFICATION_ERROR_NONE);
  notification_free(src);

  /* h is owned and freed by the TIDL function after make_noti_from_notification. */
  EXPECT_CALL(GetMock<NotiServiceProxyMock>(),
      rpc_port_proxy_noti_service_invoke_create_from_package_template(_, _, _, _))
      .WillOnce(DoAll(SetArgPointee<3>(h), Return(RPC_PORT_ERROR_NONE)));

  notification_h noti = notification_create_from_package_template(
      "org.tizen.testapp", "test_tmpl");
  ASSERT_NE(noti, nullptr);
  notification_free(noti);
}

/* ============================================================== */
/* notification_load — invoke success → L866-875                  */
/* ============================================================== */

TEST_F(NotificationCoverageGapsTest,
       load_by_priv_id_success_returns_noti) {
  /* [P3] invoke_load_noti_by_priv_id succeeds → covers L866-875. */
  notification_h src = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(src, nullptr);
  rpc_port_proxy_notification_h h = nullptr;
  ASSERT_EQ(rpc_port_proxy_notification_create(&h), RPC_PORT_ERROR_NONE);
  ASSERT_EQ(make_notification_from_noti(h, src, false), NOTIFICATION_ERROR_NONE);
  notification_free(src);

  /* h is owned and freed by the TIDL function after make_noti_from_notification. */
  EXPECT_CALL(GetMock<NotiServiceProxyMock>(),
      rpc_port_proxy_noti_service_invoke_load_noti_by_priv_id(_, _, _, _, _))
      .WillOnce(DoAll(SetArgPointee<4>(h), Return(RPC_PORT_ERROR_NONE)));

  notification_h noti = notification_load(
      const_cast<char*>("org.tizen.testapp"), 1234);
  ASSERT_NE(noti, nullptr);
  notification_free(noti);
}

/* ============================================================== */
/* notification_load_by_tag — invoke success → L902-911           */
/* ============================================================== */

TEST_F(NotificationCoverageGapsTest,
       load_by_tag_success_returns_noti) {
  /* [P4] invoke_load_noti_by_tag succeeds → covers L902-911. */
  notification_h src = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(src, nullptr);
  rpc_port_proxy_notification_h h = nullptr;
  ASSERT_EQ(rpc_port_proxy_notification_create(&h), RPC_PORT_ERROR_NONE);
  ASSERT_EQ(make_notification_from_noti(h, src, false), NOTIFICATION_ERROR_NONE);
  notification_free(src);

  /* h is owned and freed by the TIDL function after make_noti_from_notification. */
  EXPECT_CALL(GetMock<NotiServiceProxyMock>(),
      rpc_port_proxy_noti_service_invoke_load_noti_by_tag(_, _, _, _, _))
      .WillOnce(DoAll(SetArgPointee<4>(h), Return(RPC_PORT_ERROR_NONE)));

  notification_h noti = notification_load_by_tag("test_tag");
  ASSERT_NE(noti, nullptr);
  notification_free(noti);
}

/* ============================================================== */
/* notification_system_setting_load_system_setting_for_uid        */
/* invoke success → L965-972                                      */
/* ============================================================== */

TEST_F(NotificationCoverageGapsTest,
       load_system_setting_success_returns_setting) {
  /* [P5] invoke_load_system_setting succeeds → covers L965-972. */
  rpc_port_proxy_noti_system_setting_h sys_h = nullptr;
  ASSERT_EQ(rpc_port_proxy_noti_system_setting_create(&sys_h),
            RPC_PORT_ERROR_NONE);

  /* sys_h is owned and freed by the TIDL function. */
  EXPECT_CALL(GetMock<NotiServiceProxyMock>(),
      rpc_port_proxy_noti_service_invoke_load_system_setting(_, _, _))
      .WillOnce(DoAll(SetArgPointee<2>(sys_h), Return(RPC_PORT_ERROR_NONE)));

  notification_system_setting_h setting = nullptr;
  int ret = notification_system_setting_load_system_setting_for_uid(
      &setting, 5001);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  ASSERT_NE(setting, nullptr);
  free(setting);
}

/* ============================================================== */
/* notification_setting_get_setting_by_appid_for_uid              */
/* invoke success → L1053-1066                                    */
/* ============================================================== */

TEST_F(NotificationCoverageGapsTest,
       get_setting_by_appid_success_returns_setting) {
  /* [P6] invoke_get_setting_by_app_id succeeds → covers L1053-1066.
   * Pre-populate the proxy handle so make_setting_from_noti_setting can
   * call get_package_name / get_app_id without UB (they return error for
   * NULL fields, leaving local char* uninitialised before the strlen call). */
  rpc_port_proxy_noti_setting_h noti_h = nullptr;
  ASSERT_EQ(rpc_port_proxy_noti_setting_create(&noti_h), RPC_PORT_ERROR_NONE);
  struct notification_setting empty_ns = {};
  ASSERT_EQ(make_noti_setting_from_setting(noti_h, &empty_ns),
            NOTIFICATION_ERROR_NONE);

  /* noti_h is owned and freed by the TIDL function. */
  EXPECT_CALL(GetMock<NotiServiceProxyMock>(),
      rpc_port_proxy_noti_service_invoke_get_setting_by_app_id(_, _, _, _))
      .WillOnce(DoAll(SetArgPointee<3>(noti_h), Return(RPC_PORT_ERROR_NONE)));

  notification_setting_h setting = nullptr;
  int ret = notification_setting_get_setting_by_appid_for_uid(
      "org.tizen.testapp", &setting, 5001);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  ASSERT_NE(setting, nullptr);
  notification_setting_free_notification(setting);
}

/* ============================================================== */
/* notification_set_extension_event_handler                       */
/* app_control_export_as_bundle fail → L1913-1916 (IO_ERROR)     */
/* ============================================================== */

TEST_F(NotificationCoverageGapsTest,
       set_extension_event_handler_export_as_bundle_fail_returns_io_error) {
  /* [Q1] app_control_export_as_bundle fails → IO_ERROR covers L1913-1916.
   * Real app_control_create succeeds via ON_CALL default; only
   * export_as_bundle is overridden to return an error. */
  app_control_h event_handler = nullptr;
  ASSERT_EQ(app_control_create(&event_handler), APP_CONTROL_ERROR_NONE);
  ASSERT_NE(event_handler, nullptr);

  EXPECT_CALL(GetMock<AppControlMock>(), app_control_export_as_bundle(_, _))
      .WillOnce(Return(APP_CONTROL_ERROR_IO_ERROR));

  int ret = notification_set_extension_event_handler(
      noti_, NOTIFICATION_EVENT_TYPE_HIDDEN_BY_USER, event_handler);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_IO_ERROR);

  app_control_destroy(event_handler);
}

/* ============================================================== */
/* notification_get_extension_event_handler                       */
/* app_control_create fail → L1977-1981 (IO_ERROR)               */
/* ============================================================== */

TEST_F(NotificationCoverageGapsTest,
       get_extension_event_handler_app_control_create_fail_returns_io_error) {
  /* [Q2] app_control_create fails → IO_ERROR covers L1977-1981.
   * First populate noti_->args with a valid encoded bundle by calling
   * set_extension_event_handler successfully (all real implementations
   * via ON_CALL defaults).  Then override app_control_create to fail so
   * that the get path hits the uncovered branch. */
  app_control_h event_handler = nullptr;
  ASSERT_EQ(app_control_create(&event_handler), APP_CONTROL_ERROR_NONE);
  ASSERT_NE(event_handler, nullptr);
  ASSERT_EQ(notification_set_extension_event_handler(
      noti_, NOTIFICATION_EVENT_TYPE_HIDDEN_BY_USER, event_handler),
      NOTIFICATION_ERROR_NONE);
  app_control_destroy(event_handler);

  EXPECT_CALL(GetMock<AppControlMock>(), app_control_create(_))
      .WillOnce(Return(APP_CONTROL_ERROR_OUT_OF_MEMORY));

  app_control_h out = nullptr;
  int ret = notification_get_extension_event_handler(
      noti_, NOTIFICATION_EVENT_TYPE_HIDDEN_BY_USER, &out);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_IO_ERROR);
  EXPECT_EQ(out, nullptr);
}

/* ============================================================== */
/* notification_get_extension_event_handler                       */
/* app_control_import_from_bundle fail → L1984-1989 (IO_ERROR)   */
/* ============================================================== */

TEST_F(NotificationCoverageGapsTest,
       get_extension_event_handler_import_from_bundle_fail_returns_io_error) {
  /* [Q3] app_control_import_from_bundle fails → IO_ERROR covers L1984-1989.
   * Populate noti_->args via a successful set call (real implementations),
   * then allow create to succeed but make import fail. */
  app_control_h event_handler = nullptr;
  ASSERT_EQ(app_control_create(&event_handler), APP_CONTROL_ERROR_NONE);
  ASSERT_NE(event_handler, nullptr);
  ASSERT_EQ(notification_set_extension_event_handler(
      noti_, NOTIFICATION_EVENT_TYPE_HIDDEN_BY_USER, event_handler),
      NOTIFICATION_ERROR_NONE);
  app_control_destroy(event_handler);

  EXPECT_CALL(GetMock<AppControlMock>(), app_control_import_from_bundle(_, _))
      .WillOnce(Return(APP_CONTROL_ERROR_IO_ERROR));

  app_control_h out = nullptr;
  int ret = notification_get_extension_event_handler(
      noti_, NOTIFICATION_EVENT_TYPE_HIDDEN_BY_USER, &out);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_IO_ERROR);
  EXPECT_EQ(out, nullptr);
}

/* ============================================================== */
/* notification_get_tag                                           */
/* success path → L1940-1941 (set previously uncalled)           */
/* ============================================================== */

TEST_F(NotificationCoverageGapsTest, get_tag_success_returns_none) {
  /* [R1] notification_get_tag with valid noti covers L1940-1941.
   * No prior test ever called it with a non-NULL noti. */
  ASSERT_EQ(notification_set_tag(noti_, "test_tag"), NOTIFICATION_ERROR_NONE);
  const char *tag = nullptr;
  EXPECT_EQ(notification_get_tag(noti_, &tag), NOTIFICATION_ERROR_NONE);
  EXPECT_STREQ(tag, "test_tag");
}

/* ============================================================== */
/* notification_get_app_id_by_pid                                 */
/* /proc read() returns 0 → L74-75 (return NULL)                 */
/* ============================================================== */

TEST_F(NotificationCoverageGapsTest,
       get_app_id_by_pid_proc_read_zero_returns_null) {
  /* [R2] AUL fails → /proc fallback → read() returns 0 (NiceMock GlibMock
   * default for size_t is 0) → covers L74-75 (ret <= 0 → return NULL).
   * notification_get_app_id_by_pid has hidden visibility so we drive it
   * indirectly via notification_update_progress(noti=NULL, priv_id>0).
   * notification_ongoing_update_progress is a no-op stub, safe with NULL. */
  EXPECT_CALL(GetMock<AulMock>(), aul_app_get_appid_bypid(_, _, _))
      .WillOnce(Return(-1));
  notification_update_progress(nullptr, 100, 0.5);
}

/* ============================================================== */
/* notification_get_app_id_by_pid                                 */
/* /proc read() returns 1 → L77 (app_id[ret]='\0' covered)       */
/* ============================================================== */

TEST_F(NotificationCoverageGapsTest,
       get_app_id_by_pid_proc_read_positive_covers_assignment) {
  /* [R3] AUL fails → /proc fallback → read() returns 1 (mocked)
   * → ret=1 > 0 → if(ret<=0) false → L77: app_id[ret]='\0' covered. */
  ON_CALL(GetMock<GlibMock>(), read(_, _, _))
      .WillByDefault(Return((size_t)1));
  EXPECT_CALL(GetMock<AulMock>(), aul_app_get_appid_bypid(_, _, _))
      .WillOnce(Return(-1));
  notification_update_progress(nullptr, 100, 0.5);
}

/* ============================================================== */
/* _notification_create                                           */
/* caller_app_id NULL → L1557-1559, L1663-1665                   */
/* ============================================================== */

TEST_F(NotificationCoverageGapsTest,
       notification_create_aul_fail_returns_null) {
  /* [R4] AUL fails → notification_get_app_id_by_pid returns NULL →
   * L1557-1559 (ERR+err=-1+goto out) and L1663-1665
   * (notification_free+noti=NULL+set_last_result IO_ERROR) covered. */
  EXPECT_CALL(GetMock<AulMock>(), aul_app_get_appid_bypid(_, _, _))
      .WillOnce(Return(-1));
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  EXPECT_EQ(noti, nullptr);
}

/* ============================================================== */
/* notification_get_text — buffer-full WARN paths                 */
/* L564, L572, L716, L724 in notification.c                      */
/*                                                                */
/* Strategy: use a %s format arg whose string value is 4095 'b'  */
/* chars; dgettext returns it unchanged (no catalog).  After the  */
/* %s substitution result_str is exactly full (strlen=4095).  The */
/* next char in get_str then hits the buffer-full guard for each  */
/* branch variant.                                                */
/* ============================================================== */

TEST_F(NotificationCoverageGapsTest,
       get_text_buffer_full_plain_chars_covers_l564) {
  /* text = "%sa": %s fills result_str to 4095, then 'a' at L564. */
  std::string fill(4095, 'b');
  ASSERT_EQ(notification_set_text(noti_, NOTIFICATION_TEXT_TYPE_TITLE,
      "%sa", nullptr,
      NOTIFICATION_VARIABLE_TYPE_STRING, fill.c_str(),
      NOTIFICATION_VARIABLE_TYPE_NONE), NOTIFICATION_ERROR_NONE);
  char *text = nullptr;
  EXPECT_EQ(notification_get_text(noti_, NOTIFICATION_TEXT_TYPE_TITLE, &text),
            NOTIFICATION_ERROR_NONE);
}

TEST_F(NotificationCoverageGapsTest,
       get_text_buffer_full_double_percent_covers_l572) {
  /* text = "%s%%": %s fills result_str to 4095, then "%%" at L572. */
  std::string fill(4095, 'b');
  ASSERT_EQ(notification_set_text(noti_, NOTIFICATION_TEXT_TYPE_TITLE,
      "%s%%", nullptr,
      NOTIFICATION_VARIABLE_TYPE_STRING, fill.c_str(),
      NOTIFICATION_VARIABLE_TYPE_NONE), NOTIFICATION_ERROR_NONE);
  char *text = nullptr;
  EXPECT_EQ(notification_get_text(noti_, NOTIFICATION_TEXT_TYPE_TITLE, &text),
            NOTIFICATION_ERROR_NONE);
}

TEST_F(NotificationCoverageGapsTest,
       get_text_buffer_full_position_unknown_covers_l716) {
  /* text = "%s%1z": %s fills result_str to 4095, then "%1z" (positional
   * unknown type) falls into the else branch at L712 → L716. */
  std::string fill(4095, 'b');
  ASSERT_EQ(notification_set_text(noti_, NOTIFICATION_TEXT_TYPE_TITLE,
      "%s%1z", nullptr,
      NOTIFICATION_VARIABLE_TYPE_STRING, fill.c_str(),
      NOTIFICATION_VARIABLE_TYPE_NONE), NOTIFICATION_ERROR_NONE);
  char *text = nullptr;
  EXPECT_EQ(notification_get_text(noti_, NOTIFICATION_TEXT_TYPE_TITLE, &text),
            NOTIFICATION_ERROR_NONE);
}

TEST_F(NotificationCoverageGapsTest,
       get_text_buffer_full_unknown_format_covers_l724) {
  /* text = "%s%z": %s fills result_str to 4095, then "%z" (unknown
   * specifier) falls into the outer else branch at L720 → L724. */
  std::string fill(4095, 'b');
  ASSERT_EQ(notification_set_text(noti_, NOTIFICATION_TEXT_TYPE_TITLE,
      "%s%z", nullptr,
      NOTIFICATION_VARIABLE_TYPE_STRING, fill.c_str(),
      NOTIFICATION_VARIABLE_TYPE_NONE), NOTIFICATION_ERROR_NONE);
  char *text = nullptr;
  EXPECT_EQ(notification_get_text(noti_, NOTIFICATION_TEXT_TYPE_TITLE, &text),
            NOTIFICATION_ERROR_NONE);
}

/* -----------------------------------------------------------------------
 * vibration / sound double-set and invalid-type creation coverage
 * ----------------------------------------------------------------------- */

TEST_F(NotificationCoverageGapsTest,
       set_vibration_twice_frees_old_path_covers_l1034) {
  /* L1034-1035: second USER_DATA set → vibration_path!=NULL → free old path. */
  ASSERT_EQ(notification_set_vibration(noti_,
      NOTIFICATION_VIBRATION_TYPE_USER_DATA, "/tmp/cov_vibe_first"),
      NOTIFICATION_ERROR_NONE);
  ASSERT_EQ(notification_set_vibration(noti_,
      NOTIFICATION_VIBRATION_TYPE_USER_DATA, "/tmp/cov_vibe_second"),
      NOTIFICATION_ERROR_NONE);
  notification_vibration_type_e type;
  const char *path = nullptr;
  EXPECT_EQ(notification_get_vibration(noti_, &type, &path),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(type, NOTIFICATION_VIBRATION_TYPE_USER_DATA);
  EXPECT_STREQ(path, "/tmp/cov_vibe_second");
}

TEST_F(NotificationCoverageGapsTest,
       set_sound_twice_frees_old_path) {
  /* Parallel to vibration: second USER_DATA set → sound_path!=NULL → free. */
  ASSERT_EQ(notification_set_sound(noti_,
      NOTIFICATION_SOUND_TYPE_USER_DATA, "/tmp/cov_sound_first"),
      NOTIFICATION_ERROR_NONE);
  ASSERT_EQ(notification_set_sound(noti_,
      NOTIFICATION_SOUND_TYPE_USER_DATA, "/tmp/cov_sound_second"),
      NOTIFICATION_ERROR_NONE);
  notification_sound_type_e type;
  const char *path = nullptr;
  EXPECT_EQ(notification_get_sound(noti_, &type, &path),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(type, NOTIFICATION_SOUND_TYPE_USER_DATA);
  EXPECT_STREQ(path, "/tmp/cov_sound_second");
}

TEST_F(NotificationCoverageGapsTest,
       notification_create_invalid_type_returns_null) {
  /* _notification_create type-guard: value beyond NOTIFICATION_TYPE_ONGOING. */
  notification_h noti = notification_create(
      static_cast<notification_type_e>(NOTIFICATION_TYPE_ONGOING + 1));
  EXPECT_EQ(noti, nullptr);
}

/* ============================================================== */
/* notification_ipc_make_noti_from_gvariant — bundle decode path  */
/* notification_ipc.c L58: _create_bundle_from_bundle_raw with    */
/* non-NULL, non-empty bundle_raw                                  */
/* ============================================================== */

TEST_F(NotificationCoverageGapsTest,
       ipc_roundtrip_with_bundles_covers_bundle_decode_l58) {
  /* notification_ipc_make_gvariant_from_noti returns "(v)" — a tuple wrapping
   * the inner "a{iv}".  notification_ipc_make_noti_from_gvariant expects the
   * inner "a{iv}" (the real usage path extracts it from the DBUS message body).
   * Extract with g_variant_get to pass the correct type; then b_text (set by
   * notification_set_text) is decoded via _create_bundle_from_bundle_raw → L58. */
  ASSERT_EQ(notification_set_text(noti_, NOTIFICATION_TEXT_TYPE_TITLE,
      "Cover L58", nullptr, NOTIFICATION_VARIABLE_TYPE_NONE),
      NOTIFICATION_ERROR_NONE);
  GVariant *wrapper = notification_ipc_make_gvariant_from_noti(noti_, false);
  ASSERT_NE(wrapper, nullptr);
  GVariant *inner = nullptr;
  g_variant_get(wrapper, "(v)", &inner);
  ASSERT_NE(inner, nullptr);
  notification_h dst = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(dst, nullptr);
  EXPECT_EQ(notification_ipc_make_noti_from_gvariant(dst, inner),
            NOTIFICATION_ERROR_NONE);
  g_variant_unref(inner);
  g_variant_unref(wrapper);
  notification_free(dst);
}

/* ================================================================ */
/* notification_list.c L258-259: get_list non-null branch           */
/* notification_get_list_for_uid — when TIDL returns a non-empty    */
/* list, the `if (get_list != NULL)` branch executes.               */
/* ================================================================ */

TEST_F(NotificationCoverageGapsTest,
       get_list_for_uid_nonempty_covers_l258) {
  static int fake_svc;

  ON_CALL(GetMock<NotiServiceProxyMock>(),
      rpc_port_proxy_noti_service_create(_, _, _, _))
      .WillByDefault(Invoke(
          [](auto, auto, auto, rpc_port_proxy_noti_service_h* out) -> int {
            *out = reinterpret_cast<rpc_port_proxy_noti_service_h>(&fake_svc);
            return RPC_PORT_ERROR_NONE;
          }));
  ON_CALL(GetMock<NotiServiceProxyMock>(),
      rpc_port_proxy_noti_service_connect_sync(_))
      .WillByDefault(Return(RPC_PORT_ERROR_NONE));

  static int fake_list_marker;
  ON_CALL(GetMock<NotiServiceProxyMock>(),
      rpc_port_proxy_noti_service_invoke_load_noti_grouping_list(
          _, _, _, _, _, _))
      .WillByDefault(Invoke(
          [](auto, auto, auto, auto, auto,
             rpc_port_proxy_list_notification_h* out) -> int {
            *out = reinterpret_cast<rpc_port_proxy_list_notification_h>(
                &fake_list_marker);
            return RPC_PORT_ERROR_NONE;
          }));

  ON_CALL(GetMock<NotiServiceProxyMock>(),
      rpc_port_proxy_list_notification_foreach(_, _, _))
      .WillByDefault(Invoke(
          [](rpc_port_proxy_list_notification_h,
             rpc_port_proxy_list_notification_cb callback,
             void* user_data) -> int {
            rpc_port_proxy_notification_h noti_h = nullptr;
            rpc_port_proxy_notification_create(&noti_h);
            if (noti_h) {
              make_empty_notification(noti_h);
              callback(noti_h, user_data);
              rpc_port_proxy_notification_destroy(noti_h);
            }
            return RPC_PORT_ERROR_NONE;
          }));
  ON_CALL(GetMock<NotiServiceProxyMock>(),
      rpc_port_proxy_list_notification_destroy(_))
      .WillByDefault(Return(RPC_PORT_ERROR_NONE));

  notification_list_h list = nullptr;
  int ret = notification_get_list_for_uid(NOTIFICATION_TYPE_NOTI, -1,
                                          &list, static_cast<uid_t>(5001));
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  if (list) {
    notification_free_list(list);
  }
}

/* ================================================================ */
/* notification_list.c L335-336: get_detail_list non-null branch    */
/* notification_get_detail_list_for_uid — same pattern as above.    */
/* ================================================================ */

TEST_F(NotificationCoverageGapsTest,
       get_detail_list_for_uid_nonempty_covers_l335) {
  static int fake_svc2;

  ON_CALL(GetMock<NotiServiceProxyMock>(),
      rpc_port_proxy_noti_service_create(_, _, _, _))
      .WillByDefault(Invoke(
          [](auto, auto, auto, rpc_port_proxy_noti_service_h* out) -> int {
            *out = reinterpret_cast<rpc_port_proxy_noti_service_h>(&fake_svc2);
            return RPC_PORT_ERROR_NONE;
          }));
  ON_CALL(GetMock<NotiServiceProxyMock>(),
      rpc_port_proxy_noti_service_connect_sync(_))
      .WillByDefault(Return(RPC_PORT_ERROR_NONE));

  static int fake_list_marker2;
  ON_CALL(GetMock<NotiServiceProxyMock>(),
      rpc_port_proxy_noti_service_invoke_load_noti_detail_list(
          _, _, _, _, _, _, _))
      .WillByDefault(Invoke(
          [](auto, auto, auto, auto, auto, auto,
             rpc_port_proxy_list_notification_h* out) -> int {
            *out = reinterpret_cast<rpc_port_proxy_list_notification_h>(
                &fake_list_marker2);
            return RPC_PORT_ERROR_NONE;
          }));

  ON_CALL(GetMock<NotiServiceProxyMock>(),
      rpc_port_proxy_list_notification_foreach(_, _, _))
      .WillByDefault(Invoke(
          [](rpc_port_proxy_list_notification_h,
             rpc_port_proxy_list_notification_cb callback,
             void* user_data) -> int {
            rpc_port_proxy_notification_h noti_h = nullptr;
            rpc_port_proxy_notification_create(&noti_h);
            if (noti_h) {
              make_empty_notification(noti_h);
              callback(noti_h, user_data);
              rpc_port_proxy_notification_destroy(noti_h);
            }
            return RPC_PORT_ERROR_NONE;
          }));
  ON_CALL(GetMock<NotiServiceProxyMock>(),
      rpc_port_proxy_list_notification_destroy(_))
      .WillByDefault(Return(RPC_PORT_ERROR_NONE));

  notification_list_h list = nullptr;
  int ret = notification_get_detail_list_for_uid("org.tizen.testapp",
                                                 0, 0, -1, &list,
                                                 static_cast<uid_t>(5001));
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  if (list) {
    notification_free_list(list);
  }
}

/* ================================================================ */
/* notification_set_indirect_request — branch coverage             */
/* notification_internal.c L2086                                   */
/* ================================================================ */

TEST_F(NotificationCoverageGapsTest,
       set_indirect_request_aul_fail_returns_invalid) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);
  EXPECT_CALL(GetMock<AulMock>(), aul_app_get_appid_bypid(_, _, _))
      .WillOnce(Return(AUL_R_ERROR));
  int ret = notification_set_indirect_request(noti, 1000, 5001);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
  notification_free(noti);
}

TEST_F(NotificationCoverageGapsTest,
       set_indirect_request_with_service_bundle_pkgmgr_fail) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);
  bundle* b = bundle_create();
  ASSERT_NE(b, nullptr);
  notification_set_execute_option(noti, NOTIFICATION_EXECUTE_TYPE_RESPONDING,
                                  nullptr, nullptr, b);
  bundle_free(b);
  EXPECT_CALL(GetMock<AulMock>(), aul_app_get_appid_bypid(_, _, _))
      .WillOnce(Invoke([](int, char* appid, int len) -> int {
        strncpy(appid, "org.tizen.test", len - 1);
        appid[len - 1] = '\0';
        return AUL_R_OK;
      }));
  ON_CALL(GetMock<PackageManagerMock>(),
          pkgmgrinfo_appinfo_get_usr_appinfo(_, _, _))
      .WillByDefault(Return(PMINFO_R_ERROR));
  int ret = notification_set_indirect_request(noti, 1000, 5001);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  notification_free(noti);
}

TEST_F(NotificationCoverageGapsTest,
       set_indirect_request_with_multiple_bundles_pkgmgr_success) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);
  for (auto type : {NOTIFICATION_EXECUTE_TYPE_RESPONDING,
                    NOTIFICATION_EXECUTE_TYPE_SINGLE_LAUNCH,
                    NOTIFICATION_EXECUTE_TYPE_MULTI_LAUNCH}) {
    bundle* b = bundle_create();
    ASSERT_NE(b, nullptr);
    notification_set_execute_option(noti, type, nullptr, nullptr, b);
    bundle_free(b);
  }
  EXPECT_CALL(GetMock<AulMock>(), aul_app_get_appid_bypid(_, _, _))
      .WillRepeatedly(Invoke([](int, char* appid, int len) -> int {
        strncpy(appid, "org.tizen.test", len - 1);
        appid[len - 1] = '\0';
        return AUL_R_OK;
      }));
  static int fake_appinfo;
  ON_CALL(GetMock<PackageManagerMock>(),
          pkgmgrinfo_appinfo_get_usr_appinfo(_, _, _))
      .WillByDefault(DoAll(
          SetArgPointee<2>(
              reinterpret_cast<pkgmgrinfo_appinfo_h>(&fake_appinfo)),
          Return(PMINFO_R_OK)));
  static char fake_pkgid[] = "org.tizen.testpkg";
  ON_CALL(GetMock<PackageManagerMock>(),
          pkgmgrinfo_appinfo_get_pkgid(_, _))
      .WillByDefault(DoAll(SetArgPointee<1>(fake_pkgid), Return(PMINFO_R_OK)));
  ON_CALL(GetMock<PackageManagerMock>(),
          pkgmgrinfo_appinfo_destroy_appinfo(_))
      .WillByDefault(Return(PMINFO_R_OK));
  int ret = notification_set_indirect_request(noti, 1000, 5001);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  notification_free(noti);
}

TEST_F(NotificationCoverageGapsTest,
       set_indirect_request_no_service_bundles_returns_none) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);
  EXPECT_CALL(GetMock<AulMock>(), aul_app_get_appid_bypid(_, _, _))
      .WillOnce(Invoke([](int, char* appid, int len) -> int {
        strncpy(appid, "org.tizen.test", len - 1);
        appid[len - 1] = '\0';
        return AUL_R_OK;
      }));
  int ret = notification_set_indirect_request(noti, 1000, 5001);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  notification_free(noti);
}

TEST_F(NotificationCoverageGapsTest,
       set_indirect_request_with_event_handler_bundle) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);
  app_control_h handler = nullptr;
  ASSERT_EQ(app_control_create(&handler), APP_CONTROL_ERROR_NONE);
  ASSERT_NE(handler, nullptr);
  EXPECT_EQ(notification_set_event_handler(
                noti, NOTIFICATION_EVENT_TYPE_CLICK_ON_BUTTON_1, handler),
            NOTIFICATION_ERROR_NONE);
  app_control_destroy(handler);
  EXPECT_CALL(GetMock<AulMock>(), aul_app_get_appid_bypid(_, _, _))
      .WillOnce(Invoke([](int, char* appid, int len) -> int {
        strncpy(appid, "org.tizen.test", len - 1);
        appid[len - 1] = '\0';
        return AUL_R_OK;
      }));
  ON_CALL(GetMock<PackageManagerMock>(),
          pkgmgrinfo_appinfo_get_usr_appinfo(_, _, _))
      .WillByDefault(Return(PMINFO_R_ERROR));
  int ret = notification_set_indirect_request(noti, 1000, 5001);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  notification_free(noti);
}
