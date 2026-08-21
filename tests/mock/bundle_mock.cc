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

#include "bundle_mock.hh"

#include <dlfcn.h>
#include <stdexcept>

#include "mock_hook.hh"
#include "test_fixture.hh"

static int g_bundle_create_fail_on_call = -1;
static int g_bundle_create_call_count = 0;

void bundle_create_set_fail_on(int call_index) {
  g_bundle_create_fail_on_call = call_index;
  g_bundle_create_call_count = 0;
}

void bundle_create_reset_fail() {
  g_bundle_create_fail_on_call = -1;
  g_bundle_create_call_count = 0;
}

extern "C" bundle *bundle_create(void) {
  int call_n = g_bundle_create_call_count++;
  if (g_bundle_create_fail_on_call >= 0 && call_n == g_bundle_create_fail_on_call)
    return nullptr;
  using fn_t = bundle *(*)();
  static fn_t real_fn = nullptr;
  if (!real_fn)
    real_fn = reinterpret_cast<fn_t>(dlsym(RTLD_NEXT, "bundle_create"));
  return real_fn ? real_fn() : nullptr;
}

extern "C" int bundle_encode(bundle *b, bundle_raw **r, int *len) {
  try {
    return MOCK_HOOK_P3(BundleMock, bundle_encode, b, r, len);
  } catch (const std::invalid_argument&) {
    using fn_t = int (*)(bundle*, bundle_raw**, int*);
    static fn_t real =
        reinterpret_cast<fn_t>(dlsym(RTLD_NEXT, "bundle_encode"));
    return real ? real(b, r, len) : BUNDLE_ERROR_INVALID_PARAMETER;
  }
}

extern "C" int bundle_add_str(bundle *b, const char *key, const char *str) {
  try {
    return MOCK_HOOK_P3(BundleMock, bundle_add_str, b, key, str);
  } catch (const std::invalid_argument&) {
    using fn_t = int (*)(bundle*, const char*, const char*);
    static fn_t real =
        reinterpret_cast<fn_t>(dlsym(RTLD_NEXT, "bundle_add_str"));
    return real ? real(b, key, str) : BUNDLE_ERROR_INVALID_PARAMETER;
  }
}
