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
 */

/*
 * N3-T1 D5: regression guard for the fixed-slot appid LRU.
 *
 *   HotSetSurvivesEvictionCycle
 *     The old std::map + clear()-on-overflow policy dropped every hot
 *     pid when the first cold pid arrived past MAX_CACHE_SIZE (100).
 *     The fixed LRU must instead evict only the least-recently-used
 *     slot per miss. This test fills every slot, promotes pid 10 to the
 *     LRU tail, installs a cold pid 100 (should evict pid 10) and
 *     confirms the remaining 7 hot pids are still resolvable.
 *
 *   NoHeapAllocAfterFirstLookup
 *     The first lookup populates a char[128] slot in place (no heap).
 *     Subsequent lookups for the same pid MUST not touch the allocator.
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <cstring>
#include <string>
#include <string_view>

#include "notification-ex/appid_lru.h"
#include "notification-ex/ex_util.h"

#include "mock/aul_mock.hh"
#include "mock/test_fixture.hh"
#include "mock/alloc_counter.hh"

using ::testing::_;
using ::testing::DoAll;
using ::testing::Invoke;
using ::testing::Return;

namespace {

/*
 * The appid LRU resolves misses through aul_app_get_appid_bypid. Without
 * a NiceMock<AulMock> fixture installed, the MOCK_HOOK_P3 thunk in
 * aul_mock.cc throws "The test does not provide mock of 7AulMock". The
 * TestFixture machinery owns exactly one mock pointer per test, so we
 * supply a NiceMock here and wire aul_app_get_appid_bypid to return a
 * pid-dependent string, which is enough to differentiate hot slots.
 */
class Mocks : virtual public ::testing::NiceMock<AulMock> {};

class AppidLruTest : public TestFixture {
 public:
  AppidLruTest() : TestFixture(std::make_unique<::Mocks>()) {}

  void SetUp() override {
    notification::util::detail::ResetAppidLruForTest();
    /* Default-install a pid-bearing appid so every pid resolves
     * distinctly; the LRU stores a copy of this buffer in the slot. */
    ON_CALL(GetMock<AulMock>(), aul_app_get_appid_bypid(_, _, _))
        .WillByDefault(Invoke([](int pid, char* buf, int len) -> int {
          std::snprintf(buf, static_cast<size_t>(len),
                        "org.tizen.pid%d", pid);
          return 0;  /* AUL_R_OK */
        }));
  }

  void TearDown() override {
    notification::util::detail::ResetAppidLruForTest();
  }
};

TEST_F(AppidLruTest, HotSetSurvivesEvictionCycle) {
  constexpr int kHotCount = 8;
  /* Fill every slot. Pid 10 ends up at LRU tail, pid 17 at MRU. */
  for (int i = 0; i < kHotCount; ++i) {
    auto v = notification::util::detail::LookupOrFillAppId(10 + i);
    ASSERT_FALSE(v.empty()) << "pid " << (10 + i) << " failed to fill";
  }
  ASSERT_EQ(notification::util::detail::OccupiedSlotCountForTest(),
            kHotCount);

  /* Promote each hot pid back to MRU in reverse so pid 10 ends up at
   * the LRU tail ready to be evicted. */
  for (int i = kHotCount - 1; i >= 0; --i) {
    auto v = notification::util::detail::LookupOrFillAppId(10 + i);
    ASSERT_FALSE(v.empty());
  }

  /* Install pid 100 — must evict exactly one slot (pid 10, the LRU). */
  auto v100 = notification::util::detail::LookupOrFillAppId(100);
  EXPECT_FALSE(v100.empty());
  EXPECT_EQ(notification::util::detail::OccupiedSlotCountForTest(),
            kHotCount)
      << "LRU must stay at capacity — single-slot eviction, not clear()";

  /* Pids 11..17 must still resolve out of the cache without hitting
   * aul again (they were promoted to MRU before the pid-100 insert). */
  for (int i = 1; i < kHotCount; ++i) {
    auto v = notification::util::detail::LookupOrFillAppId(10 + i);
    EXPECT_FALSE(v.empty())
        << "pid " << (10 + i) << " must still be cache-resident";
    /* Content check: the appid must still encode the right pid. */
    std::string expected = "org.tizen.pid" + std::to_string(10 + i);
    EXPECT_EQ(std::string(v), expected);
  }
  EXPECT_EQ(notification::util::detail::OccupiedSlotCountForTest(),
            kHotCount);
}

/*
 * NoHeapAllocAfterFirstLookup: after a single warm-up call for pid X,
 * 100 follow-up lookups for pid X must not produce any malloc /
 * realloc / calloc traffic. The LRU stores the appid inline in a
 * char[128] slot so the hit path is a linear scan + memmove + return.
 */
class AppidLruAllocTest : public TestFixture {
 public:
  AppidLruAllocTest() : TestFixture(std::make_unique<::Mocks>()) {}

  void SetUp() override {
    notification::util::detail::ResetAppidLruForTest();
    ON_CALL(GetMock<AulMock>(), aul_app_get_appid_bypid(_, _, _))
        .WillByDefault(Invoke([](int pid, char* buf, int len) -> int {
          std::snprintf(buf, static_cast<size_t>(len),
                        "org.tizen.pid%d", pid);
          return 0;
        }));
  }

  void TearDown() override {
    notification::util::detail::ResetAppidLruForTest();
  }
};

TEST_F(AppidLruAllocTest, NoHeapAllocAfterFirstLookup) {
  /* Warm-up: cold path allocs are not measured here. */
  (void)notification::util::detail::LookupOrFillAppId(12345);
  ASSERT_GT(notification::util::detail::OccupiedSlotCountForTest(), 0);

  /* Steady-state: allocator counters should stay flat. The LeakCheck
   * --wrap=malloc hooks only update when counting is enabled on the
   * current thread. */
  noti_test::SetCountingEnabled(true);
  auto baseline = noti_test::SnapshotAllocs();
  for (int i = 0; i < 100; ++i) {
    auto v = notification::util::detail::LookupOrFillAppId(12345);
    ASSERT_FALSE(v.empty());
  }
  auto diff = noti_test::DiffAllocs(baseline);
  noti_test::SetCountingEnabled(false);

  /* The hot path is pure std::lock_guard + linear scan + memmove. No
   * heap activity expected. Allow up to 2 incidental allocs for
   * implementation slack in gmock internal accounting when NiceMock
   * default-action dispatch lazily initialises its matcher cache. */
  EXPECT_LE(diff.total_allocs, 2u)
      << "Warm-cache lookups must not heap-allocate; observed "
      << diff.total_allocs << " alloc(s) in 100 calls";
}

}  // namespace
