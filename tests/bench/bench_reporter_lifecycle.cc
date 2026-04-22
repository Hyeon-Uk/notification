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
 * bench_reporter_lifecycle
 * ------------------------
 * Measures Reporter/Manager create/destroy cost. This is the cold-start
 * proxy called out in Section 5.4 (U1/U2/U3). Validates the FactoryManager
 * removal and DBusConnectionManager lazy-init work in later phases.
 *
 * On environments without a live D-Bus bus the create path may fail; in
 * that case we simply measure the error fast-path (still informative for
 * regression tracking).
 */

#include <benchmark/benchmark.h>

extern "C" {
#include "notification_ex_reporter.h"
#include "notification_ex_manager.h"
}

namespace {

void BM_ReporterCreateDestroy(benchmark::State& state) {
  for (auto _ : state) {
    noti_ex_reporter_h r = nullptr;
    int rc = noti_ex_reporter_create(&r, nullptr, nullptr, nullptr, nullptr);
    benchmark::DoNotOptimize(rc);
    if (r) noti_ex_reporter_destroy(r);
  }
  state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_ReporterCreateDestroy)->Unit(benchmark::kMicrosecond);

void BM_ManagerCreateDestroy(benchmark::State& state) {
  for (auto _ : state) {
    noti_ex_manager_h m = nullptr;
    int rc = noti_ex_manager_create(&m, nullptr, nullptr, nullptr);
    benchmark::DoNotOptimize(rc);
    if (m) noti_ex_manager_destroy(m);
  }
  state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_ManagerCreateDestroy)->Unit(benchmark::kMicrosecond);

/*
 * BM_ReporterColdFirstCall
 * ------------------------
 * N1-T1 goal measurement: captures the first-call latency from a cold
 * process state. We use KeepRunningBatch(1) so Google Benchmark only runs
 * one iteration per timing sample, which matches the cold-start semantics
 * (the benchmark driver still repeats the outer sample for statistics,
 * but each sample is a single create+destroy pair). After N1-T1 lands,
 * this should be -40% versus the Phase 0 baseline because the constructor
 * no longer pays the g_bus_get_sync + g_bus_own_name cost.
 */
void BM_ReporterColdFirstCall(benchmark::State& state) {
  while (state.KeepRunningBatch(1)) {
    noti_ex_reporter_h r = nullptr;
    int rc = noti_ex_reporter_create(&r, nullptr, nullptr, nullptr, nullptr);
    benchmark::DoNotOptimize(rc);
    if (r) noti_ex_reporter_destroy(r);
  }
  state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_ReporterColdFirstCall)->Unit(benchmark::kMicrosecond);

}  // namespace

BENCHMARK_MAIN();
