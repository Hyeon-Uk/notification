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

#include <gtest/gtest.h>

#include "mock/test_fixture.hh"

#include "notification/include/notification_internal.h"

namespace {
class Mocks : virtual public ModuleMock {};
}  // namespace

class NotificationErrorTest : public TestFixture {
 public:
  NotificationErrorTest() : TestFixture(std::make_unique<::Mocks>()) {}
};

TEST_F(NotificationErrorTest, ErrorQuarkRegistersDomain) {
  GQuark quark = notification_error_quark();
  EXPECT_NE(quark, 0u);
}

TEST_F(NotificationErrorTest, ErrorQuarkIsStableOnSubsequentCalls) {
  GQuark first = notification_error_quark();
  GQuark second = notification_error_quark();
  EXPECT_EQ(first, second);
}
