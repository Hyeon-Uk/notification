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
#include "notification-ex/item_factory.h"
#include "notification-ex/exception.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "NOTIFICATION_EX"
#define IMAGE_PATH_KEY "__IMAGE_PATH_KEY__"

namespace notification {
namespace item {

ImageItem::ImageItem(std::string imagePath,
  std::shared_ptr<AbstractAction> action)
  : AbstractItem(AbstractItem::Image, action), impl_(new Impl(this, imagePath)) {
}

ImageItem::ImageItem(std::string id, std::string imagePath,
  std::shared_ptr<AbstractAction> action)
  : AbstractItem(id, AbstractItem::Image, action), impl_(new Impl(this, imagePath)) {
}

ImageItem::Impl::Impl(ImageItem* parent, std::string imagePath)
  : parent_(parent), imagePath_(imagePath) {
  LOGI("ImageItem impl created");
}

Bundle ImageItem::Serialize() {
  Bundle b;
  b = AbstractItem::Serialize();
  b.Add(IMAGE_PATH_KEY, impl_->imagePath_);

  return b;
}

void ImageItem::Deserialize(Bundle b) {
  AbstractItem::Deserialize(b);

  impl_->imagePath_ = b.GetString(IMAGE_PATH_KEY);
}

AbstractItem& ImageItem::FindByID(std::string id) {
  if (GetId() == id)
    return *this;

  return ItemFactory::GetNullItem();
}

std::string ImageItem::GetImagePath() const {
  return impl_->imagePath_;
}

ImageItem::~ImageItem() = default;
ImageItem::Impl::~Impl() = default;


}  // namespace item
}  // namespace notification
