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

#ifndef UNIT_TESTS_MOCK_PACKAGE_MANAGER_MOCK_H_
#define UNIT_TESTS_MOCK_PACKAGE_MANAGER_MOCK_H_

#include <gmock/gmock.h>
#include <package_manager.h>
#include <pkgmgr-info.h>
#include <pkgmgrinfo_basic.h>

#include "module_mock.hh"

class PackageManagerMock : public virtual ModuleMock {
 public:
  virtual ~PackageManagerMock() {}

  MOCK_METHOD2(package_info_create, int(const char*, package_info_h*));
  MOCK_METHOD2(package_info_get_root_path, int(package_info_h, char**));
  MOCK_METHOD3(pkgmgrinfo_appinfo_get_usr_appinfo, int(const char*, uid_t, pkgmgrinfo_appinfo_h*));
  MOCK_METHOD2(pkgmgrinfo_appinfo_get_label, int(pkgmgrinfo_appinfo_h, char**));
  MOCK_METHOD2(pkgmgrinfo_appinfo_get_pkgid, int(pkgmgrinfo_appinfo_h, char**));
  MOCK_METHOD1(pkgmgrinfo_appinfo_destroy_appinfo, int(pkgmgrinfo_appinfo_h));
  MOCK_METHOD1(pkgmgrinfo_appinfo_filter_create, int(pkgmgrinfo_appinfo_filter_h*));
  MOCK_METHOD3(pkgmgrinfo_appinfo_filter_add_string, int(pkgmgrinfo_appinfo_filter_h, const char*, const char*));
  MOCK_METHOD4(pkgmgrinfo_appinfo_usr_filter_foreach_appinfo, int(pkgmgrinfo_appinfo_filter_h, pkgmgrinfo_app_list_cb, void*, uid_t));
  MOCK_METHOD1(pkgmgrinfo_appinfo_filter_destroy, int(pkgmgrinfo_appinfo_filter_h));
  MOCK_METHOD2(pkgmgrinfo_appinfo_get_appid, int(pkgmgrinfo_appinfo_h, char**));
  MOCK_METHOD2(pkgmgrinfo_appinfo_get_pkgname, int(pkgmgrinfo_appinfo_h, char**));
};

#endif  // UNIT_TESTS_MOCK_PACKAGE_MANAGER_MOCK_H_
