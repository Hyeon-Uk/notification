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

#include <aul.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <package_manager.h>
#include <pkgmgr-info.h>
#include <tizen_type.h>
#include <tzplatform_config.h>

#include <notification.h>
#include <notification_db.h>
#include <notification_list.h>
#include <notification_noti.h>
#include <notification_debug.h>
#include <notification_private.h>
#include <notification_setting.h>
#include <notification_setting_internal.h>
#include <notification_tidl.h>
#include "notification_db_query.h"

#include "raii/sqlite3_db.hh"
#include "raii/sqlite3_stmt.hh"
#include "raii/sqlite3_free.hh"

#include <memory>
#include <type_traits>

namespace {

/* TU-local RAII for pkgmgrinfo_appinfo_filter_h.  Used by
 * _install_and_update_package and notification_setting_refresh_setting_table.
 * Promote to src/notification/src/raii/pkgmgr_handles.hh if a
 * second consumer file shows up. */
struct PkgmgrFilterDeleter {
	void operator()(pkgmgrinfo_appinfo_filter_h h) const noexcept {
		if (h != nullptr) pkgmgrinfo_appinfo_filter_destroy(h);
	}
};
using PkgmgrFilterPtr = std::unique_ptr<
	std::remove_pointer_t<pkgmgrinfo_appinfo_filter_h>,
	PkgmgrFilterDeleter>;

}  // namespace

extern "C" {

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

/* LCOV_EXCL_START */
EXPORT_API int notification_setting_get_setting_array_for_uid(notification_setting_h *setting_array, int *count, uid_t uid)
{
	if (setting_array == NULL || count == NULL) {
		ERR("Invalid parameter");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	return notification_tidl_request_get_setting_array(setting_array, count, uid);
}

EXPORT_API int notification_setting_get_setting_array(notification_setting_h *setting_array, int *count)
{
	return notification_setting_get_setting_array_for_uid(setting_array, count, aul_getuid());
}

EXPORT_API int notification_setting_get_setting_by_appid_for_uid(const char *app_id, notification_setting_h *setting, uid_t uid)
{
	if (app_id == NULL || setting == NULL) {
		ERR("Invalid parameter");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	return notification_tidl_request_get_setting_by_app_id(app_id, setting, uid);
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
EXPORT_API int notification_setting_get_setting_by_package_name(const char *package_name, notification_setting_h *setting)
{
	return notification_setting_get_setting_by_appid_for_uid(package_name, setting, aul_getuid());
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
EXPORT_API int notification_setting_get_setting(notification_setting_h *setting)
{
	int ret;
	char *app_id = NULL;

	app_id = notification_get_app_id_by_pid(getpid());
	if (app_id == NULL)
		return NOTIFICATION_ERROR_NOT_EXIST_ID;

	ret = notification_setting_get_setting_by_package_name(app_id, setting);

	free(app_id);

	return ret;
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
EXPORT_API int notification_setting_get_package_name(notification_setting_h setting, char **value)
{
	if (setting == NULL || value == NULL) {
		ERR("Invalid parameter");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	if (setting->package_name == NULL) {
		ERR("setting->package_name is null");
		return NOTIFICATION_ERROR_NOT_EXIST_ID;
	}

	*value = SAFE_STRDUP(setting->package_name);

	return NOTIFICATION_ERROR_NONE;
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
EXPORT_API int notification_setting_get_appid(notification_setting_h setting, char **app_id)
{
	if (setting == NULL || app_id == NULL) {
		ERR("Invalid parameter");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	if (setting->app_id == NULL) {
		ERR("setting->app_id is null");
		return NOTIFICATION_ERROR_NOT_EXIST_ID;
	}

	*app_id = SAFE_STRDUP(setting->app_id);

	return NOTIFICATION_ERROR_NONE;
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
EXPORT_API int notification_setting_get_allow_to_notify(notification_setting_h setting, bool *value)
{
	if (setting == NULL || value == NULL) {
		ERR("Invalid parameter");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	*value = setting->allow_to_notify;

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_setting_set_allow_to_notify(notification_setting_h setting, bool value)
{
	if (setting == NULL) {
		ERR("Invalid parameter");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	setting->allow_to_notify = value;

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_setting_get_do_not_disturb_except(notification_setting_h setting, bool *value)
{
	if (setting == NULL || value == NULL) {
		ERR("Invalid parameter");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	*value = setting->do_not_disturb_except;

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_setting_set_do_not_disturb_except(notification_setting_h setting, bool value)
{
	if (setting == NULL) {
		ERR("Invalid parameter");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	setting->do_not_disturb_except = value;

	return NOTIFICATION_ERROR_NONE;
}
/* LCOV_EXCL_STOP */

EXPORT_API int notification_setting_get_visibility_class(notification_setting_h setting, int *value)
{
	if (setting == NULL || value == NULL) {
		ERR("Invalid parameter");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	*value = setting->visibility_class;

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_setting_set_visibility_class(notification_setting_h setting, int value)
{
	if (setting == NULL) {
		ERR("Invalid parameter");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	setting->visibility_class = value;

	return NOTIFICATION_ERROR_NONE;
}

/* LCOV_EXCL_START */
EXPORT_API int notification_setting_get_pop_up_notification(notification_setting_h setting, bool *value)
{
	if (setting == NULL || value == NULL) {
		ERR("Invalid parameter");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	*value = setting->pop_up_notification;
	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_setting_set_pop_up_notification(notification_setting_h setting, bool value)
{
	if (setting == NULL) {
		ERR("Invalid parameter");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	setting->pop_up_notification = value;
	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_setting_get_lock_screen_content(notification_setting_h setting, lock_screen_content_level_e *level)
{
	if (setting == NULL || level == NULL) {
		ERR("Invalid parameter");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	*level = setting->lock_screen_content_level;

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_setting_set_lock_screen_content(notification_setting_h setting, lock_screen_content_level_e level)
{
	if (setting == NULL) {
		ERR("Invalid parameter");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	setting->lock_screen_content_level = level;

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_setting_get_app_disabled(notification_setting_h setting, bool *value)
{
	if (setting == NULL || value == NULL) {
		ERR("Invalid parameter");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	*value = setting->app_disabled;

	return NOTIFICATION_ERROR_NONE;
}
/* LCOV_EXCL_STOP */

EXPORT_API int notification_setting_update_setting_for_uid(notification_setting_h setting, uid_t uid)
{
	if (setting == NULL) {
		ERR("Invalid parameter");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	return notification_tidl_update_setting(setting, uid);
}

EXPORT_API int notification_setting_update_setting(notification_setting_h setting)
{
	return notification_setting_update_setting_for_uid(setting, aul_getuid());
}

EXPORT_API int notification_setting_free_notification(notification_setting_h setting)
{
	if (setting == NULL) {
		ERR("Invalid parameter");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	SAFE_FREE(setting->package_name);
	SAFE_FREE(setting->app_id);

	/* add codes to free all properties */

	SAFE_FREE(setting);

	return NOTIFICATION_ERROR_NONE;
}

/* LCOV_EXCL_START */
static bool _is_package_in_setting_table(sqlite3 *db, const char *package_name,
			const char *app_id, uid_t uid)
{
	using notification::raii::Sqlite3StmtPtr;
	using notification::raii::Sqlite3CharPtr;

	Sqlite3CharPtr query(
		(app_id != NULL && strlen(app_id) > 0)
			? sqlite3_mprintf("SELECT app_id FROM %s WHERE uid = %d "
					"AND package_name = %Q AND app_id = %Q",
					NOTIFICATION_SETTING_DB_TABLE,
					uid, package_name, app_id)
			: sqlite3_mprintf("SELECT package_name FROM %s "
					"WHERE uid = %d AND package_name = %Q",
					NOTIFICATION_SETTING_DB_TABLE,
					uid, package_name));
	if (!query) {
		ERR("fail to alloc query");
		return false;
	}

	sqlite3_stmt *raw_stmt = NULL;
	int sql_ret = sqlite3_prepare_v2(db, query.get(), -1, &raw_stmt, NULL);
	Sqlite3StmtPtr stmt(raw_stmt);
	if (sql_ret != SQLITE_OK) {
		ERR("sqlite3_prepare_v2 failed [%d][%s]", sql_ret,
				sqlite3_errmsg(db));
		return false;
	}

	sql_ret = sqlite3_step(stmt.get());
	if (sql_ret == SQLITE_DONE) {
		INFO("No matched [%s] from package_name [%s], [%d]",
				app_id, package_name, sql_ret);
		return false;
	}

	if (sql_ret != SQLITE_OK && sql_ret != SQLITE_ROW) {
		ERR("sqlite3_step failed [%d][%s]", sql_ret,
				sqlite3_errmsg(db));
		return false;
	}

	return true;
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
static int _foreach_app_info_callback(const pkgmgrinfo_appinfo_h handle,
				void *user_data)
{
	using notification::raii::Sqlite3CharPtr;

	setting_local_info *info = (setting_local_info *)user_data;
	sqlite3 *db = info->db;
	char *app_id = NULL;
	char *package_name = NULL;

	int ret = pkgmgrinfo_appinfo_get_appid(handle, &app_id);
	if (ret != PACKAGE_MANAGER_ERROR_NONE) {
		ERR("Failed to get appid from pkgmgrinfo [%d]", ret);
		return false;
	}

	ret = pkgmgrinfo_appinfo_get_pkgname(handle, &package_name);
	if (ret != PACKAGE_MANAGER_ERROR_NONE) {
		ERR("Failed to get pkgname from pkgmgrinfo[%d]", ret);
		return true;
	}

	if (_is_package_in_setting_table(db, package_name, app_id, info->uid) == true) {
		INFO("uid %d [%s] is exist", info->uid, app_id);
		return false;
	}

	Sqlite3CharPtr query(sqlite3_mprintf(
				"INSERT INTO %s (uid, package_name, app_id) "
				"VALUES (%d, %Q, %Q) ",
				NOTIFICATION_SETTING_DB_TABLE,
				info->uid, package_name, app_id));
	if (!query) {
		ERR("fail to alloc query");
		return false;
	}

	ret = notification_db_exec(db, query.get(), NULL);
	if (ret != NOTIFICATION_ERROR_NONE)
		return false;

	INFO("uid [%d] package_name [%s] appid [%s] is inserted",
			info->uid, package_name, app_id);
	return true;
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
static int _install_and_update_package(const char *package_name, uid_t uid)
{
	using notification::raii::Sqlite3DbPtr;

	Sqlite3DbPtr db(notification_db_open());
	if (!db)
		return get_last_result();

	pkgmgrinfo_appinfo_filter_h raw_handle = NULL;
	int pkgmgr_ret = pkgmgrinfo_appinfo_filter_create(&raw_handle);
	PkgmgrFilterPtr handle(raw_handle);
	if (pkgmgr_ret != PMINFO_R_OK) {
		ERR("Failed to create appinfo_filter[%d]", pkgmgr_ret);
		return NOTIFICATION_ERROR_FROM_DB;
	}

	pkgmgr_ret = pkgmgrinfo_appinfo_filter_add_string(handle.get(),
			PMINFO_APPINFO_PROP_PRIVILEGE, NOTIFICATION_PRIVILEGE);
	if (pkgmgr_ret != PMINFO_R_OK) {
		ERR("Failed to add string to appinfo_filter[%d]", pkgmgr_ret);
		return NOTIFICATION_ERROR_FROM_DB;
	}

	pkgmgr_ret = pkgmgrinfo_appinfo_filter_add_string(handle.get(),
			PMINFO_APPINFO_PROP_APP_PACKAGE, package_name);
	if (pkgmgr_ret != PMINFO_R_OK) {
		ERR("Failed to add string to appinfo_filter[%d]", pkgmgr_ret);
		return NOTIFICATION_ERROR_FROM_DB;
	}

	setting_local_info info;
	info.db = db.get();
	info.uid = uid;
	pkgmgr_ret = pkgmgrinfo_appinfo_usr_filter_foreach_appinfo(handle.get(),
			_foreach_app_info_callback, &info, uid);
	if (pkgmgr_ret != PMINFO_R_OK) {
		ERR("Failed to iterate appinfo[%d]", pkgmgr_ret);
		return NOTIFICATION_ERROR_FROM_DB;
	}

	return NOTIFICATION_ERROR_NONE;
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
static int _delete_package_from_setting_db(const char *package_name, uid_t uid)
{
	using notification::raii::Sqlite3DbPtr;
	using notification::raii::Sqlite3CharPtr;

	Sqlite3DbPtr db(notification_db_open());
	if (!db)
		return get_last_result();

	if (!_is_package_in_setting_table(db.get(), package_name, NULL, uid)) {
		INFO("[%s] is not exist", package_name);
		return NOTIFICATION_ERROR_NONE;
	}

	Sqlite3CharPtr query(sqlite3_mprintf(
				"DELETE FROM %s WHERE uid = %d "
				"AND package_name = %Q ",
				NOTIFICATION_SETTING_DB_TABLE, uid,
				package_name));
	if (!query) {
		ERR("Failed to alloc query");
		return NOTIFICATION_ERROR_OUT_OF_MEMORY;
	}

	return notification_db_exec(db.get(), query.get(), NULL);
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
EXPORT_API int notification_setting_refresh_setting_table(uid_t uid)
{
	using notification::raii::Sqlite3DbPtr;

	INFO("Refresh setting table [%d]", uid);

	Sqlite3DbPtr db(notification_db_open());
	if (!db)
		return get_last_result();

	pkgmgrinfo_appinfo_filter_h raw_filter = NULL;
	int pkgmgr_ret = pkgmgrinfo_appinfo_filter_create(&raw_filter);
	PkgmgrFilterPtr filter(raw_filter);
	if (pkgmgr_ret != PMINFO_R_OK) {
		ERR("Failed to create appinfo_filter[%d]", pkgmgr_ret);
		return NOTIFICATION_ERROR_FROM_DB;
	}

	pkgmgr_ret = pkgmgrinfo_appinfo_filter_add_string(filter.get(),
			PMINFO_APPINFO_PROP_PRIVILEGE, NOTIFICATION_PRIVILEGE);
	if (pkgmgr_ret != PMINFO_R_OK) {
		ERR("Failed to add string to appinfo_filter[%d]", pkgmgr_ret);
		return NOTIFICATION_ERROR_FROM_DB;
	}

	setting_local_info info;
	info.db = db.get();
	info.uid = uid;
	pkgmgr_ret = pkgmgrinfo_appinfo_usr_filter_foreach_appinfo(filter.get(),
			_foreach_app_info_callback, &info, uid);
	if (pkgmgr_ret != PMINFO_R_OK) {
		ERR("Failed to foreach appinfo[%d]", pkgmgr_ret);
		return NOTIFICATION_ERROR_FROM_DB;
	}

	return NOTIFICATION_ERROR_NONE;
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
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
		ERR("NOTIFICATION_ERROR_INVALID_PARAMETER");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	return notification_tidl_request_load_system_setting(system_setting, uid);
}

EXPORT_API int notification_system_setting_load_system_setting(notification_system_setting_h *system_setting)
{
	return notification_system_setting_load_system_setting_for_uid(system_setting, aul_getuid());
}

EXPORT_API int notification_system_setting_update_system_setting_for_uid(notification_system_setting_h system_setting, uid_t uid)
{
	if (system_setting == NULL) {
		ERR("Invalid parameter");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	return notification_tidl_update_system_setting(system_setting, uid);
}

EXPORT_API int notification_system_setting_update_system_setting(notification_system_setting_h system_setting)
{
	return notification_system_setting_update_system_setting_for_uid(system_setting, aul_getuid());
}

EXPORT_API int notification_system_setting_free_system_setting(notification_system_setting_h system_setting)
{
	if (system_setting == NULL) {
		ERR("Invalid parameter");
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
		ERR("Invalid parameter");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	*value = system_setting->do_not_disturb;

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_system_setting_set_do_not_disturb(notification_system_setting_h system_setting, bool value)
{
	if (system_setting == NULL) {
		ERR("Invalid parameter");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	system_setting->do_not_disturb = value;

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_system_setting_get_visibility_class(notification_system_setting_h system_setting, int *value)
{
	if (system_setting == NULL || value == NULL) {
		ERR("Invalid parameter");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	*value = system_setting->visibility_class;

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_system_setting_set_visibility_class(notification_system_setting_h system_setting, int value)
{
	if (system_setting == NULL) {
		ERR("Invalid parameter");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	system_setting->visibility_class = value;

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_system_setting_dnd_schedule_get_enabled(notification_system_setting_h system_setting, bool *enabled)
{
	if (system_setting == NULL || enabled == NULL) {
		ERR("Invalid parameter");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	*enabled = system_setting->dnd_schedule_enabled;

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_system_setting_dnd_schedule_set_enabled(notification_system_setting_h system_setting, bool enabled)
{
	if (system_setting == NULL) {
		ERR("Invalid parameter");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	system_setting->dnd_schedule_enabled = enabled;

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_system_setting_dnd_schedule_get_day(notification_system_setting_h system_setting, int *day)
{
	if (system_setting == NULL || day == NULL) {
		ERR("Invalid parameter");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	*day = system_setting->dnd_schedule_day;

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_system_setting_dnd_schedule_set_day(notification_system_setting_h system_setting, int day)
{
	if (system_setting == NULL) {
		ERR("Invalid parameter");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	system_setting->dnd_schedule_day = day;

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_system_setting_dnd_schedule_get_start_time(notification_system_setting_h system_setting, int *hour, int *min)
{
	if (system_setting == NULL || hour == NULL || min == NULL) {
		ERR("Invalid parameter");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	*hour = system_setting->dnd_start_hour;
	*min = system_setting->dnd_start_min;

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_system_setting_dnd_schedule_set_start_time(notification_system_setting_h system_setting, int hour, int min)
{
	if (system_setting == NULL) {
		ERR("Invalid parameter");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	system_setting->dnd_start_hour = hour;
	system_setting->dnd_start_min = min;

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_system_setting_dnd_schedule_get_end_time(notification_system_setting_h system_setting, int *hour, int *min)
{
	if (system_setting == NULL || hour == NULL || min == NULL) {
		ERR("Invalid parameter");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	*hour = system_setting->dnd_end_hour;
	*min = system_setting->dnd_end_min;

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_system_setting_dnd_schedule_set_end_time(notification_system_setting_h system_setting, int hour, int min)
{
	if (system_setting == NULL) {
		ERR("Invalid parameter");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	system_setting->dnd_end_hour = hour;
	system_setting->dnd_end_min = min;

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_system_setting_get_lock_screen_content(notification_system_setting_h system_setting, lock_screen_content_level_e *level)
{
	if (system_setting == NULL || level == NULL) {
		ERR("Invalid parameter");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	*level = system_setting->lock_screen_content_level;

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_system_setting_set_lock_screen_content(notification_system_setting_h system_setting, lock_screen_content_level_e level)
{
	if (system_setting == NULL) {
		ERR("Invalid parameter");
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
		ERR("Invalid parameter");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	list = g_list_find_custom(system_setting->dnd_allow_exceptions, GINT_TO_POINTER(type), (GCompareFunc)_dnd_allow_exception_compare);
	if (list) {
		dnd_allow_exception_data = (dnd_allow_exception_h)list->data;
		*value = dnd_allow_exception_data->value;
	} else {
		ERR("Invalid type");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_system_setting_set_dnd_allow_exceptions(notification_system_setting_h system_setting, dnd_allow_exception_type_e type, int value)
{
	dnd_allow_exception_h dnd_allow_exception_data;
	GList *list = NULL;

	if (system_setting == NULL) {
		ERR("Invalid parameter");
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
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
void notification_call_dnd_changed_cb_for_uid(int do_not_disturb, uid_t uid)
{
	GList *noti_dnd_cb_list = NULL;
	noti_dnd_cb_info_s *dnd_data = NULL;

	if (_noti_dnd_cb_hash == NULL)
		return;

	noti_dnd_cb_list = (GList *)g_hash_table_lookup(_noti_dnd_cb_hash, GUINT_TO_POINTER(uid));
	if (noti_dnd_cb_list == NULL) {
		ERR("Invalid data");
		return;
	}

	noti_dnd_cb_list = g_list_first(noti_dnd_cb_list);

	for (; noti_dnd_cb_list != NULL; noti_dnd_cb_list = noti_dnd_cb_list->next) {
		dnd_data = (noti_dnd_cb_info_s *)noti_dnd_cb_list->data;

		if (dnd_data != NULL && dnd_data->callback != NULL)
			dnd_data->callback(dnd_data->user_data, do_not_disturb);
	}
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
EXPORT_API int notification_register_system_setting_dnd_changed_cb_for_uid(dnd_changed_cb callback, void *user_data, uid_t uid)
{
	GList *noti_dnd_list = NULL;
	GList *noti_dnd_found_list = NULL;
	noti_dnd_cb_info_s *dnd_data = NULL;

	if (callback == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	if (notification_tidl_monitor_init() != NOTIFICATION_ERROR_NONE) {
		ERR("Failed to init monitor");
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
			ERR("Already existing callback");
			free(dnd_data);
			return NOTIFICATION_ERROR_INVALID_PARAMETER;
		}

		noti_dnd_list = g_list_append(noti_dnd_list, dnd_data);
	}

	return NOTIFICATION_ERROR_NONE;
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
EXPORT_API int notification_register_system_setting_dnd_changed_cb(dnd_changed_cb callback, void *user_data)
{
	return notification_register_system_setting_dnd_changed_cb_for_uid(callback, user_data, aul_getuid());
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
		dnd_data = (noti_dnd_cb_info_s *)g_list_nth_data(noti_dnd_del_list, 0);
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
		notification_tidl_monitor_fini();

	return NOTIFICATION_ERROR_NONE;
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
EXPORT_API int notification_unregister_system_setting_dnd_changed_cb(dnd_changed_cb callback)
{
	return notification_unregister_system_setting_dnd_changed_cb_for_uid(callback, aul_getuid());
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
static bool _is_uid_in_system_setting_table(sqlite3 *db, uid_t uid)
{
	using notification::raii::Sqlite3StmtPtr;
	using notification::raii::Sqlite3CharPtr;

	Sqlite3CharPtr query(sqlite3_mprintf(
				"SELECT uid FROM %s WHERE uid = %d",
				NOTIFICATION_SYSTEM_SETTING_DB_TABLE, uid));
	if (!query) {
		ERR("Failed to alloc query");
		/* Pre-existing quirk: returns truthy on OOM because the
		 * .c version did `return NOTIFICATION_ERROR_OUT_OF_MEMORY`
		 * from a bool function (any nonzero -> true). Preserved
		 * verbatim. */
		return true;
	}

	sqlite3_stmt *raw_stmt = NULL;
	int sql_ret = sqlite3_prepare_v2(db, query.get(), -1, &raw_stmt, NULL);
	Sqlite3StmtPtr stmt(raw_stmt);
	if (sql_ret != SQLITE_OK) {
		ERR("sqlite3_prepare_v2 failed [%d][%s]", sql_ret,
				sqlite3_errmsg(db));
		return false;
	}

	sql_ret = sqlite3_step(stmt.get());
	if (sql_ret == SQLITE_DONE) {
		INFO("No matched uid[%d] err[%d]", uid, sql_ret);
		return false;
	}

	if (sql_ret != SQLITE_OK && sql_ret != SQLITE_ROW) {
		ERR("sqlite3_step failed [%d][%s]",
				sql_ret, sqlite3_errmsg(db));
		return false;
	}

	return true;
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
EXPORT_API int notification_system_setting_init_system_setting_table(uid_t uid)
{
	using notification::raii::Sqlite3DbPtr;
	using notification::raii::Sqlite3CharPtr;

	INFO("init system setting table [%d]", uid);

	Sqlite3DbPtr db(notification_db_open());
	if (!db)
		return get_last_result();

	if (_is_uid_in_system_setting_table(db.get(), uid)) {
		INFO("Setting table is already initialized.");
		return NOTIFICATION_ERROR_NONE;
	}

	/* notification_system_setting */
	Sqlite3CharPtr query_system_setting(sqlite3_mprintf(
					"INSERT INTO %s (uid) "
					"VALUES (%d) ",
					NOTIFICATION_SYSTEM_SETTING_DB_TABLE,
					uid));
	if (!query_system_setting) {
		ERR("fail to alloc query");
		return NOTIFICATION_ERROR_OUT_OF_MEMORY;
	}

	int ret = notification_db_exec(db.get(), query_system_setting.get(), NULL);
	if (ret != NOTIFICATION_ERROR_NONE)
		return ret;

	/* dnd_allow_exception */
	Sqlite3CharPtr query_dnd_allow_exception(sqlite3_mprintf(
					"INSERT INTO %s "
					"(uid) VALUES (%d) ",
					NOTIFICATION_DND_ALLOW_EXCEPTION,
					uid));
	if (!query_dnd_allow_exception) {
		ERR("Failed to alloc query");
		return NOTIFICATION_ERROR_OUT_OF_MEMORY;
	}

	ret = notification_db_exec(db.get(), query_dnd_allow_exception.get(), NULL);
	if (ret != NOTIFICATION_ERROR_NONE)
		return ret;

	DBG("Initialization is success.");
	return NOTIFICATION_ERROR_NONE;
}
/* LCOV_EXCL_STOP */

}  // extern "C"
