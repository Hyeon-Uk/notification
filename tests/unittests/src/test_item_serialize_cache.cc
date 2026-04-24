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
 * N3-T1 D7: regression guard for the AbstractItem::Impl serialisation
 * cache.
 *
 *   SecondSerializeZeroAlloc
 *     First Serialize() builds and caches the Bundle. The second call
 *     with no intervening setter must allocate strictly fewer bytes —
 *     the 30+ Bundle::Add() calls are skipped and only a Bundle copy
 *     runs.
 *
 *   SetterInvalidatesCache
 *     A setter flips dirty_ back to true; the next Serialize rebuilds
 *     and the new value lands in the serialised Bundle.
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <memory>

#include <bundle_cpp.h>

#include "notification-ex/button_item.h"

#include "mock/aul_mock.hh"
#include "mock/app_common_mock.hh"
#include "mock/test_fixture.hh"
#include "mock/alloc_counter.hh"

using ::testing::_;
using ::testing::Invoke;

namespace {

class Mocks :
    virtual public ::testing::NiceMock<AulMock>,
    virtual public ::testing::NiceMock<AppCommonMock> {};

class ItemSerializeCacheTest : public TestFixture {
 public:
  ItemSerializeCacheTest() : TestFixture(std::make_unique<::Mocks>()) {}

  void SetUp() override {
    /* ButtonItem::Impl ctor invokes util::GetAppId() which funnels
     * through the LRU and hits aul_app_get_appid_bypid. Default-
     * install a simple responder so construction does not throw. */
    ON_CALL(GetMock<AulMock>(), aul_app_get_appid_bypid(_, _, _))
        .WillByDefault(Invoke([](int, char* buf, int len) -> int {
          std::snprintf(buf, static_cast<size_t>(len),
                        "org.tizen.cachetest");
          return 0;
        }));
  }

  std::unique_ptr<notification::item::ButtonItem> MakeItem() {
    return std::make_unique<notification::item::ButtonItem>(
        std::string("test-id"), std::string("test-title"));
  }
};

TEST_F(ItemSerializeCacheTest, SecondSerializeZeroAlloc) {
  auto item = MakeItem();

  /* Cold path: first Serialize rebuilds and caches. */
  noti_test::SetCountingEnabled(true);
  auto snap1 = noti_test::SnapshotAllocs();
  tizen_base::Bundle b1 = item->Serialize();
  auto diff1 = noti_test::DiffAllocs(snap1);

  /* Hot path: cache hit, no Bundle::Add replay. */
  auto snap2 = noti_test::SnapshotAllocs();
  tizen_base::Bundle b2 = item->Serialize();
  auto diff2 = noti_test::DiffAllocs(snap2);
  noti_test::SetCountingEnabled(false);

  /* Second ≤ 60% × first (same ceiling used by
   * BundleCacheTest.BundleReuse_SecondPostFewerAllocs). */
  const size_t threshold = (diff1.total_allocs * 6 + 9) / 10;
  EXPECT_LE(diff2.total_allocs, threshold)
      << "1st=" << diff1.total_allocs
      << " 2nd=" << diff2.total_allocs
      << " threshold=" << threshold;

  /* Sanity: the cached Bundle carries the same id key value. */
  EXPECT_FALSE(b1.GetString("__ABSTRACT_ITEM_ID_KEY__").empty());
  EXPECT_EQ(b1.GetString("__ABSTRACT_ITEM_ID_KEY__"),
            b2.GetString("__ABSTRACT_ITEM_ID_KEY__"));
}

TEST_F(ItemSerializeCacheTest, SetterInvalidatesCache) {
  auto item = MakeItem();

  /* Prime the cache. */
  (void)item->Serialize();

  /* Setter flips dirty_ — the next Serialize must rebuild and carry
   * the new tag. */
  item->SetTag("mutated-tag-value");
  auto b = item->Serialize();
  EXPECT_EQ(b.GetString("__ABSTRACT_ITEM_TAG_KEY__"),
            std::string("mutated-tag-value"))
      << "Setter value must propagate into the next Serialize";
}

}  // namespace
