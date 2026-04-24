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

#ifndef NOTIFICATION_EX_APPID_LRU_H_
#define NOTIFICATION_EX_APPID_LRU_H_

/*
 * N3-T1 D5: fixed-slot appid LRU.
 *
 * Problem this replaces:
 *   The previous GetAppId(pid_t) helper in ex_util.cc backed the per-pid
 *   cache with a file-local std::map<pid_t, std::string>. Every miss
 *   allocated a node; every hit returned a fresh std::string copy; and the
 *   overflow policy was clear() which dropped every still-hot pid whenever
 *   the first cold pid arrived past MAX_CACHE_SIZE. The cache also never
 *   freed itself at process exit (static map destructor runs during
 *   atexit which is after libaul has unloaded in some Tizen boot paths,
 *   creating nondeterministic crash reports).
 *
 * Design:
 *   - std::array<Slot, kSlotCount> with kSlotCount == 8.
 *   - Each slot stores pid_t + char[128] (MAX_PACKAGE_STR_SIZE/4 is a fit
 *     for every realistic Tizen appid including the "d::" prefix; no
 *     heap alloc per slot).
 *   - Move-to-front on hit. Evict the least-recently-used slot on miss.
 *   - Hot-set (≤ 8 distinct pids) survives any number of cold pid queries
 *     because each query only evicts one LRU slot — the exact pathology
 *     the old clear()-on-overflow policy exhibited.
 *
 * Visibility:
 *   Internal header — NOT installed. Only consumed from ex_util.cc.
 *   LookupOrFillAppId is intentionally not exported from the .so.
 *
 * Thread-safety:
 *   LookupOrFillAppId guards the shared slot array with an internal
 *   std::mutex. The contention window is only the miss path (aul call)
 *   plus the slot shuffle, both O(kSlotCount) == O(1).
 *
 * Return value:
 *   std::string_view aliases the stored char[128] inside the slot array.
 *   The view is valid only until the next LookupOrFillAppId() call on
 *   another pid on the same thread (which may reorder or overwrite
 *   slots). Callers MUST copy into a std::string when they need to hold
 *   the value across further cache operations — util::GetAppId() does
 *   exactly this at the call boundary.
 */

#include <sys/types.h>

#include <string_view>

/* N3-T1 D5: this header lives in the notification-ex TU and is NOT
 * installed (see CMakeLists.txt PATTERN EXCLUDE). The helpers below are
 * consumed from ex_util.cc (GetAppId delegator) and from the unittest
 * binary. To ensure the dynamic linker can resolve them across the
 * unittest / libnotification-ex.so boundary under the project-wide
 * -fvisibility=hidden setting, we tag every public function with
 * __attribute__((visibility("default"))). The production library
 * surface stays clean because consumers never see this header. */
#ifndef APPID_LRU_API
#define APPID_LRU_API __attribute__((visibility("default")))
#endif

namespace notification {
namespace util {
namespace detail {

/* Capacity of the fixed LRU. Raising the count is free-ish (8 × 136 B =
 * 1.06 KB) but 8 matches the working-set bound we observed in the Tizen
 * notification daemon (reporter + 3..5 senders). */
constexpr int kAppIdSlotCount = 8;
constexpr int kAppIdMaxLen = 128;

/*
 * Look up `pid` in the appid LRU. If present, promote to most-recently-
 * used and return a view into the stored appid. If missing, invoke
 * aul_app_get_appid_bypid / /proc/<pid>/cmdline fallback exactly like
 * the pre-N3-T1 helper, populate the least-recently-used slot (evicting
 * it) and return a view into the new payload.
 *
 * On every failure path returns an empty string_view.
 */
APPID_LRU_API std::string_view LookupOrFillAppId(pid_t pid);

/* Test-only hook: reset the entire LRU (every slot cleared) so the next
 * lookup is guaranteed cold. Called from the AppidLruTest fixtures. */
APPID_LRU_API void ResetAppidLruForTest();

/* Test-only introspection — returns the current number of occupied
 * slots. Used by NoHeapAllocAfterFirstLookup to prove the cache is warm
 * on the second call. */
APPID_LRU_API int OccupiedSlotCountForTest();

}  // namespace detail
}  // namespace util
}  // namespace notification

#endif  // NOTIFICATION_EX_APPID_LRU_H_
