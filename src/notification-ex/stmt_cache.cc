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

#include "notification-ex/stmt_cache.h"

#include <dlog.h>

#include <utility>

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "NOTIFICATION_EX"

namespace notification {

void StmtCache::AcquiredStmt::ReleaseToCache() noexcept {
  if (stmt_ == nullptr)
    return;

  if (owner_ == nullptr) {
    /* Orphaned acquisition (e.g. cache was cleared between Acquire and
     * scope exit). Finalize to avoid leaking native memory. */
    sqlite3_finalize(stmt_);
    stmt_ = nullptr;
    return;
  }

  owner_->ReinsertFront(key_, stmt_);
  stmt_ = nullptr;
  owner_ = nullptr;
}

StmtCache::AcquiredStmt StmtCache::Acquire(sqlite3* db, const char* sql) {
  if (db == nullptr || sql == nullptr)
    return AcquiredStmt();

  std::string key(sql);
  sqlite3_stmt* stmt = nullptr;

  {
    std::lock_guard<std::mutex> lk(mutex_);
    auto it = index_.find(key);
    if (it != index_.end()) {
      /* Cache hit: take ownership out of the LRU list and return. The
       * stmt will be re-inserted on scope exit via ReleaseToCache. */
      stmt = it->second->second;
      lru_.erase(it->second);
      index_.erase(it);
    }
  }

  if (stmt != nullptr) {
    /* Clean slate for reuse. If either step fails, we still return the
     * stmt so the caller can bind / step / detect errors themselves.
     * Both calls are defensive and idempotent. */
    (void)sqlite3_reset(stmt);
    (void)sqlite3_clear_bindings(stmt);
    return AcquiredStmt(this, std::move(key), stmt);
  }

  /* Cache miss: prepare a fresh statement. Intentionally *not* holding
   * the mutex across prepare_v2 - prepare can be slow and other threads
   * should still be able to look up their own cached plans. */
  int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
  if (rc != SQLITE_OK || stmt == nullptr) {
    LOGE("StmtCache: prepare_v2 failed rc=%d sql=[%s] err=[%s]",
         rc, sql, sqlite3_errmsg(db));
    if (stmt != nullptr)
      sqlite3_finalize(stmt);
    return AcquiredStmt();
  }

  return AcquiredStmt(this, std::move(key), stmt);
}

void StmtCache::ReinsertFront(const std::string& key,
                              sqlite3_stmt* stmt) noexcept {
  if (stmt == nullptr)
    return;

  std::lock_guard<std::mutex> lk(mutex_);

  /* If an entry with the same key reappeared while we were using this
   * stmt (e.g. Acquire re-prepared it), prefer to finalize the incoming
   * one and keep the newer cached entry. */
  auto existing = index_.find(key);
  if (existing != index_.end()) {
    sqlite3_finalize(stmt);
    return;
  }

  lru_.emplace_front(key, stmt);
  index_.emplace(key, lru_.begin());
  DropLRUIfOver();
}

void StmtCache::DropLRUIfOver() noexcept {
  while (lru_.size() > capacity_) {
    auto& back = lru_.back();
    auto it = index_.find(back.first);
    if (it != index_.end())
      index_.erase(it);
    if (back.second != nullptr)
      sqlite3_finalize(back.second);
    lru_.pop_back();
  }
}

void StmtCache::Clear() noexcept {
  std::lock_guard<std::mutex> lk(mutex_);
  for (auto& entry : lru_) {
    if (entry.second != nullptr)
      sqlite3_finalize(entry.second);
  }
  lru_.clear();
  index_.clear();
}

}  // namespace notification
