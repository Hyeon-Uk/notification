# N8 notification memory-leak audit (pass 1)

Follow-up to the NULL-safe audit (a5932a8 + 3f51314). Different bug class,
same TDD methodology — enumerate allocation sites with grep, walk every
return / goto / fall-through path, write a TDD-red test for any real bug,
land minimal patch, watch test go green.

Scope: the 17 source files under `src/notification/src/` carrying
EXPORT_API functions (301 entry points total).

## Per-file findings

| file | EXPORT_API count | leaks found | notes |
|---|---|---|---|
| notification.cc | 60 | 0 | set/get matrix is uniformly free-before-strdup; `_notification_create` cleanup label good |
| notification_bundle_util.cc | 7 | 0 | recently migrated to C++; heap pairing audited |
| notification_context.cc | 4 | 0 | calloc + populate + transfer-or-free on error; clean |
| notification_db.cc | 7 | 1 | `__recover_corrupted_db` double-close + leak — fixed |
| notification_db_query.h | 0 | 0 | header-only |
| notification_error.cc | 1 | 0 | string LUT, no allocation |
| notification_group.cc | 2 | 0 | sqlite cleanup paired everywhere |
| notification_internal.cc | 80 | 0 | OK; `register_changed_cb_for_uid` has init/fini imbalance on the rare malloc-after-init failure path but `notification_tidl_monitor_init` is idempotent so this is not a leak |
| notification_internal_tidl.cc | 13 | 0 | empty-bundle is freed on every exit; OK |
| notification_ipc.cc | 2 | 1 | `make_noti_from_gvariant` leaked ~15 fields on re-entry — fixed |
| notification_list.cc | 12 | 0 | linked-list helpers; transfer of ownership clean |
| notification_noti.cc | 25 | 0 | sqlite cleanup labels + tmp/info-on-error free are paired throughout; goto err discipline holds |
| notification_ongoing.cc | 5 | 0 | trivial setters |
| notification_setting.cc | 28 | 0 | all malloc'd cb_info objects either reach the hash table (owned) or are freed before return on the dup-callback path; the `notification_tidl_monitor_init` init/fini imbalance is again non-leaking due to idempotency |
| notification_setting_service.cc | 18 | 0 | sqlite_get_table + malloc paths use uniform out: cleanup |
| notification_shared_file.cc | 24 | 0 | private sharing req_data + uid_info both reach a list before any error return; on `__set_sharing_for_new_target` failure the local `new_file_list` is freed |
| notification_status.cc | 5 | 0 | dbus connection lifecycle paired with monitor_message_cb_unset |
| notification_tidl.cc | 0 | 0 | no EXPORT_API; internal helpers only |
| notification_viewer.cc | 6 | 0 | viewer slot strdup guarded against double-init; iniparser_freedict on every exit |
| shared_file_common.cc | 2 | 0 | path canonicalisation helpers; freed on error |

**Total: 2 real leaks across 2 files. Single commit per task contract.**

## Fixes (commit 5764fc5)

### Bug 1: `notification_ipc_make_noti_from_gvariant` field leak

`notification_ipc.cc:373-568` — decoder ran free-before-overwrite for
`pkg_id` / `caller_app_id` / `domain` / `dir` only, leaving 11 strdup'd
char* fields and 12 owned `bundle *` fields silently leaking on re-entry.
Centralised the contract in two macros (`_ASSIGN_OWNED_STR` /
`_ASSIGN_OWNED_BUNDLE`) so every field gets the same free-then-assign
treatment.

### Bug 2: `notification_db_init` corrupted-DB recovery double-close

`notification_db.cc:102-138` and `:208-216` — `__recover_corrupted_db`
took the `sqlite3 *` by value, closed it, opened a fresh handle into the
SAME local — leaking the new handle (caller's pointer unchanged, freed
pointer ⊥ accessible) — and then the caller's epilogue ran
`sqlite3_close()` on the freed pointer, hitting `SQLITE_MISUSE` /
use-after-free. Converted the parameter to `sqlite3 **` and NULL the
caller's pointer after the swap, so the epilogue is a safe no-op.

## Tests

`tests/unittests/src/test_notification_memleak_audit.cc` (3 active tests):

- `MakeNotiFromGVariant_DoubleDecode_NoStaleFields` — encode two distinct
  payloads, decode both into the same handle, assert fields hold the
  SECOND payload's values (proves the first decode's strdup'd allocs
  were freed in-place, not orphaned).
- `MakeNotiFromGVariant_StressLoop_NoCrash` — 64 iterations of decode-
  into-same-handle. Pre-fix valgrind / asan would surface; post-fix
  loop is O(1) heap. (Cross-library --wrap=malloc cannot observe libc
  free()s inside the .so, see test_notification_paths_extra.cc prior
  art — so this is a behavioural / no-crash pin, not an alloc-counter
  pin.)
- `DbInit_IdempotentReinit` — call `notification_db_init()` four times
  in a row. Pre-fix the corruption-recovery branch would hit
  `sqlite3_close()` on an already-closed pointer; post-fix the loop
  runs cleanly. SUCCEED() reach is the success signal.

The fourth case (`DISABLED_DbInit_RecoveryPath_NoDoubleClose`) is a
DISABLED placeholder for the recovery-hook follow-up; deterministic
corruption injection requires a private hook that is out of scope here.

## Verification

- `gbs build -A x86_64 --include-all -B /home/kimhyeonuk/GBS-ROOT-NOTIF-MEMLEAK` — **PASS**
- ctest: **401 PASSED + 75 SKIPPED + 15 DISABLED** (baseline 398/75/15;
  +3 new memleak tests + 1 new disabled placeholder)
- ABI: `nm --dynamic --defined-only` symbol-name set diff = 0 (316
  baseline, 316 new)
- No regressions

## Audit gaps

- The `_dup_string()` empty-input → NULL behaviour means an empty
  payload now NULLs a previously-set field via the new free-before-
  assign macro. This matches the legacy semantics for the four fields
  that were already free-before-strdup; it is a NEW behaviour for the
  remaining fields, and it is the correct memory-safety contract. If
  a downstream caller relied on "empty payload preserves prior value"
  it would observe a difference, but that semantics was non-uniform
  (and undocumented) in the old code anyway.
- `notification_register_*` and `notification_register_system_setting_dnd_changed_cb_for_uid`
  have init/fini imbalances on malloc-failure rollback paths. These are
  NOT leaks today because `notification_tidl_monitor_init` is idempotent
  with internal lazy-init guards, so an unmatched init call adds no
  state. Flagged here for a future "monitor refcount" cleanup.
