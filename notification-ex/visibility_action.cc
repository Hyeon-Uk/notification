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

#include "notification-ex/abstract_item.h"
#include "notification-ex/visibility_action.h"
#include "notification-ex/visibility_action_implementation.h"
#include "notification-ex/exception.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "NOTIFICATION_EX"
#define VISIBILITY_ACTION_ENTITY_KEY "__VISIBILITY_ACTION_VISIBLE_KEY__"
#define VISIBILITY_ACTION_ENTITY_SIZE_KEY "__VISIBILITY_ACTION_ENTITY_SIZE_KEY__"

using namespace tizen_base;

namespace notification {
namespace item {

VisibilityAction::VisibilityAction()
    : AbstractAction(true), impl_(new Impl(this)) {
}

VisibilityAction::VisibilityAction(std::string extra)
    : AbstractAction(true, extra), impl_(new Impl(this)) {
}

VisibilityAction::Impl::Impl(VisibilityAction* parent) : parent_(parent) {
  LOGI("VisibilityAction created");
}

VisibilityAction::~VisibilityAction() = default;
VisibilityAction::Impl::~Impl() = default;

int VisibilityAction::GetType() const {
  return AbstractAction::Visibility;
}

Bundle VisibilityAction::Serialize() const {
  char buf_key[128];
  char buf_data[128];
  Bundle b;
  int i = 0;

  b = AbstractAction::Serialize();

  std::list<std::unique_ptr<Impl::VisibilityEntity>>::iterator it;

  for (it = impl_->entities_.begin(); it != impl_->entities_.end(); it++) {
    buf_key[0] = '\0';
    buf_data[0] = '\0';

    snprintf(buf_key, sizeof(buf_key), "%s:%d",
      VISIBILITY_ACTION_ENTITY_SIZE_KEY, i);

    snprintf(buf_data, sizeof(buf_data), "%s:%d",
      (*it)->id_.c_str(), (*it)->visibility_);

    b.Add(std::string(buf_key), std::string(buf_data));
    i++;
  }

  b.Add(VISIBILITY_ACTION_ENTITY_SIZE_KEY, std::to_string(i));
  return b;
}

void VisibilityAction::Deserialize(Bundle b) {
  char buf_key[128];
  int size;
  std::size_t pos;

  AbstractAction::Deserialize(b);

  size = std::stoi(b.GetString(VISIBILITY_ACTION_ENTITY_SIZE_KEY));

  for (int i = 0; i < size; i++) {
    buf_key[0] = '\0';

    snprintf(buf_key, sizeof(buf_key), "%s:%d",
      VISIBILITY_ACTION_ENTITY_SIZE_KEY, i);

    std::string str = b.GetString(std::string(buf_key));

    pos = str.find(":");
    std::string id = str.substr(0, pos);
    bool visible = std::stoi(str.substr(pos+1));

    impl_->entities_.emplace_back(new VisibilityAction::Impl::VisibilityEntity(
      id, visible));
  }
}

bool VisibilityAction::IsLocal() const {
  return AbstractAction::IsLocal();
}

void VisibilityAction::Execute(std::shared_ptr<AbstractItem> item) {
  for (const auto& entity : impl_->entities_) {
    AbstractItem& item_ = item->FindByID(entity->id_);
    if (item_.GetType() != AbstractItem::NullObject)
      item_.SetVisible(entity->visibility_);
  }
}

std::string VisibilityAction::GetExtra() const {
  return AbstractAction::GetExtra();
}

void VisibilityAction::SetVisibility(std::string id, bool visible) {
  impl_->entities_.emplace_back(new VisibilityAction::Impl::VisibilityEntity(
      id, visible));
}

}  //namespace item
}  //namespace notification
