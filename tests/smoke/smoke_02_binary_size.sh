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

# Resolve symlinks so stat/file operate on the actual binary
if [ -L "$LIB_PATH" ]; then
  RESOLVED=$(readlink -f "$LIB_PATH" 2>/dev/null || realpath "$LIB_PATH" 2>/dev/null)
  if [ -n "$RESOLVED" ] && [ -f "$RESOLVED" ]; then
    LIB_PATH="$RESOLVED"
  fi
fi

OVERALL_FAIL=0

# SIZE_01: file exists
if test -f "$LIB_PATH"; then
  echo "PASS [SIZE_01]: File exists: $LIB_PATH"
else
  echo "FAIL [SIZE_01]: File does not exist: $LIB_PATH"
  OVERALL_FAIL=1
fi

# SIZE_02: file is not empty
if test -s "$LIB_PATH"; then
  echo "PASS [SIZE_02]: File is not empty"
else
  echo "FAIL [SIZE_02]: File is empty"
  OVERALL_FAIL=1
fi

ACTUAL_SIZE=$(stat -c%s "$LIB_PATH")
ACTUAL_KB=$((ACTUAL_SIZE / 1024))

# SIZE_03: minimum size >= 100KB
MIN_KB=100
if [ "$ACTUAL_KB" -ge "$MIN_KB" ]; then
  echo "PASS [SIZE_03]: Minimum size check: ${ACTUAL_KB}KB >= ${MIN_KB}KB"
else
  echo "FAIL [SIZE_03]: File too small: ${ACTUAL_KB}KB < ${MIN_KB}KB"
  OVERALL_FAIL=1
fi

ABSOLUTE_MAX_KB=2048

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
    OVERALL_FAIL=1
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
    OVERALL_FAIL=1
  fi
fi

# SIZE_06: verify ELF format via `file` command
if command -v file >/dev/null 2>&1; then
  FILE_OUTPUT=$(file "$LIB_PATH" 2>/dev/null)
  if echo "$FILE_OUTPUT" | grep -qi "ELF"; then
    echo "PASS [SIZE_06]: ELF format verified: $FILE_OUTPUT"
  else
    echo "FAIL [SIZE_06]: Not an ELF file: $FILE_OUTPUT"
    OVERALL_FAIL=1
  fi
else
  echo "SKIP [SIZE_06]: 'file' command not available"
fi

# SIZE_07: verify "shared object" in `file` output
if command -v file >/dev/null 2>&1; then
  FILE_OUTPUT=$(file "$LIB_PATH" 2>/dev/null)
  if echo "$FILE_OUTPUT" | grep -qi "shared object"; then
    echo "PASS [SIZE_07]: Shared object format verified"
  else
    echo "FAIL [SIZE_07]: Not a shared object: $FILE_OUTPUT"
    OVERALL_FAIL=1
  fi
else
  echo "SKIP [SIZE_07]: 'file' command not available"
fi

# SIZE_10: .text section exists via readelf -S
if command -v readelf >/dev/null 2>&1; then
  if readelf -S "$LIB_PATH" 2>/dev/null | grep -q "\.text"; then
    echo "PASS [SIZE_10]: .text section present"
  else
    echo "FAIL [SIZE_10]: .text section not found in ELF section headers"
    OVERALL_FAIL=1
  fi
else
  echo "SKIP [SIZE_10]: 'readelf' not available"
fi

if [ "$OVERALL_FAIL" -ne 0 ]; then
  echo "FAIL: One or more binary size checks failed"
  exit 1
fi

echo "PASS: All binary size checks passed"
exit 0
