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

#ifndef NOTIFICATION_EX_STMT_CACHE_H_
#define NOTIFICATION_EX_STMT_CACHE_H_

#include <sqlite3.h>

#include <cstddef>
#include <list>
#include <mutex>
#include <string>
#include <unordered_map>

#include "notification-ex/sqlite_stmt.h"

#ifndef EXPORT_API
#define EXPORT_API __attribute__((visibility("default")))
#endif

/*
 * N1-T6: Prepared statement cache (LRU, per sqlite3*).
 *
 * Contract
 *   - Capacity is fixed (default 16). Beyond that, the least-recently-used
 *     statement is sqlite3_finalize()'d to reclaim native memory.
 *   - Keys are SQL text strings. Callers MUST produce parameterised SQL
 *     (use '?' placeholders) so a single key services many calls; do not
 *     embed per-call values into the SQL (that defeats the cache and is
 *     also an injection risk).
 *   - Acquire() returns an AcquiredStmt RAII handle. On a hit we
 *     sqlite3_reset + sqlite3_clear_bindings before returning, so callers
 *     always start from a clean slate. Release happens automatically in
 *     the dtor, which moves the handle back into the LRU front.
 *   - Cache lifetime is tied to the connection it caches for. Callers
 *     must Clear() the cache before the underlying sqlite3* is closed
 *     (the DBManager holder does so on Invalidate()).
 *
 * Non-goals
 *   - No cross-connection caching: two connections serve distinct keys.
 *   - No thread-local caches: the cache is shared and serialized by its
 *     own mutex, matching the recursive_mutex that already guards the
 *     connection.
 *
 * Not EXPORT_API; internal implementation detail.
 */

namespace notification {

// N2-T5: StmtCache is consumed from the unittest binary (see
// test_stmt_cache.cc) as well as from inside the .so. Expose its non-inline
// members so the test TU can link once project-wide -fvisibility=hidden is
// active on C++ code.
class EXPORT_API StmtCache {
 public:
  static constexpr std::size_t kDefaultCapacity = 16;

  class EXPORT_API AcquiredStmt {
   public:
    AcquiredStmt() noexcept = default;
    AcquiredStmt(StmtCache* owner, std::string key, sqlite3_stmt* stmt) noexcept
        : owner_(owner), key_(std::move(key)), stmt_(stmt) {}

    AcquiredStmt(const AcquiredStmt&) = delete;
    AcquiredStmt& operator=(const AcquiredStmt&) = delete;

    AcquiredStmt(AcquiredStmt&& other) noexcept
        : owner_(other.owner_),
          key_(std::move(other.key_)),
          stmt_(other.stmt_) {
      other.owner_ = nullptr;
      other.stmt_ = nullptr;
    }

    AcquiredStmt& operator=(AcquiredStmt&& other) noexcept {
      if (this != &other) {
        ReleaseToCache();
        owner_ = other.owner_;
        key_ = std::move(other.key_);
        stmt_ = other.stmt_;
        other.owner_ = nullptr;
        other.stmt_ = nullptr;
      }
      return *this;
    }

    ~AcquiredStmt() {
      ReleaseToCache();
    }

    sqlite3_stmt* Get() const noexcept { return stmt_; }
    explicit operator bool() const noexcept { return stmt_ != nullptr; }

    /* Drop the statement instead of re-caching it (e.g. step returned
     * SQLITE_ERROR and the prepared plan may be in a weird state). */
    void Invalidate() noexcept {
      if (stmt_ != nullptr) {
        sqlite3_finalize(stmt_);
        stmt_ = nullptr;
      }
      owner_ = nullptr;
    }

   private:
    void ReleaseToCache() noexcept;

    StmtCache* owner_ = nullptr;
    std::string key_;
    sqlite3_stmt* stmt_ = nullptr;
  };

  explicit StmtCache(std::size_t capacity = kDefaultCapacity) noexcept
      : capacity_(capacity == 0 ? 1 : capacity) {}

  ~StmtCache() { Clear(); }

  StmtCache(const StmtCache&) = delete;
  StmtCache& operator=(const StmtCache&) = delete;

  /* Acquire a reusable prepared statement for `sql` on connection `db`.
   * On cache hit, performs sqlite3_reset + sqlite3_clear_bindings before
   * handing the stmt to the caller. On miss, prepares a new statement.
   * On prepare failure, returns an empty AcquiredStmt (boolean-false). */
  AcquiredStmt Acquire(sqlite3* db, const char* sql);
  AcquiredStmt Acquire(sqlite3* db, const std::string& sql) {
    return Acquire(db, sql.c_str());
  }

  /* Finalize every cached statement. Must be called before the underlying
   * sqlite3* is closed. */
  void Clear() noexcept;

  std::size_t Size() const noexcept {
    std::lock_guard<std::mutex> lk(mutex_);
    return lru_.size();
  }

  std::size_t Capacity() const noexcept { return capacity_; }

 private:
  friend class AcquiredStmt;

  /* LRU entry: the ordering list contains <key, stmt> pairs, with the
   * front being most-recently-used. The map indexes into the list for
   * O(1) lookup. */
  using Entry = std::pair<std::string, sqlite3_stmt*>;
  using ListIter = std::list<Entry>::iterator;

  void ReinsertFront(const std::string& key, sqlite3_stmt* stmt) noexcept;
  void DropLRUIfOver() noexcept;

  mutable std::mutex mutex_;
  std::size_t capacity_;
  std::list<Entry> lru_;
  std::unordered_map<std::string, ListIter> index_;
};

}  // namespace notification

#endif  // NOTIFICATION_EX_STMT_CACHE_H_
