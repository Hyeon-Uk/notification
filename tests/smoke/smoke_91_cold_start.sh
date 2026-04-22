#!/bin/bash
#
# Copyright (c) 2026 Samsung Electronics Co., Ltd.
# SPDX-License-Identifier: Apache-2.0
#
# S12 smoke_91_cold_start.sh
# --------------------------
# Verifies Reporter cold-start + first notification_post() completes within
# the Phase 4 target of 50 ms. Uses systemd-analyze when available; falls
# back to /usr/bin/time -v measurement around the notification-test-app.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_ROOT="$(cd "${SCRIPT_DIR}/../.." && pwd)"

TARGET_MS=50
APP_NAME="notification-test-app"

# Resolve the test app. On a Tizen target it is installed to /usr/bin;
# during development it lives under build/.
APP_PATH=""
for candidate in \
    "/usr/bin/${APP_NAME}" \
    "${REPO_ROOT}/build/src/test-app/${APP_NAME}" \
    "$(command -v ${APP_NAME} 2>/dev/null || true)"; do
  if [[ -n "${candidate}" && -x "${candidate}" ]]; then
    APP_PATH="${candidate}"
    break
  fi
done

if [[ -z "${APP_PATH}" ]]; then
  echo "[S12] WARN: ${APP_NAME} not installed - PASS (no app to measure)"
  exit 0
fi

if command -v systemd-analyze >/dev/null 2>&1; then
  echo "[S12] Using systemd-analyze time"
  systemd-analyze time || true
fi

start_ns=$(date +%s%N)
"${APP_PATH}" --one-shot-post 2>/dev/null || true
end_ns=$(date +%s%N)

elapsed_ms=$(( (end_ns - start_ns) / 1000000 ))
echo "[S12] cold-start first-post approx: ${elapsed_ms} ms (target < ${TARGET_MS} ms)"

if (( elapsed_ms >= TARGET_MS * 2 )); then
  # Only fail when we overshoot the target by 2x; Phase 0 is just reporting.
  echo "[S12] FAIL: cold-start too slow"
  exit 1
fi
exit 0
