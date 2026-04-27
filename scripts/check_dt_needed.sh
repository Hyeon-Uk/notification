#!/bin/bash
#
# Copyright (c) 2026 Samsung Electronics Co., Ltd.
# SPDX-License-Identifier: Apache-2.0
#
# scripts/check_dt_needed.sh
# --------------------------
# N4-T2 baseline-comparison gate for the DT_NEEDED list of
# libnotification.so / libnotification-ex.so.
#
# This script is the CI counterpart of tests/smoke/smoke_93_dt_needed.sh:
# the smoke test enforces a static allow-list, while this script computes a
# diff against a checked-in baseline file. Use the smoke test for the gate
# and this script when investigating a delta.
#
# Modes:
#   * default        — compare current NEEDED list against
#                       tests/baseline/dt_needed_<libname>.txt
#                       (exit 1 on mismatch)
#   * --save         — overwrite the baseline with the current list (manual,
#                       only when the diff is intentional)
#   * --print        — print the current NEEDED list and exit 0
#
# Usage:
#   scripts/check_dt_needed.sh [BUILD_DIR] [--save|--print]

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"

BUILD_DIR="${1:-${REPO_ROOT}/build}"
shift || true
MODE="${1:-check}"

BASELINE_DIR="${REPO_ROOT}/tests/baseline"
mkdir -p "${BASELINE_DIR}"

declare -a TARGETS=("libnotification.so" "libnotification-ex.so")

locate_so() {
  local name="$1"
  local p
  for p in \
      "${BUILD_DIR}/src/notification/${name}.1.0.0" \
      "${BUILD_DIR}/src/notification-ex/${name}.1.0.0" \
      "${BUILD_DIR}/src/notification/${name}" \
      "${BUILD_DIR}/src/notification-ex/${name}" \
      "${BUILD_DIR}/${name}"; do
    if [[ -f "${p}" ]]; then echo "${p}"; return 0; fi
  done
  find "${BUILD_DIR}" -name "${name}*" -type f 2>/dev/null | head -1
}

extract_needed() {
  # Portable: use sed -E (mawk lacks 3-arg match(); Tizen rootfs may have
  # neither gawk nor python).
  readelf -d "$1" \
    | sed -nE 's/.*\(NEEDED\).*Shared library:[[:space:]]*\[([^]]+)\].*/\1/p' \
    | sort -u
}

rc=0

for lib in "${TARGETS[@]}"; do
  path=$(locate_so "${lib}" || true)
  if [[ -z "${path}" ]]; then
    echo "[needed] SKIP: ${lib} not found under ${BUILD_DIR}"
    continue
  fi

  baseline="${BASELINE_DIR}/dt_needed_${lib}.txt"
  current=$(mktemp)
  extract_needed "${path}" > "${current}"

  case "${MODE}" in
    --print)
      echo "[needed] ${lib}:"
      sed 's/^/  /' "${current}"
      ;;
    --save)
      cp "${current}" "${baseline}"
      echo "[needed] saved baseline ${baseline} (${lib})"
      ;;
    *)
      if [[ ! -f "${baseline}" ]]; then
        echo "[needed] WARN: baseline ${baseline} missing — initializing"
        cp "${current}" "${baseline}"
        continue
      fi
      if ! diff -u "${baseline}" "${current}" > /dev/null; then
        echo "[needed] FAIL: ${lib} DT_NEEDED differs from baseline"
        diff -u "${baseline}" "${current}" || true
        rc=1
      else
        echo "[needed] PASS: ${lib} DT_NEEDED matches baseline ($(wc -l < "${current}") entries)"
      fi
      ;;
  esac

  rm -f "${current}" 2>/dev/null || true
done

exit "${rc}"
