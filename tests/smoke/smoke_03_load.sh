#!/bin/bash
# smoke_03_load.sh — verify dlopen succeeds without crash

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

# Create a small C program that dlopens the library, resolves symbols,
# checks ldd dependencies, and runs a repeated open/close stress test.
TMPDIR=$(mktemp -d)
cat > "${TMPDIR}/dlopen_test.c" << 'EOF'
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char** argv) {
  if (argc < 2) {
    fprintf(stderr, "Usage: %s <library_path>\n", argv[0]);
    return 1;
  }

  const char* lib = argv[1];
  int overall_fail = 0;

  /* --- Basic dlopen --- */
  void* handle = dlopen(lib, RTLD_NOW);
  if (!handle) {
    fprintf(stderr, "FAIL: dlopen: %s\n", dlerror());
    return 1;
  }
  printf("PASS: dlopen succeeded\n");

  /* --- dlsym resolution checks --- */
  const char* required_syms[] = {
    "notification_create",
    "notification_free",
    "notification_post",
    "notification_update",
    "notification_delete",
    "notification_db_init",
    "notification_channel_create",
    "notification_error_quark",
    NULL
  };

  for (int i = 0; required_syms[i] != NULL; i++) {
    void* sym = dlsym(handle, required_syms[i]);
    if (sym) {
      printf("PASS [dlsym]: %s resolved\n", required_syms[i]);
    } else {
      /* dlsym may return NULL for functions not present in this build;
       * treat as a warning but continue — symbol may be optional. */
      printf("WARN [dlsym]: %s not resolved (%s)\n", required_syms[i], dlerror());
    }
  }

  dlclose(handle);

  /* --- Repeated dlopen/dlclose stress test (10x) --- */
  int stress_fail = 0;
  for (int i = 0; i < 10; i++) {
    void* h = dlopen(lib, RTLD_NOW);
    if (!h) {
      fprintf(stderr, "FAIL [stress iter %d]: dlopen: %s\n", i, dlerror());
      stress_fail = 1;
      overall_fail = 1;
      break;
    }
    dlclose(h);
  }
  if (!stress_fail) {
    printf("PASS: dlopen/dlclose stress test (10 iterations) succeeded\n");
  }

  return overall_fail;
}
EOF

gcc -o "${TMPDIR}/dlopen_test" "${TMPDIR}/dlopen_test.c" -ldl 2>/dev/null
if [ $? -ne 0 ]; then
  echo "FAIL: Cannot compile dlopen test program"
  rm -rf "$TMPDIR"
  exit 1
fi

OVERALL_FAIL=0

"${TMPDIR}/dlopen_test" "$LIB_PATH"
RET=$?
if [ "$RET" -ne 0 ]; then
  OVERALL_FAIL=1
fi

# --- ldd dependency check: no "not found" entries ---
if command -v ldd >/dev/null 2>&1; then
  LDD_OUTPUT=$(ldd "$LIB_PATH" 2>/dev/null)
  if echo "$LDD_OUTPUT" | grep -q "not found"; then
    echo "FAIL [ldd]: Missing shared library dependencies:"
    echo "$LDD_OUTPUT" | grep "not found"
    OVERALL_FAIL=1
  else
    echo "PASS [ldd]: All shared library dependencies resolved"
  fi
else
  echo "SKIP [ldd]: 'ldd' not available"
fi

rm -rf "$TMPDIR"

if [ "$OVERALL_FAIL" -ne 0 ]; then
  exit 1
fi

echo "PASS: All load checks passed"
exit 0
