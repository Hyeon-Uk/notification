#!/bin/bash
# smoke_05_memory.sh — post x100, VmRSS delta < 2MB

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
cat > "${TMPDIR}/memory_test.c" << 'EOF'
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

long get_vmrss_kb() {
  FILE* f = fopen("/proc/self/status", "r");
  if (!f) return -1;
  char line[256];
  long rss = -1;
  while (fgets(line, sizeof(line), f)) {
    if (strncmp(line, "VmRSS:", 6) == 0) {
      sscanf(line + 6, " %ld", &rss);
      break;
    }
  }
  fclose(f);
  return rss;
}

int main(int argc, char** argv) {
  if (argc < 2) return 1;

  void* handle = dlopen(argv[1], RTLD_NOW);
  if (!handle) {
    fprintf(stderr, "FAIL: dlopen: %s\n", dlerror());
    return 1;
  }

  typedef void* (*create_fn)(int);
  typedef int (*free_fn)(void*);

  create_fn noti_create = (create_fn)dlsym(handle, "notification_create");
  free_fn noti_free = (free_fn)dlsym(handle, "notification_free");

  if (!noti_create || !noti_free) {
    fprintf(stderr, "FAIL: Cannot find notification_create/notification_free\n");
    dlclose(handle);
    return 1;
  }

  long rss_before = get_vmrss_kb();
  if (rss_before < 0) {
    fprintf(stderr, "FAIL: Cannot read VmRSS\n");
    dlclose(handle);
    return 1;
  }

  // Create and free 100 notifications
  for (int i = 0; i < 100; i++) {
    void* noti = noti_create(1); // NOTIFICATION_TYPE_NOTI = 1
    if (noti) noti_free(noti);
  }

  long rss_after = get_vmrss_kb();
  long delta_kb = rss_after - rss_before;

  printf("VmRSS before: %ldKB, after: %ldKB, delta: %ldKB (limit: 2048KB)\n",
         rss_before, rss_after, delta_kb);

  dlclose(handle);

  if (delta_kb > 2048) {
    printf("FAIL: Memory growth exceeds 2MB\n");
    return 1;
  }

  printf("PASS: Memory within limits\n");
  return 0;
}
EOF

gcc -o "${TMPDIR}/memory_test" "${TMPDIR}/memory_test.c" -ldl 2>/dev/null
if [ $? -ne 0 ]; then
  echo "FAIL: Cannot compile memory test"
  rm -rf "$TMPDIR"
  exit 1
fi

"${TMPDIR}/memory_test" "$LIB_PATH"
RET=$?

rm -rf "$TMPDIR"
exit $RET
