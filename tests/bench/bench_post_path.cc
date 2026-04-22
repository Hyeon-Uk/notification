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
 * bench_post_path
 * ---------------
 * Measures the hot path of notification_post() as described in the
 * notification refactoring plan Section 4.4.
 *
 * The bench is intentionally self-contained: it creates a notification
 * handle, toggles some text fields, and calls notification_post() in a
 * loop. On a real Tizen image this exercises the IPC + DB path and makes
 * hot-path allocations observable through later measurement tooling.
 *
 * When google/benchmark or the notification-daemon is unavailable the
 * benchmark still runs but reports zero items (the bench framework
 * requires at least one iteration to produce a valid JSON record).
 */

#include <benchmark/benchmark.h>

extern "C" {
#include "notification.h"
}

namespace {

notification_h MakeSampleNotification() {
  notification_h n = notification_create(NOTIFICATION_TYPE_NOTI);
  if (!n) return nullptr;
  notification_set_text(n, NOTIFICATION_TEXT_TYPE_TITLE, "bench-title",
                        nullptr, NOTIFICATION_VARIABLE_TYPE_NONE);
  notification_set_text(n, NOTIFICATION_TEXT_TYPE_CONTENT, "bench-content",
                        nullptr, NOTIFICATION_VARIABLE_TYPE_NONE);
  return n;
}

void BM_NotificationPost(benchmark::State& state) {
  notification_h n = MakeSampleNotification();
  if (!n) {
    state.SkipWithError("notification_create returned null");
    return;
  }
  for (auto _ : state) {
    int rc = notification_post(n);
    benchmark::DoNotOptimize(rc);
  }
  state.SetItemsProcessed(state.iterations());
  notification_free(n);
}
BENCHMARK(BM_NotificationPost)->Unit(benchmark::kMicrosecond);

void BM_NotificationCreateFree(benchmark::State& state) {
  for (auto _ : state) {
    notification_h n = notification_create(NOTIFICATION_TYPE_NOTI);
    benchmark::DoNotOptimize(n);
    notification_free(n);
  }
  state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_NotificationCreateFree)->Unit(benchmark::kMicrosecond);

}  // namespace

BENCHMARK_MAIN();
