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
// To do
#if 0
  ChatMessageItem* p;

  if (handle == nullptr || message_type < NOTI_EX_ITEM_CHAT_MESSAGE_TYPE_USER
      || message_type > NOTI_EX_ITEM_CHAT_MESSAGE_TYPE_SENDER) {
    LOGE("Invalid parameter");
    return NOTI_EX_ERROR_INVALID_PARAMETER;
  }

  p = new (std::nothrow) ChatMessageItem(id,
                static_cast<TextItem*>(name),
                static_cast<TextItem*>(text),
                static_cast<ImageItem*>(image),
                static_cast<TimeItem*>(time),
                static_cast<ChatMessageItem::Type>((int)message_type));
  if (p == nullptr) {
    LOGE("Out-of-memory");
    return NOTI_EX_ERROR_OUT_OF_MEMORY;
  }

  *handle = p;

  return NOTI_EX_ERROR_NONE;
#else
  return NOTI_EX_ERROR_NONE;
#endif
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

  // To do
  //ChatMessageItem* p = static_cast<ChatMessageItem*>(handle);
  //*message_type = p->GetMessageType();

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

  // To do
  //GroupItem* p = static_cast<GroupItem*>(handle);
  //p->AddChild(static_cast<AbstractItem*>(child));

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
  return 0;
}

extern "C" EXPORT_API int noti_ex_item_input_selector_get_contents(
    noti_ex_item_h handle, char ***list, int *count) {
  return 0;
}

extern "C" EXPORT_API int noti_ex_item_input_selector_set_contents(
    noti_ex_item_h handle, const char **contents, int count) {
  return 0;
}

extern "C" EXPORT_API int noti_ex_color_create(noti_ex_color_h *handle,
    unsigned char a, unsigned char r, unsigned char g, unsigned char b) {
  return 0;
}

extern "C" EXPORT_API int noti_ex_color_destroy(noti_ex_color_h handle) {
  return 0;
}

extern "C" EXPORT_API int noti_ex_color_get_alpha(noti_ex_color_h handle,
    unsigned char *val) {
  return 0;
}

extern "C" EXPORT_API int noti_ex_color_get_red(noti_ex_color_h handle,
    unsigned char *val) {
  return 0;
}

extern "C" EXPORT_API int noti_ex_color_get_green(noti_ex_color_h handle,
    unsigned char *val) {
  return 0;
}

extern "C" EXPORT_API int noti_ex_color_get_blue(noti_ex_color_h handle,
    unsigned char *val) {
  return 0;
}

extern "C" EXPORT_API int noti_ex_padding_create(noti_ex_padding_h *handle,
    int left, int top, int right, int bottom) {
  return 0;
}

extern "C" EXPORT_API int noti_ex_padding_destroy(noti_ex_padding_h handle) {
  return 0;
}

extern "C" EXPORT_API int noti_ex_padding_get_left(noti_ex_padding_h handle,
    int *val) {
  return 0;
}

extern "C" EXPORT_API int noti_ex_padding_get_top(noti_ex_padding_h handle,
    int *val) {
  return 0;
}

extern "C" EXPORT_API int noti_ex_padding_get_right(noti_ex_padding_h handle,
    int *val) {
  return 0;
}

extern "C" EXPORT_API int noti_ex_padding_get_bottom(noti_ex_padding_h handle,
    int *val) {
  return 0;
}

extern "C" EXPORT_API int noti_ex_geometry_create(noti_ex_geometry_h *handle,
    int x, int y, int w, int h) {
  return 0;
}

extern "C" EXPORT_API int noti_ex_geometry_destroy(noti_ex_geometry_h handle) {
  return 0;
}

extern "C" EXPORT_API int noti_ex_geometry_get_x(noti_ex_geometry_h handle,
    int *val) {
  return 0;
}

extern "C" EXPORT_API int noti_ex_geometry_get_y(noti_ex_geometry_h handle,
    int *val) {
  return 0;
}

extern "C" EXPORT_API int noti_ex_geometry_get_width(noti_ex_geometry_h handle,
    int *val) {
  return 0;
}

extern "C" EXPORT_API int noti_ex_geometry_get_height(noti_ex_geometry_h handle,
    int *val) {
  return 0;
}

extern "C" EXPORT_API int noti_ex_style_create(noti_ex_style_h *handle,
    noti_ex_color_h color,
    noti_ex_padding_h padding,
    noti_ex_geometry_h geometry) {
  return 0;
}

extern "C" EXPORT_API int noti_ex_style_destroy(noti_ex_style_h handle) {
  return 0;
}

extern "C" EXPORT_API int noti_ex_style_get_padding(noti_ex_style_h handle,
    const noti_ex_padding_h *padding) {
  return 0;
}

extern "C" EXPORT_API int noti_ex_style_get_color(noti_ex_style_h handle,
    const noti_ex_color_h *color) {
  return 0;
}

extern "C" EXPORT_API int noti_ex_style_get_geometry(noti_ex_style_h handle,
    const noti_ex_geometry_h *geometry) {
  return 0;
}

extern "C" EXPORT_API int noti_ex_led_info_create(noti_ex_led_info_h *handle,
    noti_ex_color_h color) {
  return 0;
}

extern "C" EXPORT_API int noti_ex_led_info_destroy(noti_ex_led_info_h handle) {
  return 0;
}

extern "C" EXPORT_API int noti_ex_led_info_set_on_period(
    noti_ex_led_info_h handle, int ms) {
  return 0;
}

extern "C" EXPORT_API int noti_ex_led_info_get_on_period(
    noti_ex_led_info_h handle, int *ms) {
  return 0;
}

extern "C" EXPORT_API int noti_ex_led_info_set_off_period(
    noti_ex_led_info_h handle, int ms) {
  return 0;
}

extern "C" EXPORT_API int noti_ex_led_info_get_off_period(
    noti_ex_led_info_h handle, int *ms) {
  return 0;
}

extern "C" EXPORT_API int noti_ex_led_info_get_color(
    noti_ex_led_info_h handle, noti_ex_color_h *color) {
  return 0;
}

extern "C" EXPORT_API int noti_ex_action_destroy(noti_ex_action_h handle) {
  return 0;
}

extern "C" EXPORT_API int noti_ex_action_get_type(noti_ex_action_h handle,
    int *type) {
  return 0;
}

extern "C" EXPORT_API int noti_ex_action_is_local(noti_ex_action_h handle,
    bool *local) {
  return 0;
}

extern "C" EXPORT_API int noti_ex_action_execute(noti_ex_action_h handle,
    noti_ex_item_h item) {
  return 0;
}

extern "C" EXPORT_API int noti_ex_action_get_extra(noti_ex_action_h handle,
    char **extra) {
  return 0;
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
    const char *id, const noti_ex_item_h *item) {
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
    const char *receiver_group, noti_ex_manager_events_s ev, void *data) {
  return 0;
}

extern "C" EXPORT_API int noti_ex_manager_deatroy(noti_ex_manager_h handle) {
  return 0;
}

extern "C" EXPORT_API int noti_ex_manager_get(noti_ex_manager_h handle,
    noti_ex_item_h *items, int *cnt) {
  return 0;
}

extern "C" EXPORT_API int noti_ex_manager_update(noti_ex_manager_h handle,
    noti_ex_item_h noti) {
  return 0;
}

extern "C" EXPORT_API int noti_ex_manager_delete(noti_ex_manager_h handle,
    noti_ex_item_h noti) {
  return 0;
}

extern "C" EXPORT_API int noti_ex_manager_delete_all(noti_ex_manager_h handle) {
  return 0;
}

extern "C" EXPORT_API int noti_ex_manager_hide(noti_ex_manager_h handle,
    noti_ex_item_h noti) {
  return 0;
}

extern "C" EXPORT_API int noti_ex_manager_find_by_root_id(
    noti_ex_manager_h handle, const char *id, noti_ex_item_h *item) {
  return 0;
}

extern "C" EXPORT_API int noti_ex_manager_send_event(noti_ex_manager_h handle,
    noti_ex_event_info_h info, noti_ex_item_h noti) {
  return 0;
}

extern "C" EXPORT_API int noti_ex_manager_send_error(noti_ex_manager_h handle,
    noti_ex_event_info_h info, noti_ex_error_e error) {
  return 0;
}

extern "C" EXPORT_API int noti_ex_manager_get_count(noti_ex_manager_h handle,
    int *cnt) {
  return 0;
}

extern "C" EXPORT_API int noti_ex_item_progress_create(noti_ex_item_h *handle,
    const char *id, float min, float current, float max) {
  return 0;
}

extern "C" EXPORT_API int noti_ex_item_progress_get_current(
    noti_ex_item_h handle, float *current) {
  return 0;
}

extern "C" EXPORT_API int noti_ex_item_progress_set_current(
    noti_ex_item_h handle, float current) {
  return 0;
}

extern "C" EXPORT_API int noti_ex_item_progress_get_min(noti_ex_item_h handle,
    float *min) {
  return 0;
}

extern "C" EXPORT_API int noti_ex_item_progress_get_max(noti_ex_item_h handle,
    float *max) {
  return 0;
}

extern "C" EXPORT_API int noti_ex_reporter_send_event(noti_ex_reporter_h handle,
    noti_ex_event_info_h info, noti_ex_item_h noti) {
  return 0;
}

extern "C" EXPORT_API int noti_ex_reporter_send_error(noti_ex_reporter_h handle,
    noti_ex_event_info_h info, noti_ex_error_e error) {
  return 0;
}

extern "C" EXPORT_API int noti_ex_reporter_post(noti_ex_reporter_h handle,
    noti_ex_item_h noti) {
  return 0;
}

extern "C" EXPORT_API int noti_ex_reporter_post_list(noti_ex_reporter_h handle,
    noti_ex_item_h *notiList, int cnt) {
  return 0;
}

extern "C" EXPORT_API int noti_ex_reporter_update(noti_ex_reporter_h handle,
    noti_ex_item_h noti) {
  return 0;
}

extern "C" EXPORT_API int noti_ex_reporter_delete(noti_ex_reporter_h handle,
    noti_ex_item_h noti) {
  return 0;
}

extern "C" EXPORT_API int noti_ex_reporter_delete_all(
    noti_ex_reporter_h handle) {
  return 0;
}

extern "C" EXPORT_API int noti_ex_reporter_find_by_root_id(
    noti_ex_reporter_h handle, const char *id, const noti_ex_item_h *item) {
  return 0;
}

extern "C" EXPORT_API int noti_ex_item_text_create(noti_ex_item_h *handle,
    const char *id, const char *text, const char *hyper_link) {
  return 0;
}

extern "C" EXPORT_API int noti_ex_item_text_set_contents(noti_ex_item_h handle,
    const char *contents) {
  return 0;
}

extern "C" EXPORT_API int noti_ex_item_text_get_contents(noti_ex_item_h handle,
    char **contents) {
  return 0;
}

extern "C" EXPORT_API int noti_ex_item_text_get_hyper_link(
    noti_ex_item_h handle, char **hyper_link) {
  return 0;
}

extern "C" EXPORT_API int noti_ex_item_time_create(noti_ex_item_h *handle,
    const char *id, time_t time) {
  return 0;
}

extern "C" EXPORT_API int noti_ex_item_time_get_time(noti_ex_item_h handle,
    time_t *time) {
  return 0;
}

extern "C" EXPORT_API int noti_ex_action_visibility_create(
    noti_ex_action_h *handle, const char *extra) {
  return 0;
}

extern "C" EXPORT_API int noti_ex_action_visibility_set(noti_ex_action_h handle,
    const char *id, bool visible) {
  return 0;
}
