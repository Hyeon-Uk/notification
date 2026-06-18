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

#include <dlfcn.h>

#include "mock/test_fixture.hh"
#include "mock/aul_mock.hh"
#include "mock/config_mock.hh"
#include "mock/glib_mock.hh"
#include "mock/noti_service_proxy_mock.hh"
#include "mock/rpc_port_mock.hh"
#include "mock/sqlite_mock.hh"
#include "mock/package_manager_mock.hh"

#include "notification/include/notification.h"
#include "notification/include/notification_setting.h"
#include "notification/include/notification_setting_internal.h"

namespace {

class Mocks :
    virtual public ::testing::NiceMock<AulMock>,
    virtual public ::testing::NiceMock<ConfigMock>,
    virtual public ::testing::NiceMock<GlibMock>,
    virtual public ::testing::NiceMock<PackageManagerMock>,
    virtual public ::testing::NiceMock<SqliteMock> {};

/* Extended Mocks that include IPC layer for TIDL-reachable paths. */
class MocksWithTidl :
    virtual public ::testing::NiceMock<AulMock>,
    virtual public ::testing::NiceMock<ConfigMock>,
    virtual public ::testing::NiceMock<GlibMock>,
    virtual public ::testing::NiceMock<NotiServiceProxyMock>,
    virtual public ::testing::NiceMock<PackageManagerMock>,
    virtual public ::testing::NiceMock<RpcPortMock>,
    virtual public ::testing::NiceMock<SqliteMock> {};

int __setting_fake_aul_app_get_appid_bypid(int pid, char* appid, int len) {
  snprintf(appid, len, "%s", "org.tizen.settingtest");
  return AUL_R_OK;
}

int __setting_fake_sqlite3_open_v2(const char*, sqlite3** out, int, const char*) {
  char** table = (char**)calloc(10, sizeof(char*));
  *out = (sqlite3*)table;
  return SQLITE_OK;
}

static char __fake_stmt_buf[16] = {};
static sqlite3_stmt* __fake_stmt_ptr = reinterpret_cast<sqlite3_stmt*>(__fake_stmt_buf);

static char __fake_filter_buf[8] = {};
static pkgmgrinfo_appinfo_filter_h __fake_filter_ptr =
    reinterpret_cast<pkgmgrinfo_appinfo_filter_h>(__fake_filter_buf);

static char __fake_appinfo_buf[8] = {};
static pkgmgrinfo_appinfo_h __fake_appinfo_ptr =
    reinterpret_cast<pkgmgrinfo_appinfo_h>(__fake_appinfo_buf);

static char __fake_app_id_str[] = "org.test.app";
static char __fake_pkg_name_str[] = "org.test.pkg";
static char __fake_empty_str[] = "";

static int __invoke_foreach_callback(pkgmgrinfo_appinfo_filter_h /*filter*/,
    pkgmgrinfo_app_list_cb cb, void *user_data, uid_t /*uid*/) {
  cb(__fake_appinfo_ptr, user_data);
  return PMINFO_R_OK;
}

} // namespace

using ::testing::_;
using ::testing::Invoke;
using ::testing::Return;
using ::testing::AtLeast;
using ::testing::DoAll;
using ::testing::SetArgPointee;

class NotificationSettingTest : public TestFixture {
 public:
  NotificationSettingTest() : TestFixture(std::make_unique<::Mocks>()) {}
  virtual void SetUp() {
    ON_CALL(GetMock<AulMock>(), aul_getuid()).WillByDefault(Return(5001));
  }
  virtual void TearDown() {
  }
};

/* ============================================================== */
/* notification_setting_get_setting — AUL failure (N1)            */
/* ============================================================== */

TEST_F(NotificationSettingTest, setting_get_setting_aul_fail_returns_not_exist) {
  /* [N1] AUL pid lookup fails → NOT_EXIST_ID before IPC */
  ON_CALL(GetMock<AulMock>(), aul_app_get_appid_bypid(_, _, _))
      .WillByDefault(Return(AUL_R_ERROR));
  notification_setting_h setting = nullptr;
  EXPECT_EQ(notification_setting_get_setting(&setting),
            NOTIFICATION_ERROR_NOT_EXIST_ID);
}

TEST_F(NotificationSettingTest, notification_setting_test) {
  int ret;

  notification_setting_h setting;
  setting = (struct notification_setting *)malloc(sizeof(struct notification_setting));

  setting->package_name = strdup("org.tizen.testpkg");
  setting->app_id = strdup("org.tizen.testapp");
  setting->allow_to_notify = true;
  setting->do_not_disturb_except = false;
  setting->pop_up_notification = false;
  setting->visibility_class = 1;
  setting->lock_screen_content_level = SHOW_ALL_CONTENT;
  setting->app_disabled = false;

  int visibility;
  ret = notification_setting_get_visibility_class(setting, &visibility);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  ret = notification_setting_set_visibility_class(setting, 3);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  ret = notification_setting_update_setting(nullptr);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);

  ret = notification_setting_free_notification(setting);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);
}

/* ============================================================== */
/* notification_setting_refresh_setting_table — DB fail (N1)      */
/* ============================================================== */

TEST_F(NotificationSettingTest,
       refresh_setting_table_db_fail_returns_error) {
  /* TV profile: TIZEN_FEATURE_ENABLE_NOTI_SETTING is false, so DB path is reached */
  ON_CALL(GetMock<ConfigMock>(), _config_get_tizen_profile())
      .WillByDefault(Return(TIZEN_PROFILE_TV));
  /* [N1] sqlite3_open_v2 fails → db_open returns NULL → non-NONE error */
  ON_CALL(GetMock<SqliteMock>(), sqlite3_open_v2(_, _, _, _))
      .WillByDefault(Return(SQLITE_ERROR));
  int ret = notification_setting_refresh_setting_table(5001);
  EXPECT_NE(ret, NOTIFICATION_ERROR_NONE);
}

/* ============================================================== */
/* notification_system_setting_init_system_setting_table — DB (N1) */
/* ============================================================== */

TEST_F(NotificationSettingTest,
       system_setting_init_table_db_fail_returns_error) {
  /* [N1] sqlite3_open_v2 fails → db_open returns NULL → non-NONE error */
  ON_CALL(GetMock<SqliteMock>(), sqlite3_open_v2(_, _, _, _))
      .WillByDefault(Return(SQLITE_ERROR));
  int ret = notification_system_setting_init_system_setting_table(5001);
  EXPECT_NE(ret, NOTIFICATION_ERROR_NONE);
}

/* ============================================================== */
/* notification_system_setting_init_system_setting_table — L796-841 */
/* ============================================================== */

/* C1: uid already in table → skip INSERTs → NONE (covers L796-798) */
TEST_F(NotificationSettingTest,
       system_setting_init_table_uid_already_present_skips_inserts) {
  ON_CALL(GetMock<SqliteMock>(), sqlite3_open_v2(_, _, _, _))
      .WillByDefault(Invoke(__setting_fake_sqlite3_open_v2));
  /* SELECT step returns SQLITE_ROW → uid found → _is_uid_in_system_setting_table true */
  ON_CALL(GetMock<SqliteMock>(), sqlite3_step(_))
      .WillByDefault(Return(SQLITE_ROW));
  ON_CALL(GetMock<SqliteMock>(), sqlite3_close(_))
      .WillByDefault(Return(SQLITE_OK));
  int ret = notification_system_setting_init_system_setting_table(5001);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
}

/* P1: uid NOT in table → both INSERTs succeed → NONE (covers L801-842) */
TEST_F(NotificationSettingTest,
       system_setting_init_table_uid_not_present_inserts_both_rows) {
  ON_CALL(GetMock<SqliteMock>(), sqlite3_open_v2(_, _, _, _))
      .WillByDefault(Invoke(__setting_fake_sqlite3_open_v2));
  /* First step (SELECT) → DONE (uid not in table); subsequent steps (INSERTs) → OK */
  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_step(_))
      .WillOnce(Return(SQLITE_DONE))
      .WillRepeatedly(Return(SQLITE_OK));
  ON_CALL(GetMock<SqliteMock>(), sqlite3_close(_))
      .WillByDefault(Return(SQLITE_OK));
  int ret = notification_system_setting_init_system_setting_table(5001);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
}

/* N2: uid NOT in table, first INSERT prepare fails → FROM_DB (covers L812-814) */
TEST_F(NotificationSettingTest,
       system_setting_init_table_first_insert_prepare_fail_returns_error) {
  ON_CALL(GetMock<SqliteMock>(), sqlite3_open_v2(_, _, _, _))
      .WillByDefault(Invoke(__setting_fake_sqlite3_open_v2));
  /* SELECT: prepare OK, step DONE; INSERT1: prepare fails */
  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_prepare_v2(_, _, _, _, _))
      .WillOnce(Return(SQLITE_OK))      /* SELECT prepare */
      .WillOnce(Return(SQLITE_ERROR));  /* INSERT1 prepare → FROM_DB */
  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_step(_))
      .WillOnce(Return(SQLITE_DONE));   /* SELECT step */
  ON_CALL(GetMock<SqliteMock>(), sqlite3_close(_))
      .WillByDefault(Return(SQLITE_OK));
  int ret = notification_system_setting_init_system_setting_table(5001);
  EXPECT_NE(ret, NOTIFICATION_ERROR_NONE);
}

/* E3: SELECT prepare fails → _is_uid_in_system_setting_table false → INSERTs proceed */
/* Covers L753-758 (prepare-fail path) and confirms INSERTs still execute (L801-842) */
TEST_F(NotificationSettingTest,
       system_setting_init_table_select_prepare_fail_proceeds_to_inserts) {
  ON_CALL(GetMock<SqliteMock>(), sqlite3_open_v2(_, _, _, _))
      .WillByDefault(Invoke(__setting_fake_sqlite3_open_v2));
  /* SELECT prepare fails → _is_uid_in_system_setting_table returns false;
   * INSERT1 and INSERT2 prepare succeed */
  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_prepare_v2(_, _, _, _, _))
      .WillOnce(Return(SQLITE_ERROR))     /* SELECT prepare */
      .WillRepeatedly(Return(SQLITE_OK)); /* INSERTs */
  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_step(_))
      .WillRepeatedly(Return(SQLITE_OK)); /* both INSERT steps */
  ON_CALL(GetMock<SqliteMock>(), sqlite3_close(_))
      .WillByDefault(Return(SQLITE_OK));
  int ret = notification_system_setting_init_system_setting_table(5001);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
}

/* ============================================================== */
/* notification_system_setting_init_system_setting_table           */
/* sqlite3_mprintf OOM paths: L749-750, L807-809, L822-824         */
/* ============================================================== */

/* L749-750: sqlite3_mprintf fails for SELECT → _is_uid_in_system_setting_table OOM */
TEST_F(NotificationSettingTest,
       system_setting_init_table_select_mprintf_oom_covers_L749_L750) {
  ON_CALL(GetMock<SqliteMock>(), sqlite3_open_v2(_, _, _, _))
      .WillByDefault(Invoke(__setting_fake_sqlite3_open_v2));
  ON_CALL(GetMock<SqliteMock>(), sqlite3_close(_))
      .WillByDefault(Return(SQLITE_OK));
  ON_CALL(GetMock<SqliteMock>(), sqlite3_prepare_v2(_, _, _, _, _))
      .WillByDefault(Return(SQLITE_ERROR));
  sqlite3_mprintf_set_fail_on(0);
  int ret = notification_system_setting_init_system_setting_table(5001);
  sqlite3_mprintf_reset_fail();
  /* mprintf fails → _is_uid_in_system_setting_table returns OOM (non-zero → bool true)
   * → caller skips INSERT → NOTIFICATION_ERROR_NONE */
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
}

/* L807-809: sqlite3_mprintf fails for first INSERT (SELECT succeeds, uid not in table) */
TEST_F(NotificationSettingTest,
       system_setting_init_table_insert1_mprintf_oom_covers_L807_L809) {
  ON_CALL(GetMock<SqliteMock>(), sqlite3_open_v2(_, _, _, _))
      .WillByDefault(Invoke(__setting_fake_sqlite3_open_v2));
  ON_CALL(GetMock<SqliteMock>(), sqlite3_close(_))
      .WillByDefault(Return(SQLITE_OK));
  ON_CALL(GetMock<SqliteMock>(), sqlite3_prepare_v2(_, _, _, _, _))
      .WillByDefault(Return(SQLITE_OK));
  ON_CALL(GetMock<SqliteMock>(), sqlite3_step(_))
      .WillByDefault(Return(SQLITE_DONE));
  ON_CALL(GetMock<SqliteMock>(), sqlite3_finalize(_))
      .WillByDefault(Return(SQLITE_OK));
  sqlite3_mprintf_set_fail_on(1);  /* call 0=SELECT ok, call 1=INSERT1 fails */
  int ret = notification_system_setting_init_system_setting_table(5001);
  sqlite3_mprintf_reset_fail();
  EXPECT_EQ(ret, NOTIFICATION_ERROR_OUT_OF_MEMORY);
}

/* L822-824: sqlite3_mprintf fails for second INSERT (SELECT ok, INSERT1 ok) */
TEST_F(NotificationSettingTest,
       system_setting_init_table_insert2_mprintf_oom_covers_L822_L824) {
  ON_CALL(GetMock<SqliteMock>(), sqlite3_open_v2(_, _, _, _))
      .WillByDefault(Invoke(__setting_fake_sqlite3_open_v2));
  ON_CALL(GetMock<SqliteMock>(), sqlite3_close(_))
      .WillByDefault(Return(SQLITE_OK));
  ON_CALL(GetMock<SqliteMock>(), sqlite3_prepare_v2(_, _, _, _, _))
      .WillByDefault(Return(SQLITE_OK));
  ON_CALL(GetMock<SqliteMock>(), sqlite3_step(_))
      .WillByDefault(Return(SQLITE_DONE));
  ON_CALL(GetMock<SqliteMock>(), sqlite3_finalize(_))
      .WillByDefault(Return(SQLITE_OK));
  sqlite3_mprintf_set_fail_on(2);  /* call 0=SELECT ok, call 1=INSERT1 ok, call 2=INSERT2 fails */
  int ret = notification_system_setting_init_system_setting_table(5001);
  sqlite3_mprintf_reset_fail();
  EXPECT_EQ(ret, NOTIFICATION_ERROR_OUT_OF_MEMORY);
}

/* ============================================================== */
/* notification_setting_refresh_setting_table — pkgmgrinfo paths  */
/* (L370-402, TV profile only)                                     */
/* ============================================================== */

/* N2: filter_create fails → FROM_DB (covers L371-374) */
TEST_F(NotificationSettingTest,
       refresh_setting_table_filter_create_fail_returns_error) {
  ON_CALL(GetMock<ConfigMock>(), _config_get_tizen_profile())
      .WillByDefault(Return(TIZEN_PROFILE_TV));
  ON_CALL(GetMock<SqliteMock>(), sqlite3_open_v2(_, _, _, _))
      .WillByDefault(Invoke(__setting_fake_sqlite3_open_v2));
  ON_CALL(GetMock<SqliteMock>(), sqlite3_close(_))
      .WillByDefault(Return(SQLITE_OK));
  ON_CALL(GetMock<PackageManagerMock>(), pkgmgrinfo_appinfo_filter_create(_))
      .WillByDefault(Return(PMINFO_R_ERROR));
  int ret = notification_setting_refresh_setting_table(5001);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_FROM_DB);
}

/* N3: filter_create ok, add_string fails → FROM_DB (covers L379-382) */
TEST_F(NotificationSettingTest,
       refresh_setting_table_filter_add_string_fail_returns_error) {
  ON_CALL(GetMock<ConfigMock>(), _config_get_tizen_profile())
      .WillByDefault(Return(TIZEN_PROFILE_TV));
  ON_CALL(GetMock<SqliteMock>(), sqlite3_open_v2(_, _, _, _))
      .WillByDefault(Invoke(__setting_fake_sqlite3_open_v2));
  ON_CALL(GetMock<SqliteMock>(), sqlite3_close(_))
      .WillByDefault(Return(SQLITE_OK));
  /* filter_create: NiceMock default returns 0 = PMINFO_R_OK */
  ON_CALL(GetMock<PackageManagerMock>(), pkgmgrinfo_appinfo_filter_add_string(_, _, _))
      .WillByDefault(Return(PMINFO_R_ERROR));
  int ret = notification_setting_refresh_setting_table(5001);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_FROM_DB);
}

/* N4: filter_create+add_string ok, foreach fails → FROM_DB (covers L389-392) */
TEST_F(NotificationSettingTest,
       refresh_setting_table_filter_foreach_fail_returns_error) {
  ON_CALL(GetMock<ConfigMock>(), _config_get_tizen_profile())
      .WillByDefault(Return(TIZEN_PROFILE_TV));
  ON_CALL(GetMock<SqliteMock>(), sqlite3_open_v2(_, _, _, _))
      .WillByDefault(Invoke(__setting_fake_sqlite3_open_v2));
  ON_CALL(GetMock<SqliteMock>(), sqlite3_close(_))
      .WillByDefault(Return(SQLITE_OK));
  ON_CALL(GetMock<PackageManagerMock>(), pkgmgrinfo_appinfo_usr_filter_foreach_appinfo(_, _, _, _))
      .WillByDefault(Return(PMINFO_R_ERROR));
  int ret = notification_setting_refresh_setting_table(5001);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_FROM_DB);
}

/* P1: all pkgmgrinfo calls succeed → NONE (covers L370, L377, L385-393) */
TEST_F(NotificationSettingTest,
       refresh_setting_table_all_pkgmgrinfo_succeed_returns_none) {
  ON_CALL(GetMock<ConfigMock>(), _config_get_tizen_profile())
      .WillByDefault(Return(TIZEN_PROFILE_TV));
  ON_CALL(GetMock<SqliteMock>(), sqlite3_open_v2(_, _, _, _))
      .WillByDefault(Invoke(__setting_fake_sqlite3_open_v2));
  ON_CALL(GetMock<SqliteMock>(), sqlite3_close(_))
      .WillByDefault(Return(SQLITE_OK));
  /* All pkgmgrinfo calls: NiceMock defaults return 0 = PMINFO_R_OK */
  int ret = notification_setting_refresh_setting_table(5001);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
}

/* N_L397: filter_create sets non-NULL filter, add_string fails → filter_destroy called at L397 */
TEST_F(NotificationSettingTest,
       refresh_setting_table_filter_cleanup_covers_filter_destroy) {
  ON_CALL(GetMock<ConfigMock>(), _config_get_tizen_profile())
      .WillByDefault(Return(TIZEN_PROFILE_TV));
  ON_CALL(GetMock<SqliteMock>(), sqlite3_open_v2(_, _, _, _))
      .WillByDefault(Invoke(__setting_fake_sqlite3_open_v2));
  ON_CALL(GetMock<SqliteMock>(), sqlite3_close(_))
      .WillByDefault(Return(SQLITE_OK));
  /* filter_create sets *filter to non-NULL so cleanup at L397 is triggered */
  ON_CALL(GetMock<PackageManagerMock>(), pkgmgrinfo_appinfo_filter_create(_))
      .WillByDefault(DoAll(SetArgPointee<0>(__fake_filter_ptr), Return(PMINFO_R_OK)));
  /* add_string fails → goto out WITH filter != NULL → L397 pkgmgrinfo_appinfo_filter_destroy */
  ON_CALL(GetMock<PackageManagerMock>(), pkgmgrinfo_appinfo_filter_add_string(_, _, _))
      .WillByDefault(Return(PMINFO_R_ERROR));
  EXPECT_CALL(GetMock<PackageManagerMock>(),
              pkgmgrinfo_appinfo_filter_destroy(__fake_filter_ptr))
      .Times(1);
  int ret = notification_setting_refresh_setting_table(5001);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_FROM_DB);
}

/* N_L769_L776: prepare_v2 sets non-NULL stmt, step returns SQLITE_BUSY → L769 ERR + L776 finalize */
TEST_F(NotificationSettingTest,
       system_setting_init_table_step_busy_covers_error_log_and_finalize) {
  ON_CALL(GetMock<SqliteMock>(), sqlite3_open_v2(_, _, _, _))
      .WillByDefault(Invoke(__setting_fake_sqlite3_open_v2));
  /* All prepare_v2 calls set stmt to non-NULL so if(stmt) finalize runs (L776) */
  ON_CALL(GetMock<SqliteMock>(), sqlite3_prepare_v2(_, _, _, _, _))
      .WillByDefault(DoAll(SetArgPointee<3>(__fake_stmt_ptr), Return(SQLITE_OK)));
  /* step returns SQLITE_BUSY: not OK(0)/ROW(100)/DONE(101) → L769 ERR branch fires */
  ON_CALL(GetMock<SqliteMock>(), sqlite3_step(_))
      .WillByDefault(Return(SQLITE_BUSY));
  /* sqlite3_finalize must be called with the non-NULL stmt (L776) */
  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_finalize(__fake_stmt_ptr))
      .Times(AtLeast(1));
  ON_CALL(GetMock<SqliteMock>(), sqlite3_close(_))
      .WillByDefault(Return(SQLITE_OK));
  int ret = notification_system_setting_init_system_setting_table(5001);
  EXPECT_NE(ret, NOTIFICATION_ERROR_NONE);
}

/* N_L829: SELECT done, INSERT1 ok, INSERT2 prepare fails → L829 goto out */
TEST_F(NotificationSettingTest,
       system_setting_init_table_second_insert_fails_covers_L829) {
  ON_CALL(GetMock<SqliteMock>(), sqlite3_open_v2(_, _, _, _))
      .WillByDefault(Invoke(__setting_fake_sqlite3_open_v2));
  /* Sequence: SELECT prepare OK → INSERT1 prepare OK → INSERT2 prepare FAILS */
  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_prepare_v2(_, _, _, _, _))
      .WillOnce(DoAll(SetArgPointee<3>(__fake_stmt_ptr), Return(SQLITE_OK)))   /* SELECT */
      .WillOnce(DoAll(SetArgPointee<3>(__fake_stmt_ptr), Return(SQLITE_OK)))   /* INSERT1 */
      .WillOnce(Return(SQLITE_ERROR));                                          /* INSERT2 → FROM_DB */
  /* SELECT step → DONE (uid not in table); INSERT1 step → OK */
  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_step(_))
      .WillOnce(Return(SQLITE_DONE))    /* SELECT */
      .WillOnce(Return(SQLITE_OK));     /* INSERT1 */
  ON_CALL(GetMock<SqliteMock>(), sqlite3_close(_))
      .WillByDefault(Return(SQLITE_OK));
  int ret = notification_system_setting_init_system_setting_table(5001);
  EXPECT_NE(ret, NOTIFICATION_ERROR_NONE);
}

/* ============================================================== */
/* _foreach_app_info_callback / _is_package_in_setting_table      */
/* (L242-351 in notification_setting.c)                           */
/* ============================================================== */

/* S1: get_appid fails → L302-307 error branch → no SQL (covers L302-L307) */
TEST_F(NotificationSettingTest,
       foreach_callback_get_appid_fails_covers_L302_L307) {
  ON_CALL(GetMock<ConfigMock>(), _config_get_tizen_profile())
      .WillByDefault(Return(TIZEN_PROFILE_TV));
  ON_CALL(GetMock<SqliteMock>(), sqlite3_open_v2(_, _, _, _))
      .WillByDefault(Invoke(__setting_fake_sqlite3_open_v2));
  ON_CALL(GetMock<SqliteMock>(), sqlite3_close(_))
      .WillByDefault(Return(SQLITE_OK));
  ON_CALL(GetMock<PackageManagerMock>(),
          pkgmgrinfo_appinfo_usr_filter_foreach_appinfo(_, _, _, _))
      .WillByDefault(Invoke(__invoke_foreach_callback));
  ON_CALL(GetMock<PackageManagerMock>(),
          pkgmgrinfo_appinfo_get_appid(_, _))
      .WillByDefault(Return(PMINFO_R_ERROR));
  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_prepare_v2(_, _, _, _, _))
      .Times(0);
  EXPECT_EQ(notification_setting_refresh_setting_table(5001),
            NOTIFICATION_ERROR_NONE);
}

/* S2: get_pkgname fails → L309-314 error branch → no SQL (covers L309-L314) */
TEST_F(NotificationSettingTest,
       foreach_callback_get_pkgname_fails_covers_L309_L314) {
  ON_CALL(GetMock<ConfigMock>(), _config_get_tizen_profile())
      .WillByDefault(Return(TIZEN_PROFILE_TV));
  ON_CALL(GetMock<SqliteMock>(), sqlite3_open_v2(_, _, _, _))
      .WillByDefault(Invoke(__setting_fake_sqlite3_open_v2));
  ON_CALL(GetMock<SqliteMock>(), sqlite3_close(_))
      .WillByDefault(Return(SQLITE_OK));
  ON_CALL(GetMock<PackageManagerMock>(),
          pkgmgrinfo_appinfo_usr_filter_foreach_appinfo(_, _, _, _))
      .WillByDefault(Invoke(__invoke_foreach_callback));
  ON_CALL(GetMock<PackageManagerMock>(),
          pkgmgrinfo_appinfo_get_appid(_, _))
      .WillByDefault(DoAll(SetArgPointee<1>(__fake_app_id_str),
                           Return(PMINFO_R_OK)));
  ON_CALL(GetMock<PackageManagerMock>(),
          pkgmgrinfo_appinfo_get_pkgname(_, _))
      .WillByDefault(Return(PMINFO_R_ERROR));
  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_prepare_v2(_, _, _, _, _))
      .Times(0);
  EXPECT_EQ(notification_setting_refresh_setting_table(5001),
            NOTIFICATION_ERROR_NONE);
}

/* S3: package found in table → SELECT returns ROW → skip INSERT (covers L242-L321) */
TEST_F(NotificationSettingTest,
       foreach_callback_package_in_table_appid_branch_covers_L242_L321) {
  ON_CALL(GetMock<ConfigMock>(), _config_get_tizen_profile())
      .WillByDefault(Return(TIZEN_PROFILE_TV));
  ON_CALL(GetMock<SqliteMock>(), sqlite3_open_v2(_, _, _, _))
      .WillByDefault(Invoke(__setting_fake_sqlite3_open_v2));
  ON_CALL(GetMock<SqliteMock>(), sqlite3_close(_))
      .WillByDefault(Return(SQLITE_OK));
  ON_CALL(GetMock<PackageManagerMock>(),
          pkgmgrinfo_appinfo_usr_filter_foreach_appinfo(_, _, _, _))
      .WillByDefault(Invoke(__invoke_foreach_callback));
  ON_CALL(GetMock<PackageManagerMock>(),
          pkgmgrinfo_appinfo_get_appid(_, _))
      .WillByDefault(DoAll(SetArgPointee<1>(__fake_app_id_str),
                           Return(PMINFO_R_OK)));
  ON_CALL(GetMock<PackageManagerMock>(),
          pkgmgrinfo_appinfo_get_pkgname(_, _))
      .WillByDefault(DoAll(SetArgPointee<1>(__fake_pkg_name_str),
                           Return(PMINFO_R_OK)));
  ON_CALL(GetMock<SqliteMock>(), sqlite3_prepare_v2(_, _, _, _, _))
      .WillByDefault(Return(SQLITE_OK));
  /* SELECT step returns ROW → package found → callback skips INSERT */
  ON_CALL(GetMock<SqliteMock>(), sqlite3_step(_))
      .WillByDefault(Return(SQLITE_ROW));
  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_prepare_v2(_, _, _, _, _))
      .Times(1);
  EXPECT_EQ(notification_setting_refresh_setting_table(5001),
            NOTIFICATION_ERROR_NONE);
}

/* S4: package not in table → SELECT DONE → INSERT succeeds (covers L324-L351) */
TEST_F(NotificationSettingTest,
       foreach_callback_package_not_in_table_insert_succeeds_covers_L324_L351) {
  ON_CALL(GetMock<ConfigMock>(), _config_get_tizen_profile())
      .WillByDefault(Return(TIZEN_PROFILE_TV));
  ON_CALL(GetMock<SqliteMock>(), sqlite3_open_v2(_, _, _, _))
      .WillByDefault(Invoke(__setting_fake_sqlite3_open_v2));
  ON_CALL(GetMock<SqliteMock>(), sqlite3_close(_))
      .WillByDefault(Return(SQLITE_OK));
  ON_CALL(GetMock<PackageManagerMock>(),
          pkgmgrinfo_appinfo_usr_filter_foreach_appinfo(_, _, _, _))
      .WillByDefault(Invoke(__invoke_foreach_callback));
  ON_CALL(GetMock<PackageManagerMock>(),
          pkgmgrinfo_appinfo_get_appid(_, _))
      .WillByDefault(DoAll(SetArgPointee<1>(__fake_app_id_str),
                           Return(PMINFO_R_OK)));
  ON_CALL(GetMock<PackageManagerMock>(),
          pkgmgrinfo_appinfo_get_pkgname(_, _))
      .WillByDefault(DoAll(SetArgPointee<1>(__fake_pkg_name_str),
                           Return(PMINFO_R_OK)));
  ON_CALL(GetMock<SqliteMock>(), sqlite3_prepare_v2(_, _, _, _, _))
      .WillByDefault(Return(SQLITE_OK));
  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_step(_))
      .WillOnce(Return(SQLITE_DONE))   /* SELECT: not found */
      .WillOnce(Return(SQLITE_DONE));  /* INSERT: success */
  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_prepare_v2(_, _, _, _, _))
      .Times(2);
  EXPECT_EQ(notification_setting_refresh_setting_table(5001),
            NOTIFICATION_ERROR_NONE);
}

/* S5: INSERT step fails → SQLITE_BUSY → callback still returns OK (covers L337-L341) */
TEST_F(NotificationSettingTest,
       foreach_callback_insert_fails_covers_L337_L341) {
  ON_CALL(GetMock<ConfigMock>(), _config_get_tizen_profile())
      .WillByDefault(Return(TIZEN_PROFILE_TV));
  ON_CALL(GetMock<SqliteMock>(), sqlite3_open_v2(_, _, _, _))
      .WillByDefault(Invoke(__setting_fake_sqlite3_open_v2));
  ON_CALL(GetMock<SqliteMock>(), sqlite3_close(_))
      .WillByDefault(Return(SQLITE_OK));
  ON_CALL(GetMock<PackageManagerMock>(),
          pkgmgrinfo_appinfo_usr_filter_foreach_appinfo(_, _, _, _))
      .WillByDefault(Invoke(__invoke_foreach_callback));
  ON_CALL(GetMock<PackageManagerMock>(),
          pkgmgrinfo_appinfo_get_appid(_, _))
      .WillByDefault(DoAll(SetArgPointee<1>(__fake_app_id_str),
                           Return(PMINFO_R_OK)));
  ON_CALL(GetMock<PackageManagerMock>(),
          pkgmgrinfo_appinfo_get_pkgname(_, _))
      .WillByDefault(DoAll(SetArgPointee<1>(__fake_pkg_name_str),
                           Return(PMINFO_R_OK)));
  ON_CALL(GetMock<SqliteMock>(), sqlite3_prepare_v2(_, _, _, _, _))
      .WillByDefault(Return(SQLITE_OK));
  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_step(_))
      .WillOnce(Return(SQLITE_DONE))   /* SELECT: not found */
      .WillOnce(Return(SQLITE_BUSY));  /* INSERT: fails */
  EXPECT_EQ(notification_setting_refresh_setting_table(5001),
            NOTIFICATION_ERROR_NONE);
}

/* S6: SELECT prepare fails → _is_package returns false → INSERT proceeds (covers L266-L270) */
TEST_F(NotificationSettingTest,
       is_package_in_table_prepare_fails_covers_L266_L270) {
  ON_CALL(GetMock<ConfigMock>(), _config_get_tizen_profile())
      .WillByDefault(Return(TIZEN_PROFILE_TV));
  ON_CALL(GetMock<SqliteMock>(), sqlite3_open_v2(_, _, _, _))
      .WillByDefault(Invoke(__setting_fake_sqlite3_open_v2));
  ON_CALL(GetMock<SqliteMock>(), sqlite3_close(_))
      .WillByDefault(Return(SQLITE_OK));
  ON_CALL(GetMock<PackageManagerMock>(),
          pkgmgrinfo_appinfo_usr_filter_foreach_appinfo(_, _, _, _))
      .WillByDefault(Invoke(__invoke_foreach_callback));
  ON_CALL(GetMock<PackageManagerMock>(),
          pkgmgrinfo_appinfo_get_appid(_, _))
      .WillByDefault(DoAll(SetArgPointee<1>(__fake_app_id_str),
                           Return(PMINFO_R_OK)));
  ON_CALL(GetMock<PackageManagerMock>(),
          pkgmgrinfo_appinfo_get_pkgname(_, _))
      .WillByDefault(DoAll(SetArgPointee<1>(__fake_pkg_name_str),
                           Return(PMINFO_R_OK)));
  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_prepare_v2(_, _, _, _, _))
      .WillOnce(Return(SQLITE_ERROR))  /* SELECT prepare fails → _is_package false */
      .WillOnce(Return(SQLITE_OK));    /* INSERT prepare succeeds */
  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_step(_))
      .WillOnce(Return(SQLITE_DONE));  /* INSERT step */
  EXPECT_EQ(notification_setting_refresh_setting_table(5001),
            NOTIFICATION_ERROR_NONE);
}

/* S7: SELECT step returns SQLITE_BUSY → _is_package false → INSERT proceeds (covers L282-L287) */
TEST_F(NotificationSettingTest,
       is_package_in_table_step_unexpected_covers_L282_L287) {
  ON_CALL(GetMock<ConfigMock>(), _config_get_tizen_profile())
      .WillByDefault(Return(TIZEN_PROFILE_TV));
  ON_CALL(GetMock<SqliteMock>(), sqlite3_open_v2(_, _, _, _))
      .WillByDefault(Invoke(__setting_fake_sqlite3_open_v2));
  ON_CALL(GetMock<SqliteMock>(), sqlite3_close(_))
      .WillByDefault(Return(SQLITE_OK));
  ON_CALL(GetMock<PackageManagerMock>(),
          pkgmgrinfo_appinfo_usr_filter_foreach_appinfo(_, _, _, _))
      .WillByDefault(Invoke(__invoke_foreach_callback));
  ON_CALL(GetMock<PackageManagerMock>(),
          pkgmgrinfo_appinfo_get_appid(_, _))
      .WillByDefault(DoAll(SetArgPointee<1>(__fake_app_id_str),
                           Return(PMINFO_R_OK)));
  ON_CALL(GetMock<PackageManagerMock>(),
          pkgmgrinfo_appinfo_get_pkgname(_, _))
      .WillByDefault(DoAll(SetArgPointee<1>(__fake_pkg_name_str),
                           Return(PMINFO_R_OK)));
  ON_CALL(GetMock<SqliteMock>(), sqlite3_prepare_v2(_, _, _, _, _))
      .WillByDefault(Return(SQLITE_OK));
  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_step(_))
      .WillOnce(Return(SQLITE_BUSY))   /* SELECT step unexpected → _is_package false */
      .WillOnce(Return(SQLITE_DONE));  /* INSERT step */
  EXPECT_EQ(notification_setting_refresh_setting_table(5001),
            NOTIFICATION_ERROR_NONE);
}

/* S8: empty appid string → else branch in _is_package (uses pkgname key) (covers L255-L259) */
TEST_F(NotificationSettingTest,
       is_package_in_table_empty_appid_covers_else_branch_L255_L259) {
  ON_CALL(GetMock<ConfigMock>(), _config_get_tizen_profile())
      .WillByDefault(Return(TIZEN_PROFILE_TV));
  ON_CALL(GetMock<SqliteMock>(), sqlite3_open_v2(_, _, _, _))
      .WillByDefault(Invoke(__setting_fake_sqlite3_open_v2));
  ON_CALL(GetMock<SqliteMock>(), sqlite3_close(_))
      .WillByDefault(Return(SQLITE_OK));
  ON_CALL(GetMock<PackageManagerMock>(),
          pkgmgrinfo_appinfo_usr_filter_foreach_appinfo(_, _, _, _))
      .WillByDefault(Invoke(__invoke_foreach_callback));
  /* Empty appid triggers the else branch (package-name key path) */
  ON_CALL(GetMock<PackageManagerMock>(),
          pkgmgrinfo_appinfo_get_appid(_, _))
      .WillByDefault(DoAll(SetArgPointee<1>(__fake_empty_str),
                           Return(PMINFO_R_OK)));
  ON_CALL(GetMock<PackageManagerMock>(),
          pkgmgrinfo_appinfo_get_pkgname(_, _))
      .WillByDefault(DoAll(SetArgPointee<1>(__fake_pkg_name_str),
                           Return(PMINFO_R_OK)));
  ON_CALL(GetMock<SqliteMock>(), sqlite3_prepare_v2(_, _, _, _, _))
      .WillByDefault(Return(SQLITE_OK));
  /* SELECT step returns ROW → package found in else branch → skip INSERT */
  ON_CALL(GetMock<SqliteMock>(), sqlite3_step(_))
      .WillByDefault(Return(SQLITE_ROW));
  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_prepare_v2(_, _, _, _, _))
      .Times(1);
  EXPECT_EQ(notification_setting_refresh_setting_table(5001),
            NOTIFICATION_ERROR_NONE);
}

/* S9: prepare_v2 sets non-NULL stmt + step SQLITE_ROW → finalize called at out: (covers L291) */
TEST_F(NotificationSettingTest,
       is_package_in_table_nonnull_stmt_row_covers_L291_finalize) {
  ON_CALL(GetMock<ConfigMock>(), _config_get_tizen_profile())
      .WillByDefault(Return(TIZEN_PROFILE_TV));
  ON_CALL(GetMock<SqliteMock>(), sqlite3_open_v2(_, _, _, _))
      .WillByDefault(Invoke(__setting_fake_sqlite3_open_v2));
  ON_CALL(GetMock<SqliteMock>(), sqlite3_close(_))
      .WillByDefault(Return(SQLITE_OK));
  ON_CALL(GetMock<PackageManagerMock>(),
          pkgmgrinfo_appinfo_usr_filter_foreach_appinfo(_, _, _, _))
      .WillByDefault(Invoke(__invoke_foreach_callback));
  ON_CALL(GetMock<PackageManagerMock>(), pkgmgrinfo_appinfo_get_appid(_, _))
      .WillByDefault(DoAll(SetArgPointee<1>(__fake_app_id_str), Return(PMINFO_R_OK)));
  ON_CALL(GetMock<PackageManagerMock>(), pkgmgrinfo_appinfo_get_pkgname(_, _))
      .WillByDefault(DoAll(SetArgPointee<1>(__fake_pkg_name_str), Return(PMINFO_R_OK)));
  /* prepare_v2 sets non-NULL stmt so cleanup at out: triggers sqlite3_finalize (L291) */
  ON_CALL(GetMock<SqliteMock>(), sqlite3_prepare_v2(_, _, _, _, _))
      .WillByDefault(DoAll(SetArgPointee<3>(__fake_stmt_ptr), Return(SQLITE_OK)));
  /* step returns ROW → package found → callback skips INSERT */
  ON_CALL(GetMock<SqliteMock>(), sqlite3_step(_))
      .WillByDefault(Return(SQLITE_ROW));
  EXPECT_CALL(GetMock<SqliteMock>(),
              sqlite3_finalize(__fake_stmt_ptr)).Times(AtLeast(1));
  EXPECT_EQ(notification_setting_refresh_setting_table(5001),
            NOTIFICATION_ERROR_NONE);
}

/* S10: _is_package_in_setting_table SELECT mprintf OOM → L261-L263  */
TEST_F(NotificationSettingTest,
       is_package_in_table_select_mprintf_oom_covers_L261_L263) {
  ON_CALL(GetMock<ConfigMock>(), _config_get_tizen_profile())
      .WillByDefault(Return(TIZEN_PROFILE_TV));
  ON_CALL(GetMock<SqliteMock>(), sqlite3_open_v2(_, _, _, _))
      .WillByDefault(Invoke(__setting_fake_sqlite3_open_v2));
  ON_CALL(GetMock<SqliteMock>(), sqlite3_close(_))
      .WillByDefault(Return(SQLITE_OK));
  ON_CALL(GetMock<PackageManagerMock>(),
          pkgmgrinfo_appinfo_usr_filter_foreach_appinfo(_, _, _, _))
      .WillByDefault(Invoke(__invoke_foreach_callback));
  ON_CALL(GetMock<PackageManagerMock>(), pkgmgrinfo_appinfo_get_appid(_, _))
      .WillByDefault(DoAll(SetArgPointee<1>(__fake_app_id_str), Return(PMINFO_R_OK)));
  ON_CALL(GetMock<PackageManagerMock>(), pkgmgrinfo_appinfo_get_pkgname(_, _))
      .WillByDefault(DoAll(SetArgPointee<1>(__fake_pkg_name_str), Return(PMINFO_R_OK)));
  /* call 0 = SELECT mprintf in _is_package_in_setting_table → fails → return false */
  sqlite3_mprintf_set_fail_on(0);
  int ret = notification_setting_refresh_setting_table(5001);
  sqlite3_mprintf_reset_fail();
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
}

/* S11: _foreach_app_info_callback INSERT mprintf OOM → L334-L337   */
TEST_F(NotificationSettingTest,
       foreach_callback_insert_mprintf_oom_covers_L334_L337) {
  ON_CALL(GetMock<ConfigMock>(), _config_get_tizen_profile())
      .WillByDefault(Return(TIZEN_PROFILE_TV));
  ON_CALL(GetMock<SqliteMock>(), sqlite3_open_v2(_, _, _, _))
      .WillByDefault(Invoke(__setting_fake_sqlite3_open_v2));
  ON_CALL(GetMock<SqliteMock>(), sqlite3_close(_))
      .WillByDefault(Return(SQLITE_OK));
  ON_CALL(GetMock<PackageManagerMock>(),
          pkgmgrinfo_appinfo_usr_filter_foreach_appinfo(_, _, _, _))
      .WillByDefault(Invoke(__invoke_foreach_callback));
  ON_CALL(GetMock<PackageManagerMock>(), pkgmgrinfo_appinfo_get_appid(_, _))
      .WillByDefault(DoAll(SetArgPointee<1>(__fake_app_id_str), Return(PMINFO_R_OK)));
  ON_CALL(GetMock<PackageManagerMock>(), pkgmgrinfo_appinfo_get_pkgname(_, _))
      .WillByDefault(DoAll(SetArgPointee<1>(__fake_pkg_name_str), Return(PMINFO_R_OK)));
  ON_CALL(GetMock<SqliteMock>(), sqlite3_prepare_v2(_, _, _, _, _))
      .WillByDefault(Return(SQLITE_OK));
  /* SELECT step DONE → package not in table → callback tries INSERT mprintf */
  ON_CALL(GetMock<SqliteMock>(), sqlite3_step(_))
      .WillByDefault(Return(SQLITE_DONE));
  /* call 0 = SELECT mprintf succeeds; call 1 = INSERT mprintf fails */
  sqlite3_mprintf_set_fail_on(1);
  int ret = notification_setting_refresh_setting_table(5001);
  sqlite3_mprintf_reset_fail();
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
}

/* ============================================================== */
/* NotificationSettingTidlTest — fixture with TIDL mocks          */
/* ============================================================== */

class NotificationSettingTidlTest : public TestFixture {
 public:
  NotificationSettingTidlTest()
      : TestFixture(std::make_unique<::MocksWithTidl>()) {}

  void SetUp() override {
    ON_CALL(GetMock<AulMock>(), aul_getuid()).WillByDefault(Return(5001));
  }
};

/* ============================================================== */
/* notification_setting_get_setting — AUL success → TIDL path     */
/* (L92-96 in notification_setting.c)                              */
/* ============================================================== */

TEST_F(NotificationSettingTidlTest,
       setting_get_setting_aul_success_enters_tidl_path) {
  /* [P1-partial] AUL lookup succeeds → function calls
   * notification_setting_get_setting_by_package_name which calls TIDL.
   * With TIDL connect mocked to fail we get a non-NONE error that is NOT
   * NOT_EXIST_ID (which would mean the AUL path failed). The key invariant
   * is that the AUL+free branch (L92-96) executes. */
  ON_CALL(GetMock<AulMock>(), aul_app_get_appid_bypid(_, _, _))
      .WillByDefault(Invoke(__setting_fake_aul_app_get_appid_bypid));
  ON_CALL(GetMock<NotiServiceProxyMock>(),
          rpc_port_proxy_noti_service_connect_sync(_))
      .WillByDefault(Return(RPC_PORT_ERROR_IO_ERROR));

  notification_setting_h setting = nullptr;
  int ret = notification_setting_get_setting(&setting);
  /* AUL succeeded so we should NOT get NOT_EXIST_ID. */
  EXPECT_NE(ret, NOTIFICATION_ERROR_NOT_EXIST_ID);
  if (setting) notification_setting_free_notification(setting);
}

/* ============================================================== */
/* GROUP E — notification_register_system_setting_dnd_changed_cb_for_uid */
/* ============================================================== */

/* E1: duplicate callback pointer returns INVALID_PARAMETER (L677-682) */
TEST_F(NotificationSettingTidlTest,
       register_dnd_changed_cb_duplicate_pointer_returns_invalid_parameter) {
  ON_CALL(GetMock<NotiServiceProxyMock>(),
          rpc_port_proxy_noti_service_connect_sync(_))
      .WillByDefault(Return(RPC_PORT_ERROR_NONE));

  using GListAppendFn = GList*(*)(GList*, gpointer);
  ON_CALL(GetMock<GlibMock>(), g_list_append(_, _))
      .WillByDefault(Invoke([](GList* list, gpointer data) -> GList* {
        static GListAppendFn real = reinterpret_cast<GListAppendFn>(
            dlsym(RTLD_NEXT, "g_list_append"));
        return real ? real(list, data) : nullptr;
      }));

  static dnd_changed_cb cb = [](void*, int) {};

  ASSERT_EQ(notification_register_system_setting_dnd_changed_cb_for_uid(
                cb, nullptr, 7025),
            NOTIFICATION_ERROR_NONE);
  /* Duplicate pointer: _noti_dnd_cb_compare finds it → INVALID_PARAMETER */
  EXPECT_EQ(notification_register_system_setting_dnd_changed_cb_for_uid(
                cb, nullptr, 7025),
            NOTIFICATION_ERROR_INVALID_PARAMETER);

  /* Cleanup */
  notification_unregister_system_setting_dnd_changed_cb_for_uid(cb, 7025);
}

/* E2: different callback pointer appends to existing list (L685) */
TEST_F(NotificationSettingTidlTest,
       register_dnd_changed_cb_different_cb_appends_to_existing_list) {
  ON_CALL(GetMock<NotiServiceProxyMock>(),
          rpc_port_proxy_noti_service_connect_sync(_))
      .WillByDefault(Return(RPC_PORT_ERROR_NONE));

  using GListAppendFn = GList*(*)(GList*, gpointer);
  ON_CALL(GetMock<GlibMock>(), g_list_append(_, _))
      .WillByDefault(Invoke([](GList* list, gpointer data) -> GList* {
        static GListAppendFn real = reinterpret_cast<GListAppendFn>(
            dlsym(RTLD_NEXT, "g_list_append"));
        return real ? real(list, data) : nullptr;
      }));

  static dnd_changed_cb cb1 = [](void*, int) {};
  static dnd_changed_cb cb2 = [](void*, int) {};

  /* First register: list==NULL → insert */
  ASSERT_EQ(notification_register_system_setting_dnd_changed_cb_for_uid(
                cb1, nullptr, 7026),
            NOTIFICATION_ERROR_NONE);
  /* Second register: different pointer → not dup → L685 append */
  EXPECT_EQ(notification_register_system_setting_dnd_changed_cb_for_uid(
                cb2, nullptr, 7026),
            NOTIFICATION_ERROR_NONE);

  /* Cleanup */
  notification_unregister_system_setting_dnd_changed_cb_for_uid(cb1, 7026);
  notification_unregister_system_setting_dnd_changed_cb_for_uid(cb2, 7026);
}

/* ============================================================== */
/* GROUP F — notification_unregister_system_setting_dnd_changed_cb_for_uid */
/* ============================================================== */

/* F1: last entry removed → steal from hash + monitor_fini (L712-718, L723-724, L730-731) */
TEST_F(NotificationSettingTidlTest,
       unregister_dnd_changed_cb_last_entry_steals_from_hash_and_calls_monitor_fini) {
  ON_CALL(GetMock<NotiServiceProxyMock>(),
          rpc_port_proxy_noti_service_connect_sync(_))
      .WillByDefault(Return(RPC_PORT_ERROR_NONE));

  using GListAppendFn = GList*(*)(GList*, gpointer);
  ON_CALL(GetMock<GlibMock>(), g_list_append(_, _))
      .WillByDefault(Invoke([](GList* list, gpointer data) -> GList* {
        static GListAppendFn real = reinterpret_cast<GListAppendFn>(
            dlsym(RTLD_NEXT, "g_list_append"));
        return real ? real(list, data) : nullptr;
      }));

  static dnd_changed_cb cb1 = [](void*, int) {};

  ASSERT_EQ(notification_register_system_setting_dnd_changed_cb_for_uid(
                cb1, nullptr, 7027),
            NOTIFICATION_ERROR_NONE);
  /* Remove only entry: list→NULL → steal (L724), hash size==0 → fini (L730-731) */
  EXPECT_EQ(notification_unregister_system_setting_dnd_changed_cb_for_uid(cb1, 7027),
            NOTIFICATION_ERROR_NONE);
}

/* F2: remaining entry after delete → hash replace instead of steal (L725-727) */
TEST_F(NotificationSettingTidlTest,
       unregister_dnd_changed_cb_remaining_entry_replaces_hash) {
  ON_CALL(GetMock<NotiServiceProxyMock>(),
          rpc_port_proxy_noti_service_connect_sync(_))
      .WillByDefault(Return(RPC_PORT_ERROR_NONE));

  using GListAppendFn = GList*(*)(GList*, gpointer);
  ON_CALL(GetMock<GlibMock>(), g_list_append(_, _))
      .WillByDefault(Invoke([](GList* list, gpointer data) -> GList* {
        static GListAppendFn real = reinterpret_cast<GListAppendFn>(
            dlsym(RTLD_NEXT, "g_list_append"));
        return real ? real(list, data) : nullptr;
      }));

  static dnd_changed_cb cb1 = [](void*, int) {};
  static dnd_changed_cb cb2 = [](void*, int) {};

  ASSERT_EQ(notification_register_system_setting_dnd_changed_cb_for_uid(
                cb1, nullptr, 7028),
            NOTIFICATION_ERROR_NONE);
  ASSERT_EQ(notification_register_system_setting_dnd_changed_cb_for_uid(
                cb2, nullptr, 7028),
            NOTIFICATION_ERROR_NONE);

  /* cb2 remains after cb1 removed → list non-empty → L726-727 replace, no fini */
  EXPECT_EQ(notification_unregister_system_setting_dnd_changed_cb_for_uid(cb1, 7028),
            NOTIFICATION_ERROR_NONE);

  /* Cleanup */
  notification_unregister_system_setting_dnd_changed_cb_for_uid(cb2, 7028);
}

/* F3: callback not registered returns INVALID_PARAMETER (L719-720) */
TEST_F(NotificationSettingTidlTest,
       unregister_dnd_changed_cb_not_registered_returns_invalid_parameter) {
  ON_CALL(GetMock<NotiServiceProxyMock>(),
          rpc_port_proxy_noti_service_connect_sync(_))
      .WillByDefault(Return(RPC_PORT_ERROR_NONE));

  using GListAppendFn = GList*(*)(GList*, gpointer);
  ON_CALL(GetMock<GlibMock>(), g_list_append(_, _))
      .WillByDefault(Invoke([](GList* list, gpointer data) -> GList* {
        static GListAppendFn real = reinterpret_cast<GListAppendFn>(
            dlsym(RTLD_NEXT, "g_list_append"));
        return real ? real(list, data) : nullptr;
      }));

  static dnd_changed_cb cb1 = [](void*, int) {};
  static dnd_changed_cb cb2 = [](void*, int) {};

  ASSERT_EQ(notification_register_system_setting_dnd_changed_cb_for_uid(
                cb1, nullptr, 7029),
            NOTIFICATION_ERROR_NONE);

  /* cb2 was never registered → find_custom returns NULL → L720 INVALID_PARAMETER */
  EXPECT_EQ(notification_unregister_system_setting_dnd_changed_cb_for_uid(cb2, 7029),
            NOTIFICATION_ERROR_INVALID_PARAMETER);

  /* Cleanup */
  notification_unregister_system_setting_dnd_changed_cb_for_uid(cb1, 7029);
}
