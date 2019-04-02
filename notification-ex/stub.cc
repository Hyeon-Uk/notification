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
  return 0;
}

extern "C" EXPORT_API int noti_ex_action_app_control_set(
    noti_ex_action_h handle, app_control_h app_control) {
  return 0;
}

extern "C" EXPORT_API int noti_ex_action_app_control_get(
    noti_ex_action_h handle, app_control_h *app_control) {
  return 0;
}

extern "C" EXPORT_API int noti_ex_item_button_create(noti_ex_item_h *handle,
    const char *id, const char *title) {
  return 0;
}

extern "C" EXPORT_API int noti_ex_item_button_get_title(noti_ex_item_h handle,
    char **title) {
  return 0;
}

extern "C" EXPORT_API int noti_ex_item_chat_message_create(
    noti_ex_item_h *handle, const char *id, noti_ex_item_h name,
    noti_ex_item_h text, noti_ex_item_h image, noti_ex_item_h time,
    noti_ex_item_chat_message_type_e message_type) {
  return 0;
}

extern "C" EXPORT_API int noti_ex_item_chat_message_get_name(
    const noti_ex_item_h *text) {
  return 0;
}

extern "C" EXPORT_API int noti_ex_item_chat_message_get_text(
    const noti_ex_item_h *text) {
  return 0;
}

extern "C" EXPORT_API int noti_ex_item_chat_message_get_image(
    const noti_ex_item_h *image) {
  return 0;
}

extern "C" EXPORT_API int noti_ex_item_chat_message_get_time(
    const noti_ex_item_h *time) {
  return 0;
}

extern "C" EXPORT_API int noti_ex_item_chat_message_get_message_type(
    noti_ex_item_chat_message_type_e *message_type) {
  return 0;
}
