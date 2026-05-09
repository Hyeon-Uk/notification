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
#include <gmock/gmock.h>

#include "mock/test_fixture.hh"
#include "mock/aul_mock.hh"
#include "mock/glib_mock.hh"

#include "notification/include/notification.h"
#include "notification/include/notification_internal.h"
#include "notification/include/notification_text_domain.h"

namespace {
class Mocks : virtual public ::testing::NiceMock<AulMock>,
              virtual public ::testing::NiceMock<GlibMock> {};
constexpr int kInvalid = NOTIFICATION_ERROR_INVALID_PARAMETER;
constexpr int kNone = NOTIFICATION_ERROR_NONE;
}  // namespace

class NotificationTextDomainTest : public TestFixture {
 public:
  NotificationTextDomainTest() : TestFixture(std::make_unique<::Mocks>()) {}
};

TEST_F(NotificationTextDomainTest, set_get_text_domain_P) {
  notification_h n = notification_new(NOTIFICATION_TYPE_NOTI, 1, 0);
  if (n == nullptr) GTEST_SKIP() << "notification_new unavailable";
  EXPECT_EQ(notification_set_text_domain(n, "domain", "/usr/share/locale"),
            kNone);
  char *d = nullptr; char *dir = nullptr;
  EXPECT_EQ(notification_get_text_domain(n, &d, &dir), kNone);
  EXPECT_STREQ(d, "domain");
  EXPECT_STREQ(dir, "/usr/share/locale");
  notification_free(n);
}

TEST_F(NotificationTextDomainTest, set_text_domain_N_null_handle) {
  EXPECT_EQ(notification_set_text_domain(nullptr, "d", "/locale"), kInvalid);
}

TEST_F(NotificationTextDomainTest, set_text_domain_N_null_domain) {
  notification_h n = notification_new(NOTIFICATION_TYPE_NOTI, 1, 0);
  if (n == nullptr) GTEST_SKIP() << "notification_new unavailable";
  int r = notification_set_text_domain(n, nullptr, "/locale");
  EXPECT_EQ(r, kInvalid);
  notification_free(n);
}

TEST_F(NotificationTextDomainTest, get_text_domain_N_null_handle) {
  char *d = nullptr; char *dir = nullptr;
  EXPECT_EQ(notification_get_text_domain(nullptr, &d, &dir), kInvalid);
}

TEST_F(NotificationTextDomainTest, get_text_domain_N_null_outs) {
  EXPECT_EQ(notification_get_text_domain(nullptr, nullptr, nullptr), kInvalid);
}
