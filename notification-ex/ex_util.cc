/*
 * Copyright (c) 2018 Samsung Electronics Co., Ltd.
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

#include <dlog.h>
#include <aul.h>
#include <fcntl.h>
#include <unistd.h>
#include <package_info.h>
#include <package_manager.h>

#include <string>
#include <memory>

#include "notification-ex/ex_util.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "NOTIFICATION_EX"
#define MAX_PACKAGE_STR_SIZE 512

using namespace std;
namespace notification {
namespace util {

int GetRequestId() {
  static int id = 0;
  g_atomic_int_inc(&id);
  return id;
}

string GetAppId() {
  static string appid = "";
  char appid_buf[MAX_PACKAGE_STR_SIZE] = {0, };

  if (!appid.empty()) {
    LOGI("appid(%s)", appid.c_str());
    return appid;
  }

  int pid = getpid();
  int ret = aul_app_get_appid_bypid(pid, appid_buf, sizeof(appid_buf));
  if (ret == AUL_R_OK) {
    appid = string(appid_buf);
  } else {
    int fd, i;
    int last_slash_index = 0;
    char proc_buf[MAX_PACKAGE_STR_SIZE] = { 0, };

    snprintf(proc_buf, sizeof(proc_buf), "/proc/%d/cmdline", pid);

    fd = open(proc_buf, O_RDONLY);
    if (fd < 0) {
      LOGE("Fail to get appid (%d)", errno);
      return "";
    }

    ret = read(fd, appid_buf, sizeof(appid_buf) - 1);
    if (ret <= 0) {
      LOGE("Fail to get appid (%d)", errno);
      close(fd);
      return "";
    }
    close(fd);

    for (i = 0 ; i < ret ; i++) {
      if (appid_buf[i] == '/')
        last_slash_index = i;
    }

    if (last_slash_index == (ret - 1)) {
      LOGE("Fail to get appid (%s)", appid_buf);
      return "";
    }

    if (last_slash_index == 0)
      appid = string(appid_buf);
    else
      appid = string(&appid_buf[last_slash_index + 1]);
  }

  LOGI("appid(%s)", appid.c_str());
  return appid;
}

string GetPkgId() {
  static string pkgid = "";
  if (!pkgid.empty()) {
    LOGI("pkgid(%s)", pkgid.c_str());
    return pkgid;
  }

  char pkgid_buf[MAX_PACKAGE_STR_SIZE] = {0, };
  int ret = aul_app_get_pkgid_bypid(getpid(), pkgid_buf, sizeof(pkgid_buf));
  if (ret == AUL_R_OK)
    pkgid = string(pkgid_buf);

  return pkgid;
}

string GetDomainName() {
  static string domain_str = "";
  if (!domain_str.empty()) {
    LOGI("domain(%s)", domain_str.c_str());
    return domain_str;
  }

  string pkgid = GetPkgId();
  if (pkgid.empty())
    return "";

  string::size_type pos = pkgid.rfind('.');
  if (pos != string::npos)
    ++pos;
  else
    return "";

  domain_str = pkgid.substr(pos);
  return domain_str;
}

string GetLocaleDirectory() {
  string pkgid = GetPkgId();
  if (pkgid.empty())
    return "";

  static string locale_directory = "";
  if (!locale_directory.empty()) {
    LOGI("locale_directory(%s)", locale_directory.c_str());
    return locale_directory;
  }

  package_info_h package_info;
  int err = package_info_create(pkgid.c_str(), &package_info);
  if (err != PACKAGE_MANAGER_ERROR_NONE) {
    LOGE("fail to get package info");
    return "";
  }
  unique_ptr<package_info_s, decltype(package_info_destroy)*> pkg_ptr(
      package_info, package_info_destroy);

  char* app_root_path;
  err = package_info_get_root_path(pkg_ptr.get(), &app_root_path);
  if (err != PACKAGE_MANAGER_ERROR_NONE) {
    LOGE("Failed to get root path err[%d] path[%p]",
        err, app_root_path);
    return "";
  }

  string app_root_str(app_root_path);
  free(app_root_path);

  locale_directory = app_root_str + string("/res/locale");
  return locale_directory;
}

}  // namespace util
}  // namespace notification
