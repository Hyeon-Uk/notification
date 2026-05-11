# notification Refactoring — Master Prompt

## CRITICAL: Branch Rule
ALL work on the **`default`** branch only.
Do NOT create or switch to any other branch.
All pushes: `git push origin default`

## ABSOLUTE REQUIREMENT — READ BEFORE ANYTHING ELSE

**You MUST NOT write a single line of refactoring code until ALL THREE test
suites are complete and passing.**

Test suites are complete ONLY when:
1. **Unit tests** — 100% method coverage, all Public API + Internal API
   (notification-ex excluded)
2. **Integration tests** — all end-to-end flows, real DB/filesystem (no mocks)
3. **Smoke tests** — binary loads, all symbols present, perf/memory/FD checks

If refactoring code is committed before all three suites pass → FAILURE.

## GBS Build — NO SUDO
```bash
gbs build -A x86_64 --include-all -B /home/kimhyeonuk/GBS-ROOT-<TAG>
```
No sudo. Ignore any stop condition citing sudo/root.

## Excluded
`src/notification-ex/` — do NOT touch, do NOT test

## Code Style (for C++ written in Phase 2)
Google C++ Style Guide, K&R braces, 2-space indent, omit single-line braces,
max 80 chars/line

---

# PHASE 1 — TEST SUITE (mandatory, blocks all refactoring)

## Step 1-A: API Inventory

Read all files in `src/notification/src/` and `src/notification/include/`.
List every symbol:
- `EXPORT_API` functions (Public API)
- Non-static internal functions (Internal API)
- Static helpers
- Structs, enums, typedefs

Write to:
`../refactoring-plans/execution/tasks/notification/api_inventory.md`

## Step 1-B: Test Scenario Generation

Use the `test-scenario-generator` skill for EVERY symbol in the inventory.

Per function, generate:
- Happy path
- NULL / invalid param paths
- Boundary values (0, INT_MAX, empty string, max-length)
- Error propagation (DB fail, alloc fail, IPC fail)
- State transitions (init → use → cleanup)
- Concurrent access where applicable

Write to:
`../refactoring-plans/execution/tasks/notification/test_scenarios_unit.md`
`../refactoring-plans/execution/tasks/notification/test_scenarios_integration.md`
`../refactoring-plans/execution/tasks/notification/test_scenarios_smoke.md`

## Step 1-C: Unit Tests

Write tests in `tests/unittests/src/` for every scenario in
`test_scenarios_unit.md`.

- GTest + GMock
- Mocks allowed for sqlite, dbus, GLib
- Every Public + Internal function tested
- 100% method coverage (verify with gcov/lcov)
- One test file per source file

Build:
```bash
gbs build -A x86_64 --include-all -B /home/kimhyeonuk/GBS-ROOT-UNIT-TEST
```

Gate: ALL tests GREEN + 100% method coverage confirmed.

Commit: `test(unit): comprehensive unit tests — 100% method coverage`
Push: `git push origin default`

## Step 1-D: Integration Tests

Write tests in `tests/integration/` for every scenario in
`test_scenarios_integration.md`.

- Real temp DB (no sqlite mock)
- Real temp filesystem
- Each test: setup → run → teardown (no leftover files)
- Multi-step flows: post→query, post→update→delete, concurrent threads
- Runnable inside GBS chroot (no live daemon)
- Registered with ADD_TEST in CMakeLists.txt

Build:
```bash
gbs build -A x86_64 --include-all -B /home/kimhyeonuk/GBS-ROOT-INTEG-TEST
```

Gate: ALL integration tests GREEN.

Commit: `test(integration): comprehensive integration test suite`
Push: `git push origin default`

## Step 1-E: Smoke Tests

Write scripts in `tests/smoke/` for every scenario in
`test_scenarios_smoke.md`.

Required (bash, exit 0=PASS / non-zero=FAIL, NO early PASS if .so missing):
- `smoke_01_symbol_check.sh` — all EXPORT_API symbols in libnotification.so
- `smoke_02_binary_size.sh` — .so size <= baseline + 5%
- `smoke_03_load.sh` — dlopen succeeds, no crash
- `smoke_04_cold_start.sh` — first notification_post < 500ms
- `smoke_05_memory.sh` — post×100, VmRSS delta < 2MB
- `smoke_06_fd_leak.sh` — post + free, open FD count unchanged

Register all with ADD_TEST in `tests/smoke/CMakeLists.txt`.

Build:
```bash
gbs build -A x86_64 --include-all -B /home/kimhyeonuk/GBS-ROOT-SMOKE-TEST
```

Gate: ALL smoke tests PASS.

Commit: `test(smoke): comprehensive smoke test suite`
Push: `git push origin default`

## Phase 1 Gate (ALL must be true before Phase 2)
- [ ] api_inventory.md complete
- [ ] Unit tests: 100% method coverage, all GREEN
- [ ] Integration tests: all flows GREEN
- [ ] Smoke tests: all 6 scripts PASS
- [ ] All three commits pushed to default

Write STATE.md:
```
PHASE: test-suite-complete
unit_tests: COMPLETE
integration_tests: COMPLETE
smoke_tests: COMPLETE
refactoring: NOT_STARTED
```
`git -C ../refactoring-plans push origin default`

---

# PHASE 2 — REFACTORING (only after Phase 1 gate passes)

Use `tizen-refactoring` skill to analyze `src/notification/src/`:
- C→C++ migration (all .c, no notification-ex)
- Memory: g_malloc/g_strdup → std::string/vector, heap→stack
- RAII: sqlite3*, FDs, GLib objects, locks, DB transactions
- Storage: dead code, unused exports

Roadmap → `../refactoring-plans/execution/tasks/notification/ROADMAP.md`

Task order:
1. SETUP-001 — CMakeLists C++ flags
2. MIG-001..N — .c→.cc per file group
3. RAII-001 — RAII wrappers
4. MEM-001 — STL migration
5. OPT-001 — dead code
6. COV-001 — re-run all three test suites, verify zero regression

After each task: run unit + integration + smoke → all must pass.
Commit + push per release: `git push origin default`

---

# PHASE 3 — data-provider-master
Same Phase 1 + Phase 2 for:
`/home/kimhyeonuk/.openclaw/workspace/refactoring/data-provider-master`

---

## Execution Overrides
1. Auto-advance on task completion — no stopping between tasks
2. GBS: no sudo, run directly
3. Autonomous judgment — no human input
4. Valid stops: GBS FAIL after 3 retries, or unresolvable ABI break only
