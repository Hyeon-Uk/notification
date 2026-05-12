# notification — Phase A: Fix Integration Test SKIPs (AUL PLT Interposition)

## CRITICAL: Branch Rule
ALL work on the **`default`** branch only. All pushes: `git push origin default`

## GBS Build — NO SUDO
```bash
gbs build -A x86_64 --include-all -B /home/kimhyeonuk/GBS-ROOT-REFACTOR
```

## Excluded
`src/notification-ex/` — do NOT touch, do NOT test

---

## Background
33 integration tests SKIP because `notification_create()` internally calls
`aul_app_get_appid_bypid()`. GBS chroot has no AUL daemon → returns error → NULL handle.
`test_db_override.cc` and `tidl_override.cc` already use the same PLT interposition technique.
`AUX_SOURCE_DIRECTORY` auto-picks all `.cc` files in `tests/integration/src/` — no CMakeLists
edit needed, just drop the file.

## Also check pkg_manager mock
`notification_create` also calls `package_manager_get_package_info` for uid >= REGULAR_UID_MIN.
Check `tests/mock/package_manager_mock.cc` for existing interpositions and extend if needed.

---

## Task A-1: Create tests/integration/src/aul_override.cc

```cpp
/*
 * PLT interposition for AUL functions in integration tests.
 * Mirrors test_db_override.cc / tidl_override.cc pattern.
 * Provides a fake app context so notification_create() works in GBS chroot.
 */
#include <cstring>
#include <aul.h>

static const char kFakeId[] = "org.tizen.test.notification.integration";

extern "C" int aul_app_get_appid_bypid(int /*pid*/, char* appid, int len) {
  strncpy(appid, kFakeId, len - 1);
  appid[len - 1] = '\0';
  return 0;
}

extern "C" int aul_app_get_pkgname_bypid(int /*pid*/, char* pkg, int len) {
  strncpy(pkg, kFakeId, len - 1);
  pkg[len - 1] = '\0';
  return 0;
}

extern "C" int aul_app_get_pkgid_bypid(int /*pid*/, char* pkgid, int len) {
  strncpy(pkgid, kFakeId, len - 1);
  pkgid[len - 1] = '\0';
  return 0;
}
```

## Task A-2: Check package_manager interposition

Read `tests/integration/src/test_db_override.cc` — if `package_info_h` or
`package_manager_get_package_info` is NOT already stubbed there, add stubs in
`aul_override.cc`:

```cpp
#include <package_manager.h>
extern "C" int package_manager_get_package_info(
    const char* /*package_id*/, package_info_h* package_info) {
  *package_info = nullptr;
  return PACKAGE_MANAGER_ERROR_NONE;
}
extern "C" int package_info_destroy(package_info_h /*info*/) {
  return PACKAGE_MANAGER_ERROR_NONE;
}
```

## Task A-3: GBS build + verify

```bash
gbs build -A x86_64 --include-all -B /home/kimhyeonuk/GBS-ROOT-REFACTOR
```
Gate: exit 0, 0 compile errors.

Then run integration tests inside chroot:
```
ctest -R notification-integration --output-on-failure -v
```
Gate: SKIP count < 10 AND 0 FAILs.

If SKIP count is still ≥ 10, read the skip messages and add more interpositions until gate passes.

## Task A-4: Commit and push

```
git add tests/integration/src/aul_override.cc
git commit -m "test(integration): add AUL PLT interposition to fix notification_create SKIPs"
git push origin default
```

## Task A-5: Run full test suite and write result to file

Run all three suites:
```
ctest -R notification --output-on-failure -v 2>&1 | tee /tmp/noti-test-after-aul-fix.txt
```

Write summary to `../refactoring-plans/execution/tasks/notification/TEST_RESULT_PHASE_A.md`:
```markdown
# Test Result after Phase A (AUL PLT Fix)
Date: <today>
## Unit Tests
Total: X  PASS: X  FAIL: 0  SKIP: 0
## Integration Tests
Total: X  PASS: X  FAIL: 0  SKIP: X (list remaining skip messages)
## Smoke Tests
Total: 6  PASS: 6
```

Commit and push:
```
git -C ../refactoring-plans add execution/tasks/notification/TEST_RESULT_PHASE_A.md
git -C ../refactoring-plans commit -m "test: Phase A test results (AUL PLT fix)"
git -C ../refactoring-plans push origin default
```

## Execution Rules
1. Auto-advance — no human input
2. GBS: no sudo, run directly
3. Valid stops: GBS FAIL after 3 retries
