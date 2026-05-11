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

# Baseline: 500KB (reasonable for this library)
BASELINE_KB=500
MAX_KB=$((BASELINE_KB * 105 / 100))

ACTUAL_SIZE=$(stat -c%s "$LIB_PATH")
ACTUAL_KB=$((ACTUAL_SIZE / 1024))

echo "Library size: ${ACTUAL_KB}KB (max allowed: ${MAX_KB}KB)"

if [ $ACTUAL_KB -gt $MAX_KB ]; then
  echo "FAIL: Library too large (${ACTUAL_KB}KB > ${MAX_KB}KB)"
  exit 1
fi

echo "PASS: Size within limits"
exit 0
