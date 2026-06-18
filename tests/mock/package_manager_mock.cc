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

extern "C" int pkgmgrinfo_appinfo_get_pkgid(pkgmgrinfo_appinfo_h handle, char **pkgid) {
  return MOCK_HOOK_P2(PackageManagerMock, pkgmgrinfo_appinfo_get_pkgid, handle, pkgid);
}

extern "C" int pkgmgrinfo_appinfo_destroy_appinfo(pkgmgrinfo_appinfo_h handle) {
  return MOCK_HOOK_P1(PackageManagerMock, pkgmgrinfo_appinfo_destroy_appinfo, handle);
}

extern "C" int pkgmgrinfo_appinfo_filter_create(pkgmgrinfo_appinfo_filter_h *handle) {
  return MOCK_HOOK_P1(PackageManagerMock, pkgmgrinfo_appinfo_filter_create, handle);
}

extern "C" int pkgmgrinfo_appinfo_filter_add_string(pkgmgrinfo_appinfo_filter_h handle,
    const char *property, const char *value) {
  return MOCK_HOOK_P3(PackageManagerMock, pkgmgrinfo_appinfo_filter_add_string, handle, property, value);
}

extern "C" int pkgmgrinfo_appinfo_usr_filter_foreach_appinfo(pkgmgrinfo_appinfo_filter_h handle,
    pkgmgrinfo_app_list_cb app_func, void *user_data, uid_t uid) {
  return MOCK_HOOK_P4(PackageManagerMock, pkgmgrinfo_appinfo_usr_filter_foreach_appinfo, handle, app_func, user_data, uid);
}

extern "C" int pkgmgrinfo_appinfo_filter_destroy(pkgmgrinfo_appinfo_filter_h handle) {
  return MOCK_HOOK_P1(PackageManagerMock, pkgmgrinfo_appinfo_filter_destroy, handle);
}

extern "C" int pkgmgrinfo_appinfo_get_appid(pkgmgrinfo_appinfo_h handle, char **appid) {
  return MOCK_HOOK_P2(PackageManagerMock, pkgmgrinfo_appinfo_get_appid, handle, appid);
}

extern "C" int pkgmgrinfo_appinfo_get_pkgname(pkgmgrinfo_appinfo_h handle,
    char **pkg_name) {
  return MOCK_HOOK_P2(PackageManagerMock, pkgmgrinfo_appinfo_get_pkgname, handle, pkg_name);
}
