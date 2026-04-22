/*
 * Copyright (c) 2026 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef UNIT_TESTS_MOCK_ALLOC_COUNTER_H_
#define UNIT_TESTS_MOCK_ALLOC_COUNTER_H_

#include <atomic>
#include <cstddef>

/*
 * Lightweight allocation counter used by PerfAwareFixture /
 * LeakCheckFixture. Counters are incremented from --wrap=malloc/free/realloc
 * symbols (see alloc_counter.cc).
 *
 * This is test-only; production libs must not link against this.
 */
namespace noti_test {

struct AllocStats {
  std::atomic<size_t> total_allocs{0};
  std::atomic<size_t> total_frees{0};
  std::atomic<size_t> live_allocs{0};
  std::atomic<size_t> live_bytes{0};
};

/* Global singleton - thread-safe through atomics. */
AllocStats& GetAllocStats();

/* Snapshot helpers. */
struct AllocSnapshot {
  size_t total_allocs;
  size_t total_frees;
  size_t live_allocs;
  size_t live_bytes;
};

AllocSnapshot SnapshotAllocs();

/* Compute diff: current - prev. */
AllocSnapshot DiffAllocs(const AllocSnapshot& prev);

/* Thread-local enable flag so tests can scope counting to specific regions. */
bool IsCountingEnabled();
void SetCountingEnabled(bool enabled);

}  // namespace noti_test

#endif  // UNIT_TESTS_MOCK_ALLOC_COUNTER_H_
