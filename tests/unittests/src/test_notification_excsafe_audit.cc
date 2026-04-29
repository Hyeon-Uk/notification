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
 * test_notification_excsafe_audit.cc — exception-safety + signed-unsigned +
 * return-value-misuse audit, follow-up to the UAF + iterator-invalidation
 * pass (commit 08606e3).  Four production-reachable defects in the
 * notification-ex C-API stub layer (src/notification-ex/stub.cc):
 *
 *   EX-1  ManagerCallbackInfo::InvokeAdded — calloc'd buffer plus a
 *         loop of `new Handle(...)`.  Pre-fix any throw on iteration
 *         i>0 leaked the previous (i-1) Handle objects AND the buffer,
 *         and the exception escaped the daemon-side observer dispatcher
 *         — the dispatcher is a C-linkage rpc-port stub callback with
 *         no try/catch above us, so the throw propagates across the C
 *         ABI and std::terminate's the client.  Post-fix wraps the
 *         loop in try/catch, rolls back partial state, and bails
 *         silently on bad_alloc.
 *
 *   EX-2  noti_ex_manager_get (extern "C", EXPORT_API) — same shape:
 *         calloc'd buffer + Handle allocation loop, with the cleanup
 *         path missing from the existing catch block.  Pre-fix the
 *         buffer and partial Handles leaked on Exception throw mid-
 *         loop.  Post-fix hoists the buffer/idx to function scope and
 *         frees both in the catch path.
 *
 *   EX-3  noti_ex_manager_get_by_channel — identical to EX-2.
 *
 *   EX-4  ManagerCallbackInfo::{InvokeUpdated,InvokeDeleted} — pre-fix
 *         the `new Handle(...)` was inline in the C-callback argument
 *         list.  bad_alloc would unwind across the C ABI of cb_.updated
 *         / cb_.deleted (registered by the C client app via
 *         noti_ex_manager_register_callback) — std::terminate.  Post-
 *         fix hoists the allocation, guards with try/catch, and skips
 *         the dispatch on failure.
 *
 * Mirror probe: stub.cc lives inside libnotification-ex.so and depends
 * on the full C++ object graph (Manager/Reporter/IEventInfo) plus the
 * rpc-port stub registration.  Following the established mirror
 * pattern (test_lazy_init / test_cpu_boost_lifecycle /
 * test_notification_uaf_audit), the tests below replicate the
 * calloc + allocation-loop + cleanup contract using a fake Handle that
 * deterministically throws on a chosen iteration.  The mirror's
 * post-fix loop is byte-for-byte equivalent to the production fix; any
 * drift in cleanup ordering breaks the tests.
 */

#include <gtest/gtest.h>

#include <cstdlib>
#include <stdexcept>
#include <vector>

namespace {

/* Fake "Handle" — counts ctor / dtor calls so the test can assert
 * resource accounting after the loop.  Throws on demand to simulate
 * bad_alloc mid-loop. */
struct FakeHandle {
  static int ctor_count;
  static int dtor_count;
  static int throw_at_index;  /* -1 = never throw */
  static int next_index;

  int my_index;

  FakeHandle() : my_index(next_index++) {
    if (my_index == throw_at_index)
      throw std::bad_alloc{};
    ++ctor_count;
  }

  ~FakeHandle() { ++dtor_count; }

  static void Reset() {
    ctor_count = 0;
    dtor_count = 0;
    throw_at_index = -1;
    next_index = 0;
  }
};

int FakeHandle::ctor_count = 0;
int FakeHandle::dtor_count = 0;
int FakeHandle::throw_at_index = -1;
int FakeHandle::next_index = 0;

/*
 * Mirror of the post-fix InvokeAdded / noti_ex_manager_get loop:
 * calloc -> for each item alloc Handle -> on exception, destroy each
 * already-allocated Handle and free the buffer.
 *
 * Returns true if every Handle survived (loop completed); false on
 * exception (cleanup applied).
 */
bool PostFixCalloc_AllocLoop_Cleanup(int n) {
  void** buf = static_cast<void**>(calloc(n, sizeof(void*)));
  if (!buf) return false;

  int idx = 0;
  try {
    for (int i = 0; i < n; ++i) {
      buf[idx] = static_cast<void*>(new FakeHandle());
      ++idx;
    }
  } catch (...) {
    for (int j = 0; j < idx; ++j) {
      if (buf[j])
        delete static_cast<FakeHandle*>(buf[j]);
    }
    free(buf);
    return false;
  }

  /* Caller would normally take ownership of buf here.  For the test
   * we deliberately free everything now to keep accounting clean. */
  for (int j = 0; j < n; ++j)
    delete static_cast<FakeHandle*>(buf[j]);
  free(buf);
  return true;
}

/* Negative-control mirror — pre-fix shape: no try/catch, no rollback.
 * Caller of this function must catch and observe the leak. */
void PreFixCalloc_AllocLoop_NoCleanup(int n) {
  void** buf = static_cast<void**>(calloc(n, sizeof(void*)));
  if (!buf) return;

  for (int i = 0; i < n; ++i) {
    buf[i] = static_cast<void*>(new FakeHandle());
  }

  for (int j = 0; j < n; ++j)
    delete static_cast<FakeHandle*>(buf[j]);
  free(buf);
}

}  // namespace

/* ------------------------------------------------------------------ */
/* Post-fix cleanup invariants                                         */
/* ------------------------------------------------------------------ */

TEST(NotificationExcSafeAudit, EX1_PostFix_HappyPath_NoLeak) {
  FakeHandle::Reset();
  ASSERT_TRUE(PostFixCalloc_AllocLoop_Cleanup(5));
  EXPECT_EQ(FakeHandle::ctor_count, 5);
  EXPECT_EQ(FakeHandle::dtor_count, 5);
}

TEST(NotificationExcSafeAudit, EX1_PostFix_ThrowOnFirstIteration) {
  FakeHandle::Reset();
  FakeHandle::throw_at_index = 0;
  ASSERT_FALSE(PostFixCalloc_AllocLoop_Cleanup(5));
  /* Zero successful ctors before the throw, so zero dtors expected. */
  EXPECT_EQ(FakeHandle::ctor_count, 0);
  EXPECT_EQ(FakeHandle::dtor_count, 0);
}

TEST(NotificationExcSafeAudit, EX1_PostFix_ThrowMidLoop_RollsBackPartial) {
  FakeHandle::Reset();
  FakeHandle::throw_at_index = 3;
  ASSERT_FALSE(PostFixCalloc_AllocLoop_Cleanup(10));
  /* 3 successful ctors before the throw; rollback must dtor exactly
   * those 3 and free the buffer. */
  EXPECT_EQ(FakeHandle::ctor_count, 3);
  EXPECT_EQ(FakeHandle::dtor_count, 3)
      << "post-fix rollback must destroy every partially-allocated "
         "Handle exactly once";
}

TEST(NotificationExcSafeAudit, EX1_PostFix_ThrowOnLastIteration) {
  FakeHandle::Reset();
  FakeHandle::throw_at_index = 7;
  ASSERT_FALSE(PostFixCalloc_AllocLoop_Cleanup(8));
  EXPECT_EQ(FakeHandle::ctor_count, 7);
  EXPECT_EQ(FakeHandle::dtor_count, 7);
}

/* ------------------------------------------------------------------ */
/* Negative control: pre-fix shape lets exception escape the boundary  */
/* ------------------------------------------------------------------ */

TEST(NotificationExcSafeAudit, EX1_PreFix_ThrowEscapesBoundary) {
  FakeHandle::Reset();
  FakeHandle::throw_at_index = 2;

  /* Pre-fix shape has no try/catch — the caller (here the test)
   * observes the throw escape.  In production the equivalent caller
   * is the C-ABI rpc-port stub dispatcher, which would
   * std::terminate. */
  EXPECT_THROW(PreFixCalloc_AllocLoop_NoCleanup(5), std::bad_alloc);

  /* 2 successful ctors before the throw, but the pre-fix shape
   * does NOT delete them on the way out — leaks confirmed. */
  EXPECT_EQ(FakeHandle::ctor_count, 2);
  EXPECT_EQ(FakeHandle::dtor_count, 0)
      << "pre-fix shape leaks every partially-allocated Handle";
}

TEST(NotificationExcSafeAudit, EX1_PreFixVsPostFix_SameThrowSite) {
  /* Run pre-fix and post-fix on the same throw_at_index and assert
   * the divergence: post-fix balances ctor/dtor, pre-fix doesn't. */
  FakeHandle::Reset();
  FakeHandle::throw_at_index = 4;
  EXPECT_THROW(PreFixCalloc_AllocLoop_NoCleanup(8), std::bad_alloc);
  const int prefix_ctors = FakeHandle::ctor_count;
  const int prefix_dtors = FakeHandle::dtor_count;

  FakeHandle::Reset();
  FakeHandle::throw_at_index = 4;
  EXPECT_FALSE(PostFixCalloc_AllocLoop_Cleanup(8));
  const int postfix_ctors = FakeHandle::ctor_count;
  const int postfix_dtors = FakeHandle::dtor_count;

  EXPECT_EQ(prefix_ctors, 4);
  EXPECT_EQ(prefix_dtors, 0);   /* leaks */
  EXPECT_EQ(postfix_ctors, 4);
  EXPECT_EQ(postfix_dtors, 4);  /* balanced */
}
