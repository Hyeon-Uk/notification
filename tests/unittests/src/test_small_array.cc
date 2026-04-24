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
 * N3-T5 A4: regression guard for util::SmallArray<T, N> — the stack-
 * buffered scratch array with heap fallback that replaces the per-call
 * calloc() in the reporter-event invocation path in stub.cc.
 *
 * Invariants under test:
 *   StackBufferUnderCapacity:
 *     - An N-sized allocation when n <= N stays on the stack. No heap
 *       alloc is observable via data()'s address being inside the
 *       SmallArray object itself.
 *     - Elements are zero-initialised (matching calloc).
 *   HeapFallbackOverCapacity:
 *     - An allocation with n > N falls back to calloc(). data() points
 *       OUTSIDE the SmallArray object.
 *     - Elements are zero-initialised and the dtor free()s without
 *       leaking (caught by --wrap=calloc/free counters).
 */

#include <gtest/gtest.h>

#include <cstddef>
#include <cstdint>

#include "notification-ex/small_array.h"
#include "mock/test_fixture.hh"
#include "mock/alloc_counter.hh"

namespace {

/* Test fixture: no mock object needed — SmallArray only uses calloc/free
 * via libc. The alloc_counter wrap in the test binary catches those. */
class SmallArrayTest : public ::testing::Test {};

using notification::util::SmallArray;

TEST_F(SmallArrayTest, StackBufferUnderCapacity) {
  constexpr std::size_t kN = 16;
  SmallArray<void*, kN> arr(8);

  ASSERT_TRUE(arr.alloc_ok());
  ASSERT_TRUE(arr.on_stack());
  EXPECT_EQ(arr.size(), 8u);

  /* Zero-init contract — every element is a null pointer. */
  for (std::size_t i = 0; i < arr.size(); ++i)
    EXPECT_EQ(arr[i], nullptr);

  /* data() must lie inside the SmallArray object for the stack path. */
  auto* self = reinterpret_cast<const std::uint8_t*>(&arr);
  auto* d = reinterpret_cast<const std::uint8_t*>(arr.data());
  EXPECT_GE(d, self);
  EXPECT_LT(d, self + sizeof(arr));

  /* Writes survive until dtor. */
  int sentinel = 42;
  arr[0] = &sentinel;
  arr[7] = &sentinel;
  EXPECT_EQ(arr[0], &sentinel);
  EXPECT_EQ(arr[7], &sentinel);
}

TEST_F(SmallArrayTest, StackBufferAtExactCapacity) {
  constexpr std::size_t kN = 16;
  SmallArray<int, kN> arr(kN);  /* n == N is still stack */
  ASSERT_TRUE(arr.alloc_ok());
  EXPECT_TRUE(arr.on_stack());
  for (std::size_t i = 0; i < kN; ++i)
    EXPECT_EQ(arr[i], 0);
}

TEST_F(SmallArrayTest, HeapFallbackOverCapacity) {
  constexpr std::size_t kN = 16;
  SmallArray<void*, kN> arr(kN + 32);  /* 48 > 16 → heap */

  ASSERT_TRUE(arr.alloc_ok());
  EXPECT_FALSE(arr.on_stack());
  EXPECT_EQ(arr.size(), kN + 32);

  /* Zero-init on heap path too. */
  for (std::size_t i = 0; i < arr.size(); ++i)
    EXPECT_EQ(arr[i], nullptr);

  /* data() must lie OUTSIDE the SmallArray object for the heap path. */
  auto* self = reinterpret_cast<std::uint8_t*>(&arr);
  auto* d = reinterpret_cast<std::uint8_t*>(arr.data());
  bool outside = (d < self) || (d >= self + sizeof(arr));
  EXPECT_TRUE(outside)
      << "heap data() unexpectedly inside SmallArray object";

  void* marker = static_cast<void*>(self);
  arr[0] = marker;                   /* writeable */
  arr[kN + 31] = marker;             /* last slot writeable */
  EXPECT_EQ(arr[0], marker);
  EXPECT_EQ(arr[kN + 31], marker);
}

TEST_F(SmallArrayTest, ZeroSizeIsSafe) {
  SmallArray<int, 16> arr(0);
  EXPECT_TRUE(arr.alloc_ok());
  EXPECT_EQ(arr.size(), 0u);
  /* data() on zero-size can legitimately be nullptr — we just require
   * no crash and no heap allocation. */
  EXPECT_TRUE(arr.on_stack());
}

}  // namespace
