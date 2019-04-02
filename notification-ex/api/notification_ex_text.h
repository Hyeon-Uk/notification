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

#ifndef CAPI_NOTIFICATION_EX_TEXT_H_
#define CAPI_NOTIFICATION_EX_TEXT_H_

#include <notification_ex_item.h>

#ifdef __cplusplus
extern "C" {
#endif

int noti_ex_item_text_create(noti_ex_item_h *handle, const char *id, const char *text, const char *hyper_link);
int noti_ex_item_text_set_contents(noti_ex_item_h handle, const char *contents);
int noti_ex_item_text_get_contents(noti_ex_item_h handle, char **contents);
int noti_ex_item_text_get_hyper_link(noti_ex_item_h handle, char **hyper_link);

#ifdef __cplusplus
}
#endif
#endif  // CAPI_NOTIFICATION_EX_TEXT_H_
