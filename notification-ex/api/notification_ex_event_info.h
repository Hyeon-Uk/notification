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

#ifndef CAPI_NOTIFICATION_EX_EVENT_INFO_H_
#define CAPI_NOTIFICATION_EX_EVENT_INFO_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef enum _noti_ex_event_info_type {
	Post,
	Update,
	Delete,
	Get,
	Error,
} noti_ex_event_info_type_e;

typedef void* noti_ex_event_info_h;

int noti_ex_event_info_create(noti_ex_event_info_h *handle,
		noti_ex_event_info_type_e type, const char *owner,
		const char *channel, const char *item_id);
int noti_ex_event_info_destroy(noti_ex_event_info_h handle);
int noti_ex_event_info_get_event_type(noti_ex_event_info_h handle, noti_ex_event_info_type_e *event_type);
int noti_ex_event_info_get_owner(noti_ex_event_info_h handle, char **owner);
int noti_ex_event_info_get_channel(noti_ex_event_info_h handle, char **channel);
int noti_ex_event_info_get_item_id(noti_ex_event_info_h handle, char **item_id);
int noti_ex_event_info_get_request_id(noti_ex_event_info_h handle, int *req_id);

#ifdef __cplusplus
}
#endif
#endif  // CAPI_NOTIFICATION_EX_EVENT_INFO_H_
