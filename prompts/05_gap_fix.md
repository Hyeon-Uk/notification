# notification — Gap Analysis & Fill: Uncovered APIs and Scenarios

## CRITICAL: Branch Rule
ALL work on the **`default`** branch only. All pushes: `git push origin default`

## GBS Build — NO SUDO
```bash
gbs build -A x86_64 --include-all -B /home/kimhyeonuk/GBS-ROOT-GAP
```

## Excluded
`src/notification-ex/` — do NOT touch, do NOT test

---

## Your Job

### Step 1: Cross-reference APIs vs Tests (Unit)

Read:
- `../refactoring-plans/execution/tasks/notification/api_inventory.md` — all public + internal functions
- All files in `tests/unittests/src/` — what is currently tested

For each function in the API inventory:
- Check if at least one `TEST` or `TEST_F` in the unit test files covers it (even indirectly via a fixture that calls it)
- Mark as UNCOVERED if no test touches it at all

### Step 2: Cross-reference Scenarios vs Tests (Unit)

Read:
- `../refactoring-plans/execution/tasks/notification/test_scenarios_unit.md`
- All files in `tests/unittests/src/`

For each scenario ID in test_scenarios_unit.md:
- Check if the corresponding test case exists in the test files
- Mark as MISSING if no test implements it

### Step 3: Cross-reference Scenarios vs Tests (Integration)

Read:
- `../refactoring-plans/execution/tasks/notification/test_scenarios_integration.md`
- All files in `tests/integration/src/`

For each scenario ID in test_scenarios_integration.md:
- Check if a corresponding integration test exists
- Mark as MISSING if no test covers it

### Step 4: Cross-reference Scenarios vs Tests (Smoke)

Read:
- `../refactoring-plans/execution/tasks/notification/test_scenarios_smoke.md`
- All files in `tests/smoke/`

For each scenario ID in test_scenarios_smoke.md:
- Check if a corresponding smoke test exists
- Mark as MISSING if no smoke test covers it

### Step 5: Write gap report

Write gap report to:
`../refactoring-plans/execution/tasks/notification/GAP_REPORT.md`

Format:
```markdown
# Coverage Gap Report
Date: <today>
Loop iteration: <N>

## Uncovered Public API Functions (Unit Tests)
- function_name(args) — source file

## Uncovered Internal API Functions (Unit Tests)  
- function_name(args) — source file

## Missing Unit Test Scenarios
- SCENARIO_ID: description

## Missing Integration Test Scenarios
- SCENARIO_ID: description

## Missing Smoke Test Scenarios
- SCENARIO_ID: description

## Summary
Total gaps: N
```

### Step 6: Implement missing tests

For each gap found:

**Unit test gaps:** Write tests in the appropriate `tests/unittests/src/test_<source_file>.cc`.
Use the same pattern as existing tests in that file. Follow the mock pattern in `tests/mock/`.

**Integration test gaps:** Write tests in `tests/integration/src/test_notification_<category>_integration.cc`.
Use real temp DB (mkstemp pattern from existing files). No mocks.

**Smoke test gaps:** Add to appropriate script in `tests/smoke/`.

### Step 7: GBS build + verify

```bash
gbs build -A x86_64 --include-all -B /home/kimhyeonuk/GBS-ROOT-GAP
ctest -R notification --output-on-failure -v 2>&1 | tee /tmp/noti-gap-test-result.txt
```

Gate: 0 FAILs across all suites.

### Step 8: Commit and push

```
git add tests/
git commit -m "test: fill coverage gaps — <N> missing scenarios added"
git push origin default
```

### Step 9: Update gap report with final test counts

Append to `GAP_REPORT.md`:
```markdown
## After Fix
Unit: X PASS / 0 FAIL
Integration: X PASS / Y SKIP / 0 FAIL
Smoke: 6 PASS
Gaps remaining: 0 (or N if any couldn't be implemented)
```

Commit:
```
git -C ../refactoring-plans add execution/tasks/notification/GAP_REPORT.md
git -C ../refactoring-plans commit -m "test: update gap report after fix"
git -C ../refactoring-plans push origin default
```

## Execution Rules
1. Auto-advance — no human input needed
2. GBS: no sudo, run directly  
3. Implement every missing test unless technically impossible in GBS chroot
4. If a test is truly impossible (requires live daemon, real hardware), note it in GAP_REPORT.md as BLOCKED with reason
5. Valid stops: GBS FAIL after 3 retries on same file
