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

#ifndef NOTIFICATION_RAII_SQLITE3_STMT_HH_
#define NOTIFICATION_RAII_SQLITE3_STMT_HH_

#include <memory>
#include <sqlite3.h>

namespace notification::raii {

/*
 * Phase 3 RAII wrapper for sqlite3_stmt*.  Finalizes via
 * sqlite3_finalize on scope exit.
 *
 * Replaces the explicit
 *   if (stmt) sqlite3_finalize(stmt);
 * cleanup line that appears at the bottom of nearly every
 * EXPORT_API function in notification_db.c, _noti.c,
 * _setting_service.c, etc.
 */
struct Sqlite3StmtDeleter {
  void operator()(sqlite3_stmt *stmt) const noexcept {
    if (stmt != nullptr) sqlite3_finalize(stmt);
  }
};
using Sqlite3StmtPtr = std::unique_ptr<sqlite3_stmt, Sqlite3StmtDeleter>;

}  // namespace notification::raii

#endif  // NOTIFICATION_RAII_SQLITE3_STMT_HH_
