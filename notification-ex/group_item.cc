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
#include <app_common.h>

#include <memory>
#include <vector>

#include "notification-ex/group_item.h"
#include "notification-ex/group_item_implementation.h"
#include "notification-ex/factory_manager.h"
#include "notification-ex/exception.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "NOTIFICATION_EX"
#define GROUP_CHILDREN_KEY "__GROUP_CHILDREN_KEY__"
#define GROUP_CHILDREN_TYPE_KEY "__GROUP_CHILDREN_TYPE_KEY__"
#define GROUP_DIRECTION_KEY "__GROUP_DIRECTION_KEY__"
#define GROUP_DIRECTION_VERTICAL "VERTICAL"
#define GROUP_DIRECTION_HORIZONTAL "HORIZONTAL"
#define GROUP_APP_LABEL_KEY "__GROUP_APP_LABEL_KEY__"

using namespace std;
namespace notification {
namespace item {

GroupItem::GroupItem(string id, shared_ptr<AbstractAction> action)
    : AbstractItem(id), impl_(new Impl(this)) {
}

GroupItem::GroupItem(shared_ptr<AbstractAction> action)
    : AbstractItem(), impl_(new Impl(this)) {
}

GroupItem::Impl::Impl(GroupItem* parent)
    : parent_(parent) {
  LOGI("GroupItem created");
}

GroupItem::~GroupItem() {
}

int GroupItem::GetType() const {
  return AbstractItem::Group;
}

Bundle GroupItem::Serialize() const {
  Bundle b;
  b = AbstractItem::Serialize();
  b.Add(GROUP_DIRECTION_KEY, impl_->is_vertical_ ?
      GROUP_DIRECTION_VERTICAL : GROUP_DIRECTION_HORIZONTAL);
  if (!impl_->app_label_.empty())
    b.Add(GROUP_APP_LABEL_KEY, impl_->app_label_);

  if (impl_->children_list_.size() == 0)
    return b;

  vector<std::string> arr;
  for (auto& i : impl_->children_list_) {
    Bundle serialized = i.get()->Serialize();
    serialized.Add(
        GROUP_CHILDREN_TYPE_KEY, to_string(static_cast<int>(i.get()->GetType())));
    arr.push_back(reinterpret_cast<char*>(serialized.ToRaw().first.get()));
  }
  b.Add(GROUP_CHILDREN_KEY, arr);

  return b;
}

void GroupItem::Deserialize(Bundle b) {
  AbstractItem::Deserialize(b);
  impl_->is_vertical_ =
      (b.GetString(GROUP_DIRECTION_KEY) == GROUP_DIRECTION_VERTICAL);
  impl_->app_label_ = b.GetString(GROUP_APP_LABEL_KEY);

  vector<string> str_arr = b.GetStringArray(GROUP_CHILDREN_KEY);
  if (str_arr.size() == 0)
    return;

  for (string str : str_arr) {
    Bundle serialized(str);
    string type_str = serialized.GetString(GROUP_CHILDREN_TYPE_KEY);
    AbstractItem::Type child_type =
        static_cast<AbstractItem::Type>(strtol(type_str.c_str(), NULL, 10));
    shared_ptr<AbstractItem> child =
        FactoryManager::GetInst().CreateItem(child_type);
    child.get()->Deserialize(serialized);
    AddChild(child);
  }
}

AbstractItem& GroupItem::FindByID(std::string id) {
  if (GetId() == id)
    return *this;

  for (auto& i : impl_->children_list_) {
    auto& re = i.get()->FindByID(id);
    if (re.GetId() == id)
      return re;
  }
  return FactoryManager::GetInst().GetNullItem();
}

list<string> GroupItem::GetSharedPath() const {
  list<string> ret;

  for (auto& i : impl_->children_list_) {
    auto s = i->GetSharedPath();
    for (auto& j : s) {
      ret.push_back(move(j));
    }
  }

  return ret;
}

void GroupItem::AddChild(shared_ptr<AbstractItem> child) {
  impl_->children_list_.emplace_back(child);
}

void GroupItem::RemoveChild(string itemId) {
  for (auto& i : impl_->children_list_) {
    if (i.get()->GetId() == itemId) {
      impl_->children_list_.remove(i);
      break;
    }
  }
}

list<shared_ptr<AbstractItem>> GroupItem::GetChildren() {
  return impl_->children_list_;
}

void GroupItem::SetDirection(bool vertical) {
  impl_->is_vertical_ = vertical;
}

bool GroupItem::IsVertical() {
  return impl_->is_vertical_;
}

string GroupItem::GetAppLabel() {
  if (impl_->app_label_.empty()) {
    char* name;
    int ret = app_get_name(&name);
    if (ret != APP_ERROR_NONE)
      THROW(ERROR_IO_ERROR);
    impl_->app_label_ = string(name);
  }
  return impl_->app_label_;
}

void GroupItem::SetAppLabel(string label) {
  impl_->app_label_ = label;
}

}  // namespace item
}  // namespace notification
