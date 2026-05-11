#!/bin/bash
# smoke_06_fd_leak.sh — post + free, open FD count unchanged

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
cat > "${TMPDIR}/fd_test.c" << 'EOF'
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>

int count_fds() {
  char path[64];
  snprintf(path, sizeof(path), "/proc/%d/fd", getpid());
  DIR* d = opendir(path);
  if (!d) return -1;
  int count = 0;
  while (readdir(d)) count++;
  closedir(d);
  return count - 2; // subtract . and ..
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
    fprintf(stderr, "FAIL: Cannot find symbols\n");
    dlclose(handle);
    return 1;
  }

  int fd_before = count_fds();

  // Create and free notifications
  for (int i = 0; i < 10; i++) {
    void* noti = noti_create(1);
    if (noti) noti_free(noti);
  }

  int fd_after = count_fds();
  int delta = fd_after - fd_before;

  printf("FDs before: %d, after: %d, delta: %d\n", fd_before, fd_after, delta);

  dlclose(handle);

  if (delta > 0) {
    printf("FAIL: FD leak detected (delta=%d)\n", delta);
    return 1;
  }

  printf("PASS: No FD leaks\n");
  return 0;
}
EOF

gcc -o "${TMPDIR}/fd_test" "${TMPDIR}/fd_test.c" -ldl 2>/dev/null
if [ $? -ne 0 ]; then
  echo "FAIL: Cannot compile FD test"
  rm -rf "$TMPDIR"
  exit 1
fi

"${TMPDIR}/fd_test" "$LIB_PATH"
RET=$?

rm -rf "$TMPDIR"
exit $RET
