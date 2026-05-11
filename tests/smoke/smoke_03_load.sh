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

# Create a small C program that dlopens the library
TMPDIR=$(mktemp -d)
cat > "${TMPDIR}/dlopen_test.c" << 'EOF'
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv) {
  if (argc < 2) {
    fprintf(stderr, "Usage: %s <library_path>\n", argv[0]);
    return 1;
  }
  void* handle = dlopen(argv[1], RTLD_NOW);
  if (!handle) {
    fprintf(stderr, "FAIL: dlopen: %s\n", dlerror());
    return 1;
  }
  printf("PASS: dlopen succeeded\n");
  dlclose(handle);
  return 0;
}
EOF

gcc -o "${TMPDIR}/dlopen_test" "${TMPDIR}/dlopen_test.c" -ldl 2>/dev/null
if [ $? -ne 0 ]; then
  echo "FAIL: Cannot compile dlopen test program"
  rm -rf "$TMPDIR"
  exit 1
fi

"${TMPDIR}/dlopen_test" "$LIB_PATH"
RET=$?

rm -rf "$TMPDIR"
exit $RET
