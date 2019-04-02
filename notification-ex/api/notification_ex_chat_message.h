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

#ifndef CAPI_NOTIFICATION_EX_CHAT_MESSAGE_H_
#define CAPI_NOTIFICATION_EX_CHAT_MESSAGE_H_

#include <notification_ex_item.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum _noti_ex_item_chat_message_type {
	NOTI_EX_ITEM_CHAT_MESSAGE_TYPE_USER,
	NOTI_EX_ITEM_CHAT_MESSAGE_TYPE_SENDER,
} noti_ex_item_chat_message_type_e;

int noti_ex_item_chat_message_create(noti_ex_item_h *handle, const char *id, noti_ex_item_h name,
		noti_ex_item_h text, noti_ex_item_h image, noti_ex_item_h time, noti_ex_item_chat_message_type_e message_type);
int noti_ex_item_chat_message_get_name(const noti_ex_item_h *text);
int noti_ex_item_chat_message_get_text(const noti_ex_item_h *text);
int noti_ex_item_chat_message_get_image(const noti_ex_item_h *image);
int noti_ex_item_chat_message_get_time(const noti_ex_item_h *time);
int noti_ex_item_chat_message_get_message_type(noti_ex_item_chat_message_type_e *message_type);

#ifdef __cplusplus
}
#endif
#endif  // CAPI_NOTIFICATION_EX_CHAT_MESSAGE_H_
