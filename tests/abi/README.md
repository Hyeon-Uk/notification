# tests/abi/

ABI snapshot + check tooling for `libnotification.so`. Frozen in
task `01-tooling-baseline-audit` of the C → C++ refactor.

## Files

| File | Purpose |
|---|---|
| `snapshot.sh` | Capture symbols + struct layout from a `.so` |
| `struct-layout.sh` | Layout dumper used by `snapshot.sh` (pahole → objdump → gcc fallback) |
| `check-abi.sh` | Diff a current `.so` against a frozen baseline; exits non-zero on any regression |
| `test-check-abi.sh` | Self-test for `check-abi.sh` (synthetic .so, deliberate elision) |
| `snapshot-0.12.1.txt` | Frozen baseline symbol list (captured from gbs build of `Release version 0.12.1`) |
| `struct-layout-0.12.1.txt` | Frozen baseline struct layout |
| `whitelist.txt` | (optional) Symbol names allowed as pure additions during the refactor |
| `layout-whitelist.txt` | (optional) Regex patterns for layout-diff lines that don't constitute regression |

## Capture (one-time, per release)

```sh
# inside the gbs build root:
tests/abi/snapshot.sh \
    /path/to/built/libnotification.so \
    tests/abi/snapshot-0.12.1.txt \
    tests/abi/struct-layout-0.12.1.txt
```

The snapshot script writes a `# captured-from:` header line so
reviewers can audit which build the snapshot came from.

## Verify (every refactor task)

```sh
tests/abi/check-abi.sh \
    --baseline 0.12.1 \
    /path/to/current/libnotification.so
```

Exit codes:
- `0` — zero diff, or only whitelisted additions
- `1` — symbol regression (a baseline symbol disappeared, or an
  addition not on the whitelist appeared)
- `2` — struct-layout drift not whitelisted
- `3` — internal error (snapshot file missing, etc.)

The supervisor's exit checkpoint runs `check-abi.sh` against the
gbs-produced `.so` for every task. R-ABI-STABLE.

## Self-test

```sh
tests/abi/test-check-abi.sh
```

Builds two tiny synthetic `.so`s (one matching, one with a removed
symbol), runs `check-abi.sh` against both, asserts the matching one
exits 0 and the broken one exits non-zero. Run before relying on
`check-abi.sh`'s verdict.
