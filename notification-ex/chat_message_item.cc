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

#include "notification-ex/chat_message_item.h"
#include "notification-ex/chat_message_item_implementation.h"
#include "notification-ex/factory_manager.h"
#include "notification-ex/exception.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "NOTIFICATION_EX"
#define CHATMESSAGE_NAME_KEY "__CHATMESSAGE_NAME_KEY__"
#define CHATMESSAGE_TEXT_KEY "__CHATMESSAGE_TEXT_KEY__"
#define CHATMESSAGE_IMAGE_KEY "__CHATMESSAGE_IMAGE_KEY__"
#define CHATMESSAGE_TIME_KEY "__CHATMESSAGE_TIME_KEY__"
#define CHATMESSAGE_TYPE_KEY "__CHATMESSAGE_TYPE_KEY__"

using namespace tizen_base;

namespace notification {
namespace item {

ChatMessageItem::ChatMessageItem(std::string id,
    std::shared_ptr<TextItem> name, std::shared_ptr<TextItem> text,
    std::shared_ptr<ImageItem> image, std::shared_ptr<TimeItem> time,
    Type type, std::shared_ptr<AbstractAction> action)
    : AbstractItem(id, action),
    impl_(new Impl(this, name, text, image, time, type)) {
}

ChatMessageItem::Impl::Impl(ChatMessageItem* parent,
    std::shared_ptr<TextItem> name, std::shared_ptr<TextItem> text,
    std::shared_ptr<ImageItem> image, std::shared_ptr<TimeItem> time, Type type)
    : parent_(parent), name_(name), text_(text), image_(image), time_(time),
    type_(type) {
  LOGI("ChatMessageItem impl created");
}

int ChatMessageItem::GetType() const {
  return AbstractItem::ChatMessage;
}

Bundle ChatMessageItem::Serialize() const {
  Bundle b;
  b = AbstractItem::Serialize();

  if (impl_->name_ != nullptr)
    b.Add(CHATMESSAGE_NAME_KEY,
      reinterpret_cast<char*>(impl_->name_->Serialize().ToRaw().first.get()));

  if (impl_->text_ != nullptr)
    b.Add(CHATMESSAGE_TEXT_KEY,
      reinterpret_cast<char*>(impl_->text_->Serialize().ToRaw().first.get()));

  if (impl_->image_ != nullptr)
    b.Add(CHATMESSAGE_IMAGE_KEY,
      reinterpret_cast<char*>(impl_->image_->Serialize().ToRaw().first.get()));

  if (impl_->time_ != nullptr)
    b.Add(CHATMESSAGE_TIME_KEY,
      reinterpret_cast<char*>(impl_->time_->Serialize().ToRaw().first.get()));

  b.Add(CHATMESSAGE_TYPE_KEY, std::to_string(static_cast<int>(impl_->type_)));
  return b;
}

void ChatMessageItem::Deserialize(Bundle b) {
  AbstractItem::Deserialize(b);

  if (!b.GetString(CHATMESSAGE_NAME_KEY).empty()) {
    std::shared_ptr<AbstractItem> name =
        FactoryManager::GetInst().CreateItem(AbstractItem::Text);
    name.get()->Deserialize(Bundle(b.GetString(CHATMESSAGE_NAME_KEY)));
    impl_->name_ = std::static_pointer_cast<TextItem>(name);
  }

  if (!b.GetString(CHATMESSAGE_TEXT_KEY).empty()) {
    std::shared_ptr<AbstractItem> text =
        FactoryManager::GetInst().CreateItem(AbstractItem::Text);
    text.get()->Deserialize(Bundle(b.GetString(CHATMESSAGE_TEXT_KEY)));
    impl_->text_ = std::static_pointer_cast<TextItem>(text);
  }

  if (!b.GetString(CHATMESSAGE_IMAGE_KEY).empty()) {
    std::shared_ptr<AbstractItem> image =
        FactoryManager::GetInst().CreateItem(AbstractItem::Image);
    image.get()->Deserialize(Bundle(b.GetString(CHATMESSAGE_IMAGE_KEY)));
    impl_->image_ = std::static_pointer_cast<ImageItem>(image);
  }

  if (!b.GetString(CHATMESSAGE_TIME_KEY).empty()) {
    std::shared_ptr<AbstractItem> time =
        FactoryManager::GetInst().CreateItem(AbstractItem::Time);
    time.get()->Deserialize(Bundle(b.GetString(CHATMESSAGE_TIME_KEY)));
    impl_->time_ = std::static_pointer_cast<TimeItem>(time);
  }

  impl_->type_ = static_cast<Type>(std::stoi(b.GetString(CHATMESSAGE_TYPE_KEY)));
}

AbstractItem& ChatMessageItem::FindByID(std::string id) {
  if (GetId() == id)
    return *this;

  if (impl_->name_->GetId() == id)
    return *(impl_->name_);

  if (impl_->text_->GetId() == id)
    return *(impl_->text_);

  if (impl_->image_->GetId() == id)
    return *(impl_->image_);

  if (impl_->time_->GetId() == id)
    return *(impl_->time_);

  return FactoryManager::GetInst().GetNullItem();
}

AbstractItem& ChatMessageItem::FindByMainType(MainType type) {
  if (GetMainType() == type)
    return *this;

  if (impl_->name_->GetMainType() == type)
    return *(impl_->name_);

  if (impl_->text_->GetMainType() == type)
    return *(impl_->text_);

  if (impl_->image_->GetMainType() == type)
    return *(impl_->image_);

  if (impl_->time_->GetMainType() == type)
    return *(impl_->time_);

  return FactoryManager::GetInst().GetNullItem();
}

bool ChatMessageItem::IsItemTypeExist(int type) {
  if (GetType() == type)
    return true;
  return false;
}

std::list<std::string> ChatMessageItem::GetSharedPath() const {
  std::list<std::string> ret;

  if (impl_->image_ != nullptr) {
    auto image = impl_->image_->GetSharedPath();
    for (auto& i : image) {
      ret.push_back(std::move(i));
    }
  }

  return ret;
}

TextItem& ChatMessageItem::GetNameItem() const {
  if (impl_->name_ == nullptr)
    return static_cast<TextItem&>(FactoryManager::GetInst().GetNullItem());
  return *(impl_->name_);
}

TextItem& ChatMessageItem::GetTextItem() const {
  if (impl_->text_ == nullptr)
    return static_cast<TextItem&>(FactoryManager::GetInst().GetNullItem());
  return *(impl_->text_);
}

ImageItem& ChatMessageItem::GetImageItem() const {
  if (impl_->image_ == nullptr)
    return static_cast<ImageItem&>(FactoryManager::GetInst().GetNullItem());
  return *(impl_->image_);
}

TimeItem& ChatMessageItem::GetTimeItem() const {
  if (impl_->time_ == nullptr)
    return static_cast<TimeItem&>(FactoryManager::GetInst().GetNullItem());
  return *(impl_->time_);
}

ChatMessageItem::Type ChatMessageItem::GetMessageType() const {
  return impl_->type_;
}

ChatMessageItem::~ChatMessageItem() = default;
ChatMessageItem::Impl::~Impl() = default;

}  // namespace item
}  // namespace notification
