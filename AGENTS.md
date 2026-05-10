# AGENTS.md - notification package

See full workspace guidance at `../agents/AGENTS.md`.

## Package: notification

This is the notification Tizen AppFW package undergoing C++ refactoring.

### Code Style (MANDATORY)
- Google C++ Style Guide
- K&R braces (opening brace on same line)
- 2-space indent (no tabs)
- Omit braces for single-line bodies
- Max 80 characters per line

### Build
```bash
gbs build -A x86_64 --include-all -B /home/kimhyeonuk/GBS-ROOT-<TAG>
```

### Refactoring Scope (MANDATORY)

**IN SCOPE:** `src/notification/src/` and `src/notification/include/` ONLY.

**OUT OF SCOPE:** `src/notification-ex/` — DO NOT analyze, plan, or modify
any file under `src/notification-ex/`.

### Current State (2026-05-10)
- Stage 8 FULLY COMPLETE — all `.c` files in `src/notification/src/` migrated
  to `.cc` (post-Stage-8 bug-fix commits applied through `812f07e`)
- C-to-C++ migration: DONE. No remaining `.c` migration tasks.
- Next work: RAII adoption, memory optimization, storage/binary size reduction
- Tests baseline: re-verify with GBS build before new tasks
- ABI: 316 dynamic public symbols (must remain stable)
