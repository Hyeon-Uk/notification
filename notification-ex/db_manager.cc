/*
 * Copyright (c) 2019 Samsung Electronics Co., Ltd.
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

#include <dlog.h>
#include <unistd.h>
#include <sqlite3.h>
#include <vconf.h>
#include <pkgmgr-info.h>
#include <tzplatform_config.h>
#include <string.h>
#include <list>
#include <map>

#include "notification-ex/common.h"
#include "notification-ex/db_manager.h"
#include "notification-ex/item_inflator.h"
#include "notification-ex/iitem_info_internal.h"
#include "notification-ex/ex_util.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "NOTIFICATION_EX"

#define CREATE_NOTIFICATION_TABLE \
  "PRAGMA journal_mode = PERSIST;\n" \
  "PRAGMA synchronous = FULL;\n" \
  "PRAGMA foreign_keys = ON;\n" \
  "CREATE TABLE IF NOT EXISTS noti_ex_list (\n" \
  " root_id TEXT NOT NULL,\n" \
  " app_id TEXT NOT NULL,\n" \
  " uid INTEGER,\n" \
  " priv_id INTEGER,\n" \
  " pkg_id TEXT,\n" \
  " policy INTEGER,\n" \
  " data TEXT NOT NULL,\n" \
  " insert_time INTEGER,\n" \
  " hide_list TEXT,\n" \
  " PRIMARY KEY (root_id, app_id, uid));\n" \
  "CREATE TABLE IF NOT EXISTS receiver_list (\n" \
  " root_id TEXT NOT NULL,\n" \
  " app_id TEXT NOT NULL,\n" \
  " uid INTEGER,\n" \
  " receiver_group TEXT NOT NULL,\n" \
  " FOREIGN KEY (root_id, app_id, uid)\n" \
  " REFERENCES noti_ex_list(root_id, app_id, uid) ON DELETE CASCADE);\n"

#define DBPATH tzplatform_mkpath(TZ_SYS_DB, ".notification.db")
#define NOTI_LIMIT 100

using namespace std;
using namespace notification::item;

namespace notification {

DBManager::DBManager() = default;
DBManager::~DBManager() = default;

sqlite3* DBManager::OpenDB() {
  int ret;
  sqlite3* db;

  ret = sqlite3_open_v2(DBPATH, &db, SQLITE_OPEN_READWRITE, nullptr);
  if (ret != SQLITE_OK) {
    LOGE("open db(%s) error: %d", DBPATH, ret);
    return nullptr;
  }

  return db;
}

void DBManager::CloseDB(sqlite3* db) {
  int ret = SQLITE_OK;

  if (db)
    ret = sqlite3_close_v2(db);

  if (ret != SQLITE_OK)
    LOGE("close db error");
}

int DBManager::ExecuteQuery(sqlite3* db, const char* query, int* num_changes) {
  int ret = ERROR_NONE;
  sqlite3_stmt *stmt;

  if (db == nullptr || query == nullptr)
    return ERROR_INVALID_PARAMETER;

  ret = sqlite3_prepare_v2(db, query, strlen(query), &stmt, nullptr);
  if (ret != SQLITE_OK) {
    /* LCOV_EXCL_START */
    LOGE("Sqlite3 err[%d][%s]", ret, sqlite3_errmsg(db));
    return ERROR_FROM_DB;
    /* LCOV_EXCL_STOP */
  }

  ret = sqlite3_step(stmt);
  if (ret == SQLITE_OK || ret == SQLITE_DONE) {
    if (num_changes != nullptr)
    *num_changes = sqlite3_changes(db);
    ret = ERROR_NONE;
  } else {
    /* LCOV_EXCL_START */
    LOGE("Sqlite err[%d][%s]", ret, sqlite3_errmsg(db));
    ret = ERROR_FROM_DB;
    /* LCOV_EXCL_STOP */
  }

  sqlite3_finalize(stmt);
  return ret;
}

int DBManager::ExecuteQuery(const char* query, int* num_changes) {
  int ret;

  if (query == nullptr)
    return ERROR_INVALID_PARAMETER;

  sqlite3* db = OpenDB();
  if (db == nullptr)
    return ERROR_FROM_DB;

  ret = ExecuteQuery(db, query, num_changes);
  CloseDB(db);

  return ret;
}

int DBManager::CheckDBIntegrity(void* user_data, int argc,
                                       char** argv, char** notUsed) {
  bool* is_db_corrupted = static_cast<bool*>(user_data);

  if (std::string(argv[0]).compare("ok")) {
    LOGE("db integrity result : %s", argv[0]);
    *is_db_corrupted = true;
    return -1;
  }

  LOGI("db integrity result : %s", argv[0]);
  return 0;
}

int DBManager::RecoverCorruptedDB(sqlite3* db) {
  int ret = ERROR_NONE;
  char* errmsg = nullptr;

  LOGI("DB is corrupted, start to recover corrupted db");
  if (db)
    sqlite3_close(db);
  unlink(DBPATH);

  ret = sqlite3_open_v2(DBPATH, &db,
                        SQLITE_OPEN_CREATE | SQLITE_OPEN_READWRITE, nullptr);
  if (ret != SQLITE_OK) {
    LOGE("Failed to open db[%d]", ret);
    unlink(DBPATH);
    ret = ERROR_FROM_DB;
    goto out;
  }

  ret = sqlite3_exec(db, CREATE_NOTIFICATION_TABLE, nullptr, nullptr, &errmsg);
  if (ret != SQLITE_OK) {
    LOGE("Failed to exec query[%d][%s]", ret, errmsg);
    ret = ERROR_FROM_DB;
  }

out:
  if (errmsg)
    sqlite3_free(errmsg);

  return ret;
}

int DBManager::InitializeDB() {
  int ret = ERROR_NONE;
  int sql_ret;
  sqlite3* db;
  char *errmsg = nullptr;
  bool is_db_corrupted = false;

  sql_ret = sqlite3_open_v2(DBPATH, &db,
                 SQLITE_OPEN_CREATE | SQLITE_OPEN_READWRITE, nullptr);
  if (sql_ret != SQLITE_OK) {
    LOGE("Failed to open db[%d]", ret);
    ret = ERROR_FROM_DB;
    goto out;
  }

  sql_ret = sqlite3_exec(db, CREATE_NOTIFICATION_TABLE, nullptr, nullptr, &errmsg);
  if (sql_ret != SQLITE_OK) {
    LOGE("Failed to exec sqlite[%d][%s]", ret, errmsg);
    ret = ERROR_FROM_DB;
    goto out;
  }

  sql_ret = sqlite3_exec(db, "PRAGMA foreign_keys = ON", NULL, NULL, NULL);
  if (sql_ret != SQLITE_OK) {
    LOGE("Failed to exec sqlite[%d][%s]", ret, errmsg);
    ret = ERROR_FROM_DB;
    goto out;
  }

  sql_ret = sqlite3_exec(db, "PRAGMA integrity_check",
              CheckDBIntegrity, &is_db_corrupted, &errmsg);
  if (sql_ret != SQLITE_OK || is_db_corrupted) {
    LOGE("Failed to exec query[%d][%s]", sql_ret, errmsg);
    ret = ERROR_FROM_DB;
  }

out:
  if (sql_ret == SQLITE_CORRUPT || sql_ret == SQLITE_NOTADB || is_db_corrupted)
    ret = RecoverCorruptedDB(db);
  if (errmsg)
    sqlite3_free(errmsg);
  if (db)
    sqlite3_close(db);

  return ret;
}

void DBManager::InitializeData() {
  char* query;

  query = sqlite3_mprintf("DELETE FROM noti_ex_list WHERE policy & %d",
  static_cast<int>(item::AbstractItem::Policy::OnBootClear));
  if (!query) {
    LOGE("OOM - sql query");
    return;
  }

  ExecuteQuery(query, nullptr);
  sqlite3_free(query);
}

void DBManager::CheckLimit(shared_ptr<item::AbstractItem> addedItem, sqlite3* db) {
  int ret, count;
  char* query;
  sqlite3_stmt* stmt = nullptr;
  sqlite3_stmt* delete_stmt = nullptr;
  int uid = static_pointer_cast<IItemInfoInternal>(addedItem->GetInfo())->GetUid();

  ret = GetCount(addedItem->GetSenderAppId(), uid, &count);
  if (ret != ERROR_NONE || count <= NOTI_LIMIT)
    return;

  query = sqlite3_mprintf("SELECT root_id FROM noti_ex_list WHERE uid = %d"
                          " AND app_id = %Q ORDER BY insert_time ASC",
                          uid, addedItem->GetSenderAppId().c_str());
  if (query == nullptr) {
    LOGE("OOM - sql query");
    return;
  }

  ret = sqlite3_prepare_v2(db, query, -1, &stmt, nullptr);
  if (ret != SQLITE_OK) {
    /* LCOV_EXCL_START */
    LOGE("sqlite3_prepare_v2 Failed [%d][%s]", ret, sqlite3_errmsg(db));
    goto out;
    /* LCOV_EXCL_STOP */
  }

  count -= NOTI_LIMIT;

  sqlite3_free(query);
  query = sqlite3_mprintf("DELETE FROM noti_ex_list"
                          " WHERE root_id = ? AND app_id = %Q AND uid = %d",
                          addedItem->GetSenderAppId().c_str(), uid);
  if (query == nullptr) {
    LOGE("OOM - sql query");
    sqlite3_finalize(stmt);
    return;
  }

  ret = sqlite3_prepare_v2(db, query, -1, &delete_stmt, nullptr);
  if (ret != SQLITE_OK) {
    /* LCOV_EXCL_START */
    LOGE("sqlite3_prepare_v2 Failed [%d][%s]", ret, sqlite3_errmsg(db));
    goto out;
    /* LCOV_EXCL_STOP */
  }

  while (sqlite3_step(stmt) == SQLITE_ROW && count > 0) {
    ret = sqlite3_bind_text(delete_stmt, 1,
            reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0)),
            -1, SQLITE_TRANSIENT);
    if (ret != SQLITE_OK) {
      LOGE("sqlite3_bind_int() error: %d(%s)", ret, sqlite3_errmsg(db));
      goto out;
    }

    ret = sqlite3_step(delete_stmt);
    if (ret != SQLITE_DONE) {
      LOGE("step error: %d(%s)", ret, sqlite3_errmsg(db));
      goto out;
    }

    ret = sqlite3_reset(delete_stmt);
    if (ret != SQLITE_OK) {
      LOGE("sqlite3_reset() error: %d", ret);
      goto out;
    }

    sqlite3_clear_bindings(delete_stmt);

    count--;
  }

out:
  if (stmt)
    sqlite3_finalize(stmt);
  if (delete_stmt)
    sqlite3_finalize(delete_stmt);
  if (query)
    sqlite3_free(query);
}

int DBManager::UpdateReceiverList
    (shared_ptr<item::AbstractItem> updatedItem,  sqlite3* db) {
  int ret;
  char* query;
  int uid = static_pointer_cast<IItemInfoInternal>(updatedItem->GetInfo())->GetUid();

  query = sqlite3_mprintf("DELETE FROM receiver_list"
            " WHERE root_id = %Q AND app_id = %Q AND uid = %d",
            updatedItem->GetId().c_str(), updatedItem->GetSenderAppId().c_str(),
            uid);
  if (!query) {
    LOGE("OOM - sql query");
    return ERROR_OUT_OF_MEMORY;
  }

  ret = ExecuteQuery(db, query, nullptr);
  sqlite3_free(query);

  if (ret != ERROR_NONE)
    return ret;

  for (auto& i : updatedItem->GetReceiverList()) {
    query = sqlite3_mprintf("INSERT INTO receiver_list"
                            " (root_id, app_id, uid, receiver_group)"
                            " VALUES (%Q, %Q, %d, %Q)",
                            updatedItem->GetId().c_str(),
                            updatedItem->GetSenderAppId().c_str(),
                            uid,
                            i.c_str());

    if (!query) {
      LOGE("OOM - sql query");
      return ERROR_OUT_OF_MEMORY;
    }

    ret = ExecuteQuery(db, query, nullptr);
    sqlite3_free(query);

    if (ret != ERROR_NONE)
      return ret;
  }
  return ERROR_NONE;
}

int DBManager::InsertNotification(list<shared_ptr<item::AbstractItem>> addedItem) {
  int ret, count;
  char* query;
  sqlite3* db = OpenDB();

  if (db == nullptr)
    return ERROR_FROM_DB;

  if (sqlite3_exec(db, "BEGIN TRANSACTION", nullptr, nullptr, nullptr)) {
    LOGE("begin transaction error : %s", sqlite3_errmsg(db));
    CloseDB(db);
    return ERROR_FROM_DB;
  }

  for (auto& i : addedItem) {
    int uid = static_pointer_cast<IItemInfoInternal>(i->GetInfo())->GetUid();
    ret = GetCount(i->GetId(), i->GetSenderAppId(), uid, &count);
    if (ret != ERROR_NONE)
       break;

    if (count > 0) {
      LOGE("already exist id :[%s] appid[%s]",
               i->GetId().c_str(), i->GetSenderAppId().c_str());
      ret = ERROR_ALREADY_EXIST_ID;
      break;
    }

    Bundle b = i->Serialize();
    query = sqlite3_mprintf("INSERT INTO noti_ex_list"
          " (root_id, app_id, uid, priv_id, pkg_id, policy, data, insert_time)"
          " VALUES (%Q, %Q, %d, %d, %Q, %d, %Q, %d)",
          i->GetId().c_str(),
          i->GetSenderAppId().c_str(),
          uid,
          util::GetQuarkFromString(i->GetId() + to_string(uid)),
          GetPkgId(i->GetSenderAppId(),uid).c_str(),
          static_cast<int>(i->GetPolicy()),
          reinterpret_cast<char*>(b.ToRaw().first.get()),
          static_pointer_cast<IItemInfo>(i->GetInfo())->GetTime());

    if (!query) {
      LOGE("OOM - sql query");
      ret = ERROR_OUT_OF_MEMORY;
      break;
    }

    ret = ExecuteQuery(db, query, nullptr);
    sqlite3_free(query);
    if (ret != ERROR_NONE)
      break;

    ret = UpdateReceiverList(i, db);
    if (ret != ERROR_NONE)
      break;
  }

  if (ret == ERROR_NONE) {
    CheckLimit(*(addedItem.begin()), db);
    if (sqlite3_exec(db, "END TRANSACTION", nullptr, nullptr, nullptr)) {
      LOGE("end transaction error : %s", sqlite3_errmsg(db));
      ret = ERROR_FROM_DB;
    }
  } else {
    if (sqlite3_exec(db, "ROLLBACK TRANSACTION", nullptr, nullptr, nullptr))
    LOGE("rollback transaction error : %s", sqlite3_errmsg(db));
  }

  CloseDB(db);
  return ret;
}

map<string, string> DBManager::GetHideMap() {
  map<string, string> hide_map;
  char* query;
  sqlite3_stmt* stmt;
  sqlite3* db;
  int ret;
  string key;

  query = sqlite3_mprintf("SELECT root_id, app_id, uid, hide_list FROM noti_ex_list"
                            " WHERE hide_list IS NOT NULL");
  if (!query) {
    LOGE("OOM - sql query");
    return hide_map;
  }

  db = OpenDB();
  if (db == nullptr) {
    sqlite3_free(query);
    return hide_map;
  }

  ret = sqlite3_prepare_v2(db, query, strlen(query), &stmt, nullptr);
  if (ret != SQLITE_OK) {
    LOGE("Failed to sqlite3_prepare [%d][%s]", ret,	sqlite3_errmsg(db));
    CloseDB(db);
    sqlite3_free(query);
    return hide_map;
  }
  sqlite3_free(query);

  while (sqlite3_step(stmt) == SQLITE_ROW) {
    key = string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0))) \
          + string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1))) \
          + string(to_string(sqlite3_column_int(stmt, 2)));
    hide_map[key] = string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3)));
  }

  sqlite3_finalize(stmt);
  CloseDB(db);
  return hide_map;
}

int DBManager::GetCount(const string& app_id, int uid, int* count) {
  return GetCount(string(), app_id, uid, count);
}

int DBManager::GetCount(const string& root_id, const string& app_id,
                              int uid, int* count) {
  int ret;
  char* query;
  sqlite3_stmt *stmt;
  sqlite3* db;

  if (root_id.empty()) {
    query = sqlite3_mprintf("SELECT count(*) FROM noti_ex_list"
                            " WHERE app_id = %Q AND uid = %d", app_id.c_str(), uid);
  } else {
    query = sqlite3_mprintf("SELECT count(*) FROM noti_ex_list"
                            " WHERE root_id = %Q AND app_id = %Q AND uid = %d",
                            root_id.c_str(), app_id.c_str(), uid);
  }
  if (!query) {
    LOGE("OOM - sql query");
    return ERROR_OUT_OF_MEMORY;
  }

  db = OpenDB();
  if (db == nullptr) {
    sqlite3_free(query);
    return ERROR_FROM_DB;
  }

  ret = sqlite3_prepare_v2(db, query, strlen(query), &stmt, nullptr);
  if (ret != SQLITE_OK) {
    LOGE("Failed to sqlite3_prepare [%d][%s]", ret,	sqlite3_errmsg(db));
    sqlite3_free(query);
    CloseDB(db);
    return ERROR_FROM_DB;
  }
  sqlite3_free(query);

  ret = sqlite3_step(stmt);
  if (ret == SQLITE_ROW)
    *count = sqlite3_column_int(stmt, 0);
  else
    *count = 0;

  sqlite3_finalize(stmt);
  CloseDB(db);
  return ERROR_NONE;
}

int DBManager::UpdateHideList(shared_ptr<item::AbstractItem> updatedItem,
    const string& hide_list) {
  int ret;
  char* query;

  query = sqlite3_mprintf("UPDATE noti_ex_list SET hide_list = %Q"
            " WHERE root_id = %Q AND app_id = %Q AND uid = %d",
            hide_list.c_str(), updatedItem->GetId().c_str(),
            updatedItem->GetSenderAppId().c_str(),
            static_pointer_cast<IItemInfoInternal>(updatedItem->GetInfo())->GetUid());
  if (!query) {
    LOGE("OOM - sql query");
    return ERROR_OUT_OF_MEMORY;
  }

  ret = ExecuteQuery(query, nullptr);
  sqlite3_free(query);

  return ret;
}

int DBManager::UpdateNotification(shared_ptr<item::AbstractItem> updatedItem) {
  int count, ret;
  char* query;
  int uid = static_pointer_cast<IItemInfoInternal>(updatedItem->GetInfo())->GetUid();

  ret = GetCount(updatedItem->GetId(), updatedItem->GetSenderAppId(), uid, &count);
  if (ret != ERROR_NONE) {
    LOGE("fail to get count");
    return ret;
  }

  if (count <= 0) {
    LOGE("not exist id(%s) for appid(%s)",
        updatedItem->GetId().c_str(), updatedItem->GetSenderAppId().c_str());
    return ERROR_NOT_EXIST_ID;
  }

  sqlite3* db = OpenDB();
  if (db == nullptr)
    return ERROR_FROM_DB;

  Bundle b = updatedItem->Serialize();
  query = sqlite3_mprintf("UPDATE noti_ex_list SET"
     " priv_id = %d, pkg_id = %Q, policy = %d, data = %Q, insert_time = %d"
     " WHERE root_id = %Q AND app_id = %Q AND uid = %d",
     util::GetQuarkFromString(updatedItem->GetId() + to_string(uid)),
     GetPkgId(updatedItem->GetSenderAppId(), uid).c_str(),
     static_cast<int>(updatedItem->GetPolicy()),
     reinterpret_cast<char*>(b.ToRaw().first.get()),
     static_pointer_cast<IItemInfo>(updatedItem->GetInfo())->GetTime(),
     updatedItem->GetId().c_str(),
     updatedItem->GetSenderAppId().c_str(),
     uid);

  if (!query) {
    LOGE("OOM - sql query");
    CloseDB(db);
    return ERROR_OUT_OF_MEMORY;
  }

  if (sqlite3_exec(db, "BEGIN TRANSACTION", nullptr, nullptr, nullptr)) {
    LOGE("begin transaction error : %s", sqlite3_errmsg(db));
    sqlite3_free(query);
    CloseDB(db);
    return ERROR_FROM_DB;
  }

  ret = ExecuteQuery(db, query, nullptr);
  sqlite3_free(query);
  if (ret != ERROR_NONE)
    goto out;

  ret = UpdateReceiverList(updatedItem, db);

out:
  if (ret == ERROR_NONE) {
    if (sqlite3_exec(db, "END TRANSACTION", nullptr, nullptr, nullptr)) {
      LOGE("end transaction error : %s", sqlite3_errmsg(db));
      ret = ERROR_FROM_DB;
    }
  } else {
    if (sqlite3_exec(db, "ROLLBACK TRANSACTION", nullptr, nullptr, nullptr))
    LOGE("rollback transaction error : %s", sqlite3_errmsg(db));
  }

  CloseDB(db);
  return ret;
}

list<shared_ptr<item::AbstractItem>> DBManager::ExecuteGetList(char* query) {
  sqlite3_stmt *stmt;
  sqlite3* db;
  int ret;
  list<shared_ptr<item::AbstractItem>> item_list;

  if (!query) {
    LOGE("Invalid parameter");
    return item_list;
  }

  db = OpenDB();
  if (db == nullptr)
    return item_list;

  ret = sqlite3_prepare_v2(db, query, strlen(query), &stmt, nullptr);
  if (ret != SQLITE_OK) {
    LOGE("Failed to sqlite3_prepare [%d][%s]", ret, sqlite3_errmsg(db));
    CloseDB(db);
    return item_list;
  }

  while (sqlite3_step(stmt) == SQLITE_ROW) {
    LOGE("[%s]", sqlite3_column_text(stmt, 0));

    Bundle serialized(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)));
    shared_ptr<item::AbstractItem> gen_item = item::ItemInflator::Create(serialized);
    item_list.emplace_back(gen_item);
  }

  sqlite3_finalize(stmt);
  CloseDB(db);
  return item_list;
}

list<shared_ptr<item::AbstractItem>> DBManager::GetNotificationList(string app_id, int uid) {
  char* query;
  list<shared_ptr<item::AbstractItem>> item_list;

  query = sqlite3_mprintf("SELECT data FROM noti_ex_list"
                    " WHERE uid = %d AND app_id = %Q ORDER BY insert_time DESC",
                    uid, app_id.c_str());
  if (!query) {
    LOGE("OOM - sql query");
    return item_list;
  }

  item_list = ExecuteGetList(query);
  sqlite3_free(query);

  return item_list;
}

list<shared_ptr<item::AbstractItem>> DBManager::GetNotificationList
                             (string app_id, string root_id, int uid) {
  char* query;
  list<shared_ptr<item::AbstractItem>> item_list;

  query = sqlite3_mprintf("SELECT data FROM noti_ex_list"
                           " WHERE uid = %d AND app_id = %Q AND root_id = %Q",
                           uid, app_id.c_str(), root_id.c_str());

  if (!query) {
    LOGE("OOM - sql query");
    return item_list;
  }

  item_list = ExecuteGetList(query);
  sqlite3_free(query);

  return item_list;
}

list<shared_ptr<item::AbstractItem>> DBManager::GetNotificationList(int uid) {
  int ret, sim_mode;
  char* query;
  list<shared_ptr<item::AbstractItem>> item_list;

  /* Check current sim status */
  ret = vconf_get_int(VCONFKEY_TELEPHONY_SIM_SLOT, &sim_mode);
  if (ret < 0) {
    sim_mode = VCONFKEY_TELEPHONY_SIM_INSERTED;
    LOGI("vconf_get_int");
  }

  if (sim_mode == VCONFKEY_TELEPHONY_SIM_INSERTED) {
    query = sqlite3_mprintf("SELECT data FROM noti_ex_list WHERE uid = %d"
                            " ORDER BY insert_time DESC", uid);
  } else {
    query = sqlite3_mprintf("SELECT data FROM noti_ex_list"
            " WHERE  uid = %d AND (policy & %d) != 0 ORDER BY insert_time DESC",
            uid, static_cast<int>(item::AbstractItem::Policy::SimMode));
  }

  if (!query) {
    LOGE("OOM - sql query");
    return item_list;
  }

  item_list = ExecuteGetList(query);
  sqlite3_free(query);

  return item_list;
}

int DBManager::DeleteNotification(shared_ptr<item::AbstractItem> deletedItem) {
  int ret;
  char* query;
  int uid = static_pointer_cast<IItemInfoInternal>(deletedItem->GetInfo())->GetUid();

  query = sqlite3_mprintf("DELETE FROM noti_ex_list"
            " WHERE root_id = %Q AND app_id = %Q AND uid = %d",
            deletedItem->GetId().c_str(), deletedItem->GetSenderAppId().c_str(),
            uid);
  if (!query) {
    LOGE("OOM - sql query");
    return ERROR_OUT_OF_MEMORY;
  }

  ret = ExecuteQuery(query, nullptr);
  sqlite3_free(query);
  if (ret != ERROR_NONE)
    return ret;

  query = sqlite3_mprintf("DELETE FROM receiver_list"
                " WHERE root_id = %Q AND app_id = %Q AND uid = %d",
                deletedItem->GetId().c_str(), deletedItem->GetSenderAppId().c_str(),
                uid);
  if (!query) {
    LOGE("OOM - sql query");
    return ERROR_OUT_OF_MEMORY;
  }

  ret = ExecuteQuery(query, nullptr);
  sqlite3_free(query);

  return ret;
}

string DBManager::GetPkgId(const string& app_id, int uid) {
  pkgmgrinfo_pkginfo_h handle;
  char *buf = nullptr;
  string pkgid;

  if (pkgmgrinfo_appinfo_get_usr_appinfo(app_id.c_str(), uid, &handle) == PMINFO_R_OK) {
    if (pkgmgrinfo_appinfo_get_pkgid(handle, &buf) == PMINFO_R_OK) {
      if (buf)
        pkgid = string(buf);
    }
    pkgmgrinfo_appinfo_destroy_appinfo(handle);
  }
  return pkgid;
}
}  // namespace notification
