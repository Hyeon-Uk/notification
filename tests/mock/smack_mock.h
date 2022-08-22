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

#ifndef UNIT_TESTS_MOCK_SMACK_MOCK_H_
#define UNIT_TESTS_MOCK_SMACK_MOCK_H_

#include <gmock/gmock.h>
#include <sys/smack.h>

#include "module_mock.h"

class SmackMock : public virtual ModuleMock {
 public:
  SmackMock() {
    using ::testing::_;
    using ::testing::Return;
    using ::testing::Invoke;
  }

  MOCK_METHOD4(smack_new_label_from_path, ssize_t (const char*,
    const char*, int, char**));
};

#endif  // UNIT_TESTS_MOCK_SMACK_MOCK_H_