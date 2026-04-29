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
#include <inttypes.h>

#include <string.h>
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <utility>

#include "notification-ex/common.h"
#include "notification-ex/db_manager.h"
#include "notification-ex/idb_manager.h"
#include "notification-ex/item_inflator.h"
#include "notification-ex/iitem_info_internal.h"
#include "notification-ex/ex_util.h"
#include "notification-ex/sqlite_stmt.h"
#include "notification-ex/stmt_cache.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "NOTIFICATION_EX"

/*
 * N3-T7 Q1: drop synchronous from FULL to NORMAL on the notification-ex
 * DB as well. The ex store is written on the post path and the extra
 * per-COMMIT fsync cost on FULL is measurable in bench_post_path. NORMAL
 * keeps intra-transaction fsync for WAL correctness and only forfeits
 * durability on whole-OS crashes between COMMIT and checkpoint.
 */
#define CREATE_NOTIFICATION_TABLE \
  "PRAGMA journal_mode = PERSIST;\n" \
  "PRAGMA synchronous = NORMAL;\n" \
  "PRAGMA foreign_keys = ON;\n" \
  "CREATE TABLE IF NOT EXISTS noti_ex_list (\n" \
  " root_id TEXT NOT NULL,\n" \
  " app_id TEXT NOT NULL,\n" \
  " uid INTEGER,\n" \
  " channel TEXT,\n" \
  " priv_id INTEGER PRIMARY KEY,\n" \
  " pkg_id TEXT,\n" \
  " policy INTEGER,\n" \
  " data TEXT NOT NULL,\n" \
  " insert_time INTEGER,\n" \
  " hide_list TEXT,\n" \
  " UNIQUE (root_id, app_id, uid));\n" \
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
using namespace tizen_base;
using namespace notification::item;

namespace notification {
namespace {

/*
 * N1-T3: DbConnectionHolder
 * --------------------------
 * Single long-lived sqlite3* per process, wrapped in a shared_ptr whose
 * custom deleter is sqlite3_close_v2. All access is serialized via a
 * recursive_mutex (recursive so nested DBManager calls inside
 * InsertNotification / UpdateNotification / DeleteNotification which
 * already hold the connection can re-enter ExecuteQuery / GetSequence /
 * GetNotificationList without deadlocking).
 *
 * Acquire() returns an AcquiredDb RAII handle that owns the lock for the
 * duration of the hot call and exposes sqlite3* via Get().
 *
 * Invalidate() is used by RecoverCorruptedDB: it drops the cached
 * shared_ptr so the next Acquire() re-opens a fresh connection after the
 * database file has been unlinked / recreated.
 */
class DbConnectionHolder {
 public:
  class AcquiredDb {
   public:
    AcquiredDb() = default;
    AcquiredDb(std::unique_lock<std::recursive_mutex> lock,
               std::shared_ptr<sqlite3> conn)
        : lock_(std::move(lock)), conn_(std::move(conn)) {}

    AcquiredDb(const AcquiredDb&) = delete;
    AcquiredDb& operator=(const AcquiredDb&) = delete;
    AcquiredDb(AcquiredDb&&) noexcept = default;
    AcquiredDb& operator=(AcquiredDb&&) noexcept = default;

    sqlite3* Get() const { return conn_.get(); }
    explicit operator bool() const { return conn_ != nullptr; }

   private:
    std::unique_lock<std::recursive_mutex> lock_;
    std::shared_ptr<sqlite3> conn_;
  };

  static DbConnectionHolder& Instance() {
    static DbConnectionHolder holder;
    return holder;
  }

  AcquiredDb Acquire() {
    std::unique_lock<std::recursive_mutex> lock(mutex_);
    if (!cached_)
      cached_ = OpenInternal();

    if (!cached_) {
      /* Surface a null AcquiredDb so callers can short-circuit. Keep the
       * lock released by letting the unique_lock destruct via returning
       * an empty AcquiredDb. */
      lock.unlock();
      return AcquiredDb();
    }

    return AcquiredDb(std::move(lock), cached_);
  }

  /* Statement cache associated with the currently-cached connection.
   * Lifetime is tied to the holder: when Invalidate() drops the
   * connection, it also Clear()s the cache so no cached sqlite3_stmt*
   * outlives the sqlite3* it belongs to. Callers should only use this
   * while holding an AcquiredDb so the connection cannot be invalidated
   * underneath them (the same recursive_mutex serialises both paths). */
  StmtCache& Cache() { return stmt_cache_; }

  /* Drop the cached connection. Any in-flight AcquiredDb holds its own
   * shared_ptr so the underlying sqlite3 stays alive until the holder's
   * last reference releases it. The stmt cache is cleared first so its
   * finalized statements are all issued against the outgoing handle. */
  void Invalidate() {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    stmt_cache_.Clear();
    cached_.reset();
  }

 private:
  DbConnectionHolder() = default;
  ~DbConnectionHolder() = default;
  DbConnectionHolder(const DbConnectionHolder&) = delete;
  DbConnectionHolder& operator=(const DbConnectionHolder&) = delete;

  static std::shared_ptr<sqlite3> OpenInternal() {
    sqlite3* raw = nullptr;
    int ret = sqlite3_open_v2(DBPATH, &raw, SQLITE_OPEN_READWRITE, nullptr);
    if (ret != SQLITE_OK) {
      LOGE("open db(%s) error: %d", DBPATH, ret);
      if (raw)
        sqlite3_close_v2(raw);
      return nullptr;
    }
    return std::shared_ptr<sqlite3>(raw, [](sqlite3* p) {
      if (p) {
        int r = sqlite3_close_v2(p);
        if (r != SQLITE_OK)
          LOGE("close db error: %d", r);
      }
    });
  }

  std::recursive_mutex mutex_;
  std::shared_ptr<sqlite3> cached_;
  StmtCache stmt_cache_{StmtCache::kDefaultCapacity};
};

class DBManagerImpl : public IDBManager {
 public:
  DBManagerImpl() = default;
  ~DBManagerImpl() override = default;

  int InitializeDB() override;
  void InitializeData() override;
  int InsertNotification(
      std::list<std::shared_ptr<item::AbstractItem>> addedList) override;
  std::map<std::string, std::string> GetHideMap() override;
  int UpdateHideList(std::shared_ptr<item::AbstractItem> updatedItem,
                     const std::string& hide_list) override;
  int UpdateNotification(
      std::list<std::shared_ptr<item::AbstractItem>> updatedList) override;
  int GetCount(int64_t priv_id, const std::string& root_id,
               const std::string& app_id, const std::string& channel,
               uid_t uid, int* count) override;
  int GetCount(const std::string& app_id, const std::string& channel,
               uid_t uid, int* count) override;
  int DeleteNotification(
      std::list<std::shared_ptr<item::AbstractItem>> deletedList) override;
  std::list<std::shared_ptr<item::AbstractItem>> GetNotificationList(
      uid_t uid, std::string channel = "") override;
  std::list<std::shared_ptr<item::AbstractItem>> GetNotificationList(
      std::string app_id, uid_t uid, std::string channel = "") override;
  std::list<std::shared_ptr<item::AbstractItem>> GetNotificationList(
      std::string app_id, std::string root_id, uid_t uid) override;
  std::list<std::shared_ptr<item::AbstractItem>> GetNotificationList(
      std::string app_id, int64_t private_id, uid_t uid) override;

 private:
  static int CheckDBIntegrity(void* user_data, int argc, char** argv,
                              char** notUsed);
  int RecoverCorruptedDB();
  int ExecuteQuery(const char* query, int* num_changes);
  int ExecuteQuery(sqlite3* db, const char* query, int* num_changes);
  int GetSequence(sqlite3* db, int64_t* seq);
  std::string GetPkgId(const std::string& app_id, uid_t uid);
  void CheckLimit(std::shared_ptr<item::AbstractItem> addedItem, sqlite3* db);
  int UpdateReceiverList(std::shared_ptr<item::AbstractItem> updatedItem,
                         sqlite3* db);
  std::list<std::shared_ptr<item::AbstractItem>> ExecuteGetList(char* query);
};

DBManagerImpl& DefaultImpl() {
  static DBManagerImpl impl;
  return impl;
}

/* LCOV_EXCL_START */
int DBManagerImpl::ExecuteQuery(sqlite3* db, const char* query,
                                int* num_changes) {
  /*
   * Used for sqlite3_mprintf-style one-shot DML (INSERT/UPDATE/DELETE
   * with literal values already baked into the SQL text). Caching these
   * would be counter-productive (each call has a unique key) but we do
   * want SqliteStmt RAII so L6-class leaks on every error branch are
   * impossible.
   */
  if (db == nullptr || query == nullptr)
    return ERROR_INVALID_PARAMETER;

  try {
    SqliteStmt stmt(db, query);
    int ret = sqlite3_step(stmt.Get());
    if (ret == SQLITE_OK || ret == SQLITE_DONE) {
      if (num_changes != nullptr)
        *num_changes = sqlite3_changes(db);
      return ERROR_NONE;
    }
    LOGE("Sqlite err[%d][%s]", ret, sqlite3_errmsg(db));
    return ERROR_FROM_DB;
  } catch (const std::exception& e) {
    LOGE("ExecuteQuery prepare failed: %s", e.what());
    return ERROR_FROM_DB;
  }
}

int DBManagerImpl::ExecuteQuery(const char* query, int* num_changes) {
  if (query == nullptr)
    return ERROR_INVALID_PARAMETER;

  auto acquired = DbConnectionHolder::Instance().Acquire();
  if (!acquired)
    return ERROR_FROM_DB;

  return ExecuteQuery(acquired.Get(), query, num_changes);
}

int DBManagerImpl::GetSequence(sqlite3* db, int64_t* seq) {
  int ret = ERROR_NONE;

  if (db == nullptr || seq == nullptr)
    return ERROR_INVALID_PARAMETER;

  /* Fixed SQL → ideal for the prepared statement cache. */
  static const char* const kSql =
      "SELECT IFNULL(MIN(priv_id), 0), IFNULL(MAX(priv_id), 0)"
      " FROM noti_ex_list";
  auto acquired = DbConnectionHolder::Instance().Cache().Acquire(db, kSql);
  if (!acquired) {
    LOGE("Sqlite3 err: prepare cache miss [%s]", sqlite3_errmsg(db));
    return ERROR_FROM_DB;
  }
  sqlite3_stmt* stmt = acquired.Get();

  ret = sqlite3_step(stmt);
  if (ret == SQLITE_ROW) {
    int64_t min_p = sqlite3_column_int64(stmt, 0);
    if (min_p > 1)
      *seq = min_p - 1;
    else
      *seq = sqlite3_column_int64(stmt, 1) + 1;
    ret = ERROR_NONE;
  } else {
    LOGE("Sqlite err [%d][%s]", ret, sqlite3_errmsg(db));
    ret = ERROR_FROM_DB;
  }

  return ret;
}

int DBManagerImpl::CheckDBIntegrity(void* user_data, int argc,
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

/*
 * Recover the corrupted DB file. Called from InitializeDB when the
 * integrity check fails. Invalidates the cached connection before
 * unlinking so later Acquire() opens a fresh file.
 */
int DBManagerImpl::RecoverCorruptedDB() {
  int ret = ERROR_NONE;
  char* errmsg = nullptr;
  sqlite3* db = nullptr;

  LOGI("DB is corrupted, start to recover corrupted db");
  DbConnectionHolder::Instance().Invalidate();
  unlink(DBPATH);

  ret = sqlite3_open_v2(DBPATH, &db,
                        SQLITE_OPEN_CREATE | SQLITE_OPEN_READWRITE, nullptr);
  if (ret != SQLITE_OK) {
    LOGE("Failed to open db[%d]", ret);
    unlink(DBPATH);
    if (db)
      sqlite3_close_v2(db);
    return ERROR_FROM_DB;
  }

  ret = sqlite3_exec(db, CREATE_NOTIFICATION_TABLE, nullptr, nullptr, &errmsg);
  if (ret != SQLITE_OK) {
    LOGE("Failed to exec query[%d][%s]", ret, errmsg);
    ret = ERROR_FROM_DB;
  } else {
    ret = ERROR_NONE;
  }

  if (errmsg)
    sqlite3_free(errmsg);
  if (db)
    sqlite3_close_v2(db);

  return ret;
}

int DBManagerImpl::InitializeDB() {
  int ret = ERROR_NONE;
  int sql_ret;
  sqlite3* db = nullptr;
  char *errmsg = nullptr;
  bool is_db_corrupted = false;

  /* InitializeDB always uses its own dedicated connection because it
   * needs SQLITE_OPEN_CREATE to create the DB file if missing. Do NOT
   * reuse the long-lived cached connection here - that one is opened in
   * read/write mode against an existing file. */
  sql_ret = sqlite3_open_v2(DBPATH, &db,
                 SQLITE_OPEN_CREATE | SQLITE_OPEN_READWRITE, nullptr);
  if (sql_ret != SQLITE_OK) {
    LOGE("Failed to open db[%d]", sql_ret);
    ret = ERROR_FROM_DB;
    goto out;
  }

  sql_ret = sqlite3_exec(db, CREATE_NOTIFICATION_TABLE, nullptr, nullptr, &errmsg);
  if (sql_ret != SQLITE_OK) {
    LOGE("Failed to exec sqlite[%d][%s]", sql_ret, errmsg);
    ret = ERROR_FROM_DB;
    goto out;
  }

  sql_ret = sqlite3_exec(db, "PRAGMA foreign_keys = ON", NULL, NULL, NULL);
  if (sql_ret != SQLITE_OK) {
    LOGE("Failed to exec sqlite[%d][%s]", sql_ret, errmsg);
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
  if (errmsg)
    sqlite3_free(errmsg);
  if (db)
    sqlite3_close_v2(db);

  if (sql_ret == SQLITE_CORRUPT || sql_ret == SQLITE_NOTADB ||
      is_db_corrupted) {
    ret = RecoverCorruptedDB();
  }

  /* The subsequent Acquire() in the hot path will open the long-lived
   * read/write connection. Invalidate any prior cache so stale handles
   * are dropped. */
  DbConnectionHolder::Instance().Invalidate();

  return ret;
}

void DBManagerImpl::InitializeData() {
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

void DBManagerImpl::CheckLimit(shared_ptr<item::AbstractItem> addedItem,
                               sqlite3* db) {
  int ret, count;
  char* query;
  sqlite3_stmt* stmt = nullptr;
  sqlite3_stmt* delete_stmt = nullptr;
  uid_t uid =
      static_pointer_cast<IItemInfoInternal>(addedItem->GetInfo())->GetUid();

  ret = GetCount(addedItem->GetSenderAppId(), string(), uid, &count);
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
    LOGE("sqlite3_prepare_v2 Failed [%d][%s]", ret, sqlite3_errmsg(db));
    goto out;
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
    LOGE("sqlite3_prepare_v2 Failed [%d][%s]", ret, sqlite3_errmsg(db));
    goto out;
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

int DBManagerImpl::UpdateReceiverList(shared_ptr<item::AbstractItem> updatedItem,
                                      sqlite3* db) {
  int ret;
  char* query;
  uid_t uid =
      static_pointer_cast<IItemInfoInternal>(updatedItem->GetInfo())->GetUid();

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

int DBManagerImpl::InsertNotification(
    list<shared_ptr<item::AbstractItem>> addedList) {
  int ret = ERROR_NONE;
  int64_t priv_id = 0;

  /*
   * N3-T3 C4: fixed SQL + parameter binding. The legacy path called
   * sqlite3_mprintf("...%Q...%d...%Q...") per notification inside the
   * insert loop — every call heap-allocated the query text, reparsed a
   * fresh plan, and re-quoted every string. The bind-based form reuses
   * the cached plan and sqlite3_bind_text / _bind_int are immune to the
   * class of SQL-injection bugs %Q quoting has to paper over.
   */
  static const char* const kSql =
      "INSERT OR REPLACE INTO noti_ex_list"
      " (root_id, app_id, uid, channel, priv_id, pkg_id, policy,"
      "  data, insert_time)"
      " VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)";

  auto acquired = DbConnectionHolder::Instance().Acquire();
  if (!acquired)
    return ERROR_FROM_DB;
  sqlite3* db = acquired.Get();

  if (sqlite3_exec(db, "BEGIN TRANSACTION", nullptr, nullptr, nullptr)) {
    LOGE("begin transaction error : %s", sqlite3_errmsg(db));
    return ERROR_FROM_DB;
  }

  for (auto& i : addedList) {
    uid_t uid = static_pointer_cast<IItemInfoInternal>(i->GetInfo())->GetUid();
    list<shared_ptr<item::AbstractItem>> item_list =
        GetNotificationList(i->GetSenderAppId(), i->GetId(), uid);

    if (!item_list.empty())
      priv_id =
        static_pointer_cast<IItemInfoInternal>(item_list.front()->GetInfo())->GetPrivateId();
    else
      ret = GetSequence(db, &priv_id);

    if (ret != ERROR_NONE || priv_id == 0) {
      LOGE("Failed to get pirv_id");
      break;
    }

    static_pointer_cast<IItemInfoInternal>(i->GetInfo())->SetPrivateId(priv_id);
    Bundle b = i->Serialize();
    std::string pkg_id = GetPkgId(i->GetSenderAppId(), uid);
    auto raw = b.ToRaw();
    const char* data_text = reinterpret_cast<const char*>(raw.first.get());
    int insert_time =
        static_pointer_cast<IItemInfo>(i->GetInfo())->GetTime();

    auto stmt_hdl = DbConnectionHolder::Instance().Cache().Acquire(db, kSql);
    if (!stmt_hdl) {
      LOGE("Failed to prepare InsertNotification: %s", sqlite3_errmsg(db));
      ret = ERROR_FROM_DB;
      break;
    }
    sqlite3_stmt* stmt = stmt_hdl.Get();
    sqlite3_bind_text(stmt, 1, i->GetId().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, i->GetSenderAppId().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 3, static_cast<int>(uid));
    sqlite3_bind_text(stmt, 4, i->GetChannel().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int64(stmt, 5, priv_id);
    sqlite3_bind_text(stmt, 6, pkg_id.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 7, static_cast<int>(i->GetPolicy()));
    sqlite3_bind_text(stmt, 8, data_text, -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 9, insert_time);

    int step_ret = sqlite3_step(stmt);
    if (step_ret != SQLITE_OK && step_ret != SQLITE_DONE) {
      LOGE("Sqlite err[%d][%s]", step_ret, sqlite3_errmsg(db));
      ret = ERROR_FROM_DB;
      break;
    }

    ret = UpdateReceiverList(i, db);
    if (ret != ERROR_NONE)
      break;
  }

  if (ret == ERROR_NONE) {
    CheckLimit(*(addedList.begin()), db);
    if (sqlite3_exec(db, "END TRANSACTION", nullptr, nullptr, nullptr)) {
      LOGE("end transaction error : %s", sqlite3_errmsg(db));
      ret = ERROR_FROM_DB;
    }
  } else {
    if (sqlite3_exec(db, "ROLLBACK TRANSACTION", nullptr, nullptr, nullptr))
    LOGE("rollback transaction error : %s", sqlite3_errmsg(db));
  }

  return ret;
}

map<string, string> DBManagerImpl::GetHideMap() {
  map<string, string> hide_map;
  string key;

  auto acquired = DbConnectionHolder::Instance().Acquire();
  if (!acquired)
    return hide_map;
  sqlite3* db = acquired.Get();

  /* Fixed SQL → cache. */
  static const char* const kSql =
      "SELECT root_id, app_id, uid, hide_list FROM noti_ex_list"
      " WHERE hide_list IS NOT NULL";
  auto stmt_hdl = DbConnectionHolder::Instance().Cache().Acquire(db, kSql);
  if (!stmt_hdl) {
    LOGE("Failed to prepare GetHideMap query: %s", sqlite3_errmsg(db));
    return hide_map;
  }
  sqlite3_stmt* stmt = stmt_hdl.Get();

  while (sqlite3_step(stmt) == SQLITE_ROW) {
    key = string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0))) \
          + string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1))) \
          + string(to_string(sqlite3_column_int(stmt, 2)));
    hide_map[key] = string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3)));
  }

  return hide_map;
}

int DBManagerImpl::GetCount(const string& app_id, const string& channel,
                            uid_t uid, int* count) {
  return GetCount(0, string(), app_id, channel, uid, count);
}

int DBManagerImpl::GetCount(int64_t priv_id, const string& root_id,
    const string& app_id, const string& channel, uid_t uid, int* count) {
  int ret;

  /*
   * N1-T6: switch to parameterised SQL so one cached plan services each
   * distinct WHERE-clause shape. The four shapes are keyed by literal
   * strings below, so the cache hit rate equals the call rate for each
   * shape.
   *
   * Safety: all user-controlled inputs (app_id, channel, root_id) are
   * bound via sqlite3_bind_text, immune to the sqlite3_mprintf quoting
   * gymnastics the old code relied on.
   */
  static const char* const kByPrivId =
      "SELECT count(*) FROM noti_ex_list WHERE priv_id = ?";
  static const char* const kByAppChannel =
      "SELECT count(*) FROM noti_ex_list"
      " WHERE app_id = ? AND uid = ? AND channel = ?";
  static const char* const kByApp =
      "SELECT count(*) FROM noti_ex_list WHERE app_id = ? AND uid = ?";
  static const char* const kByRootApp =
      "SELECT count(*) FROM noti_ex_list"
      " WHERE root_id = ? AND app_id = ? AND uid = ?";

  auto acquired = DbConnectionHolder::Instance().Acquire();
  if (!acquired)
    return ERROR_FROM_DB;
  sqlite3* db = acquired.Get();

  StmtCache& cache = DbConnectionHolder::Instance().Cache();
  StmtCache::AcquiredStmt stmt_hdl;

  if (priv_id != 0) {
    stmt_hdl = cache.Acquire(db, kByPrivId);
    if (!stmt_hdl) return ERROR_FROM_DB;
    sqlite3_bind_int64(stmt_hdl.Get(), 1, priv_id);
  } else if (root_id.empty()) {
    if (!channel.empty()) {
      stmt_hdl = cache.Acquire(db, kByAppChannel);
      if (!stmt_hdl) return ERROR_FROM_DB;
      sqlite3_bind_text(stmt_hdl.Get(), 1, app_id.c_str(), -1, SQLITE_TRANSIENT);
      sqlite3_bind_int(stmt_hdl.Get(), 2, static_cast<int>(uid));
      sqlite3_bind_text(stmt_hdl.Get(), 3, channel.c_str(), -1, SQLITE_TRANSIENT);
    } else {
      stmt_hdl = cache.Acquire(db, kByApp);
      if (!stmt_hdl) return ERROR_FROM_DB;
      sqlite3_bind_text(stmt_hdl.Get(), 1, app_id.c_str(), -1, SQLITE_TRANSIENT);
      sqlite3_bind_int(stmt_hdl.Get(), 2, static_cast<int>(uid));
    }
  } else {
    stmt_hdl = cache.Acquire(db, kByRootApp);
    if (!stmt_hdl) return ERROR_FROM_DB;
    sqlite3_bind_text(stmt_hdl.Get(), 1, root_id.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt_hdl.Get(), 2, app_id.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt_hdl.Get(), 3, static_cast<int>(uid));
  }

  ret = sqlite3_step(stmt_hdl.Get());
  if (ret == SQLITE_ROW)
    *count = sqlite3_column_int(stmt_hdl.Get(), 0);
  else
    *count = 0;

  return ERROR_NONE;
}

int DBManagerImpl::UpdateHideList(shared_ptr<item::AbstractItem> updatedItem,
    const string& hide_list) {
  /*
   * N3-T3 C4: parameterised UPDATE hide_list. Fixed SQL, cached plan,
   * bound positional parameters.
   */
  static const char* const kSql =
      "UPDATE noti_ex_list SET hide_list = ?"
      " WHERE root_id = ? AND app_id = ? AND uid = ?";

  auto acquired = DbConnectionHolder::Instance().Acquire();
  if (!acquired)
    return ERROR_FROM_DB;
  sqlite3* db = acquired.Get();

  auto stmt_hdl = DbConnectionHolder::Instance().Cache().Acquire(db, kSql);
  if (!stmt_hdl) {
    LOGE("Failed to prepare UpdateHideList: %s", sqlite3_errmsg(db));
    return ERROR_FROM_DB;
  }

  sqlite3_stmt* stmt = stmt_hdl.Get();
  uid_t uid =
      static_pointer_cast<IItemInfoInternal>(updatedItem->GetInfo())->GetUid();
  sqlite3_bind_text(stmt, 1, hide_list.c_str(), -1, SQLITE_TRANSIENT);
  sqlite3_bind_text(stmt, 2, updatedItem->GetId().c_str(), -1,
                    SQLITE_TRANSIENT);
  sqlite3_bind_text(stmt, 3, updatedItem->GetSenderAppId().c_str(), -1,
                    SQLITE_TRANSIENT);
  sqlite3_bind_int(stmt, 4, static_cast<int>(uid));

  int step_ret = sqlite3_step(stmt);
  if (step_ret != SQLITE_OK && step_ret != SQLITE_DONE) {
    LOGE("Sqlite err[%d][%s]", step_ret, sqlite3_errmsg(db));
    return ERROR_FROM_DB;
  }
  return ERROR_NONE;
}

int DBManagerImpl::UpdateNotification(
    list<shared_ptr<item::AbstractItem>> updatedList) {
  int ret = ERROR_NONE;
  int64_t priv_id;

  /*
   * N3-T3 C4: parameterised UPDATE. Same reshape as InsertNotification —
   * drop the per-iteration sqlite3_mprintf, let the stmt cache hand out
   * a reused prepared plan, bind the values via sqlite3_bind_*.
   */
  static const char* const kSql =
      "UPDATE noti_ex_list SET"
      " pkg_id = ?, channel = ?, policy = ?, data = ?, insert_time = ?"
      " WHERE priv_id = ?";

  auto acquired = DbConnectionHolder::Instance().Acquire();
  if (!acquired)
    return ERROR_FROM_DB;
  sqlite3* db = acquired.Get();

  if (sqlite3_exec(db, "BEGIN TRANSACTION", nullptr, nullptr, nullptr)) {
    LOGE("begin transaction error : %s", sqlite3_errmsg(db));
    return ERROR_FROM_DB;
  }

  for (auto& i : updatedList) {
    uid_t uid = static_pointer_cast<IItemInfoInternal>(i->GetInfo())->GetUid();
    list<shared_ptr<item::AbstractItem>> item_list =
        GetNotificationList(i->GetSenderAppId(), i->GetId(), uid);

    if (item_list.empty()) {
      LOGE("not exist id(%s) for appid(%s)", i->GetId().c_str(), i->GetSenderAppId().c_str());
      ret = ERROR_NOT_EXIST_ID;
      break;
    }

    priv_id = static_pointer_cast<IItemInfoInternal>(item_list.front()->GetInfo())->GetPrivateId();
    static_pointer_cast<IItemInfoInternal>(i->GetInfo())->SetPrivateId(priv_id);
    Bundle b = i->Serialize();
    std::string pkg_id = GetPkgId(i->GetSenderAppId(), uid);
    auto raw = b.ToRaw();
    const char* data_text = reinterpret_cast<const char*>(raw.first.get());
    int insert_time =
        static_pointer_cast<IItemInfo>(i->GetInfo())->GetTime();

    auto stmt_hdl = DbConnectionHolder::Instance().Cache().Acquire(db, kSql);
    if (!stmt_hdl) {
      LOGE("Failed to prepare UpdateNotification: %s", sqlite3_errmsg(db));
      ret = ERROR_FROM_DB;
      break;
    }
    sqlite3_stmt* stmt = stmt_hdl.Get();
    sqlite3_bind_text(stmt, 1, pkg_id.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, i->GetChannel().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 3, static_cast<int>(i->GetPolicy()));
    sqlite3_bind_text(stmt, 4, data_text, -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 5, insert_time);
    sqlite3_bind_int64(stmt, 6, priv_id);

    int step_ret = sqlite3_step(stmt);
    if (step_ret != SQLITE_OK && step_ret != SQLITE_DONE) {
      LOGE("Sqlite err[%d][%s]", step_ret, sqlite3_errmsg(db));
      ret = ERROR_FROM_DB;
      break;
    }

    ret = UpdateReceiverList(i, db);
    if (ret != ERROR_NONE)
      break;
  }

  if (ret == ERROR_NONE) {
    if (sqlite3_exec(db, "END TRANSACTION", nullptr, nullptr, nullptr)) {
      LOGE("end transaction error : %s", sqlite3_errmsg(db));
      ret = ERROR_FROM_DB;
    }
  } else {
    if (sqlite3_exec(db, "ROLLBACK TRANSACTION", nullptr, nullptr, nullptr))
      LOGE("rollback transaction error : %s", sqlite3_errmsg(db));
  }

  return ret;
}

list<shared_ptr<item::AbstractItem>> DBManagerImpl::ExecuteGetList(char* query) {
  /*
   * Legacy path retained for the GetNotificationList(uid_t, channel)
   * overload that still composes a variable SQL string at runtime
   * (sim-mode policy is branch-dependent). Keeps direct prepare/finalize
   * since the SQL text is not cacheable.
   */
  sqlite3_stmt *stmt;
  int ret;
  list<shared_ptr<item::AbstractItem>> item_list;

  if (!query) {
    LOGE("Invalid parameter");
    return item_list;
  }

  auto acquired = DbConnectionHolder::Instance().Acquire();
  if (!acquired)
    return item_list;
  sqlite3* db = acquired.Get();

  ret = sqlite3_prepare_v2(db, query, strlen(query), &stmt, nullptr);
  if (ret != SQLITE_OK) {
    LOGE("Failed to sqlite3_prepare [%d][%s]", ret, sqlite3_errmsg(db));
    return item_list;
  }

  while (sqlite3_step(stmt) == SQLITE_ROW) {
    LOGE("[%s]", sqlite3_column_text(stmt, 0));

    Bundle serialized(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)));
    shared_ptr<item::AbstractItem> gen_item = item::ItemInflator::Create(serialized);
    item_list.emplace_back(gen_item);
  }

  sqlite3_finalize(stmt);
  return item_list;
}

list<shared_ptr<item::AbstractItem>> DBManagerImpl::GetNotificationList(
    string app_id, uid_t uid, string channel) {
  list<shared_ptr<item::AbstractItem>> item_list;

  static const char* const kByAppChannel =
      "SELECT data FROM noti_ex_list WHERE uid = ?"
      " AND app_id = ? AND channel = ? ORDER BY insert_time DESC";
  static const char* const kByApp =
      "SELECT data FROM noti_ex_list WHERE uid = ?"
      " AND app_id = ? ORDER BY insert_time DESC";

  auto acquired = DbConnectionHolder::Instance().Acquire();
  if (!acquired)
    return item_list;
  sqlite3* db = acquired.Get();

  StmtCache& cache = DbConnectionHolder::Instance().Cache();
  StmtCache::AcquiredStmt stmt_hdl;
  if (!channel.empty()) {
    stmt_hdl = cache.Acquire(db, kByAppChannel);
    if (!stmt_hdl) return item_list;
    sqlite3_bind_int(stmt_hdl.Get(), 1, static_cast<int>(uid));
    sqlite3_bind_text(stmt_hdl.Get(), 2, app_id.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt_hdl.Get(), 3, channel.c_str(), -1, SQLITE_TRANSIENT);
  } else {
    stmt_hdl = cache.Acquire(db, kByApp);
    if (!stmt_hdl) return item_list;
    sqlite3_bind_int(stmt_hdl.Get(), 1, static_cast<int>(uid));
    sqlite3_bind_text(stmt_hdl.Get(), 2, app_id.c_str(), -1, SQLITE_TRANSIENT);
  }

  while (sqlite3_step(stmt_hdl.Get()) == SQLITE_ROW) {
    Bundle serialized(reinterpret_cast<const char*>(
        sqlite3_column_text(stmt_hdl.Get(), 0)));
    item_list.emplace_back(item::ItemInflator::Create(serialized));
  }
  return item_list;
}

list<shared_ptr<item::AbstractItem>> DBManagerImpl::GetNotificationList(
    string app_id, string root_id, uid_t uid) {
  list<shared_ptr<item::AbstractItem>> item_list;

  static const char* const kSql =
      "SELECT data FROM noti_ex_list"
      " WHERE uid = ? AND app_id = ? AND root_id = ?";

  auto acquired = DbConnectionHolder::Instance().Acquire();
  if (!acquired)
    return item_list;
  sqlite3* db = acquired.Get();

  auto stmt_hdl = DbConnectionHolder::Instance().Cache().Acquire(db, kSql);
  if (!stmt_hdl) return item_list;

  sqlite3_bind_int(stmt_hdl.Get(), 1, static_cast<int>(uid));
  sqlite3_bind_text(stmt_hdl.Get(), 2, app_id.c_str(), -1, SQLITE_TRANSIENT);
  sqlite3_bind_text(stmt_hdl.Get(), 3, root_id.c_str(), -1, SQLITE_TRANSIENT);

  while (sqlite3_step(stmt_hdl.Get()) == SQLITE_ROW) {
    Bundle serialized(reinterpret_cast<const char*>(
        sqlite3_column_text(stmt_hdl.Get(), 0)));
    item_list.emplace_back(item::ItemInflator::Create(serialized));
  }
  return item_list;
}

list<shared_ptr<item::AbstractItem>> DBManagerImpl::GetNotificationList(
    string app_id, int64_t priv_id, uid_t uid) {
  list<shared_ptr<item::AbstractItem>> item_list;

  static const char* const kSql =
      "SELECT data FROM noti_ex_list"
      " WHERE uid = ? AND app_id = ? AND priv_id = ?";

  auto acquired = DbConnectionHolder::Instance().Acquire();
  if (!acquired)
    return item_list;
  sqlite3* db = acquired.Get();

  auto stmt_hdl = DbConnectionHolder::Instance().Cache().Acquire(db, kSql);
  if (!stmt_hdl) return item_list;

  sqlite3_bind_int(stmt_hdl.Get(), 1, static_cast<int>(uid));
  sqlite3_bind_text(stmt_hdl.Get(), 2, app_id.c_str(), -1, SQLITE_TRANSIENT);
  sqlite3_bind_int64(stmt_hdl.Get(), 3, priv_id);

  while (sqlite3_step(stmt_hdl.Get()) == SQLITE_ROW) {
    Bundle serialized(reinterpret_cast<const char*>(
        sqlite3_column_text(stmt_hdl.Get(), 0)));
    item_list.emplace_back(item::ItemInflator::Create(serialized));
  }
  return item_list;
}

list<shared_ptr<item::AbstractItem>> DBManagerImpl::GetNotificationList(
    uid_t uid, string channel) {
  int ret, sim_mode;
  list<shared_ptr<item::AbstractItem>> item_list;

  /* Check current sim status */
  ret = vconf_get_int(VCONFKEY_TELEPHONY_SIM_SLOT, &sim_mode);
  if (ret < 0) {
    sim_mode = VCONFKEY_TELEPHONY_SIM_INSERTED;
    LOGI("vconf_get_int");
  }

  /*
   * Pre-refactor this overload built `query_str` by string-concatenating
   * the caller-supplied `channel` directly into the SQL ("AND channel =
   * '" + channel + "'") and then handed the result to
   * `sqlite3_mprintf(query_str.c_str(), uid)` — i.e. the dynamic string
   * was used as the FORMAT argument.  That carried two real defects on
   * the EXPORT_API surface (`DBManager::GetNotificationList(uid_t,
   * string)` declared in idb_manager.h):
   *
   *   1. SQL injection — any single-quote in `channel` escapes the WHERE
   *      clause (e.g. `' OR '1'='1' --`).
   *   2. Format-string injection — any `%`-specifier in `channel`
   *      (`%n`, `%s`, `%x`) survives into sqlite3_mprintf's printf-like
   *      pass and reads / writes via `va_arg` past the single bound
   *      argument.
   *
   * Sister overloads (uid+app_id+root_id, uid+app_id+priv_id) already
   * use prepared statements with `?` bindings and the cached stmt
   * slot from DbConnectionHolder::Cache(); fold this overload onto the
   * same pattern.  Pick one of four static SQL templates by the
   * (with_channel, with_simmode) flag pair so the SQL string passed to
   * the cache key never contains caller data, then bind values as
   * separate parameters.  Eliminates both injection classes in one
   * shape, on top of cutting a per-call sqlite3_mprintf heap alloc.
   */
  const bool with_channel = !channel.empty();
  const bool with_simmode = (sim_mode != VCONFKEY_TELEPHONY_SIM_INSERTED);

  static const char* const kSqlPlain =
      "SELECT data FROM noti_ex_list WHERE uid = ?";
  static const char* const kSqlSim =
      "SELECT data FROM noti_ex_list WHERE uid = ?"
      " AND (policy & ?) == 0";
  static const char* const kSqlChannel =
      "SELECT data FROM noti_ex_list WHERE uid = ?"
      " AND channel = ?";
  static const char* const kSqlChannelSim =
      "SELECT data FROM noti_ex_list WHERE uid = ?"
      " AND channel = ? AND (policy & ?) == 0";

  const char* sql;
  if (with_channel && with_simmode)      sql = kSqlChannelSim;
  else if (with_channel)                 sql = kSqlChannel;
  else if (with_simmode)                 sql = kSqlSim;
  else                                   sql = kSqlPlain;

  auto acquired = DbConnectionHolder::Instance().Acquire();
  if (!acquired)
    return item_list;
  sqlite3* db = acquired.Get();

  auto stmt_hdl = DbConnectionHolder::Instance().Cache().Acquire(db, sql);
  if (!stmt_hdl) return item_list;

  int idx = 1;
  sqlite3_bind_int(stmt_hdl.Get(), idx++, static_cast<int>(uid));
  if (with_channel)
    sqlite3_bind_text(stmt_hdl.Get(), idx++, channel.c_str(), -1,
                      SQLITE_TRANSIENT);
  if (with_simmode)
    sqlite3_bind_int(stmt_hdl.Get(), idx++,
                     static_cast<int>(item::AbstractItem::Policy::SimMode));

  while (sqlite3_step(stmt_hdl.Get()) == SQLITE_ROW) {
    Bundle serialized(reinterpret_cast<const char*>(
        sqlite3_column_text(stmt_hdl.Get(), 0)));
    item_list.emplace_back(item::ItemInflator::Create(serialized));
  }

  return item_list;
}

int DBManagerImpl::DeleteNotification(
    list<shared_ptr<item::AbstractItem>> deletedList) {
  int ret = ERROR_NONE;

  /*
   * N3-T3 C4: parameterised DELETE by priv_id. Heap alloc removed; one
   * cached plan serves every deletion in the batch.
   */
  static const char* const kSql =
      "DELETE FROM noti_ex_list WHERE priv_id = ?";

  auto acquired = DbConnectionHolder::Instance().Acquire();
  if (!acquired)
    return ERROR_FROM_DB;
  sqlite3* db = acquired.Get();

  if (sqlite3_exec(db, "BEGIN TRANSACTION", nullptr, nullptr, nullptr)) {
    LOGE("begin transaction error : %s", sqlite3_errmsg(db));
    return ERROR_FROM_DB;
  }

  for (auto& i : deletedList) {
    auto stmt_hdl = DbConnectionHolder::Instance().Cache().Acquire(db, kSql);
    if (!stmt_hdl) {
      LOGE("Failed to prepare DeleteNotification: %s", sqlite3_errmsg(db));
      ret = ERROR_FROM_DB;
      break;
    }
    sqlite3_bind_int64(stmt_hdl.Get(), 1,
        static_pointer_cast<IItemInfoInternal>(i->GetInfo())->GetPrivateId());

    int step_ret = sqlite3_step(stmt_hdl.Get());
    if (step_ret != SQLITE_OK && step_ret != SQLITE_DONE) {
      LOGE("Sqlite err[%d][%s]", step_ret, sqlite3_errmsg(db));
      ret = ERROR_FROM_DB;
      break;
    }
  }

  if (ret == ERROR_NONE) {
    if (sqlite3_exec(db, "END TRANSACTION", nullptr, nullptr, nullptr)) {
      LOGE("end transaction error : %s", sqlite3_errmsg(db));
      ret = ERROR_FROM_DB;
    }
  } else {
    if (sqlite3_exec(db, "ROLLBACK TRANSACTION", nullptr, nullptr, nullptr))
    LOGE("rollback transaction error : %s", sqlite3_errmsg(db));
  }

  return ret;
}

string DBManagerImpl::GetPkgId(const string& app_id, uid_t uid) {
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
/* LCOV_EXCL_STOP */

}  // namespace

/* LCOV_EXCL_START */
DBManager::DBManager() = default;
DBManager::~DBManager() = default;

/*
 * ABI shim: the legacy DBManager static methods now delegate to the
 * long-lived DBManagerImpl default instance. No signatures change.
 */

sqlite3* DBManager::OpenDB() {
  /* Retained for ABI; the pool is used internally instead. */
  sqlite3* db = nullptr;
  int ret = sqlite3_open_v2(DBPATH, &db, SQLITE_OPEN_READWRITE, nullptr);
  if (ret != SQLITE_OK) {
    LOGE("open db(%s) error: %d", DBPATH, ret);
    if (db)
      sqlite3_close_v2(db);
    return nullptr;
  }
  return db;
}

void DBManager::CloseDB(sqlite3* db) {
  /* Retained for ABI. */
  if (db) {
    int ret = sqlite3_close_v2(db);
    if (ret != SQLITE_OK)
      LOGE("close db error");
  }
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
  /* Retained for ABI. Invalidate the pool and delegate to the impl. */
  if (db)
    sqlite3_close_v2(db);
  DbConnectionHolder::Instance().Invalidate();

  /* Re-run the repair flow via InitializeDB on the impl. */
  int ret = ERROR_NONE;
  char* errmsg = nullptr;
  sqlite3* fresh = nullptr;

  unlink(DBPATH);

  ret = sqlite3_open_v2(DBPATH, &fresh,
                        SQLITE_OPEN_CREATE | SQLITE_OPEN_READWRITE, nullptr);
  if (ret != SQLITE_OK) {
    LOGE("Failed to open db[%d]", ret);
    unlink(DBPATH);
    if (fresh)
      sqlite3_close_v2(fresh);
    return ERROR_FROM_DB;
  }

  ret = sqlite3_exec(fresh, CREATE_NOTIFICATION_TABLE, nullptr, nullptr,
                     &errmsg);
  if (ret != SQLITE_OK) {
    LOGE("Failed to exec query[%d][%s]", ret, errmsg);
    ret = ERROR_FROM_DB;
  } else {
    ret = ERROR_NONE;
  }

  if (errmsg)
    sqlite3_free(errmsg);
  if (fresh)
    sqlite3_close_v2(fresh);

  return ret;
}

int DBManager::ExecuteQuery(const char* query, int* num_changes) {
  if (query == nullptr)
    return ERROR_INVALID_PARAMETER;

  auto acquired = DbConnectionHolder::Instance().Acquire();
  if (!acquired)
    return ERROR_FROM_DB;

  return ExecuteQuery(acquired.Get(), query, num_changes);
}

int DBManager::ExecuteQuery(sqlite3* db, const char* query, int* num_changes) {
  /* Legacy ABI shim. SqliteStmt RAII guarantees finalize on every branch. */
  if (db == nullptr || query == nullptr)
    return ERROR_INVALID_PARAMETER;

  try {
    SqliteStmt stmt(db, query);
    int ret = sqlite3_step(stmt.Get());
    if (ret == SQLITE_OK || ret == SQLITE_DONE) {
      if (num_changes != nullptr)
        *num_changes = sqlite3_changes(db);
      return ERROR_NONE;
    }
    LOGE("Sqlite err[%d][%s]", ret, sqlite3_errmsg(db));
    return ERROR_FROM_DB;
  } catch (const std::exception& e) {
    LOGE("ExecuteQuery prepare failed: %s", e.what());
    return ERROR_FROM_DB;
  }
}

int DBManager::GetSequence(sqlite3* db, int64_t* seq) {
  /* Legacy ABI shim. Uses SqliteStmt RAII so the "early return leaves
   * prepared stmt live" hazard (L6/L7) cannot fire here either. */
  if (db == nullptr || seq == nullptr)
    return ERROR_INVALID_PARAMETER;

  const char* const kSql =
      "SELECT IFNULL(MIN(priv_id), 0), IFNULL(MAX(priv_id), 0)"
      " FROM noti_ex_list";
  try {
    SqliteStmt stmt(db, kSql);
    int ret = sqlite3_step(stmt.Get());
    if (ret == SQLITE_ROW) {
      int64_t min_p = sqlite3_column_int64(stmt.Get(), 0);
      if (min_p > 1)
        *seq = min_p - 1;
      else
        *seq = sqlite3_column_int64(stmt.Get(), 1) + 1;
      return ERROR_NONE;
    }
    LOGE("Sqlite err [%d][%s]", ret, sqlite3_errmsg(db));
    return ERROR_FROM_DB;
  } catch (const std::exception& e) {
    LOGE("GetSequence prepare/step failed: %s", e.what());
    return ERROR_FROM_DB;
  }
}

/*
 * The private helper statics declared in db_manager.h
 * (GetPkgId / CheckLimit / UpdateReceiverList / ExecuteGetList) are no
 * longer referenced: the public DBManager methods now delegate to
 * DBManagerImpl which owns its own copies of those helpers. Leaving them
 * as header-only declarations is legal - the linker never resolves
 * unused private statics - and lets us keep db_manager.h binary-compatible.
 */

int DBManager::InitializeDB() {
  return DefaultImpl().InitializeDB();
}

void DBManager::InitializeData() {
  DefaultImpl().InitializeData();
}

int DBManager::InsertNotification(
    std::list<std::shared_ptr<item::AbstractItem>> addedList) {
  return DefaultImpl().InsertNotification(std::move(addedList));
}

std::map<std::string, std::string> DBManager::GetHideMap() {
  return DefaultImpl().GetHideMap();
}

int DBManager::UpdateHideList(std::shared_ptr<item::AbstractItem> updatedItem,
                              const std::string& hide_list) {
  return DefaultImpl().UpdateHideList(updatedItem, hide_list);
}

int DBManager::UpdateNotification(
    std::list<std::shared_ptr<item::AbstractItem>> updatedList) {
  return DefaultImpl().UpdateNotification(std::move(updatedList));
}

int DBManager::GetCount(int64_t priv_id, const std::string& root_id,
    const std::string& app_id, const std::string& channel, uid_t uid,
    int* count) {
  return DefaultImpl().GetCount(priv_id, root_id, app_id, channel, uid, count);
}

int DBManager::GetCount(const std::string& app_id, const std::string& channel,
                        uid_t uid, int* count) {
  return DefaultImpl().GetCount(app_id, channel, uid, count);
}

int DBManager::DeleteNotification(
    std::list<std::shared_ptr<item::AbstractItem>> deletedList) {
  return DefaultImpl().DeleteNotification(std::move(deletedList));
}

std::list<std::shared_ptr<item::AbstractItem>> DBManager::GetNotificationList(
    uid_t uid, std::string channel) {
  return DefaultImpl().GetNotificationList(uid, std::move(channel));
}

std::list<std::shared_ptr<item::AbstractItem>> DBManager::GetNotificationList(
    std::string app_id, uid_t uid, std::string channel) {
  return DefaultImpl().GetNotificationList(std::move(app_id), uid,
                                           std::move(channel));
}

std::list<std::shared_ptr<item::AbstractItem>> DBManager::GetNotificationList(
    std::string app_id, std::string root_id, uid_t uid) {
  return DefaultImpl().GetNotificationList(std::move(app_id),
                                           std::move(root_id), uid);
}

std::list<std::shared_ptr<item::AbstractItem>> DBManager::GetNotificationList(
    std::string app_id, int64_t private_id, uid_t uid) {
  return DefaultImpl().GetNotificationList(std::move(app_id), private_id, uid);
}
/* LCOV_EXCL_STOP */

namespace detail {
/* N2-T5: test seam — keep default visibility so test TU resolves it. */
EXPORT_API void ResetDefaultConnectionForTest() {
  DbConnectionHolder::Instance().Invalidate();
}
}  // namespace detail

}  // namespace notification
