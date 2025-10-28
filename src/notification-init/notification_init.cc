/*
 * Copyright (c) 2016 - 2025 Samsung Electronics Co., Ltd. All rights reserved.
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
 *
 */

#include <aul.h>
#include <dirent.h>
#include <errno.h>
#include <notification_setting.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <tzplatform_config.h>
#include <unistd.h>

#define OWNER_ROOT 0

#undef _E
#define _E(fmt, arg...)                                                \
  fprintf(stderr, "[NOTIFICATION_INIT][E][%s,%d] " fmt "\n", __func__, \
          __LINE__, ##arg)

static bool IsAuthorized(void) {
  if (aul_getuid() == OWNER_ROOT)
    return true;

  return false;
}

int main(int argc, char** argv) {
  if (!IsAuthorized()) {
    _E("Not an authorized user");
    return -1;
  }

  uid_t uid = 0;
  if (argc > 2)
    uid = static_cast<uid_t>(atoi(argv[2]));

  int ret = notification_setting_refresh_setting_table(uid);
  if (ret != NOTIFICATION_ERROR_NONE) {
    _E("Failed to refresh setting table");
    return ret;
  }

  ret = notification_system_setting_init_system_setting_table(uid);
  if (ret != NOTIFICATION_ERROR_NONE) {
    _E("Failed to init system setting table");
    return ret;
  }

  return ret;
}
