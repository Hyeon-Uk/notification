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
#include <glib.h>
#include <unistd.h>

#include <list>

#include "api/notification_ex_app_control_action.h"
#include "api/notification_ex_button.h"
#include "api/notification_ex_chat_message.h"
#include "api/notification_ex_checkbox.h"
#include "api/notification_ex_entry.h"
#include "api/notification_ex_event_info.h"
#include "api/notification_ex_group.h"
#include "api/notification_ex_image.h"
#include "api/notification_ex_input_selector.h"
#include "api/notification_ex_item.h"
#include "api/notification_ex_manager.h"
#include "api/notification_ex_progress.h"
#include "api/notification_ex_reporter.h"
#include "api/notification_ex_text.h"
#include "api/notification_ex_time.h"
#include "api/notification_ex_visibility_action.h"
#include "notification-ex/reporter.h"
#include "notification-ex/app_control_action.h"
#include "notification-ex/button_item.h"
#include "notification-ex/chat_message_item.h"
#include "notification-ex/checkbox_item.h"
#include "notification-ex/entry_item.h"
#include "notification-ex/group_item.h"
#include "notification-ex/input_selector_item.h"
#include "notification-ex/abstract_item.h"
#include "notification-ex/progress_item.h"
#include "notification-ex/time_item.h"
#include "notification-ex/visibility_action.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "NOTIFICATION_EX"

#ifdef EXPORT_API
#undef EXPORT_API
#endif
#define EXPORT_API __attribute__((visibility("default")))

using namespace std;
using namespace notification::item;

extern "C" EXPORT_API int noti_ex_action_app_control_create(
    noti_ex_action_h *handle, app_control_h app_control,
    const char *extra) {
  if (handle == nullptr || app_control == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  auto* p = new (std::nothrow) AppControlAction(app_control, extra);
  if (p == nullptr) {
    LOGE("Out-of-memory");
    return NOTI_EX_ERROR_OUT_OF_MEMORY;
  }

  *handle = p;

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_action_app_control_set(
    noti_ex_action_h handle, app_control_h app_control) {
  if (handle == nullptr || app_control == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  AppControlAction* p = static_cast<AppControlAction*>(handle);
  p->SetAppControl(app_control);

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_action_app_control_get(
    noti_ex_action_h handle, app_control_h *app_control) {
  if (handle == nullptr || app_control == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  AppControlAction* p = static_cast<AppControlAction*>(handle);
  *app_control = p->GetAppControl();

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_item_button_create(noti_ex_item_h *handle,
    const char *id, const char *title) {
  ButtonItem* p;

  if (handle == nullptr || title == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  if (id)
    p = new (std::nothrow) ButtonItem(id, title);
  else
    p = new (std::nothrow) ButtonItem(title);

  if (p == nullptr) {
    LOGE("Out-of-memory");
    return NOTI_EX_ERROR_OUT_OF_MEMORY;
  }

  *handle = p;

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_item_button_get_title(noti_ex_item_h handle,
    char **title) {
  if (handle == nullptr || title == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  ButtonItem* p = static_cast<ButtonItem*>(handle);
  if (!p->GetTitle().empty()) {
    *title = strdup(p->GetTitle().c_str());
    if (*title == nullptr) {
      LOGE("Out-of-memory");
      return NOTI_EX_ERROR_OUT_OF_MEMORY;
    }
  }

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_item_chat_message_create(
    noti_ex_item_h *handle, const char *id, noti_ex_item_h name,
    noti_ex_item_h text, noti_ex_item_h image, noti_ex_item_h time,
    noti_ex_item_chat_message_type_e message_type) {
  if (handle == nullptr || message_type > NOTI_EX_ITEM_CHAT_MESSAGE_TYPE_SENDER) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  TextItem* name_ = static_cast<TextItem*>(name);
  TextItem* text_ = static_cast<TextItem*>(text);
  ImageItem* image_ = static_cast<ImageItem*>(image);
  TimeItem* time_ = static_cast<TimeItem*>(time);

  auto* p = new (std::nothrow) ChatMessageItem(id,
                static_cast<std::shared_ptr<TextItem>>(name_),
                static_cast<std::shared_ptr<TextItem>>(text_),
                static_cast<std::shared_ptr<ImageItem>>(image_),
                static_cast<std::shared_ptr<TimeItem>>(time_),
                static_cast<ChatMessageItem::Type>((int)message_type));
  if (p == nullptr) {
    LOGE("Out-of-memory");
    return NOTI_EX_ERROR_OUT_OF_MEMORY;
  }

  *handle = p;

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_item_chat_message_get_name(
    noti_ex_item_h handle, noti_ex_item_h *name) {
  if (handle == nullptr || name == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  ChatMessageItem* p = static_cast<ChatMessageItem*>(handle);
  *name = &(p->GetNameItem());

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_item_chat_message_get_text(
    noti_ex_item_h handle, noti_ex_item_h *text) {
  if (handle == nullptr || text == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  ChatMessageItem* p = static_cast<ChatMessageItem*>(handle);
  *text = &(p->GetTextItem());

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_item_chat_message_get_image(
    noti_ex_item_h handle, noti_ex_item_h *image) {
  if (handle == nullptr || image == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  ChatMessageItem* p = static_cast<ChatMessageItem*>(handle);
  *image =  &(p->GetImageItem());

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_item_chat_message_get_time(
    noti_ex_item_h handle, noti_ex_item_h *time) {
  if (handle == nullptr || time == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  ChatMessageItem* p = static_cast<ChatMessageItem*>(handle);
  *time = &(p->GetTimeItem());

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_item_chat_message_get_message_type(
    noti_ex_item_h handle, noti_ex_item_chat_message_type_e *message_type) {
  if (handle == nullptr || message_type == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  ChatMessageItem* p = static_cast<ChatMessageItem*>(handle);
  *message_type = (noti_ex_item_chat_message_type_e)(p->GetMessageType());

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_item_checkbox_create(noti_ex_item_h *handle,
    const char *id, const char *title, bool checked) {
  CheckBoxItem* p;

  if (handle == nullptr || title == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  p = new (std::nothrow) CheckBoxItem(id, title, checked);
  if (p == nullptr) {
    LOGE("Out-of-memory");
    return NOTI_EX_ERROR_OUT_OF_MEMORY;
  }

  *handle = p;

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_item_checkbox_get_title(noti_ex_item_h handle,
    char **title) {
  if (handle == nullptr || title == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  CheckBoxItem* p = static_cast<CheckBoxItem*>(handle);
  if (!p->GetTitle().empty()) {
    *title = strdup(p->GetTitle().c_str());
    if (*title == nullptr) {
        LOGE("Out-of-memory");
        return NOTI_EX_ERROR_OUT_OF_MEMORY;
    }
  }

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_item_checkbox_is_checked(noti_ex_item_h handle,
    bool *checked) {
  if (handle == nullptr || checked == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  CheckBoxItem* p = static_cast<CheckBoxItem*>(handle);
  *checked = p->IsChecked();

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_item_entry_create(noti_ex_item_h *handle,
    const char *id) {
  EntryItem* p;

  if (handle == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  p = new (std::nothrow) EntryItem(id);
  if (p == nullptr) {
    LOGE("Out-of-memory");
    return NOTI_EX_ERROR_OUT_OF_MEMORY;
  }

  *handle = p;

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_item_entry_get_text(noti_ex_item_h handle,
    char **text) {
  if (handle == nullptr || text == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  EntryItem* p = static_cast<EntryItem*>(handle);
  if (!p->GetText().empty()) {
    *text = strdup(p->GetText().c_str());
    if (*text == nullptr) {
        LOGE("Out-of-memory");
        return NOTI_EX_ERROR_OUT_OF_MEMORY;
    }
  }

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_item_entry_set_text(noti_ex_item_h handle,
    const char *text) {
  if (handle == nullptr || text == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  EntryItem* p = static_cast<EntryItem*>(handle);
  p->SetText(std::string(text));

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_event_info_create(noti_ex_event_info_h *handle,
    noti_ex_event_info_type_e type, const char *owner,
    const char *channel, const char *item_id) {
  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_event_info_destroy(
    noti_ex_event_info_h handle) {
  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_event_info_get_event_type(
    noti_ex_event_info_h handle, noti_ex_event_info_type_e *event_type) {
  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_event_info_get_owner(
    noti_ex_event_info_h handle, char **owner) {
  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_event_info_get_channel(
    noti_ex_event_info_h handle, char **channel) {
  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_event_info_get_item_id(
    noti_ex_event_info_h handle, char **item_id) {
  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_event_info_get_request_id(
    noti_ex_event_info_h handle, int *req_id) {
  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_item_group_create(noti_ex_item_h *handle,
    const char *id) {
  GroupItem* p;

  if (handle == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  if (id)
    p = new (std::nothrow) GroupItem(id);
  else
    p = new (std::nothrow) GroupItem();

  if (p == nullptr) {
    LOGE("Out-of-memory");
    return NOTI_EX_ERROR_OUT_OF_MEMORY;
  }

  *handle = p;

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_item_group_set_direction(noti_ex_item_h handle,
    bool vertical) {
  if (handle == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  GroupItem* p = static_cast<GroupItem*>(handle);
  p->SetDirection(vertical);

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_item_group_is_vertical(noti_ex_item_h handle,
    bool *vertical) {
  if (handle == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  GroupItem* p = static_cast<GroupItem*>(handle);
  *vertical = p->IsVertical();

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_item_group_get_app_label(noti_ex_item_h handle,
    char **label) {
  if (handle == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  GroupItem* p = static_cast<GroupItem*>(handle);
  if (!p->GetAppLabel().empty()) {
    *label = strdup(p->GetAppLabel().c_str());
    if (*label == nullptr) {
      LOGE("Out-of-memory");
      return NOTI_EX_ERROR_OUT_OF_MEMORY;
    }
  }

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_item_group_add_child(noti_ex_item_h handle,
    noti_ex_item_h child) {
  if (handle == nullptr || child == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  GroupItem* p = static_cast<GroupItem*>(handle);

  AbstractItem* child_ = static_cast<AbstractItem*>(child);
  p->AddChild(static_cast<std::shared_ptr<AbstractItem>>(child_));

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_item_group_remove_child(noti_ex_item_h handle,
    const char *item_id) {
  if (handle == nullptr || item_id == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  GroupItem* p = static_cast<GroupItem*>(handle);
  p->RemoveChild(std::string(item_id));

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_item_group_foreach(
    noti_ex_item_group_foreach_cb callback, void *data) {
  if (callback == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  // To do

  return 0;
}

extern "C" EXPORT_API int noti_ex_item_image_create(noti_ex_item_h *handle,
    const char *id, const char *image_path) {
  ImageItem* p;

  if (handle == nullptr  || image_path == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  if (id)
    p = new (std::nothrow) ImageItem(id, image_path);
  else
    p = new (std::nothrow) ImageItem(image_path);

  if (p == nullptr) {
    LOGE("Out-of-memory");
    return NOTI_EX_ERROR_OUT_OF_MEMORY;
  }

  *handle = p;

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_item_image_get_image_path(
    noti_ex_item_h handle, char **image_path) {
  if (handle == nullptr || image_path == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  ImageItem* p = static_cast<ImageItem*>(handle);
  if (!p->GetImagePath().empty()) {
    *image_path = strdup(p->GetImagePath().c_str());
    if (*image_path == nullptr) {
      LOGE("Out-of-memory");
      return NOTI_EX_ERROR_OUT_OF_MEMORY;
    }
  }

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_item_input_selector_create(
    noti_ex_item_h *handle, const char *id) {
  InputSelectorItem* p;

  if (handle == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  if (id)
    p = new (std::nothrow) InputSelectorItem(id);
  else
    p = new (std::nothrow) InputSelectorItem();

  if (p == nullptr) {
    LOGE("Out-of-memory");
    return NOTI_EX_ERROR_OUT_OF_MEMORY;
  }

  *handle = p;

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_item_input_selector_get_contents(
    noti_ex_item_h handle, char ***list, int *count) {
  if (handle == nullptr || list == nullptr || count == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  // To do

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_item_input_selector_set_contents(
    noti_ex_item_h handle, const char **contents, int count) {
  if (handle == nullptr || contents == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  // To do

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_color_create(noti_ex_color_h *handle,
    unsigned char a, unsigned char r, unsigned char g, unsigned char b) {
  if (handle == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  auto* p = new (std::nothrow) Color(a, r, g, b);
  if (p == nullptr) {
    LOGE("Out-of-memory");
    return NOTI_EX_ERROR_OUT_OF_MEMORY;
  }

  *handle = p;

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_color_destroy(noti_ex_color_h handle) {
  if (handle == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  Color* p = static_cast<Color*>(handle);
  p->~Color();

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_color_get_alpha(noti_ex_color_h handle,
    unsigned char *val) {
  if (handle == nullptr || val == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  Color* p = static_cast<Color*>(handle);
  *val = p->GetAVal();

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_color_get_red(noti_ex_color_h handle,
    unsigned char *val) {
  if (handle == nullptr || val == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  Color* p = static_cast<Color*>(handle);
  *val = p->GetRVal();

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_color_get_green(noti_ex_color_h handle,
    unsigned char *val) {
  if (handle == nullptr || val == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  Color* p = static_cast<Color*>(handle);
  *val = p->GetGVal();

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_color_get_blue(noti_ex_color_h handle,
    unsigned char *val) {
  if (handle == nullptr || val == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  Color* p = static_cast<Color*>(handle);
  *val = p->GetBVal();

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_padding_create(noti_ex_padding_h *handle,
    int left, int top, int right, int bottom) {
  if (handle == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  auto* p = new (std::nothrow) Padding(left, top, right, bottom);
  if (p == nullptr) {
    LOGE("Out-of-memory");
    return NOTI_EX_ERROR_OUT_OF_MEMORY;
  }

  *handle = p;

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_padding_destroy(noti_ex_padding_h handle) {
  if (handle == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  Padding* p = static_cast<Padding*>(handle);
  p->~Padding();

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_padding_get_left(noti_ex_padding_h handle,
    int *val) {
  if (handle == nullptr || val == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  Padding* p = static_cast<Padding*>(handle);
  *val = p->GetLeft();

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_padding_get_top(noti_ex_padding_h handle,
    int *val) {
  if (handle == nullptr || val == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  Padding* p = static_cast<Padding*>(handle);
  *val = p->GetTop();

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_padding_get_right(noti_ex_padding_h handle,
    int *val) {
  if (handle == nullptr || val == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  Padding* p = static_cast<Padding*>(handle);
  *val = p->GetRight();

  return 0;
}

extern "C" EXPORT_API int noti_ex_padding_get_bottom(noti_ex_padding_h handle,
    int *val) {
  if (handle == nullptr || val == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  Padding* p = static_cast<Padding*>(handle);
  *val = p->GetBottom();

  return 0;
}

extern "C" EXPORT_API int noti_ex_geometry_create(noti_ex_geometry_h *handle,
    int x, int y, int w, int h) {
  if (handle == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  auto* p = new (std::nothrow) Geometry(x, y, w, h);
  if (p == nullptr) {
    LOGE("Out-of-memory");
    return NOTI_EX_ERROR_OUT_OF_MEMORY;
  }

  *handle = p;

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_geometry_destroy(noti_ex_geometry_h handle) {
  if (handle == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  Geometry* p = static_cast<Geometry*>(handle);
  p->~Geometry();

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_geometry_get_x(noti_ex_geometry_h handle,
    int *val) {
  if (handle == nullptr || val == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  Geometry* p = static_cast<Geometry*>(handle);
  *val = p->GetX();

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_geometry_get_y(noti_ex_geometry_h handle,
    int *val) {
  if (handle == nullptr || val == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  Geometry* p = static_cast<Geometry*>(handle);
  *val = p->GetY();

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_geometry_get_width(noti_ex_geometry_h handle,
    int *val) {
  if (handle == nullptr || val == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  Geometry* p = static_cast<Geometry*>(handle);
  *val = p->GetWidth();

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_geometry_get_height(noti_ex_geometry_h handle,
    int *val) {
  if (handle == nullptr || val == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  Geometry* p = static_cast<Geometry*>(handle);
  *val = p->GetHeight();

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_style_create(noti_ex_style_h *handle,
    noti_ex_color_h color,
    noti_ex_padding_h padding,
    noti_ex_geometry_h geometry) {
  if (handle == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  Color* color_ = static_cast<Color*>(color);
  Padding* padding_ = static_cast<Padding*>(padding);
  Geometry* geo_ = static_cast<Geometry*>(geometry);

  auto* p = new (std::nothrow) Style(*color_, *padding_, *geo_);
  if (p == nullptr) {
    LOGE("Out-of-memory");
    return NOTI_EX_ERROR_OUT_OF_MEMORY;
  }

  *handle = p;

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_style_destroy(noti_ex_style_h handle) {
  if (handle == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  Style* p = static_cast<Style*>(handle);
  p->~Style();

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_style_get_padding(noti_ex_style_h handle,
    noti_ex_padding_h *padding) {
  if (handle == nullptr || padding == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  Style* p = static_cast<Style*>(handle);
  Padding* padding_ = new (std::nothrow) Padding(p->GetPadding());
  if (padding_ == nullptr) {
    LOGE("Out-of-memory");
    return NOTI_EX_ERROR_OUT_OF_MEMORY;
  }

  *padding = padding_;

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_style_get_color(noti_ex_style_h handle,
    noti_ex_color_h *color) {
  if (handle == nullptr || color == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  Style* p = static_cast<Style*>(handle);
  Color* color_ = new (std::nothrow) Color(p->GetColor());
  if (color_ == nullptr) {
    LOGE("Out-of-memory");
    return NOTI_EX_ERROR_OUT_OF_MEMORY;
  }

  *color = color_;

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_style_get_geometry(noti_ex_style_h handle,
    noti_ex_geometry_h *geometry) {
  if (handle == nullptr || geometry == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  Style* p = static_cast<Style*>(handle);
  Geometry* geo_ = new (std::nothrow) Geometry(p->GetGeometry());
  if (geo_ == nullptr) {
    LOGE("Out-of-memory");
    return NOTI_EX_ERROR_OUT_OF_MEMORY;
  }

  *geometry = geo_;

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_led_info_create(noti_ex_led_info_h *handle,
    noti_ex_color_h color) {
  if (handle == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  Color* color_ = static_cast<Color*>(color);
  auto* p = new (std::nothrow) LEDInfo(*color_);
  if (p == nullptr) {
    LOGE("Out-of-memory");
    return NOTI_EX_ERROR_OUT_OF_MEMORY;
  }

  *handle = p;

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_led_info_destroy(noti_ex_led_info_h handle) {
  if (handle == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  LEDInfo* p = static_cast<LEDInfo*>(handle);
  p->~LEDInfo();

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_led_info_set_on_period(
    noti_ex_led_info_h handle, int ms) {
  if (handle == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  LEDInfo* p = static_cast<LEDInfo*>(handle);
  p->SetOnPeriod(ms);

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_led_info_get_on_period(
    noti_ex_led_info_h handle, int *ms) {
  if (handle == nullptr || ms == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  LEDInfo* p = static_cast<LEDInfo*>(handle);
  *ms = p->GetOnPeriod();

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_led_info_set_off_period(
    noti_ex_led_info_h handle, int ms) {
  if (handle == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  LEDInfo* p = static_cast<LEDInfo*>(handle);
  p->SetOffPeriod(ms);

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_led_info_get_off_period(
    noti_ex_led_info_h handle, int *ms) {
  if (handle == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  LEDInfo* p = static_cast<LEDInfo*>(handle);
  *ms = p->GetOffPeriod();

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_led_info_get_color(
    noti_ex_led_info_h handle, noti_ex_color_h *color) {
  if (handle == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  LEDInfo* p = static_cast<LEDInfo*>(handle);
  Color* color_ = new (std::nothrow) Color(p->GetColor());
  if (color_ == nullptr) {
    LOGE("Out-of-memory");
    return NOTI_EX_ERROR_OUT_OF_MEMORY;
  }

  *color = color_;

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_action_destroy(noti_ex_action_h handle) {
  if (handle == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  AbstractAction* p = static_cast<AbstractAction*>(handle);
  p->~AbstractAction();

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_action_get_type(noti_ex_action_h handle,
    int *type) {
  if (handle == nullptr || type == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  AbstractAction* p = static_cast<AbstractAction*>(handle);
  *type = p->GetType();

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_action_is_local(noti_ex_action_h handle,
    bool *local) {
  if (handle == nullptr || local == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  AbstractAction* p = static_cast<AbstractAction*>(handle);
  *local = p->IsLocal();

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_action_execute(noti_ex_action_h handle,
    noti_ex_item_h item) {
  if (handle == nullptr || item==nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  AbstractAction* p = static_cast<AbstractAction*>(handle);
  AbstractItem* item_ = static_cast<AbstractItem*>(item);
  p->Execute(static_cast<std::shared_ptr<AbstractItem>>(item_));

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_action_get_extra(noti_ex_action_h handle,
    char **extra) {
  if (handle == nullptr || extra == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  AbstractAction* p = static_cast<AbstractAction*>(handle);
  if (!p->GetExtra().empty()) {
    *extra = strdup(p->GetExtra().c_str());
    if (*extra == nullptr) {
      LOGE("Out-of-memory");
      return NOTI_EX_ERROR_OUT_OF_MEMORY;
    }
  }

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_item_info_get_hide_time(
    noti_ex_item_info_h handle, int *hide_time) {
  return 0;
}

extern "C" EXPORT_API int noti_ex_item_info_set_hide_time(
    noti_ex_item_info_h handle, int hide_time) {
  return 0;
}

extern "C" EXPORT_API int noti_ex_item_info_get_delete_time(
    noti_ex_item_info_h handle, int *delete_time) {
  return 0;
}

extern "C" EXPORT_API int noti_ex_item_info_set_delete_time(
    noti_ex_item_info_h handle, int delete_time) {
  return 0;
}

extern "C" EXPORT_API int noti_ex_item_info_get_time(
    noti_ex_item_info_h handle, time_t *time) {
  return 0;
}

extern "C" EXPORT_API int noti_ex_item_destroy(noti_ex_item_h handle) {
  return 0;
}

extern "C" EXPORT_API int noti_ex_item_find_by_id(noti_ex_item_h handle,
    const char *id, noti_ex_item_h *item) {
  return 0;
}

extern "C" EXPORT_API int noti_ex_item_get_type(noti_ex_item_h handle,
    int *type) {
  return 0;
}

extern "C" EXPORT_API int noti_ex_item_get_shared_path(noti_ex_item_h handle,
    char ***path, int *count) {
  return 0;
}

extern "C" EXPORT_API int noti_ex_item_get_id(noti_ex_item_h handle,
    char **id) {
  return 0;
}

extern "C" EXPORT_API int noti_ex_item_set_id(noti_ex_item_h handle,
    const char *id) {
  return 0;
}

extern "C" EXPORT_API int noti_ex_item_get_action(noti_ex_item_h handle,
    noti_ex_action_h *action) {
  return 0;
}

extern "C" EXPORT_API int noti_ex_item_set_action(noti_ex_item_h handle,
    noti_ex_action_h action) {
  return 0;
}

extern "C" EXPORT_API int noti_ex_item_get_style(noti_ex_item_h handle,
    noti_ex_style_h *style) {
  return 0;
}

extern "C" EXPORT_API int noti_ex_item_set_style(noti_ex_item_h handle,
    noti_ex_style_h style) {
  return 0;
}

extern "C" EXPORT_API int noti_ex_item_set_visible(noti_ex_item_h handle,
    bool visible) {
  return 0;
}

extern "C" EXPORT_API int noti_ex_item_get_visible(noti_ex_item_h handle,
    bool *visible) {
  return 0;
}

extern "C" EXPORT_API int noti_ex_item_set_enable(noti_ex_item_h handle,
    bool enable) {
  return 0;
}

extern "C" EXPORT_API int noti_ex_item_get_enable(noti_ex_item_h handle,
    bool *enable) {
  return 0;
}

extern "C" EXPORT_API int noti_ex_item_add_receiver(noti_ex_item_h handle,
    const char *receiver_group) {
  return 0;
}

extern "C" EXPORT_API int noti_ex_item_remove_receiver(noti_ex_item_h handle,
    const char *receiver_group) {
  return 0;
}

extern "C" EXPORT_API int noti_ex_item_get_receiver_list(noti_ex_item_h handle,
    char ***list, int *count) {
  return 0;
}

extern "C" EXPORT_API int noti_ex_item_set_policy(noti_ex_item_h handle,
    int policy) {
  return 0;
}

extern "C" EXPORT_API int noti_ex_item_get_policy(noti_ex_item_h handle,
    int *policy) {
  return 0;
}

extern "C" EXPORT_API int noti_ex_item_get_channel(noti_ex_item_h handle,
    char **channel) {
  return 0;
}

extern "C" EXPORT_API int noti_ex_item_set_channel(noti_ex_item_h handle,
    const char *channel) {
  return 0;
}

extern "C" EXPORT_API int noti_ex_item_set_led_info(noti_ex_item_h handle,
    noti_ex_led_info_h led) {
  return 0;
}

extern "C" EXPORT_API int noti_ex_item_get_led_info(noti_ex_item_h handle,
    noti_ex_led_info_h *led) {
  return 0;
}

extern "C" EXPORT_API int noti_ex_item_set_sound_path(noti_ex_item_h handle,
    const char *path) {
  return 0;
}

extern "C" EXPORT_API int noti_ex_item_set_vibration_path(noti_ex_item_h handle,
    const char *path) {
  return 0;
}

extern "C" EXPORT_API int noti_ex_item_get_sound_path(noti_ex_item_h handle,
    char **path) {
  return 0;
}

extern "C" EXPORT_API int noti_ex_item_get_vibration_path(noti_ex_item_h handle,
    char **path) {
  return 0;
}

extern "C" EXPORT_API int noti_ex_item_get_info(noti_ex_item_h handle,
    noti_ex_item_info_h *info) {
  return 0;
}

extern "C" EXPORT_API int noti_ex_item_get_sender_app_id(noti_ex_item_h handle,
    char **id) {
  return 0;
}

extern "C" EXPORT_API int noti_ex_item_set_sender_app_id(noti_ex_item_h handle,
    const char *id) {
  return 0;
}

extern "C" EXPORT_API int noti_ex_item_get_tag(noti_ex_item_h handle,
    char **tag) {
  return 0;
}

extern "C" EXPORT_API int noti_ex_item_set_tag(noti_ex_item_h handle,
    const char *tag) {
  return 0;
}

extern "C" EXPORT_API int noti_ex_manager_create(noti_ex_manager_h *handle,
    const char *receiver_group, noti_ex_manager_events_s event_callbacks,
    void *data) {
  return 0;
}

extern "C" EXPORT_API int noti_ex_manager_deatroy(noti_ex_manager_h handle) {
  return 0;
}

extern "C" EXPORT_API int noti_ex_manager_get(noti_ex_manager_h handle,
    noti_ex_item_h **items, int *count) {
  return 0;
}

extern "C" EXPORT_API int noti_ex_manager_update(noti_ex_manager_h handle,
    noti_ex_item_h noti, int *request_id) {
  return 0;
}

extern "C" EXPORT_API int noti_ex_manager_delete(noti_ex_manager_h handle,
    noti_ex_item_h noti, int *request_id) {
  return 0;
}

extern "C" EXPORT_API int noti_ex_manager_delete_all(noti_ex_manager_h handle,
    int *request_id) {
  return 0;
}

extern "C" EXPORT_API int noti_ex_manager_hide(noti_ex_manager_h handle,
    noti_ex_item_h noti, int *request_id) {
  return 0;
}

extern "C" EXPORT_API int noti_ex_manager_find_by_root_id(
    noti_ex_manager_h handle, const char *root_id, noti_ex_item_h *item) {
  return 0;
}

extern "C" EXPORT_API int noti_ex_manager_send_error(noti_ex_manager_h handle,
    noti_ex_event_info_h info, noti_ex_error_e error, int *request_id) {
  return 0;
}

extern "C" EXPORT_API int noti_ex_manager_get_count(noti_ex_manager_h handle,
    int *cnt) {
  return 0;
}

extern "C" EXPORT_API int noti_ex_item_progress_create(noti_ex_item_h *handle,
    const char *id, float min, float current, float max) {
  ProgressItem* p;

  if (handle == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  if (id)
    p = new (std::nothrow) ProgressItem(id, min, current, max);
  else
    p = new (std::nothrow) ProgressItem(min, current, max);

  if (p == nullptr) {
    LOGE("Out-of-memory");
    return NOTI_EX_ERROR_OUT_OF_MEMORY;
  }

  *handle = p;

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_item_progress_get_current(
    noti_ex_item_h handle, float *current) {
  if (handle == nullptr || current == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  ProgressItem* p = static_cast<ProgressItem*>(handle);
  *current = p->GetCurrent();

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_item_progress_set_current(
    noti_ex_item_h handle, float current) {
  if (handle == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  ProgressItem* p = static_cast<ProgressItem*>(handle);
  p->SetCurrent(current);

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_item_progress_get_min(noti_ex_item_h handle,
    float *min) {
  if (handle == nullptr || min == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  ProgressItem* p = static_cast<ProgressItem*>(handle);
  *min = p->GetMin();

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_item_progress_get_max(noti_ex_item_h handle,
    float *max) {
  if (handle == nullptr || max == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  ProgressItem* p = static_cast<ProgressItem*>(handle);
  *max = p->GetMax();

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_reporter_create(noti_ex_reporter_h *handle,
    noti_ex_reporter_events_s event_callbacks, void *data) {
  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_reporter_destroy(noti_ex_reporter_h handle) {
  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_reporter_send_error(noti_ex_reporter_h handle,
    noti_ex_event_info_h info, noti_ex_error_e error, int *request_id) {
  return 0;
}

extern "C" EXPORT_API int noti_ex_reporter_post(noti_ex_reporter_h handle,
    noti_ex_item_h noti, int *request_id) {
  return 0;
}

extern "C" EXPORT_API int noti_ex_reporter_post_list(noti_ex_reporter_h handle,
    noti_ex_item_h *noti_list, int count, int *request_id) {
  return 0;
}

extern "C" EXPORT_API int noti_ex_reporter_update(noti_ex_reporter_h handle,
    noti_ex_item_h noti, int *request_id) {
  return 0;
}

extern "C" EXPORT_API int noti_ex_reporter_delete(noti_ex_reporter_h handle,
    noti_ex_item_h noti, int *request_id) {
  return 0;
}

extern "C" EXPORT_API int noti_ex_reporter_delete_all(
    noti_ex_reporter_h handle, int *request_id) {
  return 0;
}

extern "C" EXPORT_API int noti_ex_reporter_find_by_root_id(
    noti_ex_reporter_h handle, const char *root_id, noti_ex_item_h *item) {
  return 0;
}

extern "C" EXPORT_API int noti_ex_item_text_create(noti_ex_item_h *handle,
    const char *id, const char *text, const char *hyper_link) {
  if (handle == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  auto* p = new (std::nothrow) TextItem(id, std::string(text),
                std::string(hyper_link));
  if (p == nullptr) {
    LOGE("Out-of-memory");
    return NOTI_EX_ERROR_OUT_OF_MEMORY;
  }

  *handle = p;

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_item_text_set_contents(noti_ex_item_h handle,
    const char *contents) {
  if (handle == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  TextItem* p = static_cast<TextItem*>(handle);
  p->SetContents(std::string(contents));

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_item_text_get_contents(noti_ex_item_h handle,
    char **contents) {
  if (handle == nullptr || contents == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  TextItem* p = static_cast<TextItem*>(handle);
  if (!p->GetContents().empty()) {
    *contents = strdup(p->GetContents().c_str());
    if (*contents == nullptr) {
      LOGE("Out-of-memory");
      return NOTI_EX_ERROR_OUT_OF_MEMORY;
    }
  }

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_item_text_get_hyper_link(
    noti_ex_item_h handle, char **hyper_link) {
  if (handle == nullptr || hyper_link == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  TextItem* p = static_cast<TextItem*>(handle);
  if (!p->GetHyperLink().empty()) {
    *hyper_link = strdup(p->GetHyperLink().c_str());
    if (*hyper_link == nullptr) {
      LOGE("Out-of-memory");
      return NOTI_EX_ERROR_OUT_OF_MEMORY;
    }
  }

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_item_time_create(noti_ex_item_h *handle,
    const char *id, time_t time) {
  TimeItem* p;

  if (handle == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  if (time) {
    if (id)
      p = new (std::nothrow) TimeItem(id, time);
    else
      p = new (std::nothrow) TimeItem(time);
  } else {
      p = new (std::nothrow) TimeItem();
  }

  if (p == nullptr) {
    LOGE("Out-of-memory");
    return NOTI_EX_ERROR_OUT_OF_MEMORY;
  }

  *handle = p;

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_item_time_get_time(noti_ex_item_h handle,
    time_t *time) {
  if (handle == nullptr || time == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  TimeItem*p = static_cast<TimeItem*>(handle);
  *time = p->GetTime();

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_action_visibility_create(
    noti_ex_action_h *handle, const char *extra) {
  if (handle == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  auto* p = new (std::nothrow) VisibilityAction(extra);
  if (p == nullptr) {
    LOGE("Out-of-memory");
    return NOTI_EX_ERROR_OUT_OF_MEMORY;
  }

  *handle = p;

  return NOTI_EX_ERROR_NONE;
}

extern "C" EXPORT_API int noti_ex_action_visibility_set(noti_ex_action_h handle,
    const char *id, bool visible) {
  if (handle == nullptr || id == nullptr) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  VisibilityAction* p = static_cast<VisibilityAction*>(handle);
  p->SetVisibility(id, visible);

  return 0;
}
