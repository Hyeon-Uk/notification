# Stage 7 Attempt Log (tidlc Phase 2 + valgrind baseline)

Date: 2026-04-28
Branch: `tizen` @ `d369e4e` (Stage 8 N8-C complete)
Owner: Stage 7 / N7.5-D follow-up agent
Scope: ROADMAP_v3 Stage 7 — tidlc regeneration + valgrind leak baseline

## Summary

| Item | Outcome |
|---|---|
| 1. tidlc regeneration | **Blocked — tool absent on host & external Phase 2 tidl not merged** |
| 2. valgrind baseline (notification-test-app / notification-unittests) | **Blocked — valgrind absent on host; spec ctest path does not gate on valgrind** |
| Net Stage 7 status | `external-dep-blocked` (no usable diff to capture) |

## 1. tidlc regeneration attempt

### Tool inventory (host)

```
$ command -v tidlc      ; echo $?
                        ; 1
$ command -v valgrind   ; echo $?
                        ; 1
```

Neither `tidlc` nor `valgrind` is installed on the build host. Both are
provided exclusively by the Tizen rootfs inside `gbs build` (see
`packaging/notification.spec`: `BuildRequires: tidl`). They are not on
any apt / dnf / pip channel for Ubuntu 22.04 host. Running
`src/tidl/prebuild.sh` directly from the host therefore aborts with
`tidlc: command not found`.

### Inside the gbs chroot (indirect attempt)

`prebuild.sh` is invoked automatically as part of the `%build` stage in
`packaging/notification.spec` whenever the spec line that references it
is uncommented; in current `tizen` HEAD the spec does NOT call the
script (the generated `notification_tidl_proxy.{h,c}` are committed in
tree under `src/notification/{include,src}/`). Stage 7 of ROADMAP_v3
plans to add a Phase-2 service-side proxy block to `noti_service.tidl`
once external Tizen `tidl` Phase 2 lands. Against current upstream
`tidl` (Phase 1 only) tidlc emits identical output to what is already
checked in — so even if we ran the script in-chroot, the diff would be
zero and there is no "baseline" to capture.

### Decision

Defer `prebuild.sh` execution until external `tidl` Phase 2 is merged
upstream. At that point ROADMAP_v3 Stage 7 will:

  1. Run `prebuild.sh` (in chroot via a one-shot
     `gbs chroot --root <root>` session, or temporarily uncomment the
     `prebuild.sh` invocation in spec `%prep`).
  2. Capture the resulting diff in
     `src/notification/{include/notification_tidl_proxy.h,
     src/notification_tidl_proxy.c}`.
  3. Verify ABI is additive (new RPC verbs only — no removal /
     signature change of existing exported entry points).
  4. Re-run unittests + integration tests to confirm no regression.

No changes are made to the auto-generated files in this commit, in
keeping with project rule "TIDL 자동생성 변경 금지".

## 2. valgrind baseline attempt

### Inventory

`valgrind` is not present on host (see §1). The `%check` block of
`packaging/notification.spec` runs:

```
%check
export LD_LIBRARY_PATH="../../src/notification:../../src/notification-ex"
ctest -V
```

`ctest -V` is invoked **without** valgrind. The Tizen rootfs used by
gbs (unified / standard repo) does not include valgrind in the default
build deps either, so adding `valgrind --leak-check=full` to `%check`
would require an additional `BuildRequires: valgrind` and may pull in
glibc-debuginfo for line-accurate suppression files — out of scope for
N7.5-D follow-up.

### Targets considered

| Binary | Suitable for valgrind? | Notes |
|---|---|---|
| `notification-unittests` | Partial — uses `--wrap=malloc/free/realloc/calloc` link option, which valgrind treats as "fishy" but tolerates with `--soname-synonyms=somalloc=NONE`. Heavy gmock teardown emits ~200 "still reachable" warnings inherent to gtest globals. | OK as a **definitely-lost** baseline; reachable counts are noisy. |
| `notification-test-app` | Yes — pure GMainLoop driver. Cleanest leak target. | Requires a TIDL daemon on `bus_name=org.tizen.notification` to register; otherwise loop runs but exercises only the dbus-lazy-init path. |
| `notification-tidl-app` (N4-T1) | Same as above. | Same daemon dependency. |

### Decision

Stage 7 will adopt the following baseline plan once valgrind is
available on the gbs unified standard rootfs:

  1. Add `valgrind` to `BuildRequires` (gated `%if 0%{?leakcheck:1}`).
  2. Run `valgrind --leak-check=full --show-leak-kinds=definite
       --error-exitcode=2 --suppressions=tools/leakcheck/notification.supp
       ./notification-unittests --gtest_filter=-*Death*` in `%check`.
  3. Capture log to `tools/leakcheck/baseline.log`, gate the "definitely
     lost" delta to 0 vs. baseline.

This work is deferred — environment cannot host the run today.

## 3. valgrind-free leak posture (current commit)

For completeness, the `--wrap=malloc/free/realloc/calloc` hooks already
present in `tests/unittests/CMakeLists.txt` (N0-T1 alloc counter +
`PerfAware` / `LeakCheck` gtest fixtures) provide a coarse net-alloc
delta per test. `make test` already reports any test that net-allocates
without freeing under the existing CI gate. This is the de-facto leak
baseline until valgrind comes online.

## 4. N7.5-D production bug follow-up (this commit)

Independent of Stage 7 deferral, two production NULL-deref crashes
documented in test_notification_noti_internal.cc as known untestable
sites have been fixed in this commit:

  - `notification_noti_get_count(..., NULL count, ...)` — added explicit
    NULL out-arg guard (1 line).
  - `notification_noti_check_tag(NULL noti)` — added NULL handle guard
    (1 line) and moved the `WARN("@@ check tag : %s", noti->tag)` log
    statement to AFTER the `noti->tag == NULL` branch (was reading
    `noti->tag` before the guard, double UB on `noti == NULL`).

New regression tests:
`tests/unittests/src/test_notification_noti_internal_nullsafe.cc`
(4 TEST_F: GetCount NULL count x2, CheckTag NULL handle, CheckTag valid
handle / NULL tag).

ABI: additive only — no exported symbol added or removed; signatures
unchanged. Existing return contract (INVALID_PARAMETER on bad input)
matches the rest of the EXPORT_API surface for both functions.
