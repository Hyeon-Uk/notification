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
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <aul.h>
#include <unistd.h>
#include <dlog.h>
#include <linux/xattr.h>
#include <gio/gio.h>
#include <glib/gstdio.h>
#include <utime.h>
#include <tzplatform_config.h>
#include <security-manager.h>
#include <sys/statvfs.h>

#include <string>
#include <list>
#include <map>
#include <algorithm>
#include <vector>

#include "notification-ex/common.h"
#include "notification-ex/shared_file.h"
#include "notification-ex/group_item.h"

/*
 * N2-T6: shared_file_common.h is a plain C header declared under
 * src/notification/src/. It ships with "extern C" guards so including it
 * here is safe; the implementation lives in a CMake OBJECT library that
 * both libnotification.so and libnotification-ex.so pull in.
 */
extern "C" {
#include "shared_file_common.h"
}

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "NOTIFICATION_EX"
#define NOTI_PRIV_DATA_DIR "data/.notification_ex"
#define ERR_BUFFER_SIZE 1024

using namespace std;

namespace notification {
namespace item {
SharedFile::SharedFile() = default;
SharedFile::~SharedFile() = default;
SharedFile::SharingData::SharingData() = default;
SharedFile::SharingData::~SharingData() = default;
SharedFile::SharingTarget::SharingTarget() = default;
SharedFile::SharingTarget::~SharingTarget() = default;

// N2-T6: delegate to the C object library so both .so's emit a single
// copy of these helpers. Public SharedFile:: names stay unchanged so
// existing callers (and the header) keep their ABI.
const char* SharedFile::GetLastIndex(const char* path, const char* search) {
  return shared_file_common_last_index_of(path, search);
}

/* LCOV_EXCL_START */
bool SharedFile::MakeDir(const char* path) {
  return shared_file_common_make_dir(path);
}

string SharedFile::GetDir(string path) {
  int dir_len;
  const char* index;
  string dir;

  if (path.empty())
    return "";

  index = GetLastIndex(path.c_str(), "/");
  if (index == nullptr) {
    LOGE("Failed to find directory separator");
    return "";
  }

  dir_len = index - path.c_str() + 1;
  if (dir_len <= 0 || dir_len > PATH_MAX)
    return "";

  dir.reserve(PATH_MAX);
  snprintf(&dir[0], dir_len, "%s", path.c_str());

  return dir;
}

int SharedFile::CopyFile(const char* source, const char* dest) {
  // N2-T6: shared_file_common_copy_file returns the notification error enum
  // values, which are numerically identical to the ex ERROR_* constants,
  // so we can forward them as-is.
  return shared_file_common_copy_file(source, dest);
}

vector<char*> SharedFile::ConvertListToArray(const list<string>& data) {
  vector<char*> v;

  if (data.empty())
    return {};

  for (const auto& i : data)
    v.push_back(const_cast<char*>(i.c_str()));

  return v;
}

bool SharedFile::IsPrivatePath(string path) const {
  if (path.empty()) {
    LOGE("Invalid parameter");
    return false;
  }
  return shared_file_common_is_private_path(path.c_str());
}

string SharedFile::GetDataPath(string app_id, string path) const {
  if (app_id.empty() || path.empty()) {
    LOGE("Invalid parameter");
    return "";
  }
  // N2-T6: the C helper allocates the joined path; we adopt it into
  // std::string and free() the original buffer. Passing NOTI_PRIV_DATA_DIR
  // keeps the ex-specific subdir semantics ("data/.notification_ex").
  char* raw = shared_file_common_get_data_path(
      app_id.c_str(), path.c_str(), NOTI_PRIV_DATA_DIR);
  if (raw == nullptr)
    return "";
  string dir(raw);
  free(raw);
  return dir;
}

int SharedFile::SetSharingData(const SharingData& sharing_data,
    list<string> new_shared_file_list, list<string> new_receiver_group_list,
    multimap<string, string> receiver_group_map) {
  private_sharing_req* handle = nullptr;
  vector<char*> path_array;
  vector<char*> target_array;
  const char* appid = nullptr;
  int ret;

  if (new_shared_file_list.empty() || new_receiver_group_list.empty()) {
    LOGE("Invalid parameter");
    return ERROR_INVALID_PARAMETER;
  }

  ret = security_manager_private_sharing_req_new(&handle);
  if (ret != SECURITY_MANAGER_SUCCESS) {
    LOGE("Failed to create private sharing request handle[%d]", ret);
    return ERROR_IO_ERROR;
  }

  auto req = unique_ptr<private_sharing_req,
                decltype(&security_manager_private_sharing_req_free)>(
                handle, security_manager_private_sharing_req_free);
  if (req == nullptr)
    return ERROR_IO_ERROR;

  appid = sharing_data.app_id.c_str();

  ret = security_manager_private_sharing_req_set_owner_appid(req.get(), appid);
  if (ret != SECURITY_MANAGER_SUCCESS) {
    LOGE("Failed to set owner appid[%s][%d]", appid, ret);
    return ERROR_IO_ERROR;
  }

  path_array = ConvertListToArray(new_shared_file_list);
  if (path_array.empty()) {
    LOGE("path_array is null");
    return ERROR_IO_ERROR;
  }

  ret = security_manager_private_sharing_req_add_paths(req.get(),
            const_cast<const char**>(path_array.data()), path_array.size());
  if (ret != SECURITY_MANAGER_SUCCESS) {
    LOGE("Failed to add paths [%d]", ret);
    return ERROR_IO_ERROR;
  }

  target_array = ConvertListToArray(new_receiver_group_list);
  if (target_array.empty()) {
    LOGE("target_array is null");
    return ERROR_IO_ERROR;
  }

  for (int i = 0; i < static_cast<int>(target_array.size()); i++) {
    multimap<string, string>::iterator it;
    for (it = receiver_group_map.begin(); it != receiver_group_map.end(); it++) {
      if (it->first.compare(target_array[i]) == 0) {
        ret = security_manager_private_sharing_req_set_target_appid(req.get(),
                it->second.c_str());
        if (ret != SECURITY_MANAGER_SUCCESS) {
          LOGE("Failed to set target appid [%s]", appid);
          return ERROR_IO_ERROR;
        }

        ret = security_manager_private_sharing_apply(req.get());
        if (ret != SECURITY_MANAGER_SUCCESS) {
          LOGE("Failed to apply private sharing [%d]", ret);
          return ERROR_IO_ERROR;
        }
      }
    }
  }

  return ERROR_NONE;
}

int SharedFile::UnsetSharingData(const SharingData& sharing_data,
    multimap<string, string> receiver_group_map) {
  private_sharing_req* handle = nullptr;
  vector<char*> path_array;
  vector<char*> target_array;
  const char* appid = nullptr;
  int ret;

  if (receiver_group_map.empty()) {
    LOGE("Invalid parameter");
    return ERROR_INVALID_PARAMETER;
  }

  ret = security_manager_private_sharing_req_new(&handle);
  if (ret != SECURITY_MANAGER_SUCCESS) {
    LOGE("Failed to create private sharing request handle[%d]", ret);
    return ERROR_IO_ERROR;
  }

  auto req = unique_ptr<private_sharing_req,
                decltype(&security_manager_private_sharing_req_free)>(
                handle, security_manager_private_sharing_req_free);
  if (req == nullptr)
    return ERROR_IO_ERROR;

  appid = sharing_data.app_id.c_str();

  ret = security_manager_private_sharing_req_set_owner_appid(req.get(), appid);
  if (ret != SECURITY_MANAGER_SUCCESS) {
    LOGE("Failed to set owner appid[%s][%d]", appid, ret);
    return ERROR_IO_ERROR;
  }

  path_array = ConvertListToArray(sharing_data.shared_file_list);
  if (path_array.empty()) {
    LOGE("path_array is null");
    return ERROR_IO_ERROR;
  }

  ret = security_manager_private_sharing_req_add_paths(req.get(),
            const_cast<const char**>(path_array.data()), path_array.size());
  if (ret != SECURITY_MANAGER_SUCCESS) {
    LOGE("Failed to add paths [%d]", ret);
    return ERROR_IO_ERROR;
  }

  target_array = ConvertListToArray(sharing_data.receiver_group_list);
  if (target_array.empty()) {
    LOGE("target_array is null");
    return ERROR_IO_ERROR;
  }

  for (int i = 0; i < static_cast<int>(target_array.size()); i++) {
    multimap<string, string>::iterator it;
    for (it = receiver_group_map.begin(); it != receiver_group_map.end(); it++) {
      if (it->first.compare(target_array[i]) == 0) {
        ret = security_manager_private_sharing_req_set_target_appid(req.get(),
                it->second.c_str());
        if (ret != SECURITY_MANAGER_SUCCESS) {
          LOGE("Failed to set target appid [%s]", appid);
          return ERROR_IO_ERROR;
        }

        ret = security_manager_private_sharing_drop(req.get());
        if (ret != SECURITY_MANAGER_SUCCESS) {
          LOGE("Failed to drop private sharing [%d]", ret);
          return ERROR_IO_ERROR;
        }
      }
    }
  }

  return ERROR_NONE;
}

SharedFile::SharingData SharedFile::FindSharingData(string appid) {
  if (sharing_data_list_.size() == 0 || appid.empty())
    return {};

  for (auto sharing_data : sharing_data_list_) {
    if (sharing_data.app_id.compare(appid) == 0)
      return sharing_data;
  }

  return {};
}

int SharedFile::SetPrivateSharing(list<shared_ptr<AbstractItem>> notiList,
    multimap<string, string> receiver_group_map) {
  if (notiList.empty() || receiver_group_map.empty()) {
    LOGE("Invalid parameter");
    return ERROR_INVALID_PARAMETER;
  }

  for (auto& i : notiList) {
    SharingData sharing_data;
    list<string> new_shared_file_list;
    list<string> new_receiver_group_list;

    list<string> shared_path_list = i->GetSharedPath();
    if (shared_path_list.empty())
      continue;

    string appid = i->GetSenderAppId();
    if (appid.empty())
      continue;

    sharing_data = FindSharingData(appid);
    if (!sharing_data.app_id.empty()) {
      string noti_id = i->GetId();
      list<string>::iterator it;
      it = find_if(sharing_data.noti_id_list.begin(),
                    sharing_data.noti_id_list.end(),
                    [&noti_id](std::string id){
                      return id.compare(noti_id) == 0;
                    });
      if (it == sharing_data.noti_id_list.end())
        sharing_data.noti_id_list.push_back(noti_id);

      for (auto& shared_path : shared_path_list) {
        list<string>::iterator it;
        it = find_if(sharing_data.shared_file_list.begin(),
                    sharing_data.shared_file_list.end(),
                    [&shared_path](std::string shared_file){
                      return shared_file.compare(shared_path) == 0;
                    });
        if (it == sharing_data.shared_file_list.end()) {
          sharing_data.shared_file_list.push_back(shared_path);
          new_shared_file_list.push_back(shared_path);
        }
      }

      list<string> receiver_list = i->GetReceiverList();
      for (auto& receiver : receiver_list) {
        list<string>::iterator it;
        it = find_if(sharing_data.receiver_group_list.begin(),
                    sharing_data.receiver_group_list.end(),
                    [&receiver](std::string receiver_group){
                      return receiver_group.compare(receiver) == 0;
                    });
        if (it == sharing_data.receiver_group_list.end()) {
          sharing_data.receiver_group_list.push_back(receiver);
          new_receiver_group_list.push_back(receiver);
        }
      }
    } else {
      sharing_data.app_id = appid;
      sharing_data.dir = GetDir(*(shared_path_list.begin()));
      sharing_data.noti_id_list.emplace_back(i->GetId());
      sharing_data.shared_file_list = shared_path_list;
      sharing_data.receiver_group_list = i->GetReceiverList();
      sharing_data_list_.emplace_back(sharing_data);

      new_shared_file_list = shared_path_list;
      new_receiver_group_list = i->GetReceiverList();
    }

    if (SetSharingData(sharing_data, new_shared_file_list,
                new_receiver_group_list, receiver_group_map) == ERROR_NONE)
      i->SetSharedPath();
  }

  LOGD("SetPrivateSharing");
  return ERROR_NONE;
}

int SharedFile::UpdatePrivateSharing(list<shared_ptr<AbstractItem>> notiList,
    multimap<string, string> receiver_group_map) {
  if (notiList.empty() || receiver_group_map.empty()) {
    LOGE("Invalid parameter");
    return ERROR_INVALID_PARAMETER;
  }

  for (auto& i : notiList) {
    list<string> new_shared_file_list;
    list<string> new_receiver_group_list;
    list<string> shared_path_list = i->GetSharedPath();
    if (shared_path_list.empty())
      continue;

    string appid = i->GetSenderAppId();
    SharingData sharing_data = FindSharingData(appid);
    if (appid.empty() || sharing_data.app_id.empty())
      continue;

    for (auto& shared_path : shared_path_list) {
      std::list<std::string>::iterator it;
      it = std::find_if(sharing_data.shared_file_list.begin(),
                    sharing_data.shared_file_list.end(),
                    [&shared_path](std::string shared_file){
                      return shared_file.compare(shared_path) == 0;
                    });
      if (it == sharing_data.shared_file_list.end()) {
        sharing_data.shared_file_list.push_back(shared_path);
        new_shared_file_list.push_back(shared_path);
      }
    }

    new_receiver_group_list = i->GetReceiverList();
    if (SetSharingData(sharing_data, new_shared_file_list,
        new_receiver_group_list, receiver_group_map) == ERROR_NONE)
      i->SetSharedPath();
  }

  LOGD("UpdatePrivateSharing");
  return ERROR_NONE;
}

int SharedFile::RemovePrivateSharing(list<shared_ptr<AbstractItem>> notiList,
    multimap<string, string> receiver_group_map) {
  if (notiList.empty() || receiver_group_map.empty()) {
    LOGE("Invalid parameter");
    return ERROR_INVALID_PARAMETER;
  }

  for (auto& i : notiList) {
    string appid = i->GetSenderAppId();
    if (appid.empty())
      continue;

    SharingData sharing_data = FindSharingData(appid);
    if (sharing_data.app_id.empty())
      continue;

    sharing_data.noti_id_list.remove(i->GetId());
    if (sharing_data.noti_id_list.size() == 0) {
      UnsetSharingData(sharing_data, receiver_group_map);

      vector<char*> path_array = ConvertListToArray(sharing_data.shared_file_list);
      if (!path_array.empty()) {
        for (int i = 0; i < static_cast<int>(path_array.size()); i++) {
          if (g_remove(path_array[i]) == -1)
          LOGE("Failed to remove shared_file(%s)", path_array[i]);
        }
      }

      if (g_rmdir(sharing_data.dir.c_str()) != 0)
        LOGE("Failed to remove sharing directory(%s)(%d)",
            sharing_data.dir.c_str(), errno);
    }
  }

  LOGD("RemovePrivateSharing");
  return ERROR_NONE;
}

int SharedFile::CopyPrivateFile(shared_ptr<item::AbstractItem> item) {
  if (item == nullptr) {
    LOGE("Invalid paramenter");
    return ERROR_INVALID_PARAMETER;
  }

  list<map<string, string>> path_map_list = item->GetPathMapList();
  if (path_map_list.empty())
    return ERROR_NONE;

  if (!MakeDir(GetDir((*(path_map_list.begin())).begin()->second).c_str()))
    return ERROR_IO_ERROR;

  list<map<string, string>>::iterator list_it;
  for (list_it = path_map_list.begin(); list_it != path_map_list.end(); ++list_it) {
    map<string, string>::iterator map_it;
    for (map_it = (*list_it).begin(); map_it != (*list_it).end(); map_it++) {
      CopyFile((map_it->first).c_str(), (map_it->second).c_str());
    }
  }

  return ERROR_NONE;
}
/* LCOV_EXCL_STOP */

}  // namespace item
}  // namespace notification
