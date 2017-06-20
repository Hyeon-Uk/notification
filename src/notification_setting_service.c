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

#include <sqlite3.h>
#include <db-util.h>
#include <tizen.h>

#include <notification.h>
#include <notification_db.h>
#include <notification_error.h>
#include <notification_debug.h>
#include <notification_private.h>
#include <notification_setting.h>
#include <notification_setting_internal.h>
#include <notification_setting_service.h>

static int _get_table_field_data_int(char  **table, int *buf, int index)
{
	if (table == NULL || buf == NULL || index < 0) {
		/* LCOV_EXCL_START */
		NOTIFICATION_ERR("table[%p], buf[%p], index[%d]", table, buf, index);
		return false;
		/* LCOV_EXCL_STOP */
	}

	if (table[index] != NULL) {
		*buf = atoi(table[index]);
		return true;
	}

	/* LCOV_EXCL_START */
	*buf = 0;
	return false;
	/* LCOV_EXCL_STOP */
}

static int _get_table_field_data_string(char **table, char **buf, int ucs2, int index)
{
	int ret = false;
	int sLen = 0;
	char *pTemp;

	if (table == NULL || buf == NULL || index < 0) {
		/* LCOV_EXCL_START */
		NOTIFICATION_ERR("table[%p], buf[%p], index[%d]", table, buf, index);
		return false;
		/* LCOV_EXCL_STOP */
	}

	pTemp = table[index];

	if (pTemp == NULL) {
		*buf = NULL; /* LCOV_EXCL_LINE */
	} else {
		sLen = strlen(pTemp);
		if (sLen) {
			*buf = (char *)malloc(sLen + 1);
			if (*buf == NULL) {
				NOTIFICATION_ERR("malloc is failed"); /* LCOV_EXCL_LINE */
				goto out;
			}
			memset(*buf, 0, sLen + 1);
			strncpy(*buf, pTemp, sLen);
		} else {
			*buf = NULL; /* LCOV_EXCL_LINE */
		}
	}

	ret = true;
out:

	return ret;
}

EXPORT_API
int noti_setting_service_get_setting_by_app_id(const char *app_id, notification_setting_h *setting, uid_t uid)
{
	int ret = NOTIFICATION_ERROR_NONE;
	sqlite3 *db = NULL;
	char *query = NULL;
	char **query_result = NULL;
	int sql_ret;
	int row_count = 0;
	int column_count = 0;
	int i = 0;
	int col_index = 0;
	notification_setting_h result_setting_array = NULL;

	if (app_id == NULL || setting == NULL) {
		NOTIFICATION_ERR("NOTIFICATION_ERROR_INVALID_PARAMETER"); /* LCOV_EXCL_LINE */
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	sql_ret = db_util_open(DBPATH, &db, 0);
	if (sql_ret != SQLITE_OK || db == NULL) {
		NOTIFICATION_ERR("db_util_open failed [%d]", sql_ret); /* LCOV_EXCL_LINE */
		return NOTIFICATION_ERROR_FROM_DB;
	}

	query = sqlite3_mprintf("SELECT package_name, app_id, allow_to_notify, do_not_disturb_except, visibility_class, "
				    "pop_up_notification, lock_screen_content_level, app_disabled FROM %s "
				    "WHERE app_id = %Q AND uid = %d", NOTIFICATION_SETTING_DB_TABLE, app_id, uid);
	if (!query) {
		NOTIFICATION_ERR("fail to alloc query"); /* LCOV_EXCL_LINE */
		ret = NOTIFICATION_ERROR_OUT_OF_MEMORY;
		goto out;
	}

	sql_ret = sqlite3_get_table(db, query, &query_result, &row_count, &column_count, NULL);
	if (sql_ret != SQLITE_OK && sql_ret != -1) {
		NOTIFICATION_ERR("sqlite3_get_table failed [%d][%s]", sql_ret, query); /* LCOV_EXCL_LINE */
		ret = NOTIFICATION_ERROR_FROM_DB;
		goto out;
	}

	if (!row_count) {
		NOTIFICATION_DBG("No setting found for [%s]", app_id); /* LCOV_EXCL_LINE */
		ret = NOTIFICATION_ERROR_NOT_EXIST_ID;
		goto out;
	}

	NOTIFICATION_DBG("row_count [%d] column_count [%d]", row_count, column_count);

	row_count = 1;

	if (!(result_setting_array = (struct notification_setting *)malloc(sizeof(struct notification_setting) * row_count))) {
		NOTIFICATION_ERR("malloc failed..."); /* LCOV_EXCL_LINE */
		ret = NOTIFICATION_ERROR_OUT_OF_MEMORY;
		goto out;
	}

	col_index = column_count;

	_get_table_field_data_string(query_result, &(result_setting_array[i].package_name), 1, col_index++);
	_get_table_field_data_string(query_result, &(result_setting_array[i].app_id), 1, col_index++);
	_get_table_field_data_int(query_result, (int *)&(result_setting_array[i].allow_to_notify), col_index++);
	_get_table_field_data_int(query_result, (int *)&(result_setting_array[i].do_not_disturb_except), col_index++);
	_get_table_field_data_int(query_result, &(result_setting_array[i].visibility_class), col_index++);
	_get_table_field_data_int(query_result, (int *)&(result_setting_array[i].pop_up_notification), col_index++);
	_get_table_field_data_int(query_result, (int *)&(result_setting_array[i].lock_screen_content_level), col_index++);
	_get_table_field_data_int(query_result, (int *)&(result_setting_array[i].app_disabled), col_index++);

	*setting = result_setting_array;

out:
	if (query_result)
		sqlite3_free_table(query_result);

	if (query)
		sqlite3_free(query);

	if (db) {
		sql_ret = db_util_close(db);
		if (sql_ret != SQLITE_OK)
			NOTIFICATION_WARN("fail to db_util_close - [%d]", sql_ret); /* LCOV_EXCL_LINE */
	}

	return ret;
}

EXPORT_API
int noti_setting_get_setting_array(notification_setting_h *setting_array, int *count, uid_t uid)
{
	int ret = NOTIFICATION_ERROR_NONE;
	sqlite3 *db = NULL;
	char *query = NULL;
	char **query_result = NULL;
	int sql_ret;
	int row_count = 0;
	int column_count = 0;
	int i = 0;
	int col_index = 0;
	notification_setting_h result_setting_array = NULL;

	if (setting_array == NULL || count == NULL) {
		NOTIFICATION_ERR("NOTIFICATION_ERROR_INVALID_PARAMETER"); /* LCOV_EXCL_LINE */
		return  NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	sql_ret = db_util_open(DBPATH, &db, 0);
	if (sql_ret != SQLITE_OK || db == NULL) {
		NOTIFICATION_ERR("db_util_open failed [%d]", sql_ret); /* LCOV_EXCL_LINE */
		return NOTIFICATION_ERROR_FROM_DB;
	}

	query = sqlite3_mprintf("SELECT package_name, app_id, allow_to_notify, do_not_disturb_except, visibility_class, "
				    "pop_up_notification, lock_screen_content_level, app_disabled FROM %s WHERE uid = %d AND app_disabled = %d "
				    "ORDER BY package_name, app_id ", NOTIFICATION_SETTING_DB_TABLE, uid, 0);
	if (!query) {
		NOTIFICATION_ERR("fail to alloc query"); /* LCOV_EXCL_LINE */
		ret = NOTIFICATION_ERROR_OUT_OF_MEMORY;
		goto out;
	}

	sql_ret = sqlite3_get_table(db, query, &query_result, &row_count, &column_count, NULL);
	if (sql_ret != SQLITE_OK && sql_ret != -1) {
		NOTIFICATION_ERR("NOTIFICATION_ERROR_FROM_DB failed [%d][%s]", sql_ret, query);	/* LCOV_EXCL_LINE */
		ret = NOTIFICATION_ERROR_FROM_DB;
		goto out;
	}

	if (!row_count) {
		NOTIFICATION_DBG("No setting found..."); /* LCOV_EXCL_LINE */
		ret = NOTIFICATION_ERROR_NOT_EXIST_ID;
		goto out;
	}

	NOTIFICATION_DBG("row_count [%d] column_count [%d]", row_count, column_count);

	if (!(result_setting_array = (struct notification_setting *)malloc(sizeof(struct notification_setting) * row_count))) {
		NOTIFICATION_ERR("malloc failed..."); /* LCOV_EXCL_LINE */
		ret = NOTIFICATION_ERROR_OUT_OF_MEMORY;
		goto out;
	}

	col_index = column_count;

	for (i = 0; i < row_count; i++) {
		_get_table_field_data_string(query_result, &(result_setting_array[i].package_name), 1, col_index++);
		_get_table_field_data_string(query_result, &(result_setting_array[i].app_id), 1, col_index++);
		_get_table_field_data_int(query_result, (int *)&(result_setting_array[i].allow_to_notify), col_index++);
		_get_table_field_data_int(query_result, (int *)&(result_setting_array[i].do_not_disturb_except), col_index++);
		_get_table_field_data_int(query_result, (int *)&(result_setting_array[i].visibility_class), col_index++);
		_get_table_field_data_int(query_result, (int *)&(result_setting_array[i].pop_up_notification), col_index++);
		_get_table_field_data_int(query_result, (int *)&(result_setting_array[i].lock_screen_content_level), col_index++);
		_get_table_field_data_int(query_result, (int *)&(result_setting_array[i].app_disabled), col_index++);
	}

	*setting_array = result_setting_array;
	*count = row_count;

out:
	if (query_result)
		sqlite3_free_table(query_result);

	if (query)
		sqlite3_free(query);

	if (db) {
		sql_ret = db_util_close(db);
		if (sql_ret != SQLITE_OK)
			NOTIFICATION_WARN("fail to db_util_close - [%d]", sql_ret); /* LCOV_EXCL_LINE */
	}

	return ret;
}


EXPORT_API
int noti_system_setting_load_system_setting(notification_system_setting_h *system_setting, uid_t uid)
{
	int ret = NOTIFICATION_ERROR_NONE;
	sqlite3 *db = NULL;
	char *query = NULL;
	char **query_result = NULL;
	int sql_ret;
	int row_count = 0;
	int column_count = 0;
	int col_index = 0;
	notification_system_setting_h result_system_setting = NULL;

	if (system_setting == NULL) {
		NOTIFICATION_ERR("NOTIFICATION_ERROR_INVALID_PARAMETER"); /* LCOV_EXCL_LINE */
		return  NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	sql_ret = db_util_open(DBPATH, &db, 0);
	if (sql_ret != SQLITE_OK || db == NULL) {
		NOTIFICATION_ERR("db_util_open failed [%d]", sql_ret); /* LCOV_EXCL_LINE */
		return NOTIFICATION_ERROR_FROM_DB;
	}

	query = sqlite3_mprintf("SELECT do_not_disturb, visibility_class, "
			"dnd_schedule_enabled, dnd_schedule_day, "
			"dnd_start_hour, dnd_start_min, dnd_end_hour, dnd_end_min, "
			"lock_screen_content_level "
			"FROM %s WHERE uid = %d", NOTIFICATION_SYSTEM_SETTING_DB_TABLE, uid);
	if (!query) {
		NOTIFICATION_ERR("fail to alloc query"); /* LCOV_EXCL_LINE */
		ret = NOTIFICATION_ERROR_OUT_OF_MEMORY;
		goto out;
	}

	sql_ret = sqlite3_get_table(db, query, &query_result, &row_count, &column_count, NULL);
	if (sql_ret != SQLITE_OK && sql_ret != -1) {
		NOTIFICATION_ERR("sqlite3_get_table failed [%d][%s]", sql_ret, query); /* LCOV_EXCL_LINE */
		ret = NOTIFICATION_ERROR_FROM_DB;
		goto out;
	}

	NOTIFICATION_DBG("row_count [%d] column_count [%d]", row_count, column_count);

	if (!(result_system_setting = (struct notification_system_setting *)malloc(sizeof(struct notification_system_setting)))) {
		NOTIFICATION_ERR("malloc failed..."); /* LCOV_EXCL_LINE */
		ret = NOTIFICATION_ERROR_OUT_OF_MEMORY;
		goto out;
	}

	/* no system setting record. allow everyting */
	if (!row_count) {
		NOTIFICATION_DBG("No setting found..."); /* LCOV_EXCL_LINE */
		result_system_setting->do_not_disturb = 0;
		result_system_setting->visibility_class = 0;
		result_system_setting->dnd_schedule_enabled = 0;
		result_system_setting->dnd_schedule_day = 0;
		result_system_setting->dnd_start_hour = 0;
		result_system_setting->dnd_start_min = 0;
		result_system_setting->dnd_end_hour = 0;
		result_system_setting->dnd_end_min = 0;
		result_system_setting->lock_screen_content_level = 0;
		result_system_setting->dnd_allow_exceptions = NULL;
	} else {
		/* LCOV_EXCL_START */
		col_index = column_count;
		_get_table_field_data_int(query_result, (int *)&(result_system_setting->do_not_disturb), col_index++);
		_get_table_field_data_int(query_result, &(result_system_setting->visibility_class), col_index++);
		_get_table_field_data_int(query_result, (int *)&(result_system_setting->dnd_schedule_enabled), col_index++);
		_get_table_field_data_int(query_result, &(result_system_setting->dnd_schedule_day), col_index++);
		_get_table_field_data_int(query_result, &(result_system_setting->dnd_start_hour), col_index++);
		_get_table_field_data_int(query_result, &(result_system_setting->dnd_start_min), col_index++);
		_get_table_field_data_int(query_result, &(result_system_setting->dnd_end_hour), col_index++);
		_get_table_field_data_int(query_result, &(result_system_setting->dnd_end_min), col_index++);
		_get_table_field_data_int(query_result, (int *)&(result_system_setting->lock_screen_content_level), col_index++);
		result_system_setting->dnd_allow_exceptions = NULL;
		/* LCOV_EXCL_STOP */
	}

	*system_setting = result_system_setting;
out:
	if (query_result)
			sqlite3_free_table(query_result);

	if (query)
		sqlite3_free(query);

	if (db) {
		sql_ret = db_util_close(db);
		if (sql_ret != SQLITE_OK)
			NOTIFICATION_WARN("fail to db_util_close - [%d]", sql_ret); /* LCOV_EXCL_LINE */
	}

	return ret;
}

EXPORT_API
int notification_setting_db_update(const char *package_name, const char *app_id,
				int allow_to_notify, int do_not_disturb_except,
				int visibility_class, int pop_up_notification,
				int lock_screen_content_level, uid_t uid)
{
	int ret = NOTIFICATION_ERROR_NONE;
	sqlite3 *db = NULL;
	char *sqlbuf = NULL;
	int sql_ret;

	if (package_name == NULL || strlen(package_name) == 0
	    || app_id == NULL || strlen(app_id) == 0)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	sql_ret = db_util_open(DBPATH, &db, 0);
	if (sql_ret != SQLITE_OK || db == NULL) {
		NOTIFICATION_ERR("db_util_open failed [%s][%d]", DBPATH, sql_ret);
		return NOTIFICATION_ERROR_FROM_DB;
	}

	sqlbuf = sqlite3_mprintf("UPDATE %s SET allow_to_notify = %d, do_not_disturb_except = %d, visibility_class = %d, " \
				 "pop_up_notification = %d, lock_screen_content_level = %d " \
				 "WHERE app_id = %Q AND package_name = %Q AND uid = %d",
			NOTIFICATION_SETTING_DB_TABLE, allow_to_notify, do_not_disturb_except, visibility_class,
			pop_up_notification, lock_screen_content_level, app_id, package_name, uid);
	if (!sqlbuf) {
		NOTIFICATION_ERR("fail to alloc query");
		ret = NOTIFICATION_ERROR_OUT_OF_MEMORY;
		goto out;
	}

	ret = notification_db_exec(db, sqlbuf, NULL);

out:
	if (sqlbuf)
		sqlite3_free(sqlbuf);

	if (db) {
		sql_ret = db_util_close(db);
		if (sql_ret != SQLITE_OK)
			NOTIFICATION_WARN("fail to db_util_close - [%d]", sql_ret); /* LCOV_EXCL_LINE */
	}

	return ret;
}

EXPORT_API
int notification_setting_db_update_system_setting(int do_not_disturb, int visibility_class,
		int dnd_schedule_enabled, int dnd_schedule_day,
		int dnd_start_hour, int dnd_start_min,
		int dnd_end_hour, int dnd_end_min,
		int lock_screen_content_level, uid_t uid)
{
	int ret = NOTIFICATION_ERROR_NONE;
	int sql_ret;
	int field_index = 1;
	sqlite3 *db = NULL;
	sqlite3_stmt *stmt = NULL;

	sql_ret = db_util_open(DBPATH, &db, 0);
	if (sql_ret != SQLITE_OK || db == NULL) {
		NOTIFICATION_ERR("db_util_open failed [%s][%d][%s]", DBPATH, sql_ret, sqlite3_errmsg(db));
		return  NOTIFICATION_ERROR_FROM_DB;
	}

	sqlite3_exec(db, "BEGIN immediate;", NULL, NULL, NULL);

	sql_ret = sqlite3_prepare_v2(db, "INSERT OR REPLACE INTO notification_system_setting "
			"(uid, do_not_disturb, visibility_class, dnd_schedule_enabled, "
			"dnd_schedule_day, dnd_start_hour, dnd_start_min, "
			"dnd_end_hour, dnd_end_min, lock_screen_content_level) "
			"VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?) ", -1, &stmt, NULL);
	if (sql_ret != SQLITE_OK) {
		NOTIFICATION_ERR("sqlite3_prepare_v2 failed [%d][%s]", sql_ret, sqlite3_errmsg(db));
		ret =  NOTIFICATION_ERROR_FROM_DB;
		goto out;
	}

	sqlite3_bind_int(stmt, field_index++, uid);
	sqlite3_bind_int(stmt, field_index++, do_not_disturb);
	sqlite3_bind_int(stmt, field_index++, visibility_class);
	sqlite3_bind_int(stmt, field_index++, dnd_schedule_enabled);
	sqlite3_bind_int(stmt, field_index++, dnd_schedule_day);
	sqlite3_bind_int(stmt, field_index++, dnd_start_hour);
	sqlite3_bind_int(stmt, field_index++, dnd_start_min);
	sqlite3_bind_int(stmt, field_index++, dnd_end_hour);
	sqlite3_bind_int(stmt, field_index++, dnd_end_min);
	sqlite3_bind_int(stmt, field_index++, lock_screen_content_level);

	sql_ret = sqlite3_step(stmt);
	if (sql_ret != SQLITE_OK && sql_ret != SQLITE_DONE) {
		NOTIFICATION_ERR("sqlite3_step failed [%d][%s]", sql_ret, sqlite3_errmsg(db));
		ret =  NOTIFICATION_ERROR_FROM_DB;
		goto out;
	}

	sql_ret = sqlite3_changes(db);
	if (sql_ret == 0)
		NOTIFICATION_WARN("No changes on DB");

out:
	if (stmt)
		sqlite3_finalize(stmt);

	if (db) {
		if (ret == NOTIFICATION_ERROR_NONE)
			sqlite3_exec(db, "END;", NULL, NULL, NULL);
		else
			sqlite3_exec(db, "ROLLBACK;", NULL, NULL, NULL);

		sql_ret = db_util_close(db);
	}

	if (sql_ret != SQLITE_OK)
		NOTIFICATION_WARN("fail to db_util_close - [%d]", sql_ret);

	return ret;
}

/* LCOV_EXCL_START */
EXPORT_API
int notification_setting_db_update_do_not_disturb(int do_not_disturb, uid_t uid)
{
	int ret = NOTIFICATION_ERROR_NONE;
	int sql_ret;
	sqlite3 *db = NULL;
	char *sqlbuf = NULL;

	sql_ret = db_util_open(DBPATH, &db, 0);
	if (sql_ret != SQLITE_OK || db == NULL) {
		NOTIFICATION_ERR("db_util_open failed [%s][%d]", DBPATH, sql_ret);
		return NOTIFICATION_ERROR_FROM_DB;
	}

	sqlbuf = sqlite3_mprintf("UPDATE notification_system_setting SET do_not_disturb = %d WHERE uid = %d", do_not_disturb, uid);
	if (!sqlbuf) {
		NOTIFICATION_ERR("fail to alloc query");
		ret = NOTIFICATION_ERROR_OUT_OF_MEMORY;
		goto out;
	}

	ret = notification_db_exec(db, sqlbuf, NULL);

out:
	if (sqlbuf)
		sqlite3_free(sqlbuf);

	if (db) {
		sql_ret = db_util_close(db);
		if (sql_ret != SQLITE_OK)
			NOTIFICATION_WARN("fail to db_util_close - [%d]", sql_ret); /* LCOV_EXCL_LINE */
	}

	return ret;
}
/* LCOV_EXCL_STOP */

EXPORT_API
int notification_system_setting_get_dnd_schedule_enabled_uid(uid_t **uids, int *count)
{
	int ret = NOTIFICATION_ERROR_NONE;
	int i;
	int sql_ret;
	int row_count = 0;
	int column_count = 0;
	int column_index = 0;
	char *query = NULL;
	char **query_result = NULL;
	sqlite3 *db = NULL;
	uid_t *result_uids;

	sql_ret = db_util_open(DBPATH, &db, 0);
	if (sql_ret != SQLITE_OK || db == NULL) {
		NOTIFICATION_ERR("db_util_open failed [%s][%d]", DBPATH, sql_ret);
		return NOTIFICATION_ERROR_FROM_DB;
	}

	query = sqlite3_mprintf("SELECT uid FROM %s WHERE dnd_schedule_enabled = 1",
				NOTIFICATION_SYSTEM_SETTING_DB_TABLE);
	if (query == NULL) {
		ret = NOTIFICATION_ERROR_OUT_OF_MEMORY;
		goto out;
	}

	ret = sqlite3_get_table(db, query, &query_result, &row_count, &column_count, NULL);
	if (ret != SQLITE_OK && ret != -1) {
		NOTIFICATION_ERR("NOTIFICATION_ERROR_FROM_DB failed [%d][%s]", ret, query);	/* LCOV_EXCL_LINE */
		ret = NOTIFICATION_ERROR_FROM_DB;
		goto out;
	}

	if (row_count == 0) {
		NOTIFICATION_DBG("No enabled do_not_disturb user");
		ret = NOTIFICATION_ERROR_NONE;
		goto out;
	}

	if (!(result_uids = (uid_t *)malloc(sizeof(int) * row_count))) {
		NOTIFICATION_ERR("Memory allocation fail");
		ret = NOTIFICATION_ERROR_OUT_OF_MEMORY;
		goto out;
	}

	column_index = column_count;
	for (i = 0; i < row_count; i++)
		_get_table_field_data_int(query_result, (int *)&(result_uids[i]), column_index++);

	*uids = result_uids;
	*count = row_count;

out:
	if (query_result)
		sqlite3_free_table(query_result);

	if (query)
		sqlite3_free(query);

	if (db) {
		sql_ret = db_util_close(db);
		if (sql_ret != SQLITE_OK)
			NOTIFICATION_WARN("fail to db_util_close - [%d]", sql_ret); /* LCOV_EXCL_LINE */
	}

	return ret;
}

EXPORT_API
int notification_get_dnd_and_allow_to_notify(const char *app_id,
						int *do_not_disturb,
						int *do_not_disturb_except,
						int *allow_to_notify,
						uid_t uid)
{
	int ret = NOTIFICATION_ERROR_NONE;
	int sql_ret;
	int row_count;
	int col_count;
	int col_index;
	sqlite3 *db;
	char *query_setting = NULL;
	char **query_setting_result = NULL;
	char *query_system_setting = NULL;
	char **query_system_setting_result = NULL;

	if (app_id == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	sql_ret = db_util_open(DBPATH, &db, 0);
	if (sql_ret != SQLITE_OK || db == NULL) {
		NOTIFICATION_ERR("db_util_open failed [%s][%d]", DBPATH, sql_ret);
		return NOTIFICATION_ERROR_FROM_DB;
	}

	query_setting = sqlite3_mprintf("SELECT allow_to_notify, do_not_disturb_except " \
					"FROM %s WHERE app_id = %Q AND (uid = %d OR uid = %d) " \
					"ORDER BY uid DESC;",
					NOTIFICATION_SETTING_DB_TABLE, app_id,
					uid, tzplatform_getuid(TZ_SYS_GLOBALAPP_USER));
	if (query_setting == NULL) {
		NOTIFICATION_ERR("fail to alloc query");
		ret = NOTIFICATION_ERROR_OUT_OF_MEMORY;
		goto out;
	}

	query_system_setting = sqlite3_mprintf("SELECT do_not_disturb FROM %s WHERE uid = %d",
					       NOTIFICATION_SYSTEM_SETTING_DB_TABLE, uid);

	if (query_system_setting == NULL) {
		NOTIFICATION_ERR("fail to alloc query");
		ret = NOTIFICATION_ERROR_OUT_OF_MEMORY;
		goto out;
	}

	sql_ret = sqlite3_get_table(db, query_setting, &query_setting_result, &row_count, &col_count, NULL);
	if (sql_ret != SQLITE_OK && sql_ret != -1) {
		NOTIFICATION_ERR("NOTIFICATION_ERROR_FROM_DB failed [%d][%s]", sql_ret, query_setting);
		ret = NOTIFICATION_ERROR_FROM_DB;
		goto out;
	}
	if (row_count == 0) {
		NOTIFICATION_ERR("Invalid uid [%d] or app id [%s]", uid, app_id);
		ret = NOTIFICATION_ERROR_INVALID_PARAMETER;
		goto out;
	}

	col_index = col_count;
	_get_table_field_data_int(query_setting_result, (int *)allow_to_notify, col_index++);
	_get_table_field_data_int(query_setting_result, (int *)do_not_disturb_except, col_index++);

	sql_ret = sqlite3_get_table(db, query_system_setting, &query_system_setting_result, &row_count, &col_count, NULL);
	if (sql_ret != SQLITE_OK && sql_ret != -1) {
		NOTIFICATION_ERR("NOTIFICATION_ERROR_FROM_DB failed [%d][%s]", sql_ret, query_setting);
		ret = NOTIFICATION_ERROR_FROM_DB;
		goto out;
	}
	if (row_count == 0) {
		NOTIFICATION_ERR("Invalid uid [%d]", uid);
		ret = NOTIFICATION_ERROR_INVALID_PARAMETER;
		goto out;
	}

	col_index = col_count;
	_get_table_field_data_int(query_system_setting_result, (int *)do_not_disturb, col_index++);

out:
	if (query_setting_result)
		sqlite3_free_table(query_setting_result);

	if (query_system_setting_result)
		sqlite3_free_table(query_system_setting_result);

	if (query_setting)
		sqlite3_free(query_setting);

	if (query_system_setting)
		sqlite3_free(query_system_setting);

	if (db) {
		sql_ret = db_util_close(db);
		if (sql_ret != SQLITE_OK)
			NOTIFICATION_WARN("fail to db_util_close - [%d]", sql_ret); /* LCOV_EXCL_LINE */
	}

	return ret;
}

EXPORT_API int notification_system_setting_load_dnd_allow_exception(dnd_allow_exception_h *dnd_allow_exception, int *count, uid_t uid)
{
	int ret = NOTIFICATION_ERROR_NONE;
	int sql_ret;
	int row_count = 0;
	int column_count = 0;
	int col_index = 0;
	int i;
	char *query = NULL;
	char **query_result = NULL;
	sqlite3 *db = NULL;
	dnd_allow_exception_h dnd_allow_exception_data = NULL;

	if (dnd_allow_exception == NULL) {
		NOTIFICATION_ERR("NOTIFICATION_ERROR_INVALID_PARAMETER"); /* LCOV_EXCL_LINE */
		return  NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	sql_ret = db_util_open(DBPATH, &db, 0);
	if (sql_ret != SQLITE_OK || db == NULL) {
		NOTIFICATION_ERR("db_util_open failed [%d]", sql_ret); /* LCOV_EXCL_LINE */
		return NOTIFICATION_ERROR_FROM_DB;
	}

	query = sqlite3_mprintf("SELECT type, value FROM %s WHERE uid = %d",
			NOTIFICATION_DND_ALLOW_EXCEPTION, uid);
	if (!query) {
		NOTIFICATION_ERR("fail to alloc query"); /* LCOV_EXCL_LINE */
		ret = NOTIFICATION_ERROR_OUT_OF_MEMORY;
		goto out;
	}

	sql_ret = sqlite3_get_table(db, query, &query_result, &row_count, &column_count, NULL);
	if (sql_ret != SQLITE_OK && sql_ret != -1) {
		NOTIFICATION_ERR("sqlite3_get_table failed [%d][%s]", sql_ret, query); /* LCOV_EXCL_LINE */
		ret = NOTIFICATION_ERROR_FROM_DB;
		goto out;
	}

	if (!row_count) {
		NOTIFICATION_DBG("No setting found..."); /* LCOV_EXCL_LINE */
		goto out;
	} else {
		dnd_allow_exception_data = (dnd_allow_exception_h)malloc(sizeof(struct notification_system_setting_dnd_allow_exception) * row_count);
		if (dnd_allow_exception_data == NULL) {
			NOTIFICATION_ERR("failed to malloc");
			return NOTIFICATION_ERROR_OUT_OF_MEMORY;
		}

		col_index = column_count;

		for (i = 0; i < row_count; i++) {
			_get_table_field_data_int(query_result, (int *)&(dnd_allow_exception_data[i].type), col_index++);
			_get_table_field_data_int(query_result, (int *)&(dnd_allow_exception_data[i].value), col_index++);
		}
	}

	*dnd_allow_exception = dnd_allow_exception_data;
	*count = row_count;

out:
	if (query_result)
		sqlite3_free_table(query_result);

	if (query)
		sqlite3_free(query);

	if (db) {
		sql_ret = db_util_close(db);
		if (sql_ret != SQLITE_OK)
			NOTIFICATION_WARN("fail to db_util_close - [%d]", sql_ret); /* LCOV_EXCL_LINE */
	}

	return ret;
}

EXPORT_API int notification_system_setting_update_dnd_allow_exception(int type, int value, uid_t uid)
{
	int ret = NOTIFICATION_ERROR_NONE;
	int sql_ret;
	int field_index = 1;
	sqlite3 *db = NULL;
	sqlite3_stmt *stmt = NULL;

	sql_ret = db_util_open(DBPATH, &db, 0);
	if (sql_ret != SQLITE_OK || db == NULL) {
		NOTIFICATION_ERR("db_util_open failed [%d]", sql_ret); /* LCOV_EXCL_LINE */
		return NOTIFICATION_ERROR_FROM_DB;
	}

	sqlite3_exec(db, "BEGIN immediate;", NULL, NULL, NULL);

	sql_ret = sqlite3_prepare_v2(db, "INSERT OR REPLACE INTO dnd_allow_exception (uid, type, value) VALUES(?, ?, ?) ", -1, &stmt, NULL);
	if (sql_ret != SQLITE_OK) {
		NOTIFICATION_ERR("sqlite3_prepare_v2 failed [%d][%s]", sql_ret, sqlite3_errmsg(db));
		ret = NOTIFICATION_ERROR_FROM_DB;
		goto out;
	}

	sqlite3_bind_int(stmt, field_index++, uid);
	sqlite3_bind_int(stmt, field_index++, type);
	sqlite3_bind_int(stmt, field_index++, value);

	sql_ret = sqlite3_step(stmt);
	if (sql_ret != SQLITE_OK && sql_ret != SQLITE_DONE) {
		NOTIFICATION_ERR("sqlite3_step failed [%d][%s]", sql_ret, sqlite3_errmsg(db));
		ret = NOTIFICATION_ERROR_FROM_DB;
		goto out;
	}

	sql_ret = sqlite3_changes(db);
	if (sql_ret == 0)
		NOTIFICATION_WARN("No changes on DB");

out:
	if (stmt)
		sqlite3_finalize(stmt);

	if (ret == NOTIFICATION_ERROR_NONE)
		sqlite3_exec(db, "END;", NULL, NULL, NULL);
	else
		sqlite3_exec(db, "ROLLBACK;", NULL, NULL, NULL);

	if (db) {
		sql_ret = db_util_close(db);
		if (sql_ret != SQLITE_OK)
			NOTIFICATION_WARN("fail to db_util_close - [%d]", sql_ret); /* LCOV_EXCL_LINE */
	}

	return ret;
}

EXPORT_API int noti_system_setting_get_do_not_disturb(int *do_not_disturb, uid_t uid)
{
	int ret = NOTIFICATION_ERROR_NONE;
	int sql_ret;
	int row_count = 0;
	int col_count = 0;
	int col_index = 0;
	char *query = NULL;
	char **query_result = NULL;
	sqlite3 *db = NULL;

	sql_ret = db_util_open(DBPATH, &db, 0);
	if (sql_ret != SQLITE_OK || db == NULL) {
		NOTIFICATION_ERR("db_util_open failed [%d]", sql_ret); /* LCOV_EXCL_LINE */
		return NOTIFICATION_ERROR_FROM_DB;
	}

	query = sqlite3_mprintf("SELECT do_not_disturb FROM %s WHERE uid = %d",
				NOTIFICATION_SYSTEM_SETTING_DB_TABLE, uid);
	if (query == NULL) {
		NOTIFICATION_ERR("fail to alloc query");
		ret = NOTIFICATION_ERROR_OUT_OF_MEMORY;
		goto out;
	}

	ret = sqlite3_get_table(db, query, &query_result, &row_count, &col_count, NULL);
	if (ret != SQLITE_OK && ret != -1) {
		NOTIFICATION_ERR("NOTIFICATION_ERROR_FROM_DB failed [%d][%s]", ret, query);
		ret = NOTIFICATION_ERROR_FROM_DB;
		goto out;
	}

	col_index = col_count;
	if (row_count == 0) {
		NOTIFICATION_ERR("No system setting found");
		ret = NOTIFICATION_ERROR_INVALID_PARAMETER;
	} else {
		if (_get_table_field_data_int(query_result, (int *)do_not_disturb, col_index++) == false)
			ret = NOTIFICATION_ERROR_FROM_DB;
	}

out:
	if (query_result)
		sqlite3_free_table(query_result);

	if (query)
		sqlite3_free(query);

	if (db) {
		sql_ret = db_util_close(db);
		if (sql_ret != SQLITE_OK)
			NOTIFICATION_WARN("fail to db_util_close - [%d]", sql_ret); /* LCOV_EXCL_LINE */
	}

	return ret;
}

/* LCOV_EXCL_START */
EXPORT_API int notification_setting_db_update_app_disabled(const char *app_id, bool value, uid_t uid)
{
	int ret = NOTIFICATION_ERROR_NONE;
	int sql_ret;
	sqlite3 *db = NULL;
	char *sqlbuf = NULL;

	if (app_id == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	sql_ret = db_util_open(DBPATH, &db, 0);
	if (sql_ret != SQLITE_OK || db == NULL) {
		NOTIFICATION_ERR("db_util_open failed [%s][%d]", DBPATH, sql_ret);
		return NOTIFICATION_ERROR_FROM_DB;
	}

	sqlbuf = sqlite3_mprintf("UPDATE notification_setting SET " \
			"app_disabled = %d WHERE app_id = %Q AND uid = %d",
			value, app_id, uid);
	if (!sqlbuf) {
		NOTIFICATION_ERR("fail to alloc query");
		ret = NOTIFICATION_ERROR_OUT_OF_MEMORY;
		goto out;
	}

	ret = notification_db_exec(db, sqlbuf, NULL);

out:
	if (sqlbuf)
		sqlite3_free(sqlbuf);

	if (db) {
		sql_ret = db_util_close(db);
		if (sql_ret != SQLITE_OK)
			NOTIFICATION_WARN("fail to db_util_close - [%d]", sql_ret);
	}

	return ret;
}
/* LCOV_EXCL_STOP */
