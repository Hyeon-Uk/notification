#!/usr/bin/env bash
# Capture an ABI snapshot of libnotification.so.
#
# Usage:
#   tests/abi/snapshot.sh <so-path> <out-symbols.txt> <out-layout.txt>
#
# Inputs:
#   $1 — path to libnotification.so (or a symlink chain ending at one)
#   $2 — destination file for the symbol snapshot
#        (output of `nm -D --defined-only --extern-only`, sorted)
#   $3 — destination file for the struct layout snapshot
#        (output of struct-layout.sh)
#
# The snapshot is the canonical record we diff against in
# tests/abi/check-abi.sh. Keep both outputs deterministic:
# stable sort, no timestamps, no host paths.

set -euo pipefail

SO_PATH="${1:?usage: $0 <so> <out-symbols> <out-layout>}"
OUT_SYMS="${2:?usage: $0 <so> <out-symbols> <out-layout>}"
OUT_LAYOUT="${3:?usage: $0 <so> <out-symbols> <out-layout>}"

if [[ ! -f "$SO_PATH" ]]; then
    echo "snapshot.sh: $SO_PATH not found" >&2
    exit 2
fi

# Symbols: defined, externally visible, function or data.
# Strip addresses + section identifiers; keep symbol-type letter and name.
nm -D --defined-only --extern-only "$SO_PATH" \
    | awk '{print $2, $3}' \
    | grep -vE '^[A-Za-z] (_init|_fini|__bss_start|_edata|_end|__cxa_finalize)$' \
    | LC_ALL=C sort \
    > "$OUT_SYMS.tmp"

# Header line records the source so reviewers can audit it later.
{
    echo "# libnotification ABI symbol snapshot"
    echo "# captured-from: $(readlink -f "$SO_PATH")"
    echo "# tool: nm -D --defined-only --extern-only"
    echo "# count: $(wc -l <"$OUT_SYMS.tmp")"
    echo "#"
    cat "$OUT_SYMS.tmp"
} > "$OUT_SYMS"
rm -f "$OUT_SYMS.tmp"

"$(dirname "$0")/struct-layout.sh" "$SO_PATH" "$OUT_LAYOUT"

echo "snapshot.sh: wrote $OUT_SYMS ($(grep -cv '^#' "$OUT_SYMS") symbols)"
echo "snapshot.sh: wrote $OUT_LAYOUT"
