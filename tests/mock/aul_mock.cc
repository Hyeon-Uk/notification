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

#include "aul_mock.hh"

#include <atomic>

#include "mock_hook.hh"
#include "test_fixture.hh"

namespace {
/*
 * N3-T3 C3: per-process call counter for aul_getuid().
 *
 * The mock previously returned a constant 5001 without observability,
 * so the N3-T3 "cache keeps aul_getuid() calls at 1" invariant could not
 * be asserted. We keep the constant-return default (to avoid rewriting
 * every pre-existing TEST to install an AulMock fixture) and expose a
 * standalone atomic counter plus reset helper that CachedUidTest drives
 * directly.
 */
std::atomic<int> g_aul_getuid_call_count{0};
}  // namespace

extern "C" uid_t aul_getuid() {
  g_aul_getuid_call_count.fetch_add(1, std::memory_order_relaxed);
  return 5001;
}

/* Test-only observability hooks — consumed by test_cached_uid.cc. */
extern "C" int noti_test_aul_getuid_call_count() {
  return g_aul_getuid_call_count.load(std::memory_order_relaxed);
}

extern "C" void noti_test_aul_getuid_reset_count() {
  g_aul_getuid_call_count.store(0, std::memory_order_relaxed);
}

extern "C" int aul_app_get_appid_bypid(int pid, char* appid, int len) {
  return MOCK_HOOK_P3(AulMock, aul_app_get_appid_bypid, pid, appid, len);
}

extern "C" int aul_app_get_pkgid_bypid(int pid, char* pkgid, int len) {
  return MOCK_HOOK_P3(AulMock, aul_app_get_pkgid_bypid, pid, pkgid, len);
}
