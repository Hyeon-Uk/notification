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

#include "security_manager_mock.hh"

#include "mock_hook.hh"
#include "test_fixture.hh"

extern "C" int security_manager_private_sharing_req_new(private_sharing_req** arg1) {
  return MOCK_HOOK_P1(SecurityManagerMock, security_manager_private_sharing_req_new, arg1);
}

extern "C" int security_manager_private_sharing_apply(const private_sharing_req* arg1) {
  return MOCK_HOOK_P1(SecurityManagerMock, security_manager_private_sharing_apply, arg1);
}

extern "C" int security_manager_private_sharing_drop(const private_sharing_req* arg1) {
  return MOCK_HOOK_P1(SecurityManagerMock, security_manager_private_sharing_apply, arg1);
}

extern "C" int security_manager_private_sharing_req_set_target_appid(private_sharing_req* arg1,
    const char* arg2) {
  return MOCK_HOOK_P2(SecurityManagerMock,
      security_manager_private_sharing_req_set_target_appid, arg1, arg2);
}

extern "C" int security_manager_private_sharing_req_add_paths(private_sharing_req* arg1,
    const char** arg2, size_t arg3) {
  return MOCK_HOOK_P3(SecurityManagerMock,
      security_manager_private_sharing_req_add_paths, arg1, arg2, arg3);
}