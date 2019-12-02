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
#include <unistd.h>

#include <memory>
#include <algorithm>
#include <vector>
#include <map>

#include "notification-ex/exception.h"
#include "notification-ex/abstract_item.h"
#include "notification-ex/abstract_item_implementation.h"
#include "notification-ex/item_info_internal.h"
#include "notification-ex/multi_language.h"
#include "notification-ex/ex_util.h"
#include "notification-ex/action_inflator.h"
#include "notification-ex/factory_manager.h"
#include "notification-ex/shared_file.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "NOTIFICATION_EX"
#define ABSTRACT_ITEM_TYPE_KEY "__ABSTRACT_ITEM_TYPE_KEY__"
#define ABSTRACT_ITEM_ID_KEY "__ABSTRACT_ITEM_ID_KEY__"
#define ABSTRACT_ITEM_UID_KEY "__ABSTRACT_ITEM_UID_KEY__"
#define ABSTRACT_ITEM_PRIVATE_ID_KEY "__ABSTRACT_ITEM_PRIVATE_ID_KEY__"
#define ABSTRACT_ITEM_TIME_KEY "__ABSTRACT_ITEM_TIME_KEY__"
#define ABSTRACT_ITEM_SENDER_APPID_KEY "__ABSTRACT_ITEM_SENDER_APPID_KEY__"
#define ABSTRACT_ITEM_TAG_KEY "__ABSTRACT_ITEM_TAG_KEY__"
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
#define ABSTRACT_ITEM_STYLE_BG_COLOR_A_KEY "__ABSTRACT_ITEM_STYLE_BG_COLOR_A_KEY__"
#define ABSTRACT_ITEM_STYLE_BG_COLOR_R_KEY "__ABSTRACT_ITEM_STYLE_BG_COLOR_R_KEY__"
#define ABSTRACT_ITEM_STYLE_BG_COLOR_G_KEY "__ABSTRACT_ITEM_STYLE_BG_COLOR_G_KEY__"
#define ABSTRACT_ITEM_STYLE_BG_COLOR_B_KEY "__ABSTRACT_ITEM_STYLE_BG_COLOR_B_KEY__"
#define ABSTRACT_ITEM_STYLE_BG_IMAGE_PATH_KEY "__ABSTRACT_ITEM_STYLE_BG_IMAGE_PATH_KEY__"
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
#define ABSTRACT_ITEM_HIDE_VIEWER_KEY "__ABSTRACT_ITEM_HIDE_VIEWER_KEY__"
#define ABSTRACT_ITEM_TRUE "TRUE"
#define ABSTRACT_ITEM_ONGOING_KEY "__ABSTRACT_ITEM_ONGOING_KEY__"
#define ABSTRACT_ITEM_MAIN_TYPE_KEY "__ABSTRACT_ITEM_MAIN_TYPE_KEY__"
#define ABSTRACT_ITEM_MULTI_LANG_ARR_KEY "__ABSTRACT_ITEM_MULTI_LANG_ARR_KEY__"
#define ABSTRACT_ITEM_MULTI_LANG_KEY "__ABSTRACT_ITEM_MULTI_LANG_KEY__"

using namespace std;
using namespace tizen_base;
namespace notification {
namespace item {

const string ReceiverGroup::Panel = "tizen.org/receiver/panel";
const string ReceiverGroup::Ticker = "tizen.org/receiver/ticker";
const string ReceiverGroup::LockScreen = "tizen.org/receiver/lockscreen";
const string ReceiverGroup::Indicator = "tizen.org/receiver/indicator";
const string ReceiverGroup::Popup = "tizen.org/receiver/popup";

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
AbstractItem::Impl::~Impl() = default;

Bundle AbstractItem::Serialize() const {
  Bundle b;
  struct tm timeinfo;
  char buf[80] = {0, };

  if (impl_->uid_ == 0)
    impl_->uid_ = getuid();

  if (!impl_->channel_.empty())
    b.Add(ABSTRACT_ITEM_CHANNEL_KEY, impl_->channel_);

  b.Add(ABSTRACT_ITEM_ID_KEY, impl_->id_);
  b.Add(ABSTRACT_ITEM_SENDER_APPID_KEY, GetSenderAppId().c_str());
  b.Add(ABSTRACT_ITEM_TYPE_KEY, to_string(GetType()));
  b.Add(ABSTRACT_ITEM_VERSION_KEY, to_string(impl_->version_));
  b.Add(ABSTRACT_ITEM_HIDE_TIME_KEY, to_string(impl_->hide_time_));
  b.Add(ABSTRACT_ITEM_DELETE_TIME_KEY, to_string(impl_->delete_time_));
  b.Add(ABSTRACT_ITEM_UID_KEY, to_string(impl_->uid_));
  b.Add(ABSTRACT_ITEM_PRIVATE_ID_KEY, to_string(impl_->private_id_));
  b.Add(ABSTRACT_ITEM_TAG_KEY, impl_->tag_);

  localtime_r(&impl_->time_, &timeinfo);
  strftime (buf, sizeof(buf), "%s", &timeinfo);
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
  b.Add(ABSTRACT_ITEM_ONGOING_KEY, to_string((int)impl_->ongoing_));
  b.Add(ABSTRACT_ITEM_MAIN_TYPE_KEY, to_string((int)impl_->main_type_));

  if (impl_->receiver_group_list_.size() != 0) {
    vector<string> arr;

    for (auto& i : impl_->receiver_group_list_) {
      string receiver = i;

      arr.push_back(receiver);
    }
    b.Add(ABSTRACT_ITEM_RECEIVER_GROUP_KEY, arr);
  }

  if (impl_->style_ != nullptr) {
    shared_ptr<Padding> padding = impl_->style_->GetPadding();
    if (padding.get() != nullptr) {
      b.Add(ABSTRACT_ITEM_STYLE_PADDING_LEFT_KEY, to_string(padding->GetLeft()));
      b.Add(ABSTRACT_ITEM_STYLE_PADDING_TOP_KEY, to_string(padding->GetTop()));
      b.Add(ABSTRACT_ITEM_STYLE_PADDING_RIGHT_KEY,
          to_string(padding->GetRight()));
      b.Add(ABSTRACT_ITEM_STYLE_PADDING_BOTTOM_KEY,
          to_string(padding->GetBottom()));
    }

    shared_ptr<Color> color = impl_->style_->GetColor();
    if (color.get() != nullptr) {
      b.Add(ABSTRACT_ITEM_STYLE_COLOR_A_KEY,
          to_string(static_cast<int>(color->GetAVal())));
      b.Add(ABSTRACT_ITEM_STYLE_COLOR_R_KEY,
          to_string(static_cast<int>(color->GetRVal())));
      b.Add(ABSTRACT_ITEM_STYLE_COLOR_G_KEY,
          to_string(static_cast<int>(color->GetGVal())));
      b.Add(ABSTRACT_ITEM_STYLE_COLOR_B_KEY,
          to_string(static_cast<int>(color->GetBVal())));
    }

    shared_ptr<Geometry> geometry = impl_->style_->GetGeometry();
    if (geometry.get() != nullptr) {
      b.Add(ABSTRACT_ITEM_STYLE_GEOMETRY_X_KEY, to_string(geometry->GetX()));
      b.Add(ABSTRACT_ITEM_STYLE_GEOMETRY_Y_KEY, to_string(geometry->GetY()));
      b.Add(ABSTRACT_ITEM_STYLE_GEOMETRY_WIDTH_KEY,
          to_string(geometry->GetWidth()));
      b.Add(ABSTRACT_ITEM_STYLE_GEOMETRY_HEIGHT_KEY,
          to_string(geometry->GetHeight()));
    }

    shared_ptr<Color> bg_color = impl_->style_->GetBackgroundColor();
    if (bg_color.get() != nullptr) {
      b.Add(ABSTRACT_ITEM_STYLE_BG_COLOR_A_KEY,
          to_string(static_cast<int>(bg_color->GetAVal())));
      b.Add(ABSTRACT_ITEM_STYLE_BG_COLOR_R_KEY,
          to_string(static_cast<int>(bg_color->GetRVal())));
      b.Add(ABSTRACT_ITEM_STYLE_BG_COLOR_G_KEY,
          to_string(static_cast<int>(bg_color->GetGVal())));
      b.Add(ABSTRACT_ITEM_STYLE_BG_COLOR_B_KEY,
          to_string(static_cast<int>(bg_color->GetBVal())));
    }
    b.Add(ABSTRACT_ITEM_STYLE_BG_IMAGE_PATH_KEY,
        impl_->style_->GetBackgroundImage());
    b.Add(ABSTRACT_ITEM_STYLE_IS_EXIST_KEY, ABSTRACT_ITEM_TRUE);
  }

  if (impl_->led_ != nullptr) {
    b.Add(ABSTRACT_ITEM_LED_ON_PERIOD_KEY, to_string(impl_->led_->GetOnPeriod()));
    b.Add(ABSTRACT_ITEM_LED_OFF_PERIOD_KEY, to_string(impl_->led_->GetOffPeriod()));
    b.Add(ABSTRACT_ITEM_LED_COLOR_A_KEY,
        to_string(static_cast<int>(impl_->led_->GetColor()->GetAVal())));
    b.Add(ABSTRACT_ITEM_LED_COLOR_R_KEY,
        to_string(static_cast<int>(impl_->led_->GetColor()->GetRVal())));
    b.Add(ABSTRACT_ITEM_LED_COLOR_G_KEY,
        to_string(static_cast<int>(impl_->led_->GetColor()->GetGVal())));
    b.Add(ABSTRACT_ITEM_LED_COLOR_B_KEY,
        to_string(static_cast<int>(impl_->led_->GetColor()->GetBVal())));
    b.Add(ABSTRACT_ITEM_LED_IS_EXIST_KEY, ABSTRACT_ITEM_TRUE);
  }

  if (impl_->action_ != nullptr) {
    b.Add(ABSTRACT_ITEM_ACTION_KEY,
      reinterpret_cast<char*>(impl_->action_->Serialize().ToRaw().first.get()));
  }

  if (impl_->multi_lang_ != nullptr) {
    b.Add(ABSTRACT_ITEM_MULTI_LANG_KEY,
      reinterpret_cast<char*>(impl_->multi_lang_->Serialize().ToRaw().first.get()));
  }

  if (impl_->multi_lang_arr_.size() > 0) {
    vector<string> arr;
    for (auto& i : impl_->multi_lang_arr_) {
      arr.push_back(reinterpret_cast<char*>(i->Serialize().ToRaw().first.get()));
    }
    b.Add(ABSTRACT_ITEM_MULTI_LANG_ARR_KEY, arr);
  }

  if (impl_->hide_viewer_list_.size() != 0) {
    vector<string> arr;

    for (auto& i : impl_->hide_viewer_list_) {
      string viewer = i;

      arr.push_back(viewer);
    }
    b.Add(ABSTRACT_ITEM_HIDE_VIEWER_KEY, arr);
  }

  return b;
}

void AbstractItem::Deserialize(Bundle b) {
  string time_s;
  struct tm timeinfo;

  string type_str = b.GetString(ABSTRACT_ITEM_TYPE_KEY);
  if (type_str.empty())
    THROW(ERROR_IO_ERROR);

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
  impl_->private_id_ = stoi(b.GetString(ABSTRACT_ITEM_PRIVATE_ID_KEY));
  impl_->tag_ = b.GetString(ABSTRACT_ITEM_TAG_KEY);

  time_s = b.GetString(ABSTRACT_ITEM_TIME_KEY);
  strptime(time_s.c_str(), "%s", &timeinfo);
  impl_->time_ = mktime(&timeinfo);

  string policy_str = b.GetString(ABSTRACT_ITEM_POLICY_KEY);

  impl_->policy_ = static_cast<Policy>(stoi(policy_str));
  impl_->visible_ = static_cast<bool>(stoi(b.GetString(ABSTRACT_ITEM_VISIBLE_KEY)));
  impl_->enable_ = static_cast<bool>(stoi(b.GetString(ABSTRACT_ITEM_ENABLE_KEY)));
  impl_->ongoing_ = static_cast<bool>(stoi(b.GetString(ABSTRACT_ITEM_ONGOING_KEY)));
  impl_->main_type_ = static_cast<MainType>(stoi(b.GetString(ABSTRACT_ITEM_MAIN_TYPE_KEY)));

  vector<string> receiver_group = b.GetStringArray(ABSTRACT_ITEM_RECEIVER_GROUP_KEY);
  if (receiver_group.size() != 0) {
    for (string str : receiver_group) {
      impl_->receiver_group_list_.push_back(str);
    }
  }

  if (!b.GetString(ABSTRACT_ITEM_STYLE_IS_EXIST_KEY).compare(ABSTRACT_ITEM_TRUE)) {
    shared_ptr<Padding> padding;
    if (!b.GetString(ABSTRACT_ITEM_STYLE_PADDING_LEFT_KEY).empty()) {
      padding = make_shared<Padding>(
        stoi(b.GetString(ABSTRACT_ITEM_STYLE_PADDING_LEFT_KEY)),
        stoi(b.GetString(ABSTRACT_ITEM_STYLE_PADDING_TOP_KEY)),
        stoi(b.GetString(ABSTRACT_ITEM_STYLE_PADDING_RIGHT_KEY)),
        stoi(b.GetString(ABSTRACT_ITEM_STYLE_PADDING_BOTTOM_KEY)));
    }

    shared_ptr<Color> color;
    if (!b.GetString(ABSTRACT_ITEM_STYLE_COLOR_A_KEY).empty()) {
      color = make_shared<Color>(
        static_cast<unsigned char>(
            stoi(b.GetString(ABSTRACT_ITEM_STYLE_COLOR_A_KEY))),
        static_cast<unsigned char>(
            stoi(b.GetString(ABSTRACT_ITEM_STYLE_COLOR_R_KEY))),
        static_cast<unsigned char>(
            stoi(b.GetString(ABSTRACT_ITEM_STYLE_COLOR_G_KEY))),
        static_cast<unsigned char>(
            stoi(b.GetString(ABSTRACT_ITEM_STYLE_COLOR_B_KEY))));
    }

    shared_ptr<Geometry> geometry;
    if (!b.GetString(ABSTRACT_ITEM_STYLE_GEOMETRY_X_KEY).empty()) {
      geometry = make_shared<Geometry>(
        stoi(b.GetString(ABSTRACT_ITEM_STYLE_GEOMETRY_X_KEY)),
        stoi(b.GetString(ABSTRACT_ITEM_STYLE_GEOMETRY_Y_KEY)),
        stoi(b.GetString(ABSTRACT_ITEM_STYLE_GEOMETRY_WIDTH_KEY)),
        stoi(b.GetString(ABSTRACT_ITEM_STYLE_GEOMETRY_HEIGHT_KEY)));
    }

    shared_ptr<Color> bg_color;
    if (!b.GetString(ABSTRACT_ITEM_STYLE_BG_COLOR_A_KEY).empty()) {
      bg_color = make_shared<Color>(
        static_cast<unsigned char>(
            stoi(b.GetString(ABSTRACT_ITEM_STYLE_BG_COLOR_A_KEY))),
        static_cast<unsigned char>(
            stoi(b.GetString(ABSTRACT_ITEM_STYLE_BG_COLOR_R_KEY))),
        static_cast<unsigned char>(
            stoi(b.GetString(ABSTRACT_ITEM_STYLE_BG_COLOR_G_KEY))),
        static_cast<unsigned char>(
            stoi(b.GetString(ABSTRACT_ITEM_STYLE_BG_COLOR_B_KEY))));
    }

    impl_->style_ = make_shared<Style>(color, padding, geometry, bg_color,
        b.GetString(ABSTRACT_ITEM_STYLE_BG_IMAGE_PATH_KEY));
  }

  if (!b.GetString(ABSTRACT_ITEM_LED_IS_EXIST_KEY).compare(ABSTRACT_ITEM_TRUE)) {
    shared_ptr<Color> led_color = make_shared<Color>(
      static_cast<unsigned char>(
        stoi(b.GetString(ABSTRACT_ITEM_LED_COLOR_A_KEY))),
      static_cast<unsigned char>(
        stoi(b.GetString(ABSTRACT_ITEM_LED_COLOR_R_KEY))),
      static_cast<unsigned char>(
        stoi(b.GetString(ABSTRACT_ITEM_LED_COLOR_G_KEY))),
      static_cast<unsigned char>(
        stoi(b.GetString(ABSTRACT_ITEM_LED_COLOR_B_KEY))));

    impl_->led_ = make_shared<LEDInfo>(led_color);
    impl_->led_->SetOnPeriod(stoi(b.GetString(ABSTRACT_ITEM_LED_ON_PERIOD_KEY)));
    impl_->led_->SetOffPeriod(stoi(b.GetString(ABSTRACT_ITEM_LED_OFF_PERIOD_KEY)));
  }

  string action_str = b.GetString(ABSTRACT_ITEM_ACTION_KEY);
  if (!action_str.empty())
    impl_->action_ = ActionInflator::Create(Bundle(action_str));

  string multi_str = b.GetString(ABSTRACT_ITEM_MULTI_LANG_KEY);
  if (!multi_str.empty())
    impl_->multi_lang_ = shared_ptr<MultiLanguage>(new MultiLanguage(multi_str));

  vector<string> multi_arr_str = b.GetStringArray(ABSTRACT_ITEM_MULTI_LANG_ARR_KEY);
  if (!multi_arr_str.empty()) {
    for (string str : multi_arr_str) {
      impl_->multi_lang_arr_.push_back(
          shared_ptr<MultiLanguage>(new MultiLanguage(str)));
    }
  }

  vector<string> hide_viewer = b.GetStringArray(ABSTRACT_ITEM_HIDE_VIEWER_KEY);
  if (hide_viewer.size() != 0) {
    for (string str : hide_viewer) {
      impl_->hide_viewer_list_.push_back(str);
    }
  }
}

void AbstractItem::SetId(std::string id) {
  impl_->id_ = id;
}

string AbstractItem::GetId() const {
  return impl_->id_;
}

std::list<std::string> AbstractItem::GetSharedPath() const {
  std::list<std::string> ret;

  if (!impl_->priv_sound_path_.empty())
    ret.push_back(impl_->priv_sound_path_);

  if (!impl_->priv_vibration_path_.empty())
    ret.push_back(impl_->priv_vibration_path_);

  return ret;
}

void AbstractItem::SetSharedPath() {
  if (!impl_->priv_sound_path_.empty())
    impl_->sound_path_ = impl_->priv_sound_path_;

  if (!impl_->priv_vibration_path_.empty())
    impl_->vibration_path_ = impl_->priv_vibration_path_;
}

std::list<std::map<std::string, std::string>> AbstractItem::GetPathMapList() const {
  std::list<std::map<std::string, std::string>> path_map_list;
  std::map<std::string, std::string> path_map;

  if (!impl_->priv_sound_path_.empty()) {
    path_map.insert(std::pair<std::string, std::string>(impl_->sound_path_,
        impl_->priv_sound_path_));
    path_map_list.push_back(path_map);
  }

  if (!impl_->priv_vibration_path_.empty()) {
    path_map.insert(std::pair<std::string, std::string>(impl_->vibration_path_,
        impl_->priv_vibration_path_));
    path_map_list.push_back(path_map);
  }

  return path_map_list;
}

shared_ptr<AbstractAction> AbstractItem::GetAction() const {
  return impl_->action_;
}

void AbstractItem::SetAction(std::shared_ptr<AbstractAction> action) {
  impl_->action_ = action;
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
    THROW(ERROR_IO_ERROR);
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

void AbstractItem::SetPolicy(int policy) {
  impl_->policy_ = policy;
}

int AbstractItem::GetPolicy() const {
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
  UpdateSoundPrivatePath();
}

void AbstractItem::SetVibrationPath(std::string path) {
  impl_->vibration_path_ = path;
  UpdateVibrationPrivatePath();
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

std::string AbstractItem::GetTag() const {
  return impl_->tag_;
}

void AbstractItem::SetTag(std::string tag) {
  impl_->tag_ = tag;
}

void AbstractItem::SetOnGoingState(bool ongoing) {
  impl_->ongoing_ = ongoing;
}

bool AbstractItem::GetOnGoingState() const {
  return impl_->ongoing_;
}

bool AbstractItem::Impl::IsValidMainType(AbstractItem& target, MainType type) {
  int item_type = target.GetType();

  switch (type) {
    case MainNone:
      return true;
    case MainTitle:
    case MainContents:
      if (item_type != Text)
        return false;
      break;
    case MainIcon:
      if (item_type != Image)
        return false;
      break;
    case MainButton:
      if (item_type != Button)
        return false;
      break;
    default:
      return false;
  }

  return true;
}

bool AbstractItem::SetMainType(std::string target_id, MainType type) {
  AbstractItem& target = FindByID(target_id);
  if (target.GetType() == NullObject)
    return false;

  if (type == MainNone) {
    LOGW("Clear main type from %s", target_id.c_str());
    target.impl_->main_type_ = MainNone;
    return true;
  }

  if (!impl_->IsValidMainType(target, type)) {
    LOGE("Main type and item type are not matched (%d, %d)",
        type, target.GetType());
    return false;
  }

  AbstractItem& old = FindByMainType(type);
  if (target.GetType() != NullObject)
    old.impl_->main_type_ = MainNone;
  target.impl_->main_type_ = type;

  return true;
}

AbstractItem::MainType AbstractItem::GetMainType() const {
  return impl_->main_type_;
}

AbstractItem& AbstractItem::FindByID(std::string id) {
  if (GetId() == id)
    return *this;

  return FactoryManager::GetInst().GetNullItem();
}

AbstractItem& AbstractItem::FindByMainType(AbstractItem::MainType type) {
  if (impl_->main_type_ == type)
    return *this;

  return FactoryManager::GetInst().GetNullItem();
}

std::shared_ptr<MultiLanguage> AbstractItem::GetMultiLanguage() const {
  return impl_->multi_lang_;
}

vector<shared_ptr<MultiLanguage>> AbstractItem::GetMultiLanguageArr() const {
  return impl_->multi_lang_arr_;
}

void AbstractItem::SetMultiLanguage(std::shared_ptr<MultiLanguage> multi) {
  impl_->multi_lang_ = multi;
}

void AbstractItem::SetMultiLanguage(
    vector<shared_ptr<MultiLanguage>> multi_arr) {
  impl_->multi_lang_arr_ = multi_arr;
}

void AbstractItem::UpdateSoundPrivatePath() {
  SharedFile* shared_file = new SharedFile();
  if (!shared_file->IsPrivatePath(impl_->sound_path_)) {
    delete(shared_file);
    return;
  }

  std::string path = shared_file->GetDataPath(AbstractItem::GetSenderAppId(),
            impl_->sound_path_);
  if (!path.empty())
    impl_->priv_sound_path_ = path;

  delete(shared_file);
}

void AbstractItem::UpdateVibrationPrivatePath() {
  SharedFile* shared_file = new SharedFile();
  if (!shared_file->IsPrivatePath(impl_->vibration_path_)) {
    delete(shared_file);
    return;
  }

  std::string path = shared_file->GetDataPath(AbstractItem::GetSenderAppId(),
            impl_->vibration_path_);
  if (!path.empty())
    impl_->priv_vibration_path_ = path;

  delete(shared_file);
}

}  // namespace item
}  // namespace notification
