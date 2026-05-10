# notification — integration tests

This directory hosts the integration-test suite for the
`notification` package. Unlike the `unittests/` suite, which mocks
out every external dependency, the integration suite drives the
real ones: real `sqlite3` against a per-test tmpdir database, the
real `glib` mainloop, an in-process `rpc-port` loopback server,
and (where the build sandbox permits) real `vconf` and `aul`.

## Layout

- `include/integ_harness.hh` — primitive harness types
  (`integ::Harness`, `integ::TmpDb`, `integ::ScopedEnv`,
  `integ::MainloopTicker`).
- `src/integ_harness.cc` — implementations.
- `src/test_main.cc` — gtest entry point and harness-boot
  sentinel tests.
- Per-cluster test sources (`test_db.cc`, `test_noti.cc`,
  `test_ipc_loopback.cc`, …) are added one work-unit commit at
  a time and each owns one symbol cluster from the master
  scenario inventory in
  `refactoring-plans/05_logs/19a-integ-build-out.md`.

## Test-only environment

The harness sets `NOTIFICATION_INTEGTEST_DB_PATH` from
`integ::TmpDb`'s constructor and clears it on destruction. The
production library reads this env var from
`_notification_db_resolve_path()` in
`src/notification/src/notification_db.c` and falls back to the
default `tzplatform_mkpath(TZ_SYS_DB, ".notification.db")` when
the variable is unset or empty. The variable is **never** set in
production; setting it manually outside an integration-test
process is unsupported.

## Environment gating

Tests that require a dependency the build sandbox may not provide
(real `vconf`, real `aul`, on-device DBus) probe availability
through `integ::Harness::*Available()` and call `GTEST_SKIP()`
when the probe returns false, rather than silently passing or
hard-failing. Skipped tests are listed in the gtest summary so
the tester can audit them.

## Running

The suite is driven by GBS as part of `%check`:

```
gbs build -A x86_64 --include-all
```

`%check` captures `INTEGRATION_EXIT=` alongside `UNITTEST_EXIT=`
in the `.build.log`. Both must be 0 for the integ contract to
hold.

The suite is also installed as a separate RPM,
`notification-integtests`, with a `run-integtest.sh` driver
under `/usr/bin/tizen-integtests/notification/`. QA runs it
post-install; the integ build is the same binary in either path.
