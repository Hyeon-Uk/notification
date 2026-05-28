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

/**
 * @internal
 * @brief Copies a source file into the notification-shared private location.
 * @param[in] src_path Source file path (absolute).
 * @param[in] dst_path Destination file path within the shared area.
 * @return NOTIFICATION_ERROR_NONE on success, otherwise an error code.
 */
int notification_copy_private_file(const char* src_path, const char* dst_path);

/**
 * @internal
 * @brief Returns the canonical shared path for @p file_path under @p pkg_id,
 *        or NULL if @p file_path is not a private path of @p pkg_id.
 * @param[in] pkg_id   Owning package id.
 * @param[in] file_path File path to verify.
 * @return Newly-allocated string (caller frees) or NULL.
 */
char *notification_check_file_path_is_private(const char *pkg_id, const char *file_path);

#ifdef __cplusplus
}
#endif
#endif /* __NOTIFICATION_SHARED_FILE_H__ */
