/*
 * Copyright (c) 2022 Samsung Electronics Co., Ltd.
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

#include "glib_mock.h"

#include "mock_hook.h"
#include "test_fixture.h"

extern "C" GFile* g_file_new_for_path(const char* arg1) {
  return MOCK_HOOK_P1(GlibMock, g_file_new_for_path, arg1);
}

extern "C" gboolean g_file_query_exists(GFile* arg1, GCancellable* arg2) {
  return MOCK_HOOK_P2(GlibMock, g_file_query_exists, arg1, arg2);
}

extern "C" gboolean g_file_make_directory(GFile* arg1, GCancellable* arg2,
    GError** arg3) {
  return MOCK_HOOK_P3(GlibMock, g_file_make_directory, arg1, arg2, arg3);
}

extern "C" int access(const char* arg1, int arg2) {
  return MOCK_HOOK_P2(GlibMock, access, arg1, arg2);
}
