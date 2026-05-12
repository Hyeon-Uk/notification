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
#include <stdlib.h>
#include <string.h>

#include <notification_debug.h>
#include <notification_group.h>
#include <notification_db.h>
#include <notification_raii.h>

/* LCOV_EXCL_START */
static int _notification_group_check_data_inserted(const char *app_id,
						   int group_id, sqlite3 *db)
{
	char query[NOTIFICATION_QUERY_MAX] = { 0, };
	int result;

	snprintf(query, sizeof(query),
		 "select count(*) from noti_group_data where caller_app_id = '%s' and group_id = %d",
		 app_id, group_id);

	StmtGuard stmt;
	int ret = sqlite3_prepare(db, query, strlen(query),
			stmt.addr(), NULL);
	if (ret != SQLITE_OK) {
		ERR("Get count DB err(%d) : %s", ret, sqlite3_errmsg(db));
		return NOTIFICATION_ERROR_FROM_DB;
	}

	ret = sqlite3_step(stmt.get());
	if (ret == SQLITE_ROW)
		result = sqlite3_column_int(stmt.get(), 0);
	else
		result = 0;

	if (result > 0)
		return NOTIFICATION_ERROR_ALREADY_EXIST_ID;

	INFO("Check Data Inserted appid[%s] group_id[%d] result[%d]",
			  app_id, group_id, result);

	return NOTIFICATION_ERROR_NONE;
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
int notification_group_set_badge(const char *app_id,
						  int group_id, int count)
{
	char query[NOTIFICATION_QUERY_MAX] = { 0, };

	sqlite3 *db_raw = notification_db_open();
	if (!db_raw)
		return get_last_result();
	DbGuard db(db_raw);

	int ret = _notification_group_check_data_inserted(
			app_id, group_id, db.get());

	if (ret == NOTIFICATION_ERROR_NONE) {
		snprintf(query, sizeof(query), "insert into noti_group_data ("
			 "caller_app_id, group_id, badge, content, loc_content) values ("
			 "'%s', %d, %d, '', '')", app_id, group_id, count);
	} else {
		snprintf(query, sizeof(query), "update noti_group_data "
			 "set badge = %d "
			 "where caller_app_id = '%s' and group_id = %d",
			 count, app_id, group_id);
	}

	StmtGuard stmt;
	ret = sqlite3_prepare_v2(db.get(), query, -1,
			stmt.addr(), NULL);
	if (ret != SQLITE_OK) {
		ERR("Failed to insert data app_id[%s] err[%d][%s]",
				 app_id, ret, sqlite3_errmsg(db.get()));
		return NOTIFICATION_ERROR_FROM_DB;
	}

	ret = sqlite3_step(stmt.get());
	if (ret == SQLITE_OK || ret == SQLITE_DONE)
		return NOTIFICATION_ERROR_NONE;

	return NOTIFICATION_ERROR_FROM_DB;
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
int notification_group_get_badge(const char *app_id,
						  int group_id, int *count)
{
	char query[NOTIFICATION_QUERY_MAX] = { 0, };

	sqlite3 *db_raw = notification_db_open();
	if (!db_raw)
		return get_last_result();
	DbGuard db(db_raw);

	if (group_id == NOTIFICATION_GROUP_ID_NONE) {
		int ret = _notification_group_check_data_inserted(
				app_id, group_id, db.get());
		if (ret == NOTIFICATION_ERROR_NONE)
			snprintf(query, sizeof(query),
				 "select sum(badge) "
				 "from noti_group_data "
				 "where caller_app_id = '%s'", app_id);
		else
			snprintf(query, sizeof(query),
				 "select badge "
				 "from noti_group_data "
				 "where caller_app_id = '%s' and group_id = %d",
				 app_id, group_id);
	} else {
		snprintf(query, sizeof(query),
			 "select badge "
			 "from noti_group_data "
			 "where caller_app_id = '%s' and group_id = %d",
			 app_id, group_id);
	}

	INFO("Get badge : query[%s]", query);

	StmtGuard stmt;
	int ret = sqlite3_prepare(db.get(), query, strlen(query),
			stmt.addr(), NULL);
	if (ret != SQLITE_OK) {
		ERR("Failed to insert data app_id[%s] err[%d][%s]",
				app_id, ret, sqlite3_errmsg(db.get()));
		return NOTIFICATION_ERROR_FROM_DB;
	}

	ret = sqlite3_step(stmt.get());
	if (ret == SQLITE_ROW)
		*count = sqlite3_column_int(stmt.get(), 0);

	return NOTIFICATION_ERROR_NONE;
}
/* LCOV_EXCL_STOP */
