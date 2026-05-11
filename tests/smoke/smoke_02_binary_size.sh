#!/bin/bash
# smoke_02_binary_size.sh — verify .so size <= baseline + 5%

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
LIB_PATH=""

for candidate in \
  "${SCRIPT_DIR}/../../src/notification/libnotification.so" \
  "/usr/lib/libnotification.so" \
  "/usr/lib64/libnotification.so" \
  "$(find "${SCRIPT_DIR}/../.." -name 'libnotification.so' 2>/dev/null | head -1)"; do
  if [ -f "$candidate" ]; then
    LIB_PATH="$candidate"
    break
  fi
done

if [ -z "$LIB_PATH" ]; then
  echo "FAIL: libnotification.so not found"
  exit 1
fi

ACTUAL_SIZE=$(stat -c%s "$LIB_PATH")
ACTUAL_KB=$((ACTUAL_SIZE / 1024))

BASELINE_FILE="${SCRIPT_DIR}/.binary_size_baseline"

if [ -f "$BASELINE_FILE" ]; then
  BASELINE_KB=$(cat "$BASELINE_FILE")
else
  # First run: record the current size as the baseline
  echo "$ACTUAL_KB" > "$BASELINE_FILE"
  BASELINE_KB=$ACTUAL_KB
  echo "INFO: No baseline found, recording current size as baseline: ${ACTUAL_KB}KB"
fi

MAX_KB=$((BASELINE_KB * 105 / 100))

echo "Library size: ${ACTUAL_KB}KB (baseline: ${BASELINE_KB}KB, max allowed: ${MAX_KB}KB)"

if [ $ACTUAL_KB -gt $MAX_KB ]; then
  echo "FAIL: Library too large (${ACTUAL_KB}KB > ${MAX_KB}KB, baseline ${BASELINE_KB}KB)"
  exit 1
fi

echo "PASS: Size within limits"
exit 0
