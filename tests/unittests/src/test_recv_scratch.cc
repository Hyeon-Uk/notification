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
 * N3-T1 D6: regression guard for the thread-local receive scratch
 * buffer (util::EnsureRecvBuf / RecvBufCapacity).
 *
 * Invariant under test (HighWatermarkRetained):
 *   - EnsureRecvBuf(1KB) returns a 1KB buffer (cold path: one alloc).
 *   - EnsureRecvBuf(4KB) grows to 4KB (one realloc).
 *   - EnsureRecvBuf(512B) reuses the 4KB buffer unchanged — capacity
 *     is high-watermark, NOT shrunk. No alloc, no realloc.
 *   - Subsequent EnsureRecvBuf(1024) ≤ cap also reuses.
 *
 * This replaces the per-call calloc() pattern that lived in
 * ReadBundleList() pre-N3-T1.
 */

#include <gtest/gtest.h>

#include <cstddef>

#include "notification-ex/ex_util.h"
#include "mock/test_fixture.hh"
#include "mock/alloc_counter.hh"

namespace {

class RecvScratchTest : public ::testing::Test {
 protected:
  void SetUp() override {
    notification::util::ResetRecvBufForTest();
  }
  void TearDown() override {
    notification::util::ResetRecvBufForTest();
  }
};

TEST_F(RecvScratchTest, HighWatermarkRetained) {
  /* Cold start: no capacity yet. */
  EXPECT_EQ(notification::util::RecvBufCapacity(), 0u);

  /* First grow to 1 KB. */
  char* p1 = notification::util::EnsureRecvBuf(1024);
  ASSERT_NE(p1, nullptr);
  EXPECT_GE(notification::util::RecvBufCapacity(), 1024u);
  size_t cap_after_1k = notification::util::RecvBufCapacity();

  /* Second grow to 4 KB must expand capacity. */
  char* p4 = notification::util::EnsureRecvBuf(4096);
  ASSERT_NE(p4, nullptr);
  EXPECT_GE(notification::util::RecvBufCapacity(), 4096u);
  size_t cap_after_4k = notification::util::RecvBufCapacity();
  EXPECT_GT(cap_after_4k, cap_after_1k);

  /* Shrink request must NOT shrink the buffer — high-watermark retained. */
  char* p_small = notification::util::EnsureRecvBuf(512);
  ASSERT_NE(p_small, nullptr);
  EXPECT_EQ(notification::util::RecvBufCapacity(), cap_after_4k)
      << "EnsureRecvBuf must not shrink the scratch buffer";
  EXPECT_EQ(p_small, p4)
      << "A shrink request must return the already-allocated buffer";

  /* Another request below cap must reuse without realloc. */
  char* p_reuse = notification::util::EnsureRecvBuf(1024);
  ASSERT_NE(p_reuse, nullptr);
  EXPECT_EQ(p_reuse, p4);
  EXPECT_EQ(notification::util::RecvBufCapacity(), cap_after_4k);
}

/* Alloc-accounting check: after the initial grow sequence, follow-up
 * sub-capacity requests must not produce any malloc/realloc activity. */
class RecvScratchAllocTest : public ::testing::Test {
 protected:
  void SetUp() override {
    notification::util::ResetRecvBufForTest();
  }
  void TearDown() override {
    noti_test::SetCountingEnabled(false);
    notification::util::ResetRecvBufForTest();
  }
};

TEST_F(RecvScratchAllocTest, ReuseBelowCapacityDoesNotRealloc) {
  /* Warm: first alloc (cold path), second grows. */
  (void)notification::util::EnsureRecvBuf(1024);
  (void)notification::util::EnsureRecvBuf(4096);

  /* Steady state: 50 × below-cap requests must produce no allocator
   * activity. */
  auto baseline = noti_test::SnapshotAllocs();
  for (int i = 0; i < 50; ++i) {
    (void)notification::util::EnsureRecvBuf(1024);
  }
  auto diff = noti_test::DiffAllocs(baseline);
  EXPECT_EQ(diff.total_allocs, 0u)
      << "Sub-capacity EnsureRecvBuf must not touch the allocator; "
         "observed " << diff.total_allocs << " allocs in 50 calls";
}

}  // namespace
