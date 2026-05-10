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

### Current State (2026-05-10)
- Stage 8 FULLY COMPLETE — all 19 `.c` files migrated to `.cc` (N8-C: `d369e4e`)
- Post-N8-C bug-fix commits applied: `b2c390e`…`812f07e` (NULL-deref, leak, race, SQL injection)
- Next work: RAII adoption, memory optimization, storage/binary size reduction, Stage 7.5 coverage gates
- Tests baseline: 382 PASSED (N8-B.2 era — stale; re-verify before new tasks)
