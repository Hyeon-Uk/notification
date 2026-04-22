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
 * I3 Cold-Start integration test (skeleton).
 *
 * Goal: measure the wall time from process start to the first successful
 * notification_post() call. Phase 4 target is < 30 ms.
 *
 * This file is a skeleton; the hot assertion is commented out so the
 * binary exits 0 on environments where the daemon is not available. The
 * measurement scaffolding (clock + scratch notification) is wired in so
 * Phase 4 can flip the assertion on.
 */

#include <gtest/gtest.h>

#include <chrono>
#include <cstdio>

extern "C" {
#include "notification.h"
}

namespace {

using std::chrono::duration_cast;
using std::chrono::microseconds;
using std::chrono::steady_clock;

constexpr double kColdStartTargetMs = 30.0;  // Phase 4 goal.

}  // namespace

TEST(ColdStart, FirstPostMeasured) {
  auto t0 = steady_clock::now();

  notification_h n = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(n, nullptr);
  notification_set_text(n, NOTIFICATION_TEXT_TYPE_TITLE, "cold-start",
                        nullptr, NOTIFICATION_VARIABLE_TYPE_NONE);
  int rc = notification_post(n);

  auto t1 = steady_clock::now();
  double us = duration_cast<microseconds>(t1 - t0).count();
  double ms = us / 1000.0;

  std::fprintf(stderr, "[cold_start] first notification_post rc=%d in %.3f ms\n",
               rc, ms);

  /*
   * TODO(phase4): Flip the following guard once Phase 4 lands.
   *
   *   EXPECT_LT(ms, kColdStartTargetMs);
   *
   * For Phase 0 we only verify that the call path does not crash.
   */
  (void)kColdStartTargetMs;

  notification_free(n);
}
