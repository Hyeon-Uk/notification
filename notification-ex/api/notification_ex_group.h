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

#ifndef CAPI_NOTIFICATION_EX_GROUP_H_
#define CAPI_NOTIFICATION_EX_GROUP_H_

#include <notification_ex_item.h>

#ifdef __cplusplus
extern "C" {
#endif

int noti_ex_item_group_create(noti_ex_item_h *handle, const char *id);
int noti_ex_item_group_set_direction(noti_ex_item_h handle, bool vertical);
int noti_ex_item_group_is_vertical(noti_ex_item_h handle, bool *vertical);
int noti_ex_item_group_get_app_label(noti_ex_item_h handle, char **label);
int noti_ex_item_group_add_child(noti_ex_item_h handle, noti_ex_item_h child);
int noti_ex_item_group_remove_child(noti_ex_item_h handle, const char *item_id);

typedef int (*noti_ex_item_group_foreach_cb)(noti_ex_item_h handle, void *data);
int noti_ex_item_group_foreach(noti_ex_item_group_foreach_cb callback, void *data);

#ifdef __cplusplus
}
#endif
#endif  // CAPI_NOTIFICATION_EX_GROUP_H_
