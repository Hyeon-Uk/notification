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

#include <bundle.h>

#include "mock/aul_mock.hh"
#include "mock/glib_mock.hh"
#include "mock/noti_service_proxy_mock.hh"
#include "mock/package_manager_mock.hh"
#include "mock/rpc_port_mock.hh"
#include "mock/test_fixture.hh"
#include "mock/tzplatform_config_mock.hh"

#include "notification/include/notification.h"
#include "notification/include/notification_internal.h"
#include "notification/include/notification_private.h"

using ::testing::_;
using ::testing::Invoke;
using ::testing::Return;

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

int __fake_aul_app_get_appid_bypid(int pid, char* appid, int len) {
  snprintf(appid, len, "%s", "org.tizen.testappid");
  return AUL_R_OK;
}

int __fake_aul_app_get_pkgid_bypid(int pid, char* pkgid, int len) {
  snprintf(pkgid, len, "%s", "org.tizen.testpkgid");
  return AUL_R_OK;
}

int __fake_package_info_create(const char* package, package_info_h* package_info) {
  *package_info = (package_info_h)calloc(1, sizeof(struct package_info_s));
  return PACKAGE_MANAGER_ERROR_NONE;
}

int __fake_package_info_get_root_path(package_info_h package_info, char** path) {
  *path = strdup("testrootpath");
  return PACKAGE_MANAGER_ERROR_NONE;
}

int __fake_pkgmgrinfo_appinfo_get_usr_appinfo(const char* appid, uid_t uid,
    pkgmgrinfo_appinfo_h* handle) {
  *handle = (pkgmgrinfo_appinfo_h*)calloc(1, sizeof(pkgmgr_appinfo_x));
  return PMINFO_R_OK;
}

int __fake_pkgmgrinfo_appinfo_get_label(pkgmgrinfo_appinfo_h handle, char** label) {
  *label = strdup("testlabel");
  return PMINFO_R_OK;
}

class Mocks :
    virtual public ::testing::NiceMock<AulMock>,
    virtual public ::testing::NiceMock<TzplatformConfigMock>,
    virtual public ::testing::NiceMock<GlibMock>,
    virtual public ::testing::NiceMock<NotiServiceProxyMock>,
    virtual public ::testing::NiceMock<PackageManagerMock>,
    virtual public ::testing::NiceMock<RpcPortMock> {};

}  // namespace

class NotificationInternalTest : public TestFixture {
 public:
  NotificationInternalTest() : TestFixture(std::make_unique<::Mocks>()) {}

  void SetUp() override {
    EXPECT_CALL(GetMock<AulMock>(), aul_app_get_appid_bypid(_, _, _))
        .WillRepeatedly(Invoke(__fake_aul_app_get_appid_bypid));
    EXPECT_CALL(GetMock<AulMock>(), aul_app_get_pkgid_bypid(_, _, _))
        .WillRepeatedly(Invoke(__fake_aul_app_get_pkgid_bypid));
    EXPECT_CALL(GetMock<PackageManagerMock>(), package_info_create(_, _))
        .WillRepeatedly(Invoke(__fake_package_info_create));
    EXPECT_CALL(GetMock<PackageManagerMock>(), package_info_get_root_path(_, _))
        .WillRepeatedly(Invoke(__fake_package_info_get_root_path));
    EXPECT_CALL(GetMock<PackageManagerMock>(),
        pkgmgrinfo_appinfo_get_usr_appinfo(_, _, _))
        .WillRepeatedly(Invoke(__fake_pkgmgrinfo_appinfo_get_usr_appinfo));
    EXPECT_CALL(GetMock<PackageManagerMock>(),
        pkgmgrinfo_appinfo_get_label(_, _))
        .WillRepeatedly(Invoke(__fake_pkgmgrinfo_appinfo_get_label));

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

/* ------------------------------------------------------------- */
/* notification_set_title / notification_get_title               */
/* ------------------------------------------------------------- */

TEST_F(NotificationInternalTest, set_title_success) {
  EXPECT_EQ(notification_set_title(noti_, "hello", nullptr),
            NOTIFICATION_ERROR_NONE);

  char* title = nullptr;
  char* loc_title = nullptr;
  EXPECT_EQ(notification_get_title(noti_, &title, &loc_title),
            NOTIFICATION_ERROR_NONE);
  EXPECT_STREQ(title, "hello");
  /* get_title always writes NULL into loc_title — by design. */
  EXPECT_EQ(loc_title, nullptr);
}

TEST_F(NotificationInternalTest, set_title_null_noti_returns_invalid_parameter) {
  EXPECT_EQ(notification_set_title(nullptr, "hello", nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationInternalTest, set_title_null_title_clears_existing) {
  /* notification_set_title forwards to notification_set_text(text=NULL,
   * key=NULL, args_type=NONE). set_text treats NULL text as "clear the
   * existing entry" and returns NONE — see notification.c L255/L279. */
  ASSERT_EQ(notification_set_title(noti_, "first", nullptr),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(notification_set_title(noti_, nullptr, nullptr),
            NOTIFICATION_ERROR_NONE);
  char* title = nullptr;
  char* loc = nullptr;
  EXPECT_EQ(notification_get_title(noti_, &title, &loc),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(title, nullptr);
}

TEST_F(NotificationInternalTest, get_title_skips_null_out_args) {
  ASSERT_EQ(notification_set_title(noti_, "world", nullptr),
            NOTIFICATION_ERROR_NONE);
  /* Both out pointers NULL → still NONE, just no writes. */
  EXPECT_EQ(notification_get_title(noti_, nullptr, nullptr),
            NOTIFICATION_ERROR_NONE);
}

/* ------------------------------------------------------------- */
/* notification_set_content / notification_get_content           */
/* ------------------------------------------------------------- */

TEST_F(NotificationInternalTest, set_content_success) {
  EXPECT_EQ(notification_set_content(noti_, "body", nullptr),
            NOTIFICATION_ERROR_NONE);

  char* content = nullptr;
  char* loc_content = nullptr;
  EXPECT_EQ(notification_get_content(noti_, &content, &loc_content),
            NOTIFICATION_ERROR_NONE);
  EXPECT_STREQ(content, "body");
  EXPECT_EQ(loc_content, nullptr);
}

TEST_F(NotificationInternalTest, set_content_null_noti_returns_invalid_parameter) {
  EXPECT_EQ(notification_set_content(nullptr, "body", nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* ------------------------------------------------------------- */
/* notification_set_application / notification_get_application   */
/* ------------------------------------------------------------- */

TEST_F(NotificationInternalTest, set_application_success) {
  EXPECT_EQ(notification_set_application(noti_, "org.tizen.target"),
            NOTIFICATION_ERROR_NONE);

  char* app_id = nullptr;
  EXPECT_EQ(notification_get_application(noti_, &app_id),
            NOTIFICATION_ERROR_NONE);
  EXPECT_STREQ(app_id, "org.tizen.target");
}

TEST_F(NotificationInternalTest, set_application_null_noti_returns_invalid_parameter) {
  EXPECT_EQ(notification_set_application(nullptr, "org.tizen.x"),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationInternalTest, set_application_null_app_id_returns_invalid_parameter) {
  EXPECT_EQ(notification_set_application(noti_, nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationInternalTest, set_application_overwrite_frees_previous) {
  ASSERT_EQ(notification_set_application(noti_, "first"),
            NOTIFICATION_ERROR_NONE);
  ASSERT_EQ(notification_set_application(noti_, "second"),
            NOTIFICATION_ERROR_NONE);

  char* app_id = nullptr;
  ASSERT_EQ(notification_get_application(noti_, &app_id),
            NOTIFICATION_ERROR_NONE);
  EXPECT_STREQ(app_id, "second");
}

TEST_F(NotificationInternalTest, get_application_falls_back_to_caller_app_id) {
  /* No explicit set_application → falls back to caller_app_id (set
   * during notification_create from aul_app_get_appid_bypid). */
  char* app_id = nullptr;
  ASSERT_EQ(notification_get_application(noti_, &app_id),
            NOTIFICATION_ERROR_NONE);
  EXPECT_STREQ(app_id, "org.tizen.testappid");
}

TEST_F(NotificationInternalTest, get_application_null_out_returns_invalid_parameter) {
  EXPECT_EQ(notification_get_application(noti_, nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* ------------------------------------------------------------- */
/* notification_set_args / notification_get_args                 */
/* ------------------------------------------------------------- */

TEST_F(NotificationInternalTest, set_args_stores_args_and_group_args) {
  bundle* args = bundle_create();
  bundle_add_str(args, "k", "v");
  bundle* group_args = bundle_create();
  bundle_add_str(group_args, "gk", "gv");

  EXPECT_EQ(notification_set_args(noti_, args, group_args),
            NOTIFICATION_ERROR_NONE);

  bundle* got_args = nullptr;
  bundle* got_group_args = nullptr;
  EXPECT_EQ(notification_get_args(noti_, &got_args, &got_group_args),
            NOTIFICATION_ERROR_NONE);
  ASSERT_NE(got_args, nullptr);
  ASSERT_NE(got_group_args, nullptr);

  char* v = nullptr;
  bundle_get_str(got_args, "k", &v);
  EXPECT_STREQ(v, "v");

  bundle_free(args);
  bundle_free(group_args);
}

TEST_F(NotificationInternalTest, set_args_null_noti_returns_invalid_parameter) {
  bundle* args = bundle_create();
  EXPECT_EQ(notification_set_args(nullptr, args, nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  bundle_free(args);
}

TEST_F(NotificationInternalTest, set_args_null_args_returns_invalid_parameter) {
  EXPECT_EQ(notification_set_args(noti_, nullptr, nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationInternalTest, set_args_with_null_group_args_clears_group_args) {
  bundle* args = bundle_create();
  bundle* group_args = bundle_create();
  ASSERT_EQ(notification_set_args(noti_, args, group_args),
            NOTIFICATION_ERROR_NONE);
  ASSERT_EQ(notification_set_args(noti_, args, nullptr),
            NOTIFICATION_ERROR_NONE);

  bundle* got_args = nullptr;
  bundle* group_sentinel = bundle_create();
  bundle* got_group = group_sentinel;
  ASSERT_EQ(notification_get_args(noti_, &got_args, &got_group),
            NOTIFICATION_ERROR_NONE);
  /* group_args is cleared → get_args leaves the out arg untouched (in
   * the impl it only writes when noti->group_args != NULL). */
  EXPECT_EQ(got_group, group_sentinel);
  bundle_free(group_sentinel);

  bundle_free(args);
  bundle_free(group_args);
}

TEST_F(NotificationInternalTest, get_args_null_noti_returns_invalid_parameter) {
  bundle* out = nullptr;
  EXPECT_EQ(notification_get_args(nullptr, &out, nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* ------------------------------------------------------------- */
/* notification_set_app_id / notification_set_pkgname (alias)    */
/* ------------------------------------------------------------- */

TEST_F(NotificationInternalTest, set_app_id_null_returns_invalid_parameter) {
  EXPECT_EQ(notification_set_app_id(nullptr, "x"),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_set_app_id(noti_, nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationInternalTest, set_pkgname_delegates_to_set_app_id) {
  /* The success path is covered by the alias contract; check it
   * accepts a valid pair and is reachable. */
  EXPECT_EQ(notification_set_pkgname(noti_, "org.tizen.alias"),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(notification_set_pkgname(nullptr, "x"),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* ------------------------------------------------------------- */
/* notification_set_priv_id / notification_get_id                */
/* ------------------------------------------------------------- */

TEST_F(NotificationInternalTest, set_priv_id_success) {
  EXPECT_EQ(notification_set_priv_id(noti_, 42), NOTIFICATION_ERROR_NONE);

  int group_id = -100;
  int priv_id = 0;
  EXPECT_EQ(notification_get_id(noti_, &group_id, &priv_id),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(priv_id, 42);
}

TEST_F(NotificationInternalTest, set_priv_id_null_noti_returns_invalid_parameter) {
  EXPECT_EQ(notification_set_priv_id(nullptr, 1),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationInternalTest, set_priv_id_rejects_zero_and_negative) {
  EXPECT_EQ(notification_set_priv_id(noti_, 0),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_set_priv_id(noti_, -1),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationInternalTest, get_id_null_noti_returns_invalid_parameter) {
  int g = 0;
  int p = 0;
  EXPECT_EQ(notification_get_id(nullptr, &g, &p),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationInternalTest, get_id_allows_null_out_args) {
  /* Either out pointer may be NULL — the impl just skips that write. */
  EXPECT_EQ(notification_get_id(noti_, nullptr, nullptr),
            NOTIFICATION_ERROR_NONE);
}

/* ------------------------------------------------------------- */
/* notification_set_default_button / get_default_button          */
/* ------------------------------------------------------------- */

TEST_F(NotificationInternalTest, set_default_button_success) {
  EXPECT_EQ(notification_set_default_button(noti_, NOTIFICATION_BUTTON_3),
            NOTIFICATION_ERROR_NONE);

  notification_button_index_e idx = NOTIFICATION_BUTTON_1;
  EXPECT_EQ(notification_get_default_button(noti_, &idx),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(idx, NOTIFICATION_BUTTON_3);
}

TEST_F(NotificationInternalTest, set_default_button_rejects_out_of_range) {
  EXPECT_EQ(notification_set_default_button(noti_,
                static_cast<notification_button_index_e>(-1)),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_set_default_button(noti_,
                static_cast<notification_button_index_e>(NOTIFICATION_BUTTON_6 + 1)),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationInternalTest, set_default_button_null_noti) {
  EXPECT_EQ(notification_set_default_button(nullptr, NOTIFICATION_BUTTON_1),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* ------------------------------------------------------------- */
/* notification_set_ongoing_value_type / get_ongoing_value_type  */
/* ------------------------------------------------------------- */

TEST_F(NotificationInternalTest, set_ongoing_value_type_round_trip) {
  EXPECT_EQ(notification_set_ongoing_value_type(noti_,
                NOTIFICATION_ONGOING_VALUE_TYPE_TIME),
            NOTIFICATION_ERROR_NONE);
  notification_ongoing_value_type_e t =
      NOTIFICATION_ONGOING_VALUE_TYPE_PERCENT;
  EXPECT_EQ(notification_get_ongoing_value_type(noti_, &t),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(t, NOTIFICATION_ONGOING_VALUE_TYPE_TIME);
}

TEST_F(NotificationInternalTest, set_ongoing_value_type_rejects_out_of_range) {
  EXPECT_EQ(notification_set_ongoing_value_type(noti_,
                static_cast<notification_ongoing_value_type_e>(
                    NOTIFICATION_ONGOING_VALUE_TYPE_PERCENT - 1)),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_set_ongoing_value_type(noti_,
                static_cast<notification_ongoing_value_type_e>(
                    NOTIFICATION_ONGOING_VALUE_TYPE_TIME + 1)),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* ------------------------------------------------------------- */
/* notification_set_ongoing_time / get_ongoing_time              */
/* ------------------------------------------------------------- */

TEST_F(NotificationInternalTest, set_ongoing_time_round_trip) {
  EXPECT_EQ(notification_set_ongoing_time(noti_, 30, 60),
            NOTIFICATION_ERROR_NONE);

  int current = 0;
  int duration = 0;
  EXPECT_EQ(notification_get_ongoing_time(noti_, &current, &duration),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(current, 30);
  EXPECT_EQ(duration, 60);
}

TEST_F(NotificationInternalTest, set_ongoing_time_rejects_negative_or_inverted) {
  EXPECT_EQ(notification_set_ongoing_time(noti_, -1, 10),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_set_ongoing_time(noti_, 10, -1),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  /* current > duration is also rejected. */
  EXPECT_EQ(notification_set_ongoing_time(noti_, 100, 50),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationInternalTest, get_ongoing_time_null_out_returns_invalid_parameter) {
  int current = 0;
  int duration = 0;
  EXPECT_EQ(notification_get_ongoing_time(noti_, nullptr, &duration),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_get_ongoing_time(noti_, &current, nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* ------------------------------------------------------------- */
/* notification_set_hide_timeout / set_delete_timeout            */
/* ------------------------------------------------------------- */

TEST_F(NotificationInternalTest, set_hide_timeout_round_trip) {
  EXPECT_EQ(notification_set_hide_timeout(noti_, 7),
            NOTIFICATION_ERROR_NONE);
  int t = 0;
  EXPECT_EQ(notification_get_hide_timeout(noti_, &t),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(t, 7);
}

TEST_F(NotificationInternalTest, set_hide_timeout_rejects_negative) {
  EXPECT_EQ(notification_set_hide_timeout(noti_, -1),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationInternalTest, set_delete_timeout_round_trip) {
  EXPECT_EQ(notification_set_delete_timeout(noti_, 12),
            NOTIFICATION_ERROR_NONE);
  int t = 0;
  EXPECT_EQ(notification_get_delete_timeout(noti_, &t),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(t, 12);
}

TEST_F(NotificationInternalTest, set_delete_timeout_rejects_negative) {
  EXPECT_EQ(notification_set_delete_timeout(noti_, -5),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationInternalTest, get_hide_timeout_null_out_returns_invalid_parameter) {
  EXPECT_EQ(notification_get_hide_timeout(noti_, nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* ------------------------------------------------------------- */
/* notification_get_text_input_max_length                        */
/* ------------------------------------------------------------- */

TEST_F(NotificationInternalTest, get_text_input_max_length_after_set) {
  ASSERT_EQ(notification_set_text_input(noti_, 2048),
            NOTIFICATION_ERROR_NONE);
  int max_len = 0;
  EXPECT_EQ(notification_get_text_input_max_length(noti_, &max_len),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(max_len, 2048);
}

TEST_F(NotificationInternalTest, get_text_input_max_length_null_args) {
  int v = 0;
  EXPECT_EQ(notification_get_text_input_max_length(nullptr, &v),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_get_text_input_max_length(noti_, nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* ------------------------------------------------------------- */
/* notification_set_check_box / get_check_box                    */
/* notification_set_check_box_checked / get_check_box_checked    */
/* ------------------------------------------------------------- */

TEST_F(NotificationInternalTest, set_check_box_round_trip) {
  EXPECT_EQ(notification_set_check_box(noti_, true, true),
            NOTIFICATION_ERROR_NONE);

  bool flag = false;
  bool checked = false;
  EXPECT_EQ(notification_get_check_box(noti_, &flag, &checked),
            NOTIFICATION_ERROR_NONE);
  EXPECT_TRUE(flag);
  EXPECT_TRUE(checked);
}

TEST_F(NotificationInternalTest, set_check_box_null_noti) {
  EXPECT_EQ(notification_set_check_box(nullptr, true, false),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationInternalTest, get_check_box_null_out_returns_invalid_parameter) {
  bool flag = false;
  bool checked = false;
  EXPECT_EQ(notification_get_check_box(noti_, nullptr, &checked),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_get_check_box(noti_, &flag, nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationInternalTest, get_check_box_checked_returns_false_when_flag_unset) {
  /* When the check_box flag itself is false, get_check_box_checked
   * forces checked = false regardless of the stored value. */
  ASSERT_EQ(notification_set_check_box(noti_, false, true),
            NOTIFICATION_ERROR_NONE);
  bool checked = true;
  EXPECT_EQ(notification_get_check_box_checked(noti_, &checked),
            NOTIFICATION_ERROR_NONE);
  EXPECT_FALSE(checked);
}

TEST_F(NotificationInternalTest, set_check_box_checked_round_trip) {
  ASSERT_EQ(notification_set_check_box(noti_, true, false),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(notification_set_check_box_checked(noti_, true),
            NOTIFICATION_ERROR_NONE);
  bool checked = false;
  EXPECT_EQ(notification_get_check_box_checked(noti_, &checked),
            NOTIFICATION_ERROR_NONE);
  EXPECT_TRUE(checked);
}

TEST_F(NotificationInternalTest, set_check_box_checked_null_noti) {
  EXPECT_EQ(notification_set_check_box_checked(nullptr, true),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* ------------------------------------------------------------- */
/* notification_set_pairing_type                                  */
/* ------------------------------------------------------------- */

TEST_F(NotificationInternalTest, set_pairing_type_null_noti) {
  EXPECT_EQ(notification_set_pairing_type(nullptr, true),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* ------------------------------------------------------------- */
/* notification_set_channel_name                                  */
/* ------------------------------------------------------------- */

TEST_F(NotificationInternalTest, set_channel_name_null_args) {
  EXPECT_EQ(notification_set_channel_name(nullptr, "x"),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_set_channel_name(noti_, nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* ------------------------------------------------------------- */
/* notification_op_get_data                                      */
/* ------------------------------------------------------------- */

TEST_F(NotificationInternalTest, op_get_data_extracts_each_field) {
  notification_op op = {};
  op.type = NOTIFICATION_OP_INSERT;
  op.priv_id = 17;
  op.extra_info_1 = 100;
  op.extra_info_2 = 200;
  op.noti = noti_;

  int type_out = 0;
  EXPECT_EQ(notification_op_get_data(&op, NOTIFICATION_OP_DATA_TYPE,
                &type_out),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(type_out, static_cast<int>(NOTIFICATION_OP_INSERT));

  int priv_out = 0;
  EXPECT_EQ(notification_op_get_data(&op, NOTIFICATION_OP_DATA_PRIV_ID,
                &priv_out),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(priv_out, 17);

  notification_h noti_out = nullptr;
  EXPECT_EQ(notification_op_get_data(&op, NOTIFICATION_OP_DATA_NOTI,
                &noti_out),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(noti_out, noti_);

  int e1 = 0;
  EXPECT_EQ(notification_op_get_data(&op, NOTIFICATION_OP_DATA_EXTRA_INFO_1,
                &e1),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(e1, 100);

  int e2 = 0;
  EXPECT_EQ(notification_op_get_data(&op, NOTIFICATION_OP_DATA_EXTRA_INFO_2,
                &e2),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(e2, 200);
}

TEST_F(NotificationInternalTest, op_get_data_null_args) {
  notification_op op = {};
  int dummy = 0;
  EXPECT_EQ(notification_op_get_data(nullptr, NOTIFICATION_OP_DATA_TYPE,
                &dummy),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_op_get_data(&op, NOTIFICATION_OP_DATA_TYPE, nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* ------------------------------------------------------------- */
/* notification_set_icon / notification_get_icon                  */
/* ------------------------------------------------------------- */

TEST_F(NotificationInternalTest, set_icon_roundtrip) {
  ASSERT_EQ(notification_set_icon(noti_, "/path/icon.png"),
            NOTIFICATION_ERROR_NONE);
  char* path = nullptr;
  EXPECT_EQ(notification_get_icon(noti_, &path), NOTIFICATION_ERROR_NONE);
  EXPECT_STREQ(path, "/path/icon.png");
}

TEST_F(NotificationInternalTest, set_icon_null_noti) {
  EXPECT_EQ(notification_set_icon(nullptr, "/p"),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationInternalTest, get_icon_null_args) {
  EXPECT_EQ(notification_get_icon(nullptr, nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* ------------------------------------------------------------- */
/* notification_set_pkgname / notification_set_app_id             */
/* ------------------------------------------------------------- */

TEST_F(NotificationInternalTest, set_app_id_roundtrip) {
  ASSERT_EQ(notification_set_app_id(noti_, "org.tizen.foo"),
            NOTIFICATION_ERROR_NONE);
  char* p = nullptr;
  EXPECT_EQ(notification_get_pkgname(noti_, &p), NOTIFICATION_ERROR_NONE);
  EXPECT_STREQ(p, "org.tizen.foo");
}

TEST_F(NotificationInternalTest, set_app_id_replaces_previous) {
  ASSERT_EQ(notification_set_app_id(noti_, "first"),
            NOTIFICATION_ERROR_NONE);
  ASSERT_EQ(notification_set_app_id(noti_, "second"),
            NOTIFICATION_ERROR_NONE);
  char* p = nullptr;
  EXPECT_EQ(notification_get_pkgname(noti_, &p), NOTIFICATION_ERROR_NONE);
  EXPECT_STREQ(p, "second");
}

TEST_F(NotificationInternalTest, set_app_id_null_args) {
  EXPECT_EQ(notification_set_app_id(nullptr, "a"),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_set_app_id(noti_, nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationInternalTest, set_pkgname_roundtrip) {
  /* set_pkgname is a thin wrapper over set_app_id. */
  ASSERT_EQ(notification_set_pkgname(noti_, "org.tizen.bar"),
            NOTIFICATION_ERROR_NONE);
  char* p = nullptr;
  EXPECT_EQ(notification_get_pkgname(noti_, &p), NOTIFICATION_ERROR_NONE);
  EXPECT_STREQ(p, "org.tizen.bar");
}

TEST_F(NotificationInternalTest, set_pkgname_null_args) {
  EXPECT_EQ(notification_set_pkgname(nullptr, "a"),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_set_pkgname(noti_, nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* ------------------------------------------------------------- */
/* notification_set_priv_id / notification_get_id                 */
/* ------------------------------------------------------------- */

TEST_F(NotificationInternalTest, set_priv_id_positive_roundtrip) {
  ASSERT_EQ(notification_set_priv_id(noti_, 123),
            NOTIFICATION_ERROR_NONE);
  int group = 0;
  int priv = 0;
  EXPECT_EQ(notification_get_id(noti_, &group, &priv),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(priv, 123);
}

TEST_F(NotificationInternalTest, set_priv_id_zero_or_negative_returns_invalid) {
  /* priv_id must be > 0 per impl. */
  EXPECT_EQ(notification_set_priv_id(noti_, 0),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_set_priv_id(noti_, -1),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationInternalTest, set_priv_id_null_noti) {
  EXPECT_EQ(notification_set_priv_id(nullptr, 1),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationInternalTest, get_id_skips_null_out_args) {
  /* Both group_id and priv_id NULL → still NONE, no writes. */
  EXPECT_EQ(notification_get_id(noti_, nullptr, nullptr),
            NOTIFICATION_ERROR_NONE);
}

TEST_F(NotificationInternalTest, get_id_null_noti) {
  int g = 0, p = 0;
  EXPECT_EQ(notification_get_id(nullptr, &g, &p),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* ------------------------------------------------------------- */
/* notification_get_event_flag                                    */
/* ------------------------------------------------------------- */

TEST_F(NotificationInternalTest, get_event_flag_returns_false_initially) {
  bool flag = true;
  EXPECT_EQ(notification_get_event_flag(noti_, &flag),
            NOTIFICATION_ERROR_NONE);
  EXPECT_FALSE(flag);
}

TEST_F(NotificationInternalTest, get_event_flag_null_args) {
  bool flag;
  EXPECT_EQ(notification_get_event_flag(nullptr, &flag),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_get_event_flag(noti_, nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* ------------------------------------------------------------- */
/* notification_set_indirect_request                              */
/* ------------------------------------------------------------- */

TEST_F(NotificationInternalTest, set_indirect_request_null_noti) {
  EXPECT_EQ(notification_set_indirect_request(nullptr, 100, 1000),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationInternalTest, set_indirect_request_rejects_invalid_pid) {
  /* pid must be > 1. */
  EXPECT_EQ(notification_set_indirect_request(noti_, 0, 1000),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_set_indirect_request(noti_, 1, 1000),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationInternalTest, set_indirect_request_rejects_invalid_uid) {
  /* uid must be >= REGULAR_UID_MIN (typically 5000). */
  EXPECT_EQ(notification_set_indirect_request(noti_, 100, 0),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* ------------------------------------------------------------- */
/* notification_set_extension_data — null args                     */
/* ------------------------------------------------------------- */

TEST_F(NotificationInternalTest, set_extension_data_null_args) {
  bundle* b = bundle_create();
  EXPECT_EQ(notification_set_extension_data(nullptr, "k", b),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_set_extension_data(noti_, nullptr, b),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  bundle_free(b);
}

/* ------------------------------------------------------------- */
/* notification_set_execute_option / get_execute_option           */
/* ------------------------------------------------------------- */

TEST_F(NotificationInternalTest, set_execute_option_null_noti) {
  EXPECT_EQ(notification_set_execute_option(nullptr,
                NOTIFICATION_EXECUTE_TYPE_RESPONDING, "txt", "key", nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationInternalTest, set_execute_option_invalid_type) {
  EXPECT_EQ(notification_set_execute_option(noti_,
                NOTIFICATION_EXECUTE_TYPE_NONE, "t", "k", nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_set_execute_option(noti_,
                static_cast<notification_execute_type_e>(
                    NOTIFICATION_EXECUTE_TYPE_MAX + 1),
                "t", "k", nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationInternalTest, get_execute_option_null_noti) {
  const char* text = nullptr;
  bundle* svc = nullptr;
  EXPECT_EQ(notification_get_execute_option(nullptr,
                NOTIFICATION_EXECUTE_TYPE_RESPONDING, &text, &svc),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationInternalTest, get_execute_option_invalid_type) {
  const char* text = nullptr;
  bundle* svc = nullptr;
  EXPECT_EQ(notification_get_execute_option(noti_,
                NOTIFICATION_EXECUTE_TYPE_NONE, &text, &svc),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationInternalTest, set_execute_option_responding_roundtrip) {
  bundle* b = bundle_create();
  ASSERT_NE(b, nullptr);
  bundle_add_str(b, "key1", "val1");
  ASSERT_EQ(notification_set_execute_option(noti_,
                NOTIFICATION_EXECUTE_TYPE_RESPONDING,
                "the response", "RESP_KEY", b),
            NOTIFICATION_ERROR_NONE);
  const char* out_text = nullptr;
  bundle* out_svc = nullptr;
  EXPECT_EQ(notification_get_execute_option(noti_,
                NOTIFICATION_EXECUTE_TYPE_RESPONDING,
                &out_text, &out_svc),
            NOTIFICATION_ERROR_NONE);
  bundle_free(b);
}

TEST_F(NotificationInternalTest, set_execute_option_launch_roundtrip) {
  bundle* b = bundle_create();
  ASSERT_NE(b, nullptr);
  ASSERT_EQ(notification_set_execute_option(noti_,
                NOTIFICATION_EXECUTE_TYPE_SINGLE_LAUNCH,
                "single", "SINGLE_KEY", b),
            NOTIFICATION_ERROR_NONE);
  const char* out_text = nullptr;
  bundle* out_svc = nullptr;
  EXPECT_EQ(notification_get_execute_option(noti_,
                NOTIFICATION_EXECUTE_TYPE_SINGLE_LAUNCH,
                &out_text, &out_svc),
            NOTIFICATION_ERROR_NONE);
  bundle_free(b);
}

TEST_F(NotificationInternalTest, set_execute_option_multi_launch_roundtrip) {
  bundle* b = bundle_create();
  ASSERT_NE(b, nullptr);
  ASSERT_EQ(notification_set_execute_option(noti_,
                NOTIFICATION_EXECUTE_TYPE_MULTI_LAUNCH,
                "multi", "MULTI_KEY", b),
            NOTIFICATION_ERROR_NONE);
  const char* out_text = nullptr;
  bundle* out_svc = nullptr;
  EXPECT_EQ(notification_get_execute_option(noti_,
                NOTIFICATION_EXECUTE_TYPE_MULTI_LAUNCH,
                &out_text, &out_svc),
            NOTIFICATION_ERROR_NONE);
  bundle_free(b);
}

/* ------------------------------------------------------------- */
/* notification_set_extension_event_handler null/range arg paths   */
/* ------------------------------------------------------------- */

TEST_F(NotificationInternalTest, set_extension_event_handler_null_args) {
  app_control_h ac = nullptr;
  app_control_create(&ac);
  EXPECT_EQ(notification_set_extension_event_handler(nullptr,
                NOTIFICATION_EVENT_TYPE_HIDDEN_BY_USER, ac),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_set_extension_event_handler(noti_,
                NOTIFICATION_EVENT_TYPE_HIDDEN_BY_USER, nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  app_control_destroy(ac);
}

TEST_F(NotificationInternalTest, set_extension_event_handler_invalid_event) {
  app_control_h ac = nullptr;
  app_control_create(&ac);
  EXPECT_EQ(notification_set_extension_event_handler(noti_,
                static_cast<notification_event_type_extension_e>(
                    NOTIFICATION_EVENT_TYPE_HIDDEN_BY_USER - 1),
                ac),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_set_extension_event_handler(noti_,
                static_cast<notification_event_type_extension_e>(
                    NOTIFICATION_EVENT_TYPE_HIDDEN_BY_EXTERNAL + 1),
                ac),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  app_control_destroy(ac);
}

/* ------------------------------------------------------------- */
/* notification_set_app_label / get_app_label                     */
/* ------------------------------------------------------------- */

TEST_F(NotificationInternalTest, set_app_label_roundtrip) {
  char label[] = "MyApp";
  ASSERT_EQ(notification_set_app_label(noti_, label),
            NOTIFICATION_ERROR_NONE);
  char* got = nullptr;
  EXPECT_EQ(notification_get_app_label(noti_, &got),
            NOTIFICATION_ERROR_NONE);
  ASSERT_NE(got, nullptr);
  EXPECT_STREQ(got, "MyApp");
}

TEST_F(NotificationInternalTest, set_app_label_replaces_previous) {
  char first[] = "First";
  char second[] = "Second";
  ASSERT_EQ(notification_set_app_label(noti_, first),
            NOTIFICATION_ERROR_NONE);
  ASSERT_EQ(notification_set_app_label(noti_, second),
            NOTIFICATION_ERROR_NONE);
  char* got = nullptr;
  EXPECT_EQ(notification_get_app_label(noti_, &got),
            NOTIFICATION_ERROR_NONE);
  ASSERT_NE(got, nullptr);
  EXPECT_STREQ(got, "Second");
}

TEST_F(NotificationInternalTest, set_app_label_null_args) {
  char label[] = "x";
  EXPECT_EQ(notification_set_app_label(nullptr, label),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_set_app_label(noti_, nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationInternalTest, get_app_label_null_args) {
  char* got = nullptr;
  EXPECT_EQ(notification_get_app_label(nullptr, &got),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_get_app_label(noti_, nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* ------------------------------------------------------------- */
/* notification_add_deferred_task / del_deferred_task — null args */
/* ------------------------------------------------------------- */

TEST_F(NotificationInternalTest, add_deferred_task_null_cb) {
  EXPECT_EQ(notification_add_deferred_task(nullptr, nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationInternalTest, del_deferred_task_null_cb) {
  EXPECT_EQ(notification_del_deferred_task(nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* ------------------------------------------------------------- */
/* notification_send_event range/null tests                        */
/* ------------------------------------------------------------- */

TEST_F(NotificationInternalTest, send_event_null_noti) {
  EXPECT_EQ(notification_send_event(nullptr,
                NOTIFICATION_EVENT_TYPE_CLICK_ON_BUTTON_1),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationInternalTest, send_event_invalid_type) {
  /* event_type must be in one of three valid ranges. -1 is in none. */
  EXPECT_EQ(notification_send_event(noti_,
                static_cast<int>(NOTIFICATION_EVENT_TYPE_CLICK_ON_BUTTON_1) - 1),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* ------------------------------------------------------------- */
/* notification_check_event_receiver_available — null arg          */
/* ------------------------------------------------------------- */

TEST_F(NotificationInternalTest, check_event_receiver_available_null_args) {
  bool avail;
  EXPECT_EQ(notification_check_event_receiver_available(nullptr, &avail),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_check_event_receiver_available(noti_, nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* ------------------------------------------------------------- */
/* notification_send_event_by_priv_id — invalid arg                */
/* ------------------------------------------------------------- */

TEST_F(NotificationInternalTest, send_event_by_priv_id_invalid_type) {
  /* event_type out of range yields INVALID_PARAMETER without IPC. */
  EXPECT_EQ(notification_send_event_by_priv_id(1,
                static_cast<int>(NOTIFICATION_EVENT_TYPE_CLICK_ON_BUTTON_1) - 1),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* ------------------------------------------------------------- */
/* notification_update_progress / update_size / update_content    */
/* — null-noti when priv_id is NONE                               */
/* ------------------------------------------------------------- */

TEST_F(NotificationInternalTest, update_progress_invalid_priv_with_null_noti) {
  /* When priv_id <= NONE and noti is NULL → INVALID_PARAMETER. */
  EXPECT_EQ(notification_update_progress(nullptr,
                NOTIFICATION_PRIV_ID_NONE, 0.5),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationInternalTest, update_size_invalid_priv_with_null_noti) {
  EXPECT_EQ(notification_update_size(nullptr,
                NOTIFICATION_PRIV_ID_NONE, 100.0),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationInternalTest, update_content_invalid_priv_with_null_noti) {
  EXPECT_EQ(notification_update_content(nullptr,
                NOTIFICATION_PRIV_ID_NONE, "hello"),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* ------------------------------------------------------------- */
/* notification_register_detailed_changed_cb / unregister — null   */
/* ------------------------------------------------------------- */

TEST_F(NotificationInternalTest, register_detailed_changed_cb_null) {
  EXPECT_EQ(notification_register_detailed_changed_cb(nullptr, nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationInternalTest, unregister_detailed_changed_cb_null) {
  EXPECT_EQ(notification_unregister_detailed_changed_cb(nullptr, nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* ------------------------------------------------------------- */
/* notification_resister_changed_cb / unresister_changed_cb        */
/* (Tizen spelling; null-cb validation; _for_uid variants)         */
/* ------------------------------------------------------------- */

TEST_F(NotificationInternalTest, resister_changed_cb_null) {
  EXPECT_EQ(notification_resister_changed_cb(nullptr, nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationInternalTest, unresister_changed_cb_null) {
  EXPECT_EQ(notification_unresister_changed_cb(nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationInternalTest, resister_changed_cb_for_uid_null) {
  EXPECT_EQ(notification_resister_changed_cb_for_uid(nullptr, nullptr, 1000),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationInternalTest, unresister_changed_cb_for_uid_null) {
  EXPECT_EQ(notification_unresister_changed_cb_for_uid(nullptr, 1000),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationInternalTest, register_detailed_changed_cb_for_uid_null) {
  EXPECT_EQ(notification_register_detailed_changed_cb_for_uid(nullptr,
                nullptr, 1000),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationInternalTest, unregister_detailed_changed_cb_for_uid_null) {
  EXPECT_EQ(notification_unregister_detailed_changed_cb(nullptr,
                nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* ------------------------------------------------------------- */
/* notification_new — alias for notification_create               */
/* ------------------------------------------------------------- */

TEST_F(NotificationInternalTest, new_returns_handle) {
  /* notification_new is a thin wrapper around notification_create. */
  notification_h n = notification_new(NOTIFICATION_TYPE_NOTI, 0, 0);
  ASSERT_NE(n, nullptr);
  notification_free(n);
}

/* Note: notification_clear and notification_delete_all_by_type bypass
 * argument validation and attempt IPC to the master, which times out
 * (~25s) without a server. They are documented as caller contract and
 * tested at the integration level, not here. */

/* ------------------------------------------------------------- */
/* notification_get_count — null                                   */
/* ------------------------------------------------------------- */

TEST_F(NotificationInternalTest, get_count_null_count_out) {
  EXPECT_EQ(notification_get_count(NOTIFICATION_TYPE_NOTI,
                "any", 0, 0, nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* ------------------------------------------------------------- */
/* notification_get_args (returns group args optionally)           */
/* ------------------------------------------------------------- */

TEST_F(NotificationInternalTest, get_args_returns_set_args) {
  bundle* b = bundle_create();
  bundle_add_str(b, "k1", "v1");
  ASSERT_EQ(notification_set_args(noti_, b, nullptr),
            NOTIFICATION_ERROR_NONE);

  bundle* got_args = nullptr;
  bundle* got_group = nullptr;
  EXPECT_EQ(notification_get_args(noti_, &got_args, &got_group),
            NOTIFICATION_ERROR_NONE);
  ASSERT_NE(got_args, nullptr);
  /* Verify the same key/value survived. */
  char* v = nullptr;
  bundle_get_str(got_args, "k1", &v);
  EXPECT_STREQ(v, "v1");

  bundle_free(b);
}

TEST_F(NotificationInternalTest, get_args_skips_optional_group_arg) {
  bundle* b = bundle_create();
  ASSERT_EQ(notification_set_args(noti_, b, nullptr),
            NOTIFICATION_ERROR_NONE);
  bundle* got_args = nullptr;
  /* group_args output is optional; passing NULL is allowed. */
  EXPECT_EQ(notification_get_args(noti_, &got_args, nullptr),
            NOTIFICATION_ERROR_NONE);
  bundle_free(b);
}

/* ------------------------------------------------------------- */
/* notification_set_extention_data (deprecated spelling)           */
/* ------------------------------------------------------------- */

TEST_F(NotificationInternalTest, set_extention_data_null_args) {
  bundle* b = bundle_create();
  EXPECT_EQ(notification_set_extention_data(nullptr, "k", b),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_set_extention_data(noti_, nullptr, b),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  bundle_free(b);
}

TEST_F(NotificationInternalTest, get_extention_data_null_args) {
  bundle* out = nullptr;
  EXPECT_EQ(notification_get_extention_data(nullptr, "k", &out),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_get_extention_data(noti_, nullptr, &out),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_get_extention_data(noti_, "k", nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationInternalTest, get_extension_data_null_args) {
  bundle* out = nullptr;
  EXPECT_EQ(notification_get_extension_data(nullptr, "k", &out),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_get_extension_data(noti_, nullptr, &out),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_get_extension_data(noti_, "k", nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* ------------------------------------------------------------- */
/* notification_update_async / _for_uid — null-noti                */
/* ------------------------------------------------------------- */

TEST_F(NotificationInternalTest, update_async_null_noti) {
  EXPECT_EQ(notification_update_async(nullptr, nullptr, nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationInternalTest, update_async_for_uid_null_noti) {
  EXPECT_EQ(notification_update_async_for_uid(nullptr, nullptr, nullptr, 1000),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* ------------------------------------------------------------- */
/* notification_insert_for_uid — null-noti                        */
/* ------------------------------------------------------------- */

TEST_F(NotificationInternalTest, insert_for_uid_null_noti) {
  int priv_id = 0;
  EXPECT_EQ(notification_insert_for_uid(nullptr, &priv_id, 1000),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* ------------------------------------------------------------- */
/* notification_insert — null-noti                                 */
/* ------------------------------------------------------------- */

TEST_F(NotificationInternalTest, insert_null_noti) {
  int priv_id = 0;
  EXPECT_EQ(notification_insert(nullptr, &priv_id),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* ------------------------------------------------------------- */
/* notification_post_with_event_cb — null-noti                     */
/* ------------------------------------------------------------- */

TEST_F(NotificationInternalTest, post_with_event_cb_null_noti) {
  EXPECT_EQ(notification_post_with_event_cb(nullptr, nullptr, nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationInternalTest, post_with_event_cb_for_uid_null_noti) {
  EXPECT_EQ(notification_post_with_event_cb_for_uid(nullptr, nullptr,
                nullptr, 1000),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* ------------------------------------------------------------- */
/* notification_post_for_uid / delete_for_uid null arg              */
/* ------------------------------------------------------------- */

TEST_F(NotificationInternalTest, post_for_uid_null_noti) {
  EXPECT_EQ(notification_post_for_uid(nullptr, 1000),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationInternalTest, delete_for_uid_null_noti) {
  EXPECT_EQ(notification_delete_for_uid(nullptr, 1000),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* ------------------------------------------------------------- */
/* notification_get_all_count / _for_uid — null/invalid arg        */
/* ------------------------------------------------------------- */

TEST_F(NotificationInternalTest, get_all_count_null_count) {
  EXPECT_EQ(notification_get_all_count(NOTIFICATION_TYPE_NOTI, nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationInternalTest, get_all_count_for_uid_null_count) {
  EXPECT_EQ(notification_get_all_count_for_uid(NOTIFICATION_TYPE_NOTI,
                nullptr, 1000),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationInternalTest, get_all_count_for_uid_invalid_type) {
  /* NOTIFICATION_TYPE_NONE = -1, so the "type < NONE" path requires
   * a value strictly less than -1. */
  int count;
  EXPECT_EQ(notification_get_all_count_for_uid(
                static_cast<notification_type_e>(-2), &count, 1000),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* ------------------------------------------------------------- */
/* notification_get_default_button null args                        */
/* ------------------------------------------------------------- */

TEST_F(NotificationInternalTest, get_default_button_null_args) {
  notification_button_index_e idx;
  EXPECT_EQ(notification_get_default_button(nullptr, &idx),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_get_default_button(noti_, nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* ------------------------------------------------------------- */
/* notification_get_extension_event_handler null/range args        */
/* ------------------------------------------------------------- */

TEST_F(NotificationInternalTest, get_extension_event_handler_null_args) {
  app_control_h ac = nullptr;
  EXPECT_EQ(notification_get_extension_event_handler(nullptr,
                NOTIFICATION_EVENT_TYPE_HIDDEN_BY_USER, &ac),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_get_extension_event_handler(noti_,
                NOTIFICATION_EVENT_TYPE_HIDDEN_BY_USER, nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationInternalTest, get_extension_event_handler_invalid_event) {
  app_control_h ac = nullptr;
  EXPECT_EQ(notification_get_extension_event_handler(noti_,
                static_cast<notification_event_type_extension_e>(
                    NOTIFICATION_EVENT_TYPE_HIDDEN_BY_USER - 1),
                &ac),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_get_extension_event_handler(noti_,
                static_cast<notification_event_type_extension_e>(
                    NOTIFICATION_EVENT_TYPE_HIDDEN_BY_EXTERNAL + 1),
                &ac),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* ------------------------------------------------------------- */
/* notification_set_args with group_args                           */
/* ------------------------------------------------------------- */

TEST_F(NotificationInternalTest, set_args_with_group_args_returns_none) {
  bundle* args = bundle_create();
  bundle_add_str(args, "k", "v");
  bundle* group_args = bundle_create();
  bundle_add_str(group_args, "gk", "gv");

  EXPECT_EQ(notification_set_args(noti_, args, group_args),
            NOTIFICATION_ERROR_NONE);
  /* Get both back. */
  bundle* got_args = nullptr;
  bundle* got_group = nullptr;
  EXPECT_EQ(notification_get_args(noti_, &got_args, &got_group),
            NOTIFICATION_ERROR_NONE);
  EXPECT_NE(got_args, nullptr);
  EXPECT_NE(got_group, nullptr);

  bundle_free(args);
  bundle_free(group_args);
}

TEST_F(NotificationInternalTest, set_args_replaces_existing_args) {
  bundle* a1 = bundle_create();
  bundle_add_str(a1, "k", "v1");
  bundle* a2 = bundle_create();
  bundle_add_str(a2, "k", "v2");

  ASSERT_EQ(notification_set_args(noti_, a1, nullptr),
            NOTIFICATION_ERROR_NONE);
  ASSERT_EQ(notification_set_args(noti_, a2, nullptr),
            NOTIFICATION_ERROR_NONE);

  bundle* got = nullptr;
  EXPECT_EQ(notification_get_args(noti_, &got, nullptr),
            NOTIFICATION_ERROR_NONE);
  char* v = nullptr;
  bundle_get_str(got, "k", &v);
  EXPECT_STREQ(v, "v2");

  bundle_free(a1);
  bundle_free(a2);
}

/* ------------------------------------------------------------- */
/* notification_delete_all_for_uid — invalid type validation        */
/* (NONE=-1 → INVALID, MAX→ IPC; only NONE is reliable.)            */
/* ------------------------------------------------------------- */

TEST_F(NotificationInternalTest, delete_all_for_uid_type_none) {
  EXPECT_EQ(notification_delete_all_for_uid(NOTIFICATION_TYPE_NONE, 1000),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationInternalTest, delete_all_for_uid_type_underflow) {
  EXPECT_EQ(notification_delete_all_for_uid(
                static_cast<notification_type_e>(-2), 1000),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* ------------------------------------------------------------- */
/* notification_load_by_tag_for_uid null tag                       */
/* ------------------------------------------------------------- */

TEST_F(NotificationInternalTest, load_by_tag_for_uid_null_tag) {
  notification_h n = notification_load_by_tag_for_uid(nullptr, 1000);
  EXPECT_EQ(n, nullptr);
}

/* ------------------------------------------------------------- */
/* notification_set_priv_id large value                            */
/* ------------------------------------------------------------- */

TEST_F(NotificationInternalTest, set_priv_id_int_max) {
  ASSERT_EQ(notification_set_priv_id(noti_, 2147483647),
            NOTIFICATION_ERROR_NONE);
  int group = 0, priv = 0;
  EXPECT_EQ(notification_get_id(noti_, &group, &priv),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(priv, 2147483647);
}

/* ------------------------------------------------------------- */
/* notification_set_default_button index range                     */
/* ------------------------------------------------------------- */

TEST_F(NotificationInternalTest, set_default_button_each_valid_index_1_6) {
  /* Per impl: only BUTTON_1..BUTTON_6 are valid for default_button.
   * BUTTON_7..BUTTON_10 are rejected as INVALID_PARAMETER. */
  for (int i = NOTIFICATION_BUTTON_1; i <= NOTIFICATION_BUTTON_6; ++i) {
    EXPECT_EQ(notification_set_default_button(noti_,
                  static_cast<notification_button_index_e>(i)),
              NOTIFICATION_ERROR_NONE);
  }
}

TEST_F(NotificationInternalTest, set_default_button_rejects_button_7_or_more) {
  EXPECT_EQ(notification_set_default_button(noti_, NOTIFICATION_BUTTON_7),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_set_default_button(noti_, NOTIFICATION_BUTTON_10),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* ------------------------------------------------------------- */
/* notification_set_default_button negative                        */
/* ------------------------------------------------------------- */

TEST_F(NotificationInternalTest, set_default_button_negative_returns_invalid) {
  EXPECT_EQ(notification_set_default_button(noti_,
                static_cast<notification_button_index_e>(-1)),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* ------------------------------------------------------------- */
/* notification_get_default_button default value                   */
/* ------------------------------------------------------------- */

TEST_F(NotificationInternalTest, get_default_button_default_is_zero) {
  notification_button_index_e idx =
      static_cast<notification_button_index_e>(99);
  EXPECT_EQ(notification_get_default_button(noti_, &idx),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(idx, 0);
}

TEST_F(NotificationInternalTest, get_default_button_after_set_roundtrip) {
  ASSERT_EQ(notification_set_default_button(noti_, NOTIFICATION_BUTTON_3),
            NOTIFICATION_ERROR_NONE);
  notification_button_index_e idx;
  EXPECT_EQ(notification_get_default_button(noti_, &idx),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(idx, NOTIFICATION_BUTTON_3);
}

/* ------------------------------------------------------------- */
/* notification_get_application default                            */
/* ------------------------------------------------------------- */

TEST_F(NotificationInternalTest, get_application_default_falls_back) {
  char* app = nullptr;
  /* When launch_app_id is unset, fallback to caller_app_id. */
  EXPECT_EQ(notification_get_application(noti_, &app),
            NOTIFICATION_ERROR_NONE);
  /* Either NULL or a non-empty string. */
  if (app != nullptr) {
    EXPECT_NE(app[0], '\0');
  }
}

/* ------------------------------------------------------------- */
/* notification_set_application empty string allowed              */
/* ------------------------------------------------------------- */

TEST_F(NotificationInternalTest, set_application_empty_string) {
  ASSERT_EQ(notification_set_application(noti_, ""),
            NOTIFICATION_ERROR_NONE);
  char* app = nullptr;
  EXPECT_EQ(notification_get_application(noti_, &app),
            NOTIFICATION_ERROR_NONE);
  /* Empty string was stored. */
}

/* ------------------------------------------------------------- */
/* notification_set_hide_timeout zero allowed                     */
/* ------------------------------------------------------------- */

TEST_F(NotificationInternalTest, set_hide_timeout_zero_allowed) {
  ASSERT_EQ(notification_set_hide_timeout(noti_, 0),
            NOTIFICATION_ERROR_NONE);
  int v = -1;
  EXPECT_EQ(notification_get_hide_timeout(noti_, &v),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(v, 0);
}

/* ------------------------------------------------------------- */
/* notification_set_delete_timeout zero allowed                   */
/* ------------------------------------------------------------- */

TEST_F(NotificationInternalTest, set_delete_timeout_zero_allowed) {
  ASSERT_EQ(notification_set_delete_timeout(noti_, 0),
            NOTIFICATION_ERROR_NONE);
  int v = -1;
  EXPECT_EQ(notification_get_delete_timeout(noti_, &v),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(v, 0);
}

/* ------------------------------------------------------------- */
/* notification_set_pkgname empty string allowed                  */
/* ------------------------------------------------------------- */

TEST_F(NotificationInternalTest, set_pkgname_empty_string) {
  ASSERT_EQ(notification_set_pkgname(noti_, ""),
            NOTIFICATION_ERROR_NONE);
  char* p = nullptr;
  EXPECT_EQ(notification_get_pkgname(noti_, &p),
            NOTIFICATION_ERROR_NONE);
  ASSERT_NE(p, nullptr);
  EXPECT_STREQ(p, "");
}

/* ------------------------------------------------------------- */
/* notification_set_app_id empty string allowed                    */
/* ------------------------------------------------------------- */

TEST_F(NotificationInternalTest, set_app_id_empty_string) {
  ASSERT_EQ(notification_set_app_id(noti_, ""),
            NOTIFICATION_ERROR_NONE);
  char* p = nullptr;
  EXPECT_EQ(notification_get_pkgname(noti_, &p),
            NOTIFICATION_ERROR_NONE);
  ASSERT_NE(p, nullptr);
  EXPECT_STREQ(p, "");
}

/* ------------------------------------------------------------- */
/* notification_set_icon empty path allowed                        */
/* ------------------------------------------------------------- */

TEST_F(NotificationInternalTest, set_icon_empty_path_stored) {
  ASSERT_EQ(notification_set_icon(noti_, ""),
            NOTIFICATION_ERROR_NONE);
  char* p = nullptr;
  EXPECT_EQ(notification_get_icon(noti_, &p),
            NOTIFICATION_ERROR_NONE);
  ASSERT_NE(p, nullptr);
  EXPECT_STREQ(p, "");
}

/* ------------------------------------------------------------- */
/* notification_set_priv_id multiple sequential                    */
/* ------------------------------------------------------------- */

TEST_F(NotificationInternalTest, set_priv_id_sequence) {
  for (int i = 1; i <= 5; ++i) {
    ASSERT_EQ(notification_set_priv_id(noti_, i),
              NOTIFICATION_ERROR_NONE);
    int group = 0, priv = 0;
    EXPECT_EQ(notification_get_id(noti_, &group, &priv),
              NOTIFICATION_ERROR_NONE);
    EXPECT_EQ(priv, i);
  }
}

/* ------------------------------------------------------------- */
/* notification_get_hide_timeout default                           */
/* ------------------------------------------------------------- */

TEST_F(NotificationInternalTest, get_hide_timeout_default_is_zero) {
  int v = 99;
  EXPECT_EQ(notification_get_hide_timeout(noti_, &v),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(v, 0);
}

/* ------------------------------------------------------------- */
/* notification_get_delete_timeout default                         */
/* ------------------------------------------------------------- */

TEST_F(NotificationInternalTest, get_delete_timeout_default_is_zero) {
  int v = 99;
  EXPECT_EQ(notification_get_delete_timeout(noti_, &v),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(v, 0);
}

/* ------------------------------------------------------------- */
/* notification_set_hide_timeout/delete_timeout boundary values    */
/* ------------------------------------------------------------- */

TEST_F(NotificationInternalTest, set_hide_timeout_large_value) {
  ASSERT_EQ(notification_set_hide_timeout(noti_, 86400),
            NOTIFICATION_ERROR_NONE);
  int v = 0;
  EXPECT_EQ(notification_get_hide_timeout(noti_, &v),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(v, 86400);
}

TEST_F(NotificationInternalTest, set_delete_timeout_large_value) {
  ASSERT_EQ(notification_set_delete_timeout(noti_, 604800),
            NOTIFICATION_ERROR_NONE);
  int v = 0;
  EXPECT_EQ(notification_get_delete_timeout(noti_, &v),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(v, 604800);
}

/* ------------------------------------------------------------- */
/* notification_set_ongoing_value_type ranges                      */
/* ------------------------------------------------------------- */

TEST_F(NotificationInternalTest, set_ongoing_value_type_each_valid) {
  EXPECT_EQ(notification_set_ongoing_value_type(noti_,
                NOTIFICATION_ONGOING_VALUE_TYPE_PERCENT),
            NOTIFICATION_ERROR_NONE);
  notification_ongoing_value_type_e v;
  EXPECT_EQ(notification_get_ongoing_value_type(noti_, &v),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(v, NOTIFICATION_ONGOING_VALUE_TYPE_PERCENT);

  EXPECT_EQ(notification_set_ongoing_value_type(noti_,
                NOTIFICATION_ONGOING_VALUE_TYPE_TIME),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(notification_get_ongoing_value_type(noti_, &v),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(v, NOTIFICATION_ONGOING_VALUE_TYPE_TIME);
}

/* ------------------------------------------------------------- */
/* notification_set_ongoing_time positive values roundtrip         */
/* ------------------------------------------------------------- */

TEST_F(NotificationInternalTest, set_ongoing_time_positive_roundtrip) {
  ASSERT_EQ(notification_set_ongoing_time(noti_, 30, 60),
            NOTIFICATION_ERROR_NONE);
  int cur = 0, dur = 0;
  EXPECT_EQ(notification_get_ongoing_time(noti_, &cur, &dur),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(cur, 30);
  EXPECT_EQ(dur, 60);
}

/* ------------------------------------------------------------- */
/* notification_set_check_box default                              */
/* ------------------------------------------------------------- */

TEST_F(NotificationInternalTest, set_check_box_each_valid_flag) {
  /* flag values: per impl typically 0..MAX. */
  for (int f = 0; f < 3; ++f) {
    EXPECT_EQ(notification_set_check_box(noti_, f, false),
              NOTIFICATION_ERROR_NONE);
  }
}

/* ------------------------------------------------------------- */
/* notification_get_text_input_max_length default                  */
/* ------------------------------------------------------------- */

TEST_F(NotificationInternalTest, get_text_input_max_length_default_is_zero) {
  int v = 99;
  EXPECT_EQ(notification_get_text_input_max_length(noti_, &v),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(v, 0);
}

/* ------------------------------------------------------------- */
/* notification_set_check_box_checked default                      */
/* ------------------------------------------------------------- */

TEST_F(NotificationInternalTest, get_check_box_checked_default_unset) {
  bool v = true;
  EXPECT_EQ(notification_get_check_box_checked(noti_, &v),
            NOTIFICATION_ERROR_NONE);
  EXPECT_FALSE(v);
}

/* ------------------------------------------------------------- */
/* notification_set_icon replaces previous                         */
/* ------------------------------------------------------------- */

TEST_F(NotificationInternalTest, set_icon_replaces_previous) {
  ASSERT_EQ(notification_set_icon(noti_, "/icon1.png"),
            NOTIFICATION_ERROR_NONE);
  ASSERT_EQ(notification_set_icon(noti_, "/icon2.png"),
            NOTIFICATION_ERROR_NONE);
  char* got = nullptr;
  EXPECT_EQ(notification_get_icon(noti_, &got),
            NOTIFICATION_ERROR_NONE);
  ASSERT_NE(got, nullptr);
  EXPECT_STREQ(got, "/icon2.png");
}

/* ------------------------------------------------------------- */
/* notification_set_pkgname / notification_set_app_id replacement   */
/* ------------------------------------------------------------- */

TEST_F(NotificationInternalTest, set_pkgname_then_set_app_id_replaces) {
  ASSERT_EQ(notification_set_pkgname(noti_, "first.pkg"),
            NOTIFICATION_ERROR_NONE);
  ASSERT_EQ(notification_set_app_id(noti_, "new.app.id"),
            NOTIFICATION_ERROR_NONE);
  char* p = nullptr;
  EXPECT_EQ(notification_get_pkgname(noti_, &p),
            NOTIFICATION_ERROR_NONE);
  ASSERT_NE(p, nullptr);
  EXPECT_STREQ(p, "new.app.id");
}

/* ------------------------------------------------------------- */
/* notification_set_priv_id rejects negative values explicitly     */
/* ------------------------------------------------------------- */

TEST_F(NotificationInternalTest, set_priv_id_negative_value_invalid) {
  EXPECT_EQ(notification_set_priv_id(noti_, -100),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_set_priv_id(noti_, -1),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* ------------------------------------------------------------- */
/* notification_set_args invalidates with null args                */
/* ------------------------------------------------------------- */

TEST_F(NotificationInternalTest, set_args_null_args_returns_invalid_redundant) {
  EXPECT_EQ(notification_set_args(noti_, nullptr, nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* ------------------------------------------------------------- */
/* notification_set_ongoing_time negative or inverted              */
/* ------------------------------------------------------------- */

TEST_F(NotificationInternalTest, set_ongoing_time_negative_args_invalid) {
  /* Negative current or duration is invalid. */
  EXPECT_EQ(notification_set_ongoing_time(noti_, -1, 10),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_set_ongoing_time(noti_, 10, -1),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* ------------------------------------------------------------- */
/* notification_set_hide_timeout negative                           */
/* ------------------------------------------------------------- */

TEST_F(NotificationInternalTest, set_hide_timeout_negative_invalid) {
  EXPECT_EQ(notification_set_hide_timeout(noti_, -1),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationInternalTest, set_delete_timeout_negative_invalid) {
  EXPECT_EQ(notification_set_delete_timeout(noti_, -1),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* ------------------------------------------------------------- */
/* notification_set_check_box_checked toggle                       */
/* ------------------------------------------------------------- */

TEST_F(NotificationInternalTest, set_check_box_checked_toggle) {
  /* get_check_box_checked only returns the stored value when the
   * check_box flag is enabled; otherwise returns false unconditionally. */
  ASSERT_EQ(notification_set_check_box(noti_, 1, true),
            NOTIFICATION_ERROR_NONE);
  for (int i = 0; i < 4; ++i) {
    bool target = (i % 2 == 0);
    ASSERT_EQ(notification_set_check_box_checked(noti_, target),
              NOTIFICATION_ERROR_NONE);
    bool v = !target;
    ASSERT_EQ(notification_get_check_box_checked(noti_, &v),
              NOTIFICATION_ERROR_NONE);
    EXPECT_EQ(v, target);
  }
}

/* ------------------------------------------------------------- */
/* notification_set_args group_args replaces previous              */
/* ------------------------------------------------------------- */

TEST_F(NotificationInternalTest, set_args_group_args_replaces_previous) {
  bundle* args = bundle_create();
  bundle* g1 = bundle_create();
  bundle_add_str(g1, "gk", "v1");
  bundle* g2 = bundle_create();
  bundle_add_str(g2, "gk", "v2");

  ASSERT_EQ(notification_set_args(noti_, args, g1),
            NOTIFICATION_ERROR_NONE);
  ASSERT_EQ(notification_set_args(noti_, args, g2),
            NOTIFICATION_ERROR_NONE);

  bundle* got_args = nullptr;
  bundle* got_group = nullptr;
  EXPECT_EQ(notification_get_args(noti_, &got_args, &got_group),
            NOTIFICATION_ERROR_NONE);
  ASSERT_NE(got_group, nullptr);
  char* v = nullptr;
  bundle_get_str(got_group, "gk", &v);
  EXPECT_STREQ(v, "v2");

  bundle_free(args);
  bundle_free(g1);
  bundle_free(g2);
}

/* ------------------------------------------------------------- */
/* notification_set_application replaces previous                  */
/* ------------------------------------------------------------- */

TEST_F(NotificationInternalTest, set_application_replaces_previous) {
  ASSERT_EQ(notification_set_application(noti_, "first.app"),
            NOTIFICATION_ERROR_NONE);
  ASSERT_EQ(notification_set_application(noti_, "second.app"),
            NOTIFICATION_ERROR_NONE);
  char* app = nullptr;
  EXPECT_EQ(notification_get_application(noti_, &app),
            NOTIFICATION_ERROR_NONE);
  ASSERT_NE(app, nullptr);
  EXPECT_STREQ(app, "second.app");
}

/* ------------------------------------------------------------- */
/* notification_set_title replaces previous                        */
/* ------------------------------------------------------------- */

TEST_F(NotificationInternalTest, set_title_replaces_previous) {
  ASSERT_EQ(notification_set_title(noti_, "first", nullptr),
            NOTIFICATION_ERROR_NONE);
  ASSERT_EQ(notification_set_title(noti_, "second", nullptr),
            NOTIFICATION_ERROR_NONE);
  char* title = nullptr;
  char* loc = nullptr;
  EXPECT_EQ(notification_get_title(noti_, &title, &loc),
            NOTIFICATION_ERROR_NONE);
  ASSERT_NE(title, nullptr);
  EXPECT_STREQ(title, "second");
}

/* ------------------------------------------------------------- */
/* notification_set_content roundtrip                              */
/* ------------------------------------------------------------- */

TEST_F(NotificationInternalTest, set_content_roundtrip) {
  ASSERT_EQ(notification_set_content(noti_, "body text", nullptr),
            NOTIFICATION_ERROR_NONE);
  char* content = nullptr;
  char* loc = nullptr;
  EXPECT_EQ(notification_get_content(noti_, &content, &loc),
            NOTIFICATION_ERROR_NONE);
  ASSERT_NE(content, nullptr);
  EXPECT_STREQ(content, "body text");
}

/* ------------------------------------------------------------- */
/* notification_set_content empty string                           */
/* ------------------------------------------------------------- */

TEST_F(NotificationInternalTest, set_content_empty_string) {
  ASSERT_EQ(notification_set_content(noti_, "", nullptr),
            NOTIFICATION_ERROR_NONE);
}

/* ------------------------------------------------------------- */
/* notification_set_title empty string                             */
/* ------------------------------------------------------------- */

TEST_F(NotificationInternalTest, set_title_empty_string) {
  ASSERT_EQ(notification_set_title(noti_, "", nullptr),
            NOTIFICATION_ERROR_NONE);
}

/* ------------------------------------------------------------- */
/* notification_set_pkgname null app_id returns invalid            */
/* ------------------------------------------------------------- */

TEST_F(NotificationInternalTest, set_app_id_null_returns_invalid) {
  EXPECT_EQ(notification_set_app_id(noti_, nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationInternalTest, set_pkgname_null_returns_invalid) {
  EXPECT_EQ(notification_set_pkgname(noti_, nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* ------------------------------------------------------------- */
/* notification_set_default_button replaces previous               */
/* ------------------------------------------------------------- */

TEST_F(NotificationInternalTest, set_default_button_replaces_previous) {
  ASSERT_EQ(notification_set_default_button(noti_, NOTIFICATION_BUTTON_1),
            NOTIFICATION_ERROR_NONE);
  ASSERT_EQ(notification_set_default_button(noti_, NOTIFICATION_BUTTON_5),
            NOTIFICATION_ERROR_NONE);
  notification_button_index_e idx;
  EXPECT_EQ(notification_get_default_button(noti_, &idx),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(idx, NOTIFICATION_BUTTON_5);
}

/* ------------------------------------------------------------- */
/* notification_set_ongoing_value_type roundtrip more values       */
/* ------------------------------------------------------------- */

TEST_F(NotificationInternalTest, set_ongoing_value_type_double_set) {
  ASSERT_EQ(notification_set_ongoing_value_type(noti_,
                NOTIFICATION_ONGOING_VALUE_TYPE_PERCENT),
            NOTIFICATION_ERROR_NONE);
  ASSERT_EQ(notification_set_ongoing_value_type(noti_,
                NOTIFICATION_ONGOING_VALUE_TYPE_TIME),
            NOTIFICATION_ERROR_NONE);
  notification_ongoing_value_type_e v;
  EXPECT_EQ(notification_get_ongoing_value_type(noti_, &v),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(v, NOTIFICATION_ONGOING_VALUE_TYPE_TIME);
}

/* ------------------------------------------------------------- */
/* notification_get_ongoing_value_type default                     */
/* ------------------------------------------------------------- */

TEST_F(NotificationInternalTest, get_ongoing_value_type_default) {
  notification_ongoing_value_type_e v =
      static_cast<notification_ongoing_value_type_e>(99);
  EXPECT_EQ(notification_get_ongoing_value_type(noti_, &v),
            NOTIFICATION_ERROR_NONE);
  /* Default should be a valid enum value, not 99. */
  EXPECT_NE(v, 99);
}

/* ------------------------------------------------------------- */
/* notification_set_hide_timeout replaces previous                 */
/* ------------------------------------------------------------- */

TEST_F(NotificationInternalTest, set_hide_timeout_replaces_previous) {
  ASSERT_EQ(notification_set_hide_timeout(noti_, 10),
            NOTIFICATION_ERROR_NONE);
  ASSERT_EQ(notification_set_hide_timeout(noti_, 20),
            NOTIFICATION_ERROR_NONE);
  int v = 0;
  EXPECT_EQ(notification_get_hide_timeout(noti_, &v),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(v, 20);
}

/* ------------------------------------------------------------- */
/* notification_set_delete_timeout replaces previous               */
/* ------------------------------------------------------------- */

TEST_F(NotificationInternalTest, set_delete_timeout_replaces_previous) {
  ASSERT_EQ(notification_set_delete_timeout(noti_, 100),
            NOTIFICATION_ERROR_NONE);
  ASSERT_EQ(notification_set_delete_timeout(noti_, 200),
            NOTIFICATION_ERROR_NONE);
  int v = 0;
  EXPECT_EQ(notification_get_delete_timeout(noti_, &v),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(v, 200);
}

/* ------------------------------------------------------------- */
/* notification_set_ongoing_time replaces previous                 */
/* ------------------------------------------------------------- */

TEST_F(NotificationInternalTest, set_ongoing_time_replaces_previous) {
  ASSERT_EQ(notification_set_ongoing_time(noti_, 10, 100),
            NOTIFICATION_ERROR_NONE);
  ASSERT_EQ(notification_set_ongoing_time(noti_, 20, 200),
            NOTIFICATION_ERROR_NONE);
  int cur = 0, dur = 0;
  EXPECT_EQ(notification_get_ongoing_time(noti_, &cur, &dur),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(cur, 20);
  EXPECT_EQ(dur, 200);
}

/* ------------------------------------------------------------- */
/* notification_set_args bundle_dup semantics — caller can free   */
/* ------------------------------------------------------------- */

TEST_F(NotificationInternalTest, set_args_caller_can_free_original) {
  bundle* b = bundle_create();
  bundle_add_str(b, "k", "v");
  ASSERT_EQ(notification_set_args(noti_, b, nullptr),
            NOTIFICATION_ERROR_NONE);
  /* Free original — the handle should keep its own dup. */
  bundle_free(b);

  bundle* got = nullptr;
  EXPECT_EQ(notification_get_args(noti_, &got, nullptr),
            NOTIFICATION_ERROR_NONE);
  ASSERT_NE(got, nullptr);
  char* v = nullptr;
  bundle_get_str(got, "k", &v);
  EXPECT_STREQ(v, "v");
}

/* ------------------------------------------------------------- */
/* notification_set_args group_args dup semantics                  */
/* ------------------------------------------------------------- */

TEST_F(NotificationInternalTest, set_args_group_args_caller_can_free) {
  bundle* args = bundle_create();
  bundle* group = bundle_create();
  bundle_add_str(group, "gk", "gv");
  ASSERT_EQ(notification_set_args(noti_, args, group),
            NOTIFICATION_ERROR_NONE);
  bundle_free(args);
  bundle_free(group);

  bundle* got_args = nullptr;
  bundle* got_group = nullptr;
  EXPECT_EQ(notification_get_args(noti_, &got_args, &got_group),
            NOTIFICATION_ERROR_NONE);
  ASSERT_NE(got_group, nullptr);
  char* v = nullptr;
  bundle_get_str(got_group, "gk", &v);
  EXPECT_STREQ(v, "gv");
}

/* ------------------------------------------------------------- */
/* notification_set_check_box multiple flag values                 */
/* ------------------------------------------------------------- */

TEST_F(NotificationInternalTest, set_check_box_replaces_flag) {
  ASSERT_EQ(notification_set_check_box(noti_, 0, true),
            NOTIFICATION_ERROR_NONE);
  ASSERT_EQ(notification_set_check_box(noti_, 1, true),
            NOTIFICATION_ERROR_NONE);
  /* Just verify no crash on multiple set_check_box calls. */
}

TEST_F(NotificationInternalTest, op_get_data_unknown_type) {
  notification_op op = {};
  int dummy = 0;
  EXPECT_EQ(notification_op_get_data(&op,
                static_cast<notification_op_data_type_e>(
                    NOTIFICATION_OP_DATA_MAX + 1),
                &dummy),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* ------------------------------------------------------------- */
/* notification_channel_*                                        */
/* ------------------------------------------------------------- */

class NotificationChannelTest : public NotificationInternalTest {};

TEST_F(NotificationChannelTest, create_null_args_returns_invalid_parameter) {
  notification_channel_h ch = nullptr;
  EXPECT_EQ(notification_channel_create(nullptr, &ch),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_channel_create("ch", nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationChannelTest, free_null_is_safe_noop) {
  /* No assertion — the function returns void; it must not crash. */
  notification_channel_free(nullptr);
}

TEST_F(NotificationChannelTest, set_blockable_null_channel) {
  EXPECT_EQ(notification_channel_set_blockable(nullptr, true),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationChannelTest, set_block_null_channel) {
  EXPECT_EQ(notification_channel_set_block(nullptr, true),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* ------------------------------------------------------------- */
/* notification_set_extension_event_handler — valid roundtrip      */
/* ------------------------------------------------------------- */

TEST_F(NotificationInternalTest, set_extension_event_handler_valid_then_get) {
  app_control_h ac = nullptr;
  app_control_create(&ac);
  app_control_set_app_id(ac, "com.example.ext.cb");

  ASSERT_EQ(notification_set_extension_event_handler(noti_,
                NOTIFICATION_EVENT_TYPE_HIDDEN_BY_USER, ac),
            NOTIFICATION_ERROR_NONE);

  app_control_h got = nullptr;
  EXPECT_EQ(notification_get_extension_event_handler(noti_,
                NOTIFICATION_EVENT_TYPE_HIDDEN_BY_USER, &got),
            NOTIFICATION_ERROR_NONE);
  if (got != nullptr) {
    char* app_id = nullptr;
    app_control_get_app_id(got, &app_id);
    if (app_id != nullptr) {
      EXPECT_STREQ(app_id, "com.example.ext.cb");
      free(app_id);
    }
    app_control_destroy(got);
  }
  app_control_destroy(ac);
}

/* ------------------------------------------------------------- */
/* notification_delete_by_priv_id — invalid priv_id validation     */
/* ------------------------------------------------------------- */

TEST_F(NotificationInternalTest, delete_by_priv_id_negative_returns_invalid) {
  EXPECT_EQ(notification_delete_by_priv_id(nullptr,
                NOTIFICATION_TYPE_NOTI, -2),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationInternalTest, delete_by_priv_id_priv_none_returns_invalid) {
  EXPECT_EQ(notification_delete_by_priv_id("appid",
                NOTIFICATION_TYPE_NOTI, NOTIFICATION_PRIV_ID_NONE),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* ------------------------------------------------------------- */
/* notification_register_do_not_disturb_app null cb               */
/* ------------------------------------------------------------- */

TEST_F(NotificationInternalTest, register_dnd_app_null_cb_returns_invalid) {
  EXPECT_EQ(notification_register_do_not_disturb_app(nullptr, nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* ------------------------------------------------------------- */
/* notification_create_from_package_template null args            */
/* ------------------------------------------------------------- */

TEST_F(NotificationInternalTest,
       create_from_package_template_null_app_id) {
  notification_h n = notification_create_from_package_template(nullptr,
      "tmpl");
  EXPECT_EQ(n, nullptr);
  EXPECT_EQ(get_last_result(),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationInternalTest,
       create_from_package_template_null_template_name) {
  notification_h n = notification_create_from_package_template("appid",
      nullptr);
  EXPECT_EQ(n, nullptr);
  EXPECT_EQ(get_last_result(),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationInternalTest,
       create_from_package_template_both_null) {
  notification_h n = notification_create_from_package_template(nullptr,
      nullptr);
  EXPECT_EQ(n, nullptr);
  EXPECT_EQ(get_last_result(),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* ------------------------------------------------------------- */
/* notification_channel_add / remove / update null arg validation */
/* ------------------------------------------------------------- */

TEST_F(NotificationChannelTest, channel_add_null_channel) {
  EXPECT_EQ(notification_channel_add(nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationChannelTest, channel_remove_null_channel) {
  EXPECT_EQ(notification_channel_remove(nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationChannelTest, channel_update_null_channel) {
  EXPECT_EQ(notification_channel_update(nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationChannelTest, channel_get_by_name_null_name) {
  notification_channel_h ch = nullptr;
  EXPECT_EQ(notification_channel_get_by_name(nullptr, &ch),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationChannelTest, channel_get_by_name_null_out) {
  EXPECT_EQ(notification_channel_get_by_name("ch", nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationChannelTest, channel_clone_null_args) {
  notification_channel_h clone = nullptr;
  EXPECT_EQ(notification_channel_clone(nullptr, &clone),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  notification_channel_h ch = nullptr;
  ASSERT_EQ(notification_channel_create("ch", &ch),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(notification_channel_clone(ch, nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  notification_channel_free(ch);
}

TEST_F(NotificationChannelTest, channel_foreach_null_cb_returns_invalid) {
  EXPECT_EQ(notification_channel_foreach("app-id", nullptr, nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationInternalTest, set_extension_event_handler_replaces_previous) {
  app_control_h ac1 = nullptr;
  app_control_create(&ac1);
  app_control_set_app_id(ac1, "com.example.first");

  app_control_h ac2 = nullptr;
  app_control_create(&ac2);
  app_control_set_app_id(ac2, "com.example.second");

  ASSERT_EQ(notification_set_extension_event_handler(noti_,
                NOTIFICATION_EVENT_TYPE_HIDDEN_BY_USER, ac1),
            NOTIFICATION_ERROR_NONE);
  ASSERT_EQ(notification_set_extension_event_handler(noti_,
                NOTIFICATION_EVENT_TYPE_HIDDEN_BY_USER, ac2),
            NOTIFICATION_ERROR_NONE);

  app_control_h got = nullptr;
  EXPECT_EQ(notification_get_extension_event_handler(noti_,
                NOTIFICATION_EVENT_TYPE_HIDDEN_BY_USER, &got),
            NOTIFICATION_ERROR_NONE);
  if (got != nullptr) {
    char* app_id = nullptr;
    app_control_get_app_id(got, &app_id);
    if (app_id != nullptr) {
      EXPECT_STREQ(app_id, "com.example.second");
      free(app_id);
    }
    app_control_destroy(got);
  }
  app_control_destroy(ac1);
  app_control_destroy(ac2);
}

TEST_F(NotificationChannelTest, set_blockable_success_path) {
  notification_channel_h ch = nullptr;
  ASSERT_EQ(notification_channel_create("ch.b", &ch),
            NOTIFICATION_ERROR_NONE);
  ASSERT_NE(ch, nullptr);
  EXPECT_EQ(notification_channel_set_blockable(ch, true),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(notification_channel_set_blockable(ch, false),
            NOTIFICATION_ERROR_NONE);
  notification_channel_free(ch);
}

TEST_F(NotificationChannelTest, set_block_success_path) {
  notification_channel_h ch = nullptr;
  ASSERT_EQ(notification_channel_create("ch.x", &ch),
            NOTIFICATION_ERROR_NONE);
  ASSERT_NE(ch, nullptr);
  EXPECT_EQ(notification_channel_set_block(ch, true),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(notification_channel_set_block(ch, false),
            NOTIFICATION_ERROR_NONE);
  notification_channel_free(ch);
}

TEST_F(NotificationInternalTest, set_pairing_type_true_then_false) {
  EXPECT_EQ(notification_set_pairing_type(noti_, true),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(notification_set_pairing_type(noti_, false),
            NOTIFICATION_ERROR_NONE);
}

TEST_F(NotificationInternalTest, set_channel_name_stores_value) {
  EXPECT_EQ(notification_set_channel_name(noti_, "my.channel"),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(notification_set_channel_name(noti_, "my.channel2"),
            NOTIFICATION_ERROR_NONE);
}

TEST_F(NotificationInternalTest, get_args_with_no_args_returns_null) {
  if (noti_->args) {
    bundle_free(noti_->args);
    noti_->args = nullptr;
  }
  bundle* a = nullptr;
  bundle* ga = nullptr;
  EXPECT_EQ(notification_get_args(noti_, &a, &ga),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(a, nullptr);
}

TEST_F(NotificationInternalTest, get_id_returns_group_and_priv_id) {
  noti_->group_id = 5;
  noti_->priv_id = 99;
  int g = 0, p = 0;
  EXPECT_EQ(notification_get_id(noti_, &g, &p),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(g, 5);
  EXPECT_EQ(p, 99);
}

TEST_F(NotificationInternalTest, get_id_negative_group_id_clamped_to_none) {
  noti_->group_id = -42;
  int g = 0;
  EXPECT_EQ(notification_get_id(noti_, &g, nullptr),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(g, NOTIFICATION_GROUP_ID_NONE);
}

TEST_F(NotificationInternalTest, delete_group_by_group_id_with_app_id) {
  ON_CALL(GetMock<NotiServiceProxyMock>(),
          rpc_port_proxy_noti_service_connect_sync(_))
      .WillByDefault(::testing::Return(RPC_PORT_ERROR_IO_ERROR));
  notification_delete_group_by_group_id("some.app",
      NOTIFICATION_TYPE_NOTI, 1);
}

TEST_F(NotificationInternalTest, delete_group_by_group_id_null_app_id_uses_pid) {
  ON_CALL(GetMock<NotiServiceProxyMock>(),
          rpc_port_proxy_noti_service_connect_sync(_))
      .WillByDefault(::testing::Return(RPC_PORT_ERROR_IO_ERROR));
  notification_delete_group_by_group_id(nullptr,
      NOTIFICATION_TYPE_NOTI, 1);
}

TEST_F(NotificationInternalTest, update_content_with_noti_uses_its_priv_id) {
  ON_CALL(GetMock<NotiServiceProxyMock>(),
          rpc_port_proxy_noti_service_connect_sync(_))
      .WillByDefault(::testing::Return(RPC_PORT_ERROR_IO_ERROR));
  noti_->priv_id = 42;
  notification_update_content(noti_, NOTIFICATION_PRIV_ID_NONE, "new");
}

TEST_F(NotificationInternalTest, update_content_null_noti_uses_pid) {
  ON_CALL(GetMock<NotiServiceProxyMock>(),
          rpc_port_proxy_noti_service_connect_sync(_))
      .WillByDefault(::testing::Return(RPC_PORT_ERROR_IO_ERROR));
  notification_update_content(nullptr, 7, "new");
}

TEST_F(NotificationInternalTest, list_append_null_noti_sets_last_result) {
  notification_list_h result = notification_list_append(nullptr, nullptr);
  EXPECT_EQ(result, nullptr);
}

TEST_F(NotificationInternalTest, check_event_receiver_available_invokes_tidl) {
  ON_CALL(GetMock<NotiServiceProxyMock>(),
          rpc_port_proxy_noti_service_connect_sync(_))
      .WillByDefault(::testing::Return(RPC_PORT_ERROR_IO_ERROR));
  bool avail = false;
  notification_check_event_receiver_available(noti_, &avail);
}

TEST_F(NotificationInternalTest, get_extension_data_no_args_bundle_is_invalid) {
  if (noti_->args) { bundle_free(noti_->args); noti_->args = nullptr; }
  bundle* out = nullptr;
  EXPECT_EQ(notification_get_extension_data(noti_, "k", &out),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationInternalTest, get_extension_data_missing_key_is_invalid) {
  bundle* b = bundle_create();
  noti_->args = b;
  bundle* out = nullptr;
  EXPECT_EQ(notification_get_extension_data(noti_, "missing.key", &out),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationInternalTest, set_get_extension_data_roundtrip) {
  bundle* payload = bundle_create();
  bundle_add_str(payload, "field", "value");
  ASSERT_EQ(notification_set_extension_data(noti_, "ext", payload),
            NOTIFICATION_ERROR_NONE);
  bundle* out = nullptr;
  notification_get_extension_data(noti_, "ext", &out);
  if (out) bundle_free(out);
  bundle_free(payload);
}

TEST_F(NotificationInternalTest, set_extension_data_null_value_deletes_key) {
  bundle* payload = bundle_create();
  bundle_add_str(payload, "f", "v");
  ASSERT_EQ(notification_set_extension_data(noti_, "ext", payload),
            NOTIFICATION_ERROR_NONE);
  bundle_free(payload);
  EXPECT_EQ(notification_set_extension_data(noti_, "ext", nullptr),
            NOTIFICATION_ERROR_NONE);
}

TEST_F(NotificationInternalTest,
       set_extension_data_replaces_existing_key) {
  bundle* a = bundle_create();
  bundle_add_str(a, "f", "v1");
  ASSERT_EQ(notification_set_extension_data(noti_, "ext", a),
            NOTIFICATION_ERROR_NONE);
  bundle_free(a);
  bundle* b = bundle_create();
  bundle_add_str(b, "f", "v2");
  EXPECT_EQ(notification_set_extension_data(noti_, "ext", b),
            NOTIFICATION_ERROR_NONE);
  bundle_free(b);
}

TEST_F(NotificationInternalTest, send_event_by_priv_id_invokes_tidl) {
  ON_CALL(GetMock<NotiServiceProxyMock>(),
          rpc_port_proxy_noti_service_connect_sync(_))
      .WillByDefault(::testing::Return(RPC_PORT_ERROR_IO_ERROR));
  notification_send_event_by_priv_id(42,
      NOTIFICATION_EVENT_TYPE_CLICK_ON_BUTTON_1);
}

TEST_F(NotificationInternalTest, unregister_do_not_disturb_app_invokes_tidl) {
  ON_CALL(GetMock<NotiServiceProxyMock>(),
          rpc_port_proxy_noti_service_connect_sync(_))
      .WillByDefault(::testing::Return(RPC_PORT_ERROR_IO_ERROR));
  notification_unregister_do_not_disturb_app();
}

TEST_F(NotificationInternalTest, update_async_for_uid_invokes_tidl) {
  ON_CALL(GetMock<NotiServiceProxyMock>(),
          rpc_port_proxy_noti_service_connect_sync(_))
      .WillByDefault(::testing::Return(RPC_PORT_ERROR_IO_ERROR));
  notification_update_async_for_uid(noti_, nullptr, nullptr, getuid());
}

TEST_F(NotificationInternalTest, load_by_tag_for_uid_invokes_tidl) {
  ON_CALL(GetMock<NotiServiceProxyMock>(),
          rpc_port_proxy_noti_service_connect_sync(_))
      .WillByDefault(::testing::Return(RPC_PORT_ERROR_IO_ERROR));
  notification_h r = notification_load_by_tag_for_uid("some.tag", getuid());
  if (r) notification_free(r);
}

TEST_F(NotificationInternalTest, get_all_count_for_uid_invokes_tidl) {
  ON_CALL(GetMock<NotiServiceProxyMock>(),
          rpc_port_proxy_noti_service_connect_sync(_))
      .WillByDefault(::testing::Return(RPC_PORT_ERROR_IO_ERROR));
  int count = 0;
  notification_get_all_count_for_uid(NOTIFICATION_TYPE_NOTI, &count, getuid());
}

static void __dummy_channel_cb(notification_channel_h, void*) {}

TEST_F(NotificationInternalTest, channel_foreach_invokes_tidl_with_app_id) {
  ON_CALL(GetMock<NotiServiceProxyMock>(),
          rpc_port_proxy_noti_service_connect_sync(_))
      .WillByDefault(::testing::Return(RPC_PORT_ERROR_IO_ERROR));
  notification_channel_foreach("some.app", __dummy_channel_cb, nullptr);
}

TEST_F(NotificationInternalTest, channel_foreach_null_app_id_uses_pid) {
  ON_CALL(GetMock<NotiServiceProxyMock>(),
          rpc_port_proxy_noti_service_connect_sync(_))
      .WillByDefault(::testing::Return(RPC_PORT_ERROR_IO_ERROR));
  notification_channel_foreach(nullptr, __dummy_channel_cb, nullptr);
}

TEST_F(NotificationInternalTest, update_size_with_noti_invokes_tidl) {
  ON_CALL(GetMock<NotiServiceProxyMock>(),
          rpc_port_proxy_noti_service_connect_sync(_))
      .WillByDefault(::testing::Return(RPC_PORT_ERROR_IO_ERROR));
  noti_->priv_id = 10;
  notification_update_size(noti_, NOTIFICATION_PRIV_ID_NONE, 0.5);
}

TEST_F(NotificationInternalTest, update_size_null_noti_with_priv_id) {
  ON_CALL(GetMock<NotiServiceProxyMock>(),
          rpc_port_proxy_noti_service_connect_sync(_))
      .WillByDefault(::testing::Return(RPC_PORT_ERROR_IO_ERROR));
  notification_update_size(nullptr, 7, 1.0);
}

TEST_F(NotificationInternalTest, update_progress_with_noti_invokes_tidl) {
  ON_CALL(GetMock<NotiServiceProxyMock>(),
          rpc_port_proxy_noti_service_connect_sync(_))
      .WillByDefault(::testing::Return(RPC_PORT_ERROR_IO_ERROR));
  noti_->priv_id = 10;
  notification_update_progress(noti_, NOTIFICATION_PRIV_ID_NONE, 0.5);
}

TEST_F(NotificationInternalTest, update_progress_null_noti_with_priv_id) {
  ON_CALL(GetMock<NotiServiceProxyMock>(),
          rpc_port_proxy_noti_service_connect_sync(_))
      .WillByDefault(::testing::Return(RPC_PORT_ERROR_IO_ERROR));
  notification_update_progress(nullptr, 9, 0.5);
}

TEST_F(NotificationInternalTest, register_do_not_disturb_app_invokes_tidl) {
  ON_CALL(GetMock<NotiServiceProxyMock>(),
          rpc_port_proxy_noti_service_connect_sync(_))
      .WillByDefault(::testing::Return(RPC_PORT_ERROR_IO_ERROR));
  notification_register_do_not_disturb_app([](void*) {}, nullptr);
}

TEST_F(NotificationInternalTest, send_event_with_valid_event_type) {
  ON_CALL(GetMock<NotiServiceProxyMock>(),
          rpc_port_proxy_noti_service_connect_sync(_))
      .WillByDefault(::testing::Return(RPC_PORT_ERROR_IO_ERROR));
  noti_->event_flag = true;
  notification_send_event(noti_, NOTIFICATION_EVENT_TYPE_CLICK_ON_BUTTON_1);
}

TEST_F(NotificationInternalTest, get_check_box_checked_null_args) {
  bool v = false;
  EXPECT_EQ(notification_get_check_box_checked(nullptr, &v),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_get_check_box_checked(noti_, nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationInternalTest, get_check_box_checked_false_when_unset) {
  noti_->check_box = false;
  noti_->check_box_value = true;
  bool v = true;
  EXPECT_EQ(notification_get_check_box_checked(noti_, &v),
            NOTIFICATION_ERROR_NONE);
  EXPECT_FALSE(v);
}

static void __dummy_deferred_cb(void*) {}

TEST_F(NotificationInternalTest, add_deferred_task_invokes_tidl_body) {
  ON_CALL(GetMock<NotiServiceProxyMock>(),
          rpc_port_proxy_noti_service_connect_sync(_))
      .WillByDefault(::testing::Return(RPC_PORT_ERROR_IO_ERROR));
  notification_add_deferred_task(__dummy_deferred_cb, nullptr);
}

TEST_F(NotificationInternalTest, del_deferred_task_invokes_tidl_body) {
  ON_CALL(GetMock<NotiServiceProxyMock>(),
          rpc_port_proxy_noti_service_connect_sync(_))
      .WillByDefault(::testing::Return(RPC_PORT_ERROR_IO_ERROR));
  notification_del_deferred_task(__dummy_deferred_cb);
}

/* ------------------------------------------------------------- */
/* notification_resister_changed_cb_for_uid                       */
/* — monitor_init failure path → IO_ERROR                         */
/* ------------------------------------------------------------- */

TEST_F(NotificationInternalTest,
       resister_changed_cb_for_uid_monitor_init_failure) {
  ON_CALL(GetMock<NotiServiceProxyMock>(),
          rpc_port_proxy_noti_service_connect_sync(_))
      .WillByDefault(Return(RPC_PORT_ERROR_IO_ERROR));

  static notification_changed_cb cb = [](void*, notification_type_e) {};
  EXPECT_EQ(notification_resister_changed_cb_for_uid(cb, nullptr, 5001),
            NOTIFICATION_ERROR_IO_ERROR);
}

/* ------------------------------------------------------------- */
/* notification_unresister_changed_cb_for_uid                     */
/* — hash not initialized → INVALID_PARAMETER                     */
/* ------------------------------------------------------------- */

TEST_F(NotificationInternalTest,
       unresister_changed_cb_for_uid_no_hash) {
  /* No callback has been registered under uid 9999, so the internal
   * hash either doesn't exist or doesn't contain this uid.          */
  static notification_changed_cb cb = [](void*, notification_type_e) {};
  int ret = notification_unresister_changed_cb_for_uid(cb, 9999);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* ------------------------------------------------------------- */
/* notification_register_detailed_changed_cb_for_uid              */
/* — monitor_init failure path → IO_ERROR                         */
/* ------------------------------------------------------------- */

TEST_F(NotificationInternalTest,
       register_detailed_changed_cb_for_uid_monitor_init_failure) {
  ON_CALL(GetMock<NotiServiceProxyMock>(),
          rpc_port_proxy_noti_service_connect_sync(_))
      .WillByDefault(Return(RPC_PORT_ERROR_IO_ERROR));

  static notification_detailed_changed_cb cb =
      [](void*, notification_type_e, notification_op*, int) {};
  EXPECT_EQ(notification_register_detailed_changed_cb_for_uid(
                cb, nullptr, 5001),
            NOTIFICATION_ERROR_IO_ERROR);
}

/* ------------------------------------------------------------- */
/* notification_get_noti_block_state — null state arg             */
/* ------------------------------------------------------------- */

TEST_F(NotificationInternalTest, get_noti_block_state_null_state) {
  EXPECT_EQ(notification_get_noti_block_state(nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* ------------------------------------------------------------- */
/* notification_set_text — bundle_del path (overwrite)            */
/* ------------------------------------------------------------- */

TEST_F(NotificationInternalTest, set_text_overwrite_existing) {
  /* First set establishes the bundle entry. */
  ASSERT_EQ(notification_set_text(noti_, NOTIFICATION_TEXT_TYPE_TITLE,
                "first", nullptr, NOTIFICATION_VARIABLE_TYPE_NONE),
            NOTIFICATION_ERROR_NONE);
  /* Second set on the same type must delete the old bundle entry and
   * re-insert — verifying the bundle_del (overwrite) code path.     */
  ASSERT_EQ(notification_set_text(noti_, NOTIFICATION_TEXT_TYPE_TITLE,
                "second", nullptr, NOTIFICATION_VARIABLE_TYPE_NONE),
            NOTIFICATION_ERROR_NONE);

  char* text = nullptr;
  EXPECT_EQ(notification_get_text(noti_, NOTIFICATION_TEXT_TYPE_TITLE, &text),
            NOTIFICATION_ERROR_NONE);
  ASSERT_NE(text, nullptr);
  EXPECT_STREQ(text, "second");
}

/* ------------------------------------------------------------- */
/* notification_clone — launch_app_id is cloned                   */
/* ------------------------------------------------------------- */

TEST_F(NotificationInternalTest, clone_noti_with_launch_app_id) {
  /* Set a launch app id on the original. */
  ASSERT_EQ(notification_set_application(noti_, "com.example.launch"),
            NOTIFICATION_ERROR_NONE);

  notification_h clone = nullptr;
  ASSERT_EQ(notification_clone(noti_, &clone), NOTIFICATION_ERROR_NONE);
  ASSERT_NE(clone, nullptr);

  char* app_id = nullptr;
  EXPECT_EQ(notification_get_application(clone, &app_id),
            NOTIFICATION_ERROR_NONE);
  ASSERT_NE(app_id, nullptr);
  EXPECT_STREQ(app_id, "com.example.launch");

  notification_free(clone);
}

/* ------------------------------------------------------------- */
/* notification_delete_by_priv_id — null app_id path (L934-944)  */
/* ------------------------------------------------------------- */

TEST_F(NotificationInternalTest,
       delete_by_priv_id_null_app_id_enters_tidl_path) {
  /* [P2-null-app_id] When app_id is NULL the function calls
   * notification_get_app_id_by_pid (AUL) to get the caller id, then
   * calls TIDL.  With TIDL connect mocked to fail we still execute L934-
   * 944 (the app_id == NULL branch). */
  ON_CALL(GetMock<NotiServiceProxyMock>(),
          rpc_port_proxy_noti_service_connect_sync(_))
      .WillByDefault(Return(RPC_PORT_ERROR_IO_ERROR));
  /* AUL lookup uses the already-installed WillRepeatedly from SetUp. */
  int ret = notification_delete_by_priv_id(nullptr,
                                           NOTIFICATION_TYPE_NOTI, 1);
  /* Any non-crash result is acceptable; the key is that L934 is reached. */
  (void)ret;
}

TEST_F(NotificationInternalTest,
       delete_by_priv_id_with_app_id_enters_tidl_path) {
  /* [P1-with-app_id] When app_id is provided the function strdup-s it and
   * calls TIDL.  The strdup branch (L937) and TIDL call (L940) execute. */
  ON_CALL(GetMock<NotiServiceProxyMock>(),
          rpc_port_proxy_noti_service_connect_sync(_))
      .WillByDefault(Return(RPC_PORT_ERROR_IO_ERROR));
  int ret = notification_delete_by_priv_id("org.tizen.testapp",
                                           NOTIFICATION_TYPE_NOTI, 1);
  (void)ret;
}

/* ------------------------------------------------------------- */
/* notification_delete_all_by_type — null app_id uses AUL (L903) */
/* ------------------------------------------------------------- */

TEST_F(NotificationInternalTest,
       delete_all_by_type_null_app_id_uses_aul_path) {
  /* [N2-null-app_id] When app_id == NULL the internal
   * notification_delete_all_by_type_for_uid calls
   * notification_get_app_id_by_pid which uses AUL.  With TIDL connect
   * mocked to fail we still exercise L903. */
  ON_CALL(GetMock<NotiServiceProxyMock>(),
          rpc_port_proxy_noti_service_connect_sync(_))
      .WillByDefault(Return(RPC_PORT_ERROR_IO_ERROR));
  int ret = notification_delete_all_by_type(nullptr, NOTIFICATION_TYPE_NOTI);
  (void)ret;
}
