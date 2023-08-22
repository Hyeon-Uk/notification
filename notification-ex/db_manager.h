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

#ifndef NOTIFICATION_EX_DB_MANAGER_H_
#define NOTIFICATION_EX_DB_MANAGER_H_

#include <sqlite3.h>

#include <string>
#include <memory>
#include <map>
#include <list>

#include "notification-ex/abstract_item.h"

#ifndef EXPORT_API
#define EXPORT_API __attribute__((visibility("default")))
#endif

namespace notification {

/* LCOV_EXCL_START */
class EXPORT_API DBManager {
 public:
  static int InitializeDB();
  static void InitializeData();
  static int InsertNotification(std::list<std::shared_ptr<item::AbstractItem>> addedList);
  static std::map<std::string, std::string> GetHideMap();
  static int UpdateHideList(std::shared_ptr<item::AbstractItem> updatedItem, const std::string& hide_list);
  static int UpdateNotification(std::list<std::shared_ptr<item::AbstractItem>> updatedList);
  static int GetCount(int64_t priv_id, const std::string& root_id,
                  const std::string& app_id, const std::string& channel, uid_t uid, int* count);
  static int GetCount(const std::string& app_id, const std::string& channel,
                  uid_t uid, int* count);
  static int DeleteNotification(std::list<std::shared_ptr<item::AbstractItem>> deletedList);
  static std::list<std::shared_ptr<item::AbstractItem>> GetNotificationList(uid_t uid, std::string channel = "");
  static std::list<std::shared_ptr<item::AbstractItem>> GetNotificationList(std::string app_id, uid_t uid, std::string channel = "");
  static std::list<std::shared_ptr<item::AbstractItem>> GetNotificationList(std::string app_id, std::string root_id, uid_t uid);
  static std::list<std::shared_ptr<item::AbstractItem>> GetNotificationList(std::string app_id, int64_t private_id, uid_t uid);

 private:
  DBManager(); /* LCOV_EXCL_LINE */
  ~DBManager(); /* LCOV_EXCL_LINE */
  static sqlite3* OpenDB();
  static void CloseDB(sqlite3* db);
  static int CheckDBIntegrity(void* user_data, int argc, char** argv, char** notUsed);
  static int RecoverCorruptedDB(sqlite3* db);
  static int ExecuteQuery(const char* query, int* num_changes);
  static int ExecuteQuery(sqlite3* db, const char* query, int* num_changes);
  static int GetSequence(sqlite3* db, int64_t* seq);
  static std::string GetPkgId(const std::string& app_id, uid_t uid);
  static void CheckLimit(std::shared_ptr<item::AbstractItem> addedItem, sqlite3* db);
  static int UpdateReceiverList(std::shared_ptr<item::AbstractItem> updatedItem,  sqlite3* db);
  static std::list<std::shared_ptr<item::AbstractItem>> ExecuteGetList(char* query);
};
/* LCOV_EXCL_STOP */

}  // namespace notification

#endif  // NOTIFICATION_EX_DB_MANAGER_H_
