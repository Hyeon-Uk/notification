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

#ifndef UNIT_TESTS_MOCK_BUNDLE_MOCK_H_
#define UNIT_TESTS_MOCK_BUNDLE_MOCK_H_

#include <gmock/gmock.h>
#include <bundle.h>

#include "module_mock.hh"

class BundleMock : public virtual ModuleMock {
 public:
  virtual ~BundleMock() {}
  MOCK_METHOD3(bundle_encode, int(bundle*, bundle_raw**, int*));
  MOCK_METHOD3(bundle_add_str, int(bundle*, const char*, const char*));
};

// bundle_create OOM injection helpers (not a GMock method — call-counter based)
void bundle_create_set_fail_on(int call_index);  // fail on Nth call (0-indexed); resets counter
void bundle_create_reset_fail();                  // disable; resets counter

#endif  // UNIT_TESTS_MOCK_BUNDLE_MOCK_H_
