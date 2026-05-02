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

/*
 * notification-smoke — Public-API contract sanity check.
 *
 * The unittests target links libnotification and libnotification-ex.
 * This binary links ONLY libnotification; it is the seed for the
 * Phase-1 task 1.17 contract test suite.
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "mock/test_fixture.hh"
#include "mock/aul_mock.hh"
#include "mock/package_manager_mock.hh"
#include "mock/tzplatform_config_mock.hh"
#include "mock/glib_mock.hh"

#include "notification/include/notification.h"

namespace {

int __fake_aul_app_get_appid_bypid(int pid, char* appid, int len) {
  snprintf(appid, len, "%s", "org.tizen.smoketestappid");
  return AUL_R_OK;
}

int __fake_aul_app_get_pkgid_bypid(int pid, char* pkgid, int len) {
  snprintf(pkgid, len, "%s", "org.tizen.smoketestpkgid");
  return AUL_R_OK;
}

class Mocks :
    virtual public ::testing::NiceMock<AulMock>,
    virtual public ::testing::NiceMock<TzplatformConfigMock>,
    virtual public ::testing::NiceMock<GlibMock>,
    virtual public ::testing::NiceMock<PackageManagerMock> {};

}  // namespace

using ::testing::_;
using ::testing::Invoke;

class SmokeFixture : public TestFixture {
 public:
  SmokeFixture() : TestFixture(std::make_unique<::Mocks>()) {}
};

TEST_F(SmokeFixture, NotificationCreateAndFreeRoundTrip) {
  EXPECT_CALL(GetMock<AulMock>(), aul_app_get_appid_bypid(_, _, _))
      .WillRepeatedly(Invoke(__fake_aul_app_get_appid_bypid));
  EXPECT_CALL(GetMock<AulMock>(), aul_app_get_pkgid_bypid(_, _, _))
      .WillRepeatedly(Invoke(__fake_aul_app_get_pkgid_bypid));

  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);
  EXPECT_EQ(notification_free(noti), NOTIFICATION_ERROR_NONE);
}

TEST_F(SmokeFixture, NotificationCreateInvalidTypeReturnsNull) {
  notification_h noti = notification_create(
      static_cast<notification_type_e>(-1));
  EXPECT_EQ(noti, nullptr);
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
