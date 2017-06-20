/*
 * Copyright (c) 2000 - 2017 Samsung Electronics Co., Ltd. All rights reserved.
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <db-util.h>
#include <package_manager.h>
#include <pkgmgr-info.h>
#include <tizen_type.h>
#include <tzplatform_config.h>

#include <notification.h>
#include <notification_db.h>
#include <notification_list.h>
#include <notification_noti.h>
#include <notification_debug.h>
#include <notification_ipc.h>
#include <notification_private.h>
#include <notification_setting.h>
#include <notification_setting_internal.h>

#define NOTIFICATION_PRIVILEGE "http://tizen.org/privilege/notification"

typedef struct _noti_dnd_cb_info noti_dnd_cb_info_s;

typedef struct {
	uid_t uid;
	sqlite3 *db;
} setting_local_info;

struct _noti_dnd_cb_info {
	dnd_changed_cb callback;
	void *user_data;
};

static GHashTable *_noti_dnd_cb_hash = NULL;

EXPORT_API int notification_setting_get_setting_array_for_uid(notification_setting_h *setting_array, int *count, uid_t uid)
{
	if (setting_array == NULL || count == NULL) {
		NOTIFICATION_ERR("NOTIFICATION_ERROR_INVALID_PARAMETER");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	return notification_ipc_request_get_setting_array(setting_array, count, uid);
}

EXPORT_API int notification_setting_get_setting_array(notification_setting_h *setting_array, int *count)
{
	return notification_setting_get_setting_array_for_uid(setting_array, count, getuid());
}

EXPORT_API int notification_setting_get_setting_by_appid_for_uid(const char *app_id, notification_setting_h *setting, uid_t uid)
{
	if (app_id == NULL || setting == NULL) {
		NOTIFICATION_ERR("NOTIFICATION_ERROR_INVALID_PARAMETER");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	return notification_ipc_request_get_setting_by_app_id(app_id, setting, uid);
}

/* LCOV_EXCL_START */
EXPORT_API int notification_setting_get_setting_by_package_name(const char *package_name, notification_setting_h *setting)
{
	return notification_setting_get_setting_by_appid_for_uid(package_name, setting, getuid());
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
EXPORT_API int notification_setting_get_setting(notification_setting_h *setting)
{
	int ret;
	char *package_name = NULL;

	package_name = notification_get_app_id_by_pid();

	if (package_name == NULL)
		return NOTIFICATION_ERROR_NOT_EXIST_ID;

	ret = notification_setting_get_setting_by_package_name(package_name, setting);

	free(package_name);

	return ret;
}
/* LCOV_EXCL_STOP */

EXPORT_API int notification_setting_get_package_name(notification_setting_h setting, char **value)
{
	if (setting == NULL || value == NULL) {
		NOTIFICATION_ERR("Invalid parameter");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	if (setting->package_name == NULL) {
		NOTIFICATION_ERR("setting->package_name is null");
		return NOTIFICATION_ERROR_NOT_EXIST_ID;
	}

	*value = SAFE_STRDUP(setting->package_name);

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_setting_get_appid(notification_setting_h setting, char **app_id)
{
	if (setting == NULL || app_id == NULL) {
		NOTIFICATION_ERR("Invalid parameter");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	if (setting->app_id == NULL) {
		NOTIFICATION_ERR("setting->app_id is null");
		return NOTIFICATION_ERROR_NOT_EXIST_ID;
	}

	*app_id = SAFE_STRDUP(setting->app_id);

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_setting_get_allow_to_notify(notification_setting_h setting, bool *value)
{
	if (setting == NULL || value == NULL) {
		NOTIFICATION_ERR("Invalid parameter");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	*value = setting->allow_to_notify;

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_setting_set_allow_to_notify(notification_setting_h setting, bool value)
{
	if (setting == NULL) {
		NOTIFICATION_ERR("Invalid parameter");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	setting->allow_to_notify = value;

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_setting_get_do_not_disturb_except(notification_setting_h setting, bool *value)
{
	if (setting == NULL || value == NULL) {
		NOTIFICATION_ERR("Invalid parameter");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	*value = setting->do_not_disturb_except;

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_setting_set_do_not_disturb_except(notification_setting_h setting, bool value)
{
	if (setting == NULL) {
		NOTIFICATION_ERR("Invalid parameter");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	setting->do_not_disturb_except = value;

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_setting_get_visibility_class(notification_setting_h setting, int *value)
{
	if (setting == NULL || value == NULL) {
		NOTIFICATION_ERR("Invalid parameter");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	*value = setting->visibility_class;

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_setting_set_visibility_class(notification_setting_h setting, int value)
{
	if (setting == NULL) {
		NOTIFICATION_ERR("Invalid parameter");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	setting->visibility_class = value;

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_setting_get_pop_up_notification(notification_setting_h setting, bool *value)
{
	if (setting == NULL || value == NULL) {
		NOTIFICATION_ERR("Invalid parameter");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	*value = setting->pop_up_notification;
	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_setting_set_pop_up_notification(notification_setting_h setting, bool value)
{
	if (setting == NULL) {
		NOTIFICATION_ERR("Invalid parameter");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	setting->pop_up_notification = value;
	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_setting_get_lock_screen_content(notification_setting_h setting, lock_screen_content_level_e *level)
{
	if (setting == NULL || level == NULL) {
		NOTIFICATION_ERR("Invalid parameter");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	*level = setting->lock_screen_content_level;

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_setting_set_lock_screen_content(notification_setting_h setting, lock_screen_content_level_e level)
{
	if (setting == NULL) {
		NOTIFICATION_ERR("Invalid parameter");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	setting->lock_screen_content_level = level;

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_setting_get_app_disabled(notification_setting_h setting, bool *value)
{
	if (setting == NULL || value == NULL) {
		NOTIFICATION_ERR("Invalid parameter");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	*value = setting->app_disabled;

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_setting_update_setting_for_uid(notification_setting_h setting, uid_t uid)
{
	if (setting == NULL) {
		NOTIFICATION_ERR("Invalid parameter");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	return notification_ipc_update_setting(setting, uid);
}

EXPORT_API int notification_setting_update_setting(notification_setting_h setting)
{
	return notification_setting_update_setting_for_uid(setting, getuid());
}

EXPORT_API int notification_setting_free_notification(notification_setting_h setting)
{
	if (setting == NULL) {
		NOTIFICATION_ERR("Invalid parameter");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	SAFE_FREE(setting->package_name);
	SAFE_FREE(setting->app_id);

	/* add codes to free all properties */

	SAFE_FREE(setting);

	return NOTIFICATION_ERROR_NONE;
}

static bool _is_package_in_setting_table(sqlite3 *db, const char *package_name, const char* app_id, uid_t uid)
{
	sqlite3_stmt *stmt = NULL;
	int sql_ret = SQLITE_OK;
	bool err = true;
	int field_index = 1;

	if (app_id != NULL)
		sql_ret = sqlite3_prepare_v2(db, "SELECT app_id FROM notification_setting WHERE uid = ? AND package_name = ? AND app_id = ?", -1, &stmt, NULL);
	else
		sql_ret = sqlite3_prepare_v2(db, "SELECT package_name FROM notification_setting WHERE uid = ? AND package_name = ?", -1, &stmt, NULL);

	if (sql_ret != SQLITE_OK) {
		NOTIFICATION_ERR("sqlite3_prepare_v2 failed [%d][%s]", sql_ret, sqlite3_errmsg(db));
		err = false;
		goto out;
	}

	sqlite3_bind_int(stmt, field_index++, uid);
	sqlite3_bind_text(stmt, field_index++, package_name, -1, SQLITE_TRANSIENT);
	if (app_id != NULL)
		sqlite3_bind_text(stmt, field_index++, app_id, -1, SQLITE_TRANSIENT);

	sql_ret = sqlite3_step(stmt);
	if (sql_ret == SQLITE_DONE) {
		NOTIFICATION_INFO("no matched app_id from package_name found[%s][%s][%d]", package_name, app_id, sql_ret);
		err = false;
		goto out;
	}

	if (sql_ret != SQLITE_OK && sql_ret != SQLITE_ROW) {
		NOTIFICATION_ERR("sqlite3_step failed [%d][%s]", sql_ret, sqlite3_errmsg(db));
		err = false;
		goto out;
	}
out:
	if (stmt)
		sqlite3_finalize(stmt);

	return err;
}

static int foreach_app_info_callback(const pkgmgrinfo_appinfo_h handle, void *user_data)
{
	setting_local_info *info = (setting_local_info *)user_data;
	sqlite3 *db = info->db;
	sqlite3_stmt *stmt = NULL;
	int pkgmgr_ret = PACKAGE_MANAGER_ERROR_NONE;
	int err = true;
	int field_index = 1;
	int sql_ret = SQLITE_OK;
	char *app_id = NULL;
	char *package_name = NULL;

	pkgmgr_ret = pkgmgrinfo_appinfo_get_appid(handle, &app_id);
	if (pkgmgr_ret != PACKAGE_MANAGER_ERROR_NONE) {
		NOTIFICATION_ERR("pkgmgrinfo_appinfo_get_appid failed [%d]", pkgmgr_ret);
		err = false;
		goto out;
	}

	pkgmgr_ret = pkgmgrinfo_appinfo_get_pkgname(handle, &package_name);
	if (pkgmgr_ret != PACKAGE_MANAGER_ERROR_NONE) {
		NOTIFICATION_ERR("pkgmgrinfo_appinfo_get_pkgname failed [%d]", pkgmgr_ret);
		goto out;
	}

	if (_is_package_in_setting_table(db, package_name, app_id, info->uid) == true) {
		NOTIFICATION_INFO("uid %d [%s] is exist", info->uid, app_id);
		err = false;
		goto out;
	}

	NOTIFICATION_INFO("uid %d package_name %s [%s] will be inserted", info->uid, package_name, app_id);
	sql_ret = sqlite3_prepare_v2(db, "INSERT INTO notification_setting (uid, package_name, app_id) "
					 "VALUES (?, ?, ?) ", -1, &stmt, NULL);

	if (sql_ret != SQLITE_OK) {
		NOTIFICATION_ERR("sqlite3_prepare_v2 failed [%d][%s]", sql_ret, sqlite3_errmsg(db));
		err = false;
		goto out;
	}

	sqlite3_bind_int(stmt, field_index++, info->uid);
	sqlite3_bind_text(stmt, field_index++, package_name, -1, SQLITE_TRANSIENT);
	sqlite3_bind_text(stmt, field_index++, app_id, -1, SQLITE_TRANSIENT);

	sql_ret = sqlite3_step(stmt);
	if (sql_ret != SQLITE_OK && sql_ret != SQLITE_DONE) {
		NOTIFICATION_ERR("sqlite3_step failed [%d][%s]", sql_ret, sqlite3_errmsg(db));
		err = false;
	}

out:
	if (stmt)
		sqlite3_finalize(stmt);

	return err;
}

static int foreach_package_info_callback(const pkgmgrinfo_pkginfo_h package_info, void *user_data)
{
	char *package_name = NULL;
	int pkgmgr_ret = PACKAGE_MANAGER_ERROR_NONE;
	int err = true;
	pkgmgrinfo_appinfo_filter_h handle = NULL;
	setting_local_info *info = (setting_local_info *)user_data;

	pkgmgr_ret = pkgmgrinfo_pkginfo_get_pkgname(package_info, &package_name);
	if (pkgmgr_ret != PACKAGE_MANAGER_ERROR_NONE) {
		NOTIFICATION_ERR("package_info_get_package failed [%d]", pkgmgr_ret);
		err = false;
		goto out;
	}

	pkgmgr_ret = pkgmgrinfo_appinfo_filter_create(&handle);
	if (pkgmgr_ret != PMINFO_R_OK) {
		NOTIFICATION_ERR("pkgmgrinfo_appinfo_filter_create failed [%d]", pkgmgr_ret);
		err = false;
		goto out;
	}

	pkgmgr_ret = pkgmgrinfo_appinfo_filter_add_string(handle, PMINFO_APPINFO_PROP_APP_PACKAGE, package_name);
	if (pkgmgr_ret != PMINFO_R_OK) {
		NOTIFICATION_ERR("pkgmgrinfo_appinfo_filter_add_string failed [%d]", pkgmgr_ret);
		err = false;
		goto out;
	}

	pkgmgr_ret = pkgmgrinfo_appinfo_usr_filter_foreach_appinfo(handle, foreach_app_info_callback, info, info->uid);
	if (pkgmgr_ret != PMINFO_R_OK) {
		NOTIFICATION_ERR("pkgmgrinfo_pkginfo_filter_foreach_appinfo failed [%d]", pkgmgr_ret);
		err = false;
		goto out;
	}
out:
	if (handle)
		pkgmgrinfo_appinfo_filter_destroy(handle);

	return err;
}

EXPORT_API int notification_setting_refresh_setting_table(uid_t uid)
{
	int err = NOTIFICATION_ERROR_NONE;
	sqlite3 *db = NULL;
	int sql_ret = SQLITE_OK;
	int pkgmgr_ret = PACKAGE_MANAGER_ERROR_NONE;
	pkgmgrinfo_pkginfo_filter_h filter = NULL;
	setting_local_info info;

	NOTIFICATION_INFO("refresh setting table [%d]", uid);

	sql_ret = sqlite3_open_v2(DBPATH, &db, SQLITE_OPEN_READWRITE, NULL);
	if (sql_ret != SQLITE_OK || db == NULL) {
		NOTIFICATION_ERR("db_util_open failed [%s][%d]", DBPATH, sql_ret);
		return NOTIFICATION_ERROR_FROM_DB;
	}

	sqlite3_exec(db, "BEGIN immediate;", NULL, NULL, NULL);

	pkgmgr_ret = pkgmgrinfo_pkginfo_filter_create(&filter);
	if (pkgmgr_ret != PMINFO_R_OK) {
		NOTIFICATION_ERR("pkgmgrinfo_pkginfo_filter_create failed [%d]", pkgmgr_ret);
		err = NOTIFICATION_ERROR_FROM_DB;
		goto out;
	}

	pkgmgr_ret = pkgmgrinfo_pkginfo_filter_add_string(filter, PMINFO_PKGINFO_PROP_PACKAGE_PRIVILEGE, NOTIFICATION_PRIVILEGE);
	if (pkgmgr_ret != PMINFO_R_OK) {
		NOTIFICATION_ERR("pkgmgrinfo_pkginfo_filter_add_string failed [%d]", pkgmgr_ret);
		err = NOTIFICATION_ERROR_FROM_DB;
		goto out;
	}

	info.db = db;
	info.uid = uid;
	pkgmgr_ret = pkgmgrinfo_pkginfo_usr_filter_foreach_pkginfo(filter, foreach_package_info_callback, &info, uid);
	if (pkgmgr_ret != PMINFO_R_OK) {
		NOTIFICATION_ERR("pkgmgrinfo_pkginfo_usr_filter_foreach_pkginfo failed [%d]", pkgmgr_ret);
		err = NOTIFICATION_ERROR_FROM_DB;
		goto out;
	}

out:
	if (filter)
		pkgmgrinfo_pkginfo_filter_destroy(filter);

	if (db) {
		if (err == NOTIFICATION_ERROR_NONE)
			sqlite3_exec(db, "END;", NULL, NULL, NULL);
		else
			sqlite3_exec(db, "ROLLBACK;", NULL, NULL, NULL);

		if ((sql_ret = db_util_close(db)) != SQLITE_OK)
			NOTIFICATION_WARN("fail to db_util_close - [%d]", sql_ret);
	}

	return err;
}

static int _install_and_update_package(const char *package_name, uid_t uid)
{
	sqlite3 *db;
	int ret = NOTIFICATION_ERROR_NONE;
	int sql_ret = SQLITE_OK;
	int pkgmgr_ret = PACKAGE_MANAGER_ERROR_NONE;
	setting_local_info info;
	pkgmgrinfo_pkginfo_filter_h handle = NULL;

	sql_ret = sqlite3_open_v2(DBPATH, &db, SQLITE_OPEN_READWRITE, NULL);
	if (sql_ret != SQLITE_OK || db == NULL) {
		NOTIFICATION_ERR("db_util_open failed [%s][%d]", DBPATH, sql_ret);
		return NOTIFICATION_ERROR_FROM_DB;
	}

	sqlite3_exec(db, "BEGIN immediate;", NULL, NULL, NULL);

	pkgmgr_ret = pkgmgrinfo_pkginfo_filter_create(&handle);
	if (pkgmgr_ret != PMINFO_R_OK) {
		NOTIFICATION_ERR("pkgmgrinfo_pkginfo_filter_create failed [%d]", pkgmgr_ret);
		ret = NOTIFICATION_ERROR_FROM_DB;
		goto out;
	}

	pkgmgr_ret = pkgmgrinfo_pkginfo_filter_add_string(handle, PMINFO_PKGINFO_PROP_PACKAGE_PRIVILEGE, NOTIFICATION_PRIVILEGE);
	if (pkgmgr_ret != PMINFO_R_OK) {
		NOTIFICATION_ERR("pkgmgrinfo_pkginfo_filter_add_string failed [%d]", pkgmgr_ret);
		ret = NOTIFICATION_ERROR_FROM_DB;
		goto out;
	}

	pkgmgr_ret = pkgmgrinfo_pkginfo_filter_add_string(handle, PMINFO_PKGINFO_PROP_PACKAGE_ID, package_name);
	if (pkgmgr_ret != PMINFO_R_OK) {
		NOTIFICATION_ERR("pkgmgrinfo_pkginfo_filter_add_string failed [%d]", pkgmgr_ret);
		ret = NOTIFICATION_ERROR_FROM_DB;
		goto out;
	}

	info.db = db;
	info.uid = uid;
	pkgmgr_ret = pkgmgrinfo_pkginfo_usr_filter_foreach_pkginfo(handle, foreach_package_info_callback, &info, uid);
	if (pkgmgr_ret != PMINFO_R_OK) {
		NOTIFICATION_ERR("pkgmgrinfo_pkginfo_usr_filter_foreach_pkginfo failed [%d]", pkgmgr_ret);
		ret = NOTIFICATION_ERROR_FROM_DB;
		goto out;
	}

out:
	if (handle)
		pkgmgrinfo_pkginfo_filter_destroy(handle);

	if (db) {
		if (ret == NOTIFICATION_ERROR_NONE)
			sqlite3_exec(db, "END;", NULL, NULL, NULL);
		else
			sqlite3_exec(db, "ROLLBACK;", NULL, NULL, NULL);

		if ((sql_ret = db_util_close(db)) != SQLITE_OK)
			NOTIFICATION_WARN("db_util_close failed [%d]", sql_ret);
	}

	return ret;
}

static int _delete_package_from_setting_db(const char *package_name, uid_t uid)
{
	sqlite3 *db = NULL;
	sqlite3_stmt *stmt = NULL;
	int ret = NOTIFICATION_ERROR_NONE;
	int sql_ret = SQLITE_OK;
	int field_index = 1;
	bool is_package_in_setting_table = false;

	sql_ret = sqlite3_open_v2(DBPATH, &db, SQLITE_OPEN_READWRITE, NULL);
	if (sql_ret != SQLITE_OK || db == NULL) {
		NOTIFICATION_ERR("db_util_open failed [%s][%d]", DBPATH, sql_ret);
		return NOTIFICATION_ERROR_FROM_DB;
	}

	is_package_in_setting_table = _is_package_in_setting_table(db, package_name, NULL, uid);
	if (is_package_in_setting_table == false) {
		NOTIFICATION_INFO("[%s] is not exist", package_name);
		goto out;
	}

	sqlite3_exec(db, "BEGIN immediate;", NULL, NULL, NULL);

	sql_ret = sqlite3_prepare_v2(db, "DELETE FROM notification_setting WHERE uid = ? AND package_name = ? ", -1, &stmt, NULL);
	if (sql_ret != SQLITE_OK) {
		NOTIFICATION_ERR("sqlite3_prepare_v2 failed [%d][%s]", sql_ret, sqlite3_errmsg(db));
		ret = NOTIFICATION_ERROR_FROM_DB;
		goto out;
	}

	sqlite3_bind_int(stmt, field_index++, uid);
	sqlite3_bind_text(stmt, field_index++, package_name, -1, SQLITE_TRANSIENT);

	sql_ret = sqlite3_step(stmt);
	if (sql_ret != SQLITE_OK && sql_ret != SQLITE_DONE) {
		NOTIFICATION_ERR("sqlite3_step failed [%d][%s]", sql_ret, sqlite3_errmsg(db));
		ret = NOTIFICATION_ERROR_FROM_DB;
	}

out:
	if (stmt)
		sqlite3_finalize(stmt);

	if (db) {
		if (ret == NOTIFICATION_ERROR_NONE)
			sqlite3_exec(db, "END;", NULL, NULL, NULL);
		else
			sqlite3_exec(db, "ROLLBACK;", NULL, NULL, NULL);

		if ((sql_ret = db_util_close(db)) != SQLITE_OK)
			NOTIFICATION_WARN("db_util_close failed [%d]", sql_ret);
	}

	return ret;
}

EXPORT_API int notification_setting_insert_package_for_uid(const char *package_name, uid_t uid)
{
	return _install_and_update_package(package_name, uid);
}

EXPORT_API int notification_setting_delete_package_for_uid(const char *package_name, uid_t uid)
{
	return _delete_package_from_setting_db(package_name, uid);
}

EXPORT_API int notification_system_setting_load_system_setting_for_uid(notification_system_setting_h *system_setting, uid_t uid)
{
	if (system_setting == NULL) {
		NOTIFICATION_ERR("NOTIFICATION_ERROR_INVALID_PARAMETER");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	return notification_ipc_request_load_system_setting(system_setting, uid);
}

EXPORT_API int notification_system_setting_load_system_setting(notification_system_setting_h *system_setting)
{
	return notification_system_setting_load_system_setting_for_uid(system_setting, getuid());
}

EXPORT_API int notification_system_setting_update_system_setting_for_uid(notification_system_setting_h system_setting, uid_t uid)
{
	if (system_setting == NULL) {
		NOTIFICATION_ERR("Invalid parameter");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	return notification_ipc_update_system_setting(system_setting, uid);
}

EXPORT_API int notification_system_setting_update_system_setting(notification_system_setting_h system_setting)
{
	return notification_system_setting_update_system_setting_for_uid(system_setting, getuid());
}

EXPORT_API int notification_system_setting_free_system_setting(notification_system_setting_h system_setting)
{
	if (system_setting == NULL) {
		NOTIFICATION_ERR("Invalid parameter");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	/* add codes to free all properties */

	if (system_setting->dnd_allow_exceptions != NULL)
		g_list_free_full(system_setting->dnd_allow_exceptions, free);

	SAFE_FREE(system_setting);

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_system_setting_get_do_not_disturb(notification_system_setting_h system_setting, bool *value)
{
	if (system_setting == NULL || value == NULL) {
		NOTIFICATION_ERR("Invalid parameter");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	*value = system_setting->do_not_disturb;

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_system_setting_set_do_not_disturb(notification_system_setting_h system_setting, bool value)
{
	if (system_setting == NULL) {
		NOTIFICATION_ERR("Invalid parameter");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	system_setting->do_not_disturb = value;

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_system_setting_get_visibility_class(notification_system_setting_h system_setting, int *value)
{
	if (system_setting == NULL || value == NULL) {
		NOTIFICATION_ERR("Invalid parameter");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	*value = system_setting->visibility_class;

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_system_setting_set_visibility_class(notification_system_setting_h system_setting, int value)
{
	if (system_setting == NULL) {
		NOTIFICATION_ERR("Invalid parameter");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	system_setting->visibility_class = value;

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_system_setting_dnd_schedule_get_enabled(notification_system_setting_h system_setting, bool *enabled)
{
	if (system_setting == NULL || enabled == NULL) {
		NOTIFICATION_ERR("Invalid parameter");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	*enabled = system_setting->dnd_schedule_enabled;

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_system_setting_dnd_schedule_set_enabled(notification_system_setting_h system_setting, bool enabled)
{
	if (system_setting == NULL) {
		NOTIFICATION_ERR("Invalid parameter");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	system_setting->dnd_schedule_enabled = enabled;

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_system_setting_dnd_schedule_get_day(notification_system_setting_h system_setting, int *day)
{
	if (system_setting == NULL || day == NULL) {
		NOTIFICATION_ERR("Invalid parameter");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	*day = system_setting->dnd_schedule_day;

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_system_setting_dnd_schedule_set_day(notification_system_setting_h system_setting, int day)
{
	if (system_setting == NULL) {
		NOTIFICATION_ERR("Invalid parameter");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	system_setting->dnd_schedule_day = day;

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_system_setting_dnd_schedule_get_start_time(notification_system_setting_h system_setting, int *hour, int *min)
{
	if (system_setting == NULL || hour == NULL || min == NULL) {
		NOTIFICATION_ERR("Invalid parameter");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	*hour = system_setting->dnd_start_hour;
	*min = system_setting->dnd_start_min;

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_system_setting_dnd_schedule_set_start_time(notification_system_setting_h system_setting, int hour, int min)
{
	if (system_setting == NULL) {
		NOTIFICATION_ERR("Invalid parameter");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	system_setting->dnd_start_hour = hour;
	system_setting->dnd_start_min = min;

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_system_setting_dnd_schedule_get_end_time(notification_system_setting_h system_setting, int *hour, int *min)
{
	if (system_setting == NULL || hour == NULL || min == NULL) {
		NOTIFICATION_ERR("Invalid parameter");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	*hour = system_setting->dnd_end_hour;
	*min = system_setting->dnd_end_min;

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_system_setting_dnd_schedule_set_end_time(notification_system_setting_h system_setting, int hour, int min)
{
	if (system_setting == NULL) {
		NOTIFICATION_ERR("Invalid parameter");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	system_setting->dnd_end_hour = hour;
	system_setting->dnd_end_min = min;

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_system_setting_get_lock_screen_content(notification_system_setting_h system_setting, lock_screen_content_level_e *level)
{
	if (system_setting == NULL || level == NULL) {
		NOTIFICATION_ERR("Invalid parameter");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	*level = system_setting->lock_screen_content_level;

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_system_setting_set_lock_screen_content(notification_system_setting_h system_setting, lock_screen_content_level_e level)
{
	if (system_setting == NULL) {
		NOTIFICATION_ERR("Invalid parameter");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	system_setting->lock_screen_content_level = level;

	return NOTIFICATION_ERROR_NONE;
}

static gint _dnd_allow_exception_compare(gconstpointer a, gconstpointer b)
{
	dnd_allow_exception_h dnd_allow_exception_data_a;

	if (a == NULL)
		return -1;

	dnd_allow_exception_data_a = (dnd_allow_exception_h)a;

	if (dnd_allow_exception_data_a->type == GPOINTER_TO_INT(b))
		return 0;

	return -1;
}

EXPORT_API int notification_system_setting_get_dnd_allow_exceptions(notification_system_setting_h system_setting, dnd_allow_exception_type_e type, int *value)
{
	dnd_allow_exception_h dnd_allow_exception_data;
	GList *list;

	if (system_setting == NULL || value == NULL) {
		NOTIFICATION_ERR("Invalid parameter");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	list = g_list_find_custom(system_setting->dnd_allow_exceptions, GINT_TO_POINTER(type), (GCompareFunc)_dnd_allow_exception_compare);
	if (list) {
		dnd_allow_exception_data = (dnd_allow_exception_h)list->data;
		*value = dnd_allow_exception_data->value;
	} else {
		NOTIFICATION_ERR("Invalid parameter - type");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_system_setting_set_dnd_allow_exceptions(notification_system_setting_h system_setting, dnd_allow_exception_type_e type, int value)
{
	dnd_allow_exception_h dnd_allow_exception_data;
	GList *list = NULL;

	if (system_setting == NULL) {
		NOTIFICATION_ERR("Invalid parameter");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	list = g_list_first(system_setting->dnd_allow_exceptions);
	list = g_list_find_custom(list, GINT_TO_POINTER(type), (GCompareFunc)_dnd_allow_exception_compare);

	if (list) {
		dnd_allow_exception_data = (dnd_allow_exception_h)list->data;
		dnd_allow_exception_data->value = value;
	} else {
		dnd_allow_exception_data = (dnd_allow_exception_h)malloc(sizeof(struct notification_system_setting_dnd_allow_exception));
		if (dnd_allow_exception_data == NULL)
			return NOTIFICATION_ERROR_OUT_OF_MEMORY;

		dnd_allow_exception_data->type = type;
		dnd_allow_exception_data->value = value;
		system_setting->dnd_allow_exceptions = g_list_append(list, dnd_allow_exception_data);
	}

	return NOTIFICATION_ERROR_NONE;
}

static gint _noti_dnd_cb_compare(gconstpointer a, gconstpointer b)
{
	noti_dnd_cb_info_s *info = NULL;

	if (a == NULL)
		return -1;

	info = (noti_dnd_cb_info_s *)a;
	if (info->callback == b)
		return 0;

	return 1;
}

void notification_call_dnd_changed_cb_for_uid(int do_not_disturb, uid_t uid)
{
	GList *noti_dnd_cb_list = NULL;
	noti_dnd_cb_info_s *dnd_data = NULL;

	if (_noti_dnd_cb_hash == NULL)
		return;

	noti_dnd_cb_list = (GList *)g_hash_table_lookup(_noti_dnd_cb_hash, GUINT_TO_POINTER(uid));
	if (noti_dnd_cb_list == NULL) {
		NOTIFICATION_ERR("Invalide data");
		return;
	}

	noti_dnd_cb_list = g_list_first(noti_dnd_cb_list);

	for (; noti_dnd_cb_list != NULL; noti_dnd_cb_list = noti_dnd_cb_list->next) {
		dnd_data = noti_dnd_cb_list->data;

		if (dnd_data != NULL && dnd_data->callback != NULL)
			dnd_data->callback(dnd_data->user_data, do_not_disturb);
	}
}

EXPORT_API int notification_register_system_setting_dnd_changed_cb_for_uid(dnd_changed_cb callback, void *user_data, uid_t uid)
{
	GList *noti_dnd_list = NULL;
	GList *noti_dnd_found_list = NULL;
	noti_dnd_cb_info_s *dnd_data = NULL;

	if (callback == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	if (notification_ipc_monitor_init(uid) != NOTIFICATION_ERROR_NONE) {
		NOTIFICATION_ERR("notification_ipc_monitor_init error");
		return NOTIFICATION_ERROR_IO_ERROR;
	}
	if (_noti_dnd_cb_hash == NULL)
		_noti_dnd_cb_hash = g_hash_table_new(g_direct_hash, g_direct_equal);

	dnd_data = (noti_dnd_cb_info_s *)malloc(sizeof(noti_dnd_cb_info_s));
	if (dnd_data == NULL)
		return NOTIFICATION_ERROR_OUT_OF_MEMORY;

	dnd_data->callback = callback;
	dnd_data->user_data = user_data;

	noti_dnd_list = (GList *)g_hash_table_lookup(_noti_dnd_cb_hash, GUINT_TO_POINTER(uid));

	if (noti_dnd_list == NULL) {
		noti_dnd_list = g_list_append(noti_dnd_list, dnd_data);
		g_hash_table_insert(_noti_dnd_cb_hash, GUINT_TO_POINTER(uid), noti_dnd_list);
	} else {
		noti_dnd_found_list = g_list_find_custom(noti_dnd_list, (gconstpointer)callback,
							 (GCompareFunc)_noti_dnd_cb_compare);
		if (noti_dnd_found_list) {
			NOTIFICATION_ERR("Already existing callback");
			free(dnd_data);
			return NOTIFICATION_ERROR_INVALID_PARAMETER;
		} else {
			noti_dnd_list = g_list_append(noti_dnd_list, dnd_data);
		}
	}

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_register_system_setting_dnd_changed_cb(dnd_changed_cb callback, void *user_data)
{
	return notification_register_system_setting_dnd_changed_cb_for_uid(callback, user_data, getuid());
}

EXPORT_API int notification_unregister_system_setting_dnd_changed_cb_for_uid(dnd_changed_cb callback, uid_t uid)
{
	GList *noti_dnd_cb_list = NULL;
	GList *noti_dnd_del_list = NULL;
	noti_dnd_cb_info_s *dnd_data = NULL;

	if (callback == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	if (_noti_dnd_cb_hash == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	noti_dnd_cb_list = (GList *)g_hash_table_lookup(_noti_dnd_cb_hash, GUINT_TO_POINTER(uid));

	if (noti_dnd_cb_list == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	noti_dnd_del_list = g_list_find_custom(noti_dnd_cb_list, (gconstpointer)callback,
					       (GCompareFunc)_noti_dnd_cb_compare);

	if (noti_dnd_del_list) {
		dnd_data = g_list_nth_data(noti_dnd_del_list, 0);
		noti_dnd_cb_list = g_list_delete_link(noti_dnd_cb_list, noti_dnd_del_list);
		free(dnd_data);
	} else {
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	if (noti_dnd_cb_list == NULL) {
		g_hash_table_steal(_noti_dnd_cb_hash, GUINT_TO_POINTER(uid));
	} else {
		noti_dnd_cb_list = g_list_first(noti_dnd_cb_list);
		g_hash_table_replace(_noti_dnd_cb_hash, GUINT_TO_POINTER(uid), noti_dnd_cb_list);
	}

	if (g_hash_table_size(_noti_dnd_cb_hash) == 0)
		notification_ipc_monitor_fini();

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_unregister_system_setting_dnd_changed_cb(dnd_changed_cb callback)
{
	return notification_unregister_system_setting_dnd_changed_cb_for_uid(callback, getuid());
}

static bool _is_uid_in_system_setting_table(sqlite3 *db, uid_t uid)
{
	bool err = true;
	sqlite3_stmt *stmt = NULL;
	int sql_ret = SQLITE_OK;
	int field_index = 1;

	sql_ret = sqlite3_prepare_v2(db, "SELECT uid FROM notification_system_setting WHERE uid = ?", -1, &stmt, NULL);
	if (sql_ret != SQLITE_OK) {
		NOTIFICATION_ERR("sqlite3_prepare_v2 failed [%d][%s]", sql_ret, sqlite3_errmsg(db));
		err = false;
		goto out;
	}

	sqlite3_bind_int(stmt, field_index++, uid);

	sql_ret = sqlite3_step(stmt);
	if (sql_ret == SQLITE_DONE) {
		NOTIFICATION_INFO("no matched uid found[%d][%d]", uid, sql_ret);
		err = false;
		goto out;
	}

	if (sql_ret != SQLITE_OK && sql_ret != SQLITE_ROW) {
		NOTIFICATION_ERR("sqlite3_step failed [%d][%s]", sql_ret, sqlite3_errmsg(db));
		err = false;
		goto out;
	}

out:
	if (stmt)
		sqlite3_finalize(stmt);

	return err;
}

EXPORT_API int notification_system_setting_init_system_setting_table(uid_t uid)
{
	int err = NOTIFICATION_ERROR_NONE;
	int sql_ret = SQLITE_OK;
	int field_index = 1;
	sqlite3 *db = NULL;
	sqlite3_stmt *stmt = NULL;

	NOTIFICATION_INFO("init system setting table [%d]", uid);

	db = notification_db_open(DBPATH);
	if (db == NULL)
		return get_last_result();

	/* if notification system setting don't init. */
	if (_is_uid_in_system_setting_table(db, uid) == true) {
		NOTIFICATION_DBG("Notification system setting table is already initialized.");
	} else {
		NOTIFICATION_DBG("Notification system setting table is not initialized yet");

		/* notification_system_setting */
		sql_ret = sqlite3_prepare_v2(db, "INSERT INTO notification_system_setting (uid) VALUES (?) ", -1, &stmt, NULL);
		if (sql_ret != SQLITE_OK) {
			NOTIFICATION_ERR("sqlite3_prepare_v2 failed [%d][%s]", sql_ret, sqlite3_errmsg(db));
			err = NOTIFICATION_ERROR_FROM_DB;
			goto out;
		}

		sqlite3_bind_int(stmt, field_index, uid);

		sql_ret = sqlite3_step(stmt);
		if (sql_ret != SQLITE_OK && sql_ret != SQLITE_DONE) {
			NOTIFICATION_ERR("sqlite3_step failed [%d][%s]", sql_ret, sqlite3_errmsg(db));
			err = NOTIFICATION_ERROR_FROM_DB;
			goto out;
		}

		/* dnd_allow_exception */
		sql_ret = sqlite3_prepare_v2(db, "INSERT INTO dnd_allow_exception (uid) VALUES (?) ", -1, &stmt, NULL);
		if (sql_ret != SQLITE_OK) {
			NOTIFICATION_ERR("sqlite3_prepare_v2 failed [%d][%s]", sql_ret, sqlite3_errmsg(db));
			err = NOTIFICATION_ERROR_FROM_DB;
			goto out;
		}

		sqlite3_bind_int(stmt, field_index, uid);

		sql_ret = sqlite3_step(stmt);
		if (sql_ret != SQLITE_OK && sql_ret != SQLITE_DONE) {
			NOTIFICATION_ERR("sqlite3_step failed [%d][%s]", sql_ret, sqlite3_errmsg(db));
			err = NOTIFICATION_ERROR_FROM_DB;
			goto out;
		}
	}

	NOTIFICATION_DBG("Notification system setting tables initialization is success.");

out:
	if (db) {
		if (err == NOTIFICATION_ERROR_NONE)
			sqlite3_exec(db, "END;", NULL, NULL, NULL);
		else
			sqlite3_exec(db, "ROLLBACK;", NULL, NULL, NULL);

		notification_db_close(&db);
	}

	if (stmt)
		sqlite3_finalize(stmt);

	return err;
}
