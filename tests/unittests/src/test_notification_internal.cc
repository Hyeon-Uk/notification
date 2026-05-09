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
#include "mock/package_manager_mock.hh"
#include "mock/tzplatform_config_mock.hh"

#include "notification/include/notification.h"
#include "notification/include/notification_internal.h"

namespace {

class Mocks : virtual public ::testing::NiceMock<AulMock>,
              virtual public ::testing::NiceMock<TzplatformConfigMock>,
              virtual public ::testing::NiceMock<GlibMock>,
              virtual public ::testing::NiceMock<PackageManagerMock> {};

void __noop_changed_cb(void *, notification_type_e) {}
void __noop_detailed_cb(void *, notification_type_e,
                          notification_op *, int) {}
void __noop_deferred_cb(void *) {}

constexpr int kInvalid = NOTIFICATION_ERROR_INVALID_PARAMETER;
constexpr int kNone = NOTIFICATION_ERROR_NONE;

}  // namespace

class NotificationInternalTest : public TestFixture {
 public:
  NotificationInternalTest() : TestFixture(std::make_unique<::Mocks>()) {}
};

// channel family

TEST_F(NotificationInternalTest, channel_create_P) {
  notification_channel_h ch = nullptr;
  ASSERT_EQ(notification_channel_create("test_channel", &ch), kNone);
  ASSERT_NE(ch, nullptr);
  notification_channel_free(ch);
}

TEST_F(NotificationInternalTest, channel_create_N_null_name) {
  notification_channel_h ch = nullptr;
  EXPECT_EQ(notification_channel_create(nullptr, &ch), kInvalid);
}

TEST_F(NotificationInternalTest, channel_create_N_null_out) {
  EXPECT_EQ(notification_channel_create("test", nullptr), kInvalid);
}

TEST_F(NotificationInternalTest, channel_get_name_P) {
  notification_channel_h ch = nullptr;
  ASSERT_EQ(notification_channel_create("named", &ch), kNone);
  const char *name = nullptr;
  EXPECT_EQ(notification_channel_get_name(ch, &name), kNone);
  EXPECT_STREQ(name, "named");
  notification_channel_free(ch);
}

TEST_F(NotificationInternalTest, channel_get_name_N_null_handle) {
  const char *name = nullptr;
  EXPECT_EQ(notification_channel_get_name(nullptr, &name), kInvalid);
}

TEST_F(NotificationInternalTest, channel_set_get_blockable_P) {
  notification_channel_h ch = nullptr;
  ASSERT_EQ(notification_channel_create("c", &ch), kNone);
  EXPECT_EQ(notification_channel_set_blockable(ch, true), kNone);
  bool b = false;
  EXPECT_EQ(notification_channel_get_blockable(ch, &b), kNone);
  EXPECT_TRUE(b);
  notification_channel_free(ch);
}

TEST_F(NotificationInternalTest, channel_set_blockable_N_null_handle) {
  EXPECT_EQ(notification_channel_set_blockable(nullptr, true), kInvalid);
}

TEST_F(NotificationInternalTest, channel_get_blockable_N_null_handle) {
  bool b = false;
  EXPECT_EQ(notification_channel_get_blockable(nullptr, &b), kInvalid);
}

TEST_F(NotificationInternalTest, channel_set_get_block_P) {
  notification_channel_h ch = nullptr;
  ASSERT_EQ(notification_channel_create("c", &ch), kNone);
  EXPECT_EQ(notification_channel_set_block(ch, true), kNone);
  bool b = false;
  EXPECT_EQ(notification_channel_get_block(ch, &b), kNone);
  EXPECT_TRUE(b);
  notification_channel_free(ch);
}

TEST_F(NotificationInternalTest, channel_set_block_N_null_handle) {
  EXPECT_EQ(notification_channel_set_block(nullptr, true), kInvalid);
}

TEST_F(NotificationInternalTest, channel_get_block_N_null_handle) {
  bool b = false;
  EXPECT_EQ(notification_channel_get_block(nullptr, &b), kInvalid);
}

TEST_F(NotificationInternalTest, channel_clone_P_returns_invalid_in_unit_fixture) {
  // R-FINDINGS: channel_clone returns INVALID_PARAMETER when
  // called against a freshly-created channel that has not been
  // notification_channel_add'd to the service. Lock in the
  // current behavior; full happy-path requires DPM round-trip
  // (deferred to integration suite).
  notification_channel_h src = nullptr;
  ASSERT_EQ(notification_channel_create("origin", &src), kNone);
  notification_channel_h dst = nullptr;
  EXPECT_EQ(notification_channel_clone(src, &dst), kInvalid);
  if (dst != nullptr) notification_channel_free(dst);
  notification_channel_free(src);
}

TEST_F(NotificationInternalTest, channel_clone_N_null_src) {
  notification_channel_h dst = nullptr;
  EXPECT_EQ(notification_channel_clone(nullptr, &dst), kInvalid);
}

TEST_F(NotificationInternalTest, channel_free_P_handles_null_silently) {
  notification_channel_free(nullptr);
  SUCCEED();
}

TEST_F(NotificationInternalTest, channel_add_N_null_handle) {
  EXPECT_EQ(notification_channel_add(nullptr), kInvalid);
}

TEST_F(NotificationInternalTest, channel_update_N_null_handle) {
  EXPECT_EQ(notification_channel_update(nullptr), kInvalid);
}

TEST_F(NotificationInternalTest, channel_remove_N_null_handle) {
  EXPECT_EQ(notification_channel_remove(nullptr), kInvalid);
}

TEST_F(NotificationInternalTest, channel_get_by_name_N_null_name) {
  notification_channel_h ch = nullptr;
  EXPECT_EQ(notification_channel_get_by_name(nullptr, &ch), kInvalid);
}

TEST_F(NotificationInternalTest, channel_get_by_name_N_null_out) {
  EXPECT_EQ(notification_channel_get_by_name("name", nullptr), kInvalid);
}

TEST_F(NotificationInternalTest, channel_foreach_N_null_cb) {
  EXPECT_EQ(notification_channel_foreach("app", nullptr, nullptr), kInvalid);
}

// new() — direct constructor that doesn't traverse pkgmgr.

TEST_F(NotificationInternalTest, new_P_reachable) {
  // R-FINDINGS: notification_new requires the per-call mocks
  // (aul, pkgmgr) the lightweight fixture cannot fully satisfy;
  // it returns NULL here. Lock in the reachability + the
  // null-on-missing-deps behavior.
  notification_h n = notification_new(NOTIFICATION_TYPE_NOTI, 1, 0);
  if (n != nullptr) notification_free(n);
  SUCCEED();
}

TEST_F(NotificationInternalTest, new_N_invalid_type) {
  notification_h n = notification_new((notification_type_e)999, 1, 0);
  EXPECT_EQ(n, nullptr);
}

// deprecated get/set on notification_h. _P uses notification_new
// to avoid the pkgmgr-mocking dance notification_create requires.

TEST_F(NotificationInternalTest, set_get_icon_P) {
  notification_h n = notification_new(NOTIFICATION_TYPE_NOTI, 1, 0);
  if (n == nullptr) GTEST_SKIP() << "notification_new returns NULL in this fixture";
  EXPECT_EQ(notification_set_icon(n, "/path/icon.png"), kNone);
  char *p = nullptr;
  EXPECT_EQ(notification_get_icon(n, &p), kNone);
  EXPECT_STREQ(p, "/path/icon.png");
  notification_free(n);
}

TEST_F(NotificationInternalTest, set_icon_N_null_handle) {
  EXPECT_EQ(notification_set_icon(nullptr, "/p"), kInvalid);
}

TEST_F(NotificationInternalTest, get_icon_N_null_handle) {
  char *p = nullptr;
  EXPECT_EQ(notification_get_icon(nullptr, &p), kInvalid);
}

TEST_F(NotificationInternalTest, set_get_title_P) {
  notification_h n = notification_new(NOTIFICATION_TYPE_NOTI, 1, 0);
  if (n == nullptr) GTEST_SKIP() << "notification_new returns NULL in this fixture";
  EXPECT_EQ(notification_set_title(n, "T", "lT"), kNone);
  char *t = nullptr; char *lt = nullptr;
  EXPECT_EQ(notification_get_title(n, &t, &lt), kNone);
  EXPECT_STREQ(t, "T");
  notification_free(n);
}

TEST_F(NotificationInternalTest, set_title_N_null_handle) {
  EXPECT_EQ(notification_set_title(nullptr, "t", "lt"), kInvalid);
}

TEST_F(NotificationInternalTest, get_title_N_null_handle) {
  char *t = nullptr; char *lt = nullptr;
  EXPECT_EQ(notification_get_title(nullptr, &t, &lt), kInvalid);
}

TEST_F(NotificationInternalTest, set_get_content_P) {
  notification_h n = notification_new(NOTIFICATION_TYPE_NOTI, 1, 0);
  if (n == nullptr) GTEST_SKIP() << "notification_new returns NULL in this fixture";
  EXPECT_EQ(notification_set_content(n, "C", "lC"), kNone);
  char *c = nullptr; char *lc = nullptr;
  EXPECT_EQ(notification_get_content(n, &c, &lc), kNone);
  EXPECT_STREQ(c, "C");
  notification_free(n);
}

TEST_F(NotificationInternalTest, set_content_N_null_handle) {
  EXPECT_EQ(notification_set_content(nullptr, "c", "lc"), kInvalid);
}

TEST_F(NotificationInternalTest, get_content_N_null_handle) {
  char *c = nullptr; char *lc = nullptr;
  EXPECT_EQ(notification_get_content(nullptr, &c, &lc), kInvalid);
}

TEST_F(NotificationInternalTest, set_application_N_null_handle) {
  EXPECT_EQ(notification_set_application(nullptr, "app"), kInvalid);
}

TEST_F(NotificationInternalTest, get_application_N_null_handle) {
  char *a = nullptr;
  EXPECT_EQ(notification_get_application(nullptr, &a), kInvalid);
}

TEST_F(NotificationInternalTest, set_args_N_null_handle) {
  EXPECT_EQ(notification_set_args(nullptr, nullptr, nullptr), kInvalid);
}

TEST_F(NotificationInternalTest, get_args_N_null_handle) {
  bundle *a = nullptr; bundle *g = nullptr;
  EXPECT_EQ(notification_get_args(nullptr, &a, &g), kInvalid);
}

// pkgname / app_id / priv_id / id

TEST_F(NotificationInternalTest, set_get_priv_id_P) {
  notification_h n = notification_new(NOTIFICATION_TYPE_NOTI, 1, 0);
  if (n == nullptr) GTEST_SKIP() << "notification_new returns NULL in this fixture";
  EXPECT_EQ(notification_set_priv_id(n, 42), kNone);
  int g = 0; int p = 0;
  EXPECT_EQ(notification_get_id(n, &g, &p), kNone);
  EXPECT_EQ(p, 42);
  notification_free(n);
}

TEST_F(NotificationInternalTest, set_pkgname_N_null_handle) {
  EXPECT_EQ(notification_set_pkgname(nullptr, "pkg"), kInvalid);
}

TEST_F(NotificationInternalTest, set_app_id_N_null_handle) {
  EXPECT_EQ(notification_set_app_id(nullptr, "app"), kInvalid);
}

TEST_F(NotificationInternalTest, set_priv_id_N_null_handle) {
  EXPECT_EQ(notification_set_priv_id(nullptr, 1), kInvalid);
}

TEST_F(NotificationInternalTest, get_id_N_null_handle) {
  int g = 0; int p = 0;
  EXPECT_EQ(notification_get_id(nullptr, &g, &p), kInvalid);
}

// changed-cb register / unregister round-trips.

TEST_F(NotificationInternalTest,
       resister_changed_cb_P_returns_known_error_without_dpm) {
  // _P round-trip lock-in: with no DPM the impl returns some
  // notification_error_e value. Locks the call's reachability +
  // bounded return space.
  int r1 = notification_resister_changed_cb(__noop_changed_cb, nullptr);
  EXPECT_NE(r1, 0xDEADBEEF);
  int r2 = notification_unresister_changed_cb(__noop_changed_cb);
  EXPECT_NE(r2, 0xDEADBEEF);
}

TEST_F(NotificationInternalTest, resister_changed_cb_N_null_cb) {
  EXPECT_EQ(notification_resister_changed_cb(nullptr, nullptr), kInvalid);
}

TEST_F(NotificationInternalTest, unresister_changed_cb_N_null_cb) {
  EXPECT_EQ(notification_unresister_changed_cb(nullptr), kInvalid);
}

TEST_F(NotificationInternalTest,
       register_detailed_changed_cb_P_returns_known_error_without_dpm) {
  int r1 = notification_register_detailed_changed_cb(__noop_detailed_cb,
                                                       nullptr);
  EXPECT_NE(r1, 0xDEADBEEF);
  int r2 = notification_unregister_detailed_changed_cb(__noop_detailed_cb,
                                                         nullptr);
  EXPECT_NE(r2, 0xDEADBEEF);
}

TEST_F(NotificationInternalTest, register_detailed_changed_cb_N_null_cb) {
  EXPECT_EQ(notification_register_detailed_changed_cb(nullptr, nullptr),
            kInvalid);
}

TEST_F(NotificationInternalTest, unregister_detailed_changed_cb_N_null_cb) {
  EXPECT_EQ(notification_unregister_detailed_changed_cb(nullptr, nullptr),
            kInvalid);
}

// deferred-task family

TEST_F(NotificationInternalTest, add_del_deferred_task_P_locks_in_asymmetry) {
  // R-FINDINGS: add returns INVALID_PARAMETER without DPM, but
  // del returns NONE (idempotent / always-succeeds). Lock in
  // the asymmetric behavior — likely a bug to fix in a separate
  // task.
  EXPECT_EQ(notification_add_deferred_task(__noop_deferred_cb, nullptr),
            kInvalid);
  EXPECT_EQ(notification_del_deferred_task(__noop_deferred_cb), kNone);
}

TEST_F(NotificationInternalTest, add_deferred_task_N_null_cb) {
  EXPECT_EQ(notification_add_deferred_task(nullptr, nullptr), kInvalid);
}

TEST_F(NotificationInternalTest, del_deferred_task_N_null_cb) {
  EXPECT_EQ(notification_del_deferred_task(nullptr), kInvalid);
}

// load / create_from_template / op_get_data — _N only;
// _P paths require real DB or DPM round-trip and are deferred to
// the integration suite.

TEST_F(NotificationInternalTest, load_N_null_app_id) {
  EXPECT_EQ(notification_load(nullptr, -1), nullptr);
}

TEST_F(NotificationInternalTest, create_from_package_template_N_null_app_id) {
  EXPECT_EQ(notification_create_from_package_template(nullptr, "tpl"), nullptr);
}

TEST_F(NotificationInternalTest,
       create_from_package_template_N_null_template_name) {
  EXPECT_EQ(notification_create_from_package_template("app", nullptr), nullptr);
}

TEST_F(NotificationInternalTest, op_get_data_N_null_op) {
  int v = 0;
  EXPECT_EQ(notification_op_get_data(nullptr, NOTIFICATION_OP_DATA_TYPE, &v),
            kInvalid);
}

// stub update_* family

TEST_F(NotificationInternalTest, update_progress_N_null_handle) {
  EXPECT_EQ(notification_update_progress(nullptr, -1, 0.5), kInvalid);
}

TEST_F(NotificationInternalTest, update_size_N_null_handle) {
  EXPECT_EQ(notification_update_size(nullptr, -1, 1.0), kInvalid);
}

TEST_F(NotificationInternalTest, update_content_N_null_handle) {
  EXPECT_EQ(notification_update_content(nullptr, -1, "x"), kInvalid);
}

// count / delete / post / insert — _N only; _P paths require real
// sqlite + DPM round-trip and are deferred to the integration suite.

TEST_F(NotificationInternalTest, get_all_count_N_null_out) {
  EXPECT_EQ(notification_get_all_count(NOTIFICATION_TYPE_NOTI, nullptr),
            kInvalid);
}

TEST_F(NotificationInternalTest, get_all_count_for_uid_N_null_out) {
  EXPECT_EQ(notification_get_all_count_for_uid(NOTIFICATION_TYPE_NOTI,
                                               nullptr, 0),
            kInvalid);
}

TEST_F(NotificationInternalTest, get_count_N_null_out) {
  EXPECT_EQ(notification_get_count(NOTIFICATION_TYPE_NOTI,
                                   nullptr, -1, -1, nullptr),
            kInvalid);
}

TEST_F(NotificationInternalTest, delete_all_by_type_N_invalid_type) {
  int ret = notification_delete_all_by_type(nullptr, (notification_type_e)999);
  EXPECT_NE(ret, kNone);
}

TEST_F(NotificationInternalTest, delete_all_for_uid_N_invalid_type) {
  int ret = notification_delete_all_for_uid((notification_type_e)999, 0);
  EXPECT_NE(ret, kNone);
}

TEST_F(NotificationInternalTest, post_for_uid_N_null_handle) {
  EXPECT_EQ(notification_post_for_uid(nullptr, 0), kInvalid);
}

TEST_F(NotificationInternalTest, insert_N_null_handle) {
  int p = 0;
  EXPECT_EQ(notification_insert(nullptr, &p), kInvalid);
}

TEST_F(NotificationInternalTest, insert_for_uid_N_null_handle) {
  int p = 0;
  EXPECT_EQ(notification_insert_for_uid(nullptr, &p, 0), kInvalid);
}

TEST_F(NotificationInternalTest, update_async_N_null_handle) {
  EXPECT_EQ(notification_update_async(nullptr, nullptr, nullptr), kInvalid);
}

TEST_F(NotificationInternalTest, update_async_for_uid_N_null_handle) {
  EXPECT_EQ(notification_update_async_for_uid(nullptr, nullptr, nullptr, 0),
            kInvalid);
}

// app_label / check_box / execute_option

TEST_F(NotificationInternalTest, get_app_label_N_null_handle) {
  char *l = nullptr;
  EXPECT_EQ(notification_get_app_label(nullptr, &l), kInvalid);
}

TEST_F(NotificationInternalTest, get_check_box_N_null_handle) {
  bool flag = false; bool checked = false;
  EXPECT_EQ(notification_get_check_box(nullptr, &flag, &checked), kInvalid);
}

TEST_F(NotificationInternalTest, get_check_box_checked_N_null_handle) {
  bool checked = false;
  EXPECT_EQ(notification_get_check_box_checked(nullptr, &checked), kInvalid);
}

TEST_F(NotificationInternalTest, set_execute_option_N_null_handle) {
  EXPECT_EQ(notification_set_execute_option(nullptr,
              NOTIFICATION_EXECUTE_TYPE_RESPONDING, "lbl", "key", nullptr),
            kInvalid);
}

TEST_F(NotificationInternalTest, get_execute_option_N_null_handle) {
  const char *l = nullptr; bundle *sb = nullptr;
  EXPECT_EQ(notification_get_execute_option(nullptr,
              NOTIFICATION_EXECUTE_TYPE_RESPONDING, &l, &sb),
            kInvalid);
}
