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

#include "aul_mock.hh"

#include <stdexcept>

#include "mock_hook.hh"
#include "test_fixture.hh"

extern "C" uid_t aul_getuid() {
  try {
    return MOCK_HOOK_P0(AulMock, aul_getuid);
  } catch (const std::invalid_argument&) {
    return 5001;
  }
}

extern "C" int aul_app_get_appid_bypid(int pid, char* appid, int len) {
  return MOCK_HOOK_P3(AulMock, aul_app_get_appid_bypid, pid, appid, len);
}

extern "C" int aul_app_get_pkgid_bypid(int pid, char* pkgid, int len) {
  return MOCK_HOOK_P3(AulMock, aul_app_get_pkgid_bypid, pid, pkgid, len);
}
