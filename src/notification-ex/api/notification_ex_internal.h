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

#ifndef __TIZEN_APPFW_NOTIFICATION_EX_INTERNAL_H__
#define __TIZEN_APPFW_NOTIFICATION_EX_INTERNAL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <notification_ex.h>

#define NOTI_EX_INPUT_SELECTOR_REPLY "__NOTI_EX_INPUT_SELECTOR_REPLY__"

int noti_ex_item_free_string_list(char** list, int count);
int noti_ex_item_group_remove_children(noti_ex_item_h handle);
int noti_ex_item_time_set_time(noti_ex_item_h handle, time_t time);
int noti_ex_item_get_private_id(noti_ex_item_h handle, int64_t* private_id);

int noti_ex_style_set_geometry(noti_ex_style_h handle,
		noti_ex_geometry_h geometry);
int noti_ex_style_set_color(noti_ex_style_h handle, noti_ex_color_h color);
int noti_ex_style_set_padding(noti_ex_style_h handle,
		noti_ex_padding_h padding);

int noti_ex_led_info_set_color(noti_ex_led_info_h handle,
		noti_ex_color_h color);

int noti_ex_item_icon_create(noti_ex_item_h *handle, const char *id,
		const char *icon_path);
int noti_ex_item_icon_get_icon_path(noti_ex_item_h handle, char **icon_path);

int noti_ex_item_button_set_image(noti_ex_item_h handle, const char *path);
int noti_ex_item_button_get_image(noti_ex_item_h handle, char **path);
int noti_ex_item_button_set_contents(noti_ex_item_h handle, const char *contents);
int noti_ex_item_button_get_contents(noti_ex_item_h handle, char **contents);

int noti_ex_reporter_find_by_channel(noti_ex_reporter_h handle,
		const char *channel, noti_ex_item_h **noti_list, int *count);
int noti_ex_reporter_find_all(noti_ex_reporter_h handle,
		noti_ex_item_h **noti_list, int *count);
int noti_ex_reporter_get_count_by_channel(noti_ex_reporter_h handle,
		const char *channel, int *count);
int noti_ex_reporter_delete_by_channel(noti_ex_reporter_h handle,
		const char *channel, int *request_id);
int noti_ex_reporter_update_list(noti_ex_reporter_h handle,
		noti_ex_item_h *noti_list, int count, int *request_id);
int noti_ex_reporter_delete_list(noti_ex_reporter_h handle,
		noti_ex_item_h *noti_list, int count, int *request_id);

int noti_ex_manager_delete_by_channel(noti_ex_manager_h handle,
		const char *channel, int *request_id);
int noti_ex_manager_delete_by_appid(noti_ex_manager_h handle,
		const char *app_id, int *request_id);

#ifdef __cplusplus
}
#endif
#endif  /* __TIZEN_APPFW_NOTIFICATION_EX_INTERNAL_H__ */
