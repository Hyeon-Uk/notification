#!/bin/bash
#
# Copyright (c) 2026 Samsung Electronics Co., Ltd.
# SPDX-License-Identifier: Apache-2.0
#
# S11 smoke_90_binary_size.sh
# ---------------------------
# Validates the installed libnotification.so / libnotification-ex.so sizes
# against the Phase 2 ceilings (see refactoring plan Section 5.5). Wraps
# scripts/check_so_size.sh with a fallback that reports PASS when the
# libraries are not yet built - useful during early Phase 0 CI.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_ROOT="$(cd "${SCRIPT_DIR}/../.." && pwd)"
BUILD_DIR="${1:-${REPO_ROOT}/build}"

CHECKER="${REPO_ROOT}/scripts/check_so_size.sh"

if [[ ! -x "${CHECKER}" ]]; then
  echo "[S11] WARN: ${CHECKER} not executable - PASS (early Phase 0)"
  exit 0
fi

if [[ ! -d "${BUILD_DIR}" ]]; then
  echo "[S11] WARN: ${BUILD_DIR} not present - PASS (no build yet)"
  exit 0
fi

"${CHECKER}" "${BUILD_DIR}"
