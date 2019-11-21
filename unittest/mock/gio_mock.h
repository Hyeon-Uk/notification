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

#ifndef MOCK_GIO_H_
#define MOCK_GIO_H_

#include "mock.h"
#include <glib.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _GFile GFile;
typedef struct _GError GError;
typedef void GDBusConnection;
typedef void GCancellable;
typedef void* gpointer;
typedef int gboolean;

typedef enum {
  G_FILE_COPY_NONE                 = 0,
  G_FILE_COPY_OVERWRITE            = (1 << 0),
  G_FILE_COPY_BACKUP               = (1 << 1),
  G_FILE_COPY_NOFOLLOW_SYMLINKS    = (1 << 2),
  G_FILE_COPY_ALL_METADATA         = (1 << 3),
  G_FILE_COPY_NO_FALLBACK_FOR_MOVE = (1 << 4),
  G_FILE_COPY_TARGET_DEFAULT_PERMS = (1 << 5)
} GFileCopyFlags;


typedef void (*GFileProgressCallback) (goffset current_num_bytes,
    goffset total_num_bytes, gpointer user_data);

DECLARE_FAKE_VALUE_FUNC(GFile*, g_file_new_for_path, const char*);
DECLARE_FAKE_VALUE_FUNC(gboolean, g_file_query_exists, GFile*, GCancellable*);
DECLARE_FAKE_VALUE_FUNC(gboolean, g_file_make_directory, GFile*, GCancellable*,
    GError**);
DECLARE_FAKE_VALUE_FUNC(int, access, const char*, int);

#ifdef __cplusplus
}
#endif

#endif /* MOCK_GIO_H_ */
