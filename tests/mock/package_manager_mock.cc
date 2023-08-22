/*
 * Copyright (c) 2022 Samsung Electronics Co., Ltd.
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

#include "package_manager_mock.hh"

#include "mock_hook.hh"
#include "test_fixture.hh"

extern "C" int package_info_create(const char *package, package_info_h *package_info) {
  return MOCK_HOOK_P2(PackageManagerMock, package_info_create, package, package_info);
}

extern "C" int package_info_get_root_path(package_info_h package_info, char **path) {
  return MOCK_HOOK_P2(PackageManagerMock, package_info_get_root_path, package_info, path);
}

extern "C" int pkgmgrinfo_appinfo_get_usr_appinfo(const char *appid,
    uid_t uid, pkgmgrinfo_appinfo_h *handle) {
  return MOCK_HOOK_P3(PackageManagerMock, pkgmgrinfo_appinfo_get_usr_appinfo, appid, uid, handle);
}

extern "C" int pkgmgrinfo_appinfo_get_label(pkgmgrinfo_appinfo_h handle, char **label) {
  return MOCK_HOOK_P2(PackageManagerMock, pkgmgrinfo_appinfo_get_label, handle, label);
}
