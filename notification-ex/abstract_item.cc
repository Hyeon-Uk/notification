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
#include <uuid/uuid.h>

#include <memory>
#include <algorithm>

#include "notification-ex/exception.h"
#include "notification-ex/abstract_item.h"
#include "notification-ex/abstract_item_implementation.h"
#include "notification-ex/item_info_internal.h"
#include "notification-ex/ex_util.h"
#include "notification-ex/action_inflator.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "NOTIFICATION_EX"
#define ABSTRACT_ITEM_TYPE_KEY "__ABSTRACT_ITEM_TYPE_KEY__"
#define ABSTRACT_ITEM_ID_KEY "__ABSTRACT_ITEM_ID_KEY__"
#define ABSTRACT_ITEM_UID_KEY "__ABSTRACT_ITEM_UID_KEY__"
#define ABSTRACT_ITEM_REQUEST_ID_KEY "__ABSTRACT_ITEM_REQUEST_ID_KEY__"
#define ABSTRACT_ITEM_TIME_KEY "__ABSTRACT_ITEM_TIME_KEY__"
#define ABSTRACT_ITEM_SENDER_APPID_KEY "__ABSTRACT_ITEM_SENDER_APPID_KEY__"
#define ABSTRACT_ITEM_CHANNEL_KEY "__ABSTRACT_ITEM_CHANNEL_KEY__"
#define ABSTRACT_ITEM_VERSION_KEY "__ABSTRACT_ITEM_VERSION_KEY__"
#define ABSTRACT_ITEM_HIDE_TIME_KEY "__ABSTRACT_ITEM_HIDE_TIME_KEY__"
#define ABSTRACT_ITEM_DELETE_TIME_KEY "__ABSTRACT_ITEM_DELETE_TIME_KEY__"
#define ABSTRACT_ITEM_CAN_RECEIVE_KEY "__ABSTRACT_ITEM_CAN_RECEIVE_KEY__"
#define ABSTRACT_ITEM_SOUND_PATH_KEY "__ABSTRACT_ITEM_SOUND_PATH_KEY__"
#define ABSTRACT_ITEM_VIBRATION_PATH_KEY "__ABSTRACT_ITEM_VIBRATION_PATH_KEY__"
#define ABSTRACT_ITEM_POLICY_KEY "__ABSTRACT_ITEM_POLICY_KEY__"
#define ABSTRACT_ITEM_RECEIVER_GROUP_KEY "__ABSTRACT_ITEM_RECEIVER_GROUP_KEY__"
#define ABSTRACT_ITEM_VISIBLE_KEY "__ABSTRACT_ITEM_VISIBLE_KEY__"
#define ABSTRACT_ITEM_ENABLE_KEY "__ABSTRACT_ITEM_ENABLE_KEY__"
#define ABSTRACT_ITEM_STYLE_IS_EXIST_KEY "__ABSTRACT_ITEM_STYLE_IS_EXIST_KEY__"
#define ABSTRACT_ITEM_STYLE_PADDING_TOP_KEY "__ABSTRACT_ITEM_STYLE_PADDING_TOP_KEY__"
#define ABSTRACT_ITEM_STYLE_PADDING_LEFT_KEY "__ABSTRACT_ITEM_STYLE_PADDING_LEFT_KEY__"
#define ABSTRACT_ITEM_STYLE_PADDING_RIGHT_KEY "__ABSTRACT_ITEM_STYLE_PADDING_RIGHT_KEY__"
#define ABSTRACT_ITEM_STYLE_PADDING_BOTTOM_KEY "__ABSTRACT_ITEM_STYLE_PADDING_BOTTOM_KEY__"
#define ABSTRACT_ITEM_STYLE_COLOR_A_KEY "__ABSTRACT_ITEM_STYLE_COLOR_A_KEY__"
#define ABSTRACT_ITEM_STYLE_COLOR_R_KEY "__ABSTRACT_ITEM_STYLE_COLOR_R_KEY__"
#define ABSTRACT_ITEM_STYLE_COLOR_G_KEY "__ABSTRACT_ITEM_STYLE_COLOR_G_KEY__"
#define ABSTRACT_ITEM_STYLE_COLOR_B_KEY "__ABSTRACT_ITEM_STYLE_COLOR_B_KEY__"
#define ABSTRACT_ITEM_STYLE_GEOMETRY_X_KEY "__ABSTRACT_ITEM_STYLE_GEOMETRY_X_KEY__"
#define ABSTRACT_ITEM_STYLE_GEOMETRY_Y_KEY "__ABSTRACT_ITEM_STYLE_GEOMETRY_Y_KEY__"
#define ABSTRACT_ITEM_STYLE_GEOMETRY_WIDTH_KEY "__ABSTRACT_ITEM_STYLE_GEOMETRY_WIDTH_KEY__"
#define ABSTRACT_ITEM_STYLE_GEOMETRY_HEIGHT_KEY "__ABSTRACT_ITEM_STYLE_GEOMETRY_HEIGHT_KEY__"
#define ABSTRACT_ITEM_LED_ON_PERIOD_KEY "__ABSTRACT_ITEM_LED_ON_PERIOD_KEY__"
#define ABSTRACT_ITEM_LED_OFF_PERIOD_KEY "__ABSTRACT_ITEM_LED_OFF_PERIOD_KEY__"
#define ABSTRACT_ITEM_LED_COLOR_A_KEY "__ABSTRACT_ITEM_LED_COLOR_A_KEY__"
#define ABSTRACT_ITEM_LED_COLOR_R_KEY "__ABSTRACT_ITEM_LED_COLOR_R_KEY__"
#define ABSTRACT_ITEM_LED_COLOR_G_KEY "__ABSTRACT_ITEM_LED_COLOR_G_KEY__"
#define ABSTRACT_ITEM_LED_COLOR_B_KEY "__ABSTRACT_ITEM_LED_COLOR_B_KEY__"
#define ABSTRACT_ITEM_LED_IS_EXIST_KEY "__ABSTRACT_ITEM_LED_IS_EXIST_KEY__"
#define ABSTRACT_ITEM_ACTION_KEY "__ABSTRACT_ITEM_ACTION_KEY__"
#define ABSTRACT_ITEM_TRUE "TRUE"

using namespace std;
namespace notification {
namespace item {

string AbstractItem::Impl::GenerateItemId() {
  char uuid[37];
  uuid_t u;
  uuid_generate(u);
  uuid_unparse(u, uuid);
  string id = util::GetAppId();
  return string(uuid) + ":" + id;
}

AbstractItem::AbstractItem(std::shared_ptr<AbstractAction> action)
    : impl_(new Impl(this, action)) {
}

AbstractItem::AbstractItem(std::string id,
    std::shared_ptr<AbstractAction> action)
    : impl_(new Impl(this, id, action)) {
}

AbstractItem::Impl::Impl(AbstractItem* parent, string id,
    std::shared_ptr<AbstractAction> action)
    : id_(id), action_(action), parent_(parent),
    info_(std::make_shared<ItemInfo>(this)) {
  if (id_.empty())
    id_ = GenerateItemId();
  sender_appid_ = util::GetAppId();
  LOGI("AbstractItem created (%s)", id_.c_str());
}

AbstractItem::Impl::Impl(AbstractItem* parent,
    std::shared_ptr<AbstractAction> action)
    : action_(action), parent_(parent),
    info_(std::make_shared<ItemInfo>(this)) {
  LOGI("AbstractItem created");

  id_ = GenerateItemId();
  sender_appid_ = util::GetAppId();
  LOGI("AbstractItem created (%s)", id_.c_str());
}

AbstractItem::~AbstractItem() = default;

Bundle AbstractItem::Serialize() const {
  Bundle b;
  struct tm* timeinfo;
  char buf[80] = {0,};

  if (!impl_->channel_.empty())
    b.Add(ABSTRACT_ITEM_CHANNEL_KEY, impl_->channel_);

  b.Add(ABSTRACT_ITEM_ID_KEY, impl_->id_);
  b.Add(ABSTRACT_ITEM_SENDER_APPID_KEY, GetSenderAppId().c_str());
  b.Add(ABSTRACT_ITEM_TYPE_KEY, to_string(GetType()));
  b.Add(ABSTRACT_ITEM_VERSION_KEY, to_string(impl_->version_));
  b.Add(ABSTRACT_ITEM_HIDE_TIME_KEY, to_string(impl_->hide_time_));
  b.Add(ABSTRACT_ITEM_DELETE_TIME_KEY, to_string(impl_->delete_time_));
  b.Add(ABSTRACT_ITEM_UID_KEY, to_string(impl_->uid_));
  b.Add(ABSTRACT_ITEM_REQUEST_ID_KEY, to_string(impl_->request_id_));

  timeinfo = localtime(&impl_->time_);
  strftime (buf, sizeof(buf), "%s", timeinfo);
  b.Add(ABSTRACT_ITEM_TIME_KEY, string(buf));

  if (!impl_->can_receive_.empty())
    b.Add(ABSTRACT_ITEM_CAN_RECEIVE_KEY, impl_->can_receive_);

  if (!impl_->sound_path_.empty())
    b.Add(ABSTRACT_ITEM_SOUND_PATH_KEY, impl_->sound_path_);

  if (!impl_->vibration_path_.empty())
    b.Add(ABSTRACT_ITEM_VIBRATION_PATH_KEY, impl_->vibration_path_);

  b.Add(ABSTRACT_ITEM_VISIBLE_KEY, to_string((int)impl_->visible_));

  b.Add(ABSTRACT_ITEM_ENABLE_KEY, to_string((int)impl_->enable_));

  b.Add(ABSTRACT_ITEM_POLICY_KEY, to_string((int)impl_->policy_));

  if (impl_->receiver_group_list_.size() != 0) {
    vector<string> arr;

    for (auto& i : impl_->receiver_group_list_) {
      string receiver = i;

      arr.push_back(receiver);
    }
    b.Add(ABSTRACT_ITEM_RECEIVER_GROUP_KEY, arr);
  }

  if (impl_->style_ != nullptr) {
    Padding padding = impl_->style_->GetPadding();
    b.Add(ABSTRACT_ITEM_STYLE_PADDING_LEFT_KEY, to_string(padding.GetLeft()));
    b.Add(ABSTRACT_ITEM_STYLE_PADDING_TOP_KEY, to_string(padding.GetTop()));
    b.Add(ABSTRACT_ITEM_STYLE_PADDING_RIGHT_KEY, to_string(padding.GetRight()));
    b.Add(ABSTRACT_ITEM_STYLE_PADDING_BOTTOM_KEY, to_string(padding.GetBottom()));

    Color color = impl_->style_->GetColor();
    b.Add(ABSTRACT_ITEM_STYLE_COLOR_A_KEY,
        to_string(static_cast<int>(color.GetAVal())));
    b.Add(ABSTRACT_ITEM_STYLE_COLOR_R_KEY,
        to_string(static_cast<int>(color.GetRVal())));
    b.Add(ABSTRACT_ITEM_STYLE_COLOR_G_KEY,
        to_string(static_cast<int>(color.GetGVal())));
    b.Add(ABSTRACT_ITEM_STYLE_COLOR_B_KEY,
        to_string(static_cast<int>(color.GetBVal())));

    Geometry geometry = impl_->style_->GetGeometry();
    b.Add(ABSTRACT_ITEM_STYLE_GEOMETRY_X_KEY, to_string(geometry.GetX()));
    b.Add(ABSTRACT_ITEM_STYLE_GEOMETRY_Y_KEY, to_string(geometry.GetY()));
    b.Add(ABSTRACT_ITEM_STYLE_GEOMETRY_WIDTH_KEY, to_string(geometry.GetWidth()));
    b.Add(ABSTRACT_ITEM_STYLE_GEOMETRY_HEIGHT_KEY, to_string(geometry.GetHeight()));

    b.Add(ABSTRACT_ITEM_STYLE_IS_EXIST_KEY, ABSTRACT_ITEM_TRUE);
  }

  if (impl_->led_ != nullptr) {
    b.Add(ABSTRACT_ITEM_LED_ON_PERIOD_KEY, to_string(impl_->led_->GetOnPeriod()));
    b.Add(ABSTRACT_ITEM_LED_OFF_PERIOD_KEY, to_string(impl_->led_->GetOffPeriod()));
    b.Add(ABSTRACT_ITEM_LED_COLOR_A_KEY,
        to_string(static_cast<int>(impl_->led_->GetColor().GetAVal())));
    b.Add(ABSTRACT_ITEM_LED_COLOR_R_KEY,
        to_string(static_cast<int>(impl_->led_->GetColor().GetRVal())));
    b.Add(ABSTRACT_ITEM_LED_COLOR_G_KEY,
        to_string(static_cast<int>(impl_->led_->GetColor().GetGVal())));
    b.Add(ABSTRACT_ITEM_LED_COLOR_B_KEY,
        to_string(static_cast<int>(impl_->led_->GetColor().GetBVal())));
    b.Add(ABSTRACT_ITEM_LED_IS_EXIST_KEY, ABSTRACT_ITEM_TRUE);
  }

  if (impl_->action_ != nullptr) {
    b.Add(ABSTRACT_ITEM_ACTION_KEY,
      reinterpret_cast<char*>(impl_->action_->Serialize().ToRaw().first.get()));
  }

  return b;
}

void AbstractItem::Deserialize(Bundle b) {
  string time_s;
  struct tm timeinfo;

  string type_str = b.GetString(ABSTRACT_ITEM_TYPE_KEY);
  if (type_str.empty())
    THROW(NOTIFICATION_ERROR_IO_ERROR);

  impl_->id_ = b.GetString(ABSTRACT_ITEM_ID_KEY);
  impl_->sender_appid_ = b.GetString(ABSTRACT_ITEM_SENDER_APPID_KEY);
  impl_->channel_ = b.GetString(ABSTRACT_ITEM_CHANNEL_KEY);
  impl_->version_ = stoi(b.GetString(ABSTRACT_ITEM_VERSION_KEY));
  impl_->hide_time_ = stoi(b.GetString(ABSTRACT_ITEM_HIDE_TIME_KEY));
  impl_->delete_time_ = stoi(b.GetString(ABSTRACT_ITEM_DELETE_TIME_KEY));
  impl_->can_receive_ = b.GetString(ABSTRACT_ITEM_CAN_RECEIVE_KEY);
  impl_->sound_path_ = b.GetString(ABSTRACT_ITEM_SOUND_PATH_KEY);
  impl_->vibration_path_ = b.GetString(ABSTRACT_ITEM_VIBRATION_PATH_KEY);
  impl_->uid_ = stoi(b.GetString(ABSTRACT_ITEM_UID_KEY));
  impl_->request_id_ = stoi(b.GetString(ABSTRACT_ITEM_REQUEST_ID_KEY));

  time_s = b.GetString(ABSTRACT_ITEM_TIME_KEY);
  strptime(time_s.c_str(), "%s", &timeinfo);
  impl_->time_ = mktime(&timeinfo);

  string policy_str = b.GetString(ABSTRACT_ITEM_POLICY_KEY);

  impl_->policy_ = static_cast<Policy>(stoi(policy_str));

  impl_->visible_ = (bool)stoi(b.GetString(ABSTRACT_ITEM_VISIBLE_KEY));

  impl_->enable_ = (bool)stoi(b.GetString(ABSTRACT_ITEM_ENABLE_KEY));

  vector<string> str_arr = b.GetStringArray(ABSTRACT_ITEM_RECEIVER_GROUP_KEY);

  if (str_arr.size() != 0) {
    for (string str : str_arr) {
      impl_->receiver_group_list_.push_back(str);
    }
  }

  if (!b.GetString(ABSTRACT_ITEM_STYLE_IS_EXIST_KEY).compare(ABSTRACT_ITEM_TRUE)) {
    Padding padding(stoi(b.GetString(ABSTRACT_ITEM_STYLE_PADDING_LEFT_KEY)),
      stoi(b.GetString(ABSTRACT_ITEM_STYLE_PADDING_TOP_KEY)),
      stoi(b.GetString(ABSTRACT_ITEM_STYLE_PADDING_RIGHT_KEY)),
      stoi(b.GetString(ABSTRACT_ITEM_STYLE_PADDING_BOTTOM_KEY)));

    Color color(
      static_cast<unsigned char>(stoi(b.GetString(ABSTRACT_ITEM_STYLE_COLOR_A_KEY))),
      static_cast<unsigned char>(stoi(b.GetString(ABSTRACT_ITEM_STYLE_COLOR_R_KEY))),
      static_cast<unsigned char>(stoi(b.GetString(ABSTRACT_ITEM_STYLE_COLOR_G_KEY))),
      static_cast<unsigned char>(stoi(b.GetString(ABSTRACT_ITEM_STYLE_COLOR_B_KEY))));

    Geometry geometry(stoi(b.GetString(ABSTRACT_ITEM_STYLE_GEOMETRY_X_KEY)),
      stoi(b.GetString(ABSTRACT_ITEM_STYLE_GEOMETRY_Y_KEY)),
      stoi(b.GetString(ABSTRACT_ITEM_STYLE_GEOMETRY_WIDTH_KEY)),
      stoi(b.GetString(ABSTRACT_ITEM_STYLE_GEOMETRY_HEIGHT_KEY)));

    impl_->style_ = make_shared<Style>(color, padding, geometry);
  }

  if (!b.GetString(ABSTRACT_ITEM_LED_IS_EXIST_KEY).compare(ABSTRACT_ITEM_TRUE)) {
    Color color(
      static_cast<unsigned char>(stoi(b.GetString(ABSTRACT_ITEM_LED_COLOR_A_KEY))),
      static_cast<unsigned char>(stoi(b.GetString(ABSTRACT_ITEM_LED_COLOR_R_KEY))),
      static_cast<unsigned char>(stoi(b.GetString(ABSTRACT_ITEM_LED_COLOR_G_KEY))),
      static_cast<unsigned char>(stoi(b.GetString(ABSTRACT_ITEM_LED_COLOR_B_KEY))));

    impl_->led_ = make_shared<LEDInfo>(color);
    impl_->led_->SetOnPeriod(stoi(b.GetString(ABSTRACT_ITEM_LED_ON_PERIOD_KEY)));
    impl_->led_->SetOffPeriod(stoi(b.GetString(ABSTRACT_ITEM_LED_OFF_PERIOD_KEY)));
  }

  string action_str = b.GetString(ABSTRACT_ITEM_ACTION_KEY);

  if (!action_str.empty())
    impl_->action_ = ActionInflator::Create(Bundle(action_str));
}

string AbstractItem::GetId() const {
  return impl_->id_;
}

shared_ptr<AbstractAction> AbstractItem::GetAction() const {
  return impl_->action_;
}

shared_ptr<Style> AbstractItem::GetStyle() const {
  return impl_->style_;
}

void AbstractItem::SetStyle(shared_ptr<Style> style) {
  impl_->style_ = style;
}

void AbstractItem::SetLEDInfo(std::shared_ptr<LEDInfo> led) {
  impl_->led_ = led;
}

std::shared_ptr<LEDInfo> AbstractItem::GetLEDInfo() const {
  return impl_->led_;
}

int AbstractItem::GetType(Bundle b) {
  string type_str = b.GetString(ABSTRACT_ITEM_TYPE_KEY);
  if (type_str.empty())
    THROW(NOTIFICATION_ERROR_IO_ERROR);
  return strtol(type_str.c_str(), NULL, 10);
}

void AbstractItem::SetVisible(bool visible) {
  impl_->visible_ = visible;
}

bool AbstractItem::GetVisible() const {
  return impl_->visible_;
}

void AbstractItem::SetEnable(bool enable) {
  impl_->enable_ = enable;
}

bool AbstractItem::GetEnable() const {
  return impl_->enable_;
}

void AbstractItem::AddReceiver(std::string receiver_group) {
  impl_->receiver_group_list_.push_back(receiver_group);
}

void AbstractItem::RemoveReceiver(std::string receiver_group) {
  impl_->receiver_group_list_.remove(receiver_group);
}

list<string> AbstractItem::GetReceiverList() {
  return impl_->receiver_group_list_;
}

bool AbstractItem::CanReceive(std::string id) const {
  if (impl_->receiver_group_list_.size() == 0 || id.empty())
    return true;
  list<string>::iterator iter =
    std::find(impl_->receiver_group_list_.begin(),
      impl_->receiver_group_list_.end(), id);
  return !(iter == impl_->receiver_group_list_.end());
}

void AbstractItem::SetPolicy(AbstractItem::Policy policy) {
  impl_->policy_ = policy;
}

AbstractItem::Policy AbstractItem::GetPolicy() const {
  return impl_->policy_;
}

std::shared_ptr<IItemInfo> AbstractItem::GetInfo() const {
  return impl_->info_;
}

string AbstractItem::GetChannel() const {
  return impl_->channel_;
}

void AbstractItem::SetChannel(string channel) {
  impl_->channel_ = channel;
}

void AbstractItem::SetSoundPath(std::string path) {
  impl_->sound_path_ = path;
}

void AbstractItem::SetVibrationPath(std::string path) {
  impl_->vibration_path_ = path;
}

std::string AbstractItem::GetSoundPath() const {
  return impl_->sound_path_;
}

std::string AbstractItem::GetVibrationPath() const {
  return impl_->vibration_path_;
}

std::string AbstractItem::GetSenderAppId() const {
  return impl_->sender_appid_;
}

void AbstractItem::SetSenderAppId(std::string sender_appid) {
  impl_->sender_appid_ = sender_appid;
}

time_t AbstractItem::GetTime() {
  return impl_->time_;
}

int AbstractItem::GetUid() {
  return impl_->uid_;
}

void AbstractItem::SetUid(int uid) {
  impl_->uid_ = uid;
}

}  // namespace item
}  // namespace notification
