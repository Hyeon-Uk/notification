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

#include <sys/types.h>

#include "mock.h"
#include "app_common.h"
#include "gio_mock.h"
#include "smack_mock.h"
#include "tzplatform_config_mock.h"
#include "security_manager_mock.h"

DEFINE_FFF_GLOBALS;

DEFINE_FAKE_VALUE_FUNC(int, app_get_name, char**);

/* gio */
DEFINE_FAKE_VALUE_FUNC(GFile*, g_file_new_for_path, const char*);
DEFINE_FAKE_VALUE_FUNC(gboolean, g_file_query_exists, GFile*, GCancellable*);
DEFINE_FAKE_VALUE_FUNC(gboolean, g_file_copy, GFile*, GFile*, GFileCopyFlags,
    GCancellable*, GFileProgressCallback, gpointer, GError**);
DEFINE_FAKE_VALUE_FUNC(gboolean, g_file_make_directory, GFile*, GCancellable*,
    GError**);
DEFINE_FAKE_VALUE_FUNC(int, access, const char*, int);

/* smack */
DEFINE_FAKE_VALUE_FUNC(ssize_t, smack_new_label_from_path, const char*,
    const char*, int, char**);

/* tzplatform */
DEFINE_FAKE_VALUE_FUNC(const char*, tzplatform_getenv, enum tzplatform_variable);

/* security-manager */
DEFINE_FAKE_VALUE_FUNC(int, security_manager_private_sharing_req_new,
    private_sharing_req**);
DEFINE_FAKE_VALUE_FUNC(int, security_manager_private_sharing_req_add_paths,
    private_sharing_req*, const char**, size_t);
DEFINE_FAKE_VALUE_FUNC(int, security_manager_private_sharing_req_set_target_appid,
    private_sharing_req*, const char*);
DEFINE_FAKE_VALUE_FUNC(int, security_manager_private_sharing_apply,
    const private_sharing_req*);
DEFINE_FAKE_VALUE_FUNC(int, security_manager_private_sharing_drop,
    const private_sharing_req*);
