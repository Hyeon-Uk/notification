/*
 * Copyright (c) 2023 Samsung Electronics Co., Ltd. All rights reserved.
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

#include <dlfcn.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <tzplatform_config.h>

static const char* s_override_db_dir = nullptr;
static char s_mkpath_buf[PATH_MAX];

static bool override_dir_valid() {
  if (!s_override_db_dir)
    return false;
  struct stat st;
  return stat(s_override_db_dir, &st) == 0 && S_ISDIR(st.st_mode);
}

extern "C" void test_set_db_dir(const char* dir) {
  s_override_db_dir = dir;
}

extern "C" const char* tzplatform_getenv(enum tzplatform_variable id) {
  if (id == TZ_SYS_DB && override_dir_valid())
    return s_override_db_dir;

  typedef const char* (*fn_t)(enum tzplatform_variable);
  static fn_t real_fn = (fn_t)dlsym(RTLD_NEXT, "tzplatform_getenv");
  return real_fn ? real_fn(id) : nullptr;
}

extern "C" const char* tzplatform_mkpath(enum tzplatform_variable id,
    const char* path) {
  if (id == TZ_SYS_DB && override_dir_valid()) {
    snprintf(s_mkpath_buf, sizeof(s_mkpath_buf), "%s/%s",
        s_override_db_dir, path);
    return s_mkpath_buf;
  }

  typedef const char* (*fn_t)(enum tzplatform_variable, const char*);
  static fn_t real_fn = (fn_t)dlsym(RTLD_NEXT, "tzplatform_mkpath");
  return real_fn ? real_fn(id, path) : nullptr;
}
