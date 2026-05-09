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
#include <glib.h>

#include "mock/test_fixture.hh"
#include "mock/aul_mock.hh"
#include "mock/glib_mock.hh"
#include "mock/package_manager_mock.hh"
#include "mock/tzplatform_config_mock.hh"

#include "notification/include/notification.h"
#include "notification/include/notification_ipc.h"

namespace {

class Mocks : virtual public ::testing::NiceMock<AulMock>,
              virtual public ::testing::NiceMock<TzplatformConfigMock>,
              virtual public ::testing::NiceMock<GlibMock>,
              virtual public ::testing::NiceMock<PackageManagerMock> {};

}  // namespace

class NotificationIpcTest : public TestFixture {
 public:
  NotificationIpcTest() : TestFixture(std::make_unique<::Mocks>()) {}
};

TEST_F(NotificationIpcTest, make_noti_from_gvariant_N_null_variant) {
  notification_h n = notification_create(NOTIFICATION_TYPE_NOTI);
  if (n == nullptr) {
    GTEST_SKIP() << "notification_create unavailable in this fixture; "
                    "round-trip happy path lives in the integration suite.";
  }
  EXPECT_EQ(notification_ipc_make_noti_from_gvariant(n, nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  notification_free(n);
}

TEST_F(NotificationIpcTest, make_gvariant_from_noti_P_returns_nonnull) {
  notification_h n = notification_create(NOTIFICATION_TYPE_NOTI);
  if (n == nullptr) {
    GTEST_SKIP() << "notification_create unavailable in this fixture.";
  }
  GVariant *v = notification_ipc_make_gvariant_from_noti(n, false);
  EXPECT_NE(v, nullptr);
  if (v != nullptr) g_variant_unref(v);
  notification_free(n);
}
