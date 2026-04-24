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
 * N3-T2 C2: EventInfo::ToGVariant / FromGVariant round-trip.
 *
 * Validates that the four IEventInfo fields (type / uid / request_id /
 * error) survive a ToGVariant -> FromGVariant cycle without going
 * through the bundle+GVariant double encoding. Phase 3 scope is
 * intentionally limited to these four fields; the remaining fields
 * (owner / channel / item_id / validated_uid / validated_owner) still
 * flow through the legacy bundle path and are covered by the existing
 * EventInfoTest.SerializeDeserialize case.
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <glib.h>

#include "mock/app_common_mock.hh"
#include "mock/aul_mock.hh"
#include "mock/test_fixture.hh"

#include "notification-ex/event_info_internal.h"

using namespace notification;

namespace {
class Mocks :
    virtual public ::testing::NiceMock<AulMock>,
    virtual public ::testing::NiceMock<AppCommonMock> {};
}  // namespace

class DirectGVariantRoundTrip : public TestFixture {
 public:
  DirectGVariantRoundTrip() : TestFixture(std::make_unique<::Mocks>()) {}
};

TEST_F(DirectGVariantRoundTrip, IEventInfoMapsCorrectly) {
  /* Arrange: fully populated EventInfo with non-default values so a
   * missing field in the decoder would be obvious. */
  EventInfo src(EventInfo::Update, "owner@example", "channel-42", "item-id-xyz");
  src.SetUid(static_cast<uid_t>(5001));
  src.SetError(ERROR_INVALID_PARAMETER);
  /* request_id_ is auto-assigned at construction (util::GetRequestId).
   * Capture whatever it is so the comparison survives sequence bumps. */
  int expected_request_id = src.GetRequestId();
  int expected_type = src.GetEventType();
  uid_t expected_uid = src.GetUid();
  NotificationError expected_error = src.GetError();

  /* Act: direct-GVariant round-trip. */
  GVariant* wire = src.ToGVariant();
  ASSERT_NE(wire, nullptr);
  ASSERT_TRUE(g_variant_is_of_type(wire, G_VARIANT_TYPE("a{sv}")));

  /* `wire` still carries a floating ref from g_variant_builder_end.
   * Sink it so the unref below actually frees the object. */
  GVariant* owned = g_variant_ref_sink(wire);

  /* Decode into a fresh EventInfo whose scalar fields start at
   * deliberately different values — this exposes any silent no-op in
   * FromGVariant (the prior fields must be overwritten). */
  EventInfo dst(EventInfo::Post, "other-owner", "other-channel", "other-id");
  dst.SetUid(static_cast<uid_t>(9999));
  dst.SetError(ERROR_NONE);
  bool ok = dst.FromGVariant(owned);
  ASSERT_TRUE(ok);

  /* Assert: the four in-scope fields round-tripped. */
  EXPECT_EQ(dst.GetEventType(), expected_type);
  EXPECT_EQ(dst.GetUid(), expected_uid);
  EXPECT_EQ(dst.GetRequestId(), expected_request_id);
  EXPECT_EQ(dst.GetError(), expected_error);

  /* Phase 3 out-of-scope fields stay at the constructor-supplied values
   * — FromGVariant does NOT clobber owner / channel / item_id. Pin that
   * invariant so Phase 4 (which will extend the direct path) does not
   * silently regress this tolerance while the migration is in-flight. */
  EXPECT_EQ(dst.GetOwner(), "other-owner");
  EXPECT_EQ(dst.GetChannel(), "other-channel");
  EXPECT_EQ(dst.GetItemId(), "other-id");

  g_variant_unref(owned);
}

TEST_F(DirectGVariantRoundTrip, FromGVariantRejectsWrongType) {
  EventInfo dst(EventInfo::Post, "owner");
  int before_type = dst.GetEventType();
  uid_t before_uid = dst.GetUid();

  /* A tuple-typed variant is NOT a{sv} and must be rejected. */
  GVariant* wrong = g_variant_new("(si)", "hello", 42);
  GVariant* owned = g_variant_ref_sink(wrong);
  ASSERT_FALSE(dst.FromGVariant(owned));

  /* Null input is also a safe no-op. */
  ASSERT_FALSE(dst.FromGVariant(nullptr));

  /* State must be unchanged after a rejected decode. */
  EXPECT_EQ(dst.GetEventType(), before_type);
  EXPECT_EQ(dst.GetUid(), before_uid);

  g_variant_unref(owned);
}

TEST_F(DirectGVariantRoundTrip, FromGVariantIgnoresUnknownKeys) {
  /* Forward-compat check: a payload that contains extra keys the
   * current Phase 3 decoder does not know about must still populate
   * the four known ones and must not error out. */
  GVariantBuilder b;
  g_variant_builder_init(&b, G_VARIANT_TYPE("a{sv}"));
  g_variant_builder_add(&b, "{sv}", "type", g_variant_new_int32(
      static_cast<int>(EventInfo::Delete)));
  g_variant_builder_add(&b, "{sv}", "uid", g_variant_new_int32(777));
  g_variant_builder_add(&b, "{sv}", "request_id", g_variant_new_int32(123));
  g_variant_builder_add(&b, "{sv}", "error",
      g_variant_new_int32(static_cast<int>(ERROR_IO_ERROR)));
  /* Phase 4-style unknown key: decoder must skip silently. */
  g_variant_builder_add(&b, "{sv}", "future_owner",
      g_variant_new_string("plugin-42"));
  GVariant* v = g_variant_builder_end(&b);
  GVariant* owned = g_variant_ref_sink(v);

  EventInfo dst(EventInfo::Post, "other-owner", "other-channel", "other-id");
  ASSERT_TRUE(dst.FromGVariant(owned));

  EXPECT_EQ(dst.GetEventType(), EventInfo::Delete);
  EXPECT_EQ(dst.GetUid(), static_cast<uid_t>(777));
  EXPECT_EQ(dst.GetRequestId(), 123);
  EXPECT_EQ(dst.GetError(), ERROR_IO_ERROR);

  g_variant_unref(owned);
}
