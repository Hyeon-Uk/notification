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

#ifndef NOTIFICATION_EX_IDB_MANAGER_H_
#define NOTIFICATION_EX_IDB_MANAGER_H_

#include <sys/types.h>

#include <list>
#include <map>
#include <memory>
#include <string>

#include "notification-ex/abstract_item.h"

#ifndef EXPORT_API
#define EXPORT_API __attribute__((visibility("default")))
#endif

/*
 * N1-T3: internal abstraction used to allow a connection-pool backed
 * implementation to sit behind the legacy static DBManager facade.
 *
 * Intentionally *not* EXPORT_API. External callers keep linking against
 * the pre-existing DBManager static methods; this interface is only
 * consumed by notification-ex internal code and unit tests.
 *
 * This header also documents the contract that the connection pool
 * satisfies:
 *   1. One long-lived sqlite3* per process (shared_ptr with
 *      sqlite3_close_v2 deleter).
 *   2. Serialized access via a mutex (SQLITE_THREADSAFE-friendly).
 *   3. Cache invalidation on SQLITE_CORRUPT / RecoverCorruptedDB.
 */

namespace notification {

class IDBManager {
 public:
  virtual ~IDBManager() = default;

  virtual int InitializeDB() = 0;
  virtual void InitializeData() = 0;
  virtual int InsertNotification(
      std::list<std::shared_ptr<item::AbstractItem>> addedList) = 0;
  virtual std::map<std::string, std::string> GetHideMap() = 0;
  virtual int UpdateHideList(std::shared_ptr<item::AbstractItem> updatedItem,
                             const std::string& hide_list) = 0;
  virtual int UpdateNotification(
      std::list<std::shared_ptr<item::AbstractItem>> updatedList) = 0;
  virtual int GetCount(int64_t priv_id, const std::string& root_id,
                       const std::string& app_id, const std::string& channel,
                       uid_t uid, int* count) = 0;
  virtual int GetCount(const std::string& app_id, const std::string& channel,
                       uid_t uid, int* count) = 0;
  virtual int DeleteNotification(
      std::list<std::shared_ptr<item::AbstractItem>> deletedList) = 0;
  virtual std::list<std::shared_ptr<item::AbstractItem>> GetNotificationList(
      uid_t uid, std::string channel = "") = 0;
  virtual std::list<std::shared_ptr<item::AbstractItem>> GetNotificationList(
      std::string app_id, uid_t uid, std::string channel = "") = 0;
  virtual std::list<std::shared_ptr<item::AbstractItem>> GetNotificationList(
      std::string app_id, std::string root_id, uid_t uid) = 0;
  virtual std::list<std::shared_ptr<item::AbstractItem>> GetNotificationList(
      std::string app_id, int64_t private_id, uid_t uid) = 0;
};

namespace detail {

/*
 * Test-only seam. Resets the cached long-lived sqlite3 connection held
 * by the default DBManagerImpl so each unit test can measure
 * sqlite3_open_v2 invocation counts from a clean baseline. Production
 * code must not rely on this symbol.
 *
 * N2-T5: EXPORT_API so the unittest binary resolves it under visibility=hidden.
 */
EXPORT_API void ResetDefaultConnectionForTest();

}  // namespace detail

}  // namespace notification

#endif  // NOTIFICATION_EX_IDB_MANAGER_H_
