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
#include <aul.h>
#include <fcntl.h>
#include <unistd.h>

#include <string>

#include "notification-ex/ex_util.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "NOTIFICATION_EX"
#define MAX_PACKAGE_STR_SIZE 512

using namespace std;
namespace notification {
namespace util {
  GQuark GetQuarkFromString(string str) {
    return g_quark_from_string(str.c_str());
  }

  std::string GetQuarkToString(GQuark quark) {
    return g_quark_to_string(quark);
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
}  // namespace util
}  // namespace watchface_complication
