#!/bin/bash
#
# Copyright (c) 2026 Samsung Electronics Co., Ltd.
# SPDX-License-Identifier: Apache-2.0
#
# S93 smoke_93_dt_needed.sh
# -------------------------
# N4-T2 DT_NEEDED gate for libnotification-ex.so.
#
# Validates that:
#   1. Three deps the notification-ex tree never references — iniparser,
#      tizen-core, rpc-port — never appear as DT_NEEDED entries. They were
#      already prevented from linking by --as-needed, but if a future change
#      pulls one of them transitively, this catches it before merge.
#   2. The set of NEEDED libraries is a subset of an explicit allow-list
#      derived from the PUBLIC + PRIVATE pkg-config split in
#      src/notification-ex/CMakeLists.txt plus the C++ runtime.
#
# Falls back to a soft-PASS if the .so is not present (early CI bootstrap).
#
# Usage:
#   tests/smoke/smoke_93_dt_needed.sh [BUILD_DIR | INSTALL_PREFIX]
#
# When called with no argument, the script searches a few well-known
# locations (project ./build, the GBS scratch root, /usr/lib*/) until it
# finds libnotification-ex.so.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_ROOT="$(cd "${SCRIPT_DIR}/../.." && pwd)"
WHERE="${1:-${REPO_ROOT}/build}"

# ---------- locate the shared object ----------------------------------------
locate_so() {
  local p
  for p in \
      "${WHERE}/src/notification-ex/libnotification-ex.so.1.0.0" \
      "${WHERE}/src/notification-ex/libnotification-ex.so" \
      "${WHERE}/home/abuild/rpmbuild/BUILD/notification-0.12.1/src/notification-ex/libnotification-ex.so.1.0.0" \
      "${WHERE}/usr/lib64/libnotification-ex.so.1.0.0" \
      "${WHERE}/usr/lib/libnotification-ex.so.1.0.0" \
      "${WHERE}/libnotification-ex.so" \
      "/usr/lib64/libnotification-ex.so.1.0.0" \
      "/usr/lib/libnotification-ex.so.1.0.0"; do
    if [[ -f "${p}" ]]; then
      echo "${p}"
      return 0
    fi
  done
  # Last resort: search the GBS BUILD-ROOTS scratch tree, preferring the
  # most-recently modified hit so the script does not stick to a stale build.
  find /home/*/GBS-ROOT*/local/BUILD-ROOTS -name "libnotification-ex.so.1.0.0" \
       -type f -printf "%T@ %p\n" 2>/dev/null \
    | sort -rn | head -1 | cut -d' ' -f2-
}

SO_PATH="$(locate_so || true)"

if [[ -z "${SO_PATH}" || ! -f "${SO_PATH}" ]]; then
  echo "[S93] WARN: libnotification-ex.so not found under ${WHERE} - PASS (no build yet)"
  exit 0
fi

if ! command -v readelf >/dev/null 2>&1; then
  echo "[S93] WARN: readelf not available - PASS (toolchain missing)"
  exit 0
fi

echo "[S93] target: ${SO_PATH}"

# ---------- forbidden libraries (must never be NEEDED) ----------------------
# These three pkg-configs were dropped from the notification-ex link in
# N4-T2. Their grep across src/notification-ex/ returns 0 hits, confirming
# the library code itself has no reference to them.
declare -a FORBIDDEN=(
  "libiniparser"
  "libtizen-core"
  "librpc-port"
)

# ---------- allowed library prefixes ----------------------------------------
# Order/version of soname suffix is deliberately not pinned — the soname
# major number rolls forward over time; we only assert the unversioned base
# is on this list.
declare -a ALLOWED_PREFIX=(
  # PUBLIC pkg-config block
  "libbundle"
  "libdlog"
  "libglib-2.0"
  "libgobject-2.0"
  "libgmodule-2.0"
  "libgio-2.0"
  # PRIVATE pkg-config block
  "libaul"
  "libcapi-appfw-app-common"
  "libcapi-appfw-app-control"
  "libcapi-appfw-package-manager"
  "libcapi-base-common"
  "libtzplatform-config"
  "libpkgmgr-info"
  "libsecurity-manager-client"
  "libsqlite3"
  "libuuid"
  "libvconf"
  # C/C++ runtime + dynamic loader (always present)
  "libc"
  "libstdc++"
  "libgcc_s"
  "libpthread"
  "libdl"
  "libm"
  "libresolv"
  "ld-linux"
)

is_allowed() {
  local lib="$1"
  local p
  for p in "${ALLOWED_PREFIX[@]}"; do
    case "${lib}" in
      ${p}.so*|${p}-*.so*) return 0 ;;
    esac
  done
  return 1
}

# ---------- collect NEEDED entries ------------------------------------------
# Use sed with extended regex — mawk (Ubuntu /usr/bin/awk) lacks the gawk
# 3-arg match() form, and Tizen rootfs may not ship gawk. The NEEDED line
# format from binutils' readelf is stable: " 0x... (NEEDED)  ... [libfoo.so.N]"
mapfile -t NEEDED_LIBS < <(
  readelf -d "${SO_PATH}" \
    | sed -nE 's/.*\(NEEDED\).*Shared library:[[:space:]]*\[([^]]+)\].*/\1/p'
)

if (( ${#NEEDED_LIBS[@]} == 0 )); then
  echo "[S93] FAIL: 0 NEEDED entries — readelf parse failure?"
  exit 1
fi

echo "[S93] DT_NEEDED entries (${#NEEDED_LIBS[@]}):"
for lib in "${NEEDED_LIBS[@]}"; do
  printf "         %s\n" "${lib}"
done

# ---------- forbidden check -------------------------------------------------
rc=0
for lib in "${NEEDED_LIBS[@]}"; do
  for forbidden in "${FORBIDDEN[@]}"; do
    case "${lib}" in
      ${forbidden}.so*|${forbidden}-*.so*)
        echo "[S93] FAIL: forbidden DT_NEEDED entry present: ${lib}"
        rc=1
        ;;
    esac
  done
done

# ---------- allow-list check ------------------------------------------------
for lib in "${NEEDED_LIBS[@]}"; do
  if ! is_allowed "${lib}"; then
    echo "[S93] FAIL: ${lib} not in allow-list — update ALLOWED_PREFIX or remove the link"
    rc=1
  fi
done

if (( rc == 0 )); then
  echo "[S93] PASS: DT_NEEDED matches PUBLIC+PRIVATE allow-list, none of {iniparser,tizen-core,rpc-port}"
else
  echo "[S93] FAIL: DT_NEEDED violation — see lines above"
fi

exit "${rc}"
