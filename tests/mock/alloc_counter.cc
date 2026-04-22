/*
 * Copyright (c) 2026 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#include "alloc_counter.hh"

#include <cstdlib>
#include <cstring>
#include <dlfcn.h>

/*
 * Implementation strategy:
 * - Provide --wrap=malloc/free/realloc/calloc symbols (__wrap_malloc ...) that
 *   call through to __real_malloc (linker stub) and update counters.
 * - If the linker wrap flags are not used (e.g. when NOTI_TEST_WRAP_ALLOC is
 *   undefined), counters stay at zero but the API is still safe to call.
 *
 * To avoid perturbing gtest / libstdc++ internals too much we only count when
 * SetCountingEnabled(true) has been invoked on the current thread. Fixtures
 * toggle this around the measured region.
 */

namespace noti_test {

namespace {
thread_local bool t_counting_enabled = false;
}  // namespace

AllocStats& GetAllocStats() {
  static AllocStats stats;
  return stats;
}

bool IsCountingEnabled() { return t_counting_enabled; }

void SetCountingEnabled(bool enabled) { t_counting_enabled = enabled; }

AllocSnapshot SnapshotAllocs() {
  auto& s = GetAllocStats();
  return AllocSnapshot{
      s.total_allocs.load(std::memory_order_relaxed),
      s.total_frees.load(std::memory_order_relaxed),
      s.live_allocs.load(std::memory_order_relaxed),
      s.live_bytes.load(std::memory_order_relaxed),
  };
}

AllocSnapshot DiffAllocs(const AllocSnapshot& prev) {
  auto now = SnapshotAllocs();
  AllocSnapshot diff;
  diff.total_allocs = now.total_allocs - prev.total_allocs;
  diff.total_frees = now.total_frees - prev.total_frees;
  diff.live_allocs = now.live_allocs - prev.live_allocs;
  diff.live_bytes = now.live_bytes - prev.live_bytes;
  return diff;
}

}  // namespace noti_test

#if defined(NOTI_TEST_WRAP_ALLOC)
/*
 * Linker-wrap hooks (--wrap=malloc etc).
 * Counters are updated only when the calling thread has enabled counting via
 * SetCountingEnabled(true). This prevents gtest / libstdc++ allocations made
 * outside the measured region from polluting the stats.
 */

extern "C" {

void* __real_malloc(size_t);
void  __real_free(void*);
void* __real_realloc(void*, size_t);
void* __real_calloc(size_t, size_t);

void* __wrap_malloc(size_t size) {
  void* p = __real_malloc(size);
  if (p && noti_test::IsCountingEnabled()) {
    auto& s = noti_test::GetAllocStats();
    s.total_allocs.fetch_add(1, std::memory_order_relaxed);
    s.live_allocs.fetch_add(1, std::memory_order_relaxed);
    s.live_bytes.fetch_add(size, std::memory_order_relaxed);
  }
  return p;
}

void __wrap_free(void* p) {
  if (p && noti_test::IsCountingEnabled()) {
    auto& s = noti_test::GetAllocStats();
    s.total_frees.fetch_add(1, std::memory_order_relaxed);
    if (s.live_allocs.load(std::memory_order_relaxed) > 0)
      s.live_allocs.fetch_sub(1, std::memory_order_relaxed);
  }
  __real_free(p);
}

void* __wrap_realloc(void* p, size_t size) {
  void* r = __real_realloc(p, size);
  if (noti_test::IsCountingEnabled()) {
    auto& s = noti_test::GetAllocStats();
    s.total_allocs.fetch_add(1, std::memory_order_relaxed);
  }
  return r;
}

void* __wrap_calloc(size_t nmemb, size_t size) {
  void* p = __real_calloc(nmemb, size);
  if (p && noti_test::IsCountingEnabled()) {
    auto& s = noti_test::GetAllocStats();
    s.total_allocs.fetch_add(1, std::memory_order_relaxed);
    s.live_allocs.fetch_add(1, std::memory_order_relaxed);
    s.live_bytes.fetch_add(nmemb * size, std::memory_order_relaxed);
  }
  return p;
}

}  // extern "C"
#endif  // NOTI_TEST_WRAP_ALLOC
