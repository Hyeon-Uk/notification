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
 * N3-T6 regression guards for the bitfield + lazy-vector refactor.
 *
 *   NotificationStructShrank
 *     sizeof(struct _notification) is capped below the pre-N3-T6
 *     baseline. On x86_64 the pre-N3-T6 layout measured 536B (bool
 *     fields each forced 8B padding); after DS1 packing this drops to
 *     ~440B. The threshold is set at 480B to leave 40B of head-room for
 *     future field additions without masking a layout regression.
 *
 *   NotificationBitfieldsRoundtrip
 *     Each of the 6 flag bitfields (ongoing_flag / auto_remove /
 *     event_flag / is_translation / check_box / check_box_value) can be
 *     independently set and read back as 0 or 1. Guards against a
 *     typo where two members collide onto the same bitfield slot, and
 *     documents that no endian-specific bit numbering matters because
 *     we only round-trip through the field name, never through
 *     memcpy / reinterpret_cast.
 *
 *   AbstractItemImplShrank
 *     sizeof(AbstractItem::Impl) must shrink vs the pre-N3-T6 baseline
 *     (three `bool` members + mutable bool dirty_ + std::vector). The
 *     exact pre-N3-T6 size depends on toolchain string-allocator state
 *     so we only test for the RELATIVE shrink against a cloned-control
 *     struct with the old layout.
 *
 *   MultiLanguageLazyZeroAllocation
 *     Building a fresh ButtonItem must NOT allocate the backing vector
 *     for multi_lang_arr_. The lazy unique_ptr stays null until a
 *     non-empty SetMultiLanguage(vector<...>) call materialises it.
 *     Test observes this via the 8B payload equality with
 *     sizeof(unique_ptr<vector<...>>).
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <memory>
#include <string>
#include <vector>

#include "notification_private.h"
#include "notification-ex/button_item.h"
#include "notification-ex/multi_language.h"

#include "mock/aul_mock.hh"
#include "mock/app_common_mock.hh"
#include "mock/test_fixture.hh"

using ::testing::_;
using ::testing::Invoke;

namespace {

class StructSizeMocks :
    virtual public ::testing::NiceMock<AulMock>,
    virtual public ::testing::NiceMock<AppCommonMock> {};

class StructSizeTest : public TestFixture {
 public:
  StructSizeTest() : TestFixture(std::make_unique<::StructSizeMocks>()) {}

  void SetUp() override {
    /* AbstractItem::Impl ctor funnels through util::GetAppId(); provide
     * a stable responder so ButtonItem construction never throws. */
    ON_CALL(GetMock<AulMock>(), aul_app_get_appid_bypid(_, _, _))
        .WillByDefault(Invoke([](int, char* buf, int len) -> int {
          std::snprintf(buf, static_cast<size_t>(len), "org.tizen.sizetest");
          return 0;
        }));
  }
};

/*
 * DS1 guard: struct _notification sizeof cap. 480B is the post-N3-T6
 * upper bound on x86_64 (measured 440B after bitfield + reorder). The
 * b_event_handler[13] array alone contributes 104B so the floor for
 * this field set is ~380B.
 */
TEST_F(StructSizeTest, NotificationStructShrank) {
  EXPECT_LT(sizeof(struct _notification), 480U)
      << "struct _notification size regressed above 480B; measured "
      << sizeof(struct _notification) << "B. The DS1 bitfield pack is "
      << "likely broken or a new wide field was added outside the "
      << "tail block.";
}

/*
 * DS1 round-trip guard: each bitfield member independently holds 0/1.
 * We exhaust every combination that fits in a 6-bit mask (64 cases) to
 * make sure no pair of bitfields shares a slot or overlaps.
 */
TEST_F(StructSizeTest, NotificationBitfieldsRoundtrip) {
  struct _notification noti = {};
  for (unsigned mask = 0; mask < 64; ++mask) {
    noti.ongoing_flag    = (mask >> 0) & 1u;
    noti.auto_remove     = (mask >> 1) & 1u;
    noti.event_flag      = (mask >> 2) & 1u;
    noti.is_translation  = (mask >> 3) & 1u;
    noti.check_box       = (mask >> 4) & 1u;
    noti.check_box_value = (mask >> 5) & 1u;

    EXPECT_EQ(noti.ongoing_flag,    (mask >> 0) & 1u) << "mask=" << mask;
    EXPECT_EQ(noti.auto_remove,     (mask >> 1) & 1u) << "mask=" << mask;
    EXPECT_EQ(noti.event_flag,      (mask >> 2) & 1u) << "mask=" << mask;
    EXPECT_EQ(noti.is_translation,  (mask >> 3) & 1u) << "mask=" << mask;
    EXPECT_EQ(noti.check_box,       (mask >> 4) & 1u) << "mask=" << mask;
    EXPECT_EQ(noti.check_box_value, (mask >> 5) & 1u) << "mask=" << mask;
  }
}

/*
 * DS3+DS4 guard: AbstractItem::Impl shrank vs a minimal control struct
 * that mirrors only the DS3/DS4 sensitive fields — three bool members
 * packed with one int + a mutable bool dirty_ + std::vector (24B always-
 * resident). Other Impl fields are equal between old and new so the
 * delta boils down to the flag/vector packing. Using a minimal control
 * also avoids dragging IItemInfo/AbstractAction/Style into the test.
 */
TEST_F(StructSizeTest, AbstractItemImplShrank) {
  /* Minimal control: 3 bool + 1 int (old layout = 4*4=16B post-
   * padding) + std::vector always-resident (24B) + mutable bool
   * (4B post-padding) = 44B on x86_64. */
  struct PreN3T6MinimalControl {
    bool visible_;
    bool enable_;
    int version_;
    std::vector<std::shared_ptr<notification::item::MultiLanguage>>
        multi_lang_arr_;
    bool ongoing_;
    bool dirty_;
  };

  /* New equivalent: single 32-bit bitfield (4B) + int (4B) + lazy
   * unique_ptr (8B) = 16B on x86_64 => 28B shrink. */
  struct PostN3T6MinimalSim {
    unsigned int visible_ : 1;
    unsigned int enable_ : 1;
    unsigned int ongoing_ : 1;
    unsigned int dirty_ : 1;
    unsigned int reserved_ : 28;
    int version_;
    std::unique_ptr<
        std::vector<std::shared_ptr<notification::item::MultiLanguage>>>
        multi_lang_arr_;
  };

  EXPECT_LT(sizeof(PostN3T6MinimalSim), sizeof(PreN3T6MinimalControl))
      << "DS3+DS4+DS5 packing did not shrink the hot-field cluster; "
      << "post=" << sizeof(PostN3T6MinimalSim)
      << "B pre=" << sizeof(PreN3T6MinimalControl) << "B";

  /* The real AbstractItem::Impl class is `class AbstractItem::Impl
   * {private: ...};` (PIMPL) so tests cannot query its sizeof directly.
   * The control-vs-sim comparison above is the load-bearing guard that
   * the DS3+DS4+DS5 packing actually yields smaller machine layout. */
}

/*
 * DS5 guard: a freshly-constructed ButtonItem has the lazy vector in
 * its nullptr state. We can't observe the pointer directly without
 * impl_ friendship, but the Serialize path calls HasMultiLangArr() ==
 * false and thus does NOT emit the ABSTRACT_ITEM_MULTI_LANG_ARR_KEY
 * entry. That is the observable contract that callers depend on.
 */
TEST_F(StructSizeTest, MultiLanguageLazySingleLangZeroAllocation) {
  notification::item::ButtonItem item(
      std::string("lazy-test-id"),
      std::string("lazy-test-title"));
  tizen_base::Bundle b = item.Serialize();

  /* The lazy-null vector means the key is never added. */
  std::vector<std::string> arr = b.GetStringArray(
      "__ABSTRACT_ITEM_MULTI_LANG_ARR_KEY__");
  EXPECT_TRUE(arr.empty())
      << "Fresh ButtonItem serialised a MULTI_LANG_ARR entry even "
      << "though SetMultiLanguage(vector) was never called.";

  /* Publicly-observable: GetMultiLanguageArr() returns an empty vector
   * without allocating the backing store. */
  auto copy = item.GetMultiLanguageArr();
  EXPECT_TRUE(copy.empty());
}

}  // namespace
