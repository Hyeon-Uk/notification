# notification C++ Refactoring — RAII / Memory / Storage Pass

## Scope

**Target package:** `src/notification/` ONLY.

> **STRICTLY EXCLUDED:** `src/notification-ex/` and all its subdirectories.
> Do NOT analyze, plan, or modify any file under `src/notification-ex/`.
> Do NOT generate tasks for notification-ex.

## Current State (2026-05-10)

- Stage 8 FULLY COMPLETE on `tizen` branch (commit `812f07e`):
  All 19 `.c` files in `src/notification/src/` have been migrated to `.cc`.
  Post-Stage-8 bug-fix commits applied: `b2c390e`…`812f07e`
  (NULL-deref fixes, leak fixes, race fixes, SQL injection hardening).
- C-to-C++ migration: **DONE**. No remaining `.c` migration tasks.
- ABI: 316 dynamic public symbols — must remain stable.
- Tests baseline: re-verify with GBS build before writing new tests.

## Refactoring Goals (this session, in priority order)

1. **RAII compliance** — replace all manual resource management
2. **Memory optimization** — reduce heap allocations, eliminate leaks
3. **Storage/binary size reduction** — reduce binary footprint

## Reference Documents

The refactoring-plans repo has a new directory layout (merged from
`refactor/home`). Use these paths:

- Roadmap: `../refactoring-plans/02_roadmap/notification.md`
- Task specs: `../refactoring-plans/03_tasks/` (files 17-18 are most relevant:
  `17-memory-discipline-pass.md`, `18-storage-discipline-pass.md`)
- Progress state: `../refactoring-plans/06_progress/STATE.md`
- Process rules: `../refactoring-plans/REFACTORING-PROCESS-v6-amendment.md`
- Existing new task list: `../refactoring-plans/execution/tasks/notification/new_cpp_tasks.md`

## Step 1 — Read Plans & Confirm Scope

Read the following files to understand existing plans:

1. `../refactoring-plans/06_progress/STATE.md`
2. `../refactoring-plans/02_roadmap/notification.md`
3. `../refactoring-plans/03_tasks/17-memory-discipline-pass.md`
4. `../refactoring-plans/03_tasks/18-storage-discipline-pass.md`
5. `../refactoring-plans/execution/tasks/notification/new_cpp_tasks.md`

Confirm:
- All tasks are scoped to `src/notification/src/` only
- No notification-ex tasks exist in the plan
- Stage 8 migration is complete (no .c files remain in src/notification/src/)

## Step 2 — Analyze `src/notification/src/` for Opportunities

Analyze ONLY the `.cc` and `.h` files under `src/notification/src/`.
Do NOT touch anything under `src/notification-ex/`.

### 2.1 RAII Candidates (highest priority)

Scan for:
- `sqlite3*` connection handles → RAII `DbHandle` wrapper
- `sqlite3_stmt*` prepared statements → RAII `DbStatement` wrapper
- `BEGIN`/`COMMIT`/`ROLLBACK` patterns → RAII `DbTransaction` wrapper
- `open()`/`close()` file descriptor pairs → RAII `ScopedFd` wrapper
- `fopen()`/`fclose()` FILE* pairs → RAII wrapper
- Lock/unlock pairs (`pthread_mutex_lock`) → `std::lock_guard`

For each candidate: file, line range, current pattern, proposed RAII class.

### 2.2 Memory Optimization Candidates

Scan for:
- `g_malloc`/`g_free` → replace with `std::vector<uint8_t>` or `std::string`
- `g_strdup`/`g_free` → replace with `std::string`
- `g_strdup_printf` → replace with `std::string` + formatting
- `GList*` containers → replace with `std::vector` or `std::list`
- `GHashTable*` containers → replace with `std::unordered_map`
- Raw owning `char*` fields in structs → `std::string`
- Stack-allocatable structs currently heap-allocated

### 2.3 Storage/Binary Size Candidates

Scan for:
- Dead functions (defined but never called, or only called from tests)
- Unused `EXPORT_API` symbols that are safe to remove (ABI-verify first)
- Duplicate string literals that could be deduplicated
- Unnecessary exported symbols (hidden with `-fvisibility=hidden`)
- Large inline functions that should be out-of-line

## Step 3 — Test Scenario Generation

For each identified refactoring item (RAII wrappers, memory replacements,
storage reductions), generate test scenarios using the
`test-scenario-generator` skill.

Cover:
1. **Regression baseline** — existing behavior preserved after refactor
2. **Resource management** — RAII destruction verified (no leaks)
3. **Error paths** — destructor runs even when exceptions/early-returns occur
4. **Thread safety** — lock_guard holds through scope

Write scenarios to:
`../refactoring-plans/execution/tasks/notification/test_scenarios_raii_mem.md`

## Step 4 — Create Task List

Based on the analysis, define tasks in this order:

| Priority | Task ID | Scope | Description |
|----------|---------|-------|-------------|
| 1 | RAII-DB-001 | notification_db.cc | DbHandle, DbStatement, DbTransaction wrappers |
| 2 | RAII-FD-001 | notification_shared_file.cc, others | ScopedFd wrapper |
| 3 | MEM-GLIB-001 | all notification/*.cc | g_malloc/g_strdup → std::string/vector |
| 4 | MEM-GLIST-001 | notification_list.cc, notification_group.cc | GList → std::vector |
| 5 | MEM-GHASH-001 | any GHashTable usage | GHashTable → std::unordered_map |
| 6 | SIZE-DEAD-001 | all notification/*.cc | Dead code + -fvisibility=hidden |

For each task, specify:
- Exact files in `src/notification/src/` to modify
- LOC estimate
- GBS build tag (e.g., `GBS-ROOT-N8-RAII-DB`)
- Acceptance criteria: GBS build PASS + zero ctest regressions + ABI diff = 0

Write to: `../refactoring-plans/execution/tasks/notification/new_cpp_tasks.md`
(overwrite the existing file with updated content)

## Step 5 — Generate Per-Task Prompts

For EACH task, write/update a prompt file at `prompts/<TASK_ID>.md`.

Each prompt MUST include:
- Exact files to modify (only under `src/notification/src/`)
- RAII class design (for RAII tasks)
- Test scenarios to implement first (TDD order)
- Code style checklist: 2-space indent, K&R braces, 80-char max,
  omit braces for single-line bodies, Google naming
- Acceptance criteria

## Step 6 — Execute First Task: RAII-DB-001

After generating the plan, immediately execute RAII-DB-001:

1. Load `prompts/RAII-DB-001.md`
2. Write GTest tests first (red phase):
   - Test DbStatement RAII: finalizes on scope exit
   - Test DbTransaction commit path
   - Test DbTransaction rollback on early return
3. Implement RAII wrappers in `src/notification/src/notification_db.cc`
   and a new header (if needed, only in `src/notification/`)
4. Verify: `gbs build -A x86_64 --include-all -B /home/kimhyeonuk/GBS-ROOT-N8-RAII-DB`
5. Verify: zero ctest regressions
6. Verify: ABI diff = 0 (no new/removed public symbols)
7. Commit with message: `refactor(notification): RAII-DB-001 — DbStatement/DbTransaction wrappers`

## Code Style (MANDATORY for every line written)

```
- Google C++ Style Guide
- K&R braces (opening brace on same line)
- 2-space indent (NO tabs)
- Omit braces for single-line bodies
- Max 80 characters per line
- std::unique_ptr over raw owning pointers
- No new/delete in application code
```

## Reminder: notification-ex is OUT OF SCOPE

Any file path containing `notification-ex` must be ignored entirely.
If analysis tools surface notification-ex files, skip them without comment.
