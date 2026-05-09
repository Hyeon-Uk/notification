#!/usr/bin/env bash
# Self-test for check-abi.sh.
#
# Builds a tiny synthetic .so with two exported C symbols, captures
# a snapshot from it, then re-runs check-abi.sh with the snapshot
# elided (one symbol artificially removed from the current build)
# and asserts the script exits 1.
#
# This is the test-the-test described in the task acceptance handoff.

set -euo pipefail

HERE="$(dirname "$(readlink -f "$0")")"
TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT

cat > "$TMP/synth.c" <<'EOF'
int notification_synth_alpha(void) { return 1; }
int notification_synth_beta(void)  { return 2; }
EOF

gcc -shared -fPIC -o "$TMP/libsynth.so" "$TMP/synth.c"

# Capture as the synthetic baseline.
"$HERE/snapshot.sh" "$TMP/libsynth.so" \
    "$HERE/snapshot-synth.txt" \
    "$HERE/struct-layout-synth.txt" >/dev/null

# Build a "current" library that's missing one symbol.
cat > "$TMP/synth_break.c" <<'EOF'
int notification_synth_alpha(void) { return 1; }
EOF
gcc -shared -fPIC -o "$TMP/libsynth_broken.so" "$TMP/synth_break.c"

# Run 1 — unchanged baseline against itself: must exit 0.
if ! "$HERE/check-abi.sh" --baseline synth "$TMP/libsynth.so" >/dev/null; then
    echo "test-check-abi: FAIL (unchanged build flagged regression)" >&2
    rm -f "$HERE/snapshot-synth.txt" "$HERE/struct-layout-synth.txt"
    exit 1
fi

# Run 2 — broken build vs baseline: must exit non-zero.
if "$HERE/check-abi.sh" --baseline synth "$TMP/libsynth_broken.so" >/dev/null 2>&1; then
    echo "test-check-abi: FAIL (broken build NOT flagged)" >&2
    rm -f "$HERE/snapshot-synth.txt" "$HERE/struct-layout-synth.txt"
    exit 1
fi

# Cleanup the synthetic snapshot files (don't leak them into the repo).
rm -f "$HERE/snapshot-synth.txt" "$HERE/struct-layout-synth.txt"

echo "test-check-abi: PASS"
