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
 * N1-T4 unit tests for the C API context seam.
 *
 * Matrix:
 *   1. ContextCreateDestroy_NoLeak
 *      — 100 successive create()/destroy() pairs under LeakCheckFixture must
 *        not leak net live allocations. Guards against a regression where
 *        the destroy path forgets one of the owned sub-objects (string
 *        slot, list spine, recursive mutex).
 *
 *   2. ContextCreate_InvalidParam
 *      — create(NULL) must return NOTIFICATION_ERROR_INVALID_PARAMETER and
 *        destroy(NULL) must return the same code. The default context
 *        itself must refuse destruction (returns NOTIFICATION_ERROR_NONE
 *        without freeing) because tearing it down would race with other
 *        threads still using the process-wide singleton.
 *
 *   3. ContextIsolation_TwoContextsDoNotShare
 *      — Two independently-created contexts must own disjoint storage.
 *        Writing to the default-viewer slot of ctx_a must not be visible
 *        via the slot of ctx_b.
 */

#include <gtest/gtest.h>

#include <glib.h>
#include <cstring>
#include <string>

#include "mock/test_fixture.hh"
#include "mock/alloc_counter.hh"

extern "C" {
#include "notification_context.h"
#include "notification_error.h"
}

namespace {

class CContextTest : public LeakCheckFixture {
 public:
  CContextTest() : LeakCheckFixture() {
    /* Warm up the singleton default context *before* the LeakCheckFixture
     * snapshots its allocation baseline in SetUp(). The default context is
     * a process-wide lazily-constructed singleton; its allocation must not
     * be attributed to a per-test leak or the ContextCreateDestroy_NoLeak
     * iteration TEST would fail the first time the class is instantiated. */
    (void)_notification_get_default_context();
  }
};

/*
 * 100 create/destroy iterations stress the allocator just enough to surface
 * a per-iteration leak via --wrap=malloc. The LeakCheckFixture compares
 * allocations vs frees across the full TEST body, so a single missed free
 * per iteration would net +100 and trip ADD_FAILURE.
 */
TEST_F(CContextTest, ContextCreateDestroy_NoLeak) {
  for (int i = 0; i < 100; ++i) {
    notification_context_h ctx = nullptr;
    int r = notification_context_create(&ctx);
    ASSERT_EQ(r, NOTIFICATION_ERROR_NONE) << "iteration=" << i;
    ASSERT_NE(ctx, nullptr) << "iteration=" << i;

    /* Exercise the slot accessors so any slot-lazy allocation would also
     * need freeing at destroy time. */
    char** viewer_slot = _notification_context_default_viewer_slot(ctx);
    ASSERT_NE(viewer_slot, nullptr);
    EXPECT_EQ(*viewer_slot, nullptr);

    GList** cb_slot = _notification_context_event_cb_list_slot(ctx);
    ASSERT_NE(cb_slot, nullptr);
    EXPECT_EQ(*cb_slot, nullptr);

    bool* connected_slot = _notification_context_connected_slot(ctx);
    ASSERT_NE(connected_slot, nullptr);
    EXPECT_FALSE(*connected_slot);

    r = notification_context_destroy(ctx);
    EXPECT_EQ(r, NOTIFICATION_ERROR_NONE) << "iteration=" << i;
  }
}

TEST_F(CContextTest, ContextCreate_InvalidParam) {
  /* NULL out-ptr on create. */
  int r = notification_context_create(nullptr);
  EXPECT_EQ(r, NOTIFICATION_ERROR_INVALID_PARAMETER);

  /* NULL handle on destroy. */
  r = notification_context_destroy(nullptr);
  EXPECT_EQ(r, NOTIFICATION_ERROR_INVALID_PARAMETER);

  /* Default-context destruction is a no-op: it must return SUCCESS without
   * actually freeing the singleton. Verify by re-acquiring afterwards —
   * the returned handle must still be valid and slot accessors work. */
  notification_context_h def = _notification_get_default_context();
  ASSERT_NE(def, nullptr);
  r = notification_context_destroy(def);
  EXPECT_EQ(r, NOTIFICATION_ERROR_NONE);

  notification_context_h def2 = _notification_get_default_context();
  ASSERT_EQ(def, def2);
  bool* slot = _notification_context_connected_slot(def2);
  ASSERT_NE(slot, nullptr);
}

/*
 * The default viewer slot of one context must not be aliased to another
 * context's slot. We write to ctx_a and read back from ctx_b; if the
 * contexts shared storage the second read would observe the write.
 */
TEST_F(CContextTest, ContextIsolation_TwoContextsDoNotShare) {
  notification_context_h ctx_a = nullptr;
  notification_context_h ctx_b = nullptr;

  ASSERT_EQ(notification_context_create(&ctx_a), NOTIFICATION_ERROR_NONE);
  ASSERT_EQ(notification_context_create(&ctx_b), NOTIFICATION_ERROR_NONE);
  ASSERT_NE(ctx_a, ctx_b);

  char** slot_a = _notification_context_default_viewer_slot(ctx_a);
  char** slot_b = _notification_context_default_viewer_slot(ctx_b);
  ASSERT_NE(slot_a, nullptr);
  ASSERT_NE(slot_b, nullptr);
  EXPECT_NE(slot_a, slot_b);

  /* Write into ctx_a's slot only. */
  *slot_a = strdup("viewer.a");
  ASSERT_NE(*slot_a, nullptr);
  EXPECT_STREQ(*slot_a, "viewer.a");

  /* ctx_b must still be untouched. */
  EXPECT_EQ(*slot_b, nullptr);

  /* Cross-check the GList slot too. */
  GList** list_a = _notification_context_event_cb_list_slot(ctx_a);
  GList** list_b = _notification_context_event_cb_list_slot(ctx_b);
  ASSERT_NE(list_a, nullptr);
  ASSERT_NE(list_b, nullptr);
  EXPECT_NE(list_a, list_b);

  /* Also assert they are independent from the process default. */
  notification_context_h def = _notification_get_default_context();
  ASSERT_NE(def, nullptr);
  char** slot_def = _notification_context_default_viewer_slot(def);
  ASSERT_NE(slot_def, nullptr);
  EXPECT_NE(slot_a, slot_def);
  EXPECT_NE(slot_b, slot_def);

  /* Destroy in reverse order to catch any accidental dependency between
   * contexts. LeakCheckFixture will also verify we freed the strdup'd
   * "viewer.a" via the destroy path. */
  EXPECT_EQ(notification_context_destroy(ctx_b), NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(notification_context_destroy(ctx_a), NOTIFICATION_ERROR_NONE);
}

}  // namespace
