# notification — Phase B: C→C++ Refactoring (AFTER test coverage confirmed)

## CRITICAL: Branch Rule
ALL work on the **`default`** branch only. All pushes: `git push origin default`

## GBS Build — NO SUDO
```bash
gbs build -A x86_64 --include-all -B /home/kimhyeonuk/GBS-ROOT-REFACTOR
```

## Excluded
`src/notification-ex/` — do NOT touch

## Code Style (C++)
Google C++ Style Guide, K&R braces, 2-space indent, omit single-line braces, max 80 chars/line

## ABI Rule
All `EXPORT_API` function signatures must remain byte-identical. Do NOT rename or remove.

---

## Pre-condition (already verified by orchestrator)
- Unit tests: 157+ PASS, 0 FAIL
- Integration tests: 0 FAIL  
- Smoke tests: 6 PASS
- STATE.md: `PHASE: test-suite-complete, refactoring: NOT_STARTED`

---

## B-SETUP-001 — CMakeLists.txt C++17 configuration

Edit `src/notification/CMakeLists.txt`:
- Add `-std=c++17` to `target_compile_options` or `add_compile_options`
- Add `set_target_properties(notification PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON)`
- Note: `AUX_SOURCE_DIRECTORY` auto-picks `.cc` files — rename files, not CMakeLists source lists

GBS build: exit 0.
Commit: `build(notification): configure C++17 compilation for migration`
Push: `git push origin default`

---

## B-MIG-001..016 — .c → .cc (four batches)

### Batch 1 (MIG-001..004)
Rename + fix:
1. `notification_error.c` → `.cc`
2. `notification_list.c` → `.cc`
3. `notification_ipc.c` → `.cc`
4. `notification_db.c` → `.cc`

C++ fixes per file:
- Designated initializers `{ .field = val }` → named temporaries or constructor init
- VLAs → `std::vector<char>` or fixed-size arrays
- Compound literals `(T){...}` → named variables
- `_Bool` → `bool`
- `restrict` → remove
- `void*` implicit casts → `static_cast<T*>`
- C99 interleaved declarations → hoist to top of block
- In `.cc` files including C headers: wrap with `extern "C" { }`

After batch: GBS build exit 0 + all 3 suites 0 FAIL.
Commit: `refactor(notification): migrate error/list/ipc/db to C++ (B-MIG-001..004)`
Push: `git push origin default`

### Batch 2 (MIG-005..008)
5. `notification_noti.c` → `.cc`
6. `notification_shared_file.c` → `.cc`
7. `notification_status.c` → `.cc`
8. `notification_group.c` → `.cc`

After batch: GBS build + all 3 suites.
Commit: `refactor(notification): migrate noti/shared_file/status/group to C++ (B-MIG-005..008)`
Push: `git push origin default`

### Batch 3 (MIG-009..012)
9. `notification_viewer.c` → `.cc`
10. `notification_setting.c` → `.cc`
11. `notification_setting_service.c` → `.cc`
12. `notification_internal_tidl.c` → `.cc`

After batch: GBS build + all 3 suites.
Commit: `refactor(notification): migrate viewer/setting/setting_service/internal_tidl to C++ (B-MIG-009..012)`
Push: `git push origin default`

### Batch 4 (MIG-013..016)
13. `notification_tidl.c` → `.cc`
14. `notification_ongoing.c` → `.cc`
15. `notification_internal.c` → `.cc`
16. `notification.c` → `.cc`

After batch: GBS build + all 3 suites.
Commit: `refactor(notification): migrate tidl/ongoing/internal/notification to C++ (B-MIG-013..016)`
Push: `git push origin default`

---

## B-RAII-001 — RAII wrappers

- `sqlite3*` → RAII wrapper (open in constructor, close in destructor)
- `FILE*` / fd → RAII closer
- GLib heap → `std::unique_ptr<T, decltype(&g_free)>`
- DB transactions → RAII guard (BEGIN on construct, COMMIT/ROLLBACK on destruct)

After: GBS build + all 3 suites 0 FAIL.
Commit: `refactor(notification): add RAII wrappers (B-RAII-001)`
Push: `git push origin default`

---

## B-MEM-001 — STL migration (internal-only)

Replace ONLY where NOT in public structs or exported symbol signatures:
- `g_malloc`/`g_free` → `std::vector<uint8_t>` for buffers
- `g_strdup`/`g_free` char* → `std::string` for internal strings
- `GList` → `std::vector` or `std::list` for internal-only lists

After: GBS build + all 3 suites 0 FAIL.
Commit: `refactor(notification): STL memory migration internal-only (B-MEM-001)`
Push: `git push origin default`

---

## B-OPT-001 — Dead code removal

- Remove unused `static` functions identified by compiler warnings
- Remove dead paths in deprecated `notification_group.cc`
- Run `gbs build` with `-Wunused-function` to find candidates

After: GBS build + all 3 suites 0 FAIL.
Commit: `refactor(notification): remove dead code (B-OPT-001)`
Push: `git push origin default`

---

## B-COV-001 — Final Regression Gate

```bash
gbs build -A x86_64 --include-all -B /home/kimhyeonuk/GBS-ROOT-REFACTOR
ctest -R notification --output-on-failure -v 2>&1 | tee /tmp/noti-final-regression.txt
```

Required:
- Unit: >= 157 PASS, 0 FAIL
- Integration: 0 FAIL (SKIP allowed)
- Smoke: 6 PASS

ABI check:
```bash
nm -D /path/to/libnotification.so | grep EXPORT_API > /tmp/abi-after.txt
# compare with pre-refactoring symbol list
```

Update STATE.md:
```
PHASE: refactoring-complete
unit_tests: COMPLETE
integration_tests: COMPLETE
smoke_tests: COMPLETE
refactoring: COMPLETE
```

```bash
git -C ../refactoring-plans add execution/tasks/notification/STATE.md
git -C ../refactoring-plans commit -m "chore: notification C++17 refactoring complete"
git -C ../refactoring-plans push origin default
```

Final commit: `refactor(notification): C++17 migration complete — COV-001 PASS`
Push: `git push origin default`

Update WORKFLOWS.md phases 10-12 → [O] with commit hashes.

## Execution Rules
1. Auto-advance on task completion — no human input
2. After EACH batch: GBS build + all 3 suites → 0 FAIL before continuing
3. GBS: no sudo, run directly
4. Valid stops: GBS FAIL after 3 retries, or unresolvable ABI break
5. No `.c` files may remain in `src/notification/src/` at end
