# notification — S7BUG Task State

| Item | Value |
|---|---|
| Branch | `tizen` |
| HEAD at task start | `d369e4e` (Stage 8 N8-C complete) |
| Current task | S7BUG: Stage 7 attempt + N7.5-D production bug follow-up |
| Working GBS-ROOT | `/home/kimhyeonuk/GBS-ROOT-S7BUG` |
| Concurrent build | `GBS-ROOT-N8A1` (left running by prior session, separate root, no mount conflict) |

## Task A: Stage 7 (tidlc Phase 2 + valgrind baseline)

Status: **external-dep-blocked**.

- `tidlc` not on host, only inside Tizen rootfs (`BuildRequires: tidl`).
- External Tizen `tidl` Phase 2 not merged — regenerated proxy would
  diff to zero against committed `notification_tidl_proxy.{h,c}`.
- `valgrind` not on host or in current `BuildRequires`. spec `%check`
  uses bare `ctest -V`. No baseline can be captured today.

See `logs/stage7_attempt.md` for full attempt log + future plan.

## Task B: N7.5-D production bug follow-up

Status: **completed**.

| File | Change |
|---|---|
| `src/notification/src/notification_noti.cc` | (1) Add `if (count == NULL) return INVALID_PARAMETER;` to `notification_noti_get_count`. (2) Add `if (noti == NULL) return INVALID_PARAMETER;` to `notification_noti_check_tag` and move `WARN("@@ check tag : %s", noti->tag)` to after the `noti->tag == NULL` guard. |
| `tests/unittests/src/test_notification_noti_internal_nullsafe.cc` | New TU, 4 TEST_F covering both NULL contracts. |
| `tests/unittests/src/test_notification_noti_internal.cc` | Replace the two "we cannot pin this — production crashes" comment blocks with a pointer to the new nullsafe TU. |

ABI: additive only, no exported symbol changed/removed.

## Task C: Build / verification — completed

Build root: `/home/kimhyeonuk/GBS-ROOT-S7BUG`
Log: `logs/gbs_S7BUG.log`
Result: **succeeded**, packages built (notification, notification-devel,
notification-ex, notification-ex-devel, notification-test-app,
notification-unittests + debuginfo/debugsource).

ctest 386 PASSED / 0 FAILED / 75 SKIPPED / 13 DISABLED out of
423 tests across 70 suites (73.5 s wall). The 4 new
`NotificationNotiNullSafeTest.*` cases all PASS.

## Commit

`a5932a8` — Fix two NULL-deref crashes in notification_noti
EXPORT_API surface (3 files, +190 / -10 LOC).

Local only — remote push deferred per task rule (S7BUG is a
mid-stage follow-up, not a stage-end push event).

## ABI baseline (N8C)

- `logs/abi_n8c_libnotification.txt` — 316 exported symbols.
- `logs/abi_n8c_libnotification-ex.txt` — 1017 exported symbols.

Post-build diff is captured against this baseline; expected delta is 0
removed / 0 changed.
