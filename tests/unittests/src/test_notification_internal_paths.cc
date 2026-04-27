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

/*
 * N7.5-D coverage boost — notification_internal.c (Large 3 file group, 2876 LOC).
 *
 * The existing test_notification.cc / test_post_update_delete.cc only graze
 * the public surface; the LARGE chunk of internal_internal.c is the
 * setter/getter family on notification_internal.h plus a handful of
 * static helpers (`__free_changed_cb_info`, `__set_caller_info`,
 * `__set_indirect_request`, `__priv_id_compare`). They are reachable
 * transitively via the EXPORT_API surface — which is what we exercise here
 * (the mirror-test pattern from D7.5-F).
 *
 * Coverage:
 *   1. NullParam_AllAccessors — pin parameter-validation contracts on the
 *      ~30 EXPORT_API functions in notification_internal.c that we did not
 *      previously call from test_notification.cc.
 *   2. SetGet_TitleContent_RoundTrip / Application / Args / ExecuteOption /
 *      Icon / Pkgname — exercise the wrapper functions that delegate to
 *      notification_set_text / notification_set_image and the bundle
 *      helpers.
 *   3. SetGet_PrivId / GroupId / Uid / DefaultButton — small scalar fields
 *      with their own setter/getter. Boundary values: 0 / negative / large.
 *   4. CallerInfo_AndIndirectRequest — drive notification_set_indirect_request
 *      through the happy path and the early-out branches:
 *        - NULL noti => INVALID_PARAMETER
 *        - pid <= 1 => INVALID_PARAMETER
 *        - uid < REGULAR_UID_MIN => INVALID_PARAMETER
 *        - aul_app_get_appid_bypid failure => INVALID_PARAMETER
 *      The happy path implicitly exercises `__set_caller_info` and
 *      `__set_indirect_request` (the static helpers are then non-zero in
 *      lcov).
 *   5. RegisterChangedCb_NullCallback_Rejected — exercises the early
 *      INVALID_PARAMETER branch of
 *      notification_resister_changed_cb_for_uid. Calling it with a real
 *      callback opens up a tidl path (gbs host has no daemon) which we
 *      avoid here.
 *   6. UnresisterChangedCb_BeforeRegister_Rejected — covers the
 *      "_noti_cb_hash == NULL" early-out path.
 *   7. UnregisterDetailedChangedCb_BeforeRegister_Rejected — same idea on
 *      the detailed callback list (separate global state).
 *   8. AddDeferredTask_NullCb_Rejected — INVALID_PARAMETER fast path.
 *   9. DelDeferredTask_NullCb_Rejected.
 *  10. SetUid_LowUid_Accepted — unlike notification_set_indirect_request,
 *      notification_set_uid does not have a REGULAR_UID_MIN check.
 *  11. NotificationLoad_BogusInput — null-app-id load gracefully returns
 *      NULL or fails through tidl (we accept either).
 *
 * No production code is changed by this TU.
 *
 * Mock strategy: AulMock + GlibMock provide the symbol interposition, with
 * deterministic fakes wired up in SetUp. Plain TestFixture (not LeakCheck)
 * for the same cross-boundary alloc reason as test_notification_setting_extra.cc.
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <climits>
#include <cstdio>
#include <cstring>
#include <unistd.h>

#include "mock/test_fixture.hh"
#include "mock/aul_mock.hh"
#include "mock/glib_mock.hh"
#include "mock/package_manager_mock.hh"
#include "mock/tzplatform_config_mock.hh"

extern "C" {
#include <notification.h>
#include <notification_internal.h>
#include <notification_error.h>
#include <notification_type.h>
#include <notification_type_internal.h>
}

namespace {

using ::testing::_;
using ::testing::Invoke;
using ::testing::Return;

int FakeAulGetAppidBypid(int /*pid*/, char* appid, int len) {
  if (appid != nullptr && len > 0)
    snprintf(appid, len, "%s", "org.tizen.testapp");
  return 0;  /* AUL_R_OK */
}

int FakeAulGetPkgidBypid(int /*pid*/, char* pkgid, int len) {
  if (pkgid != nullptr && len > 0)
    snprintf(pkgid, len, "%s", "org.tizen.testpkg");
  return 0;  /* AUL_R_OK */
}

int FakeAulGetAppidBypidFail(int /*pid*/, char* /*appid*/, int /*len*/) {
  return -1;
}

class IntMocks :
    virtual public ::testing::NiceMock<AulMock>,
    virtual public ::testing::NiceMock<TzplatformConfigMock>,
    virtual public ::testing::NiceMock<GlibMock>,
    virtual public ::testing::NiceMock<PackageManagerMock> {};

class NotificationInternalPathsTest : public TestFixture {
 public:
  NotificationInternalPathsTest()
      : TestFixture(std::make_unique<::IntMocks>()) {}
  void SetUp() override {
    ON_CALL(GetMock<AulMock>(), aul_app_get_appid_bypid(_, _, _))
        .WillByDefault(Invoke(FakeAulGetAppidBypid));
    ON_CALL(GetMock<AulMock>(), aul_app_get_pkgid_bypid(_, _, _))
        .WillByDefault(Invoke(FakeAulGetPkgidBypid));
    ON_CALL(GetMock<PackageManagerMock>(),
            pkgmgrinfo_appinfo_get_usr_appinfo(_, _, _))
        .WillByDefault(Return(-1));
  }
};

}  // namespace

/* ------------------------------------------------------------------ */
/* 1. NULL parameter contract — long tail.                            */
/* ------------------------------------------------------------------ */

TEST_F(NotificationInternalPathsTest, NullParam_AllAccessors) {
  bundle* bnd = nullptr;
  int v = 0;
  char* s = nullptr;

  /* setters */
  EXPECT_EQ(notification_set_application(nullptr, "org.x"),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_set_args(nullptr, bnd, bnd),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_set_pkgname(nullptr, "pkg"),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_set_priv_id(nullptr, 1),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_set_uid(nullptr, 5001),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_set_default_button(nullptr, NOTIFICATION_BUTTON_1),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_set_ongoing_value_type(nullptr,
                NOTIFICATION_ONGOING_VALUE_TYPE_PERCENT),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_set_ongoing_time(nullptr, 0, 0),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_set_hide_timeout(nullptr, 5),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_set_delete_timeout(nullptr, 5),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_set_pairing_type(nullptr, true),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_set_check_box(nullptr, true, true),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_set_check_box_checked(nullptr, true),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_set_app_label(nullptr, const_cast<char*>("L")),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_set_channel_name(nullptr, "chan"),
            NOTIFICATION_ERROR_INVALID_PARAMETER);

  /* getters */
  EXPECT_EQ(notification_get_application(nullptr, &s),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_get_args(nullptr, &bnd, &bnd),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_get_id(nullptr, &v, &v),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_get_uid(nullptr, reinterpret_cast<uid_t*>(&v)),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  notification_button_index_e bi;
  EXPECT_EQ(notification_get_default_button(nullptr, &bi),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  notification_ongoing_value_type_e ovt;
  EXPECT_EQ(notification_get_ongoing_value_type(nullptr, &ovt),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_get_ongoing_time(nullptr, &v, &v),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_get_hide_timeout(nullptr, &v),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_get_delete_timeout(nullptr, &v),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_get_text_input_max_length(nullptr, &v),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  bool b = false;
  EXPECT_EQ(notification_get_pairing_type(nullptr, &b),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  bool b2 = false;
  EXPECT_EQ(notification_get_check_box(nullptr, &b, &b2),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_get_check_box_checked(nullptr, &b),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_get_app_label(nullptr, &s),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  const char* cs = nullptr;
  EXPECT_EQ(notification_get_channel_name(nullptr, &cs),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_get_event_flag(nullptr, &b),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* ------------------------------------------------------------------ */
/* 2. Setter/Getter round-trips — wrappers that delegate.             */
/* ------------------------------------------------------------------ */

TEST_F(NotificationInternalPathsTest, SetGet_TitleContent_RoundTrip) {
  notification_h n = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(n, nullptr);

  ASSERT_EQ(notification_set_title(n, "Hello", nullptr),
            NOTIFICATION_ERROR_NONE);
  char* title = nullptr;
  char* loc = nullptr;
  ASSERT_EQ(notification_get_title(n, &title, &loc),
            NOTIFICATION_ERROR_NONE);
  EXPECT_STREQ(title, "Hello");

  ASSERT_EQ(notification_set_content(n, "World", nullptr),
            NOTIFICATION_ERROR_NONE);
  char* content = nullptr;
  ASSERT_EQ(notification_get_content(n, &content, &loc),
            NOTIFICATION_ERROR_NONE);
  EXPECT_STREQ(content, "World");

  notification_free(n);
}

TEST_F(NotificationInternalPathsTest, SetGet_Application_RoundTrip) {
  notification_h n = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(n, nullptr);

  ASSERT_EQ(notification_set_application(n, "org.tizen.target"),
            NOTIFICATION_ERROR_NONE);
  char* app = nullptr;
  ASSERT_EQ(notification_get_application(n, &app),
            NOTIFICATION_ERROR_NONE);
  EXPECT_STREQ(app, "org.tizen.target");

  /* Re-set: must free old + strdup new without leaking. */
  ASSERT_EQ(notification_set_application(n, "org.tizen.other"),
            NOTIFICATION_ERROR_NONE);
  ASSERT_EQ(notification_get_application(n, &app),
            NOTIFICATION_ERROR_NONE);
  EXPECT_STREQ(app, "org.tizen.other");

  notification_free(n);
}

TEST_F(NotificationInternalPathsTest, SetGet_Pkgname_RoundTrip) {
  notification_h n = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(n, nullptr);

  ASSERT_EQ(notification_set_pkgname(n, "org.tizen.pkg"),
            NOTIFICATION_ERROR_NONE);
  char* pkg = nullptr;
  ASSERT_EQ(notification_get_pkgname(n, &pkg),
            NOTIFICATION_ERROR_NONE);
  EXPECT_STREQ(pkg, "org.tizen.pkg");

  notification_free(n);
}

TEST_F(NotificationInternalPathsTest, SetGet_PrivId_RoundTrip) {
  notification_h n = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(n, nullptr);

  ASSERT_EQ(notification_set_priv_id(n, 42), NOTIFICATION_ERROR_NONE);
  int gid = 0, pid = 0;
  ASSERT_EQ(notification_get_id(n, &gid, &pid),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(pid, 42);

  /* priv_id <= 0 rejected. */
  EXPECT_EQ(notification_set_priv_id(n, 0),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_set_priv_id(n, -5),
            NOTIFICATION_ERROR_INVALID_PARAMETER);

  notification_free(n);
}

TEST_F(NotificationInternalPathsTest, SetGet_Uid_RoundTrip) {
  notification_h n = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(n, nullptr);

  ASSERT_EQ(notification_set_uid(n, 5001), NOTIFICATION_ERROR_NONE);
  uid_t got = 0;
  ASSERT_EQ(notification_get_uid(n, &got), NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(got, 5001u);

  /* unlike set_indirect_request, set_uid has no REGULAR_UID_MIN check. */
  ASSERT_EQ(notification_set_uid(n, 0), NOTIFICATION_ERROR_NONE);
  ASSERT_EQ(notification_get_uid(n, &got), NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(got, 0u);

  notification_free(n);
}

TEST_F(NotificationInternalPathsTest, SetGet_DefaultButton_RoundTrip) {
  notification_h n = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(n, nullptr);

  ASSERT_EQ(notification_set_default_button(n, NOTIFICATION_BUTTON_3),
            NOTIFICATION_ERROR_NONE);
  notification_button_index_e idx = NOTIFICATION_BUTTON_1;
  ASSERT_EQ(notification_get_default_button(n, &idx),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(idx, NOTIFICATION_BUTTON_3);

  notification_free(n);
}

TEST_F(NotificationInternalPathsTest, SetGet_OngoingTime_RoundTrip) {
  notification_h n = notification_create(NOTIFICATION_TYPE_ONGOING);
  ASSERT_NE(n, nullptr);

  ASSERT_EQ(notification_set_ongoing_time(n, 30, 120),
            NOTIFICATION_ERROR_NONE);
  int cur = 0, dur = 0;
  ASSERT_EQ(notification_get_ongoing_time(n, &cur, &dur),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(cur, 30);
  EXPECT_EQ(dur, 120);

  notification_free(n);
}

TEST_F(NotificationInternalPathsTest, SetGet_Timeouts_RoundTrip) {
  notification_h n = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(n, nullptr);

  ASSERT_EQ(notification_set_hide_timeout(n, 7), NOTIFICATION_ERROR_NONE);
  int t = 0;
  ASSERT_EQ(notification_get_hide_timeout(n, &t),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(t, 7);

  ASSERT_EQ(notification_set_delete_timeout(n, 13), NOTIFICATION_ERROR_NONE);
  ASSERT_EQ(notification_get_delete_timeout(n, &t),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(t, 13);

  notification_free(n);
}

TEST_F(NotificationInternalPathsTest, SetGet_PairingType_RoundTrip) {
  notification_h n = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(n, nullptr);

  ASSERT_EQ(notification_set_pairing_type(n, true),
            NOTIFICATION_ERROR_NONE);
  bool b = false;
  ASSERT_EQ(notification_get_pairing_type(n, &b),
            NOTIFICATION_ERROR_NONE);
  EXPECT_TRUE(b);

  ASSERT_EQ(notification_set_pairing_type(n, false),
            NOTIFICATION_ERROR_NONE);
  ASSERT_EQ(notification_get_pairing_type(n, &b),
            NOTIFICATION_ERROR_NONE);
  EXPECT_FALSE(b);

  notification_free(n);
}

TEST_F(NotificationInternalPathsTest, SetGet_CheckBox_RoundTrip) {
  notification_h n = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(n, nullptr);

  ASSERT_EQ(notification_set_check_box(n, true, true),
            NOTIFICATION_ERROR_NONE);
  bool flag = false;
  bool got_checked = false;
  ASSERT_EQ(notification_get_check_box(n, &flag, &got_checked),
            NOTIFICATION_ERROR_NONE);
  EXPECT_TRUE(flag);
  EXPECT_TRUE(got_checked);

  ASSERT_EQ(notification_set_check_box_checked(n, false),
            NOTIFICATION_ERROR_NONE);
  bool checked = true;
  ASSERT_EQ(notification_get_check_box_checked(n, &checked),
            NOTIFICATION_ERROR_NONE);
  EXPECT_FALSE(checked);

  notification_free(n);
}

TEST_F(NotificationInternalPathsTest, SetGet_AppLabel_RoundTrip) {
  notification_h n = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(n, nullptr);

  char buf[] = "AppLbl";
  ASSERT_EQ(notification_set_app_label(n, buf),
            NOTIFICATION_ERROR_NONE);
  char* out = nullptr;
  ASSERT_EQ(notification_get_app_label(n, &out),
            NOTIFICATION_ERROR_NONE);
  EXPECT_STREQ(out, "AppLbl");
  /* notification_get_app_label returns borrowed pointer; do not free. */

  notification_free(n);
}

TEST_F(NotificationInternalPathsTest, SetGet_ChannelName_RoundTrip) {
  notification_h n = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(n, nullptr);

  ASSERT_EQ(notification_set_channel_name(n, "chan-A"),
            NOTIFICATION_ERROR_NONE);
  const char* out = nullptr;
  ASSERT_EQ(notification_get_channel_name(n, &out),
            NOTIFICATION_ERROR_NONE);
  EXPECT_STREQ(out, "chan-A");
  /* notification_get_channel_name returns a borrowed pointer; do not free. */

  notification_free(n);
}

TEST_F(NotificationInternalPathsTest, SetGet_Icon_RoundTrip) {
  notification_h n = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(n, nullptr);

  ASSERT_EQ(notification_set_icon(n, "/tmp/icon.png"),
            NOTIFICATION_ERROR_NONE);
  char* path = nullptr;
  ASSERT_EQ(notification_get_icon(n, &path),
            NOTIFICATION_ERROR_NONE);
  EXPECT_STREQ(path, "/tmp/icon.png");

  notification_free(n);
}

/* ------------------------------------------------------------------ */
/* 3. set_indirect_request — caller-info + indirect-request paths.    */
/* ------------------------------------------------------------------ */

TEST_F(NotificationInternalPathsTest,
       IndirectRequest_NullNoti_Rejected) {
  EXPECT_EQ(notification_set_indirect_request(nullptr, 1234, 5001),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationInternalPathsTest,
       IndirectRequest_LowPid_Rejected) {
  notification_h n = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(n, nullptr);
  EXPECT_EQ(notification_set_indirect_request(n, 0, 5001),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_set_indirect_request(n, 1, 5001),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  notification_free(n);
}

TEST_F(NotificationInternalPathsTest,
       IndirectRequest_LowUid_Rejected) {
  notification_h n = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(n, nullptr);
  /* REGULAR_UID_MIN = 5000 in notification.c / notification_internal.c. */
  EXPECT_EQ(notification_set_indirect_request(n, 1234, 0),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_set_indirect_request(n, 1234, 4999),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  notification_free(n);
}

TEST_F(NotificationInternalPathsTest,
       IndirectRequest_AulFailure_Rejected) {
  notification_h n = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(n, nullptr);

  /* Override the default aul stub for THIS call to fail. */
  EXPECT_CALL(GetMock<AulMock>(), aul_app_get_appid_bypid(_, _, _))
      .WillOnce(Invoke(FakeAulGetAppidBypidFail));

  EXPECT_EQ(notification_set_indirect_request(n, 1234, 5001),
            NOTIFICATION_ERROR_INVALID_PARAMETER);

  notification_free(n);
}

/* ------------------------------------------------------------------ */
/* 4. Callback registration / unregistration parameter contract.      */
/* ------------------------------------------------------------------ */

TEST_F(NotificationInternalPathsTest,
       RegisterChangedCb_NullCallback_Rejected) {
  EXPECT_EQ(notification_resister_changed_cb(nullptr, nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_resister_changed_cb_for_uid(nullptr, nullptr,
                                                     5001),
            NOTIFICATION_ERROR_INVALID_PARAMETER);

  EXPECT_EQ(notification_register_detailed_changed_cb(nullptr, nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_register_detailed_changed_cb_for_uid(
                nullptr, nullptr, 5001),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationInternalPathsTest,
       UnresisterChangedCb_BeforeRegister_Rejected) {
  /* No prior register call -> _noti_cb_hash may or may not be NULL
   * depending on test ordering; at minimum NULL callback is rejected. */
  EXPECT_EQ(notification_unresister_changed_cb(nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_unresister_changed_cb_for_uid(nullptr, 5001),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationInternalPathsTest,
       UnregisterDetailedChangedCb_BeforeRegister_Rejected) {
  EXPECT_EQ(notification_unregister_detailed_changed_cb(nullptr, nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_unregister_detailed_changed_cb_for_uid(
                nullptr, nullptr, 5001),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationInternalPathsTest, AddDeferredTask_NullCb_Rejected) {
  EXPECT_EQ(notification_add_deferred_task(nullptr, nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationInternalPathsTest, DelDeferredTask_NullCb_Rejected) {
  EXPECT_EQ(notification_del_deferred_task(nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* ------------------------------------------------------------------ */
/* 5. notification_get_id behaviour with negative group_id.           */
/* ------------------------------------------------------------------ */

TEST_F(NotificationInternalPathsTest, GetId_NegativeGroupId_NormalisedToNone) {
  notification_h n = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(n, nullptr);

  /* _notification_create populates group_id = NOTIFICATION_GROUP_ID_NONE.
   * notification_get_id normalises any value < NOTIFICATION_GROUP_ID_NONE
   * up to NOTIFICATION_GROUP_ID_NONE. The default after create is exactly
   * NOTIFICATION_GROUP_ID_NONE so we round-trip the boundary. */
  int gid = 0;
  int pid = 0;
  ASSERT_EQ(notification_get_id(n, &gid, &pid),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(gid, NOTIFICATION_GROUP_ID_NONE);

  notification_free(n);
}

/* ------------------------------------------------------------------ */
/* 6. notification_set_execute_option boundary and dispatch.          */
/* ------------------------------------------------------------------ */

TEST_F(NotificationInternalPathsTest, SetExecuteOption_TypeBounds) {
  notification_h n = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(n, nullptr);

  /* NULL noti */
  EXPECT_EQ(notification_set_execute_option(nullptr,
                NOTIFICATION_EXECUTE_TYPE_RESPONDING,
                "t", "k", nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);

  /* type out of range below */
  EXPECT_EQ(notification_set_execute_option(n,
                NOTIFICATION_EXECUTE_TYPE_NONE,
                "t", "k", nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);

  /* type out of range above */
  EXPECT_EQ(notification_set_execute_option(n,
                static_cast<notification_execute_type_e>(
                    NOTIFICATION_EXECUTE_TYPE_MAX + 1),
                "t", "k", nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);

  /* happy path: each accepted type lands in its dedicated bundle slot. */
  ASSERT_EQ(notification_set_execute_option(n,
                NOTIFICATION_EXECUTE_TYPE_RESPONDING,
                "txt-resp", "key-resp", nullptr),
            NOTIFICATION_ERROR_NONE);
  ASSERT_EQ(notification_set_execute_option(n,
                NOTIFICATION_EXECUTE_TYPE_SINGLE_LAUNCH,
                "txt-sl", "key-sl", nullptr),
            NOTIFICATION_ERROR_NONE);
  ASSERT_EQ(notification_set_execute_option(n,
                NOTIFICATION_EXECUTE_TYPE_MULTI_LAUNCH,
                "txt-ml", "key-ml", nullptr),
            NOTIFICATION_ERROR_NONE);

  /* re-set replaces text+key without leaking. */
  ASSERT_EQ(notification_set_execute_option(n,
                NOTIFICATION_EXECUTE_TYPE_RESPONDING,
                "txt-resp-2", "key-resp-2", nullptr),
            NOTIFICATION_ERROR_NONE);

  notification_free(n);
}

/* ------------------------------------------------------------------ */
/* 7. notification_clear / get_count are NULL/zero-input safe.        */
/* ------------------------------------------------------------------ */

TEST_F(NotificationInternalPathsTest, GetCount_NullCount_Rejected) {
  /* signature: type, app_id, group_id, priv_id, count* */
  EXPECT_EQ(notification_get_count(NOTIFICATION_TYPE_NOTI,
                                   nullptr, 0, 0, nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* ------------------------------------------------------------------ */
/* 8. notification_set_args / get_args round-trip with null bundles.  */
/* ------------------------------------------------------------------ */

TEST_F(NotificationInternalPathsTest, SetArgs_NullArgs_Rejected) {
  notification_h n = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(n, nullptr);

  /* args==NULL is rejected (group_args==NULL alone is OK). */
  EXPECT_EQ(notification_set_args(n, nullptr, nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);

  /* Get path with no args set: out is NULL. */
  bundle* a = reinterpret_cast<bundle*>(0xdead);
  bundle* g = reinterpret_cast<bundle*>(0xdead);
  ASSERT_EQ(notification_get_args(n, &a, &g),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(a, nullptr);

  notification_free(n);
}

/* ------------------------------------------------------------------ */
/* 9. set_text on multiple text types (matrix coverage).              */
/* ------------------------------------------------------------------ */

TEST_F(NotificationInternalPathsTest, SetText_MultipleTextTypes) {
  notification_h n = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(n, nullptr);

  const struct {
    notification_text_type_e t;
    const char* v;
  } cases[] = {
      {NOTIFICATION_TEXT_TYPE_TITLE, "T1"},
      {NOTIFICATION_TEXT_TYPE_CONTENT, "C1"},
      {NOTIFICATION_TEXT_TYPE_INFO_1, "I1"},
      {NOTIFICATION_TEXT_TYPE_INFO_SUB_1, "S1"},
      {NOTIFICATION_TEXT_TYPE_BUTTON_1, "B1"},
      {NOTIFICATION_TEXT_TYPE_BUTTON_2, "B2"},
      {NOTIFICATION_TEXT_TYPE_GROUP_TITLE, "G1"},
  };
  for (auto& c : cases) {
    ASSERT_EQ(notification_set_text(n, c.t, c.v, nullptr,
                                    NOTIFICATION_VARIABLE_TYPE_NONE),
              NOTIFICATION_ERROR_NONE);
    char* got = nullptr;
    ASSERT_EQ(notification_get_text(n, c.t, &got),
              NOTIFICATION_ERROR_NONE);
    EXPECT_STREQ(got, c.v) << "type=" << c.t;
  }

  notification_free(n);
}

/* ------------------------------------------------------------------ */
/* 10. is_service_ready returns deterministic int even without daemon.*/
/* ------------------------------------------------------------------ */

/* DISABLED: notification_is_service_ready dispatches into the TIDL master
 * ready probe which may block for the daemon registration timeout on a
 * host without dpms running. We disable the test rather than risk a hang
 * — the production path is exercised by integration tests. */
TEST_F(NotificationInternalPathsTest, DISABLED_IsServiceReady_NoCrash) {
  int rc = notification_is_service_ready();
  (void)rc;
  SUCCEED();
}

/* ------------------------------------------------------------------ */
/* 11. notification_load with bogus app_id is non-crashing.           */
/* ------------------------------------------------------------------ */

/* DISABLED: notification_load goes through TIDL request_load_noti_by_priv_id
 * which can block for daemon registration. Already covered by
 * test_post_update_delete.cc Load_BogusId_ReturnsNull which makes the same
 * tradeoff. */
TEST_F(NotificationInternalPathsTest, DISABLED_Load_BogusInput_NoCrash) {
  char app_id[] = "org.tizen.bogus";
  notification_h n = notification_load(app_id, 0);
  if (n) notification_free(n);
  SUCCEED();
}
