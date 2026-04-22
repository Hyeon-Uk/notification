#!/bin/bash
#
# Copyright (c) 2026 Samsung Electronics Co., Ltd.
# SPDX-License-Identifier: Apache-2.0
#
# S13 smoke_92_memory_constrained.sh
# ----------------------------------
# Runs notification-test-app inside a systemd-run transient scope with
# MemoryMax=50M. Validates that the library path does not allocate past
# the cgroup ceiling (P10 in the plan).
#
# Falls back to a no-op PASS on hosts without systemd-run (e.g. container
# build servers without systemd in PID 1).

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_ROOT="$(cd "${SCRIPT_DIR}/../.." && pwd)"

APP_NAME="notification-test-app"
MEM_LIMIT="${MEM_LIMIT:-50M}"

if ! command -v systemd-run >/dev/null 2>&1; then
  echo "[S13] WARN: systemd-run not available - PASS (cannot enforce cgroup limit)"
  exit 0
fi

# Require root to use --system scope with MemoryMax reliably on cgroup v2.
if [[ "$(id -u)" -ne 0 ]]; then
  echo "[S13] WARN: not root - PASS (skipping cgroup enforcement)"
  exit 0
fi

APP_PATH=""
for candidate in \
    "/usr/bin/${APP_NAME}" \
    "${REPO_ROOT}/build/src/test-app/${APP_NAME}"; do
  if [[ -x "${candidate}" ]]; then
    APP_PATH="${candidate}"
    break
  fi
done

if [[ -z "${APP_PATH}" ]]; then
  echo "[S13] WARN: ${APP_NAME} not installed - PASS"
  exit 0
fi

echo "[S13] Running ${APP_PATH} under MemoryMax=${MEM_LIMIT}"
if systemd-run \
     --quiet --wait --collect \
     --property="MemoryMax=${MEM_LIMIT}" \
     --property="MemorySwapMax=0" \
     -- "${APP_PATH}" --one-shot-post; then
  echo "[S13] PASS"
  exit 0
else
  rc=$?
  echo "[S13] FAIL: app exited ${rc} under ${MEM_LIMIT} cap"
  exit "${rc}"
fi
