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
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>

void sigalrm_handler(int sig) { (void)sig; }

int count_fds() {
  char path[64];
  snprintf(path, sizeof(path), "/proc/%d/fd", getpid());
  DIR* d = opendir(path);
  if (!d) return -1;
  int count = 0;
  while (readdir(d)) count++;
  closedir(d);
  return count - 2; /* subtract . and .. */
}

int main(int argc, char** argv) {
  if (argc < 2) return 1;

  void* handle = dlopen(argv[1], RTLD_NOW);
  if (!handle) {
    fprintf(stderr, "FAIL: dlopen: %s\n", dlerror());
    return 1;
  }

  typedef void* (*create_fn)(int);
  typedef int (*post_fn)(void*);
  typedef int (*free_fn)(void*);
  typedef int (*clone_fn)(void*, void**);

  create_fn noti_create = (create_fn)dlsym(handle, "notification_create");
  post_fn   noti_post   = (post_fn)dlsym(handle,   "notification_post");
  free_fn   noti_free   = (free_fn)dlsym(handle,   "notification_free");
  clone_fn  noti_clone  = (clone_fn)dlsym(handle,  "notification_clone");

  if (!noti_create || !noti_post || !noti_free) {
    fprintf(stderr, "FAIL: Cannot find core symbols\n");
    dlclose(handle);
    return 1;
  }

  /* notification_post may block on IPC without a daemon; use alarm to timeout */
  signal(SIGALRM, sigalrm_handler);

  int overall_fail = 0;

  /* -----------------------------------------------------------------------
   * Scenario A (original): create/post/free x10 — verify FD stable
   * --------------------------------------------------------------------- */

  /* Warm up to let any one-time FD allocations settle */
  void* warmup = noti_create(1);
  if (warmup) {
    alarm(1);
    noti_post(warmup);
    alarm(0);
    noti_free(warmup);
  }

  int fd_before_a = count_fds();

  for (int i = 0; i < 10; i++) {
    void* noti = noti_create(1);
    if (noti) {
      alarm(1);
      noti_post(noti);
      alarm(0);
      noti_free(noti);
    }
  }

  int fd_after_a = count_fds();
  int delta_a = fd_after_a - fd_before_a;

  printf("[Scenario A - create/post/free x10] FDs before: %d, after: %d, delta: %d\n",
         fd_before_a, fd_after_a, delta_a);

  if (delta_a > 0) {
    printf("FAIL [Scenario A]: FD leak detected (delta=%d)\n", delta_a);
    overall_fail = 1;
  } else {
    printf("PASS [Scenario A]: No FD leaks (create/post/free)\n");
  }

  /* -----------------------------------------------------------------------
   * Scenario B: create/free only cycle (no post) — verify FD stable
   * --------------------------------------------------------------------- */
  int fd_before_b = count_fds();

  for (int i = 0; i < 10; i++) {
    void* noti = noti_create(1);
    if (noti) {
      noti_free(noti);
    }
  }

  int fd_after_b = count_fds();
  int delta_b = fd_after_b - fd_before_b;

  printf("[Scenario B - create/free only x10] FDs before: %d, after: %d, delta: %d\n",
         fd_before_b, fd_after_b, delta_b);

  if (delta_b > 0) {
    printf("FAIL [Scenario B]: FD leak detected in create/free-only cycle (delta=%d)\n", delta_b);
    overall_fail = 1;
  } else {
    printf("PASS [Scenario B]: No FD leaks (create/free only)\n");
  }

  /* -----------------------------------------------------------------------
   * Scenario C: clone/free cycle — verify FD stable
   * --------------------------------------------------------------------- */
  if (noti_clone) {
    void* orig = noti_create(1);
    if (orig) {
      /* Warm up clone path */
      void* wc = NULL;
      noti_clone(orig, &wc);
      if (wc) noti_free(wc);

      int fd_before_c = count_fds();

      for (int i = 0; i < 10; i++) {
        void* c = NULL;
        int rc = noti_clone(orig, &c);
        if (rc == 0 && c) {
          noti_free(c);
        }
      }

      int fd_after_c = count_fds();
      int delta_c = fd_after_c - fd_before_c;

      printf("[Scenario C - clone/free x10] FDs before: %d, after: %d, delta: %d\n",
             fd_before_c, fd_after_c, delta_c);

      if (delta_c > 0) {
        printf("FAIL [Scenario C]: FD leak detected in clone/free cycle (delta=%d)\n", delta_c);
        overall_fail = 1;
      } else {
        printf("PASS [Scenario C]: No FD leaks (clone/free)\n");
      }

      noti_free(orig);
    } else {
      printf("SKIP [Scenario C]: notification_create returned NULL\n");
    }
  } else {
    printf("SKIP [Scenario C]: notification_clone not resolved\n");
  }

  dlclose(handle);
  return overall_fail;
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
