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

#include <dlog.h>

#include "notification-ex/image_item.h"
#include "notification-ex/image_item_implementation.h"
#include "notification-ex/factory_manager.h"
#include "notification-ex/exception.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "NOTIFICATION_EX"
#define IMAGE_PATH_KEY "__IMAGE_PATH_KEY__"

namespace notification {
namespace item {

ImageItem::ImageItem(std::string image_path,
    std::shared_ptr<AbstractAction> action)
    : AbstractItem(action), impl_(new Impl(this, image_path)) {
}

ImageItem::ImageItem(std::string id, std::string image_path,
    std::shared_ptr<AbstractAction> action)
    : AbstractItem(id, action), impl_(new Impl(this, image_path)) {
}

ImageItem::Impl::Impl(ImageItem* parent, std::string image_path)
    : parent_(parent), image_path_(image_path) {
  LOGI("ImageItem impl created");
}

int ImageItem::GetType() const {
  return AbstractItem::Image;
}

Bundle ImageItem::Serialize() const {
  Bundle b;
  b = AbstractItem::Serialize();
  b.Add(IMAGE_PATH_KEY, impl_->image_path_);

  return b;
}

void ImageItem::Deserialize(Bundle b) {
  AbstractItem::Deserialize(b);
  impl_->image_path_ = b.GetString(IMAGE_PATH_KEY);
}

AbstractItem& ImageItem::FindByID(std::string id) {
  if (GetId() == id)
    return *this;

  return FactoryManager::GetInst().GetNullItem();
}

bool ImageItem::IsItemTypeExist(int type) {
  if (GetType() == type)
    return true;
  return false;
}

std::string ImageItem::GetImagePath() const {
  return impl_->image_path_;
}

ImageItem::~ImageItem() = default;
ImageItem::Impl::~Impl() = default;

}  // namespace item
}  // namespace notification
