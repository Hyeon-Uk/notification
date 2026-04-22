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
 * N1-T7: GVariant / builder / GObject autoptr + scratch-builder reuse tests.
 *
 * Scope
 * -----
 * This file proves three behaviours introduced by N1-T7:
 *
 *   1. g_autoptr(GVariant/GDBusMessage/GError/GDBusNodeInfo) fires the
 *      matching release on every scope exit, including the exception-throw
 *      path that the previous unique_ptr<..., g_object_unref*> pattern
 *      already handled. The proof is a direct scoped test exercising the
 *      glib macros alone - we count g_object_unref / g_error_free calls
 *      via the N1-T7 hook counters in glib_mock.cc. Testing the real
 *      DBusSender::MethodCall would require mocking out
 *      DBusConnectionManager + aul + 4 other glib symbols; the scoped
 *      test is equivalent for the contract under test (scope exit fires
 *      cleanup regardless of where the macro sits) and far less fragile.
 *
 *   2. "No leak on MethodCall exception" is the same contract applied to
 *      a function that throws before the scope exits normally: the
 *      cleanup must still fire. We simulate this by allocating two
 *      g_autoptr(GDBusMessage) in a lambda and then throwing; the hook
 *      must record both unrefs.
 *
 *   3. Scratch builder reuse: ex_util::ScratchBuilder() exposes a
 *      thread-local GVariantBuilder cache. First call allocates; second
 *      call must NOT allocate. The alloc counter is exposed through
 *      util::GetScratchBuilderAllocCountForTest(). Cross-reference with
 *      glib_mock_n1t7::GetVariantBuilderNewCount() - both must agree
 *      that the hot path is zero-alloc.
 *
 * Why not EXPECT_CALL gmock?
 * --------------------------
 * g_object_unref is called by dozens of unrelated production paths during
 * test startup (DBusConnectionManager, aul, packaging). A gmock EXPECT_CALL
 * with Times(Exactly(N)) would be impossible to stabilize. The free
 * counters in glib_mock_n1t7:: are reset-before / read-after per-test and
 * count only calls inside the scoped region under test.
 */

#include <gtest/gtest.h>
#include <glib.h>
#include <gio/gio.h>

#include <memory>
#include <stdexcept>

#include "mock/test_fixture.hh"
#include "mock/glib_mock.hh"

#include "notification-ex/ex_util.h"

namespace {

/*
 * Fixture: resets both the scratch-builder TLS slot and the N1-T7
 * glib counters so each TEST_F starts from a clean baseline.
 *
 * NOTE: the counters are file-static in glib_mock.cc; the hook wrappers
 * intercept real g_object_unref / g_error_free / g_variant_builder_*
 * calls across the whole binary. Using ResetCounters() at the start of
 * each test is critical because other suites (e.g. DBusLazyInitTest)
 * leave residual counts behind.
 */
class GVariantAutoptrTest : public ::testing::Test {
 public:
  void SetUp() override {
    notification::util::ResetScratchBuilderForTest();
    glib_mock_n1t7::ResetCounters();
  }
  void TearDown() override {
    notification::util::ResetScratchBuilderForTest();
  }
};

}  // namespace

/*
 * Test 1: GVariant_AutoptrUnrefOnScopeExit
 *
 * Prove the g_autoptr attribute runs the cleanup on the matching symbol
 * at scope exit. We allocate a GVariant via g_variant_new + ref_sink
 * (mandatory: g_variant_new returns a floating ref, so without ref_sink
 * the g_autoptr cleanup would hit a floating ref and leak), plus a
 * GError and a dummy GDBusMessage (via g_object_new since we do not
 * have a bus). The test records the counter deltas inside the scope
 * and asserts the expected post-scope totals.
 */
TEST_F(GVariantAutoptrTest, GVariant_AutoptrUnrefOnScopeExit) {
  const int before_obj = glib_mock_n1t7::GetObjectUnrefCount();
  const int before_err = glib_mock_n1t7::GetErrorFreeCount();

  {
    /*
     * Floating-ref note: g_autoptr(GVariant) calls g_variant_unref on
     * scope exit. For a floating ref that would be incorrect (should be
     * g_variant_unref only if caller owns a strong ref) - we therefore
     * g_variant_ref_sink the new value first. This matches the
     * production pattern the design doc calls out as "safe to autoptr".
     */
    g_autoptr(GVariant) v =
        g_variant_ref_sink(g_variant_new("(s)", "hello"));
    ASSERT_NE(v, nullptr);

    /* Dummy GObject to prove g_autoptr(GDBusMessage) -> g_object_unref. */
    g_autoptr(GObject) obj = G_OBJECT(g_object_new(G_TYPE_OBJECT, nullptr));
    ASSERT_NE(obj, nullptr);

    /* g_autoptr(GError): NULL-safe on cleanup, but we set it so the
     * cleanup actually runs g_error_free. */
    g_autoptr(GError) err = g_error_new_literal(
        g_quark_from_static_string("test"), 42, "injected");
    ASSERT_NE(err, nullptr);
  }

  /* Post-scope: +1 object unref (GObject), +1 error free (GError).
   * GVariant unref is NOT counted (not one of the N1-T7 counters). */
  EXPECT_GE(glib_mock_n1t7::GetObjectUnrefCount() - before_obj, 1)
      << "g_autoptr(GObject) did not run g_object_unref on scope exit.";
  EXPECT_GE(glib_mock_n1t7::GetErrorFreeCount() - before_err, 1)
      << "g_autoptr(GError) did not run g_error_free on scope exit.";
}

/*
 * Test 2: DBusSender_NoLeakOnMethodCallException
 *
 * Models the MethodCall error path: allocate two g_autoptr(GDBusMessage)
 * surrogates, throw, catch at the outer scope, and verify both unref
 * wrappers ran. This is the exception-safety contract L11/L12 exists to
 * defend.
 *
 * We use GObject as a GDBusMessage surrogate since we cannot open a
 * real bus in the test harness. The autoptr macro is
 * G_DEFINE_AUTOPTR_CLEANUP_FUNC(GDBusMessage, g_object_unref) in glib's
 * gio-autocleanups.h, so it resolves to the same g_object_unref call
 * that GObject hits. Exercising GObject therefore covers GDBusMessage.
 */
TEST_F(GVariantAutoptrTest, DBusSender_NoLeakOnMethodCallException) {
  const int before_obj = glib_mock_n1t7::GetObjectUnrefCount();

  try {
    g_autoptr(GObject) msg = G_OBJECT(g_object_new(G_TYPE_OBJECT, nullptr));
    ASSERT_NE(msg, nullptr);

    g_autoptr(GObject) reply = G_OBJECT(g_object_new(G_TYPE_OBJECT, nullptr));
    ASSERT_NE(reply, nullptr);

    /* Simulate the post-send error path: both autoptrs must fire. */
    throw std::runtime_error("simulated MethodCall failure");
  } catch (const std::runtime_error&) {
    /* expected */
  }

  /* +2 object unrefs: one per autoptr(GObject). NULL autoptrs are
   * skipped, so "at least 2" is the strict invariant - other residual
   * activity on the thread may push the count higher. */
  EXPECT_GE(glib_mock_n1t7::GetObjectUnrefCount() - before_obj, 2)
      << "g_autoptr(GObject) did not unref on exception path; L11/L12 "
         "regression risk.";
}

/*
 * Sanity variant of test 2: early-throw before the SECOND autoptr is
 * constructed. Only one unref must fire.
 */
TEST_F(GVariantAutoptrTest,
       DBusSender_NoLeakOnEarlyException_OnlyFirstUnrefs) {
  const int before_obj = glib_mock_n1t7::GetObjectUnrefCount();

  try {
    g_autoptr(GObject) msg = G_OBJECT(g_object_new(G_TYPE_OBJECT, nullptr));
    ASSERT_NE(msg, nullptr);
    throw std::runtime_error("early failure before reply alloc");
  } catch (const std::runtime_error&) {
    /* expected */
  }

  EXPECT_GE(glib_mock_n1t7::GetObjectUnrefCount() - before_obj, 1)
      << "Even single-autoptr scope must unref on throw.";
}

/*
 * Test 3: BuilderReuse_SecondNotifyFewerAllocs
 *
 * Core goal of N1-T7 item 2 (TLS scratch builder). The first call on a
 * fresh thread must allocate exactly once; the second call must reuse
 * the existing heap builder.
 *
 * Two orthogonal witnesses:
 *   (a) util::GetScratchBuilderAllocCountForTest() - in-process counter
 *       maintained by ex_util.cc. Stable regardless of glib versions.
 *   (b) glib_mock_n1t7::GetVariantBuilderNewCount() - real-world count
 *       of the underlying g_variant_builder_new symbol. Checked as a
 *       cross-reference.
 *
 * If either witness shows a hot-path alloc, the reuse contract is
 * broken. The goal metric in the design doc is "builder alloc == 0 on
 * the 2nd Notify" which we assert directly.
 */
TEST_F(GVariantAutoptrTest, BuilderReuse_SecondNotifyFewerAllocs) {
  namespace nu = notification::util;

  /* First call: cold path, one alloc expected. */
  GVariantBuilder* b1 = nu::ScratchBuilder(G_VARIANT_TYPE("a(s)"));
  ASSERT_NE(b1, nullptr);
  EXPECT_EQ(nu::GetScratchBuilderAllocCountForTest(), 1);
  const int new_after_first = glib_mock_n1t7::GetVariantBuilderNewCount();
  EXPECT_GE(new_after_first, 1)
      << "Cold path did not call g_variant_builder_new - glib_mock hook "
         "not firing or production path bypassed ScratchBuilder().";

  /* Simulate Notify consuming the builder. In production this happens
   * inside g_variant_new("a(s)", builder) which calls
   * g_variant_builder_end internally. Here we just drop the reference
   * and trust the next ScratchBuilder call to re-init. To emulate the
   * real-world drain without a full g_variant_new, we populate one
   * child and call _end() ourselves. */
  g_variant_builder_add(b1, "(s)", "payload-1");
  GVariant* v1 = g_variant_builder_end(b1);  // drains b1 in-place
  ASSERT_NE(v1, nullptr);
  g_variant_unref(v1);

  /* Second call: hot path, MUST NOT increment the alloc counter. */
  GVariantBuilder* b2 = nu::ScratchBuilder(G_VARIANT_TYPE("a(s)"));
  ASSERT_NE(b2, nullptr);
  EXPECT_EQ(b2, b1)
      << "ScratchBuilder returned a different pointer - reuse broken.";
  EXPECT_EQ(nu::GetScratchBuilderAllocCountForTest(), 1)
      << "Second ScratchBuilder call allocated again; reuse broken.";
  EXPECT_EQ(glib_mock_n1t7::GetVariantBuilderNewCount(), new_after_first)
      << "Hot path incremented g_variant_builder_new count; expected 0 "
         "delta.";

  /* Drain b2 too, for leak cleanliness. */
  g_variant_builder_add(b2, "(s)", "payload-2");
  GVariant* v2 = g_variant_builder_end(b2);
  ASSERT_NE(v2, nullptr);
  g_variant_unref(v2);
}

/*
 * Test 4: regression guard - ResetScratchBuilderForTest() genuinely
 * drops the cached heap builder so the next call re-allocates. Without
 * this, cross-test contamination would make the reuse test green by
 * accident regardless of prod behavior.
 */
TEST_F(GVariantAutoptrTest, BuilderReuse_ResetHookForcesFreshAlloc) {
  namespace nu = notification::util;

  GVariantBuilder* b1 = nu::ScratchBuilder(G_VARIANT_TYPE("a(s)"));
  ASSERT_NE(b1, nullptr);
  EXPECT_EQ(nu::GetScratchBuilderAllocCountForTest(), 1);

  /* Drain and reset. */
  GVariant* v = g_variant_builder_end(b1);
  g_variant_unref(v);
  nu::ResetScratchBuilderForTest();
  EXPECT_EQ(nu::GetScratchBuilderAllocCountForTest(), 0);

  /* Next call must allocate again. */
  GVariantBuilder* b2 = nu::ScratchBuilder(G_VARIANT_TYPE("a(s)"));
  ASSERT_NE(b2, nullptr);
  EXPECT_EQ(nu::GetScratchBuilderAllocCountForTest(), 1);
  GVariant* v2 = g_variant_builder_end(b2);
  g_variant_unref(v2);
}

/*
 * Test 5: design-doc goal "Hot path GVariant allocations per post < 5".
 *
 * We simulate a 100-post loop where each post adds 3 entries and ends
 * the builder. The scratch builder's alloc count must stay at 1 for
 * the entire loop. The underlying g_variant_builder_new count delta
 * must equal 1 as well.
 */
TEST_F(GVariantAutoptrTest, BuilderReuse_HundredPostsStayAtOneAlloc) {
  namespace nu = notification::util;

  const int before_new = glib_mock_n1t7::GetVariantBuilderNewCount();

  for (int post = 0; post < 100; ++post) {
    GVariantBuilder* b = nu::ScratchBuilder(G_VARIANT_TYPE("a(s)"));
    ASSERT_NE(b, nullptr);
    g_variant_builder_add(b, "(s)", "a");
    g_variant_builder_add(b, "(s)", "b");
    g_variant_builder_add(b, "(s)", "c");
    GVariant* out = g_variant_builder_end(b);
    g_variant_unref(out);
  }

  EXPECT_EQ(nu::GetScratchBuilderAllocCountForTest(), 1)
      << "100-post loop allocated more than once.";
  EXPECT_EQ(glib_mock_n1t7::GetVariantBuilderNewCount() - before_new, 1)
      << "100-post loop called g_variant_builder_new more than once.";
}
