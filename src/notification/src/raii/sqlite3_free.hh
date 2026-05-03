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

#ifndef NOTIFICATION_RAII_SQLITE3_FREE_HH_
#define NOTIFICATION_RAII_SQLITE3_FREE_HH_

#include <memory>
#include <sqlite3.h>

namespace notification::raii {

/*
 * Phase 3 RAII wrapper for char* / void* allocations owned by
 * sqlite3 (sqlite3_mprintf, sqlite3_exec's errmsg out-param, etc.).
 * Frees via sqlite3_free on scope exit.
 *
 * Replaces the explicit
 *   if (query) sqlite3_free(query);
 *   if (errmsg) sqlite3_free(errmsg);
 * cleanup lines.
 *
 * Type-erased on void* under the hood; convenience aliases below.
 */
struct Sqlite3FreeDeleter {
  void operator()(void *p) const noexcept {
    if (p != nullptr) sqlite3_free(p);
  }
};

using Sqlite3CharPtr = std::unique_ptr<char, Sqlite3FreeDeleter>;

}  // namespace notification::raii

#endif  // NOTIFICATION_RAII_SQLITE3_FREE_HH_
