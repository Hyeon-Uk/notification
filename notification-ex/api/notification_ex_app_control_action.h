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

#ifndef CAPI_NOTIFICATION_EX_APP_CONTROL_ACTION_H_
#define CAPI_NOTIFICATION_EX_APP_CONTROL_ACTION_H_

#include <app_control.h>
#include <notification_ex_item.h>

#ifdef __cplusplus
extern "C" {
#endif

int noti_ex_action_app_control_create(noti_ex_action_h *handle, app_control_h app_control, const char *extra);
int noti_ex_action_app_control_set(noti_ex_action_h handle, app_control_h app_control);
int noti_ex_action_app_control_get(noti_ex_action_h handle, app_control_h *app_control);

#ifdef __cplusplus
}
#endif
#endif  // CAPI_NOTIFICATION_EX_APP_CONTROL_ACTION_H_
