# MEM-GLIB-001 — GLib Allocator → std::string / std::vector

## Context

After RAII-FD-001, raw GLib allocator calls remain in 5 files:
g_malloc, g_free, g_strdup, g_new. These bypass C++ RAII and are
incompatible with std::string/std::vector semantics. This task replaces
all 20 occurrences with STL equivalents.

**Prerequisite: RAII-FD-001 must be complete.**

## Acceptance Criteria

- [ ] Zero `g_malloc` / `g_free` / `g_strdup` / `g_new` in all 5 files after migration
- [ ] All string data flows through `std::string`; no raw `char*` owned by GLib
- [ ] `gbs build -A x86_64 --include-all` PASS
- [ ] `ctest --output-on-failure` regression = 0 vs RAII-FD-001 end baseline
- [ ] `nm --dynamic --defined-only libnotification.so | wc -l` = 316
- [ ] `valgrind --leak-check=full` definitely-lost = 0

## GBS Build Tag

`GBS-ROOT-MEM-GLIB`

## Files Affected

| File | g_malloc/g_strdup hits | Primary patterns |
|------|----------------------|-----------------|
| `src/notification/src/notification_bundle_util.cc` | 9 | hash buffer g_malloc |
| `src/notification/src/notification_noti.cc` | 7 | temp string g_strdup |
| `src/notification/src/notification_setting.cc` | 2 | setting value g_strdup |
| `src/notification/src/notification_ipc.cc` | 1 | ipc payload g_malloc |
| `src/notification/src/notification_tidl.cc` | 1 | tidl string g_strdup |

## TDD: Test Scenarios (Red Phase)

Write in `tests/unittests/src/test_mem_glib_migration.cc` BEFORE implementing:

```cpp
TEST_F(MemGlibTest, GlibStr_NullInput_EmptyString)
TEST_F(MemGlibTest, GlibStr_Replaced_NoGstrdup)
TEST_F(MemGlibTest, GlibAlloc_Replaced_NoGmalloc)
TEST_F(MemGlibTest, VectorRoundtrip_PreservesData)
TEST_F(MemGlibTest, Valgrind_GlibBlocks_Zero)
```

For static analysis tests:
- `Replaced_NoGstrdup`: use `grep` fixture or compile-time check (static_assert)
- `VectorRoundtrip_PreservesData`: verify notification_bundle_util hash output
  matches before and after the migration

## Implementation Specification

### Replacement rules

| Old pattern | New pattern |
|------------|-------------|
| `char* s = g_strdup(src)` | `std::string s(src ? src : "")` |
| `char* s = g_strdup_printf(fmt, ...)` | Use `std::string` + snprintf into local buffer or absl-style |
| `void* p = g_malloc(n)` | `std::vector<uint8_t> p(n)` or `std::string p(n, '\0')` |
| `g_free(s)` | Remove (automatic via std::string/vector destructor) |
| `char** arr; arr[i] = g_strdup(x)` | `std::vector<std::string> arr; arr.push_back(x)` |

### Null safety

Old code often g_strdup(possibly_null). Replacement must preserve null safety:
```cpp
// BEFORE
char* copy = g_strdup(src);  // g_strdup(null) returns null

// AFTER — preserve semantics with optional
std::string copy = src ? src : "";
```

For output parameters that callers expect to be null-able, use:
```cpp
std::optional<std::string> copy = src ? std::optional(src) : std::nullopt;
```

### notification_bundle_util.cc — hash buffer

Before:
```cpp
char* buf = (char*)g_malloc(hash_len);
// fill buf ...
g_free(buf);
```

After:
```cpp
std::vector<char> buf(hash_len);
// fill buf.data() ...
// freed automatically by vector destructor
```

### TIDL boundary safety

`notification_tidl.cc` uses g_strdup in one location. After replacement,
ensure the surrounding extern "C" function still wraps in try/catch(...).

## Code Style Checklist

- [ ] K&R braces on same line
- [ ] 2-space indent (no tabs)
- [ ] Omit braces for single-line bodies
- [ ] Max 80 chars per line
- [ ] Prefer std::string over char* for string ownership
- [ ] Prefer std::vector<T> over T* for buffer ownership
- [ ] No g_malloc / g_free / g_strdup / g_new after migration

## Commit Strategy

5 commits (one per file):
1. `notification_bundle_util.cc`
2. `notification_noti.cc`
3. `notification_setting.cc`
4. `notification_ipc.cc`
5. `notification_tidl.cc`

Per commit: `gbs build PASS` + `ctest regression = 0` + `ABI nm diff = 0`
+ `valgrind 0 definitely-lost`

## ABI Verification

```bash
nm --dynamic --defined-only \
  /home/kimhyeonuk/GBS-ROOT-MEM-GLIB/local/BUILD/x86_64/RPMS/libnotification*.so \
  | awk '{print $3}' | sort > after.txt
diff baseline_316.txt after.txt
# Must be empty
```

## Next Task

After this task → **MEM-GLIST-001**
