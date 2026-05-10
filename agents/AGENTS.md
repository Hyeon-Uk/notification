# AGENTS.md - Tizen AppFW Refactoring Workspace

## Project Overview

This workspace contains the systematic C++ refactoring of two Tizen AppFW packages:
- **notification** (`../notification/`) - C to C++ migration + memory/storage optimization
- **data-provider-master** (`../data-provider-master/`) - same goals, runs after notification completes

Refactoring goals (priority order):
1. **Memory optimization** - reduce heap allocations, use stack/RAII, eliminate leaks
2. **Storage optimization** - minimize on-disk footprint, reduce binary size
3. **C++ migration** - `.c` to `.cc`, `extern "C"` boundaries, RAII resource management
4. **RAII compliance** - replace manual resource management with RAII wrappers

## C++ Code Style Rules (MANDATORY)

All C++ code written or modified in this workspace MUST follow these rules:

### Base Style
- **Google C++ Style Guide** as the base standard

### Brace Style: K&R
```cpp
// CORRECT - K&R style
if (condition) {
  do_something();
}

// WRONG - Allman style
if (condition)
{
  do_something();
}
```

### Indentation
- **2 spaces** per indent level (NO tabs)

### Single-Line Body: Omit Braces
```cpp
// CORRECT - omit braces for single-line body
if (ptr == nullptr)
  return NOTIFICATION_ERROR_INVALID_PARAMETER;

for (auto& item : list)
  item.Reset();

// WRONG - unnecessary braces
if (ptr == nullptr) {
  return NOTIFICATION_ERROR_INVALID_PARAMETER;
}
```

### Line Length
- **Maximum 80 characters** per line
- Break long function signatures at parameters:
```cpp
// CORRECT
int NotificationDb::Insert(
    const std::string& app_id,
    notification_type_e type,
    int *priv_id) {
```

### Naming Conventions (Google style)
- Classes: `PascalCase`
- Functions/methods: `PascalCase`
- Variables: `snake_case`
- Constants/enums: `kCamelCase`
- Private members: `snake_case_` (trailing underscore)
- Files: `snake_case.cc` / `snake_case.h`

### RAII Patterns (preferred)
```cpp
// PREFERRED - RAII wrapper
class DbTransaction {
 public:
  explicit DbTransaction(sqlite3* db) : db_(db) {
    sqlite3_exec(db_, "BEGIN", nullptr, nullptr, nullptr);
  }
  ~DbTransaction() {
    if (!committed_)
      sqlite3_exec(db_, "ROLLBACK", nullptr, nullptr, nullptr);
  }
  void Commit() {
    sqlite3_exec(db_, "COMMIT", nullptr, nullptr, nullptr);
    committed_ = true;
  }
 private:
  sqlite3* db_;
  bool committed_ = false;
};
```

### Smart Pointers
- Prefer `std::unique_ptr` over raw owning pointers
- Use `std::shared_ptr` only when shared ownership is genuinely needed
- Never use `new`/`delete` directly in application code

### Memory Allocation
- Prefer stack allocation over heap when size is bounded
- Use `std::vector`/`std::string` over `g_malloc`/`g_strdup`
- Prefer `std::array<T, N>` for fixed-size arrays

## Build System

```bash
# Build (x86_64)
gbs build -A x86_64 --include-all -B /home/kimhyeonuk/GBS-ROOT-<TAG>

# Run tests inside GBS chroot
ctest --output-on-failure -j$(nproc)
```

## Test Requirements

Every task MUST follow TDD:
1. Write failing tests (red)
2. Verify tests fail as expected
3. Implement/refactor
4. All tests pass (green)
5. Zero regression in pre-existing tests

Coverage gates:
- Line coverage >= 80%
- Branch coverage >= 70%

## Workflow Skills

Use these skills in order for each task:
1. `test-scenario-generator` - generate test scenarios before writing code
2. `tizen-refactoring` - orchestrate the full pipeline
3. `architect` - design RAII wrappers, class boundaries
4. `developer` - implement changes
5. `tester` - verify GBS build + ctest
6. `reviewer` - code review for style compliance
7. `committer` - commit with proper message

## Reference Documents

- Refactoring plans: `../refactoring-plans/`
- Current roadmap: `../refactoring-plans/execution/ROADMAP_v3.md`
- Process rules: `../refactoring-plans/REFACTORING-PROCESS-v6-amendment.md`
- Progress state: `../refactoring-plans/execution/progress/STATE.md`
- Task files: `../refactoring-plans/execution/tasks/`

## Current State (as of 2026-05-10)

### notification
- Stage 8 (C to C++): N8-B.2 completed (commit `069df7b`)
- Next: N8-C - TIDL wrapper files (`notification_tidl.c` + `notification_internal_tidl.c`, ~3,218 LOC)
- Tests: 382 PASSED, 37 SKIPPED, 15 DISABLED
- ABI: 316 dynamic public symbols (stable)

### data-provider-master
- Status: pending (starts after notification completes)

## GBS Root Tags

Use incremental GBS root tags:
- `GBS-ROOT-N8C` for N8-C task (TIDL wrapper migration)
- `GBS-ROOT-N8-RAII-<module>` for new RAII tasks
- `GBS-ROOT-OPT-<module>` for optimization tasks
