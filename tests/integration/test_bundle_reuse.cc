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
 * I19 Bundle-reuse allocation test (skeleton).
 *
 * Goal: posting the same notification template twice must perform fewer
 * allocations on the second call than on the first. This becomes a real
 * gate once Phase 1 ships the bundle-encode cache (D3).
 *
 * The skeleton records allocation counter diffs around two posts. The
 * comparison is wrapped in a fprintf only; once Phase 1 lands we flip the
 * EXPECT_LT on.
 */

#include <gtest/gtest.h>

#include <cstdio>

extern "C" {
#include "notification.h"
}

#include "alloc_counter.hh"

TEST(BundleReuse, SecondPostFewerAllocs) {
  notification_h n = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(n, nullptr);
  notification_set_text(n, NOTIFICATION_TEXT_TYPE_TITLE, "reuse",
                        nullptr, NOTIFICATION_VARIABLE_TYPE_NONE);

  noti_test::SetCountingEnabled(true);
  auto before1 = noti_test::SnapshotAllocs();
  notification_post(n);
  auto diff1 = noti_test::DiffAllocs(before1);

  auto before2 = noti_test::SnapshotAllocs();
  notification_post(n);
  auto diff2 = noti_test::DiffAllocs(before2);
  noti_test::SetCountingEnabled(false);

  std::fprintf(stderr, "[bundle_reuse] first=%zu second=%zu allocs\n",
               diff1.total_allocs, diff2.total_allocs);

  /* TODO(phase1): once the bundle-encode cache is in place, enable:
   *   EXPECT_LT(diff2.total_allocs, diff1.total_allocs);
   */

  notification_free(n);
}
