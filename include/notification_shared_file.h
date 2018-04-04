/*
 * Copyright (c) 2017 Samsung Electronics Co., Ltd. All rights reserved.
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

#ifndef __NOTIFICATION_SHARED_FILE_H__
#define __NOTIFICATION_SHARED_FILE_H__

#include "notification.h"

#ifdef __cplusplus
extern "C" {
#endif

int notification_copy_private_file(const char* src_path, const char* dst_path);
char *notification_check_file_path_is_private(const char *pkg_id, const char *file_path);
int notification_set_private_sharing(notification_h noti, uid_t uid);
void notification_remove_private_sharing(const char *src_app_id, int priv_id, uid_t uid);
void notification_add_private_sharing_target_id(pid_t pid, const char *sender, uid_t uid);
void notification_remove_private_sharing_target_id(const char *sender, uid_t uid);
void notification_calibrate_private_sharing(notification_h updated_noti, notification_h source_noti);
bool notification_validate_private_sharing(notification_h updated_noti);

#ifdef __cplusplus
}
#endif
#endif /* __NOTIFICATION_SHARED_FILE_H__ */
