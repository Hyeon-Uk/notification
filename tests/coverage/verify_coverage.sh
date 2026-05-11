#!/bin/bash
# verify_coverage.sh — verify method coverage meets threshold
# Usage: verify_coverage.sh [BUILD_DIR] [THRESHOLD]
#   BUILD_DIR:  GBS build directory containing .gcno/.gcda files (default: auto-detect)
#   THRESHOLD:  minimum function coverage % required (default: 100)

set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$(cd "${SCRIPT_DIR}/../.." && pwd)"

BUILD_DIR="${1:-}"
THRESHOLD="${2:-100}"

if [ -z "$BUILD_DIR" ]; then
  BUILD_DIR=$(find /home/kimhyeonuk/GBS-ROOT-*/local/BUILD-ROOTS/scratch.*/home/abuild/rpmbuild/BUILD/notification-* -maxdepth 0 2>/dev/null | sort -r | head -1)
  if [ -z "$BUILD_DIR" ]; then
    echo "ERROR: No GBS build directory found. Build with --define 'gcov 1' first."
    echo "Usage: gbs build -A x86_64 --include-all --define 'gcov 1' -B /home/kimhyeonuk/GBS-ROOT-COV"
    exit 1
  fi
fi

echo "=== Coverage Verification ==="
echo "Build dir: ${BUILD_DIR}"
echo "Threshold: ${THRESHOLD}%"
echo ""

COVERAGE_INFO="${BUILD_DIR}/notification_coverage.info"

lcov --capture \
  --ignore-errors mismatch,graph,unused,inconsistent \
  --no-external \
  --directory "${BUILD_DIR}" \
  --output-file "${COVERAGE_INFO}" \
  --quiet

lcov --remove "${COVERAGE_INFO}" \
  '*/notification-ex/*' \
  '*/notification_tidl_proxy.c' \
  '*/notification_tidl.c' \
  '*/notification_internal_tidl.c' \
  '*/tests/*' \
  '*/mock/*' \
  --ignore-errors unused \
  --output-file "${COVERAGE_INFO}" \
  --quiet

SUMMARY=$(lcov --summary "${COVERAGE_INFO}" 2>&1)

FUNC_COV=$(echo "$SUMMARY" | grep -oP 'functions\.*:\s*\K[0-9.]+')

if [ -z "$FUNC_COV" ]; then
  echo "ERROR: Could not parse function coverage from lcov output"
  echo "$SUMMARY"
  exit 1
fi

FUNC_COV_INT=${FUNC_COV%.*}

echo "Function (method) coverage: ${FUNC_COV}%"
echo ""

if [ "$FUNC_COV_INT" -lt "$THRESHOLD" ]; then
  echo "FAIL: Function coverage ${FUNC_COV}% < ${THRESHOLD}% threshold"
  echo ""
  echo "Uncovered functions:"
  lcov --list "${COVERAGE_INFO}" 2>/dev/null | grep -v "100.0%" | head -30
  exit 1
fi

echo "PASS: Function coverage ${FUNC_COV}% >= ${THRESHOLD}%"

REPORT_DIR="${BUILD_DIR}/coverage_report"
genhtml "${COVERAGE_INFO}" --output-directory "${REPORT_DIR}" --quiet 2>/dev/null || true
if [ -d "$REPORT_DIR" ]; then
  echo "HTML report: ${REPORT_DIR}/index.html"
fi

exit 0
