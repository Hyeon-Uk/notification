# COVERAGE-VERIFY-001 — Stage 7.5 Coverage Gate Verification

## Context

Stage 8 (C→C++) is complete (N8-C at d369e4e, plus 10 post-fix commits).
Before any RAII or memory-optimization task begins, the Stage 7.5 coverage
gate must be verified and a fresh ctest baseline must be captured.
The old baseline of 382 PASSED / 37 SKIPPED / 15 DISABLED is from N8-B.2
and is stale after multiple fix commits.

**This task produces no source changes.**

## Acceptance Criteria

- [ ] `git log --oneline | grep d369e4e` returns a match (N8-C confirmed)
- [ ] `ls src/notification/src/*.c 2>/dev/null` returns nothing (zero .c files)
- [ ] Fresh ctest baseline captured (PASSED / SKIPPED / DISABLED counts recorded)
- [ ] lcov line coverage ≥ 80% per file (all 19 .cc files)
- [ ] lcov branch coverage ≥ 70% per file (all 19 .cc files)
- [ ] `tests/coverage/baseline_v3_notification.html` archived
- [ ] Any file below gate has additional tests written (TDD) before this task closes

## GBS Build Tag

`GBS-ROOT-COV-VERIFY` (use: `gbs build -A x86_64 --include-all -B /home/kimhyeonuk/GBS-ROOT-COV-VERIFY`)

## Step-by-Step Execution

### Step 1 — Verify N8-C and file status

```bash
cd /home/kimhyeonuk/.openclaw/workspace/refactoring/notification
git log --oneline | grep d369e4e
ls src/notification/src/*.c 2>/dev/null || echo "No .c files — OK"
```

### Step 2 — GBS build with coverage instrumentation

```bash
gbs build -A x86_64 --include-all \
  --define "gcov 1" \
  -B /home/kimhyeonuk/GBS-ROOT-COV-VERIFY
```

### Step 3 — Capture ctest baseline

Inside GBS chroot (or via gbs shell):
```bash
ctest --output-on-failure -j$(nproc) 2>&1 | tee /tmp/ctest_baseline.txt
grep -E '^[0-9]+ tests passed' /tmp/ctest_baseline.txt
```

Record: PASSED / FAILED / SKIPPED / DISABLED counts in STATE.md under
"Current test baseline (post-N8-C+fixes)".

### Step 4 — Generate lcov report

```bash
lcov -c -d build -o coverage.info --no-external
genhtml coverage.info -o tests/coverage/ --legend --title "Stage 7.5 baseline"
cp -r tests/coverage/ tests/coverage_baseline_v3_notification/
```

### Step 5 — Check per-file coverage

For each of the 19 .cc files, inspect `tests/coverage/index.html` or run:
```bash
lcov --list coverage.info 2>/dev/null | grep 'notification'
```

Files to check:
- notification.cc
- notification_bundle_util.cc
- notification_context.cc
- notification_db.cc
- notification_error.cc
- notification_group.cc
- notification_internal.cc
- notification_internal_tidl.cc
- notification_ipc.cc
- notification_list.cc
- notification_noti.cc
- notification_ongoing.cc
- notification_setting.cc
- notification_setting_service.cc
- notification_shared_file.cc
- notification_status.cc
- notification_tidl.cc
- notification_viewer.cc
- shared_file_common.cc

### Step 6 — Gap remediation (if any file fails gate)

If a file has line < 80% or branch < 70%:
1. Identify uncovered functions with `lcov --list coverage.info | grep <file>`
2. Write failing tests (TDD red) in `tests/unittests/src/`
3. Confirm tests fail: re-run ctest, see new failures
4. (No implementation changes needed — tests exercise existing code)
5. Re-run lcov, confirm gate now passed

### Step 7 — Archive and document

```bash
# Archive HTML report
cp tests/coverage/index.html \
   tests/coverage/baseline_v3_notification.html 2>/dev/null || true
```

Update `refactoring-plans/execution/progress/STATE.md`:
- Set Stage 7.5 gate status: VERIFIED (or list files still below gate)
- Record fresh ctest baseline counts
- Date: 2026-05-10

## Code Style Checklist (for any gap-remediation tests written)

- [ ] Google C++ Style Guide
- [ ] K&R braces: `if (cond) {` on same line
- [ ] 2-space indent, no tabs
- [ ] Omit braces for single-line bodies
- [ ] Max 80 chars per line
- [ ] No new production code changes (test-only)

## ABI Note

This task makes no source changes to libnotification.so.
ABI nm diff check is not required here, but confirm the baseline:
```bash
nm --dynamic --defined-only \
  /home/kimhyeonuk/GBS-ROOT-COV-VERIFY/local/BUILD/x86_64/RPMS/libnotification*.so \
  | wc -l
# Expected: 316
```

## Next Task

After this task is complete → **RAII-DB-001**
