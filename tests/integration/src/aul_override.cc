/*
 * Copyright (c) 2023 Samsung Electronics Co., Ltd. All rights reserved.
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

/*
 * PLT interposition for AUL functions.
 * Mirrors the technique in test_db_override.cc and tidl_override.cc.
 * Provides a fake app context so notification_create() succeeds in GBS chroot.
 */
#include <cstring>

extern "C" int aul_app_get_appid_bypid(int pid, char* appid, int len) {
  static const char kFakeAppId[] =
      "org.tizen.test.notification.integration";
  strncpy(appid, kFakeAppId, len - 1);
  appid[len - 1] = '\0';
  return 0;
}

extern "C" int aul_app_get_pkgname_bypid(int pid, char* pkg, int len) {
  static const char kFakePkg[] =
      "org.tizen.test.notification.integration";
  strncpy(pkg, kFakePkg, len - 1);
  pkg[len - 1] = '\0';
  return 0;
}

extern "C" int aul_app_get_pkgid_bypid(int pid, char* pkgid, int len) {
  static const char kFakePkgId[] =
      "org.tizen.test.notification.integration";
  strncpy(pkgid, kFakePkgId, len - 1);
  pkgid[len - 1] = '\0';
  return 0;
}

/*
 * PLT interposition for package_manager functions.
 * Defensive: uid < REGULAR_UID_MIN in GBS chroot so this path is
 * normally not reached, but guards against non-root test execution.
 */
struct package_info_s;
typedef struct package_info_s* package_info_h;

extern "C" int package_info_create(const char* /*package*/,
    package_info_h* package_info) {
  if (package_info)
    *package_info = nullptr;
  return 0;
}

extern "C" int package_info_get_root_path(package_info_h /*package_info*/,
    char** path) {
  if (path)
    *path = nullptr;
  return 0;
}

extern "C" int package_info_destroy(package_info_h /*package_info*/) {
  return 0;
}
