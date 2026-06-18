/*
 * Copyright (c) 2026 Samsung Electronics Co., Ltd.
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

#include "libc_calloc_mock.hh"

#include <dlfcn.h>
#include <stddef.h>

static size_t g_calloc_fail_size = 0;
static bool g_calloc_fail_armed = false;

void calloc_set_fail_on_size(std::size_t target_size) {
  g_calloc_fail_size = target_size;
  g_calloc_fail_armed = true;
}

void calloc_reset_fail() {
  g_calloc_fail_armed = false;
  g_calloc_fail_size = 0;
}

extern "C" void *calloc(size_t nmemb, size_t size) {
  size_t total = nmemb * size;
  if (g_calloc_fail_armed && total == g_calloc_fail_size) {
    g_calloc_fail_armed = false;
    return nullptr;
  }
  using fn_t = void *(*)(size_t, size_t);
  static fn_t real_fn = nullptr;
  if (!real_fn)
    real_fn = reinterpret_cast<fn_t>(dlsym(RTLD_NEXT, "calloc"));
  return real_fn ? real_fn(nmemb, size) : nullptr;
}

static int g_malloc_fail_target = -1;
static int g_malloc_fail_count = 0;
static bool g_malloc_fail_armed = false;

void malloc_set_fail_on_call(int call_index) {
  g_malloc_fail_target = call_index;
  g_malloc_fail_count = 0;
  g_malloc_fail_armed = true;
}

void malloc_reset_fail() {
  g_malloc_fail_armed = false;
  g_malloc_fail_target = -1;
  g_malloc_fail_count = 0;
}

extern "C" void *malloc(size_t size) {
  if (g_malloc_fail_armed) {
    if (g_malloc_fail_count == g_malloc_fail_target) {
      g_malloc_fail_armed = false;
      return nullptr;
    }
    g_malloc_fail_count++;
  }
  using fn_t = void *(*)(size_t);
  static fn_t real_fn = nullptr;
  if (!real_fn)
    real_fn = reinterpret_cast<fn_t>(dlsym(RTLD_NEXT, "malloc"));
  return real_fn ? real_fn(size) : nullptr;
}
