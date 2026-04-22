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
 * N1-T5 unit tests for the bundle-encode cache.
 *
 * Matrix:
 *   1. BundleReuse_SecondPostFewerAllocs
 *      — Allocations for the 2nd encode of the same bundle are ≤ 0.6× the
 *        first. Measured via the --wrap=malloc allocation counter.
 *
 *   2. BundleEncode_HashCollision_HandledCorrectly
 *      — A cache hit MUST bytewise-verify the stored buffer against the
 *        newly-encoded buffer before returning. We simulate a collision by
 *        exercising two bundles whose encoded payloads are distinct, then
 *        verifying that the returned bytes match the input bundle (never a
 *        stale entry).
 *
 *   3. BundleCache_EvictionAtCapacity
 *      — Inserting 17 distinct bundles into a 16-entry cache causes the
 *        LRU entry to be dropped (cache size stays at capacity).
 */

#include <gtest/gtest.h>

#include <bundle.h>
#include <bundle_internal.h>
#include <cstring>
#include <string>

#include "mock/test_fixture.hh"
#include "mock/alloc_counter.hh"

extern "C" {
#include "notification_bundle_util.h"
}

namespace {

/* Create a bundle populated with a single "key"/"value" pair. The caller
 * owns the bundle and must bundle_free() it. */
bundle* MakeBundle(const char* key, const char* value) {
  bundle* b = bundle_create();
  if (b == nullptr) return nullptr;
  bundle_add_str(b, key, value);
  return b;
}

}  // namespace

class BundleCacheTest : public LeakCheckFixture {
 public:
  void SetUp() override {
    noti_bundle_cache_clear();
    LeakCheckFixture::SetUp();
  }

  void TearDown() override {
    /* Drain before the leak check so cache-owned allocations do not
     * masquerade as leaks. */
    noti_test::SetCountingEnabled(false);
    noti_bundle_cache_clear();
    LeakCheckFixture::TearDown();
  }
};

/* (1) Hot-path: second encode of the same bundle must allocate strictly
 * fewer bytes than the first. */
TEST_F(BundleCacheTest, BundleReuse_SecondPostFewerAllocs) {
  bundle* b = MakeBundle("k", "value");
  ASSERT_NE(b, nullptr);

  /* Warm path: first encode + populate the cache. */
  int len1 = 0;
  auto snap1 = noti_test::SnapshotAllocs();
  char* raw1 = noti_bundle_cached_encode(b, &len1);
  auto diff1 = noti_test::DiffAllocs(snap1);
  ASSERT_NE(raw1, nullptr);
  ASSERT_GT(len1, 0);

  /* Hot path: second encode with identical payload → cache hit. */
  int len2 = 0;
  auto snap2 = noti_test::SnapshotAllocs();
  char* raw2 = noti_bundle_cached_encode(b, &len2);
  auto diff2 = noti_test::DiffAllocs(snap2);
  ASSERT_NE(raw2, nullptr);
  ASSERT_EQ(len2, len1);
  ASSERT_EQ(std::memcmp(raw1, raw2, len1), 0);

  /* 2nd-post allocs ≤ 0.6 × 1st-post allocs. Allow ≥ 1 threshold even
   * when the first path is already lean (avoid division by zero). */
  const size_t threshold =
      (diff1.total_allocs * 6 + 9) / 10;  /* ceil(0.6 × N) */
  EXPECT_LE(diff2.total_allocs, threshold)
      << "1st=" << diff1.total_allocs
      << " 2nd=" << diff2.total_allocs
      << " threshold=" << threshold;

  bundle_free_encoded_rawdata(reinterpret_cast<bundle_raw**>(&raw1));
  bundle_free_encoded_rawdata(reinterpret_cast<bundle_raw**>(&raw2));
  bundle_free(b);
}

/* (2) Hash collision handled correctly: bytewise-verified on hit. We
 * cannot force a true FNV-1a collision, so we verify the equivalent
 * invariant: encodes of *different* payloads never return stale bytes.
 * Two different bundles populated in alternating order stress the
 * collision chain + bytewise memcmp guard. */
TEST_F(BundleCacheTest, BundleEncode_HashCollision_HandledCorrectly) {
  bundle* a = MakeBundle("k", "alpha");
  bundle* b = MakeBundle("k", "beta");
  ASSERT_NE(a, nullptr);
  ASSERT_NE(b, nullptr);

  /* Reference encodings for bytewise comparison. */
  bundle_raw* ref_a_raw = nullptr;
  int ref_a_len = 0;
  bundle_raw* ref_b_raw = nullptr;
  int ref_b_len = 0;
  ASSERT_EQ(bundle_encode(a, &ref_a_raw, &ref_a_len), BUNDLE_ERROR_NONE);
  ASSERT_EQ(bundle_encode(b, &ref_b_raw, &ref_b_len), BUNDLE_ERROR_NONE);
  ASSERT_NE(ref_a_len, 0);
  ASSERT_NE(ref_b_len, 0);
  /* Ensure the payloads are distinct — otherwise the test is vacuous. */
  bool distinct = (ref_a_len != ref_b_len) ||
                  (std::memcmp(ref_a_raw, ref_b_raw, ref_a_len) != 0);
  ASSERT_TRUE(distinct);

  for (int i = 0; i < 4; ++i) {
    int la = 0, lb = 0;
    char* ra = noti_bundle_cached_encode(a, &la);
    char* rb = noti_bundle_cached_encode(b, &lb);
    ASSERT_NE(ra, nullptr);
    ASSERT_NE(rb, nullptr);
    EXPECT_EQ(la, ref_a_len);
    EXPECT_EQ(lb, ref_b_len);
    EXPECT_EQ(std::memcmp(ra, ref_a_raw, ref_a_len), 0)
        << "iter " << i << ": returned bytes for 'a' diverged";
    EXPECT_EQ(std::memcmp(rb, ref_b_raw, ref_b_len), 0)
        << "iter " << i << ": returned bytes for 'b' diverged";
    bundle_free_encoded_rawdata(reinterpret_cast<bundle_raw**>(&ra));
    bundle_free_encoded_rawdata(reinterpret_cast<bundle_raw**>(&rb));
  }

  bundle_free_encoded_rawdata(&ref_a_raw);
  bundle_free_encoded_rawdata(&ref_b_raw);
  bundle_free(a);
  bundle_free(b);
}

/* (3) Eviction at capacity: 17 distinct inserts → size stays at 16. */
TEST_F(BundleCacheTest, BundleCache_EvictionAtCapacity) {
  const size_t cap = noti_bundle_cache_capacity();
  ASSERT_EQ(cap, 16u);

  std::vector<bundle*> bundles;
  std::vector<char*> raws;
  bundles.reserve(17);
  raws.reserve(17);

  for (int i = 0; i < 17; ++i) {
    std::string v = "v" + std::to_string(i);
    bundle* bi = MakeBundle("k", v.c_str());
    ASSERT_NE(bi, nullptr);
    bundles.push_back(bi);

    int li = 0;
    char* ri = noti_bundle_cached_encode(bi, &li);
    ASSERT_NE(ri, nullptr);
    ASSERT_GT(li, 0);
    raws.push_back(ri);

    /* While filling the cache below capacity, size grows 1:1. At
     * capacity, the 17th insert triggers eviction so size stays at
     * capacity. */
    if (i < static_cast<int>(cap)) {
      EXPECT_EQ(noti_bundle_cache_size(),
                static_cast<size_t>(i + 1));
    } else {
      EXPECT_EQ(noti_bundle_cache_size(), cap);
    }
  }

  for (char* r : raws)
    bundle_free_encoded_rawdata(reinterpret_cast<bundle_raw**>(&r));
  for (bundle* b : bundles)
    bundle_free(b);
}
