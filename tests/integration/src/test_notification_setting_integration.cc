/*
 * Copyright (c) 2023 Samsung Electronics Co., Ltd. All rights reserved.
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
#include <sqlite3.h>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <unistd.h>
#include <sys/stat.h>
#include <tzplatform_config.h>

#include "notification/include/notification.h"
#include "notification/include/notification_db.h"
#include "notification/include/notification_setting.h"
#include "notification/include/notification_setting_internal.h"
#include "notification/include/notification_setting_service.h"

namespace {

const char* CREATE_SETTING_TABLE_SQL =
    "CREATE TABLE IF NOT EXISTS notification_setting ("
    "  uid INTEGER,"
    "  package_name TEXT NOT NULL,"
    "  app_id TEXT NOT NULL,"
    "  allow_to_notify INTEGER DEFAULT 1,"
    "  do_not_disturb_except INTEGER DEFAULT 0,"
    "  visibility_class INTEGER DEFAULT 0,"
    "  pop_up_notification INTEGER DEFAULT 1,"
    "  lock_screen_content_level INTEGER DEFAULT 0,"
    "  app_disabled INTEGER DEFAULT 0,"
    "  UNIQUE (uid, package_name, app_id));";

const char* CREATE_SYSTEM_SETTING_TABLE_SQL =
    "CREATE TABLE IF NOT EXISTS notification_system_setting ("
    "  uid INTEGER,"
    "  do_not_disturb INTEGER DEFAULT 0,"
    "  visibility_class INTEGER DEFAULT 0,"
    "  dnd_schedule_enabled INTEGER DEFAULT 0,"
    "  dnd_schedule_day INTEGER DEFAULT 62,"
    "  dnd_start_hour INTEGER DEFAULT 22,"
    "  dnd_start_min INTEGER DEFAULT 0,"
    "  dnd_end_hour INTEGER DEFAULT 8,"
    "  dnd_end_min INTEGER DEFAULT 0,"
    "  lock_screen_content_level INTEGER DEFAULT 0,"
    "  UNIQUE (uid));";

const char* CREATE_DND_EXCEPTION_TABLE_SQL =
    "CREATE TABLE IF NOT EXISTS dnd_allow_exception ("
    "  uid INTEGER,"
    "  type INTEGER DEFAULT 0,"
    "  value INTEGER DEFAULT 0,"
    "  UNIQUE (uid, type));";

}  // namespace

class NotificationSettingIntegrationTest : public ::testing::Test {
 protected:
  static void MkdirP(const std::string& path) {
    size_t pos = 0;
    while ((pos = path.find('/', pos + 1)) != std::string::npos)
      mkdir(path.substr(0, pos).c_str(), 0755);
    mkdir(path.c_str(), 0755);
  }

  void SetUp() override {
    db_path_ = std::string(DBPATH);
    std::string db_dir = db_path_.substr(0, db_path_.rfind('/'));
    MkdirP(db_dir);

    if (access(db_dir.c_str(), W_OK) != 0)
      GTEST_SKIP() << "DB directory not writable: " << db_dir;

    unlink(db_path_.c_str());

    sqlite3* setup_db = nullptr;
    int rc = sqlite3_open(db_path_.c_str(), &setup_db);
    ASSERT_EQ(rc, SQLITE_OK);

    char* err_msg = nullptr;
    rc = sqlite3_exec(setup_db, CREATE_SETTING_TABLE_SQL,
        nullptr, nullptr, &err_msg);
    if (err_msg) { sqlite3_free(err_msg); err_msg = nullptr; }
    ASSERT_EQ(rc, SQLITE_OK);

    rc = sqlite3_exec(setup_db, CREATE_SYSTEM_SETTING_TABLE_SQL,
        nullptr, nullptr, &err_msg);
    if (err_msg) { sqlite3_free(err_msg); err_msg = nullptr; }
    ASSERT_EQ(rc, SQLITE_OK);

    rc = sqlite3_exec(setup_db, CREATE_DND_EXCEPTION_TABLE_SQL,
        nullptr, nullptr, &err_msg);
    if (err_msg) { sqlite3_free(err_msg); err_msg = nullptr; }
    ASSERT_EQ(rc, SQLITE_OK);

    sqlite3_close(setup_db);
  }

  void TearDown() override {
    if (!db_path_.empty())
      unlink(db_path_.c_str());
  }

  int GetRowCount(const char* table_name) {
    sqlite3* db = nullptr;
    int rc = sqlite3_open(db_path_.c_str(), &db);
    if (rc != SQLITE_OK) return -1;

    std::string query =
        "SELECT COUNT(*) FROM " + std::string(table_name) + ";";
    sqlite3_stmt* stmt = nullptr;
    int count = -1;

    rc = sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr);
    if (rc == SQLITE_OK && sqlite3_step(stmt) == SQLITE_ROW)
      count = sqlite3_column_int(stmt, 0);
    if (stmt) sqlite3_finalize(stmt);
    sqlite3_close(db);
    return count;
  }

  std::string db_path_;
};

TEST_F(NotificationSettingIntegrationTest, UpdateSettingViaLibrary) {
  int ret = notification_setting_db_update(
      "org.tizen.testpkg", "org.tizen.testapp",
      1, 0, 0, 1, 0, 5001);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  EXPECT_EQ(GetRowCount("notification_setting"), 1);
}

TEST_F(NotificationSettingIntegrationTest, UpdateAndRetrieveSetting) {
  int ret = notification_setting_db_update(
      "org.tizen.testpkg", "org.tizen.testapp",
      1, 0, 2, 1, 1, 5001);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  notification_setting_h setting = nullptr;
  ret = noti_setting_service_get_setting_by_app_id(
      "org.tizen.testapp", &setting, 5001);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  if (setting) {
    bool allow = false;
    ret = notification_setting_get_allow_to_notify(setting, &allow);
    EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
    EXPECT_TRUE(allow);

    int vis_class = 0;
    ret = notification_setting_get_visibility_class(setting, &vis_class);
    EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
    EXPECT_EQ(vis_class, 2);

    notification_setting_free_notification(setting);
  }
}

TEST_F(NotificationSettingIntegrationTest, UpdateSettingValues) {
  int ret = notification_setting_db_update(
      "org.tizen.testpkg", "org.tizen.testapp",
      1, 0, 0, 1, 0, 5001);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  ret = notification_setting_db_update(
      "org.tizen.testpkg", "org.tizen.testapp",
      0, 1, 3, 0, 0, 5001);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  notification_setting_h setting = nullptr;
  ret = noti_setting_service_get_setting_by_app_id(
      "org.tizen.testapp", &setting, 5001);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  if (setting) {
    bool allow = true;
    ret = notification_setting_get_allow_to_notify(setting, &allow);
    EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
    EXPECT_FALSE(allow);

    bool dnd_except = false;
    ret = notification_setting_get_do_not_disturb_except(
        setting, &dnd_except);
    EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
    EXPECT_TRUE(dnd_except);

    notification_setting_free_notification(setting);
  }
}

TEST_F(NotificationSettingIntegrationTest, SystemSettingUpdate) {
  int ret = notification_setting_db_update_system_setting(
      0, 0, 0, 62, 22, 0, 8, 0, 0, 5001);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  ret = notification_setting_db_update_system_setting(
      1, 0, 1, 62, 23, 0, 7, 0, 0, 5001);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  notification_system_setting_h sys_setting = nullptr;
  ret = noti_system_setting_load_system_setting(&sys_setting, 5001);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  if (sys_setting) {
    bool dnd = false;
    ret = notification_system_setting_get_do_not_disturb(
        sys_setting, &dnd);
    EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
    EXPECT_TRUE(dnd);

    bool sched_enabled = false;
    ret = notification_system_setting_dnd_schedule_get_enabled(
        sys_setting, &sched_enabled);
    EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
    EXPECT_TRUE(sched_enabled);

    int start_hour = 0, start_min = 0;
    ret = notification_system_setting_dnd_schedule_get_start_time(
        sys_setting, &start_hour, &start_min);
    EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
    EXPECT_EQ(start_hour, 23);

    int end_hour = 0, end_min = 0;
    ret = notification_system_setting_dnd_schedule_get_end_time(
        sys_setting, &end_hour, &end_min);
    EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
    EXPECT_EQ(end_hour, 7);

    notification_system_setting_free_system_setting(sys_setting);
  }
}

TEST_F(NotificationSettingIntegrationTest, MultipleAppSettings) {
  notification_setting_db_update(
      "org.tizen.pkg1", "org.tizen.app1", 1, 0, 0, 1, 0, 5001);
  notification_setting_db_update(
      "org.tizen.pkg2", "org.tizen.app2", 0, 0, 0, 1, 0, 5001);
  notification_setting_db_update(
      "org.tizen.pkg3", "org.tizen.app3", 1, 0, 0, 1, 0, 5001);

  EXPECT_EQ(GetRowCount("notification_setting"), 3);

  notification_setting_h setting_array = nullptr;
  int count = 0;
  int ret = noti_setting_get_setting_array(
      &setting_array, &count, 5001);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(count, 3);

  if (setting_array)
    notification_setting_free_notification(setting_array);
}

TEST_F(NotificationSettingIntegrationTest, SettingNullParams) {
  int ret = notification_setting_get_allow_to_notify(nullptr, nullptr);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);

  ret = notification_setting_get_visibility_class(nullptr, nullptr);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);

  ret = notification_setting_free_notification(nullptr);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationSettingIntegrationTest, SystemSettingNullParams) {
  bool val;
  int ret = notification_system_setting_get_do_not_disturb(
      nullptr, &val);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);

  ret = notification_system_setting_free_system_setting(nullptr);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationSettingIntegrationTest, UpdateDoNotDisturb) {
  notification_setting_db_update_system_setting(
      0, 0, 0, 62, 22, 0, 8, 0, 0, 5001);

  int ret = notification_setting_db_update_do_not_disturb(1, 5001);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  int dnd = 0;
  ret = noti_system_setting_get_do_not_disturb(&dnd, 5001);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(dnd, 1);
}
