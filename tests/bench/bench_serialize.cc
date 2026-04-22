/*
 * Copyright (c) 2026 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 */

/*
 * bench_serialize
 * ---------------
 * Measures the bundle <-> GVariant serialization cost described in
 * Section 5.3 C2 of the plan. We stay high-level and exercise the public
 * notification_set_*/notification_get_* flow that drives the internal
 * bundle build. Refinement (e.g. calling notification_ipc_make_gvariant*)
 * will land with Phase 1.
 */

#include <benchmark/benchmark.h>

extern "C" {
#include "notification.h"
#include "notification_internal.h"
}

namespace {

void BM_SetTextMany(benchmark::State& state) {
  notification_h n = notification_create(NOTIFICATION_TYPE_NOTI);
  if (!n) {
    state.SkipWithError("notification_create returned null");
    return;
  }
  for (auto _ : state) {
    notification_set_text(n, NOTIFICATION_TEXT_TYPE_TITLE, "t",
                          nullptr, NOTIFICATION_VARIABLE_TYPE_NONE);
    notification_set_text(n, NOTIFICATION_TEXT_TYPE_CONTENT, "c",
                          nullptr, NOTIFICATION_VARIABLE_TYPE_NONE);
    notification_set_text(n, NOTIFICATION_TEXT_TYPE_EVENT_COUNT, "1",
                          nullptr, NOTIFICATION_VARIABLE_TYPE_NONE);
    benchmark::DoNotOptimize(n);
  }
  state.SetItemsProcessed(state.iterations() * 3);
  notification_free(n);
}
BENCHMARK(BM_SetTextMany)->Unit(benchmark::kMicrosecond);

void BM_CloneNoti(benchmark::State& state) {
  notification_h src = notification_create(NOTIFICATION_TYPE_NOTI);
  if (!src) {
    state.SkipWithError("notification_create returned null");
    return;
  }
  notification_set_text(src, NOTIFICATION_TEXT_TYPE_TITLE, "t",
                        nullptr, NOTIFICATION_VARIABLE_TYPE_NONE);
  for (auto _ : state) {
    notification_h dst = nullptr;
    notification_clone(src, &dst);
    benchmark::DoNotOptimize(dst);
    if (dst) notification_free(dst);
  }
  state.SetItemsProcessed(state.iterations());
  notification_free(src);
}
BENCHMARK(BM_CloneNoti)->Unit(benchmark::kMicrosecond);

}  // namespace

BENCHMARK_MAIN();
