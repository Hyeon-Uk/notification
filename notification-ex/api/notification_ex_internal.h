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

int noti_ex_item_free_string_list(char** list, int count);
int noti_ex_item_group_remove_children(noti_ex_item_h handle);
int noti_ex_item_time_set_time(noti_ex_item_h handle, time_t time);

int noti_ex_style_set_geometry(noti_ex_style_h handle,
		noti_ex_geometry_h geometry);
int noti_ex_style_set_color(noti_ex_style_h handle, noti_ex_color_h color);
int noti_ex_style_set_padding(noti_ex_style_h handle,
		noti_ex_padding_h padding);

int noti_ex_led_info_set_color(noti_ex_led_info_h handle,
		noti_ex_color_h color);

#ifdef __cplusplus
}
#endif
#endif  /* __TIZEN_APPFW_NOTIFICATION_EX_INTERNAL_H__ */
