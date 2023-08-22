/*
 * Copyright (c) 2019 Samsung Electronics Co., Ltd.
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

#include "mock/app_common_mock.hh"
#include "mock/test_fixture.hh"
#include "notification-ex/shared_file.h"
#include "notification-ex/image_item.h"
#include "notification-ex/common.h"

#include "mock/glib_mock.hh"
#include "mock/smack_mock.hh"
#include "mock/tzplatform_config_mock.hh"
#include "mock/security_manager_mock.hh"
#include "mock/aul_mock.hh"


using namespace notification;
using namespace notification::item;
using namespace std;

using ::testing::_;
using ::testing::Invoke;

namespace {

// smack
ssize_t __fake_smack_new_label_from_path(const char* path, const char* xattr,
    int follow, char** label) {
  *label = strdup("User::Pkg::unittest::RO");
  return 1;
}

// tzplatform
const char* __fake_tzplatform_getenv(tzplatform_variable id) {
  return "/opt/usr/home/owner/apps_rw";
}

// security-manager
int __fake_security_manager_private_sharing_req_new(private_sharing_req** pp_req) {
  return 0;
}

int __fake_security_manager_private_sharing_req_add_paths(
    private_sharing_req* p_req, const char** pp_paths, size_t count) {
  return 0;
}

int __fake_security_manager_private_sharing_req_set_target_appid(
    private_sharing_req* p_req, const char* appid) {
  return 0;
}

int __fake_security_manager_private_sharing_apply(const private_sharing_req* p_req) {
  return 0;
}

int __fake_security_manager_private_sharing_drop(const private_sharing_req* p_req) {
  return 0;
}

class Mocks :
    virtual public ::testing::NiceMock<GlibMock>,
    virtual public ::testing::NiceMock<SmackMock>,
    virtual public ::testing::NiceMock<TzplatformConfigMock>,
    virtual public ::testing::NiceMock<SecurityManagerMock> {};
} // namespace

class SharedFileTest : public TestFixture {
 public:
  SharedFileTest() : TestFixture(std::make_unique<::Mocks>()) {}
  std::shared_ptr<ImageItem> item;
  std::string id = "image_id";
  std::string image_path = "res/image.png";

  SharedFile* shared_file;

  virtual void SetUp() {
    item = make_shared<ImageItem>(id, image_path);
    shared_file = new SharedFile();
    item->SetSenderAppId("notification-ex-unittests");
  }

  virtual void TearDown() {
  }
};

TEST_F(SharedFileTest, IsPrivatePath) {
  EXPECT_CALL(GetMock<SmackMock>(), smack_new_label_from_path(_, _, _, _))
      .WillRepeatedly(Invoke(__fake_smack_new_label_from_path));

  ASSERT_TRUE(shared_file->IsPrivatePath(SharedFileTest::image_path));
}

TEST_F(SharedFileTest, GetDataPath) {
  EXPECT_CALL(GetMock<TzplatformConfigMock>(), tzplatform_getenv(_))
      .WillRepeatedly(Invoke(__fake_tzplatform_getenv));

  ASSERT_EQ(shared_file->GetDataPath(item->GetSenderAppId(), SharedFileTest::image_path),
    "/opt/usr/home/owner/apps_rw/notification-ex-unittests/data/.notification_ex/image.png");
}

TEST_F(SharedFileTest, SetPrivateSharing) {
  EXPECT_CALL(GetMock<SecurityManagerMock>(),
      security_manager_private_sharing_req_new(_))
      .WillRepeatedly(Invoke(__fake_security_manager_private_sharing_req_new));
  EXPECT_CALL(GetMock<SecurityManagerMock>(),
      security_manager_private_sharing_req_add_paths(_, _, _))
      .WillRepeatedly(Invoke(__fake_security_manager_private_sharing_req_add_paths));
  EXPECT_CALL(GetMock<SecurityManagerMock>(),
      security_manager_private_sharing_req_set_target_appid(_, _))
      .WillRepeatedly(Invoke(__fake_security_manager_private_sharing_req_set_target_appid));
  EXPECT_CALL(GetMock<SecurityManagerMock>(),
      security_manager_private_sharing_apply(_))
      .WillRepeatedly(Invoke(__fake_security_manager_private_sharing_apply));
  EXPECT_CALL(GetMock<SecurityManagerMock>(),
      security_manager_private_sharing_drop(_))
      .WillRepeatedly(Invoke(__fake_security_manager_private_sharing_drop));

  list<shared_ptr<item::AbstractItem>> notiList;
  notiList.push_back(SharedFileTest::item);

  std::multimap<std::string, std::string> map_;
  map_.insert(make_pair("tizen.org/receiver/popup", "test_appid"));

  ASSERT_EQ(shared_file->SetPrivateSharing(notiList, map_), ERROR_NONE);
}

TEST_F(SharedFileTest, RemovePrivateSharing) {
  EXPECT_CALL(GetMock<SecurityManagerMock>(),
      security_manager_private_sharing_req_new(_))
      .WillRepeatedly(Invoke(__fake_security_manager_private_sharing_req_new));
  EXPECT_CALL(GetMock<SecurityManagerMock>(),
      security_manager_private_sharing_req_add_paths(_, _, _))
      .WillRepeatedly(Invoke(__fake_security_manager_private_sharing_req_add_paths));
  EXPECT_CALL(GetMock<SecurityManagerMock>(),
      security_manager_private_sharing_req_set_target_appid(_, _))
      .WillRepeatedly(Invoke(__fake_security_manager_private_sharing_req_set_target_appid));
  EXPECT_CALL(GetMock<SecurityManagerMock>(),
      security_manager_private_sharing_apply(_))
      .WillRepeatedly(Invoke(__fake_security_manager_private_sharing_apply));
  EXPECT_CALL(GetMock<SecurityManagerMock>(),
      security_manager_private_sharing_drop(_))
      .WillRepeatedly(Invoke(__fake_security_manager_private_sharing_drop));

  list<shared_ptr<item::AbstractItem>> notiList;
  notiList.push_back(SharedFileTest::item);

  std::multimap<std::string, std::string> map_;
  map_.insert(make_pair("tizen.org/receiver/popup", "test_appid"));

  ASSERT_EQ(shared_file->RemovePrivateSharing(notiList, map_), ERROR_NONE);
}
