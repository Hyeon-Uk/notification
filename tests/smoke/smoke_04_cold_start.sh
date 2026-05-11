#!/bin/bash
# smoke_04_cold_start.sh — verify first notification_post < 500ms

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

TMPDIR=$(mktemp -d)
cat > "${TMPDIR}/cold_start_test.c" << 'EOF'
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char** argv) {
  if (argc < 2) return 1;

  struct timespec start, end;
  clock_gettime(CLOCK_MONOTONIC, &start);

  void* handle = dlopen(argv[1], RTLD_NOW);
  if (!handle) {
    fprintf(stderr, "FAIL: dlopen: %s\n", dlerror());
    return 1;
  }

  // Try to resolve notification_create
  void* sym = dlsym(handle, "notification_create");
  if (!sym) {
    fprintf(stderr, "FAIL: notification_create not found\n");
    dlclose(handle);
    return 1;
  }

  clock_gettime(CLOCK_MONOTONIC, &end);

  long elapsed_ms = (end.tv_sec - start.tv_sec) * 1000 +
                    (end.tv_nsec - start.tv_nsec) / 1000000;

  printf("Cold start time: %ldms (limit: 500ms)\n", elapsed_ms);

  dlclose(handle);

  if (elapsed_ms > 500) {
    printf("FAIL: Cold start too slow\n");
    return 1;
  }

  printf("PASS: Cold start within limit\n");
  return 0;
}
EOF

gcc -o "${TMPDIR}/cold_start_test" "${TMPDIR}/cold_start_test.c" -ldl 2>/dev/null
if [ $? -ne 0 ]; then
  echo "FAIL: Cannot compile cold start test"
  rm -rf "$TMPDIR"
  exit 1
fi

"${TMPDIR}/cold_start_test" "$LIB_PATH"
RET=$?

rm -rf "$TMPDIR"
exit $RET
