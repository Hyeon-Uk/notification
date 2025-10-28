/*
 * Copyright (c) 2000 - 2025 Samsung Electronics Co., Ltd. All rights reserved.
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

#ifndef __NOTIFICATION_IPC_H__
#define __NOTIFICATION_IPC_H__

#include <gio/gio.h>
#include <sys/types.h>

#include <notification.h>
#include <notification_list.h>

#ifdef __cplusplus
extern "C" {
#endif

GVariant *notification_ipc_make_gvariant_from_noti(notification_h noti, bool translate);
int notification_ipc_make_noti_from_gvariant(notification_h noti,
		GVariant *variant);

#ifdef __cplusplus
}
#endif
#endif /* __NOTIFICATION_IPC_H__ */

