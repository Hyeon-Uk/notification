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

#ifndef NOTIFICATION_EX_SHARED_FILE_H_
#define NOTIFICATION_EX_SHARED_FILE_H_

#include <security-manager.h>

#include <string>
#include <memory>
#include <list>
#include <map>
#include <vector>

#include "notification-ex/abstract_item.h"

#ifndef EXPORT_API
#define EXPORT_API __attribute__((visibility("default")))
#endif

namespace notification {
namespace item {

/* LCOV_EXCL_START */
class EXPORT_API SharedFile {
 public:
  SharedFile();
  virtual ~SharedFile();

  bool IsPrivatePath(std::string path) const;
  std::string GetDataPath(std::string app_id, std::string path) const;
  int SetPrivateSharing(std::list<std::shared_ptr<AbstractItem>> notiList,
          std::multimap<std::string, std::string> receiver_group_map);
  int UpdatePrivateSharing(std::list<std::shared_ptr<AbstractItem>> notiList,
          std::multimap<std::string, std::string> receiver_group_map);
  int RemovePrivateSharing(std::list<std::shared_ptr<AbstractItem>> notiList,
          std::multimap<std::string, std::string> receiver_group_map);
  static int CopyPrivateFile(std::shared_ptr<item::AbstractItem>added_item);

 private:
  class SharingData {
   public:
    SharingData();
    ~SharingData();
    std::string app_id;
    std::string dir;
    std::list<std::string> noti_id_list;
    std::list<std::string> shared_file_list;
    std::list<std::string> receiver_group_list;
  };
  class SharingTarget {
   public:
    SharingTarget();
    ~SharingTarget();
    std::string target_id;
  };
  static const char* GetLastIndex(const char* path, const char* search);
  static bool MakeDir(const char* path);
  static std::string GetDir(std::string path);
  static int CopyFile(const char* source, const char* dest);
  std::vector<char*> ConvertListToArray(const std::list<std::string>& data);
  SharingData FindSharingData(std::string appid);
  int SetSharingData(const SharingData& sharing_data,
          std::list<std::string> new_shared_file_list,
          std::list<std::string> new_receiver_group_list,
          std::multimap<std::string, std::string> receiver_group_map);
  int UnsetSharingData(const SharingData& sharing_data,
          std::multimap<std::string, std::string> receiver_group_map);

  std::list<SharingData> sharing_data_list_;
  std::list<SharingTarget> sharing_target_list_;
};  // class SharedFile
/* LCOV_EXCL_STOP */

}  // namespace item
}  // namespace notification

#endif  // NOTIFICATION_EX_SHARED_FILE_H_
