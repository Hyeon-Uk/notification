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
#include <app_common.h>

#include "notification-ex/shared_file.h"
#include "notification-ex/image_item.h"
#include "notification-ex/common.h"

#include "mock/gio_mock.h"
#include "mock/smack_mock.h"
#include "mock/tzplatform_config_mock.h"
#include "mock/security_manager_mock.h"


using namespace notification;
using namespace notification::item;
using namespace std;

namespace {

GFile* __fake_g_file_new_for_path(const char* path) {
  GFile* noti_dir = {};
  return noti_dir;
}

gboolean __fake_g_file_query_exists(GFile* file, GCancellable* cancellable) {
  return false;
}

gboolean __fake_g_file_make_directory(GFile* file, GCancellable* cancellable,
    GError** error) {
  return true;
}

int __fake_access(const char* path, int mode) {
  return 0;
}

// smack
ssize_t __fake_smack_new_label_from_path(const char* path, const char* xattr,
    int follow, char** label) {
  *label = strdup("User::Pkg::unittest::RO");
  return 1;
}

// tzplatform
const char* __fake_tzplatform_getenv(tzplatform_variable_e id) {
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

class SharedFileTest  : public ::testing::Test {
 public:
  std::shared_ptr<ImageItem> item;
  std::string id = "image_id";
  std::string image_path = "res/image.png";

  SharedFile* shared_file;

  virtual void SetUp() {
    smack_new_label_from_path_fake.custom_fake = __fake_smack_new_label_from_path;
    tzplatform_getenv_fake.custom_fake = __fake_tzplatform_getenv;
    g_file_new_for_path_fake.custom_fake = __fake_g_file_new_for_path;
    g_file_query_exists_fake.custom_fake = __fake_g_file_query_exists;
    g_file_make_directory_fake.custom_fake = __fake_g_file_make_directory;
    access_fake.custom_fake = __fake_access;
    security_manager_private_sharing_req_new_fake.custom_fake =
      __fake_security_manager_private_sharing_req_new;
    security_manager_private_sharing_req_add_paths_fake.custom_fake =
      __fake_security_manager_private_sharing_req_add_paths;
    security_manager_private_sharing_req_set_target_appid_fake.custom_fake =
      __fake_security_manager_private_sharing_req_set_target_appid;
    security_manager_private_sharing_apply_fake.custom_fake =
      __fake_security_manager_private_sharing_apply;
    security_manager_private_sharing_drop_fake.custom_fake =
      __fake_security_manager_private_sharing_drop;

    item = make_shared<ImageItem>(id, image_path);
    shared_file = new SharedFile();
  }

  virtual void TearDown() {
  }
};

TEST_F(SharedFileTest, IsPrivatePath) {
  ASSERT_TRUE(shared_file->IsPrivatePath(SharedFileTest::image_path));
}

TEST_F(SharedFileTest, GetDataPath) {
  ASSERT_EQ(shared_file->GetDataPath(item->GetSenderAppId(), SharedFileTest::image_path),
    "/opt/usr/home/owner/apps_rw/notification-ex_unittests/data/.notification_ex/image.png");
}

TEST_F(SharedFileTest, SetPrivateSharing) {
  list<shared_ptr<item::AbstractItem>> notiList;
  notiList.push_back(SharedFileTest::item);

  std::multimap<std::string, std::string> map_;
  map_.insert(make_pair("tizen.org/receiver/popup", "test_appid"));

  ASSERT_EQ(shared_file->SetPrivateSharing(notiList, map_), ERROR_NONE);
}

TEST_F(SharedFileTest, RemovePrivateSharing) {
  list<shared_ptr<item::AbstractItem>> notiList;
  notiList.push_back(SharedFileTest::item);

  std::multimap<std::string, std::string> map_;
  map_.insert(make_pair("tizen.org/receiver/popup", "test_appid"));

  ASSERT_EQ(shared_file->RemovePrivateSharing(notiList, map_), ERROR_NONE);
}

}  // namespace
