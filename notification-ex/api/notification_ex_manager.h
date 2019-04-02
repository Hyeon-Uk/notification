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

#ifndef CAPI_NOTIFICATION_EX_MANAGER_H_
#define CAPI_NOTIFICATION_EX_MANAGER_H_

#include <notification_ex_error.h>
#include <notification_ex_item.h>
#include <notification_ex_event_info.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*noti_ex_manager_events_add_cb)(noti_ex_event_info_h info, noti_ex_item_h *added_item, int cnt, void *data);
typedef void (*noti_ex_manager_events_update_cb)(noti_ex_event_info_h info, noti_ex_item_h updated_item, void *data);
typedef void (*noti_ex_manager_events_delete_cb)(noti_ex_event_info_h info, noti_ex_item_h deleted_item, void *data);
typedef void (*noti_ex_manager_events_error_cb)(noti_ex_error_e error, int req_id, void *data);
typedef void (*noti_ex_manager_events_request_cb)(noti_ex_event_info_h info, noti_ex_item_h *items, int *cnt, void *data);

typedef struct {
	noti_ex_manager_events_add_cb added;
	noti_ex_manager_events_update_cb updated;
	noti_ex_manager_events_delete_cb deleted;
	noti_ex_manager_events_error_cb error;
	noti_ex_manager_events_request_cb requested;
} noti_ex_manager_events_s;

typedef void* noti_ex_manager_h;

int noti_ex_manager_create(noti_ex_manager_h *handle, const char *receiver_group, noti_ex_manager_events_s ev, void *data);
int noti_ex_manager_deatroy(noti_ex_manager_h handle);
int noti_ex_manager_get(noti_ex_manager_h handle, noti_ex_item_h *items, int *cnt);
int noti_ex_manager_update(noti_ex_manager_h handle, noti_ex_item_h noti);
int noti_ex_manager_delete(noti_ex_manager_h handle, noti_ex_item_h noti);
int noti_ex_manager_delete_all(noti_ex_manager_h handle);
int noti_ex_manager_hide(noti_ex_manager_h handle, noti_ex_item_h noti);
int noti_ex_manager_find_by_root_id(noti_ex_manager_h handle, const char *id, noti_ex_item_h *item);
int noti_ex_manager_send_event(noti_ex_manager_h handle, noti_ex_event_info_h info, noti_ex_item_h noti);
int noti_ex_manager_send_error(noti_ex_manager_h handle, noti_ex_event_info_h info, noti_ex_error_e error);
int noti_ex_manager_get_count(noti_ex_manager_h handle, int *cnt);

#ifdef __cplusplus
}
#endif
#endif  // CAPI_NOTIFICATION_EX_MANAGER_H_
