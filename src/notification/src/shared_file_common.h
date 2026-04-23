/*
 * Copyright (c) 2026 Samsung Electronics Co., Ltd. All rights reserved.
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

#ifndef NOTIFICATION_SHARED_FILE_COMMON_H_
#define NOTIFICATION_SHARED_FILE_COMMON_H_

#include <stdbool.h>
#include <sys/types.h>

/*
 * N2-T6: shared_file duplication removal (DD1).
 *
 * src/notification/src/notification_shared_file.c and
 * src/notification-ex/shared_file.cc each reimplemented the same five
 * filesystem / security helpers (right-suffix search, mkdir-if-missing,
 * RO-mount detection, single-file copy with mtime preservation, per-app
 * data-path composition).  This header exposes a single C surface that
 * both libraries compile against via a CMake object library.
 *
 * Scope:
 *   - Internal-only. Not installed, not EXPORT_API, not part of any public
 *     ABI. Consumers: libnotification.so and libnotification-ex.so only.
 *   - All helpers are pure functions with no global state.
 *
 * Thread safety:
 *   - Each helper is reentrant; they do not touch shared mutable data.
 *     The underlying libc / glib / libtzplatform calls follow their own
 *     (documented) threading contracts.
 *
 * Error model:
 *   - *_copy_file / *_make_dir return an int that matches the notification
 *     error enum (NOTIFICATION_ERROR_NONE on success,
 *     NOTIFICATION_ERROR_IO_ERROR / _ALREADY_EXIST_ID on failure).
 *     Keeping integer codes instead of bool lets the C++ wrapper pass
 *     them through untouched (ex error namespace maps 1:1 to these).
 */

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Returns a pointer into `path` where the last occurrence of `search`
 * starts, or NULL if `search` is not a suffix match anywhere from right
 * to left.  Accepts NULL input (returns NULL).
 */
const char *shared_file_common_last_index_of(const char *path,
                                             const char *search);

/*
 * Creates directory `dir` if it does not already exist (R_OK probe +
 * g_file_make_directory). Returns true on success (including
 * already-exists), false on creation failure.
 */
bool shared_file_common_make_dir(const char *dir);

/*
 * Returns true when `file_path` lives on a read-only filesystem
 * (statvfs ST_RDONLY). Intended to flag resources that must be copied
 * into a per-app writable location before they can be shared.
 * Returns false on stat failure or writable filesystems.
 */
bool shared_file_common_is_private_path(const char *file_path);

/*
 * Copies `src_path` to `dst_path` with G_FILE_COPY_NOFOLLOW_SYMLINKS,
 * then updates the destination's mtime to now.  Returns one of:
 *   0                                    -> success
 *   NOTIFICATION_ERROR_ALREADY_EXIST_ID  -> dst already existed
 *   NOTIFICATION_ERROR_IO_ERROR          -> src/dst path invalid or
 *                                           g_file_copy failed
 * The numeric values are taken from <notification_error.h>; keeping
 * the return type plain int lets the ex/C++ side reinterpret into its
 * own error namespace without coupling this header to either enum.
 */
int shared_file_common_copy_file(const char *src_path, const char *dst_path);

/*
 * Builds <TZ_USER_APP>/<app_id>/<subdir>/<basename(file_path)> as a
 * newly-allocated C string. Callers free() the result.  Returns NULL
 * on invalid arguments or OOM.
 *
 * `subdir` is the per-library relative private-data directory
 * ("data/.notification" or "data/.notification_ex"); passing it in
 * keeps this helper neutral between the two callers.
 */
char *shared_file_common_get_data_path(const char *app_id,
                                       const char *file_path,
                                       const char *subdir);

#ifdef __cplusplus
}
#endif

#endif  /* NOTIFICATION_SHARED_FILE_COMMON_H_ */
