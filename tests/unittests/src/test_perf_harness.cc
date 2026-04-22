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
 * Sanity test: ensure the LeakCheckFixture and PerfAwareFixture harness
 * compile and run. These tests do not exercise the notification public API
 * to stay robust against the wider mock setup - their sole purpose is to
 * verify the test-harness plumbing introduced in N0-T1.
 */

#include <gtest/gtest.h>
#include <cstdlib>
#include <cstdio>

#include "mock/test_fixture.hh"

using PerfHarnessTest = PerfAwareFixture;
using LeakHarnessTest = LeakCheckFixture;

TEST_F(PerfHarnessTest, SnapshotAllocCount_CanBeRead) {
  /* Baseline taken in SetUp; a fresh snapshot must return a non-negative
   * counter diff. */
  SnapshotAllocCount();
  size_t before = AllocCountSinceSnapshot();
  (void)before;

  /* Trigger a tiny allocation to exercise the wrap path (if enabled). */
  void* p = std::malloc(64);
  ASSERT_NE(p, nullptr);
  std::free(p);

  /* Regardless of whether --wrap hooks are active, this call must not crash. */
  ExpectAllocCountUnder(10000, "perf harness sanity");
  std::fprintf(stderr, "[PerfHarness] alloc delta=%zu\n",
               AllocCountSinceSnapshot());
}

TEST_F(LeakHarnessTest, PairedMallocFree_DoesNotLeak) {
  void* p = std::malloc(32);
  ASSERT_NE(p, nullptr);
  std::free(p);
  /* TearDown will assert no net leak - paired alloc/free must be clean. */
}
