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

#define _GNU_SOURCE
#include "notification_bundle_util.h"

#include <cstdlib>
#include <cstring>
#include <cstdint>

#include <glib.h>
#include <bundle.h>
#include <bundle_internal.h>

/*
 * N8-A.2 (Stage 8): migrated from C to C++.  Public helpers
 * (noti_bundle_encode_safe / noti_bundle_encoded_clear / noti_bundle_hash /
 * noti_bundle_cached_encode / noti_bundle_cache_clear / _size / _capacity)
 * keep C linkage via the extern "C" guard in notification_bundle_util.h;
 * the entire body below sits inside a single extern "C" block so the
 * linker sees matching mangling.  File-static helpers (cache_*_locked,
 * hash_bytes, cache_lazy_init, cache_init_idempotent) live inside the
 * same block — `static` retains internal linkage regardless, and the
 * extern "C" qualifier on a static function only changes language-linkage
 * for the (unused) cross-TU case.
 *
 * <stdlib.h>/<string.h>/<stdint.h> swap to <cstdlib>/<cstring>/<cstdint>.
 * Implicit `void *` returns from g_hash_table_lookup / g_queue_pop_* /
 * g_new* receive explicit `static_cast<T *>` casts that C++ requires
 * (the original C source already used C-style casts so the diff is
 * minimal).
 */

extern "C" {

int noti_bundle_encode_safe(bundle *b, noti_bundle_encoded_t *out)
{
	bundle_raw *raw = NULL;
	int len = 0;
	int ret;

	if (out == NULL)
		return -2;

	/* Empty/NULL bundle is a valid no-op — leave out as {NULL, 0}. */
	if (b == NULL) {
		out->raw = NULL;
		out->len = 0;
		return 0;
	}

	ret = bundle_encode(b, &raw, &len);
	if (ret != BUNDLE_ERROR_NONE || raw == NULL) {
		out->raw = NULL;
		out->len = 0;
		return -1;
	}

	out->raw = (char *)raw;
	out->len = len;
	return 0;
}

void noti_bundle_encoded_clear(noti_bundle_encoded_t *out)
{
	if (out == NULL)
		return;

	if (out->raw != NULL)
		bundle_free_encoded_rawdata((bundle_raw **)&out->raw);

	out->raw = NULL;
	out->len = 0;
}

/*
 * -----------------------------------------------------------------------
 * FNV-1a 64-bit hash over a byte buffer.
 * -----------------------------------------------------------------------
 */
static uint64_t noti_bundle_hash_bytes(const void *data, size_t len)
{
	const uint8_t *p = (const uint8_t *)data;
	uint64_t h = UINT64_C(0xcbf29ce484222325);  /* FNV-1a offset basis */
	size_t i;

	for (i = 0; i < len; ++i) {
		h ^= (uint64_t)p[i];
		h *= UINT64_C(0x100000001b3);  /* FNV-1a prime */
	}
	/* Avoid returning 0 which we reserve for "failure". */
	return (h == 0) ? UINT64_C(0xdeadbeef) : h;
}

uint64_t noti_bundle_hash(bundle *b)
{
	bundle_raw *raw = NULL;
	int len = 0;
	uint64_t h;

	if (b == NULL)
		return 0;
	if (bundle_encode(b, &raw, &len) != BUNDLE_ERROR_NONE || raw == NULL)
		return 0;

	h = noti_bundle_hash_bytes(raw, (size_t)len);
	bundle_free_encoded_rawdata(&raw);
	return h;
}

/*
 * -----------------------------------------------------------------------
 * LRU(16) bundle-encode cache.
 *
 * Data structure:
 *   GHashTable<uint64_t -> cache_entry_t*>  — O(1) lookup by hash.
 *   GQueue<cache_entry_t*>                  — usage order (head = MRU,
 *                                             tail = LRU). Each entry
 *                                             stores its own GList link so
 *                                             we can promote without scan.
 *   GMutex                                  — protects both.
 *
 * Hash collisions: resolved by walking the doubly-linked list of entries
 * sharing that hash (stored inline via the `next_collision` field).
 * Bytewise memcmp validates a candidate before we declare it a hit.
 *
 * Ownership: returned buffers are *copies* — the cache retains its own
 * copy. Callers free with bundle_free_encoded_rawdata just as they do for
 * fresh bundle_encode() results.
 * -----------------------------------------------------------------------
 */

#define NOTI_BUNDLE_CACHE_CAPACITY 16

typedef struct cache_entry_s {
	uint64_t hash;
	char *raw;                        /* bundle_encode result; owned */
	int len;
	GList *link;                      /* node in usage_list */
	struct cache_entry_s *next_collision;  /* chain for same-hash entries */
} cache_entry_t;

static GHashTable *g_cache_table;     /* uint64_t (boxed) -> cache_entry_t* head */
static GQueue *g_usage_list;          /* MRU at head, LRU at tail */
static GMutex g_cache_mutex;
static gsize g_cache_init_once = 0;

static gboolean cache_init_idempotent(gpointer _unused)
{
	(void)_unused;
	g_mutex_init(&g_cache_mutex);
	/* g_int64_hash/g_int64_equal expect gint64* keys. We install a
	 * g_free destroy for keys so eviction + clear paths do not leak
	 * the boxed hash. Values are owned manually. */
	g_cache_table = g_hash_table_new_full(g_int64_hash, g_int64_equal,
			g_free, NULL);
	g_usage_list = g_queue_new();
	return TRUE;
}

static void cache_lazy_init(void)
{
	if (g_once_init_enter(&g_cache_init_once)) {
		cache_init_idempotent(NULL);
		g_once_init_leave(&g_cache_init_once, 1);
	}
}

/*
 * Must be called with g_cache_mutex held. Removes `victim` from the
 * collision chain anchored at `victim->hash` in g_cache_table. When the
 * chain becomes empty the hash-table entry is removed (and the key freed
 * via the installed g_free destroy).
 */
static void cache_unlink_from_chain_locked(cache_entry_t *victim)
{
	gint64 key = (gint64)victim->hash;
	cache_entry_t *head;
	cache_entry_t *prev;

	head = (cache_entry_t *)g_hash_table_lookup(g_cache_table, &key);
	if (head == NULL)
		return;

	if (head == victim) {
		if (victim->next_collision == NULL) {
			g_hash_table_remove(g_cache_table, &key);
		} else {
			/* Promote the next collider to the chain head. We
			 * use g_hash_table_replace so the existing key
			 * allocation (owned by the table) is freed and
			 * replaced by a fresh copy tied to the new head. */
			gint64 *key_copy = g_new(gint64, 1);
			*key_copy = (gint64)victim->next_collision->hash;
			g_hash_table_replace(g_cache_table, key_copy,
					victim->next_collision);
		}
		return;
	}

	prev = head;
	while (prev->next_collision != NULL &&
			prev->next_collision != victim)
		prev = prev->next_collision;
	if (prev->next_collision == victim)
		prev->next_collision = victim->next_collision;
}

/* Must be called with g_cache_mutex held. */
static void cache_evict_tail_locked(void)
{
	cache_entry_t *victim;

	if (g_queue_is_empty(g_usage_list))
		return;

	victim = (cache_entry_t *)g_queue_pop_tail(g_usage_list);
	if (victim == NULL)
		return;

	victim->link = NULL;
	cache_unlink_from_chain_locked(victim);

	if (victim->raw != NULL)
		bundle_free_encoded_rawdata((bundle_raw **)&victim->raw);
	g_free(victim);
}

/*
 * Caller holds the mutex. Inserts a *copy* of `raw` keyed by `hash`. Len
 * must be > 0. On OOM this is a no-op (cache stays coherent).
 */
static void cache_insert_locked(uint64_t hash, const char *raw, int len)
{
	cache_entry_t *entry;
	cache_entry_t *head;
	gint64 key = (gint64)hash;
	bundle_raw *dup;

	if (raw == NULL || len <= 0)
		return;

	while (g_queue_get_length(g_usage_list) >= NOTI_BUNDLE_CACHE_CAPACITY)
		cache_evict_tail_locked();

	/* bundle_free_encoded_rawdata calls free() on the pointer — plain
	 * malloc is ABI-compatible. */
	dup = (bundle_raw *)malloc((size_t)len);
	if (dup == NULL)
		return;
	memcpy(dup, raw, (size_t)len);

	entry = g_new0(cache_entry_t, 1);
	entry->hash = hash;
	entry->raw = (char *)dup;
	entry->len = len;
	entry->next_collision = NULL;

	head = (cache_entry_t *)g_hash_table_lookup(g_cache_table, &key);
	if (head == NULL) {
		gint64 *key_copy = g_new(gint64, 1);
		*key_copy = key;
		g_hash_table_insert(g_cache_table, key_copy, entry);
	} else {
		/* Prepend to collision chain; the table value becomes the
		 * newly-inserted entry so future lookups hit the MRU
		 * collider first. g_hash_table_replace frees the old key
		 * (owned by the table) via the installed destroy func. */
		gint64 *key_copy = g_new(gint64, 1);
		*key_copy = key;
		entry->next_collision = head;
		g_hash_table_replace(g_cache_table, key_copy, entry);
	}

	g_queue_push_head(g_usage_list, entry);
	entry->link = g_queue_peek_head_link(g_usage_list);
}

/* Caller holds the mutex. Returns duplicate buffer on hit, NULL on miss. */
static char *cache_lookup_locked(uint64_t hash, const char *raw, int len,
		int *len_out)
{
	cache_entry_t *cur;
	gint64 key = (gint64)hash;
	bundle_raw *dup;

	cur = (cache_entry_t *)g_hash_table_lookup(g_cache_table, &key);
	while (cur != NULL) {
		if (cur->hash == hash && cur->len == len &&
				memcmp(cur->raw, raw, (size_t)len) == 0) {
			/* Hit — promote to MRU. */
			if (cur->link != NULL) {
				g_queue_unlink(g_usage_list, cur->link);
				g_queue_push_head_link(g_usage_list,
						cur->link);
			}

			dup = (bundle_raw *)malloc((size_t)cur->len);
			if (dup == NULL)
				return NULL;
			memcpy(dup, cur->raw, (size_t)cur->len);
			if (len_out != NULL)
				*len_out = cur->len;
			return (char *)dup;
		}
		cur = cur->next_collision;
	}
	return NULL;
}

char *noti_bundle_cached_encode(bundle *b, int *len_out)
{
	bundle_raw *raw = NULL;
	int len = 0;
	uint64_t hash;
	char *hit;

	if (len_out != NULL)
		*len_out = 0;

	if (b == NULL)
		return NULL;

	if (bundle_encode(b, &raw, &len) != BUNDLE_ERROR_NONE || raw == NULL)
		return NULL;

	hash = noti_bundle_hash_bytes(raw, (size_t)len);

	cache_lazy_init();

	g_mutex_lock(&g_cache_mutex);
	hit = cache_lookup_locked(hash, (const char *)raw, len, len_out);
	if (hit != NULL) {
		g_mutex_unlock(&g_cache_mutex);
		bundle_free_encoded_rawdata(&raw);
		return hit;
	}

	/* Miss: insert a copy, then return the freshly-encoded buffer we
	 * already own. */
	cache_insert_locked(hash, (const char *)raw, len);
	g_mutex_unlock(&g_cache_mutex);

	if (len_out != NULL)
		*len_out = len;
	return (char *)raw;
}

void noti_bundle_cache_clear(void)
{
	cache_entry_t *entry;

	cache_lazy_init();
	g_mutex_lock(&g_cache_mutex);

	while (!g_queue_is_empty(g_usage_list)) {
		entry = (cache_entry_t *)g_queue_pop_head(g_usage_list);
		if (entry == NULL)
			continue;
		if (entry->raw != NULL)
			bundle_free_encoded_rawdata(
					(bundle_raw **)&entry->raw);
		g_free(entry);
	}
	g_hash_table_remove_all(g_cache_table);

	g_mutex_unlock(&g_cache_mutex);
}

size_t noti_bundle_cache_size(void)
{
	size_t n;

	cache_lazy_init();
	g_mutex_lock(&g_cache_mutex);
	n = (size_t)g_queue_get_length(g_usage_list);
	g_mutex_unlock(&g_cache_mutex);
	return n;
}

size_t noti_bundle_cache_capacity(void)
{
	return NOTI_BUNDLE_CACHE_CAPACITY;
}

}  // extern "C"
