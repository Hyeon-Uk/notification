# notification — Integration Test Supplement

## Branch Rule
ALL work on **`default`** branch only.
Push: `git push origin default`

## GBS Build — NO SUDO
```bash
gbs build -A x86_64 --include-all -B /home/kimhyeonuk/GBS-ROOT-INTEG-SUPPLEMENT
```

## Excluded
`src/notification-ex/` — do NOT touch, do NOT test

---

## Context

Phase 1 integration tests were only **partially implemented**.

- **Planned**: 74 scenarios across 15 categories
  (`../refactoring-plans/execution/tasks/notification/test_scenarios_integration.md`)
- **Implemented**: 27 tests across 3 files
  - `tests/integration/src/test_notification_db_integration.cc` (10 tests — DB open/close/exec/noti CRUD)
  - `tests/integration/src/test_notification_list_integration.cc` (9 tests — list append/navigate/remove)
  - `tests/integration/src/test_notification_setting_integration.cc` (8 tests — settings read/write)
- **Missing**: ~47 scenarios across 12 categories

## Objective

Implement **all missing integration test scenarios** from
`../refactoring-plans/execution/tasks/notification/test_scenarios_integration.md`.

Do NOT duplicate or modify existing tests. Only add new tests.

---

## Step 1 — Read existing work

Read all three existing integration files to understand what is already covered:
- `tests/integration/src/test_notification_db_integration.cc`
- `tests/integration/src/test_notification_list_integration.cc`
- `tests/integration/src/test_notification_setting_integration.cc`

Read the full scenario document:
- `../refactoring-plans/execution/tasks/notification/test_scenarios_integration.md`

Read the API inventory:
- `../refactoring-plans/execution/tasks/notification/api_inventory.md`

---

## Step 2 — Implement missing integration test files

Create new `.cc` files in `tests/integration/src/` for each missing category.
`AUX_SOURCE_DIRECTORY` in `tests/integration/CMakeLists.txt` picks them up automatically — do NOT modify `CMakeLists.txt`.

### Required new files and their coverage:

**`test_notification_lifecycle_integration.cc`**
- Scenarios: LIFECYCLE_01~N from test_scenarios_integration.md
- Full create→post→update→delete flow with real DB
- `notification_create`, `notification_post`, `notification_update`,
  `notification_delete`, `notification_delete_all`
- Verify DB row exists/absent after each operation

**`test_notification_tag_integration.cc`**
- Scenarios: TAG_* from test_scenarios_integration.md
- `notification_load_by_tag`, `notification_load_by_tag_for_uid`
- Set tag → post → load_by_tag → verify all fields round-trip

**`test_notification_template_integration.cc`**
- Scenarios: TEMPLATE_* from test_scenarios_integration.md
- `notification_save_as_template`, `notification_create_from_template`,
  `notification_create_from_package_template`

**`test_notification_count_integration.cc`**
- Scenarios: COUNT_* from test_scenarios_integration.md
- `notification_get_all_count`, `notification_get_all_count_for_uid`
- Multiple post → count → delete_all → verify count==0

**`test_notification_ongoing_integration.cc`**
- Scenarios: ONGOING_* from test_scenarios_integration.md
- `notification_update_progress`, `notification_update_size`,
  `notification_update_content`
- `notification_get_ongoing_flag`, `notification_set_ongoing_flag`
- `notification_get/set_ongoing_value_type`, `notification_get/set_ongoing_time`

**`test_notification_field_integration.cc`**
- Scenarios: FIELD_* / LIFECYCLE_02 fields from test_scenarios_integration.md
- `notification_set/get_image`, `notification_set/get_text`,
  `notification_set/get_sound`, `notification_set/get_vibration`,
  `notification_set/get_led`, `notification_set/get_led_time_period`
- `notification_set/get_layout`, `notification_set/get_property`,
  `notification_set/get_display_applist`, `notification_set/get_size`,
  `notification_set/get_progress`, `notification_set/get_tag`,
  `notification_set/get_auto_remove`, `notification_set/get_default_button`
- Post → load_by_tag → verify round-trip for each field

**`test_notification_extension_integration.cc`**
- Scenarios: EXTENSION_* from test_scenarios_integration.md
- `notification_set/get_extension_data` (bundle key/value)
- `notification_set/get_extension_image_size`
- `notification_set/get_extension_event_handler`

**`test_notification_checkbox_integration.cc`**
- Scenarios: CHECKBOX_* from test_scenarios_integration.md
- `notification_set/get_check_box`, `notification_set/get_check_box_checked`

**`test_notification_uid_integration.cc`**
- Scenarios: MULTI_* from test_scenarios_integration.md
- `notification_post_for_uid`, `notification_update_for_uid`,
  `notification_delete_for_uid`, `notification_delete_all_for_uid`,
  `notification_load_by_tag_for_uid`, `notification_insert_for_uid`,
  `notification_get_all_count_for_uid`

**`test_notification_channel_integration.cc`**
- Scenarios: CHANNEL_* from test_scenarios_integration.md
- `notification_channel_create`, `notification_channel_free`,
  `notification_channel_add`, `notification_channel_remove`,
  `notification_channel_update`, `notification_channel_get_by_name`
- `notification_channel_set/get_blockable`, `notification_channel_set/get_block`
- `notification_set/get_channel_name`

**`test_notification_error_recovery_integration.cc`**
- Scenarios: NEGATIVE_* / ERROR_* from test_scenarios_integration.md
- NULL handle → NOTIFICATION_ERROR_INVALID_PARAMETER
- Double-delete → NOTIFICATION_ERROR_NOT_EXIST or ERROR_NONE (idempotent)
- `notification_get_noti_block_state` (real DB)
- `notification_clone` and free both handles
- `notification_add_button` / `notification_remove_button`

**`test_notification_ipc_integration.cc`**
- Scenarios: IPC_* from test_scenarios_integration.md
- `notification_ipc_make_noti_from_gvariant` / `notification_ipc_make_gvariant_from_noti`
  round-trip (if accessible without live daemon)
- TIDL serialization: `notification_make_noti_from_package_template` path

---

## Step 3 — Rules for ALL test files

```cpp
// Each test:
// 1. SetUpTestSuite / TearDownTestSuite: open/close real sqlite DB
//    Use a unique temp path per suite: /tmp/noti_integ_<suite>_XXXXXX.db
// 2. SetUp: fresh DB state before each test
// 3. TearDown: unlink temp DB
// 4. NO mocks — real sqlite3, real filesystem
// 5. Tests that require live daemon: GTEST_SKIP() with reason
// 6. Every test has a concrete EXPECT_ assertion on return code or DB state
```

---

## Step 4 — GBS Build & Verify

```bash
gbs build -A x86_64 --include-all -B /home/kimhyeonuk/GBS-ROOT-INTEG-SUPPLEMENT
```

All new tests must:
- Compile without errors
- Pass or SKIP (no FAIL) under GBS chroot

Run `ctest -R notification-integration` to confirm.

---

## Step 5 — Commit & Push

```bash
git add tests/integration/src/test_notification_*.cc
git commit -m "test(integration): complete missing integration test scenarios (categories 2-14)"
git push origin default
```

---

## Completion Gate

- [ ] All 12 new test files created
- [ ] Total integration tests ≥ 60 (currently 27)
- [ ] GBS build: zero errors
- [ ] `ctest -R notification-integration`: all GREEN or SKIP
- [ ] Committed and pushed to `default`

Update STATE.md in refactoring-plans:
```
PHASE: test-suite-complete
unit_tests: COMPLETE
integration_tests: COMPLETE (supplemented)
smoke_tests: COMPLETE
refactoring: NOT_STARTED
```
`git -C ../refactoring-plans add -A && git -C ../refactoring-plans commit -m "chore: update notification STATE.md — integration tests supplemented" && git -C ../refactoring-plans push origin default`
