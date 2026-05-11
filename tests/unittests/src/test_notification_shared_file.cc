/*
 * Copyright (c) 2023 Samsung Electronics Co., Ltd.
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
#include "mock/security_manager_mock.hh"
#include "mock/tzplatform_config_mock.hh"
#include "mock/package_manager_mock.hh"

#include "notification/include/notification.h"
#include "notification/include/notification_shared_file.h"

namespace {

typedef struct {
  const char *package;
  char *locale;
  pkgmgrinfo_app_component app_component;
  application_x *app_info;
} pkgmgr_appinfo_x;

int __fake_aul_app_get_appid_bypid(int pid, char* appid, int len) {
  snprintf(appid, len, "%s", "org.tizen.testappid");
  return AUL_R_OK;
}

int __fake_aul_app_get_pkgid_bypid(int pid, char* pkgid, int len) {
  snprintf(pkgid, len, "%s", "org.tizen.testpkgid");
  return AUL_R_OK;
}

int __fake_pkgmgrinfo_appinfo_get_usr_appinfo(const char* appid, uid_t uid,
    pkgmgrinfo_appinfo_h *handle) {
  *handle = (pkgmgrinfo_appinfo_h*)calloc(1, sizeof(pkgmgr_appinfo_x));
  return PMINFO_R_OK;
}

int __fake_pkgmgrinfo_appinfo_get_label(pkgmgrinfo_appinfo_h handle, char** label) {
  *label = strdup("testlabel");
  return PMINFO_R_OK;
}

class Mocks :
    virtual public ::testing::NiceMock<AulMock>,
    virtual public ::testing::NiceMock<GlibMock>,
    virtual public ::testing::NiceMock<SecurityManagerMock>,
    virtual public ::testing::NiceMock<PackageManagerMock>,
    virtual public ::testing::NiceMock<TzplatformConfigMock> {};
} // namespace

using ::testing::_;
using ::testing::Invoke;
using ::testing::Return;

class NotificationSharedFileTest : public TestFixture {
 public:
  NotificationSharedFileTest() : TestFixture(std::make_unique<::Mocks>()) {}
  virtual void SetUp() {
    EXPECT_CALL(GetMock<AulMock>(), aul_app_get_appid_bypid(_, _, _))
        .WillRepeatedly(Invoke(__fake_aul_app_get_appid_bypid));
    EXPECT_CALL(GetMock<AulMock>(), aul_app_get_pkgid_bypid(_, _, _))
        .WillRepeatedly(Invoke(__fake_aul_app_get_pkgid_bypid));
    EXPECT_CALL(GetMock<PackageManagerMock>(), pkgmgrinfo_appinfo_get_usr_appinfo(_, _, _))
        .WillRepeatedly(Invoke(__fake_pkgmgrinfo_appinfo_get_usr_appinfo));
    EXPECT_CALL(GetMock<PackageManagerMock>(), pkgmgrinfo_appinfo_get_label(_, _))
        .WillRepeatedly(Invoke(__fake_pkgmgrinfo_appinfo_get_label));
  }
  virtual void TearDown() {}
};

TEST_F(NotificationSharedFileTest, notification_validate_no_private_files) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  bool result = notification_validate_private_sharing(noti);
  EXPECT_TRUE(result);

  notification_free(noti);
}

TEST_F(NotificationSharedFileTest, notification_set_private_sharing_empty_noti) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  int ret = notification_set_private_sharing(noti, 5001);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  notification_free(noti);
}

TEST_F(NotificationSharedFileTest, notification_calibrate_private_sharing_valid) {
  notification_h noti1 = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti1, nullptr);

  notification_h noti2 = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti2, nullptr);

  notification_calibrate_private_sharing(noti1, noti2);

  notification_free(noti1);
  notification_free(noti2);
}
