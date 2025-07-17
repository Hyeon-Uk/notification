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
#include <tizen.h>

#include <notification_error.h>
#include <notification_debug.h>
#include <notification_db.h>
#include "notification_db_query.h"

static bool is_db_corrupted = false;

/* LCOV_EXCL_START */
static int __check_integrity_cb(void *pid, int argc, char **argv, char **notUsed)
{
	char *check_str = "ok";

	if (strncmp(argv[0], check_str, strlen(check_str))) {
		ERR("db integrity result : %s", argv[0]);
		is_db_corrupted = true;
		return -1;
	}

	INFO("db integrity result : %s", argv[0]);
	return 0;
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
static int __recover_corrupted_db(sqlite3 *db)
{
	int ret = NOTIFICATION_ERROR_NONE;
	int sql_ret;
	char *errmsg = NULL;

	INFO("DB is corrupted, start to recover corrupted db");
	if (db)
		sqlite3_close(db);
	unlink(DBPATH);

	sql_ret = sqlite3_open_v2(DBPATH, &db,
				SQLITE_OPEN_CREATE | SQLITE_OPEN_READWRITE,
				NULL);
	if (sql_ret != SQLITE_OK) {
		ERR("Failed to open db[%d]", sql_ret);
		unlink(DBPATH);
		ret = NOTIFICATION_ERROR_FROM_DB;
		goto out;
	}

	sql_ret = sqlite3_exec(db, CREATE_NOTIFICATION_TABLE, NULL, NULL, &errmsg);
	if (sql_ret != SQLITE_OK) {
		ERR("Failed to exec query[%d][%s]", sql_ret, errmsg);
		ret = NOTIFICATION_ERROR_FROM_DB;
	}

out:
	if (errmsg)
		sqlite3_free(errmsg);

	return ret;
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
EXPORT_API int notification_db_init(void)
{
	int ret = NOTIFICATION_ERROR_NONE;
	int sql_ret;
	sqlite3 *db = NULL;
	char *errmsg = NULL;

	sql_ret = sqlite3_open_v2(DBPATH, &db,
			SQLITE_OPEN_CREATE | SQLITE_OPEN_READWRITE, NULL);
	if (sql_ret != SQLITE_OK) {
		ERR("Failed to open db[%d]", sql_ret);
		ret = NOTIFICATION_ERROR_FROM_DB;
		goto out;
	}

	sql_ret = sqlite3_exec(db, CREATE_NOTIFICATION_TABLE, NULL, NULL, &errmsg);
	if (sql_ret != SQLITE_OK) {
		ERR("Failed to exec sqlite[%d][%s]", sql_ret, errmsg);
		ret = NOTIFICATION_ERROR_FROM_DB;
		goto out;
	}

	sql_ret = sqlite3_exec(db, "PRAGMA integrity_check",
			       __check_integrity_cb, NULL, &errmsg);
	if (sql_ret != SQLITE_OK || is_db_corrupted) {
		ERR("Failed to exec query[%d][%s]", sql_ret, errmsg);
		ret = NOTIFICATION_ERROR_FROM_DB;
	}

out:
	if (sql_ret == SQLITE_CORRUPT || sql_ret == SQLITE_NOTADB || is_db_corrupted)
		ret = __recover_corrupted_db(db);
	if (errmsg)
		sqlite3_free(errmsg);
	if (db)
		sqlite3_close(db);

	return ret;
}
/* LCOV_EXCL_STOP */

sqlite3 *notification_db_open()
{
	int ret;
	sqlite3 *db;

	ret = access(DBPATH, R_OK | W_OK);
	if (ret != 0) {
		/* LCOV_EXCL_START */
		set_last_result(NOTIFICATION_ERROR_FROM_DB);
		return NULL;
		/* LCOV_EXCL_STOP */
	}

	ret = sqlite3_open_v2(DBPATH, &db, SQLITE_OPEN_READWRITE, NULL);
	if (ret != SQLITE_OK) {
		/* LCOV_EXCL_START */
		if (ret == SQLITE_PERM)
			set_last_result(NOTIFICATION_ERROR_PERMISSION_DENIED);
		else
			set_last_result(NOTIFICATION_ERROR_FROM_DB);

		return NULL;
		/* LCOV_EXCL_STOP */
	}

	return db;
}

int notification_db_close(sqlite3 **db)
{
	int ret = 0;

	if (db == NULL || *db == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	ret = sqlite3_close(*db);
	if (ret != SQLITE_OK) {
		/* LCOV_EXCL_START */
		ERR("Failed to close db[%d]", ret);
		return NOTIFICATION_ERROR_FROM_DB;
		/* LCOV_EXCL_STOP */
	}

	*db = NULL;

	return NOTIFICATION_ERROR_NONE;
}

int notification_db_exec(sqlite3 *db, const char *query, int *num_changes)
{
	int ret = NOTIFICATION_ERROR_NONE;
	sqlite3_stmt *stmt = NULL;

	if (db == NULL || query == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	ret = sqlite3_prepare_v2(db, query, strlen(query), &stmt, NULL);
	if (ret != SQLITE_OK) {
		/* LCOV_EXCL_START */
		ERR("Sqlite3 err[%d][%s]", ret, sqlite3_errmsg(db));
		return NOTIFICATION_ERROR_FROM_DB;
		/* LCOV_EXCL_STOP */
	}

	ret = sqlite3_step(stmt);
	if (ret == SQLITE_OK || ret == SQLITE_DONE) {
		if (num_changes != NULL)
			*num_changes = sqlite3_changes(db);

		ret = NOTIFICATION_ERROR_NONE;
	} else {
		/* LCOV_EXCL_START */
		ERR("Sqlite err[%d][%s]", ret, sqlite3_errmsg(db));
		ret = NOTIFICATION_ERROR_FROM_DB;
		/* LCOV_EXCL_STOP */
	}

	sqlite3_finalize(stmt);

	return ret;
}

char *notification_db_column_text(sqlite3_stmt *stmt, int col)
{
	const unsigned char *col_text = NULL;

	col_text = sqlite3_column_text(stmt, col);
	if (col_text == NULL || col_text[0] == '\0')
		return NULL;

	return strdup((char *)col_text);
}

bundle *notification_db_column_bundle(sqlite3_stmt *stmt, int col)
{
	const unsigned char *col_bundle = NULL;

	col_bundle = sqlite3_column_text(stmt, col);
	if (col_bundle == NULL || col_bundle[0] == '\0')
		return NULL;

	return bundle_decode(col_bundle, strlen((char *)col_bundle));
}

/* db upgrade sequece*/
static int __check_db_version(sqlite3 *db) {
	int ret;
	int result;
	sqlite3_stmt *stmt = NULL;

	WARN("@@ __check_db_version");

	ret = sqlite3_prepare_v2(db, "PRAGMA user_version", -1, &stmt, NULL);
	if (ret != SQLITE_OK) {
		ERR("@@ Failed to sqlite3_prepare_v2[%d][%s]",
		ret, sqlite3_errmsg(db));
		return -1;
	}

	ret = sqlite3_step(stmt);
	if (ret == SQLITE_ROW)
		result = sqlite3_column_int(stmt, 0);
	else
		result = 0;

	WARN("@@ db version : %d", result);

	if (stmt)
		sqlite3_finalize(stmt);

	if (result != DB_VERSION) {
		WARN("@@ db is not latest version");
		return -1;
	}

	return 0;
}

static int __upgrade_noti_table(sqlite3 *db) {
	int ret;
	char *errmsg = NULL;

	WARN("@@ upgrade noti list");

	ret = sqlite3_table_column_metadata(db, NULL, "noti_list",
		"b_event_handler_click_on_button_7", NULL, NULL, NULL, NULL, NULL);
	if (ret != SQLITE_OK) {
		WARN("@@ btn column not exist");
		goto upgrade;
	}

	ret = sqlite3_table_column_metadata(db, NULL, "noti_list",
		"check_box", NULL, NULL, NULL, NULL, NULL);
	if (ret == SQLITE_OK) {
		WARN("@@ columns are exist");
		return NOTIFICATION_ERROR_NONE;
	}

upgrade:
	ret = sqlite3_exec(db, NOTIFICATION_UPGRADE_DB, NULL, NULL, &errmsg);
	if (ret != SQLITE_OK) {
		ERR("@@ Failed to upgrade noti db[%d][%s]", ret, errmsg);
		ret = NOTIFICATION_ERROR_FROM_DB;
	} else {
		ret = NOTIFICATION_ERROR_NONE;
	}

	if (errmsg)
		sqlite3_free(errmsg);

	WARN("@@ upgrade noti list done");

	return ret;
}

static int __upgrade_noti_template_table(sqlite3 *db) {
	int ret;
	char *errmsg = NULL;

	WARN("@@ upgrade noti templage");

	ret = sqlite3_table_column_metadata(db, NULL, "noti_template",
		"b_event_handler_click_on_button_7", NULL, NULL, NULL, NULL, NULL);
	if (ret != SQLITE_OK) {
		WARN("@@ btn column not exist");
		goto upgrade;
	}

	ret = sqlite3_table_column_metadata(db, NULL, "noti_template",
		"check_box", NULL, NULL, NULL, NULL, NULL);
	if (ret == SQLITE_OK) {
		WARN("@@ columns are exist");
		return NOTIFICATION_ERROR_NONE;
	}

upgrade:
	ret = sqlite3_exec(db, NOTIFICATION_UPGRADE_TEMPLATE_DB, NULL, NULL, &errmsg);
	if (ret != SQLITE_OK) {
		ERR("@@ Failed to upgrade noti db[%d][%s]", ret, errmsg);
		ret = NOTIFICATION_ERROR_FROM_DB;
	} else {
		ret = NOTIFICATION_ERROR_NONE;
	}

	if (errmsg)
		sqlite3_free(errmsg);

	WARN("@@ upgrade noti templage done");

	return ret;
}

EXPORT_API int notification_upgrade_db(void)
{
	sqlite3 *db;
	int ret = NOTIFICATION_ERROR_NONE;
	char *errmsg = NULL;
	char *query = NULL;

	WARN("@@ upgrade db");

	ret = sqlite3_open_v2(DBPATH, &db, SQLITE_OPEN_READWRITE, NULL);
	if (ret != SQLITE_OK) {
		ERR("@@ Failed to open db");
		ret = NOTIFICATION_ERROR_FROM_DB;
		goto out;
	}

	// 1. check version
	if (__check_db_version(db) == 0) {
		WARN("@@ db is latest version");
		ret = NOTIFICATION_ERROR_NONE;
		goto out;
	}

	// 2. run upgrade query
	ret = __upgrade_noti_table(db);
	if (ret != NOTIFICATION_ERROR_NONE) {
		ERR("@@ Failed to upgrade noti table");
		goto out;
	}

	ret = __upgrade_noti_template_table(db);
	if (ret != NOTIFICATION_ERROR_NONE) {
		ERR("@@ Failed to upgrade noti template table");
		goto out;
	}

	// 3. set new version
	query = sqlite3_mprintf("PRAGMA user_version=%d", DB_VERSION);
	if (query == NULL) {
		ERR("@@ Failed to set verion query");
		ret = NOTIFICATION_ERROR_OUT_OF_MEMORY;
		goto out;
	}

	ret = sqlite3_exec(db, query, NULL, NULL, &errmsg);
	if (ret != SQLITE_OK) {
		ERR("@@ Failed to ser user version[%d][%s]", ret, errmsg);
		ret = NOTIFICATION_ERROR_FROM_DB;
	}
	WARN("@@ upgrade db done");

out:
	if (errmsg)
		sqlite3_free(errmsg);

	if (query)
		sqlite3_free(query);

	if (db)
		notification_db_close(&db);

	return NOTIFICATION_ERROR_NONE;
}
