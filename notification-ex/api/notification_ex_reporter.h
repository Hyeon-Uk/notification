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

#ifndef CAPI_NOTIFICATION_EX_REPORTER_H_
#define CAPI_NOTIFICATION_EX_REPORTER_H_

#include <notification_ex_error.h>
#include <notification_ex_item.h>
#include <notification_ex_event_info.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*noti_ex_reporter_events_error_cb)(noti_ex_error_e err, int requestId, void *data);
typedef void (*noti_ex_reporter_events_event_cb)(noti_ex_event_info_h info, const noti_ex_item_h* items, int cnt, void *data);

typedef void* noti_ex_reporter_h;
int noti_ex_reporter_create(noti_ex_reporter_h *handle,
		noti_ex_reporter_events_error_cb err, noti_ex_reporter_events_event_cb ev, void *data);
int noti_ex_reporter_destroy(noti_ex_reporter_h handle);

int noti_ex_reporter_send_event(noti_ex_reporter_h handle, noti_ex_event_info_h info, noti_ex_item_h noti);
int noti_ex_reporter_send_error(noti_ex_reporter_h handle, noti_ex_event_info_h info, noti_ex_error_e error);
int noti_ex_reporter_post(noti_ex_reporter_h handle, noti_ex_item_h noti);
int noti_ex_reporter_post_list(noti_ex_reporter_h handle, noti_ex_item_h *notiList, int cnt);
int noti_ex_reporter_update(noti_ex_reporter_h handle, noti_ex_item_h noti);
int noti_ex_reporter_delete(noti_ex_reporter_h handle, noti_ex_item_h noti);
int noti_ex_reporter_delete_all(noti_ex_reporter_h handle);
int noti_ex_reporter_find_by_root_id(noti_ex_reporter_h handle, const char *id, const noti_ex_item_h *item);

#ifdef __cplusplus
}
#endif
#endif  // CAPI_NOTIFICATION_EX_REPORTER_H_
