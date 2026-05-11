# notification — API-by-API Integration & Unit Tests (Group C + Gaps)

## Branch Rule
ALL work on **`default`** branch only.
Push: `git push origin default`

## GBS Build — NO SUDO
```bash
gbs build -A x86_64 --include-all -B /home/kimhyeonuk/GBS-ROOT-API-TESTS
```

## Excluded
`src/notification-ex/` — do NOT touch, do NOT test

---

## Context

Integration tests (85 tests, 14 files) and unit tests are already complete for:
- **Group A** (setter/getter): Covered by `test_notification_field_integration.cc`, `test_notification_lifecycle_integration.cc`, etc.
- **Group B** (DB layer): Covered by `test_notification_db_integration.cc`, `test_notification_noti_integration.cc` (if exists), `test_notification_lifecycle_integration.cc`, etc.

**What is MISSING** — Group C: TIDL-dependent functions.

`notification_post`, `notification_update`, `notification_delete`, `notification_delete_all`,
`notification_save_as_template`, `notification_create_from_template` all call
`notification_tidl_request_*` functions inside `libnotification.so`, which require a live
notification daemon. These currently SKIP in all test environments.

---

## Objective

Implement TIDL interposition stubs and dedicated integration tests for Group C TIDL-dependent API functions, so that they run (and pass) without a live daemon.

---

## Technical Background: PLT Interposition

`libnotification.so` is compiled with `-fPIC`. On Linux, globally-visible symbols in a PIC
shared library resolve via PLT even for intra-library calls. The dynamic linker gives
precedence to symbols defined in the **main executable** over symbols in DSOs.

This means: if we define `notification_tidl_request_insert` (etc.) in a `.cc` file compiled
into the `notification-integration-tests` executable, those definitions will intercept the
calls made INSIDE `libnotification.so` when `notification_post_for_uid` calls
`notification_tidl_request_insert`.

This is the exact same mechanism used by `tests/integration/src/test_db_override.cc` for
`tzplatform_getenv` and `tzplatform_mkpath`.

---

## Step 1 — Read existing work

Read these files before writing any code:
- `tests/integration/src/test_db_override.cc` — understand the PLT interposition pattern
- `src/notification/include/notification_tidl.h` — full list of TIDL request function signatures
- `src/notification/src/notification_internal.c` lines 1481-1560 — `notification_post_for_uid`, `notification_update_for_uid`, `notification_delete_for_uid`, `notification_delete_all_for_uid`
- `src/notification/src/notification.c` lines 1975-2000 — `notification_save_as_template`, `notification_create_from_template`
- `tests/integration/src/test_notification_lifecycle_integration.cc` — existing DB-path lifecycle tests (do NOT duplicate)
- `tests/integration/CMakeLists.txt` — build config (AUX_SOURCE_DIRECTORY picks up all .cc in src/)

---

## Step 2 — Create TIDL interposition stub file

Create `tests/integration/src/tidl_override.cc`.

This file defines stub implementations of ALL `notification_tidl_request_*` functions declared
in `notification_tidl.h`. Each stub:
- Records whether it was called (via a thread-local or static variable) so tests can verify invocations
- Returns `NOTIFICATION_ERROR_NONE` for "insert" (and sets `*priv_id = 9001`)
- Returns `NOTIFICATION_ERROR_NONE` for "update", "delete_single", "delete_multiple"
- Returns `NOTIFICATION_ERROR_NONE` for "save_as_template", "create_from_template" (and creates a real noti via `notification_create` for the out-param)
- Returns `NOTIFICATION_ERROR_IO_ERROR` for anything that would need a real connection but isn't tested
- For `notification_tidl_is_master_ready`: return 1 (connected)

Define control variables so tests can set expected return codes and inspect call counts:
```cpp
extern "C" void tidl_stub_reset(void);     // reset all counters/flags
extern "C" int  tidl_stub_insert_called;   // how many times insert was called
extern "C" int  tidl_stub_update_called;
extern "C" int  tidl_stub_delete_called;
extern "C" int  tidl_stub_delete_all_called;
extern "C" int  tidl_stub_next_insert_ret; // override return code (default 0)
```

Only stub functions declared in `notification_tidl.h`. Do NOT stub unrelated functions.

---

## Step 3 — Create Group C integration test file

Create `tests/integration/src/test_notification_post_integration.cc`.

This file tests the TIDL-path API functions using the stubs from Step 2.

### Test fixture

```cpp
class NotificationPostIntegrationTest : public ::testing::Test {
 protected:
  void SetUp() override {
    tidl_stub_reset();
    // Create temp DB for notification_noti_* side effects if needed
    char tmp_dir[] = "/tmp/noti_integ_post_XXXXXX";
    if (!mkdtemp(tmp_dir)) GTEST_SKIP() << "Failed to create temp dir";
    tmp_dir_ = tmp_dir;
    test_set_db_dir(tmp_dir_.c_str());
    // Create noti_list table
    // (copy CREATE_NOTI_TABLE_SQL from test_notification_lifecycle_integration.cc)
  }
  void TearDown() override {
    if (!db_path_.empty()) unlink(db_path_.c_str());
    if (!tmp_dir_.empty()) rmdir(tmp_dir_.c_str());
  }
  std::string tmp_dir_;
  std::string db_path_;
};
```

### Required test cases

**POST_01**: `notification_post` with valid noti → stub returns NONE → `priv_id` updated in handle
```cpp
notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
if (!noti) GTEST_SKIP() << "no app context";
notification_set_display_applist(noti, NOTIFICATION_DISPLAY_APP_NOTIFICATION_TRAY);
int ret = notification_post(noti);
EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
EXPECT_EQ(tidl_stub_insert_called, 1);
int gid = 0, pid = 0;
notification_get_id(noti, &gid, &pid);
EXPECT_EQ(pid, 9001);  // stub sets priv_id = 9001
notification_free(noti);
```

**POST_02**: `notification_post` with NULL noti → `NOTIFICATION_ERROR_INVALID_PARAMETER` (no stub call)

**POST_03**: `notification_post` when stub returns IO_ERROR → caller gets IO_ERROR
```cpp
tidl_stub_next_insert_ret = NOTIFICATION_ERROR_IO_ERROR;
// ... call notification_post → expect IO_ERROR
```

**UPDATE_01**: `notification_update` with valid noti → stub called once → NONE

**UPDATE_02**: `notification_update` with NULL → `NOTIFICATION_ERROR_INVALID_PARAMETER`

**DELETE_01**: `notification_delete` with valid noti (priv_id set) → stub called → NONE

**DELETE_02**: `notification_delete` with NULL → `NOTIFICATION_ERROR_INVALID_PARAMETER`

**DELETE_ALL_01**: `notification_delete_all(NOTIFICATION_TYPE_NOTI)` → stub called → NONE

**DELETE_ALL_02**: `notification_delete_all` with invalid type → `NOTIFICATION_ERROR_INVALID_PARAMETER`

**TEMPLATE_01**: `notification_save_as_template` with NULL noti → error

**TEMPLATE_02**: `notification_save_as_template` with NULL name → error

**TEMPLATE_03**: `notification_create_from_template` with NULL name → returns NULL handle

---

## Step 4 — Extend unit tests for uncovered TIDL paths

Read `tests/unittests/src/test_notification_tidl.cc`. If it only has NULL-parameter tests,
add the following tests to it (append to the existing file):

**Additional unit tests** (mock-based, no DB needed):
- `notification_post_calls_tidl_insert` — set up AulMock to return a fake app_id, call `notification_post` with a real noti handle, verify the mock framework processes it (or GTEST_SKIP if the mock doesn't cover TIDL internals)
- `notification_delete_all_invalid_type_returns_error` — already covered? If not, add it.
- `notification_get_noti_block_state_null_state` — `notification_get_noti_block_state(NULL)` → invalid parameter

---

## Step 5 — GBS Build & Verify

```bash
gbs build -A x86_64 --include-all -B /home/kimhyeonuk/GBS-ROOT-API-TESTS
```

The new integration tests must:
- Compile without errors or warnings
- `POST_01`, `UPDATE_01`, `DELETE_01`, `DELETE_ALL_01`: PASS (stub intercepts correctly) OR SKIP with reason if PLT interposition doesn't work on this build
- `POST_02`, `UPDATE_02`, `DELETE_02`, `DELETE_ALL_02`, etc. (NULL/invalid): always PASS
- Zero build errors

If PLT interposition does NOT work (stub is not called, tests would fail), detect this in the
`SetUp()` by calling `notification_tidl_is_master_ready()` and checking its return value. If it
returns 0 instead of 1, `GTEST_SKIP() << "PLT interposition not active in this build"`.

---

## Step 6 — Commit & Push

```bash
git add tests/integration/src/tidl_override.cc
git add tests/integration/src/test_notification_post_integration.cc
git add tests/unittests/src/test_notification_tidl.cc
git commit -m "test(integration): add Group C TIDL stub interposition + notification_post/update/delete tests"
git push origin default
```

Update STATE.md in refactoring-plans:
```
PHASE: test-suite-complete
unit_tests: COMPLETE
integration_tests: COMPLETE (Group A+B+C — 96+ tests)
smoke_tests: COMPLETE
refactoring: NOT_STARTED
```

```bash
git -C ../refactoring-plans add -A
git -C ../refactoring-plans commit -m "chore: update notification STATE.md — Group C TIDL tests complete"
git -C ../refactoring-plans push origin default
```

---

## Completion Gate

- [ ] `tests/integration/src/tidl_override.cc` created — stubs all `notification_tidl_request_*` functions
- [ ] `tests/integration/src/test_notification_post_integration.cc` created — 10+ tests
- [ ] GBS build: zero errors
- [ ] NULL/invalid-param tests: all PASS
- [ ] TIDL-path tests (POST_01 etc.): PASS or SKIP with clear reason
- [ ] Committed and pushed to `default`
- [ ] STATE.md updated and pushed in refactoring-plans

---

## Discord Notification on Completion

When ALL completion gate items are checked off, send a Discord DM to notify the user:

```
[notification] ✅ Group C (TIDL mock) 테스트 완료
- tidl_override.cc: notification_tidl_request_* 스텁 구현
- test_notification_post_integration.cc: POST/UPDATE/DELETE/DELETE_ALL 통합 테스트
- GBS 빌드 성공, 커밋 및 푸시 완료
- STATE.md 업데이트 완료
```

Send to channel: `direct:369332395545198603`
Use skill: `openclaw-skills:discord`
