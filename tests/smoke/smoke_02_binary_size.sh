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

ABSOLUTE_MAX_KB=512

BASELINE_FILE="${SCRIPT_DIR}/.binary_size_baseline"

if [ -f "$BASELINE_FILE" ]; then
  BASELINE_KB=$(cat "$BASELINE_FILE")
  if [ -z "$BASELINE_KB" ] || ! [[ "$BASELINE_KB" =~ ^[0-9]+$ ]] || [ "$BASELINE_KB" -le 0 ]; then
    echo "FAIL: Invalid baseline value '${BASELINE_KB}' in ${BASELINE_FILE}"
    exit 1
  fi
  MAX_KB=$((BASELINE_KB * 105 / 100))
  echo "Library size: ${ACTUAL_KB}KB (baseline: ${BASELINE_KB}KB, max allowed: ${MAX_KB}KB)"

  if [ $ACTUAL_KB -gt $MAX_KB ]; then
    echo "FAIL: Library too large (${ACTUAL_KB}KB > ${MAX_KB}KB, baseline ${BASELINE_KB}KB)"
    exit 1
  fi
else
  echo "WARN: No committed baseline found at ${BASELINE_FILE}"
  echo "INFO: Using absolute max ${ABSOLUTE_MAX_KB}KB"
  echo "INFO: To enable regression detection, run:"
  echo "  echo \$SIZE_IN_KB > ${BASELINE_FILE} && git add ${BASELINE_FILE}"
  MAX_KB=$ABSOLUTE_MAX_KB
  echo "Library size: ${ACTUAL_KB}KB (absolute max: ${MAX_KB}KB)"

  if [ $ACTUAL_KB -gt $MAX_KB ]; then
    echo "FAIL: Library too large (${ACTUAL_KB}KB > absolute max ${MAX_KB}KB)"
    exit 1
  fi
fi

echo "PASS: Size within limits"
exit 0
