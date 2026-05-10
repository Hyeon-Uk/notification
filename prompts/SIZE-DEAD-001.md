# SIZE-DEAD-001 — Dead Code Elimination + Symbol Visibility

## Context

All RAII and memory optimization tasks (RAII-DB-001, RAII-FD-001,
MEM-GLIB-001, MEM-GLIST-001, MEM-GHASH-001) are complete. This final
task reduces binary size by:
1. Applying `-fvisibility=hidden` to internal translation units
2. Removing dead/unreachable code
3. Verifying string literal deduplication
4. Confirming no unused exported symbols

**Prerequisite: All MEM-* tasks must be complete.**

ABI: 316 dynamic public symbols (must remain stable).

## Code Style (MANDATORY)

- Google C++ Style Guide
- K&R braces (opening brace on same line)
- 2-space indent (no tabs)
- Omit braces for single-line bodies
- Max 80 characters per line

## Files to Modify

1. `src/CMakeLists.txt` or `src/notification/CMakeLists.txt`
   - Add `-fvisibility=hidden -fvisibility-inlines-hidden` to internal TUs
2. All 19 `.cc` files — audit for dead static functions
3. All headers in `src/notification/include/` — verify EXPORT_API on public API

## Test First (TDD)

Write tests in: `tests/unittests/src/test_symbol_visibility.cc`

### Required Test Cases

| Test Name | Scenario | Expected |
|-----------|----------|----------|
| Visibility_Hidden_InternalFuncs | nm --dynamic after -fvisibility=hidden | Only EXPORT_API symbols visible; 316 total |
| DeadCode_StaticFn_Eliminated | Build with -Wunused-function | Zero warnings |
| BinarySize_Regression_Below5Pct | Compare size before/after | Size delta <= 5% (target: reduction) |
| ABI_Baseline_Stable | nm --dynamic --defined-only | Exactly 316 dynamic public symbols |
| StrDedup_NoNewSections | objdump --section-headers | No new .rodata fragmentation |
| Regression_Size_NoCtestFailure | ctest after changes | 0 new failures |

### TDD Procedure

```
1. Capture pre-task baseline:
   - Binary size: `stat -c%s libnotification.so`
   - Symbol count: `nm --dynamic --defined-only libnotification.so | wc -l`
   - Unused warnings: `gbs build ... 2>&1 | grep unused-function`
2. Write test_symbol_visibility.cc verifying 316 symbol count (link-time test)
3. gbs build -> test PASS with current binary
4. Add -fvisibility=hidden to CMakeLists.txt for notification target
5. gbs build -> if any EXPORT_API functions become hidden, fix by adding
   __attribute__((visibility("default"))) or EXPORT_API macro
6. ctest regression = 0
7. Identify dead static functions via -Wunused-function
8. Remove dead code
9. gbs build + ctest regression = 0
10. ABI nm diff = 0 (exactly 316 symbols)
11. Binary size <= baseline
```

## Implementation Steps

### Step 1: Audit Symbol Table

```bash
# In GBS chroot after build:
nm --dynamic --defined-only libnotification.so | awk '{print $3}' | sort > symbols_before.txt
wc -l symbols_before.txt  # Should be 316
```

### Step 2: Add Visibility Hidden

In `src/notification/CMakeLists.txt` (or root `src/CMakeLists.txt`):
```cmake
TARGET_COMPILE_OPTIONS(notification PRIVATE
  -fvisibility=hidden
  -fvisibility-inlines-hidden
)
```

Ensure all public API functions have `EXPORT_API` macro:
```cpp
// notification.h (installed public header)
#ifndef EXPORT_API
#define EXPORT_API __attribute__((visibility("default")))
#endif

EXPORT_API int notification_create(...);
```

### Step 3: Dead Code Analysis

```bash
# Build with warnings
gbs build ... 2>&1 | grep -E "unused-function|defined but not used"
```

For each flagged function:
1. Verify not called anywhere: `grep -rn 'function_name' src/`
2. If truly dead, remove entirely
3. If called only from tests, keep but mark with appropriate visibility

### Step 4: String Deduplication Check

```bash
# Check .rodata for duplicate string literals
objdump -s -j .rodata libnotification.so | head -200
# Look for repeated patterns; compiler should handle with -fmerge-constants
```

### Step 5: Verify Binary Size Reduction

```bash
stat -c%s libnotification.so  # After all changes
# Compare with 467,456 B (N8-B.2 baseline)
# Target: reduction (dead code removed + visibility hidden reduces PLT/GOT)
```

## Important Considerations

1. **Do NOT remove symbols from `nm --dynamic`**: The 316 public symbols
   are the ABI contract. Only internal (file-static, anonymous namespace)
   functions may be removed or hidden.

2. **-fvisibility=hidden is applied to the library target only**: Test
   executables must still be able to link against EXPORT_API symbols.

3. **Phase 2 already applied `-fvisibility=hidden`**: Verify this isn't
   double-applied. If already set in CMakeLists.txt, this task focuses
   on dead code elimination and verification only.

4. **notification-ex is separate**: This task covers `libnotification.so`
   only. `libnotification-ex.so` has its own visibility settings.

## Commit Strategy

1. `SIZE-DEAD-001a`: Add/verify `-fvisibility=hidden` in CMakeLists.txt
   + fix any newly-hidden EXPORT_API functions
2. `SIZE-DEAD-001b`: Remove dead static functions (per -Wunused-function)
3. `SIZE-DEAD-001c`: Final binary size verification + baseline update

## Acceptance Criteria

- [ ] `-fvisibility=hidden` applied (or confirmed already applied)
- [ ] `nm --dynamic --defined-only libnotification.so` shows exactly 316 symbols
- [ ] Zero `-Wunused-function` warnings in build
- [ ] Binary size <= 467,456 B (N8-B.2 baseline) or justified increase
- [ ] `gbs build -A x86_64 --include-all -B /home/kimhyeonuk/GBS-ROOT-SIZE-DEAD` PASS
- [ ] ctest regression = 0
- [ ] No new `.rodata` section fragmentation

## GBS Build Tag

`GBS-ROOT-SIZE-DEAD`
