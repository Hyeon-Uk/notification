/*
 * Copyright (c) 2019 Samsung Electronics Co., Ltd.
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

#ifndef MOCK_SECURITY_MANAGER_H
#define MOCK_SECURITY_MANAGER_H

#include "mock.h"

#include <string>
#include <vector>

#ifdef __cplusplus
extern "C" {
#endif

struct private_sharing_req {
    std::string ownerAppName;
    std::string targetAppName;
    std::vector<std::string> paths;
};

DECLARE_FAKE_VALUE_FUNC(int, security_manager_private_sharing_req_new,
    private_sharing_req**);
DECLARE_FAKE_VALUE_FUNC(int, security_manager_private_sharing_req_add_paths,
    private_sharing_req*, const char**, size_t);
DECLARE_FAKE_VALUE_FUNC(int, security_manager_private_sharing_req_set_target_appid,
    private_sharing_req*, const char*);
DECLARE_FAKE_VALUE_FUNC(int, security_manager_private_sharing_apply,
    const private_sharing_req*);
DECLARE_FAKE_VALUE_FUNC(int, security_manager_private_sharing_drop,
    const private_sharing_req*);

#ifdef __cplusplus
}
#endif
#endif  /* MOCK_SECURITY_MANAGER_H */
