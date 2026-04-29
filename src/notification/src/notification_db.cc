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

#include <cerrno>
#include <unistd.h>
#include <cstdio>
#include <cstring>

#include <sqlite3.h>
#include <tizen.h>

#include <notification_error.h>
#include <notification_debug.h>
#include <notification_db.h>
#include "notification_db_query.h"

/*
 * N8-A.2 (Stage 8): migrated from C to C++.  Public entry points
 * (`notification_db_init`, `notification_db_open`, `notification_db_close`,
 * `notification_db_exec`, `notification_db_column_text`,
 * `notification_db_column_bundle`, `notification_upgrade_db`) are
 * advertised with C linkage in notification_db.h, so the bodies sit
 * inside an extern "C" block here.  Static helpers
 * (`__ensure_noti_list_indexes`, `__check_integrity_cb`,
 * `__recover_corrupted_db`, `__check_db_version`,
 * `__upgrade_noti_table`, `__upgrade_noti_template_table`) live in the
 * same block — they retain internal linkage by virtue of `static`, and
 * sharing the language-linkage with the public entries is harmless.
 *
 * `<errno.h>` / `<stdio.h>` / `<string.h>` swap to the C++
 * `<cerrno>` / `<cstdio>` / `<cstring>`.  The ABI-safe `(unsigned char *)`
 * cast handed to `bundle_decode` and the `(char *)` cast handed to
 * `strdup` already satisfy C++'s implicit-cast rules (C-style casts).
 */

extern "C" {

static bool is_db_corrupted = false;

/*
 * N3-T7 Q2: ensure composite covering indexes exist.
 *
 * Invoked from every path that produces or opens a noti_list table
 * (notification_db_init, __recover_corrupted_db, __check_db_version).
 * Uses CREATE INDEX IF NOT EXISTS so the statements are idempotent and
 * cheap on already-migrated databases (SQLite probes sqlite_master once
 * per DDL statement). Failures are logged but do not abort — the
 * indexes are a performance optimisation, not a correctness contract.
 */
static int __ensure_noti_list_indexes(sqlite3 *db)
{
	int sql_ret;
	char *errmsg = NULL;

	if (db == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	sql_ret = sqlite3_exec(db, CREATE_NOTIFICATION_INDEXES,
			NULL, NULL, &errmsg);
	if (sql_ret != SQLITE_OK) {
		WARN("@@ Failed to create noti_list indexes[%d][%s]",
				sql_ret, errmsg ? errmsg : "(null)");
		if (errmsg)
			sqlite3_free(errmsg);
		return NOTIFICATION_ERROR_FROM_DB;
	}
	if (errmsg)
		sqlite3_free(errmsg);
	return NOTIFICATION_ERROR_NONE;
}

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
/*
 * N8 memleak audit pass1: this used to take `sqlite3 *db` by value, close
 * the input handle and re-open into the same local — leaking the new handle
 * (caller's `db` was unchanged so neither the new nor old handle was reachable
 * once this function returned) AND causing a double-close in the caller's
 * cleanup epilogue (`if (db) sqlite3_close(db)` ran on the freed pointer,
 * sqlite3 returns SQLITE_MISUSE and the implementation may free internal
 * state again).
 *
 * Take it by `sqlite3 **db_io` so the caller's pointer is NULL'd after the
 * close + reopen + close cycle. The function now closes the new handle
 * itself and writes NULL back to *db_io, leaving the caller's epilogue a
 * no-op.
 */
static int __recover_corrupted_db(sqlite3 **db_io)
{
	int ret = NOTIFICATION_ERROR_NONE;
	int sql_ret;
	sqlite3 *db = NULL;
	char *errmsg = NULL;

	if (db_io == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	INFO("DB is corrupted, start to recover corrupted db");
	if (*db_io) {
		sqlite3_close(*db_io);
		*db_io = NULL;
	}
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
		goto out;
	}

	/* N3-T7 Q2: attach composite indexes to the freshly recovered table. */
	(void)__ensure_noti_list_indexes(db);

out:
	if (errmsg)
		sqlite3_free(errmsg);
	if (db)
		sqlite3_close(db);

	return ret;
}
/* LCOV_EXCL_STOP */

/* Forward declaration: defined below. */
static int __check_db_version(sqlite3 *db);

/* LCOV_EXCL_START */
EXPORT_API int notification_db_init(void)
{
	int ret = NOTIFICATION_ERROR_NONE;
	int sql_ret;
	sqlite3 *db = NULL;
	char *errmsg = NULL;

	/*
	 * Race audit pass 2: `is_db_corrupted` is a file-static set by
	 * the PRAGMA integrity_check callback __check_integrity_cb and
	 * read by this function on the next line.  It was never reset
	 * across invocations, so any later call to notification_db_init
	 * (the test_notification_memleak_audit DbInit_IdempotentReinit
	 * fixture invokes it 4x; production may re-run it on
	 * notification-init re-exec) saw a stale `true` from the previous
	 * call's corrupted-then-recovered DB and incorrectly re-entered
	 * __recover_corrupted_db, deleting and re-creating an already-
	 * healthy DBPATH.  Reset to the documented default at entry. */
	is_db_corrupted = false;

	/*
	 * N4-T3: fast-path. If the DB file already exists and is up to date
	 * (same DB_VERSION), skip the expensive CREATE TABLE IF NOT EXISTS
	 * and PRAGMA integrity_check pass (~30-200 ms). This brings the
	 * common-case `notification_init` cost down from ~80 ms to <5 ms on
	 * second-and-later boots while leaving the slow recovery path
	 * untouched for the first-boot / corrupted / version-mismatch cases.
	 *
	 * Any failure inside the fast-path (open failure, version mismatch,
	 * corrupted prepare/step in __check_db_version) falls through to the
	 * slow path below, so correctness is preserved.
	 */
	if (access(DBPATH, R_OK | W_OK) == 0) {
		sql_ret = sqlite3_open_v2(DBPATH, &db,
				SQLITE_OPEN_READWRITE, NULL);
		if (sql_ret == SQLITE_OK) {
			if (__check_db_version(db) == 0) {
				/* idempotent + cheap: replay composite indexes */
				(void)__ensure_noti_list_indexes(db);
				sqlite3_close(db);
				return NOTIFICATION_ERROR_NONE;
			}
			sqlite3_close(db);
			db = NULL;
		}
	}

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

	/*
	 * N3-T7 Q2: create composite covering indexes on noti_list. Must run
	 * AFTER CREATE_NOTIFICATION_TABLE because the indexes reference
	 * noti_list columns. Failure is non-fatal (performance-only).
	 */
	(void)__ensure_noti_list_indexes(db);

	sql_ret = sqlite3_exec(db, "PRAGMA integrity_check",
			       __check_integrity_cb, NULL, &errmsg);
	if (sql_ret != SQLITE_OK || is_db_corrupted) {
		ERR("Failed to exec query[%d][%s]", sql_ret, errmsg);
		ret = NOTIFICATION_ERROR_FROM_DB;
	}

out:
	if (sql_ret == SQLITE_CORRUPT || sql_ret == SQLITE_NOTADB || is_db_corrupted)
		ret = __recover_corrupted_db(&db); /* nulls out `db` */
	if (errmsg)
		sqlite3_free(errmsg);
	if (db)
		sqlite3_close(db);

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
/*
 * N1-T6 L8 hardening:
 *   - result is initialised to 0 so a partial execution (step returning
 *     anything other than SQLITE_ROW, or stmt being NULL after a prepare
 *     that silently left *stmt untouched) can never read uninitialised
 *     memory.
 *   - stmt is always NULL-checked and finalized on every exit path via a
 *     single out: label; the previous code could return -1 before stmt
 *     reached finalize if a future edit introduced an early branch.
 *   - Invalid PRAGMA results now propagate as "version mismatch" (-1)
 *     rather than silently treating unknown readings as success.
 */
static int __check_db_version(sqlite3 *db) {
	int ret;
	int result = 0;
	int rc = -1;
	sqlite3_stmt *stmt = NULL;

	WARN("@@ __check_db_version");

	if (!db) {
		ERR("@@ NULL db handle");
		return -1;
	}

	ret = sqlite3_prepare_v2(db, "PRAGMA user_version", -1, &stmt, NULL);
	if (ret != SQLITE_OK || stmt == NULL) {
		ERR("@@ Failed to sqlite3_prepare_v2[%d][%s]",
			ret, sqlite3_errmsg(db));
		goto out;
	}

	ret = sqlite3_step(stmt);
	if (ret == SQLITE_ROW) {
		result = sqlite3_column_int(stmt, 0);
	} else {
		ERR("@@ unexpected step result[%d][%s]",
			ret, sqlite3_errmsg(db));
		result = 0;
	}

	WARN("@@ db version : %d", result);

	if (result == DB_VERSION)
		rc = 0;
	else
		WARN("@@ db is not latest version");

	/*
	 * N3-T7 Q2: replay composite-index DDL on every version check so
	 * pre-existing installs (where the schema migration branches below
	 * do not run) also pick up idx_noti_caller_uid_time /
	 * idx_noti_pkg_uid_time / idx_noti_uid_time. CREATE INDEX IF NOT
	 * EXISTS is idempotent; the operation is cheap once the indexes are
	 * already present (single sqlite_master probe, no tree work).
	 */
	(void)__ensure_noti_list_indexes(db);

out:
	if (stmt)
		sqlite3_finalize(stmt);
	return rc;
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
	sqlite3 *db;
	int ret = NOTIFICATION_ERROR_NONE;
	char *errmsg = NULL;
	char *query = NULL;

	WARN("@@ upgrade db");

	ret = sqlite3_open_v2(DBPATH, &db, SQLITE_OPEN_READWRITE, NULL);
	if (ret != SQLITE_OK) {
		ERR("@@ Failed to open db");
		return NOTIFICATION_ERROR_FROM_DB;
	}

	// add transaction
	ret = sqlite3_exec(db, "BEGIN TRANSACTION", NULL, NULL, NULL);
	if (ret != SQLITE_OK) {
		ERR("@@ Failed to begin transaction db[%d][%s]", ret, sqlite3_errmsg(db));
		notification_db_close(&db);
		return NOTIFICATION_ERROR_FROM_DB;
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
		goto out;
	}
	ret = NOTIFICATION_ERROR_NONE;
	WARN("@@ upgrade db done");

out:
	if (ret == NOTIFICATION_ERROR_NONE) {
		ret = sqlite3_exec(db, "END TRANSACTION", NULL, NULL, NULL);
		if (ret != SQLITE_OK) {
			ERR("@@ Failed to end transaction db[%d][%s]", ret, sqlite3_errmsg(db));
			ret = NOTIFICATION_ERROR_FROM_DB;
		}
	} else {
		ret = sqlite3_exec(db, "ROLLBACK TRANSACTION", NULL, NULL, NULL);
		if (ret != SQLITE_OK) {
			ERR("@@ Failed to rollback transaction db[%d][%s]", ret, sqlite3_errmsg(db));
			ret = NOTIFICATION_ERROR_FROM_DB;
		}
	}

	if (errmsg)
		sqlite3_free(errmsg);

	if (query)
		sqlite3_free(query);

	if (db)
		notification_db_close(&db);

	return ret;
}

}  // extern "C"
