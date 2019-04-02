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

#ifndef CAPI_NOTIFICATION_EX_ITEM_H_
#define CAPI_NOTIFICATION_EX_ITEM_H_

#include <bundle.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum _noti_ex_item_type {
	NOTI_EX_ITEM_TYPE_NULL_OBJECT,
	NOTI_EX_ITEM_TYPE_TEXT,
	NOTI_EX_ITEM_TYPE_IMAGE,
	NOTI_EX_ITEM_TYPE_ICON,
	NOTI_EX_ITEM_TYPE_BUTTON,
	NOTI_EX_ITEM_TYPE_CHAT_MESSAGE,
	NOTI_EX_ITEM_TYPE_CHECKBOX,
	NOTI_EX_ITEM_TYPE_ICON_TEXT,
	NOTI_EX_ITEM_TYPE_INPUT_SELECTOR,
	NOTI_EX_ITEM_TYPE_GROUP,
	NOTI_EX_ITEM_TYPE_ENTRY,
	NOTI_EX_ITEM_TYPE_PROGRESS,
	NOTI_EX_ITEM_TYPE_TIME,
	NOTI_EX_ITEM_TYPE_CUSTOM = 100
} noti_ex_item_type_e;

typedef enum _noti_ex_item_policy {
	NOTI_EX_ITEM_POLICY_NONE = 0,
	NOTI_EX_ITEM_POLICY_ON_BOOT_CLEAR = 1 << 0,
	NOTI_EX_ITEM_POLICY_SIM_MODE = 1 << 1,
} noti_ex_item_policy_e;

typedef void* noti_ex_color_h;

int noti_ex_color_create(noti_ex_color_h *handle, unsigned char a, unsigned char r, unsigned char g, unsigned char b);
int noti_ex_color_destroy(noti_ex_color_h handle);
int noti_ex_color_get_alpha(noti_ex_color_h handle, unsigned char *val);
int noti_ex_color_get_red(noti_ex_color_h handle, unsigned char *val);
int noti_ex_color_get_green(noti_ex_color_h handle, unsigned char *val);
int noti_ex_color_get_blue(noti_ex_color_h handle, unsigned char *val);

typedef void* noti_ex_padding_h;
int noti_ex_padding_create(noti_ex_padding_h *handle, int left, int top, int right, int bottom);
int noti_ex_padding_destroy(noti_ex_padding_h handle);
int noti_ex_padding_get_left(noti_ex_padding_h handle, int *val);
int noti_ex_padding_get_top(noti_ex_padding_h handle, int *val);
int noti_ex_padding_get_right(noti_ex_padding_h handle, int *val);
int noti_ex_padding_get_bottom(noti_ex_padding_h handle, int *val);

typedef void* noti_ex_geometry_h;
int noti_ex_geometry_create(noti_ex_geometry_h *handle, int x, int y, int w, int h);
int noti_ex_geometry_destroy(noti_ex_geometry_h handle);
int noti_ex_geometry_get_x(noti_ex_geometry_h handle, int *val);
int noti_ex_geometry_get_y(noti_ex_geometry_h handle, int *val);
int noti_ex_geometry_get_width(noti_ex_geometry_h handle, int *val);
int noti_ex_geometry_get_height(noti_ex_geometry_h handle, int *val);

typedef void* noti_ex_style_h;
int noti_ex_style_create(noti_ex_style_h *handle, noti_ex_color_h color, noti_ex_padding_h padding, noti_ex_geometry_h geometry);
int noti_ex_style_destroy(noti_ex_style_h handle);

int noti_ex_style_get_padding(noti_ex_style_h handle, const noti_ex_padding_h *padding);
int noti_ex_style_get_color(noti_ex_style_h handle, const noti_ex_color_h *color);
int noti_ex_style_get_geometry(noti_ex_style_h handle, const noti_ex_geometry_h *geometry);

typedef void* noti_ex_led_info_h;
int noti_ex_led_info_create(noti_ex_led_info_h *handle, noti_ex_color_h color);
int noti_ex_led_info_destroy(noti_ex_led_info_h handle);
int noti_ex_led_info_set_on_period(noti_ex_led_info_h handle, int ms);
int noti_ex_led_info_get_on_period(noti_ex_led_info_h handle, int *ms);
int noti_ex_led_info_set_off_period(noti_ex_led_info_h handle, int ms);
int noti_ex_led_info_get_off_period(noti_ex_led_info_h handle, int *ms);
int noti_ex_led_info_get_color(noti_ex_led_info_h handle, noti_ex_color_h *color);

typedef void* noti_ex_action_h;
typedef void* noti_ex_item_h;
typedef void* noti_ex_item_info_h;

int noti_ex_action_destroy(noti_ex_action_h handle);
int noti_ex_action_get_type(noti_ex_action_h handle, int *type);
int noti_ex_action_is_local(noti_ex_action_h handle, bool *local);
int noti_ex_action_execute(noti_ex_action_h handle, noti_ex_item_h item);
int noti_ex_action_get_extra(noti_ex_action_h handle, char **extra);

int noti_ex_item_info_get_hide_time(noti_ex_item_info_h handle, int *hide_time);
int noti_ex_item_info_set_hide_time(noti_ex_item_info_h handle, int hide_time);
int noti_ex_item_info_get_delete_time(noti_ex_item_info_h handle, int *delete_time);
int noti_ex_item_info_set_delete_time(noti_ex_item_info_h handle, int delete_time);
int noti_ex_item_info_get_time(noti_ex_item_info_h handle, time_t *time);

int noti_ex_item_destroy(noti_ex_item_h handle);
int noti_ex_item_find_by_id(noti_ex_item_h handle, const char *id, const noti_ex_item_h *item);
int noti_ex_item_get_type(noti_ex_item_h handle, int *type);
int noti_ex_item_get_shared_path(noti_ex_item_h handle, char ***path, int *count);
int noti_ex_item_get_id(noti_ex_item_h handle, char **id);
int noti_ex_item_set_id(noti_ex_item_h handle, const char *id);
int noti_ex_item_get_action(noti_ex_item_h handle, noti_ex_action_h *action);
int noti_ex_item_set_action(noti_ex_item_h handle, noti_ex_action_h action);
int noti_ex_item_get_style(noti_ex_item_h handle, noti_ex_style_h *style);
int noti_ex_item_set_style(noti_ex_item_h handle, noti_ex_style_h style);
int noti_ex_item_set_visible(noti_ex_item_h handle, bool visible);
int noti_ex_item_get_visible(noti_ex_item_h handle, bool *visible);
int noti_ex_item_set_enable(noti_ex_item_h handle, bool enable);
int noti_ex_item_get_enable(noti_ex_item_h handle, bool *enable);
int noti_ex_item_add_receiver(noti_ex_item_h handle, const char *receiver_group);
int noti_ex_item_remove_receiver(noti_ex_item_h handle, const char *receiver_group);
int noti_ex_item_get_receiver_list(noti_ex_item_h handle, char ***list, int *count);
int noti_ex_item_set_policy(noti_ex_item_h handle, int policy);
int noti_ex_item_get_policy(noti_ex_item_h handle, int *policy);
int noti_ex_item_get_channel(noti_ex_item_h handle, char **channel);
int noti_ex_item_set_channel(noti_ex_item_h handle, const char *channel);
int noti_ex_item_set_led_info(noti_ex_item_h handle, noti_ex_led_info_h led);
int noti_ex_item_get_led_info(noti_ex_item_h handle, noti_ex_led_info_h *led);
int noti_ex_item_set_sound_path(noti_ex_item_h handle, const char *path);
int noti_ex_item_set_vibration_path(noti_ex_item_h handle, const char *path);
int noti_ex_item_get_sound_path(noti_ex_item_h handle, char **path);
int noti_ex_item_get_vibration_path(noti_ex_item_h handle, char **path);
int noti_ex_item_get_info(noti_ex_item_h handle, noti_ex_item_info_h *info);
int noti_ex_item_get_sender_app_id(noti_ex_item_h handle, char **id);
int noti_ex_item_set_sender_app_id(noti_ex_item_h handle, const char *id);
int noti_ex_item_get_tag(noti_ex_item_h handle, char **tag);
int noti_ex_item_set_tag(noti_ex_item_h handle, const char *tag);

#ifdef __cplusplus
}
#endif
#endif  // CAPI_NOTIFICATION_EX_ITEM_H_
