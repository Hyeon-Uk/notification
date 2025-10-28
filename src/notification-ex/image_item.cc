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

#include <unistd.h>
#include <sys/types.h>
#include <dlog.h>

#include <map>
#include <string>
#include <list>
#include <utility>

#include "notification-ex/image_item.h"
#include "notification-ex/image_item_implementation.h"
#include "notification-ex/factory_manager.h"
#include "notification-ex/exception.h"
#include "notification-ex/shared_file.h"
#include "notification-ex/item_info_internal.h"
#include "notification-ex/ex_util.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "NOTIFICATION_EX"
#define IMAGE_PATH_KEY "__IMAGE_PATH_KEY__"
#define PRIV_IMAGE_PATH_KEY "__PRIV_IMAGE_PATH_KEY__"

using namespace tizen_base;

namespace notification {
namespace item {
ImageItem::ImageItem(std::string image_path,
    std::shared_ptr<AbstractAction> action)
    : AbstractItem(action), impl_(new Impl(this, image_path)) {
  UpdatePrivatePath();
}

ImageItem::ImageItem(std::string id, std::string image_path,
    std::shared_ptr<AbstractAction> action)
    : AbstractItem(id, action), impl_(new Impl(this, image_path)) {
  UpdatePrivatePath();
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

  if (!impl_->priv_image_path_.empty())
    b.Add(PRIV_IMAGE_PATH_KEY, impl_->priv_image_path_);

  return b;
}

void ImageItem::Deserialize(Bundle b) {
  AbstractItem::Deserialize(b);
  impl_->image_path_ = b.GetString(IMAGE_PATH_KEY);
  impl_->priv_image_path_ = b.GetString(PRIV_IMAGE_PATH_KEY);
}

bool ImageItem::IsItemTypeExist(int type) {
  if (GetType() == type)
    return true;
  return false;
}

std::string ImageItem::GetImagePath() const {
  return impl_->image_path_;
}

std::list<std::string> ImageItem::GetSharedPath() const {
  std::list<std::string> ret;

  if (!impl_->priv_image_path_.empty())
    ret.push_back(impl_->priv_image_path_);

  return ret;
}

/* LCOV_EXCL_START */
void ImageItem::SetSharedPath() {
  if (!impl_->priv_image_path_.empty())
    impl_->image_path_ = impl_->priv_image_path_;
}

std::list<std::map<std::string, std::string>> ImageItem::GetPathMapList() const {
  std::list<std::map<std::string, std::string>> path_map_list;
  std::map<std::string, std::string> path_map;

  if (!impl_->priv_image_path_.empty()) {
    path_map.insert(std::pair<std::string, std::string>(impl_->image_path_,
        impl_->priv_image_path_));
    path_map_list.push_back(path_map);
  }

  return path_map_list;
}
/* LCOV_EXCL_STOP */

void ImageItem::UpdatePrivatePath() {
  std::string path;

  SharedFile* shared_file = new SharedFile();
  if (!shared_file->IsPrivatePath(impl_->image_path_)) {
    delete(shared_file);
    return;
  }

  path = shared_file->GetDataPath(AbstractItem::GetSenderAppId(),
            impl_->image_path_);
  if (!path.empty())
    impl_->priv_image_path_ = path;

  delete(shared_file);
}

ImageItem::~ImageItem() = default;
ImageItem::Impl::~Impl() = default;

}  // namespace item
}  // namespace notification
