# notification — smoke tests

Post-install BAT ("Build Acceptance Test") for the `notification`
package. The smoke binary exercises the most critical end-to-end
path through the freshly installed library: create a notification
handle, set + read back a title, free. Nothing more.

## Layout

- `smoke_main.c` — the BAT exerciser. Exits 0 on full success;
  documented non-zero exits per failure mode (see the comment
  block at the top of the file).
- `CMakeLists.txt` — builds `notification-smoketests` and
  installs it under `${LIB_INSTALL_DIR}/../bin/`.
- `run-smoketest.sh` — generated under `%install` from the
  notification.spec template; installed to
  `/usr/bin/tizen-smoketests/notification/run-smoketest.sh`.

## QA workflow (post-install on device / emulator)

```
sdb push notification-smoketests-*.rpm /tmp/
sdb shell rpm -ivh /tmp/notification-smoketests-*.rpm
sdb shell /usr/bin/tizen-smoketests/notification/run-smoketest.sh
echo $?   # must be 0
```

The driver script wraps the smoke binary with the project's
standard `setup` / `test_main` / `teardown` shape (matches
`run-unittest.sh` and `run-integtest.sh`).

## In-build BAT

Inside the GBS `%check` block, the smoke binary is exercised
directly from the build tree (no install step required). The
result is captured as `SMOKE_EXIT=` alongside `UNITTEST_EXIT=`
and `INTEGRATION_EXIT=` for the orchestrator's pre-release
test gate (R-RELEASE-TEST-GATE).

## Out of scope

- Functional coverage — that's units (`tests/unittests/`) and
  integration (`tests/integration/`).
- `notification-ex` (R-NOTIFICATION-EX-OUT-OF-SCOPE).
- DPM / aul / vconf interactions — the BAT must run cleanly
  whether or not those daemons are present.
