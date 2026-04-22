#!/bin/bash
#
# Copyright (c) 2026 Samsung Electronics Co., Ltd.
# SPDX-License-Identifier: Apache-2.0
#
# check_so_size.sh
# ----------------
# Phase 0 N0-T4 binary-size gate.
#
# Compares the current size of libnotification.so and libnotification-ex.so
# against the baselines recorded in the notification refactoring plan
# (Section 5.5). Exits non-zero when a binary grows beyond the configured
# ceiling.
#
# Baselines (bytes):
#   libnotification.so     =  700000  (700 KB, target <500 KB from Phase 2)
#   libnotification-ex.so  = 1200000  (1.2 MB, target <850 KB from Phase 2)
#
# Usage:
#   scripts/check_so_size.sh [BUILD_DIR]
#
# BUILD_DIR defaults to ./build. The script walks BUILD_DIR looking for
# the two .so files so it works with flat or hierarchical layouts.

set -euo pipefail

BUILD_DIR="${1:-${CMAKE_BINARY_DIR:-build}}"
MAX_RATIO="${NOTI_SIZE_MAX_RATIO:-110}"  # percent, ceiling = baseline * max/100

declare -A BASELINES=(
  ["libnotification.so"]="700000"
  ["libnotification-ex.so"]="1200000"
)

rc=0

find_lib() {
  # Try a few known locations before falling back to find.
  local name="$1"
  local candidate
  for candidate in \
      "${BUILD_DIR}/src/notification/${name}" \
      "${BUILD_DIR}/src/notification-ex/${name}" \
      "${BUILD_DIR}/${name}"; do
    if [[ -f "${candidate}" ]]; then
      echo "${candidate}"
      return 0
    fi
  done
  find "${BUILD_DIR}" -name "${name}" -type f -print -quit 2>/dev/null
}

for lib in "${!BASELINES[@]}"; do
  baseline="${BASELINES[$lib]}"
  path=$(find_lib "${lib}" || true)
  if [[ -z "${path}" ]]; then
    echo "[size] SKIP: ${lib} not found under ${BUILD_DIR}"
    continue
  fi
  bytes=$(stat -c%s "${path}")
  ceiling=$(( baseline * MAX_RATIO / 100 ))
  pct=$(( bytes * 100 / baseline ))
  printf "[size] %-24s %10d B (%3d%% of %d B baseline, ceiling %d B)\n" \
         "${lib}:" "${bytes}" "${pct}" "${baseline}" "${ceiling}"
  if (( bytes > ceiling )); then
    echo "[size] FAIL: ${lib} exceeds ceiling (${bytes} > ${ceiling})"
    rc=1
  fi
done

exit "${rc}"
