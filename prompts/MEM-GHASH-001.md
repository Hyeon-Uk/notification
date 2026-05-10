# MEM-GHASH-001 — GHashTable → std::unordered_map Migration

## Context

GHashTable is used in several files for key→value lookups. It requires
manual g_hash_table_destroy, is not type-safe, and hides hash/compare
function choices. This task replaces all GHashTable usage with
std::unordered_map (O(1) average lookup, RAII cleanup, type safety).

**Prerequisite: MEM-GLIST-001 must be complete.**

## Acceptance Criteria

- [ ] Zero `GHashTable*` declarations in all affected files after migration
- [ ] Zero `g_hash_table_*` function calls in all affected files
- [ ] `gbs build -A x86_64 --include-all` PASS
- [ ] `ctest --output-on-failure` regression = 0 vs MEM-GLIST-001 end baseline
- [ ] `nm --dynamic --defined-only libnotification.so | wc -l` = 316
- [ ] `valgrind --leak-check=full` definitely-lost = 0

## GBS Build Tag

`GBS-ROOT-MEM-GHASH`

## Files Affected

Audit with:
```bash
grep -rn 'GHashTable' src/notification/src/*.cc
```

Primary files (from source analysis):
- `src/notification/src/notification_internal.cc` (18 hits)
- `src/notification/src/notification_bundle_util.cc` (14 hits)
- `src/notification/src/notification_noti.cc` (varies)
- `src/notification/src/notification_setting.cc` (varies)

## TDD: Test Scenarios (Red Phase)

Write in `tests/unittests/src/test_ghash_migration.cc` BEFORE implementing:

```cpp
TEST_F(GHashMigrationTest, GHashTable_Replaced_NoGHashTable)
TEST_F(GHashMigrationTest, UnorderedMap_Lookup_CorrectValue)
TEST_F(GHashMigrationTest, UnorderedMap_MissingKey_ReturnsFalse)
TEST_F(GHashMigrationTest, NoGHashTableDestroy_Required)
TEST_F(GHashMigrationTest, Valgrind_GHash_Zero_Leaks)
```

## Implementation Specification

### Key type mapping

| Old GHashTable key type | New std::unordered_map key type |
|------------------------|---------------------------------|
| `gchar*` (string key) | `std::string` |
| `gint` / `gpointer` cast to int | `int` or `uint32_t` |
| `guint` | `unsigned int` |

### Replacement patterns

`GHashTable* ht = g_hash_table_new(g_str_hash, g_str_equal)` →
`std::unordered_map<std::string, ValueType> ht;`

`g_hash_table_insert(ht, key, value)` →
`ht[key] = value;`

`gpointer val = g_hash_table_lookup(ht, key)` →
`auto it = ht.find(key); if (it != ht.end()) { use(it->second); }`

`gboolean found = g_hash_table_contains(ht, key)` →
`bool found = ht.count(key) > 0;`

`g_hash_table_remove(ht, key)` →
`ht.erase(key);`

`g_hash_table_destroy(ht)` → Remove (unordered_map destructor handles cleanup)

`g_hash_table_foreach(ht, fn, user_data)` →
`for (auto& [k, v] : ht) { fn(k.data(), v, user_data); }`

`g_hash_table_size(ht)` → `ht.size()`

### Value ownership

If GHashTable used `g_hash_table_new_full` with a `GDestroyNotify` value
destructor, store values as `std::unique_ptr<T>` or `std::shared_ptr<T>`
in the unordered_map so destruction is automatic.

Example:
```cpp
// Before: GHashTable with destroy notify for values
GHashTable* ht = g_hash_table_new_full(
    g_str_hash, g_str_equal, g_free, my_value_free);

// After: unordered_map with RAII value type
std::unordered_map<std::string, std::unique_ptr<MyValue>> ht;
```

### ABI safety check

Before migration, verify GHashTable does not appear in public API headers:
```bash
grep -r 'GHashTable' src/notification/include/
```
If present in a public header, leave that symbol unchanged and do NOT migrate it.

## Code Style Checklist

- [ ] K&R braces on same line
- [ ] 2-space indent (no tabs)
- [ ] Omit braces for single-line bodies
- [ ] Max 80 chars per line
- [ ] Use structured bindings (auto& [k, v]) for map iteration
- [ ] Prefer .find() over .count() when the value is needed
- [ ] No GHashTable / g_hash_table_* after migration

## Commit Strategy

One commit per file. Per commit:
`gbs build PASS` + `ctest regression = 0` + `ABI nm diff = 0`

## ABI Verification

```bash
nm --dynamic --defined-only \
  /home/kimhyeonuk/GBS-ROOT-MEM-GHASH/local/BUILD/x86_64/RPMS/libnotification*.so \
  | awk '{print $3}' | sort > after.txt
diff baseline_316.txt after.txt
# Must be empty
```

## Next Task

After this task → **SIZE-DEAD-001**
