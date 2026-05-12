/*
 * Copyright (c) 2025 Samsung Electronics Co., Ltd. All rights reserved.
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

#ifndef __NOTIFICATION_RAII_H__
#define __NOTIFICATION_RAII_H__

#include <sqlite3.h>
#include <glib.h>
#include <memory>

class DbGuard {
 public:
  DbGuard() = default;
  explicit DbGuard(sqlite3 *db) : db_(db) {}
  ~DbGuard() {
    if (db_)
      sqlite3_close(db_);
  }
  DbGuard(const DbGuard &) = delete;
  DbGuard &operator=(const DbGuard &) = delete;
  DbGuard(DbGuard &&o) noexcept : db_(o.db_) { o.db_ = nullptr; }
  DbGuard &operator=(DbGuard &&o) noexcept {
    if (this != &o) {
      if (db_) sqlite3_close(db_);
      db_ = o.db_;
      o.db_ = nullptr;
    }
    return *this;
  }
  sqlite3 *get() const { return db_; }
  sqlite3 **addr() { return &db_; }
  sqlite3 *release() { auto p = db_; db_ = nullptr; return p; }
  explicit operator bool() const { return db_ != nullptr; }
 private:
  sqlite3 *db_ = nullptr;
};

class StmtGuard {
 public:
  StmtGuard() = default;
  explicit StmtGuard(sqlite3_stmt *stmt) : stmt_(stmt) {}
  ~StmtGuard() {
    if (stmt_)
      sqlite3_finalize(stmt_);
  }
  StmtGuard(const StmtGuard &) = delete;
  StmtGuard &operator=(const StmtGuard &) = delete;
  sqlite3_stmt *get() const { return stmt_; }
  sqlite3_stmt **addr() { return &stmt_; }
  sqlite3_stmt *release() { auto p = stmt_; stmt_ = nullptr; return p; }
 private:
  sqlite3_stmt *stmt_ = nullptr;
};

class DbTransactionGuard {
 public:
  explicit DbTransactionGuard(sqlite3 *db) : db_(db) {
    int ret = sqlite3_exec(db_, "BEGIN TRANSACTION",
                           nullptr, nullptr, nullptr);
    begun_ = (ret == SQLITE_OK);
  }
  ~DbTransactionGuard() {
    if (begun_ && !committed_)
      sqlite3_exec(db_, "ROLLBACK TRANSACTION", nullptr, nullptr, nullptr);
  }
  DbTransactionGuard(const DbTransactionGuard &) = delete;
  DbTransactionGuard &operator=(const DbTransactionGuard &) = delete;
  bool valid() const { return begun_; }
  bool commit() {
    if (!begun_)
      return false;
    int ret = sqlite3_exec(db_, "END TRANSACTION",
                           nullptr, nullptr, nullptr);
    committed_ = (ret == SQLITE_OK);
    return committed_;
  }
 private:
  sqlite3 *db_;
  bool begun_ = false;
  bool committed_ = false;
};

struct GFreeDeleter {
  void operator()(gpointer p) const { g_free(p); }
};

template <typename T>
using GlibPtr = std::unique_ptr<T, GFreeDeleter>;

using GCharPtr = GlibPtr<gchar>;

struct FreeDeleter {
  void operator()(void *p) const { free(p); }
};

using CCharPtr = std::unique_ptr<char, FreeDeleter>;

struct SqliteFreeDeleter {
  void operator()(char *p) const { sqlite3_free(p); }
};

using SqliteCharPtr = std::unique_ptr<char, SqliteFreeDeleter>;

#endif /* __NOTIFICATION_RAII_H__ */
