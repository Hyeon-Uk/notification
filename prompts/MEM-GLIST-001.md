# MEM-GLIST-001 — GList → std::vector / std::list Migration

## Context

GList is used in 10 files across the notification package. It requires manual
g_list_free / g_list_free_full calls, is not type-safe, and lacks the
iterator-invalidation guarantees of STL containers. This task replaces all
GList usage with std::vector (for ordered, index-accessed lists) or std::list
(for lists requiring O(1) splice/insert in the middle).

**Prerequisite: MEM-GLIB-001 must be complete.**

## Acceptance Criteria

- [ ] Zero `GList*` declarations in all 10 files after migration
- [ ] Zero `g_list_*` function calls in all 10 files
- [ ] `gbs build -A x86_64 --include-all` PASS
- [ ] `ctest --output-on-failure` regression = 0 vs MEM-GLIB-001 end baseline
- [ ] `nm --dynamic --defined-only libnotification.so | wc -l` = 316
- [ ] `valgrind --leak-check=full` definitely-lost = 0 (no g_list_free leaks)

## GBS Build Tag

`GBS-ROOT-MEM-GLIST`

## Files Affected

| File | GList hits | Primary list use |
|------|-----------|-----------------|
| `src/notification/src/notification_shared_file.cc` | 29 | shared file list |
| `src/notification/src/notification_internal.cc` | 25 | internal noti list |
| `src/notification/src/notification_noti.cc` | varies | query result list |
| `src/notification/src/notification_setting.cc` | varies | setting list |
| `src/notification/src/notification_bundle_util.cc` | 9 | key list |
| `src/notification/src/notification_viewer.cc` | 1 | viewer list |
| `src/notification/src/notification_ipc.cc` | 4 | ipc list |
| `src/notification/src/notification_context.cc` | 3 | context list |
| `src/notification/src/notification_internal_tidl.cc` | 2 | tidl list |
| `src/notification/src/shared_file_common.cc` | 1 | common file list |

## TDD: Test Scenarios (Red Phase)

Write in `tests/unittests/src/test_glist_migration.cc` BEFORE implementing:

```cpp
TEST_F(GListMigrationTest, GList_Replaced_NoGList)
TEST_F(GListMigrationTest, Vector_IterationOrder_Preserved)
TEST_F(GListMigrationTest, GList_EmptyList_NoUBSan)
TEST_F(GListMigrationTest, NoGListFree_Required)
TEST_F(GListMigrationTest, Valgrind_STL_Zero_Leaks)
```

For IterationOrder tests:
- Build a known-order sequence using old API (test fixture captures expected order)
- Migrate; verify same order with std::vector iteration

## Implementation Specification

### Container selection

Use `std::vector<T>` for:
- Lists that are built once then iterated (most cases)
- Lists accessed by index
- Lists that are fully rebuilt per operation

Use `std::list<T>` only for:
- Lists where mid-list insertion/removal is the primary operation
- Lists where iterator stability across modification is required

### Replacement patterns

`GList* list = nullptr; list = g_list_append(list, data)` →
`std::vector<T> list; list.push_back(data);`

`GList* node = list; while (node) { use(node->data); node = node->next; }` →
`for (auto& item : list) { use(item); }`

`g_list_free(list)` → remove (vector destructor handles cleanup)

`g_list_free_full(list, (GDestroyNotify)my_free)` → RAII items in vector:
elements with destructors clean themselves up

`g_list_length(list)` → `list.size()`

`g_list_nth_data(list, n)` → `list[n]` or `list.at(n)`

`g_list_find_custom(list, key, cmp_fn)` →
`std::find_if(list.begin(), list.end(), [&](const T& e) { return cmp_fn(&e, key) == 0; })`

`g_list_remove(list, data)` →
`list.erase(std::remove(list.begin(), list.end(), data), list.end())`

### ABI safety check

Before migration, verify no GList appears in public API headers:
```bash
grep -r 'GList' src/notification/include/
```
If any public header uses GList, add a compatibility typedef or leave that
specific symbol unchanged (do NOT break the ABI).

### TIDL boundary safety

`notification_internal_tidl.cc` has 2 GList hits. After migration,
ensure all extern "C" functions remain wrapped in try/catch(...).

## Code Style Checklist

- [ ] K&R braces on same line
- [ ] 2-space indent (no tabs)
- [ ] Omit braces for single-line bodies
- [ ] Max 80 chars per line
- [ ] Use range-for instead of index loops where possible
- [ ] Prefer std::vector<T> over raw pointer arrays
- [ ] No GList / g_list_* after migration

## Commit Strategy

10 commits (one per file):
1. `notification_shared_file.cc` (29 hits — largest)
2. `notification_internal.cc` (25 hits)
3. `notification_noti.cc`
4. `notification_setting.cc`
5. `notification_bundle_util.cc`
6. `notification_viewer.cc`
7. `notification_ipc.cc`
8. `notification_context.cc`
9. `notification_internal_tidl.cc`
10. `shared_file_common.cc`

Per commit: `gbs build PASS` + `ctest regression = 0` + `ABI nm diff = 0`

## ABI Verification

```bash
nm --dynamic --defined-only \
  /home/kimhyeonuk/GBS-ROOT-MEM-GLIST/local/BUILD/x86_64/RPMS/libnotification*.so \
  | awk '{print $3}' | sort > after.txt
diff baseline_316.txt after.txt
# Must be empty
```

## Next Task

After this task → **MEM-GHASH-001**
