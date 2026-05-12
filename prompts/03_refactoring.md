# notification — Phase A (SKIP Fix) + Phase B (C→C++ Refactoring)

## CRITICAL: Branch Rule
ALL work on the **`default`** branch only. All pushes: `git push origin default`

## GBS Build — NO SUDO
```bash
gbs build -A x86_64 --include-all -B /home/kimhyeonuk/GBS-ROOT-REFACTOR
```

## Excluded
`src/notification-ex/` — do NOT touch, do NOT test

---

# PHASE A — Fix Integration Test SKIPs (do this FIRST)

## Background
33 integration tests currently SKIP. Root cause: `notification_create()` calls
`aul_app_get_appid_bypid()` internally. GBS chroot has no AUL daemon, so it returns
error, making `notification_create` return NULL. Same PLT interposition technique
already used in `test_db_override.cc` and `tidl_override.cc` can fix this.

## Task A-1: Create aul_override.cc

Create `tests/integration/src/aul_override.cc`:

```cpp
/*
 * PLT interposition for AUL functions.
 * Mirrors the technique in test_db_override.cc and tidl_override.cc.
 * Provides a fake app context so notification_create() succeeds in GBS chroot.
 */
#include <cstring>
#include <aul.h>

extern "C" int aul_app_get_appid_bypid(int pid, char* appid, int len) {
  static const char kFakeAppId[] = "org.tizen.test.notification.integration";
  strncpy(appid, kFakeAppId, len - 1);
  appid[len - 1] = '\0';
  return 0;  /* AUL_R_OK */
}

extern "C" int aul_app_get_pkgname_bypid(int pid, char* pkg, int len) {
  static const char kFakePkg[] = "org.tizen.test.notification.integration";
  strncpy(pkg, kFakePkg, len - 1);
  pkg[len - 1] = '\0';
  return 0;  /* AUL_R_OK */
}

extern "C" int aul_app_get_pkgid_bypid(int pid, char* pkgid, int len) {
  static const char kFakePkgId[] = "org.tizen.test.notification.integration";
  strncpy(pkgid, kFakePkgId, len - 1);
  pkgid[len - 1] = '\0';
  return 0;  /* AUL_R_OK */
}
```

## Task A-2: Register in CMakeLists.txt

In `tests/integration/CMakeLists.txt`, add `aul_override.cc` to the
`AUX_SOURCE_DIRECTORY` or explicit source list — same pattern as `tidl_override.cc`.

## Task A-3: Verify

```bash
gbs build -A x86_64 --include-all -B /home/kimhyeonuk/GBS-ROOT-REFACTOR
# inside chroot: ctest -R notification-integration --output-on-failure
```

Target: SKIP count drops from 33 to < 10 (GVariant/daemon tests may still skip).
Gate: 0 new FAILs.

Commit: `test(integration): add AUL PLT interposition to fix notification_create SKIPs`
Push: `git push origin default`

---

# PHASE B — C→C++ Refactoring

Only start after Phase A passes gate.

## Status
STATE.md confirms: `PHASE: test-suite-complete, refactoring: NOT_STARTED`
Phase 1 gate passed. Phase 2 (refactoring) is active.

## Code Style (C++)
Google C++ Style Guide, K&R braces, 2-space indent, omit single-line braces, max 80 chars/line

## Source Files to Migrate (16 files under src/notification/src/)
notification.c, notification_db.c, notification_error.c,
notification_group.c, notification_internal.c, notification_internal_tidl.c,
notification_ipc.c, notification_list.c, notification_noti.c,
notification_ongoing.c, notification_setting.c, notification_setting_service.c,
notification_shared_file.c, notification_status.c, notification_tidl.c,
notification_viewer.c

---

## B-SETUP-001 — CMakeLists C++ flags
- Edit `src/notification/CMakeLists.txt`:
  - Change source extensions `.c` → `.cc` in source lists
  - Add `-std=c++17` and `set_target_properties(... PROPERTIES CXX_STANDARD 17)`
  - Keep all `EXPORT_API` symbols — do NOT remove or rename any public function
- GBS build must succeed (0 errors)
- Commit: `build(notification): configure C++17 compilation for migration`
- Push: `git push origin default`

## B-MIG-001..016 — .c → .cc migration (file by file)

Migrate in dependency-safe order:
1. notification_error.c → .cc
2. notification_list.c → .cc
3. notification_ipc.c → .cc
4. notification_db.c → .cc
5. notification_noti.c → .cc
6. notification_shared_file.c → .cc
7. notification_status.c → .cc
8. notification_group.c → .cc
9. notification_viewer.c → .cc
10. notification_setting.c → .cc
11. notification_setting_service.c → .cc
12. notification_internal_tidl.c → .cc
13. notification_tidl.c → .cc
14. notification_ongoing.c → .cc
15. notification_internal.c → .cc
16. notification.c → .cc

Per file, fix C++ incompatibilities:
- Designated initializers → constructor / explicit init
- VLAs → std::vector or fixed arrays
- Compound literals → named temporaries
- `_Bool` → `bool`
- `restrict` → remove (not standard C++)
- `void*` implicit cast → explicit static_cast
- C99 mixed declarations → move to top of scope
- Add `extern "C" { }` guards in C headers included by .cc files

After each file: GBS build (0 errors), run all 3 suites (0 FAIL).
Commit per file: `refactor(notification): migrate <filename> to C++ (B-MIG-00N)`

## B-RAII-001 — RAII wrappers
- sqlite3* → RAII wrapper struct with destructor
- FILE* / fd → RAII closer
- GLib heap objects → std::unique_ptr<T, GFreeDeleter>
- DB transactions → RAII guard (BEGIN on construct, COMMIT/ROLLBACK on destruct)
After: GBS build + all 3 suites pass.
Commit: `refactor(notification): add RAII wrappers (B-RAII-001)`

## B-MEM-001 — STL migration
- g_malloc/g_free → std::vector where internal-only
- g_strdup/g_free char* → std::string where ABI-safe (not in public structs)
- GList → std::vector/std::list where internal-only
After: GBS build + all 3 suites pass.
Commit: `refactor(notification): STL memory migration (B-MEM-001)`

## B-OPT-001 — Dead code removal
- Remove unused static functions and dead paths in notification_group.c
After: GBS build + all 3 suites pass.
Commit: `refactor(notification): remove dead code (B-OPT-001)`

---

## B-COV-001 — Final Regression Gate

Run ALL three test suites and collect full results:

```bash
gbs build -A x86_64 --include-all -B /home/kimhyeonuk/GBS-ROOT-REFACTOR
# inside chroot:
ctest -R notification --output-on-failure -v
```

Required results:
- Unit tests: >= 157 tests, 0 FAIL
- Integration tests: >= 104 tests, 0 FAIL (fewer SKIP than before A is a bonus)
- Smoke tests: 6 PASS

Update STATE.md in refactoring-plans:
```
PHASE: refactoring-complete
unit_tests: COMPLETE
integration_tests: COMPLETE
smoke_tests: COMPLETE
refactoring: COMPLETE
```
`git -C ../refactoring-plans commit -am "chore: notification refactoring complete"`
`git -C ../refactoring-plans push origin default`

Final commit: `refactor(notification): C++17 migration complete — COV-001 regression gate PASS`
Push: `git push origin default`

Also update WORKFLOWS.md in the dormammu workspace:
- Mark Phase 10 [O]: actual task IDs (SETUP-001, MIG-001..016, RAII-001, MEM-001, OPT-001)
- Mark Phase 11 [O]: COV-001 regression gate result summary
- Mark Phase 12 [O]: final commit hash

---

## Execution Rules
1. Auto-advance on task completion — no human input
2. After EACH task: GBS build + run all 3 suites → all must PASS before next task
3. GBS: no sudo, run directly
4. Valid stops: GBS FAIL after 3 retries, or unresolvable ABI break
5. ABI rule: all `EXPORT_API` function signatures must remain byte-identical
6. No `.c` files may remain in `src/notification/src/` when done (CMakeLists auto-picks `.cc`)
