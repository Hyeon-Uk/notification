/*
 * Copyright (c) 2000 - 2016 Samsung Electronics Co., Ltd. All rights reserved.
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
#include <stdlib.h>
#include <string.h>

#include <notification_debug.h>
#include <notification_group.h>
#include <notification_db.h>

/* LCOV_EXCL_START */
static int _notification_group_check_data_inserted(const char *pkgname,
						   int group_id, sqlite3 *db)
{
	sqlite3_stmt *stmt = NULL;
	char query[NOTIFICATION_QUERY_MAX] = { 0, };
	int ret;
	int result;

	snprintf(query, sizeof(query),
		 "select count(*) from noti_group_data where caller_pkgname = '%s' and group_id = %d",
		 pkgname, group_id);

	ret = sqlite3_prepare(db, query, strlen(query), &stmt, NULL);
	if (ret != SQLITE_OK) {
		NOTIFICATION_ERR("Get count DB err(%d) : %s", ret,
				 sqlite3_errmsg(db));
		return NOTIFICATION_ERROR_FROM_DB;
	}

	ret = sqlite3_step(stmt);
	if (ret == SQLITE_ROW)
		result = sqlite3_column_int(stmt, 0);
	else
		result = 0;


	NOTIFICATION_INFO("Check Data Inserted : query[%s], result : [%d]",
			  query, result);

	sqlite3_finalize(stmt);

	if (result > 0)
		return NOTIFICATION_ERROR_ALREADY_EXIST_ID;

	return NOTIFICATION_ERROR_NONE;
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
int notification_group_set_badge(const char *pkgname,
						  int group_id, int count)
{
	sqlite3 *db;
	sqlite3_stmt *stmt = NULL;
	char query[NOTIFICATION_QUERY_MAX] = { 0, };
	int ret;
	int result;

	db = notification_db_open(DBPATH);
	if (!db)
		return get_last_result();

	ret = _notification_group_check_data_inserted(pkgname, group_id, db);

	if (ret == NOTIFICATION_ERROR_NONE) {
		/* Insert if does not exist */
		snprintf(query, sizeof(query), "insert into noti_group_data ("
			 "caller_pkgname, group_id, badge, content, loc_content) values ("
			 "'%s', %d, %d, '', '')", pkgname, group_id, count);

	} else {
		/* Update if exist */
		snprintf(query, sizeof(query), "update noti_group_data "
			 "set badge = %d "
			 "where caller_pkgname = '%s' and group_id = %d",
			 count, pkgname, group_id);
	}

	ret = sqlite3_prepare_v2(db, query, -1, &stmt, NULL);
	if (ret != SQLITE_OK) {
		NOTIFICATION_ERR("Insert Query : %s", query);
		NOTIFICATION_ERR("Insert DB error(%d) : %s", ret,
				 sqlite3_errmsg(db));
		if (stmt)
			sqlite3_finalize(stmt);

		if (db)
			notification_db_close(&db);

		return NOTIFICATION_ERROR_FROM_DB;
	}

	ret = sqlite3_step(stmt);
	if (ret == SQLITE_OK || ret == SQLITE_DONE)
		result = NOTIFICATION_ERROR_NONE;
	else
		result = NOTIFICATION_ERROR_FROM_DB;

	if (stmt)
		sqlite3_finalize(stmt);

	if (db)
		notification_db_close(&db);

	return result;
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
int notification_group_get_badge(const char *pkgname,
						  int group_id, int *count)
{
	sqlite3 *db;
	sqlite3_stmt *stmt = NULL;
	char query[NOTIFICATION_QUERY_MAX] = { 0, };
	int ret;
	int col = 0;

	db = notification_db_open(DBPATH);
	if (!db)
		return get_last_result();

	if (group_id == NOTIFICATION_GROUP_ID_NONE) {
		ret = _notification_group_check_data_inserted(pkgname, group_id, db);
		if (ret == NOTIFICATION_ERROR_NONE)
			/* Get all of pkgname count if none group id is not exist */
			snprintf(query, sizeof(query),
				 "select sum(badge) "
				 "from noti_group_data "
				 "where caller_pkgname = '%s'", pkgname);
		else
			/* Get none group id count */
			snprintf(query, sizeof(query),
				 "select badge "
				 "from noti_group_data "
				 "where caller_pkgname = '%s' and group_id = %d",
				 pkgname, group_id);
	} else {
		snprintf(query, sizeof(query),
			 "select badge "
			 "from noti_group_data "
			 "where caller_pkgname = '%s' and group_id = %d",
			 pkgname, group_id);
	}

	NOTIFICATION_INFO("Get badge : query[%s]", query);

	ret = sqlite3_prepare(db, query, strlen(query), &stmt, NULL);
	if (ret != SQLITE_OK) {
		NOTIFICATION_ERR("Select Query : %s", query);
		NOTIFICATION_ERR("Select DB error(%d) : %s", ret,
				 sqlite3_errmsg(db));
		if (db)
			notification_db_close(&db);

		return NOTIFICATION_ERROR_FROM_DB;
	}

	ret = sqlite3_step(stmt);
	if (ret == SQLITE_ROW)
		*count = sqlite3_column_int(stmt, col++);

	sqlite3_finalize(stmt);

	if (db)
		notification_db_close(&db);

	return NOTIFICATION_ERROR_NONE;
}
/* LCOV_EXCL_STOP */
