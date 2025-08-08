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

#ifndef UNIT_TESTS_MOCK_SECURITY_MANAGER_MOCK_H_
#define UNIT_TESTS_MOCK_SECURITY_MANAGER_MOCK_H_

#include <security-manager.h>
#include <gmock/gmock.h>

#include "module_mock.hh"

class SecurityManagerMock : public virtual ModuleMock {
 public:
  SecurityManagerMock() {
    using ::testing::_;
    using ::testing::Return;
    using ::testing::Invoke;
  }

  MOCK_METHOD1(security_manager_private_sharing_req_new, int (private_sharing_req**));
  MOCK_METHOD1(security_manager_private_sharing_apply, int (const private_sharing_req*));
  MOCK_METHOD1(security_manager_private_sharing_drop, int (const private_sharing_req*));
  MOCK_METHOD2(security_manager_private_sharing_req_set_target_appid,
      int (private_sharing_req*, const char*));
  MOCK_METHOD3(security_manager_private_sharing_req_add_paths,
      int (private_sharing_req*, const char**, size_t));
  MOCK_METHOD2(secutiy_manager_get_app_owner_uid, int (pid_t, uid_t*));
};

#endif  // UNIT_TESTS_MOCK_SECURITY_MANAGER_MOCK_H_
