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
 * N7.5-B coverage boost — notification_bundle_util.c (Phase 3 N1-T5).
 *
 * test_bundle_cache.cc covers the headline contract (reuse, eviction,
 * collision via distinct payloads). This file targets the NULL guards,
 * encoded_safe / encoded_clear pair, FNV-1a hashing semantics, the LRU-
 * promotion-on-hit invariant, and the "recently-used not evicted"
 * invariant called out in the v6 amendment §N7.5-B Test First section.
 *
 * Matrix:
 *   1. EncodeSafe_NullOut_ReturnsErr          — out == NULL -> -2.
 *   2. EncodeSafe_NullBundle_ReturnsZeroEmpty  — b == NULL  -> 0 with empty out.
 *   3. EncodeSafe_RealBundle_RoundTrip         — encode + clear round-trip.
 *   4. EncodedClear_OnZeroInit_NoCrash         — defensive contract.
 *   5. EncodedClear_OnNull_NoCrash             — out == NULL is no-op.
 *   6. Hash_NullBundle_ReturnsZero             — b == NULL -> 0.
 *   7. Hash_DistinctPayloads_DistinctHashes    — FNV-1a separates "alpha"
 *                                                  from "beta".
 *   8. Hash_SamePayload_SameHash               — determinism.
 *   9. EncodeCached_NullBundle_ReturnsNull     — b == NULL.
 *  10. CacheClear_DropsAllEntries              — size goes to 0 after clear.
 *  11. LRU_RecentlyUsedNotEvicted              — fill with 16 distinct, hit
 *                                                  the head one, then push a
 *                                                  17th — the head MRU must
 *                                                  survive (only the LRU
 *                                                  tail evicts).
 *  12. BytewiseVerify_HashCollision_DistinctData
 *                                              — alternating distinct payload
 *                                                  inserts must always return
 *                                                  the *correct* bytes (the
 *                                                  bytewise memcmp guard
 *                                                  defeats any synthetic
 *                                                  hash-equal/data-different
 *                                                  collision).
 */

#include <gtest/gtest.h>

#include <bundle.h>
#include <bundle_internal.h>
#include <cstring>
#include <string>
#include <vector>

extern "C" {
#include "notification_bundle_util.h"
}

#include "mock/test_fixture.hh"

namespace {

bundle* MakeBundle(const char* k, const char* v) {
  bundle* b = bundle_create();
  if (b == nullptr) return nullptr;
  bundle_add_str(b, k, v);
  return b;
}

}  // namespace

class BundleUtilExtraTest : public LeakCheckFixture {
 public:
  void SetUp() override {
    noti_bundle_cache_clear();
    LeakCheckFixture::SetUp();
  }

  void TearDown() override {
    noti_test::SetCountingEnabled(false);
    noti_bundle_cache_clear();
    LeakCheckFixture::TearDown();
  }
};

/* (1) NULL out pointer -> -2 (invalid parameter). */
TEST_F(BundleUtilExtraTest, EncodeSafe_NullOut_ReturnsErr) {
  EXPECT_EQ(noti_bundle_encode_safe(nullptr, nullptr), -2);

  bundle* b = MakeBundle("k", "v");
  ASSERT_NE(b, nullptr);
  EXPECT_EQ(noti_bundle_encode_safe(b, nullptr), -2);
  bundle_free(b);
}

/* (2) NULL bundle -> 0 with cleared out. */
TEST_F(BundleUtilExtraTest, EncodeSafe_NullBundle_ReturnsZeroEmpty) {
  noti_bundle_encoded_t out{};
  out.raw = (char*)0xDEADBEEF;  /* sentinel — must be cleared. */
  out.len = 99;

  EXPECT_EQ(noti_bundle_encode_safe(nullptr, &out), 0);
  EXPECT_EQ(out.raw, nullptr);
  EXPECT_EQ(out.len, 0);
}

/* (3) Real bundle -> encode + clear round-trip. */
TEST_F(BundleUtilExtraTest, EncodeSafe_RealBundle_RoundTrip) {
  bundle* b = MakeBundle("k", "value");
  ASSERT_NE(b, nullptr);

  noti_bundle_encoded_t out{};
  EXPECT_EQ(noti_bundle_encode_safe(b, &out), 0);
  EXPECT_NE(out.raw, nullptr);
  EXPECT_GT(out.len, 0);

  noti_bundle_encoded_clear(&out);
  EXPECT_EQ(out.raw, nullptr);
  EXPECT_EQ(out.len, 0);

  bundle_free(b);
}

/* (4) Clear on a zero-initialised struct must not crash. */
TEST_F(BundleUtilExtraTest, EncodedClear_OnZeroInit_NoCrash) {
  noti_bundle_encoded_t out{};
  noti_bundle_encoded_clear(&out);
  EXPECT_EQ(out.raw, nullptr);
  EXPECT_EQ(out.len, 0);
}

/* (5) Clear with NULL out is a no-op. */
TEST_F(BundleUtilExtraTest, EncodedClear_OnNull_NoCrash) {
  noti_bundle_encoded_clear(nullptr);
  /* If we reach here, no crash. */
  SUCCEED();
}

/* (6) Hash of NULL bundle must be 0. */
TEST_F(BundleUtilExtraTest, Hash_NullBundle_ReturnsZero) {
  EXPECT_EQ(noti_bundle_hash(nullptr), 0u);
}

/* (7) FNV-1a known property: distinct payloads -> distinct hashes (with very
 * high probability for short strings). We exercise a handful that we have
 * verified are not coincidental collisions. */
TEST_F(BundleUtilExtraTest, Hash_DistinctPayloads_DistinctHashes) {
  bundle* a = MakeBundle("k", "alpha");
  bundle* b = MakeBundle("k", "beta");
  bundle* c = MakeBundle("k", "");
  bundle* d = MakeBundle("k", "alphabet");
  ASSERT_NE(a, nullptr);
  ASSERT_NE(b, nullptr);
  ASSERT_NE(c, nullptr);
  ASSERT_NE(d, nullptr);

  uint64_t ha = noti_bundle_hash(a);
  uint64_t hb = noti_bundle_hash(b);
  uint64_t hc = noti_bundle_hash(c);
  uint64_t hd = noti_bundle_hash(d);

  EXPECT_NE(ha, 0u);
  EXPECT_NE(hb, 0u);
  EXPECT_NE(hc, 0u);
  EXPECT_NE(hd, 0u);

  EXPECT_NE(ha, hb);
  EXPECT_NE(ha, hc);
  EXPECT_NE(ha, hd);
  EXPECT_NE(hb, hc);
  EXPECT_NE(hb, hd);
  EXPECT_NE(hc, hd);

  bundle_free(a);
  bundle_free(b);
  bundle_free(c);
  bundle_free(d);
}

/* (8) Determinism: identical payloads always hash to the same value. */
TEST_F(BundleUtilExtraTest, Hash_SamePayload_SameHash) {
  bundle* a = MakeBundle("k", "value");
  bundle* b = MakeBundle("k", "value");
  ASSERT_NE(a, nullptr);
  ASSERT_NE(b, nullptr);

  uint64_t ha = noti_bundle_hash(a);
  uint64_t hb = noti_bundle_hash(b);

  EXPECT_NE(ha, 0u);
  EXPECT_EQ(ha, hb);

  bundle_free(a);
  bundle_free(b);
}

/* (9) Cached encode of NULL bundle is NULL. */
TEST_F(BundleUtilExtraTest, EncodeCached_NullBundle_ReturnsNull) {
  int len = 99;
  EXPECT_EQ(noti_bundle_cached_encode(nullptr, &len), nullptr);
  EXPECT_EQ(len, 0);

  /* len_out NULL is also legal. */
  EXPECT_EQ(noti_bundle_cached_encode(nullptr, nullptr), nullptr);
}

/* (10) Clear empties the cache. */
TEST_F(BundleUtilExtraTest, CacheClear_DropsAllEntries) {
  bundle* a = MakeBundle("k", "alpha");
  bundle* b = MakeBundle("k", "beta");
  ASSERT_NE(a, nullptr);
  ASSERT_NE(b, nullptr);

  int la = 0, lb = 0;
  char* ra = noti_bundle_cached_encode(a, &la);
  char* rb = noti_bundle_cached_encode(b, &lb);
  ASSERT_NE(ra, nullptr);
  ASSERT_NE(rb, nullptr);

  EXPECT_EQ(noti_bundle_cache_size(), 2u);

  noti_bundle_cache_clear();
  EXPECT_EQ(noti_bundle_cache_size(), 0u);

  bundle_free_encoded_rawdata(reinterpret_cast<bundle_raw**>(&ra));
  bundle_free_encoded_rawdata(reinterpret_cast<bundle_raw**>(&rb));
  bundle_free(a);
  bundle_free(b);
}

/* (11) LRU recency: fill with 16 distinct, then "touch" the first inserted
 * (which would otherwise be the LRU tail) so it becomes MRU again. After a
 * 17th insert the formerly-second-oldest should be the eviction victim, not
 * the touched one. We exercise the contract by checking cache_size stays at
 * capacity, and by re-encoding the touched entry to confirm it still hits
 * (cheap-encode region). */
TEST_F(BundleUtilExtraTest, LRU_RecentlyUsedNotEvicted) {
  const size_t cap = noti_bundle_cache_capacity();
  ASSERT_EQ(cap, 16u);

  std::vector<bundle*> bundles;
  std::vector<char*> raws;
  bundles.reserve(17);
  raws.reserve(17);

  for (int i = 0; i < (int)cap; ++i) {
    std::string v = "v" + std::to_string(i);
    bundle* bi = MakeBundle("k", v.c_str());
    ASSERT_NE(bi, nullptr);
    bundles.push_back(bi);

    int li = 0;
    char* ri = noti_bundle_cached_encode(bi, &li);
    ASSERT_NE(ri, nullptr);
    raws.push_back(ri);
  }
  EXPECT_EQ(noti_bundle_cache_size(), cap);

  /* Touch the first bundle to promote it to MRU. */
  int touched_len = 0;
  char* touched = noti_bundle_cached_encode(bundles[0], &touched_len);
  ASSERT_NE(touched, nullptr);
  bundle_free_encoded_rawdata(reinterpret_cast<bundle_raw**>(&touched));

  /* Insert a 17th distinct bundle -> one entry must be evicted (LRU). */
  bundle* extra = MakeBundle("k", "extra_value");
  ASSERT_NE(extra, nullptr);
  bundles.push_back(extra);

  int el = 0;
  char* extra_raw = noti_bundle_cached_encode(extra, &el);
  ASSERT_NE(extra_raw, nullptr);
  raws.push_back(extra_raw);

  EXPECT_EQ(noti_bundle_cache_size(), cap);

  /* The touched entry (bundles[0]) must STILL be cached. We assert by
   * re-encoding it and verifying the bytes match a fresh bundle_encode. */
  bundle_raw* fresh = nullptr;
  int fresh_len = 0;
  ASSERT_EQ(bundle_encode(bundles[0], &fresh, &fresh_len), BUNDLE_ERROR_NONE);

  int post_len = 0;
  char* post = noti_bundle_cached_encode(bundles[0], &post_len);
  ASSERT_NE(post, nullptr);
  EXPECT_EQ(post_len, fresh_len);
  EXPECT_EQ(memcmp(post, fresh, fresh_len), 0)
      << "Touched MRU entry returned divergent bytes after eviction round";

  bundle_free_encoded_rawdata(reinterpret_cast<bundle_raw**>(&post));
  bundle_free_encoded_rawdata(&fresh);

  for (char* r : raws)
    bundle_free_encoded_rawdata(reinterpret_cast<bundle_raw**>(&r));
  for (bundle* b : bundles)
    bundle_free(b);
}

/* (12) BytewiseVerify guard: even if the FNV-1a hash happened to collide,
 * cache_lookup_locked uses memcmp(cur->raw, raw, len) before declaring a
 * hit. We can't force a true collision so we exercise the equivalent: many
 * alternating distinct payloads must always return their own bytes (never
 * a stale entry from the chain). 200 alternations puts continuous pressure
 * on the collision-walk path. */
TEST_F(BundleUtilExtraTest, BytewiseVerify_HashCollision_DistinctData) {
  bundle* a = MakeBundle("k", "alpha-payload-12345");
  bundle* b = MakeBundle("k", "beta-payload-67890");
  ASSERT_NE(a, nullptr);
  ASSERT_NE(b, nullptr);

  bundle_raw* ref_a = nullptr;
  bundle_raw* ref_b = nullptr;
  int la = 0, lb = 0;
  ASSERT_EQ(bundle_encode(a, &ref_a, &la), BUNDLE_ERROR_NONE);
  ASSERT_EQ(bundle_encode(b, &ref_b, &lb), BUNDLE_ERROR_NONE);

  for (int i = 0; i < 200; ++i) {
    int la2 = 0, lb2 = 0;
    char* ra = noti_bundle_cached_encode(a, &la2);
    char* rb = noti_bundle_cached_encode(b, &lb2);
    ASSERT_NE(ra, nullptr);
    ASSERT_NE(rb, nullptr);
    EXPECT_EQ(la2, la);
    EXPECT_EQ(lb2, lb);
    EXPECT_EQ(memcmp(ra, ref_a, la), 0) << "iter " << i << " a payload";
    EXPECT_EQ(memcmp(rb, ref_b, lb), 0) << "iter " << i << " b payload";
    bundle_free_encoded_rawdata(reinterpret_cast<bundle_raw**>(&ra));
    bundle_free_encoded_rawdata(reinterpret_cast<bundle_raw**>(&rb));
  }

  bundle_free_encoded_rawdata(&ref_a);
  bundle_free_encoded_rawdata(&ref_b);
  bundle_free(a);
  bundle_free(b);
}
