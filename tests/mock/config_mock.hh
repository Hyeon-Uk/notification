/*
 * Copyright (c) 2026 Samsung Electronics Co., Ltd.
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

#ifndef UNIT_TESTS_MOCK_CONFIG_MOCK_H_
#define UNIT_TESTS_MOCK_CONFIG_MOCK_H_

#include <gmock/gmock.h>

extern "C" {
#include "notification/src/config.h"
}

#include "module_mock.hh"

class ConfigMock : public virtual ModuleMock {
 public:
  virtual ~ConfigMock() {}
  MOCK_METHOD(tizen_profile_t, _config_get_tizen_profile, (), ());
};

#endif  // UNIT_TESTS_MOCK_CONFIG_MOCK_H_
