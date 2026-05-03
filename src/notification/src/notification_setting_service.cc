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

#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <sqlite3.h>
#include <tizen.h>

#include <notification.h>
#include <notification_db.h>
#include <notification_error.h>
#include <notification_debug.h>
#include <notification_private.h>
#include <notification_setting.h>
#include <notification_setting_internal.h>
#include <notification_setting_service.h>
#include "notification_db_query.h"

#include "raii/sqlite3_db.hh"
#include "raii/sqlite3_stmt.hh"
#include "raii/sqlite3_free.hh"

extern "C" {

/* LCOV_EXCL_START */
static int _get_table_field_data_int(char **table, int *buf, int index)
{
	if (table == NULL || buf == NULL || index < 0) {
		ERR("table[%p], buf[%p], index[%d]", table, buf, index);
		return false;
	}

	if (table[index] != NULL) {
		*buf = atoi(table[index]);
		return true;
	}

	*buf = 0;
	return false;
}

static int _get_table_field_data_string(char **table, char **buf, int ucs2, int index)
{
	if (table == NULL || buf == NULL || index < 0) {
		ERR("table[%p], buf[%p], index[%d]", table, buf, index);
		return false;
	}

	char *pTemp = table[index];
	if (pTemp == NULL) {
		*buf = NULL; /* LCOV_EXCL_LINE */
		return true;
	}

	int sLen = strlen(pTemp);
	if (sLen == 0) {
		*buf = NULL; /* LCOV_EXCL_LINE */
		return true;
	}

	*buf = (char *)malloc(sLen + 1);
	if (*buf == NULL) {
		ERR("Failed to alloc memory"); /* LCOV_EXCL_LINE */
		return false;
	}
	memset(*buf, 0, sLen + 1);
	strncpy(*buf, pTemp, sLen + 1);
	return true;
}

EXPORT_API
int noti_setting_service_get_setting_by_app_id(const char *app_id, notification_setting_h *setting, uid_t uid)
{
	using notification::raii::Sqlite3DbPtr;
	using notification::raii::Sqlite3CharPtr;
	using notification::raii::Sqlite3TablePtr;

	if (app_id == NULL || setting == NULL) {
		ERR("Invalid parameter"); /* LCOV_EXCL_LINE */
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	Sqlite3DbPtr db(notification_db_open());
	if (!db)
		return get_last_result();

	Sqlite3CharPtr query(sqlite3_mprintf(
			"SELECT %s FROM %s WHERE app_id = %Q AND uid = %d",
			NOTIFICATION_SETTING_DB_ATTRIBUTES,
			NOTIFICATION_SETTING_DB_TABLE, app_id, uid));
	if (!query) {
		ERR("Failed to alloc query");
		return NOTIFICATION_ERROR_OUT_OF_MEMORY;
	}

	int row_count = 0;
	int column_count = 0;
	char **raw_query_result = NULL;
	int sql_ret = sqlite3_get_table(db.get(), query.get(),
			&raw_query_result, &row_count, &column_count, NULL);
	Sqlite3TablePtr query_result(raw_query_result);
	if (sql_ret != SQLITE_OK && sql_ret != -1) {
		ERR("sqlite3_get_table failed [%d][%s]", sql_ret, query.get());
		return NOTIFICATION_ERROR_FROM_DB;
	}

	if (!row_count) {
		DBG("No setting found for [%s]", app_id);
		return NOTIFICATION_ERROR_NOT_EXIST_ID;
	}

	DBG("row_count[%d] column_count[%d]", row_count, column_count);

	notification_setting_h result_setting_array =
			(struct notification_setting *)malloc(sizeof(struct notification_setting));
	if (result_setting_array == NULL) {
		ERR("Failed to alloc memory");
		return NOTIFICATION_ERROR_OUT_OF_MEMORY;
	}

	int col_index = column_count;
	int i = 0;
	_get_table_field_data_string(query_result.get(), &(result_setting_array[i].package_name), 1, col_index++);
	_get_table_field_data_string(query_result.get(), &(result_setting_array[i].app_id), 1, col_index++);
	_get_table_field_data_int(query_result.get(), (int *)&(result_setting_array[i].allow_to_notify), col_index++);
	_get_table_field_data_int(query_result.get(), (int *)&(result_setting_array[i].do_not_disturb_except), col_index++);
	_get_table_field_data_int(query_result.get(), &(result_setting_array[i].visibility_class), col_index++);
	_get_table_field_data_int(query_result.get(), (int *)&(result_setting_array[i].pop_up_notification), col_index++);
	_get_table_field_data_int(query_result.get(), (int *)&(result_setting_array[i].lock_screen_content_level), col_index++);
	_get_table_field_data_int(query_result.get(), (int *)&(result_setting_array[i].app_disabled), col_index++);

	*setting = result_setting_array;
	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API
int noti_setting_get_setting_array(notification_setting_h *setting_array, int *count, uid_t uid)
{
	using notification::raii::Sqlite3DbPtr;
	using notification::raii::Sqlite3CharPtr;
	using notification::raii::Sqlite3TablePtr;

	if (setting_array == NULL || count == NULL) {
		ERR("NOTIFICATION_ERROR_INVALID_PARAMETER"); /* LCOV_EXCL_LINE */
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	Sqlite3DbPtr db(notification_db_open());
	if (!db)
		return get_last_result();

	Sqlite3CharPtr query(sqlite3_mprintf(
			"SELECT %s FROM %s WHERE uid = %d "
			"AND app_disabled = %d ORDER BY package_name, "
			"app_id ", NOTIFICATION_SETTING_DB_ATTRIBUTES,
			NOTIFICATION_SETTING_DB_TABLE, uid, 0));
	if (!query) {
		ERR("Failed to alloc query"); /* LCOV_EXCL_LINE */
		return NOTIFICATION_ERROR_OUT_OF_MEMORY;
	}

	int row_count = 0;
	int column_count = 0;
	char **raw_query_result = NULL;
	int sql_ret = sqlite3_get_table(db.get(), query.get(),
			&raw_query_result, &row_count, &column_count, NULL);
	Sqlite3TablePtr query_result(raw_query_result);
	if (sql_ret != SQLITE_OK && sql_ret != -1) {
		ERR("Failed to get db table [%d][%s]",
			sql_ret, query.get()); /* LCOV_EXCL_LINE */
		return NOTIFICATION_ERROR_FROM_DB;
	}

	if (!row_count) {
		DBG("No setting found"); /* LCOV_EXCL_LINE */
		return NOTIFICATION_ERROR_NOT_EXIST_ID;
	}

	DBG("row_count[%d] column_count[%d]", row_count, column_count);

	notification_setting_h result_setting_array =
			(struct notification_setting *)malloc(
				sizeof(struct notification_setting) * row_count);
	if (result_setting_array == NULL) {
		ERR("Failed to alloc memory"); /* LCOV_EXCL_LINE */
		return NOTIFICATION_ERROR_OUT_OF_MEMORY;
	}

	int col_index = column_count;
	for (int i = 0; i < row_count; i++) {
		_get_table_field_data_string(query_result.get(), &(result_setting_array[i].package_name), 1, col_index++);
		_get_table_field_data_string(query_result.get(), &(result_setting_array[i].app_id), 1, col_index++);
		_get_table_field_data_int(query_result.get(), (int *)&(result_setting_array[i].allow_to_notify), col_index++);
		_get_table_field_data_int(query_result.get(), (int *)&(result_setting_array[i].do_not_disturb_except), col_index++);
		_get_table_field_data_int(query_result.get(), (int *)&(result_setting_array[i].visibility_class), col_index++);
		_get_table_field_data_int(query_result.get(), (int *)&(result_setting_array[i].pop_up_notification), col_index++);
		_get_table_field_data_int(query_result.get(), (int *)&(result_setting_array[i].lock_screen_content_level), col_index++);
		_get_table_field_data_int(query_result.get(), (int *)&(result_setting_array[i].app_disabled), col_index++);
	}

	*setting_array = result_setting_array;
	*count = row_count;
	return NOTIFICATION_ERROR_NONE;
}


EXPORT_API
int noti_system_setting_load_system_setting(notification_system_setting_h *system_setting, uid_t uid)
{
	using notification::raii::Sqlite3DbPtr;
	using notification::raii::Sqlite3CharPtr;
	using notification::raii::Sqlite3TablePtr;

	if (system_setting == NULL) {
		ERR("Invalid parameter"); /* LCOV_EXCL_LINE */
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	Sqlite3DbPtr db(notification_db_open());
	if (!db)
		return get_last_result();

	Sqlite3CharPtr query(sqlite3_mprintf(
			"SELECT %s FROM %s WHERE uid = %d",
			NOTIFICATION_SYSTEM_SETTING_DB_ATTRIBUTES,
			NOTIFICATION_SYSTEM_SETTING_DB_TABLE, uid));
	if (!query) {
		ERR("Failed to alloc query");
		return NOTIFICATION_ERROR_OUT_OF_MEMORY;
	}

	int row_count = 0;
	int column_count = 0;
	char **raw_query_result = NULL;
	int sql_ret = sqlite3_get_table(db.get(), query.get(),
			&raw_query_result, &row_count, &column_count, NULL);
	Sqlite3TablePtr query_result(raw_query_result);
	if (sql_ret != SQLITE_OK && sql_ret != -1) {
		ERR("Failed to get db table [%d][%s]", sql_ret, query.get());
		return NOTIFICATION_ERROR_FROM_DB;
	}

	DBG("row_count [%d] column_count [%d]", row_count, column_count);

	notification_system_setting_h result_system_setting =
			(struct notification_system_setting *)malloc(
				sizeof(struct notification_system_setting));
	if (result_system_setting == NULL) {
		ERR("Failed to alloc memory");
		return NOTIFICATION_ERROR_OUT_OF_MEMORY;
	}

	/* no system setting record. allow everyting */
	if (!row_count) {
		DBG("No setting found");
		result_system_setting->do_not_disturb = 0;
		result_system_setting->visibility_class = 0;
		result_system_setting->dnd_schedule_enabled = 0;
		result_system_setting->dnd_schedule_day = 0;
		result_system_setting->dnd_start_hour = 0;
		result_system_setting->dnd_start_min = 0;
		result_system_setting->dnd_end_hour = 0;
		result_system_setting->dnd_end_min = 0;
		result_system_setting->lock_screen_content_level = (lock_screen_content_level_e)0;
		result_system_setting->dnd_allow_exceptions = NULL;
	} else {
		int col_index = column_count;
		_get_table_field_data_int(query_result.get(), (int *)&(result_system_setting->do_not_disturb), col_index++);
		_get_table_field_data_int(query_result.get(), &(result_system_setting->visibility_class), col_index++);
		_get_table_field_data_int(query_result.get(), (int *)&(result_system_setting->dnd_schedule_enabled), col_index++);
		_get_table_field_data_int(query_result.get(), &(result_system_setting->dnd_schedule_day), col_index++);
		_get_table_field_data_int(query_result.get(), &(result_system_setting->dnd_start_hour), col_index++);
		_get_table_field_data_int(query_result.get(), &(result_system_setting->dnd_start_min), col_index++);
		_get_table_field_data_int(query_result.get(), &(result_system_setting->dnd_end_hour), col_index++);
		_get_table_field_data_int(query_result.get(), &(result_system_setting->dnd_end_min), col_index++);
		_get_table_field_data_int(query_result.get(), (int *)&(result_system_setting->lock_screen_content_level), col_index++);
		result_system_setting->dnd_allow_exceptions = NULL;
	}

	*system_setting = result_system_setting;
	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API
int notification_setting_db_update(const char *package_name, const char *app_id,
				int allow_to_notify, int do_not_disturb_except,
				int visibility_class, int pop_up_notification,
				int lock_screen_content_level, uid_t uid)
{
	using notification::raii::Sqlite3DbPtr;
	using notification::raii::Sqlite3CharPtr;

	if (package_name == NULL || app_id == NULL) {
		ERR("Invalid paramter"); /* LCOV_EXCL_LINE */
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	Sqlite3DbPtr db(notification_db_open());
	if (!db)
		return get_last_result();

	Sqlite3CharPtr query(sqlite3_mprintf(
			"UPDATE %s SET allow_to_notify = %d, "
			"do_not_disturb_except = %d, visibility_class = %d, "
			"pop_up_notification = %d, lock_screen_content_level = %d "
			"WHERE app_id = %Q AND package_name = %Q AND uid = %d ",
			NOTIFICATION_SETTING_DB_TABLE, allow_to_notify,
			do_not_disturb_except, visibility_class,
			pop_up_notification, lock_screen_content_level, app_id,
			package_name, uid));
	if (!query)
		return NOTIFICATION_ERROR_OUT_OF_MEMORY;

	return notification_db_exec(db.get(), query.get(), NULL);
}

EXPORT_API
int notification_setting_db_update_system_setting(int do_not_disturb, int visibility_class,
		int dnd_schedule_enabled, int dnd_schedule_day,
		int dnd_start_hour, int dnd_start_min,
		int dnd_end_hour, int dnd_end_min,
		int lock_screen_content_level, uid_t uid)
{
	using notification::raii::Sqlite3DbPtr;
	using notification::raii::Sqlite3CharPtr;

	Sqlite3DbPtr db(notification_db_open());
	if (!db)
		return get_last_result();

	Sqlite3CharPtr query(sqlite3_mprintf(
			"INSERT OR REPLACE INTO %s (uid, %s) "
			"VALUES(%d, %d, %d, %d, %d, %d, %d, %d, %d, %d) ",
			NOTIFICATION_SYSTEM_SETTING_DB_TABLE,
			NOTIFICATION_SYSTEM_SETTING_DB_ATTRIBUTES,
			uid, do_not_disturb, visibility_class,
			dnd_schedule_enabled, dnd_schedule_day, dnd_start_hour,
			dnd_start_min, dnd_end_hour, dnd_end_min,
			lock_screen_content_level));
	if (!query)
		return NOTIFICATION_ERROR_OUT_OF_MEMORY;

	int num_changes = 0;
	int ret = notification_db_exec(db.get(), query.get(), &num_changes);
	if (ret != NOTIFICATION_ERROR_NONE)
		return ret;

	if (num_changes == 0)
		WARN("No changes on DB");

	return NOTIFICATION_ERROR_NONE;
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
EXPORT_API
int notification_setting_db_update_do_not_disturb(int do_not_disturb, uid_t uid)
{
	using notification::raii::Sqlite3DbPtr;
	using notification::raii::Sqlite3CharPtr;

	Sqlite3DbPtr db(notification_db_open());
	if (!db)
		return get_last_result();

	Sqlite3CharPtr query(sqlite3_mprintf(
			"UPDATE %s SET do_not_disturb = %d "
			"WHERE uid = %d",
			NOTIFICATION_SYSTEM_SETTING_DB_TABLE,
			do_not_disturb, uid));
	if (!query) {
		ERR("Failed to alloc query");
		return NOTIFICATION_ERROR_OUT_OF_MEMORY;
	}

	return notification_db_exec(db.get(), query.get(), NULL);
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
EXPORT_API
int notification_system_setting_get_dnd_schedule_enabled_uid(uid_t **uids, int *count)
{
	using notification::raii::Sqlite3DbPtr;
	using notification::raii::Sqlite3CharPtr;
	using notification::raii::Sqlite3TablePtr;

	Sqlite3DbPtr db(notification_db_open());
	if (!db)
		return get_last_result();

	Sqlite3CharPtr query(sqlite3_mprintf(
			"SELECT uid FROM %s "
			"WHERE dnd_schedule_enabled = 1",
			NOTIFICATION_SYSTEM_SETTING_DB_TABLE));
	if (!query)
		return NOTIFICATION_ERROR_OUT_OF_MEMORY;

	int row_count = 0;
	int column_count = 0;
	char **raw_query_result = NULL;
	int sql_ret = sqlite3_get_table(db.get(), query.get(),
			&raw_query_result, &row_count, &column_count, NULL);
	Sqlite3TablePtr query_result(raw_query_result);
	if (sql_ret != SQLITE_OK && sql_ret != -1) {
		ERR("Failed to get DB table [%d][%s]", sql_ret, query.get());
		return NOTIFICATION_ERROR_FROM_DB;
	}

	if (row_count == 0) {
		DBG("No enabled do_not_disturb user");
		return NOTIFICATION_ERROR_NONE;
	}

	uid_t *result_uids = (uid_t *)malloc(sizeof(int) * row_count);
	if (result_uids == NULL) {
		ERR("Failed to alloc memory");
		return NOTIFICATION_ERROR_OUT_OF_MEMORY;
	}

	int column_index = column_count;
	for (int i = 0; i < row_count; i++)
		_get_table_field_data_int(query_result.get(), (int *)&(result_uids[i]), column_index++);

	*uids = result_uids;
	*count = row_count;
	return NOTIFICATION_ERROR_NONE;
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
EXPORT_API
int notification_get_dnd_and_allow_to_notify(const char *app_id,
					int *do_not_disturb,
					int *do_not_disturb_except,
					int *allow_to_notify,
					uid_t uid)
{
	using notification::raii::Sqlite3DbPtr;
	using notification::raii::Sqlite3CharPtr;
	using notification::raii::Sqlite3TablePtr;

	if (app_id == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	Sqlite3DbPtr db(notification_db_open());
	if (!db)
		return get_last_result();

	Sqlite3CharPtr query_setting(sqlite3_mprintf(
			"SELECT allow_to_notify, "
			"do_not_disturb_except FROM %s WHERE app_id = %Q "
			"AND (uid = %d OR uid = %d) ORDER BY uid DESC",
			NOTIFICATION_SETTING_DB_TABLE, app_id,
			uid, tzplatform_getuid(TZ_SYS_GLOBALAPP_USER)));
	if (!query_setting) {
		ERR("Failed to alloc memory");
		return NOTIFICATION_ERROR_OUT_OF_MEMORY;
	}

	Sqlite3CharPtr query_system_setting(sqlite3_mprintf(
			"SELECT do_not_disturb FROM %s WHERE uid = %d",
			NOTIFICATION_SYSTEM_SETTING_DB_TABLE, uid));
	if (!query_system_setting) {
		ERR("Failed to alloc memory");
		return NOTIFICATION_ERROR_OUT_OF_MEMORY;
	}

	int row_count;
	int col_count;
	char **raw_query_setting_result = NULL;
	int sql_ret = sqlite3_get_table(db.get(), query_setting.get(),
			&raw_query_setting_result, &row_count, &col_count, NULL);
	Sqlite3TablePtr query_setting_result(raw_query_setting_result);
	if (sql_ret != SQLITE_OK && sql_ret != -1) {
		ERR("Failed to get DB table [%d][%s]", sql_ret, query_setting.get());
		return NOTIFICATION_ERROR_FROM_DB;
	}

	if (row_count == 0) {
		ERR("Invalid uid [%d] or app id [%s]", uid, app_id);
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	int col_index = col_count;
	_get_table_field_data_int(query_setting_result.get(), (int *)allow_to_notify, col_index++);
	_get_table_field_data_int(query_setting_result.get(), (int *)do_not_disturb_except, col_index++);

	char **raw_query_system_setting_result = NULL;
	sql_ret = sqlite3_get_table(db.get(), query_system_setting.get(),
			&raw_query_system_setting_result, &row_count, &col_count, NULL);
	Sqlite3TablePtr query_system_setting_result(raw_query_system_setting_result);
	if (sql_ret != SQLITE_OK && sql_ret != -1) {
		ERR("Failed to get DB table [%d][%s]", sql_ret, query_setting.get());
		return NOTIFICATION_ERROR_FROM_DB;
	}

	if (row_count == 0) {
		ERR("Invalid uid [%d]", uid);
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	col_index = col_count;
	_get_table_field_data_int(query_system_setting_result.get(), (int *)do_not_disturb, col_index++);
	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_system_setting_load_dnd_allow_exception(dnd_allow_exception_h *dnd_allow_exception, int *count, uid_t uid)
{
	using notification::raii::Sqlite3DbPtr;
	using notification::raii::Sqlite3CharPtr;
	using notification::raii::Sqlite3TablePtr;

	if (dnd_allow_exception == NULL) {
		ERR("Invalid paramter"); /* LCOV_EXCL_LINE */
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	Sqlite3DbPtr db(notification_db_open());
	if (!db)
		return get_last_result();

	Sqlite3CharPtr query(sqlite3_mprintf(
			"SELECT type, value FROM %s WHERE uid = %d",
			NOTIFICATION_DND_ALLOW_EXCEPTION, uid));
	if (!query) {
		ERR("Failed to alloc memory");
		return NOTIFICATION_ERROR_OUT_OF_MEMORY;
	}

	int row_count = 0;
	int column_count = 0;
	char **raw_query_result = NULL;
	int sql_ret = sqlite3_get_table(db.get(), query.get(),
			&raw_query_result, &row_count, &column_count, NULL);
	Sqlite3TablePtr query_result(raw_query_result);
	if (sql_ret != SQLITE_OK && sql_ret != -1) {
		ERR("Failed to get DB table [%d][%s]", sql_ret, query.get());
		return NOTIFICATION_ERROR_FROM_DB;
	}

	if (!row_count) {
		DBG("No setting found"); /* LCOV_EXCL_LINE */
		return NOTIFICATION_ERROR_NONE;
	}

	dnd_allow_exception_h dnd_allow_exception_data =
			(dnd_allow_exception_h)malloc(
				sizeof(struct notification_system_setting_dnd_allow_exception)
				* row_count);
	if (dnd_allow_exception_data == NULL) {
		ERR("Failed to alloc memory");
		return NOTIFICATION_ERROR_OUT_OF_MEMORY;
	}

	int col_index = column_count;
	for (int i = 0; i < row_count; i++) {
		_get_table_field_data_int(query_result.get(), (int *)&(dnd_allow_exception_data[i].type), col_index++);
		_get_table_field_data_int(query_result.get(), (int *)&(dnd_allow_exception_data[i].value), col_index++);
	}

	*dnd_allow_exception = dnd_allow_exception_data;
	*count = row_count;
	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API
int notification_system_setting_update_dnd_allow_exception(int type, int value, uid_t uid)
{
	using notification::raii::Sqlite3DbPtr;
	using notification::raii::Sqlite3CharPtr;

	Sqlite3DbPtr db(notification_db_open());
	if (!db)
		return get_last_result();

	Sqlite3CharPtr query(sqlite3_mprintf(
			"INSERT OR REPLACE INTO %s (uid, type, value) "
			"VALUES(%d, %d, %d) ",
			NOTIFICATION_DND_ALLOW_EXCEPTION,
			uid, type, value));
	if (!query) {
		ERR("Failed to alloc memory");
		return NOTIFICATION_ERROR_OUT_OF_MEMORY;
	}

	int num_changes = 0;
	int ret = notification_db_exec(db.get(), query.get(), &num_changes);
	if (ret != NOTIFICATION_ERROR_NONE)
		return ret;

	if (num_changes == 0)
		WARN("No changes on DB");

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API
int noti_system_setting_get_do_not_disturb(int *do_not_disturb, uid_t uid)
{
	using notification::raii::Sqlite3DbPtr;
	using notification::raii::Sqlite3CharPtr;
	using notification::raii::Sqlite3TablePtr;

	Sqlite3DbPtr db(notification_db_open());
	if (!db)
		return get_last_result();

	Sqlite3CharPtr query(sqlite3_mprintf(
			"SELECT do_not_disturb FROM %s WHERE uid = %d",
			NOTIFICATION_SYSTEM_SETTING_DB_TABLE, uid));
	if (!query) {
		ERR("Failed to alloc memory");
		return NOTIFICATION_ERROR_OUT_OF_MEMORY;
	}

	int row_count = 0;
	int col_count = 0;
	char **raw_query_result = NULL;
	int sql_ret = sqlite3_get_table(db.get(), query.get(),
			&raw_query_result, &row_count, &col_count, NULL);
	Sqlite3TablePtr query_result(raw_query_result);
	if (sql_ret != SQLITE_OK && sql_ret != -1) {
		ERR("Failed to get DB table [%d][%s]", sql_ret, query.get());
		return NOTIFICATION_ERROR_FROM_DB;
	}

	if (row_count == 0) {
		ERR("No system setting found");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	int col_index = col_count;
	if (_get_table_field_data_int(query_result.get(), (int *)do_not_disturb, col_index++) == false)
		return NOTIFICATION_ERROR_FROM_DB;

	return NOTIFICATION_ERROR_NONE;
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
EXPORT_API
int notification_setting_db_update_app_disabled(const char *app_id, bool value, uid_t uid)
{
	using notification::raii::Sqlite3DbPtr;
	using notification::raii::Sqlite3CharPtr;

	if (app_id == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	Sqlite3DbPtr db(notification_db_open());
	if (!db)
		return get_last_result();

	Sqlite3CharPtr query(sqlite3_mprintf(
			"UPDATE %s SET app_disabled= %d "
			"WHERE app_id = %Q AND uid = %d",
			NOTIFICATION_SETTING_DB_TABLE, value,
			app_id, uid));
	if (!query) {
		ERR("Failed to alloc memory"); /* LCOV_EXCL_LINE */
		return NOTIFICATION_ERROR_OUT_OF_MEMORY;
	}

	int num_changes = 0;
	int ret = notification_db_exec(db.get(), query.get(), &num_changes);
	if (ret == NOTIFICATION_ERROR_NONE && num_changes <= 0)
		ret = NOTIFICATION_ERROR_NOT_EXIST_ID;
	return ret;
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
EXPORT_API
int notification_setting_db_update_pkg_disabled(const char *pkg_id, bool value, uid_t uid)
{
	using notification::raii::Sqlite3DbPtr;
	using notification::raii::Sqlite3CharPtr;

	if (pkg_id == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	Sqlite3DbPtr db(notification_db_open());
	if (!db)
		return get_last_result();

	Sqlite3CharPtr query(sqlite3_mprintf(
			"UPDATE %s SET app_disabled= %d "
			"WHERE package_name = %Q AND uid = %d",
			NOTIFICATION_SETTING_DB_TABLE, value,
			pkg_id, uid));
	if (!query) {
		ERR("Failed to alloc memory"); /* LCOV_EXCL_LINE */
		return NOTIFICATION_ERROR_OUT_OF_MEMORY;
	}

	int num_changes = 0;
	int ret = notification_db_exec(db.get(), query.get(), &num_changes);
	if (ret == NOTIFICATION_ERROR_NONE && num_changes <= 0)
		ret = NOTIFICATION_ERROR_NOT_EXIST_ID;
	return ret;
}
/* LCOV_EXCL_STOP */


}  // extern "C"
