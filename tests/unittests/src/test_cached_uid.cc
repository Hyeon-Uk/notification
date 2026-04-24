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

/*
 * N3-T3 C3: regression guard for the aul_getuid() cache.
 *
 * Invariant under test:
 *   - notification_cached_uid() (C side) calls aul_getuid() exactly
 *     ONCE regardless of how many times it is invoked in the process.
 *   - notification::util::GetCachedUid() (C++ side) has the same
 *     property on a fresh process.
 *
 * Observability:
 *   tests/mock/aul_mock.cc keeps aul_getuid() as a constant-return
 *   extern "C" (so pre-existing TESTs that do not install an AulMock
 *   fixture continue to observe the historical 5001), but also
 *   increments an atomic counter on every call. The counter is
 *   exposed via noti_test_aul_getuid_call_count() /
 *   noti_test_aul_getuid_reset_count() so this test can snapshot
 *   and assert call counts without having to migrate every pre-N3-T3
 *   test to a MOCK_HOOK-based path.
 */

#include <gtest/gtest.h>

#include <sys/types.h>

#include "notification-ex/ex_util.h"

extern "C" {
/* Client-side cache: declared in notification_private.h behind the
 * installed-header curtain. We re-declare here because the private
 * header drags in a lot of GLib / bundle headers that are irrelevant
 * to the test. The signature is stable (uid_t, no args) and mismatches
 * would fail to link. */
uid_t notification_cached_uid(void);
void notification_cached_uid_reset_for_test(void);

/* Mock observability hooks — defined in tests/mock/aul_mock.cc. */
int noti_test_aul_getuid_call_count(void);
void noti_test_aul_getuid_reset_count(void);
}

namespace {

class CachedUidTest : public ::testing::Test {
 protected:
  void SetUp() override {
    /* Drop any cache state written by earlier TESTs so each case
     * starts from a known-cold baseline. The reset is the only way to
     * observe the first-call vs steady-state behaviour independent of
     * test execution order. */
    notification_cached_uid_reset_for_test();
    notification::util::ResetCachedUidForTest();
    noti_test_aul_getuid_reset_count();
  }
};

/*
 * C-side invariant: 100 calls → aul_getuid invoked exactly once.
 *
 * The mock returns 5001 unconditionally (historical constant used by
 * 140+ pre-existing TESTs). The cache helper therefore caches 5001
 * on the first call and must return it from the hot slot on every
 * subsequent call without re-entering aul_getuid.
 */
TEST_F(CachedUidTest, SingleAulCallPerProcess) {
  constexpr int kIters = 100;
  uid_t first = notification_cached_uid();
  for (int i = 0; i < kIters - 1; ++i) {
    uid_t u = notification_cached_uid();
    EXPECT_EQ(u, first) << "Cached uid must not change between calls";
  }
  EXPECT_EQ(noti_test_aul_getuid_call_count(), 1)
      << "notification_cached_uid() should invoke aul_getuid exactly once";
  /* The aul mock returns 5001; any other value means the cache
   * accidentally boxed (uid_t)-1 / 0 and the sentinel check is wrong. */
  EXPECT_EQ(first, static_cast<uid_t>(5001));
}

/*
 * C++-side invariant (notification::util::GetCachedUid).
 *
 * Same assertion as above but exercised from the notification-ex TU
 * side (abstract_item.cc / event_info.cc call sites).
 */
TEST_F(CachedUidTest, CppSideSingleAulCallPerProcess) {
  constexpr int kIters = 100;
  uid_t first = notification::util::GetCachedUid();
  for (int i = 0; i < kIters - 1; ++i) {
    uid_t u = notification::util::GetCachedUid();
    EXPECT_EQ(u, first);
  }
  EXPECT_EQ(noti_test_aul_getuid_call_count(), 1)
      << "util::GetCachedUid() should invoke aul_getuid exactly once";
  EXPECT_EQ(first, static_cast<uid_t>(5001));
}

/*
 * Reset semantics: after notification_cached_uid_reset_for_test() the
 * next call re-invokes aul_getuid (so the fixture's per-TEST_F cold
 * baseline is trustworthy and the cache is not a one-way door).
 */
TEST_F(CachedUidTest, ResetReArmsCache) {
  (void)notification_cached_uid();
  EXPECT_EQ(noti_test_aul_getuid_call_count(), 1);

  notification_cached_uid_reset_for_test();
  noti_test_aul_getuid_reset_count();

  (void)notification_cached_uid();
  EXPECT_EQ(noti_test_aul_getuid_call_count(), 1)
      << "After reset, the next call must re-enter aul_getuid";
}

}  // namespace
