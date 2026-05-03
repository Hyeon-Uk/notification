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

#ifndef NOTIFICATION_RAII_SQLITE3_DB_HH_
#define NOTIFICATION_RAII_SQLITE3_DB_HH_

#include <memory>
#include <sqlite3.h>

namespace notification::raii {

/*
 * Phase 3 RAII wrapper for sqlite3*.  Closes via sqlite3_close on
 * scope exit, matching the legacy `notification_db_close(&db)` call
 * (which was itself a thin wrapper around sqlite3_close).
 *
 * Usage:
 *   Sqlite3DbPtr db(notification_db_open());
 *   if (!db) return get_last_result();
 *   // ... use db.get() with the C sqlite3 API ...
 *   // automatic close at scope exit; replaces the manual
 *   //   if (db) notification_db_close(&db);
 *   // pattern at the bottom of every notification_db.c function.
 */
struct Sqlite3DbDeleter {
  void operator()(sqlite3 *db) const noexcept {
    if (db != nullptr) sqlite3_close(db);
  }
};
using Sqlite3DbPtr = std::unique_ptr<sqlite3, Sqlite3DbDeleter>;

}  // namespace notification::raii

#endif  // NOTIFICATION_RAII_SQLITE3_DB_HH_
