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

#include "mock/smack_mock.h"
#include "mock/app_common_mock.h"
#include "mock/test_fixture.h"
#include "notification-ex/image_item.h"
#include "notification-ex/item_inflator.h"

using namespace tizen_base;
using namespace notification;
using namespace notification::item;

namespace {
class Mocks :
    virtual public ::testing::NiceMock<SmackMock>,
    virtual public ::testing::NiceMock<AppCommonMock> {};
} // namespace

class ImageItemTest : public TestFixture {
 public:
  ImageItemTest() : TestFixture(std::make_unique<::Mocks>()) {}
  ImageItem* item;
  std::string id = "image_id";
  std::string imagePath = "image_path";
  virtual void SetUp() {
    item = new ImageItem(id, imagePath);
  }
  virtual void TearDown() {
    delete item;
  }
};

TEST_F(ImageItemTest, create) {
  EXPECT_NE(ImageItemTest::item, nullptr);
}

TEST_F(ImageItemTest, FindByID) {
  AbstractItem& child = ImageItemTest::item->FindByID(ImageItemTest::id);
  ImageItem& image = static_cast<ImageItem&>(child);
  ASSERT_EQ(image.GetImagePath(), ImageItemTest::imagePath);
}

TEST_F(ImageItemTest, FindByIDNullItemReturn) {
  AbstractItem& child = ImageItemTest::item->FindByID("not_existed_item");
  ASSERT_EQ(child.GetType(), ImageItem::NullObject);
}

TEST_F(ImageItemTest, SerializeDeserialize) {
  Bundle b = ImageItemTest::item->Serialize();

  std::shared_ptr<AbstractItem> gen_item = ItemInflator::Create(b);
  ASSERT_EQ(ImageItemTest::item->GetType(), gen_item->GetType());

  auto gen_image = std::static_pointer_cast<ImageItem>(gen_item);
  ASSERT_EQ(ImageItemTest::item->GetImagePath(), gen_image->GetImagePath());
}
