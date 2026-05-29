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

/**
 * @file notification_ipc.h
 * @brief GVariant marshalling helpers used to ship a notification handle
 *        across the IPC boundary.
 */

#include <gio/gio.h>
#include <sys/types.h>

#include <notification.h>
#include <notification_list.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @internal
 * @brief Serializes @p noti to a GVariant suitable for the daemon RPC.
 * @param[in] noti      Notification handle to encode.
 * @param[in] translate Apply text-domain translation before encoding.
 * @return Newly-created GVariant (caller takes the reference).
 */
GVariant *notification_ipc_make_gvariant_from_noti(notification_h noti, bool translate);

/**
 * @internal
 * @brief Populates @p noti from a GVariant produced by the daemon side.
 * @param[in,out] noti    Notification handle to fill in-place.
 * @param[in]     variant GVariant payload from the daemon.
 * @return NOTIFICATION_ERROR_NONE on success.
 */
int notification_ipc_make_noti_from_gvariant(notification_h noti,
		GVariant *variant);

#ifdef __cplusplus
}
#endif
#endif /* __NOTIFICATION_IPC_H__ */
