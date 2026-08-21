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

#include "vconf_mock.hh"

#include <dlfcn.h>
#include <stdexcept>

#include "mock_hook.hh"
#include "test_fixture.hh"

extern "C" int vconf_notify_key_changed(const char *in_key,
    vconf_callback_fn cb, void *user_data) {
  try {
    return MOCK_HOOK_P3(VconfMock, vconf_notify_key_changed, in_key, cb, user_data);
  } catch (const std::invalid_argument&) {
    using fn_t = int (*)(const char*, vconf_callback_fn, void*);
    static fn_t real =
        reinterpret_cast<fn_t>(dlsym(RTLD_NEXT, "vconf_notify_key_changed"));
    return real ? real(in_key, cb, user_data) : -1;
  }
}
