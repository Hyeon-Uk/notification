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
#include <notification_raii.h>
#include "notification_db_query.h"

static bool is_db_corrupted = false;

/* LCOV_EXCL_START */
static int __check_integrity_cb(void *pid, int argc, char **argv, char **notUsed)
{
	const char *check_str = "ok";

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
	DbGuard db;
	char *errmsg = NULL;

	sql_ret = sqlite3_open_v2(DBPATH, db.addr(),
			SQLITE_OPEN_CREATE | SQLITE_OPEN_READWRITE, NULL);
	if (sql_ret != SQLITE_OK) {
		ERR("Failed to open db[%d]", sql_ret);
		ret = NOTIFICATION_ERROR_FROM_DB;
		goto out;
	}

	sql_ret = sqlite3_exec(db.get(), CREATE_NOTIFICATION_TABLE,
			NULL, NULL, &errmsg);
	if (sql_ret != SQLITE_OK) {
		ERR("Failed to exec sqlite[%d][%s]", sql_ret, errmsg);
		ret = NOTIFICATION_ERROR_FROM_DB;
		goto out;
	}

	sql_ret = sqlite3_exec(db.get(), "PRAGMA integrity_check",
			__check_integrity_cb, NULL, &errmsg);
	if (sql_ret != SQLITE_OK || is_db_corrupted) {
		ERR("Failed to exec query[%d][%s]", sql_ret, errmsg);
		ret = NOTIFICATION_ERROR_FROM_DB;
	}

out:
	if (sql_ret == SQLITE_CORRUPT || sql_ret == SQLITE_NOTADB
			|| is_db_corrupted)
		ret = __recover_corrupted_db(db.release());
	if (errmsg)
		sqlite3_free(errmsg);

	return ret;
}
/* LCOV_EXCL_STOP */

EXPORT_API sqlite3 *notification_db_open()
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

EXPORT_API int notification_db_close(sqlite3 **db)
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

EXPORT_API int notification_db_exec(sqlite3 *db, const char *query, int *num_changes)
{
	if (db == NULL || query == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	StmtGuard stmt;
	int ret = sqlite3_prepare_v2(db, query, strlen(query),
			stmt.addr(), NULL);
	if (ret != SQLITE_OK) {
		/* LCOV_EXCL_START */
		ERR("Sqlite3 err[%d][%s]", ret, sqlite3_errmsg(db));
		return NOTIFICATION_ERROR_FROM_DB;
		/* LCOV_EXCL_STOP */
	}

	ret = sqlite3_step(stmt.get());
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

	return ret;
}

EXPORT_API char *notification_db_column_text(sqlite3_stmt *stmt, int col)
{
	const unsigned char *col_text = NULL;

	col_text = sqlite3_column_text(stmt, col);
	if (col_text == NULL || col_text[0] == '\0')
		return NULL;

	return strdup((char *)col_text);
}

EXPORT_API bundle *notification_db_column_bundle(sqlite3_stmt *stmt, int col)
{
	const unsigned char *col_bundle = NULL;

	col_bundle = sqlite3_column_text(stmt, col);
	if (col_bundle == NULL || col_bundle[0] == '\0')
		return NULL;

	return bundle_decode(col_bundle, strlen((char *)col_bundle));
}

/* db upgrade sequece*/
static int __check_db_version(sqlite3 *db) {
	int result;

	WARN("@@ __check_db_version");

	StmtGuard stmt;
	int ret = sqlite3_prepare_v2(db, "PRAGMA user_version", -1,
			stmt.addr(), NULL);
	if (ret != SQLITE_OK) {
		ERR("@@ Failed to sqlite3_prepare_v2[%d][%s]",
		ret, sqlite3_errmsg(db));
		return -1;
	}

	ret = sqlite3_step(stmt.get());
	if (ret == SQLITE_ROW)
		result = sqlite3_column_int(stmt.get(), 0);
	else
		result = 0;

	WARN("@@ db version : %d", result);

	if (result != DB_VERSION) {
		WARN("@@ db is not latest version");
		return -1;
	}

	return 0;
}

static int __upgrade_noti_table(sqlite3 *db) {
	int ret;
	int version_num = 0;

	WARN("@@ upgrade noti list");

	ret = sqlite3_table_column_metadata(db, NULL, "noti_list",
		"check_box", NULL, NULL, NULL, NULL, NULL);
	if (ret != SQLITE_OK) {
		WARN("@@ check_box not exist");
		version_num = 2;
		goto upgrade;
	}

	ret = sqlite3_table_column_metadata(db, NULL, "noti_list",
		"channel_name", NULL, NULL, NULL, NULL, NULL);
	if (ret == SQLITE_OK) {
		WARN("@@ columns are exist");
		return NOTIFICATION_ERROR_NONE;
	}
	version_num = 3;

upgrade:
	ret = sqlite3_exec(db, NOTIFICATION_CREATE_UPGRADE_DB,
			NULL, NULL, NULL);
	if (ret != SQLITE_OK) {
		ERR("@@ Failed to create upgrade table[%d][%s]", ret, sqlite3_errmsg(db));
		ret = NOTIFICATION_ERROR_FROM_DB;
		version_num = -1;
	}

	if (version_num == 2) {
		ret = sqlite3_exec(db, NOTIFICATION_UPGRADE_DB_TO_2,
				NULL, NULL, NULL);
		if (ret != SQLITE_OK) {
			ERR("@@ Failed to upgrade noti db to 2[%d][%s]", ret, sqlite3_errmsg(db));
			ret = NOTIFICATION_ERROR_FROM_DB;
		} else {
			ret = NOTIFICATION_ERROR_NONE;
		}
	} else if (version_num == 3) {
		ret = sqlite3_exec(db, NOTIFICATION_UPGRADE_DB_TO_3,
				NULL, NULL, NULL);
		if (ret != SQLITE_OK) {
			ERR("@@ Failed to upgrade noti db to 3[%d][%s]", ret, sqlite3_errmsg(db));
			ret = NOTIFICATION_ERROR_FROM_DB;
		} else {
			ret = NOTIFICATION_ERROR_NONE;
		}
	}
	WARN("@@ upgrade noti list done");

	return ret;
}

static int __upgrade_noti_template_table(sqlite3 *db) {
	int ret;
	int version_num = 0;

	WARN("@@ upgrade noti template");

	ret = sqlite3_table_column_metadata(db, NULL, "noti_template",
		"check_box", NULL, NULL, NULL, NULL, NULL);
	if (ret != SQLITE_OK) {
		WARN("@@ check_box not exist");
		version_num = 2;
		goto upgrade;
	}

	ret = sqlite3_table_column_metadata(db, NULL, "noti_template",
		"channel_name", NULL, NULL, NULL, NULL, NULL);
	if (ret == SQLITE_OK) {
		WARN("@@ columns are exist");
		return NOTIFICATION_ERROR_NONE;
	}
	version_num = 3;

upgrade:
	ret = sqlite3_exec(db, NOTIFICATION_CREATE_UPGRADE_TEMPLATE_DB,
			NULL, NULL, NULL);
	if (ret != SQLITE_OK) {
		ERR("@@ Failed to create tempate table[%d][%s]", ret, sqlite3_errmsg(db));
		ret = NOTIFICATION_ERROR_FROM_DB;
		version_num = -1;
	}

	if (version_num == 2) {
		ret = sqlite3_exec(db, NOTIFICATION_UPGRADE_TEMPLATE_DB_TO_2,
				NULL, NULL, NULL);
		if (ret != SQLITE_OK) {
			ERR("@@ Failed to upgrade tempate db to 2[%d][%s]", ret, sqlite3_errmsg(db));
			ret = NOTIFICATION_ERROR_FROM_DB;
		} else {
			ret = NOTIFICATION_ERROR_NONE;
		}
	} else if (version_num == 3) {
		ret = sqlite3_exec(db, NOTIFICATION_UPGRADE_TEMPLATE_DB_TO_3,
					NULL, NULL, NULL);
		if (ret != SQLITE_OK) {
			ERR("@@ Failed to upgrade tempate db to 2[%d][%s]", ret, sqlite3_errmsg(db));
			ret = NOTIFICATION_ERROR_FROM_DB;
		} else {
			ret = NOTIFICATION_ERROR_NONE;
		}
	}
	WARN("@@ upgrade noti templage done");

	return ret;
}

EXPORT_API int notification_upgrade_db(void)
{
	WARN("@@ upgrade db");

	DbGuard db;
	int sql_ret = sqlite3_open_v2(DBPATH, db.addr(),
			SQLITE_OPEN_READWRITE, NULL);
	if (sql_ret != SQLITE_OK) {
		ERR("@@ Failed to open db");
		return NOTIFICATION_ERROR_FROM_DB;
	}

	DbTransactionGuard txn(db.get());

	if (__check_db_version(db.get()) == 0) {
		WARN("@@ db is latest version");
		txn.commit();
		return NOTIFICATION_ERROR_NONE;
	}

	int ret = __upgrade_noti_table(db.get());
	if (ret != NOTIFICATION_ERROR_NONE) {
		ERR("@@ Failed to upgrade noti table");
		return ret;
	}

	ret = __upgrade_noti_template_table(db.get());
	if (ret != NOTIFICATION_ERROR_NONE) {
		ERR("@@ Failed to upgrade noti template table");
		return ret;
	}

	SqliteCharPtr query(
			sqlite3_mprintf("PRAGMA user_version=%d", DB_VERSION));
	if (!query) {
		ERR("@@ Failed to set verion query");
		return NOTIFICATION_ERROR_OUT_OF_MEMORY;
	}

	char *errmsg = NULL;
	sql_ret = sqlite3_exec(db.get(), query.get(),
			NULL, NULL, &errmsg);
	if (sql_ret != SQLITE_OK) {
		ERR("@@ Failed to ser user version[%d][%s]", sql_ret, errmsg);
		sqlite3_free(errmsg);
		return NOTIFICATION_ERROR_FROM_DB;
	}

	if (!txn.commit()) {
		ERR("@@ Failed to end transaction db[%d][%s]",
				sql_ret, sqlite3_errmsg(db.get()));
		return NOTIFICATION_ERROR_FROM_DB;
	}

	WARN("@@ upgrade db done");
	return NOTIFICATION_ERROR_NONE;
}
