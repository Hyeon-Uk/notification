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

#ifndef __NOTIFICATION_DB_H__
#define __NOTIFICATION_DB_H__

#include <bundle.h>
#include <sqlite3.h>
#include <tzplatform_config.h>

#define DBPATH tzplatform_mkpath(TZ_SYS_DB, ".notification.db")
#define NOTIFICATION_QUERY_MAX 4096
#define NOTIFICATION_EMPTY_STR ""
#define NOTIFICATION_CHECK_STR(p) ((p) ? (p) : NOTIFICATION_EMPTY_STR)

#ifdef __cplusplus
extern "C" {
#endif

sqlite3 *notification_db_open(void);
int notification_db_close(sqlite3 **db);
int notification_db_exec(sqlite3 *db, const char *query, int *num_changes);
char *notification_db_column_text(sqlite3_stmt *stmt, int col);
bundle *notification_db_column_bundle(sqlite3_stmt *stmt, int col);
int notification_db_init(void);

int notification_upgrade_db(void);

#ifdef __cplusplus
}
#endif
#endif /* __NOTIFICATION_DB_H__ */

