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

std::list<std::string> ChatMessageItem::GetSharedPath() const {
  std::list<std::string> ret;

  auto name = impl_->name_->GetSharedPath();
  auto text = impl_->text_->GetSharedPath();
  auto image = impl_->image_->GetSharedPath();
  auto time = impl_->time_->GetSharedPath();

  for (auto& i : name) {
    ret.push_back(std::move(i));
  }

  for (auto& i : text) {
    ret.push_back(std::move(i));
  }

  for (auto& i : image) {
    ret.push_back(std::move(i));
  }

  for (auto& i : time) {
    ret.push_back(std::move(i));
  }

  return ret;
}

Bundle ChatMessageItem::Serialize() const {
  Bundle b;
  b = AbstractItem::Serialize();

  b.Add(CHATMESSAGE_NAME_KEY,
    reinterpret_cast<char*>(impl_->name_->Serialize().ToRaw().first.get()));
  b.Add(CHATMESSAGE_TEXT_KEY,
    reinterpret_cast<char*>(impl_->text_->Serialize().ToRaw().first.get()));
  b.Add(CHATMESSAGE_IMAGE_KEY,
    reinterpret_cast<char*>(impl_->image_->Serialize().ToRaw().first.get()));
  b.Add(CHATMESSAGE_TIME_KEY,
    reinterpret_cast<char*>(impl_->time_->Serialize().ToRaw().first.get()));
  b.Add(CHATMESSAGE_TYPE_KEY, std::to_string((int)impl_->type_));
  return b;
}

void ChatMessageItem::Deserialize(Bundle b) {
  AbstractItem::Deserialize(b);

  std::shared_ptr<AbstractItem> name =
      FactoryManager::GetInst().CreateItem(AbstractItem::Text);
  name.get()->Deserialize(Bundle(b.GetString(CHATMESSAGE_NAME_KEY)));
  impl_->name_ = std::static_pointer_cast<TextItem>(name);

  std::shared_ptr<AbstractItem> text =
      FactoryManager::GetInst().CreateItem(AbstractItem::Text);
  text.get()->Deserialize(Bundle(b.GetString(CHATMESSAGE_TEXT_KEY)));
  impl_->text_ = std::static_pointer_cast<TextItem>(text);

  std::shared_ptr<AbstractItem> image =
      FactoryManager::GetInst().CreateItem(AbstractItem::Image);
  image.get()->Deserialize(Bundle(b.GetString(CHATMESSAGE_IMAGE_KEY)));
  impl_->image_ = std::static_pointer_cast<ImageItem>(image);

  std::shared_ptr<AbstractItem> time =
      FactoryManager::GetInst().CreateItem(AbstractItem::Time);
  time.get()->Deserialize(Bundle(b.GetString(CHATMESSAGE_TIME_KEY)));
  impl_->time_ = std::static_pointer_cast<TimeItem>(time);

  impl_->type_ = static_cast<Type>(std::stoi(b.GetString(CHATMESSAGE_TYPE_KEY)));
}

AbstractItem&  ChatMessageItem::FindByID(std::string id) {
  if (GetId() == id)
    return *this;

  return FactoryManager::GetInst().GetNullItem();
}

TextItem& ChatMessageItem::GetNameItem() const {
  return *(impl_->name_);
}

TextItem& ChatMessageItem::GetTextItem() const {
  return *(impl_->text_);
}

ImageItem& ChatMessageItem::GetImageItem() const {
  return *(impl_->image_);
}

TimeItem& ChatMessageItem::GetTimeItem() const {
  return *(impl_->time_);
}

ChatMessageItem::Type ChatMessageItem::GetMessageType() const {
  return impl_->type_;
}

ChatMessageItem::~ChatMessageItem() = default;
ChatMessageItem::Impl::~Impl() = default;

}  // namespace item
}  // namespace notification_ex
