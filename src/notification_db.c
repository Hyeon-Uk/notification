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
#include <tzplatform_config.h>

#include <notification_error.h>
#include <notification_debug.h>
#include <notification_db.h>
#include "notification_db_query.h"

EXPORT_API int notification_db_init()
{
	int r;
	sqlite3 *db = NULL;
	char *errmsg = NULL;
	char defname[FILENAME_MAX];
	char *query = NULL;
	const char *db_path = tzplatform_getenv(TZ_SYS_DB);
	if (db_path == NULL) {
		NOTIFICATION_ERR("fail to get db_path"); /* LCOV_EXCL_LINE */
		return NOTIFICATION_ERROR_OUT_OF_MEMORY;
	}
	snprintf(defname, sizeof(defname), "%s/%s", db_path, NOTIFICATION_DB_NAME);

	NOTIFICATION_DBG("db path : %s", defname);
	r = sqlite3_open_v2(defname, &db, SQLITE_OPEN_CREATE | SQLITE_OPEN_READWRITE, NULL);
	if (r) {
		/* LCOV_EXCL_START */
		db_util_close(db);
		NOTIFICATION_ERR("fail to open notification db %d", r);
		return NOTIFICATION_ERROR_IO_ERROR;
		/* LCOV_EXCL_STOP */
	}
	query = sqlite3_mprintf(CREATE_NOTIFICATION_TABLE, tzplatform_getuid(TZ_SYS_DEFAULT_USER));
	NOTIFICATION_DBG("@@@ query : %s", query);

	r = sqlite3_exec(db, CREATE_NOTIFICATION_TABLE, NULL, NULL, &errmsg);
	if (query)
		sqlite3_free(query);
	if (r != SQLITE_OK) {
		/* LCOV_EXCL_START */
		NOTIFICATION_ERR("query error(%d)(%s)", r, errmsg);
		sqlite3_free(errmsg);
		db_util_close(db);
		return NOTIFICATION_ERROR_IO_ERROR;
		/* LCOV_EXCL_STOP */
	}

	db_util_close(db);
	return NOTIFICATION_ERROR_NONE;
}

sqlite3 *notification_db_open(const char *dbfile)
{
	int ret = 0;
	sqlite3 *db = 0;

	ret = db_util_open(dbfile, &db, 0);
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

	ret = db_util_close(*db);
	if (ret != SQLITE_OK) {
		/* LCOV_EXCL_START */
		NOTIFICATION_ERR("DB close error(%d)", ret);
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
		NOTIFICATION_ERR("DB err(%d) : %s", ret, sqlite3_errmsg(db));
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
		NOTIFICATION_ERR("DB err(%d) : %s", ret,
				 sqlite3_errmsg(db));
		ret = NOTIFICATION_ERROR_FROM_DB;
		/* LCOV_EXCL_STOP */
	}

	if (stmt)
		sqlite3_finalize(stmt);

	return ret;
}

char *notification_db_column_text(sqlite3_stmt * stmt, int col)
{
	const unsigned char *col_text = NULL;

	col_text = sqlite3_column_text(stmt, col);
	if (col_text == NULL || col_text[0] == '\0')
		return NULL;

	return strdup((char *)col_text);
}

bundle *notification_db_column_bundle(sqlite3_stmt * stmt, int col)
{
	const unsigned char *col_bundle = NULL;

	col_bundle = sqlite3_column_text(stmt, col);
	if (col_bundle == NULL || col_bundle[0] == '\0')
		return NULL;

	return bundle_decode(col_bundle, strlen((char *)col_bundle));
}

