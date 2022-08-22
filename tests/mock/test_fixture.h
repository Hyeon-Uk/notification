/*
 * Copyright (c) 2020 Samsung Electronics Co., Ltd.
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

#ifndef UNIT_TESTS_MOCK_TEST_FIXTURE_H_
#define UNIT_TESTS_MOCK_TEST_FIXTURE_H_

#include <gtest/gtest.h>

#include <memory>
#include <stdexcept>
#include <string>
#include <utility>

#include "module_mock.h"

class TestFixture : public ::testing::Test {
 public:
  explicit TestFixture(std::unique_ptr<ModuleMock>&& mock) {
    mock_ = std::move(mock);
  }
  virtual ~TestFixture() {
    mock_.reset();
  }

  virtual void SetUp() {}
  virtual void TearDown() {}

  template <typename T>
  static T& GetMock() {
    auto ptr = dynamic_cast<T*>(mock_.get());
    if (!ptr)
      throw std::invalid_argument("The test does not provide mock of \"" +
          std::string(typeid(T).name()) + "\"");
    return *ptr;
  }

  static std::unique_ptr<ModuleMock> mock_;
};

#endif  // UNIT_TESTS_MOCK_TEST_FIXTURE_H_
