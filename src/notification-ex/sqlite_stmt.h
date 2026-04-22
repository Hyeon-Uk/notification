/*
 * Copyright (c) 2026 Samsung Electronics Co., Ltd.
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

#ifndef NOTIFICATION_EX_SQLITE_STMT_H_
#define NOTIFICATION_EX_SQLITE_STMT_H_

#include <sqlite3.h>

#include <stdexcept>
#include <string>
#include <utility>

/*
 * N1-T6: SqliteStmt RAII
 * ----------------------
 * Thin movable wrapper around a sqlite3_stmt*. Finalizes on destruction so
 * the "prepare then forget to finalize on every error branch" class of
 * bugs (L6/L7/L8) is structurally impossible.
 *
 * Usage:
 *   SqliteStmt stmt(db, "SELECT ...");  // prepares; throws on failure
 *   sqlite3_bind_int(stmt.Get(), 1, uid);
 *   int rc = sqlite3_step(stmt.Get());
 *   // ... stmt auto-finalizes here
 *
 * Or with an explicit release (transferring ownership to StmtCache):
 *   sqlite3_stmt* raw = stmt.Release();
 *
 * Intentionally not EXPORT_API / not installed into a public header set
 * that external users depend on.  Lives under src/notification-ex/ and is
 * only consumed by the DBManager implementation and its unit tests.
 */

namespace notification {

class SqliteStmt {
 public:
  SqliteStmt() noexcept = default;

  /* Prepare-or-throw ctor. Throws std::runtime_error on prepare failure so
   * callers in C++ paths do not silently proceed with a null stmt_. */
  SqliteStmt(sqlite3* db, const char* sql) {
    if (db == nullptr || sql == nullptr)
      throw std::runtime_error("SqliteStmt: null db/sql");
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt_, nullptr);
    if (rc != SQLITE_OK || stmt_ == nullptr) {
      const char* msg = (db != nullptr) ? sqlite3_errmsg(db) : "unknown";
      stmt_ = nullptr;
      throw std::runtime_error(std::string("SqliteStmt: prepare_v2 failed: ")
                               + (msg ? msg : "(null)"));
    }
  }

  SqliteStmt(sqlite3* db, const std::string& sql)
      : SqliteStmt(db, sql.c_str()) {}

  /* Adopt a pre-prepared statement (e.g. one produced by the cache). */
  explicit SqliteStmt(sqlite3_stmt* adopted) noexcept : stmt_(adopted) {}

  ~SqliteStmt() {
    Finalize();
  }

  SqliteStmt(const SqliteStmt&) = delete;
  SqliteStmt& operator=(const SqliteStmt&) = delete;

  SqliteStmt(SqliteStmt&& other) noexcept : stmt_(other.stmt_) {
    other.stmt_ = nullptr;
  }

  SqliteStmt& operator=(SqliteStmt&& other) noexcept {
    if (this != &other) {
      Finalize();
      stmt_ = other.stmt_;
      other.stmt_ = nullptr;
    }
    return *this;
  }

  sqlite3_stmt* Get() const noexcept { return stmt_; }
  explicit operator bool() const noexcept { return stmt_ != nullptr; }

  /* Relinquish ownership without finalizing. Used when handing the stmt
   * back to the cache for reuse. */
  sqlite3_stmt* Release() noexcept {
    sqlite3_stmt* tmp = stmt_;
    stmt_ = nullptr;
    return tmp;
  }

  /* Explicit early finalize (normally driven by dtor). */
  void Finalize() noexcept {
    if (stmt_ != nullptr) {
      sqlite3_finalize(stmt_);
      stmt_ = nullptr;
    }
  }

 private:
  sqlite3_stmt* stmt_ = nullptr;
};

}  // namespace notification

#endif  // NOTIFICATION_EX_SQLITE_STMT_H_
