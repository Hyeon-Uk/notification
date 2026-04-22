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
 * N1-T5 bundle encode helpers.
 *
 * This header is *internal* (installed under src/, NOT under include/) and
 * exports no EXPORT_API. The facilities here cover three needs:
 *
 *   1. noti_bundle_encode_safe() — small RAII-like wrapper that owns the
 *      freshly allocated raw buffer + length pair produced by bundle_encode.
 *      Paired with noti_bundle_encoded_clear() which is safe to call on
 *      zero-initialized or partially populated structs.
 *
 *   2. noti_bundle_hash() — FNV-1a over the encoded bytes, used as a first-
 *      level filter for the cache (collisions are resolved by a bytewise
 *      memcmp on the cached entry).
 *
 *   3. noti_bundle_cached_encode() — LRU(16) cache of recent encodings. The
 *      cache is a file-static GHashTable + GList + GMutex. Hit returns a
 *      *duplicate* of the cached buffer so the caller can bundle_free it
 *      identically to a fresh bundle_encode() result — ABI and ownership
 *      stay identical to the legacy call sites.
 */

#ifndef SRC_NOTIFICATION_BUNDLE_UTIL_H_
#define SRC_NOTIFICATION_BUNDLE_UTIL_H_

#include <stdint.h>
#include <bundle.h>

#ifndef NOTI_BUNDLE_UTIL_API
#define NOTI_BUNDLE_UTIL_API __attribute__((visibility("default")))
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Owned buffer produced by noti_bundle_encode_safe. `raw` is a bundle_raw*
 * allocated by bundle_encode and must be freed with
 * bundle_free_encoded_rawdata. `len` carries the length.
 */
typedef struct {
	char *raw;
	int len;
} noti_bundle_encoded_t;

/*
 * Encode `b` into `out`. `out` MUST be zero-initialised by the caller (or
 * previously cleared via noti_bundle_encoded_clear). On failure out stays
 * {NULL, 0}. Returns 0 on success, -1 on encode failure, -2 on invalid
 * parameter. Safe to call with NULL bundle (no-op returning 0 with empty
 * out).
 */
NOTI_BUNDLE_UTIL_API int noti_bundle_encode_safe(bundle *b,
		noti_bundle_encoded_t *out);

/*
 * Free the raw buffer if any and reset the struct to {NULL, 0}. Safe to
 * call on zero-initialised or already-cleared structs.
 */
NOTI_BUNDLE_UTIL_API void noti_bundle_encoded_clear(
		noti_bundle_encoded_t *out);

/*
 * 64-bit FNV-1a hash of an encoded bundle.  We pay the full encode once to
 * compute this so that callers combining hash + encode get amortised cost;
 * callers that only want the hash should prefer the cached_encode entry
 * point which does both in one pass.
 *
 * Returns 0 on failure (bundle NULL / encode failed).
 */
NOTI_BUNDLE_UTIL_API uint64_t noti_bundle_hash(bundle *b);

/*
 * Cached encode. On hit, returns a freshly-duplicated buffer with ownership
 * matching bundle_encode (caller frees with bundle_free_encoded_rawdata).
 * On miss, caches the freshly-encoded buffer. Cache capacity 16 entries,
 * LRU eviction.
 *
 * Returns NULL on failure. *len_out receives the length on success.
 * Thread-safe.
 */
NOTI_BUNDLE_UTIL_API char *noti_bundle_cached_encode(bundle *b, int *len_out);

/*
 * Drop all cache entries. Safe to call any time; primarily for tests.
 */
NOTI_BUNDLE_UTIL_API void noti_bundle_cache_clear(void);

/*
 * Testing hooks — not part of the production API.
 */
NOTI_BUNDLE_UTIL_API size_t noti_bundle_cache_size(void);
NOTI_BUNDLE_UTIL_API size_t noti_bundle_cache_capacity(void);

#ifdef __cplusplus
}
#endif

#endif  /* SRC_NOTIFICATION_BUNDLE_UTIL_H_ */
