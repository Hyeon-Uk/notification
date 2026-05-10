# notification C++ Refactoring - Analysis & Planning

## Context

You are the **supervisor** for the notification package C++ refactoring.
Read the AGENTS.md at `agents/AGENTS.md` (one level up from this repo root)
for code style rules and project context before proceeding.

Current state:
- Stage 8 (C to C++) is underway. N8-B.2 completed (commit `069df7b`).
- Next in existing plan: N8-C (TIDL wrapper: `notification_tidl.c` +
  `notification_internal_tidl.c`, ~3,218 LOC)
- Existing tests: 382 PASSED, 37 SKIPPED, 15 DISABLED
- ABI: 316 dynamic public symbols (must remain stable)

Refactoring goals (this session):
1. Memory optimization (reduce heap, RAII, eliminate leaks)
2. Storage/binary size optimization
3. Complete C to C++ migration (remaining .c files)
4. RAII compliance throughout

## Step 1 - Read Existing Plans

Read these files to understand what has been done and what remains:
- `../refactoring-plans/execution/ROADMAP_v3.md`
- `../refactoring-plans/REFACTORING-PROCESS-v6-amendment.md`
- `../refactoring-plans/execution/progress/STATE.md`
- `../refactoring-plans/execution/tasks/notification/` (all task files)

Identify:
- All remaining tasks (not yet committed)
- Any D.Phase 5 tasks still pending
- N8-C TIDL wrapper (next immediate task)
- New optimization opportunities not yet in the plan

## Step 2 - Analyze Refactoring Opportunities

Using the `tizen-refactoring` skill, analyze `src/` for:

### 2.1 Memory Optimization Items
- Raw pointer ownership that should be `std::unique_ptr`
- `g_malloc`/`g_free`/`g_strdup` that should be `std::string`/`std::vector`
- Stack-allocatable structs currently on heap
- Missing destructors / resource leaks
- GLib container usage (`GList`, `GHashTable`) replaceable with STL

### 2.2 RAII Candidates
- `sqlite3*` connections and statements -> RAII wrappers
- File descriptors (`open`/`close`) -> RAII
- GLib objects with ref counting -> RAII
- Lock/unlock pairs -> `std::lock_guard`
- DB transactions (BEGIN/COMMIT/ROLLBACK) -> RAII

### 2.3 Remaining C-to-C++ Files
List all `.c` files not yet migrated to `.cc`.
For each file: LOC, complexity assessment, dependencies.

### 2.4 Storage/Binary Size Items
- Dead code elimination candidates
- String literal deduplication opportunities
- Unused exported symbols (ABI-safe to remove)

## Step 3 - Test Scenario Generation

For each identified refactoring item, use the `test-scenario-generator`
skill to generate test scenarios covering:

1. **Existing behavior preservation** (baseline/regression tests)
   - All public API behaviors (EXPORT_API functions)
   - Error path behaviors (NULL params, invalid IDs, DB failures)
   - Edge cases (empty lists, max values, concurrent access)

2. **Internal API coverage** (for files being refactored)
   - Static helpers / namespace-internal functions
   - Use `_for_test_` exports or friend class patterns

3. **Resource management verification**
   - Memory leak detection (valgrind / PerfAwareFixture)
   - RAII destruction ordering
   - Exception safety (no resource leaks on throw)

Generate test scenarios in this format:
```
| Test Name | Target | Scenario | Expected |
|-----------|--------|----------|----------|
```

Write the scenario table to:
`../refactoring-plans/execution/tasks/notification/test_scenarios_new.md`

## Step 4 - Create New Roadmap

Based on Steps 2 and 3, create a comprehensive task list:

### Ordering rule
1. N8-C TIDL wrapper (immediate - already planned)
2. RAII wrappers for DB (sqlite3 + transaction)
3. RAII wrappers for file descriptors
4. Memory: g_malloc -> std::string/vector migration
5. Memory: GList/GHashTable -> STL migration
6. Binary size: dead code elimination
7. Final coverage verification (Stage 7.5 gates)

For each task, define:
- Task ID (e.g., `RAII-DB-001`)
- Files affected
- LOC estimate
- Test scenarios required
- GBS build tag
- Acceptance criteria

Write the new task list to:
`../refactoring-plans/execution/tasks/notification/new_cpp_tasks.md`

## Step 5 - Generate Per-Task Prompts

For EACH task in the new roadmap, write a dedicated prompt file at:
`prompts/<task_id>.md`

Each prompt must include:
- Exact files to modify
- Test scenarios to implement first (TDD)
- Implementation specification
- Code style checklist (2-space, K&R, 80-char, brace omission)
- Acceptance criteria (GBS build PASS + ctest regression = 0)

## Step 6 - Execute N8-C (First Task)

Once the plan is complete, execute N8-C immediately:
1. Load prompt `prompts/N8-C.md`
2. Run test-scenario-generator for TIDL wrapper files
3. Write tests first
4. Migrate `notification_tidl.c` + `notification_internal_tidl.c` to `.cc`
5. Verify: gbs build PASS + ctest regression = 0 + ABI diff = 0
6. Commit

## Code Style Reminder

Every line of C++ you write MUST follow (see AGENTS.md for full rules):
- Google C++ Style Guide
- K&R braces: opening brace on same line
- 2-space indent (no tabs)
- Omit braces for single-line bodies
- Max 80 chars per line
- Smart pointers over raw owning pointers
