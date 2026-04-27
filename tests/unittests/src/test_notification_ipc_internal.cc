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
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * N7.5-C coverage boost — notification_ipc.c (Mid 6 file group).
 *
 * notification_ipc.c is wrapped in LCOV_EXCL because it lives at the
 * boundary with rpc-port and was not historically covered by direct unit
 * tests. We add direct tests for the two EXPORT_API entry points so the
 * gvariant marshalling layer is observably correct under refactoring:
 *
 *   1. Ipc_GVariantMarshal_Roundtrip
 *      — Build a minimally-populated notification, encode to GVariant,
 *        then decode into a fresh notification. The 4 scalar fields
 *        (type / layout / group_id / priv_id) plus the pkg_id / caller_app_id
 *        strings round-trip byte-equal.
 *   2. Ipc_GVariantMarshal_NullNoti_Rejected
 *      — make_noti_from_gvariant(NULL, v) -> INVALID_PARAMETER
 *      — make_noti_from_gvariant(n, NULL) -> INVALID_PARAMETER
 *   3. Ipc_GVariantMarshal_BoolBitfield_Roundtrip
 *      — N3-T6 DS1 bridge: ongoing_flag / auto_remove / event_flag /
 *        is_translation / check_box / check_box_value are 1-bit bitfields
 *        and the encode side writes "i" through a stack-int bridge. Pin
 *        the round-trip semantics so the bridge does not silently drop
 *        bits in a future refactor.
 *   4. Ipc_GVariantMarshal_LargePayload_Roundtrip
 *      — Multi-KB strings survive the encode/decode boundary (no buffer
 *        truncation under realistic notification body sizes).
 *   5. Ipc_GVariantMarshal_BuilderReuse_Stable
 *      — Encode the same notification twice in a row and confirm the
 *        produced GVariants are byte-equal. This pins the "no
 *        TLS/scratch state leakage between calls" invariant on the
 *        builder.
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <cstdio>
#include <cstring>
#include <string>

#include <aul.h>
#include <gio/gio.h>

#include "mock/test_fixture.hh"
#include "mock/aul_mock.hh"
#include "mock/glib_mock.hh"

extern "C" {
#include <notification.h>
#include <notification_internal.h>
#include <notification_ipc.h>
#include <notification_private.h>
#include <notification_type_internal.h>
}

namespace {

class Mocks :
    virtual public ::testing::NiceMock<AulMock>,
    virtual public ::testing::NiceMock<GlibMock> {};

int FakeAulAppGetAppidByPid(int /*pid*/, char* appid, int len) {
  snprintf(appid, len, "%s", "org.tizen.unittest.app");
  return AUL_R_OK;
}

}  // namespace

using ::testing::_;
using ::testing::Invoke;

class NotificationIpcInternalTest : public TestFixture {
 public:
  NotificationIpcInternalTest() : TestFixture(std::make_unique<::Mocks>()) {}
  void SetUp() override {
    EXPECT_CALL(GetMock<AulMock>(), aul_app_get_appid_bypid(_, _, _))
        .WillRepeatedly(Invoke(FakeAulAppGetAppidByPid));
  }
};

/* (1) Round-trip the headline scalar + string fields. */
TEST_F(NotificationIpcInternalTest, Ipc_GVariantMarshal_Roundtrip) {
  notification_h src = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(src, nullptr);

  src->layout = NOTIFICATION_LY_NOTI_EVENT_SINGLE;
  src->group_id = 7;
  src->priv_id = 99;
  src->pkg_id = strdup("org.tizen.unittest.pkg");
  src->caller_app_id = strdup("org.tizen.unittest.app");
  src->display_applist = NOTIFICATION_DISPLAY_APP_ALL;

  GVariant* wire = notification_ipc_make_gvariant_from_noti(src, false);
  ASSERT_NE(wire, nullptr);
  /* The wire format is "(v)" — a tuple containing a single variant. */
  ASSERT_TRUE(g_variant_is_of_type(wire, G_VARIANT_TYPE("(v)")));
  GVariant* owned = g_variant_ref_sink(wire);

  GVariant* inner = nullptr;
  g_variant_get(owned, "(v)", &inner);
  ASSERT_NE(inner, nullptr);
  ASSERT_TRUE(g_variant_is_of_type(inner, G_VARIANT_TYPE("a{iv}")));

  notification_h dst = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(dst, nullptr);

  int ret = notification_ipc_make_noti_from_gvariant(dst, inner);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  EXPECT_EQ(dst->type, src->type);
  EXPECT_EQ(dst->layout, src->layout);
  EXPECT_EQ(dst->group_id, src->group_id);
  EXPECT_EQ(dst->priv_id, src->priv_id);
  EXPECT_EQ(dst->display_applist, src->display_applist);
  ASSERT_NE(dst->pkg_id, nullptr);
  ASSERT_NE(dst->caller_app_id, nullptr);
  EXPECT_STREQ(dst->pkg_id, src->pkg_id);
  EXPECT_STREQ(dst->caller_app_id, src->caller_app_id);

  g_variant_unref(inner);
  g_variant_unref(owned);
  notification_free(src);
  notification_free(dst);
}

/* (2) NULL-arg rejections. */
TEST_F(NotificationIpcInternalTest, Ipc_GVariantMarshal_NullNoti_Rejected) {
  /* Build a minimal valid GVariant a{iv} so we exercise only the noti
   * NULL branch. */
  GVariantBuilder b;
  g_variant_builder_init(&b, G_VARIANT_TYPE("a{iv}"));
  GVariant* v = g_variant_builder_end(&b);
  GVariant* owned = g_variant_ref_sink(v);

  EXPECT_EQ(notification_ipc_make_noti_from_gvariant(nullptr, owned),
            NOTIFICATION_ERROR_INVALID_PARAMETER);

  notification_h n = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(n, nullptr);
  EXPECT_EQ(notification_ipc_make_noti_from_gvariant(n, nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);

  notification_free(n);
  g_variant_unref(owned);
}

/* (3) Bool bitfield bridge survives round-trip (N3-T6 DS1). */
TEST_F(NotificationIpcInternalTest, Ipc_GVariantMarshal_BoolBitfield_Roundtrip) {
  notification_h src = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(src, nullptr);
  src->ongoing_flag = 1u;
  src->auto_remove = 0u;
  src->event_flag = 1u;
  src->is_translation = 1u;
  src->check_box = 1u;
  src->check_box_value = 0u;

  GVariant* wire = notification_ipc_make_gvariant_from_noti(src, false);
  ASSERT_NE(wire, nullptr);
  GVariant* owned = g_variant_ref_sink(wire);
  GVariant* inner = nullptr;
  g_variant_get(owned, "(v)", &inner);
  ASSERT_NE(inner, nullptr);

  notification_h dst = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(dst, nullptr);
  /* Pre-set opposite bits so a missing decode write would be obvious. */
  dst->ongoing_flag = 0u;
  dst->auto_remove = 1u;
  dst->event_flag = 0u;
  dst->is_translation = 0u;
  dst->check_box = 0u;
  dst->check_box_value = 1u;

  EXPECT_EQ(notification_ipc_make_noti_from_gvariant(dst, inner),
            NOTIFICATION_ERROR_NONE);

  EXPECT_EQ(dst->ongoing_flag, src->ongoing_flag);
  EXPECT_EQ(dst->auto_remove, src->auto_remove);
  EXPECT_EQ(dst->event_flag, src->event_flag);
  EXPECT_EQ(dst->is_translation, src->is_translation);
  EXPECT_EQ(dst->check_box, src->check_box);
  EXPECT_EQ(dst->check_box_value, src->check_box_value);

  g_variant_unref(inner);
  g_variant_unref(owned);
  notification_free(src);
  notification_free(dst);
}

/* (4) Multi-KB string round-trip. */
TEST_F(NotificationIpcInternalTest, Ipc_GVariantMarshal_LargePayload_Roundtrip) {
  notification_h src = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(src, nullptr);
  /* 4 KB payload — well above any plausible inline buffer. */
  std::string big(4096, 'A');
  src->pkg_id = strdup(big.c_str());
  src->caller_app_id = strdup(big.c_str());
  ASSERT_NE(src->pkg_id, nullptr);
  ASSERT_NE(src->caller_app_id, nullptr);

  GVariant* wire = notification_ipc_make_gvariant_from_noti(src, false);
  ASSERT_NE(wire, nullptr);
  GVariant* owned = g_variant_ref_sink(wire);
  GVariant* inner = nullptr;
  g_variant_get(owned, "(v)", &inner);
  ASSERT_NE(inner, nullptr);

  notification_h dst = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(dst, nullptr);
  EXPECT_EQ(notification_ipc_make_noti_from_gvariant(dst, inner),
            NOTIFICATION_ERROR_NONE);
  ASSERT_NE(dst->pkg_id, nullptr);
  ASSERT_NE(dst->caller_app_id, nullptr);
  EXPECT_EQ(strlen(dst->pkg_id), big.size());
  EXPECT_EQ(strlen(dst->caller_app_id), big.size());
  EXPECT_STREQ(dst->pkg_id, big.c_str());
  EXPECT_STREQ(dst->caller_app_id, big.c_str());

  g_variant_unref(inner);
  g_variant_unref(owned);
  notification_free(src);
  notification_free(dst);
}

/* (5) Encoding the same notification twice produces identical wires.
 *     Pins the "no per-call scratch leakage in the GVariantBuilder seam". */
TEST_F(NotificationIpcInternalTest, Ipc_GVariantMarshal_BuilderReuse_Stable) {
  notification_h src = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(src, nullptr);
  src->priv_id = 12345;
  src->pkg_id = strdup("org.tizen.dual");
  src->caller_app_id = strdup("org.tizen.dual.app");

  GVariant* w1 = notification_ipc_make_gvariant_from_noti(src, false);
  GVariant* w2 = notification_ipc_make_gvariant_from_noti(src, false);
  ASSERT_NE(w1, nullptr);
  ASSERT_NE(w2, nullptr);
  GVariant* o1 = g_variant_ref_sink(w1);
  GVariant* o2 = g_variant_ref_sink(w2);

  /* g_variant_equal compares value-wise, not pointer-wise. */
  EXPECT_TRUE(g_variant_equal(o1, o2));

  g_variant_unref(o1);
  g_variant_unref(o2);
  notification_free(src);
}
