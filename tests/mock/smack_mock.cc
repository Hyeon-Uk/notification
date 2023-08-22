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

#include "smack_mock.hh"

#include "mock_hook.hh"
#include "test_fixture.hh"

extern "C" ssize_t smack_new_label_from_path(const char* arg1,
    const char* arg2, int arg3, char** arg4) {
  return MOCK_HOOK_P4(SmackMock, smack_new_label_from_path, arg1, arg2, arg3, arg4);
}
