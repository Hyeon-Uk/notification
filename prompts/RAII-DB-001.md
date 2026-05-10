# RAII-DB-001 — SQLite RAII Wrappers (DbStatement, DbConnection, DbTransaction)

## Context

Stage 8 (C→C++) is complete. Zero smart pointers exist in the codebase.
sqlite3_stmt* and sqlite3* are raw owning pointers throughout 5 files.
This task introduces three RAII wrapper classes to eliminate manual
sqlite3_finalize / sqlite3_close_v2 calls and to ensure BEGIN/COMMIT/ROLLBACK
balance under all code paths including exceptions.

**Prerequisite: COVERAGE-VERIFY-001 must be complete.**

## Acceptance Criteria

- [ ] `DbStatement`, `DbConnection`, `DbTransaction` classes in new header
- [ ] Zero raw `sqlite3_stmt*` owning patterns in all 5 files
- [ ] Zero unguarded `sqlite3*` owning pointers in all 5 files
- [ ] Zero unmatched BEGIN without matching COMMIT/ROLLBACK in all 5 files
- [ ] `gbs build -A x86_64 --include-all` PASS
- [ ] `ctest --output-on-failure` regression = 0 vs COVERAGE-VERIFY-001 baseline
- [ ] `nm --dynamic --defined-only libnotification.so | wc -l` = 316
- [ ] `valgrind --leak-check=full` definitely-lost = 0 for DB exercise paths

## GBS Build Tag

`GBS-ROOT-RAII-DB`

## Files Affected

| File | sqlite3 hits | Pattern |
|------|-------------|---------|
| `src/notification/src/notification_db.cc` | 82 | db open/close, prepare/step/finalize, transactions |
| `src/notification/src/notification_noti.cc` | 92 sqlite3 of 316 total | prepare/step/finalize per query |
| `src/notification/src/notification_group.cc` | 22 | prepare/step/finalize |
| `src/notification/src/notification_setting.cc` | 51 | prepare/step/finalize, transactions |
| `src/notification/src/notification_setting_service.cc` | 61 | db queries |

New header: `src/notification/include/notification_db_raii.hh`

## TDD: Test Scenarios to Implement First (Red Phase)

Write these tests in `tests/unittests/src/test_notification_db_raii.cc` BEFORE
implementing the wrapper classes. Confirm all tests FAIL with current code.

```cpp
TEST_F(DbRaiiTest, DbStatement_Destructor_FinalizesStmt)
TEST_F(DbRaiiTest, DbStatement_Destructor_NullSafe)
TEST_F(DbRaiiTest, DbStatement_Step_ReturnsRow)
TEST_F(DbRaiiTest, DbStatement_Step_Done)
TEST_F(DbRaiiTest, DbStatement_Reset_AllowsReuse)
TEST_F(DbRaiiTest, DbStatement_MoveConstruct_TransfersOwnership)
TEST_F(DbRaiiTest, DbTransaction_Commit_IssuesCommit)
TEST_F(DbRaiiTest, DbTransaction_NoCommit_RollsBack)
TEST_F(DbRaiiTest, DbTransaction_Exception_RollsBack)
TEST_F(DbRaiiTest, DbTransaction_CommitFailure_Handled)
```

TDD process:
1. Write tests → `gbs build` → `ctest` → confirm 10+ new FAILURES
2. Implement wrapper classes
3. Re-run `ctest` → confirm 0 FAILURES, 0 regressions

## Implementation Specification

### Header: `src/notification/include/notification_db_raii.hh`

DbStatement — wraps sqlite3_stmt*:
- Constructor takes sqlite3_stmt* (result of sqlite3_prepare_v2)
- Destructor calls sqlite3_finalize (noexcept, null-safe)
- Non-copyable, moveable (move transfers ownership, nullifies source)
- Methods: Step(), Reset(), Get(), Release()
- Template GetColumn(int col) for int/text/int64 specializations

DbConnection — wraps sqlite3*:
- Constructor takes sqlite3* (result of sqlite3_open_v2)
- Destructor calls sqlite3_close_v2 (noexcept)
- Non-copyable, non-moveable
- Method: Get() returns raw sqlite3*

DbTransaction — wraps BEGIN/COMMIT/ROLLBACK:
- Constructor takes sqlite3*, issues BEGIN immediately
- Destructor issues ROLLBACK if Commit() was not called (noexcept)
- Non-copyable, non-moveable
- Method: Commit() → issues COMMIT SQL, returns SQLITE_OK on success,
  sets committed_ flag

### Exception safety at TIDL/RPC boundaries

Any function in `notification_tidl.cc` or `notification_internal_tidl.cc`
that calls into DB code MUST wrap in:
```cpp
try {
  // ... db code using RAII wrappers ...
} catch (...) {
  return NOTIFICATION_ERROR_IO_ERROR;
}
```

C++ exceptions MUST NOT escape extern "C" function boundaries.

### Application pattern (before → after)

Before:
```cpp
sqlite3_stmt* stmt = nullptr;
int ret = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
if (ret != SQLITE_OK) return NOTIFICATION_ERROR_FROM_SQLITE(ret);
// use stmt ...
sqlite3_finalize(stmt);
return NOTIFICATION_ERROR_NONE;
```

After:
```cpp
sqlite3_stmt* raw = nullptr;
int ret = sqlite3_prepare_v2(db, sql, -1, &raw, nullptr);
if (ret != SQLITE_OK) return NOTIFICATION_ERROR_FROM_SQLITE(ret);
DbStatement stmt(raw);
// use stmt.Step(), stmt.GetColumn<int>(0), etc.
// finalize happens automatically in DbStatement destructor
return NOTIFICATION_ERROR_NONE;
```

## Code Style Checklist

- [ ] K&R braces: opening brace on same line
- [ ] 2-space indent (no tabs)
- [ ] Omit braces for single-line bodies
- [ ] Max 80 chars per line
- [ ] Class names: PascalCase (DbStatement, DbTransaction, DbConnection)
- [ ] Private members: trailing underscore (stmt_, db_, committed_)
- [ ] Destructors: noexcept
- [ ] No new/delete in application code
- [ ] No std::shared_ptr (use std::unique_ptr / RAII classes only)

## Commit Strategy

6 commits (each independently verifiable):
1. `src/notification/include/notification_db_raii.hh` + test file
2. `src/notification/src/notification_db.cc`
3. `src/notification/src/notification_noti.cc`
4. `src/notification/src/notification_group.cc`
5. `src/notification/src/notification_setting.cc`
6. `src/notification/src/notification_setting_service.cc`

Per commit: `gbs build PASS` + `ctest regression = 0` + `ABI nm diff = 0`

## ABI Verification (per commit)

```bash
nm --dynamic --defined-only \
  /home/kimhyeonuk/GBS-ROOT-RAII-DB/local/BUILD/x86_64/RPMS/libnotification*.so \
  | awk '{print $3}' | sort > after.txt
diff baseline_316.txt after.txt
# Must be empty (0 lines of diff)
```

## Next Task

After this task → **RAII-FD-001**
