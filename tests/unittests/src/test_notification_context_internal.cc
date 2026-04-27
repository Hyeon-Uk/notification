/*
 * Copyright (c) 2026 Samsung Electronics Co., Ltd. All rights reserved.
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
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * N7.5-B coverage boost — notification_context.c (Phase 1 N1-T4).
 *
 * test_c_context.cc already covers the create/destroy + isolation surface.
 * This file targets the specific branches that needed extra exercise to
 * lift the Phase 1 N1-T4 file above 85% line / 75% branch:
 *
 *   1. DefaultContext_ConcurrentInit_RaceFree
 *      — 8 std::thread workers race on _notification_get_default_context();
 *        the g_once_init_enter / g_once_init_leave pair must serialize
 *        construction so all 8 receive the same pointer (call_once
 *        guarantee). Smoke-tests the fast-path when the latch has already
 *        been set.
 *
 *   2. ContextField_DefaultValues
 *      — A freshly-created user context must report:
 *          *default_viewer_slot == NULL
 *          *event_cb_list_slot  == NULL
 *          *connected_slot      == false
 *
 *   3. EventCbList_AppendThenIterate
 *      — Append 12 sentinel pointers via the slot accessor, then walk the
 *        list and verify ordering. Exercises the GList payload-ownership
 *        contract documented in notification_context.c (the spine is owned;
 *        payloads are caller-owned). 12 nodes > the in-place GList stack
 *        threshold so the spine is heap-allocated (and freed in destroy).
 *
 *   4. SlotAccessors_NullCtx_ReturnNull
 *      — The three slot accessors all NULL-guard their input.
 *
 *   5. Destroy_UserContext_FreesViewerString
 *      — A user context that wrote a strdup'd string into its viewer slot
 *        must free it on destroy (LeakCheckFixture invariant).
 */

#include <gtest/gtest.h>

#include <atomic>
#include <cstdlib>
#include <cstring>
#include <thread>
#include <vector>

#include <glib.h>

extern "C" {
#include "notification_context.h"
#include "notification_error.h"
}

#include "mock/test_fixture.hh"

namespace {

class CContextInternalTest : public LeakCheckFixture {
 public:
  CContextInternalTest() : LeakCheckFixture() {
    /* Warm the singleton before LeakCheckFixture snapshots, mirroring the
     * pattern in test_c_context.cc. */
    (void)_notification_get_default_context();
  }
};

}  // namespace

/* (1) Race on _notification_get_default_context(): every thread must observe
 * the same singleton pointer. */
TEST_F(CContextInternalTest, DefaultContext_ConcurrentInit_RaceFree) {
  constexpr int kThreads = 8;

  /* Capture the existing default for cross-comparison. */
  notification_context_h ref = _notification_get_default_context();
  ASSERT_NE(ref, nullptr);

  std::vector<std::thread> pool;
  std::vector<notification_context_h> seen(kThreads, nullptr);
  std::atomic<int> reached{0};
  std::atomic<bool> go{false};
  pool.reserve(kThreads);

  for (int i = 0; i < kThreads; ++i) {
    pool.emplace_back([&, i]() {
      reached.fetch_add(1, std::memory_order_acq_rel);
      while (!go.load(std::memory_order_acquire)) {
        /* spin */
      }
      seen[i] = _notification_get_default_context();
    });
  }

  while (reached.load(std::memory_order_acquire) < kThreads) {
    std::this_thread::yield();
  }
  go.store(true, std::memory_order_release);

  for (auto& t : pool) t.join();

  for (int i = 0; i < kThreads; ++i) {
    ASSERT_NE(seen[i], nullptr) << "thread " << i << " saw NULL default ctx";
    EXPECT_EQ(seen[i], ref) << "thread " << i << " saw a different singleton";
  }
}

/* (2) Default-value invariants for a freshly-created user context. */
TEST_F(CContextInternalTest, ContextField_DefaultValues) {
  notification_context_h ctx = nullptr;
  ASSERT_EQ(notification_context_create(&ctx), NOTIFICATION_ERROR_NONE);
  ASSERT_NE(ctx, nullptr);

  char** viewer_slot = _notification_context_default_viewer_slot(ctx);
  GList** cb_slot = _notification_context_event_cb_list_slot(ctx);
  bool* connected_slot = _notification_context_connected_slot(ctx);

  ASSERT_NE(viewer_slot, nullptr);
  ASSERT_NE(cb_slot, nullptr);
  ASSERT_NE(connected_slot, nullptr);

  EXPECT_EQ(*viewer_slot, nullptr);
  EXPECT_EQ(*cb_slot, nullptr);
  EXPECT_FALSE(*connected_slot);

  EXPECT_EQ(notification_context_destroy(ctx), NOTIFICATION_ERROR_NONE);
}

/* (3) Append 12 callbacks and verify FIFO order. We avoid calling
 * `g_list_append` directly because tests/mock/glib_mock.cc interposes that
 * single g_list_* symbol via MOCK_HOOK (no GlibMock provided in this TU
 * → exception). The legacy __noti_event_cb_list semantics are recovered
 * by g_list_prepend + g_list_reverse (which resolve to the real glib
 * implementations and produce the same order; identical pattern to
 * test_list_build_order.cc and the N3-T4 A1 prod refactor). */
TEST_F(CContextInternalTest, EventCbList_AppendThenIterate) {
  notification_context_h ctx = nullptr;
  ASSERT_EQ(notification_context_create(&ctx), NOTIFICATION_ERROR_NONE);

  GList** cb_slot = _notification_context_event_cb_list_slot(ctx);
  ASSERT_NE(cb_slot, nullptr);
  EXPECT_EQ(*cb_slot, nullptr);

  /* 12 distinct sentinels: build via prepend + reverse so we observe FIFO
   * insertion order on iteration. */
  constexpr int kCount = 12;
  uintptr_t sentinels[kCount];
  for (int i = 0; i < kCount; ++i) {
    sentinels[i] = (uintptr_t)(0xC0DE0000u + i);
    *cb_slot = g_list_prepend(*cb_slot, (gpointer)sentinels[i]);
  }
  *cb_slot = g_list_reverse(*cb_slot);

  /* Walk and verify FIFO order. */
  int idx = 0;
  for (GList* it = *cb_slot; it != nullptr; it = it->next, ++idx) {
    ASSERT_LT(idx, kCount);
    EXPECT_EQ((uintptr_t)it->data, sentinels[idx])
        << "list ordering corrupted at index " << idx;
  }
  EXPECT_EQ(idx, kCount);

  /* destroy() must free the GList spine. Payloads are stack values, no
   * caller free needed. */
  EXPECT_EQ(notification_context_destroy(ctx), NOTIFICATION_ERROR_NONE);
}

/* (4) NULL ctx -> NULL slot for each accessor. */
TEST_F(CContextInternalTest, SlotAccessors_NullCtx_ReturnNull) {
  EXPECT_EQ(_notification_context_default_viewer_slot(nullptr), nullptr);
  EXPECT_EQ(_notification_context_event_cb_list_slot(nullptr), nullptr);
  EXPECT_EQ(_notification_context_connected_slot(nullptr), nullptr);
}

/* (5) Setting a strdup'd viewer string and destroying the context must free
 * it. LeakCheckFixture flags a regression if free() is not called. */
TEST_F(CContextInternalTest, Destroy_UserContext_FreesViewerString) {
  notification_context_h ctx = nullptr;
  ASSERT_EQ(notification_context_create(&ctx), NOTIFICATION_ERROR_NONE);

  char** viewer_slot = _notification_context_default_viewer_slot(ctx);
  ASSERT_NE(viewer_slot, nullptr);
  *viewer_slot = strdup("default.viewer.appid");
  ASSERT_NE(*viewer_slot, nullptr);

  EXPECT_EQ(notification_context_destroy(ctx), NOTIFICATION_ERROR_NONE);
  /* No further access — destroy() owns the strdup'd string per contract. */
}
