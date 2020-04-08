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
#include <gio/gio.h>

#ifdef __cplusplus
extern "C" {
#endif

DECLARE_FAKE_VALUE_FUNC(GFile*, g_file_new_for_path, const char*);
DECLARE_FAKE_VALUE_FUNC(gboolean, g_file_query_exists, GFile*, GCancellable*);
DECLARE_FAKE_VALUE_FUNC(gboolean, g_file_make_directory, GFile*, GCancellable*,
    GError**);
DECLARE_FAKE_VALUE_FUNC(int, access, const char*, int);

#ifdef __cplusplus
}
#endif

#endif /* MOCK_GIO_H_ */
