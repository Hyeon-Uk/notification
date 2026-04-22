#!/bin/bash
#
# Copyright (c) 2026 Samsung Electronics Co., Ltd.
# SPDX-License-Identifier: Apache-2.0
#
# run_bench.sh
# ------------
# Runs every benchmark binary under BIN_DIR and stores JSON reports under
# OUT_DIR (default: ../results). Intended to be invoked by the CMake
# `bench-all` custom target or directly from CI.
#
# Usage:
#   run_bench.sh [BIN_DIR] [OUT_DIR]
#
# When a benchmark binary is missing (e.g. google/benchmark was not
# available at cmake time), the script logs a warning and skips it. Exit
# code is always 0 unless an actual bench binary aborts.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
BIN_DIR="${1:-${SCRIPT_DIR}}"
OUT_DIR="${2:-${SCRIPT_DIR}/results}"

mkdir -p "${OUT_DIR}"

BENCHES=(
  bench_post_path
  bench_serialize
  bench_db_hot_path
  bench_reporter_lifecycle
  bench_dbus_rtt
)

run_one() {
  local name="$1"
  local bin="${BIN_DIR}/${name}"
  local out="${OUT_DIR}/${name}.json"
  if [[ ! -x "${bin}" ]]; then
    echo "[bench] SKIP: ${bin} not found or not executable"
    return 0
  fi
  echo "[bench] RUN:  ${name}"
  if ! "${bin}" \
        --benchmark_format=json \
        --benchmark_out_format=json \
        --benchmark_out="${out}" \
        > "${OUT_DIR}/${name}.console.json"; then
    echo "[bench] FAIL: ${name} (see ${OUT_DIR}/${name}.console.json)"
    return 1
  fi
}

rc=0
for b in "${BENCHES[@]}"; do
  run_one "${b}" || rc=$?
done

echo "[bench] Done. Reports in ${OUT_DIR}/"
exit "${rc}"
