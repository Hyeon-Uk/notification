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

/*
 * N8-A.1 (Stage 8): migrated from C to C++.
 *
 * notification_group.h is missing an `extern "C"` guard (the deprecated
 * notification_group_set_badge / notification_group_get_badge prototypes
 * carry NOTIFICATION_DEPRECATED_API only, no language-linkage block), so
 * the definitions below MUST live inside an explicit `extern "C"` block.
 * Otherwise C++ name mangling would hand out
 * `_Z29notification_group_set_badge...` while every legacy caller (and
 * test_notification_group_internal.cc, which wraps the include in its
 * own `extern "C"`) still expects the un-mangled symbol — yielding an
 * undefined-reference at link time and an ABI break.
 *
 * The file-static helper `_notification_group_check_data_inserted`
 * stays inside the `extern "C"` block too; keeping the same linkage as
 * its callers avoids a forward-decl/definition language-linkage
 * mismatch and makes the definition order independent of the public
 * surface.
 *
 * `<stdio.h>` / `<stdlib.h>` / `<string.h>` are replaced by the C++
 * `<cstdio>` / `<cstdlib>` / `<cstring>` headers.
 */

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <notification_debug.h>
#include <notification_group.h>
#include <notification_db.h>

extern "C" {

/* LCOV_EXCL_START */
static int _notification_group_check_data_inserted(const char *app_id,
						   int group_id, sqlite3 *db)
{
	sqlite3_stmt *stmt = NULL;
	char query[NOTIFICATION_QUERY_MAX] = { 0, };
	int ret;
	int result;

	snprintf(query, sizeof(query),
		 "select count(*) from noti_group_data where caller_app_id = '%s' and group_id = %d",
		 app_id, group_id);

	ret = sqlite3_prepare(db, query, strlen(query), &stmt, NULL);
	if (ret != SQLITE_OK) {
		ERR("Get count DB err(%d) : %s", ret, sqlite3_errmsg(db));
		return NOTIFICATION_ERROR_FROM_DB;
	}

	ret = sqlite3_step(stmt);
	if (ret == SQLITE_ROW)
		result = sqlite3_column_int(stmt, 0);
	else
		result = 0;


	sqlite3_finalize(stmt);

	if (result > 0)
		return NOTIFICATION_ERROR_ALREADY_EXIST_ID;

	INFO("Check Data Inserted appid[%s] group_id[%d] result[%d]",
			  app_id, group_id, result);

	return NOTIFICATION_ERROR_NONE;
}
/* LCOV_EXCL_STOP */

/* N7.5-A: notification_group_{set,get}_badge are declared in the
 * public installed header notification_group.h with the
 * NOTIFICATION_DEPRECATED_API attribute, but that macro does not
 * carry visibility=default — under Phase 2 D2-T4
 * -fvisibility=hidden the symbols ended up STV_HIDDEN and any
 * legacy caller (or unit test) attempting to link the advertised
 * prototype hit "undefined reference". Mark the existing wrappers
 * EXPORT_API to restore the advertised contract without changing
 * behaviour. */
/* LCOV_EXCL_START */
EXPORT_API
int notification_group_set_badge(const char *app_id,
						  int group_id, int count)
{
	sqlite3 *db;
	sqlite3_stmt *stmt = NULL;
	char query[NOTIFICATION_QUERY_MAX] = { 0, };
	int ret;
	int result;

	db = notification_db_open();
	if (!db)
		return get_last_result();

	ret = _notification_group_check_data_inserted(app_id, group_id, db);

	if (ret == NOTIFICATION_ERROR_NONE) {
		/* Insert if does not exist */
		snprintf(query, sizeof(query), "insert into noti_group_data ("
			 "caller_app_id, group_id, badge, content, loc_content) values ("
			 "'%s', %d, %d, '', '')", app_id, group_id, count);

	} else {
		/* Update if exist */
		snprintf(query, sizeof(query), "update noti_group_data "
			 "set badge = %d "
			 "where caller_app_id = '%s' and group_id = %d",
			 count, app_id, group_id);
	}

	ret = sqlite3_prepare_v2(db, query, -1, &stmt, NULL);
	if (ret != SQLITE_OK) {
		ERR("Failed to insert data app_id[%s] err[%d][%s]",
				 app_id, ret, sqlite3_errmsg(db));
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

	sqlite3_finalize(stmt);

	if (db)
		notification_db_close(&db);

	return result;
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
EXPORT_API
int notification_group_get_badge(const char *app_id,
						  int group_id, int *count)
{
	sqlite3 *db;
	sqlite3_stmt *stmt = NULL;
	char query[NOTIFICATION_QUERY_MAX] = { 0, };
	int ret;
	int col = 0;

	db = notification_db_open();
	if (!db)
		return get_last_result();

	if (group_id == NOTIFICATION_GROUP_ID_NONE) {
		ret = _notification_group_check_data_inserted(app_id, group_id, db);
		if (ret == NOTIFICATION_ERROR_NONE)
			/* Get all of app_id count if none group id is not exist */
			snprintf(query, sizeof(query),
				 "select sum(badge) "
				 "from noti_group_data "
				 "where caller_app_id = '%s'", app_id);
		else
			/* Get none group id count */
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

	ret = sqlite3_prepare(db, query, strlen(query), &stmt, NULL);
	if (ret != SQLITE_OK) {
		ERR("Failed to insert data app_id[%s] err[%d][%s]",
				app_id, ret, sqlite3_errmsg(db));
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

}  // extern "C"
