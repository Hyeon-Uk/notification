#!/usr/bin/env bash
# Diff a current libnotification.so against the frozen baseline
# captured by snapshot.sh. Exits non-zero on any regression.
#
# Usage:
#   tests/abi/check-abi.sh [--baseline <tag>] <so-path>
#
#   --baseline <tag>  Suffix used in the snapshot filenames.
#                     Defaults to "0.12.1". Looks up
#                     tests/abi/snapshot-<tag>.txt and
#                     tests/abi/struct-layout-<tag>.txt.
#
# Exit codes:
#   0  zero diff (or only whitelisted additions)
#   1  symbol regression (any line removed from the baseline)
#   2  layout regression (any byte-size or member offset changed)
#   3  internal error (snapshot file missing, etc.)

set -euo pipefail

BASE_TAG="0.12.1"
WHITELIST="$(dirname "$0")/whitelist.txt"

while [[ $# -gt 0 ]]; do
    case "$1" in
        -b|--baseline) BASE_TAG="$2"; shift 2 ;;
        -h|--help)
            sed -n '2,/^$/p' "$0" | sed 's/^# //;s/^#//'
            exit 0
            ;;
        --) shift; break ;;
        -*) echo "check-abi.sh: unknown flag $1" >&2; exit 3 ;;
        *) break ;;
    esac
done

SO_PATH="${1:?usage: $0 [--baseline <tag>] <so>}"
ABI_DIR="$(dirname "$0")"
BASE_SYMS="$ABI_DIR/snapshot-$BASE_TAG.txt"
BASE_LAYOUT="$ABI_DIR/struct-layout-$BASE_TAG.txt"

[[ -f "$BASE_SYMS" ]] || { echo "check-abi.sh: $BASE_SYMS not found" >&2; exit 3; }
[[ -f "$BASE_LAYOUT" ]] || { echo "check-abi.sh: $BASE_LAYOUT not found" >&2; exit 3; }

CUR_DIR="$(mktemp -d)"
trap 'rm -rf "$CUR_DIR"' EXIT
"$ABI_DIR/snapshot.sh" "$SO_PATH" \
    "$CUR_DIR/cur-syms.txt" "$CUR_DIR/cur-layout.txt" >/dev/null

# Symbol diff: anything REMOVED is a regression. Anything ADDED is OK
# only if it appears in tests/abi/whitelist.txt (one symbol-name per line).
diff_syms="$(diff <(grep -v '^#' "$BASE_SYMS") <(grep -v '^#' "$CUR_DIR/cur-syms.txt") || true)"

removed="$(echo "$diff_syms" | sed -n 's/^< //p' | grep -vE '^[[:space:]]*$' || true)"
added="$(echo "$diff_syms" | sed -n 's/^> //p' | grep -vE '^[[:space:]]*$' || true)"

if [[ -n "$removed" ]]; then
    echo "check-abi.sh: REGRESSION — symbols removed from $BASE_TAG baseline:"
    echo "$removed" | sed 's/^/    /'
    exit 1
fi

if [[ -n "$added" ]]; then
    if [[ -f "$WHITELIST" ]]; then
        not_listed="$(echo "$added" \
            | awk '{print $2}' \
            | grep -vxFf "$WHITELIST" \
            || true)"
    else
        not_listed="$added"
    fi
    if [[ -n "$not_listed" ]]; then
        echo "check-abi.sh: ADDITIONS not on whitelist:"
        echo "$not_listed" | sed 's/^/    /'
        exit 1
    fi
fi

# Layout diff: any difference is a regression unless explicitly
# whitelisted in tests/abi/layout-whitelist.txt.
LAYOUT_WL="$ABI_DIR/layout-whitelist.txt"
if ! diff -q "$BASE_LAYOUT" "$CUR_DIR/cur-layout.txt" >/dev/null; then
    if [[ -f "$LAYOUT_WL" ]]; then
        # Strip whitelisted regex matches from the diff before deciding.
        non_wl_diff="$(diff "$BASE_LAYOUT" "$CUR_DIR/cur-layout.txt" \
            | grep -vEf "$LAYOUT_WL" || true)"
        if [[ -z "$non_wl_diff" ]]; then
            echo "check-abi.sh: layout drift, all matches whitelisted — OK"
            exit 0
        fi
    fi
    echo "check-abi.sh: REGRESSION — struct layout drift:"
    diff "$BASE_LAYOUT" "$CUR_DIR/cur-layout.txt" | head -40 | sed 's/^/    /'
    exit 2
fi

echo "check-abi.sh: zero diff against $BASE_TAG baseline"
exit 0
