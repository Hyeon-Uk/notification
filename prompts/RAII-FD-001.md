# RAII-FD-001 — File Descriptor RAII Wrapper (ScopedFd)

## Context

After RAII-DB-001, raw POSIX file descriptors (open/close, fopen/fclose) remain
in 6 files. These raw fd patterns risk leaking descriptors on error paths.
This task introduces a minimal `ScopedFd` RAII wrapper and applies it across
all affected files.

**RAII-FD-001 depends on RAII-DB-001 completing successfully.**

## Acceptance Criteria

- [ ] `ScopedFd` class implemented in `src/notification/include/scoped_fd.hh`
- [ ] All unguarded raw `open()`/`close()` fd-owning patterns replaced by ScopedFd in 5 files
- [ ] `gbs build -A x86_64 --include-all` PASS
- [ ] `ctest --output-on-failure` regression = 0 vs RAII-DB-001 end baseline
- [ ] `nm --dynamic --defined-only libnotification.so | wc -l` = 316
- [ ] `valgrind --leak-check=full` fd leaks on error paths = 0

## GBS Build Tag

`GBS-ROOT-RAII-FD`

## Files Affected

| File | open/close hits | Pattern |
|------|----------------|---------|
| `src/notification/src/notification_noti.cc` | 49 | file read/write for shared files |
| `src/notification/src/notification_setting_service.cc` | 26 | config file access |
| `src/notification/src/notification_db.cc` | 11 | db file locking |
| `src/notification/src/notification_setting.cc` | 8 | settings file persistence |
| `src/notification/src/notification.cc` | 2 | notification file ops |

New header: `src/notification/include/scoped_fd.hh`

## TDD: Test Scenarios to Implement First (Red Phase)

Write in `tests/unittests/src/test_scoped_fd.cc` BEFORE implementing:

```cpp
TEST_F(ScopedFdTest, Destructor_ClosesOnce)
TEST_F(ScopedFdTest, InvalidFd_NoClose)
TEST_F(ScopedFdTest, Release_PreventClose)
TEST_F(ScopedFdTest, MoveConstruct_TransfersOwnership)
TEST_F(ScopedFdTest, MoveAssign_ClosesOldFd)
TEST_F(ScopedFdTest, Get_ReturnsRawFd)
TEST_F(ScopedFdTest, IsValid_NegativeFd_ReturnsFalse)
```

TDD process:
1. Write tests → build → ctest → confirm 7 FAILURES
2. Implement ScopedFd
3. Re-run ctest → 0 FAILURES, 0 regressions

## Implementation Specification

### Header: `src/notification/include/scoped_fd.hh`

```cpp
#pragma once
#include <unistd.h>

class ScopedFd {
 public:
  explicit ScopedFd(int fd = -1) noexcept : fd_(fd) {}
  ~ScopedFd() noexcept { if (fd_ >= 0) close(fd_); }

  ScopedFd(ScopedFd&& o) noexcept : fd_(o.Release()) {}
  ScopedFd& operator=(ScopedFd&& o) noexcept {
    if (this != &o) {
      if (fd_ >= 0) close(fd_);
      fd_ = o.Release();
    }
    return *this;
  }
  ScopedFd(const ScopedFd&) = delete;
  ScopedFd& operator=(const ScopedFd&) = delete;

  int Get() const noexcept { return fd_; }
  bool IsValid() const noexcept { return fd_ >= 0; }
  int Release() noexcept { int fd = fd_; fd_ = -1; return fd; }

 private:
  int fd_ = -1;
};
```

### Application pattern

Before:
```cpp
int fd = open(path, O_RDWR | O_CREAT, 0644);
if (fd < 0)
  return NOTIFICATION_ERROR_IO_ERROR;
// ... use fd ...
close(fd);
return NOTIFICATION_ERROR_NONE;
```

After:
```cpp
ScopedFd fd(open(path, O_RDWR | O_CREAT, 0644));
if (!fd.IsValid())
  return NOTIFICATION_ERROR_IO_ERROR;
// ... use fd.Get() ...
// close happens in ScopedFd destructor
return NOTIFICATION_ERROR_NONE;
```

### fopen/fclose pattern (if present)

If `fopen`/`fclose` pairs exist (FILE*), use a similar pattern:
```cpp
struct FileCloser { void operator()(FILE* f) const noexcept { fclose(f); } };
using ScopedFile = std::unique_ptr<FILE, FileCloser>;
ScopedFile f(fopen(path, "r"));
```

## Code Style Checklist

- [ ] K&R braces on same line
- [ ] 2-space indent (no tabs)
- [ ] Omit braces for single-line bodies
- [ ] Max 80 chars per line
- [ ] Class name: PascalCase (`ScopedFd`)
- [ ] Private member: trailing underscore (`fd_`)
- [ ] Destructor: `noexcept`
- [ ] No `new`/`delete` in application code

## Commit Strategy

1. `src/notification/include/scoped_fd.hh` (new header + tests)
2. `src/notification/src/notification_noti.cc`
3. `src/notification/src/notification_setting_service.cc`
4. `src/notification/src/notification_db.cc`
5. `src/notification/src/notification_setting.cc`
6. `src/notification/src/notification.cc`

Each commit: `gbs build PASS` + `ctest regression = 0` + `ABI nm diff = 0`

## ABI Verification

```bash
nm --dynamic --defined-only \
  /home/kimhyeonuk/GBS-ROOT-RAII-FD/local/BUILD/x86_64/RPMS/libnotification*.so \
  | awk '{print $3}' | sort > after.txt
diff baseline_316.txt after.txt
# Must be empty
```

## Next Task

After this task → **MEM-GLIB-001**
