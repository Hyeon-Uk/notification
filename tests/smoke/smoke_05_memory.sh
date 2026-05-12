#!/bin/bash
# smoke_05_memory.sh — post x100, VmRSS delta < 2MB; optional valgrind check

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
OVERALL_FAIL=0

# ---------------------------------------------------------------------------
# Main memory test: create/post/free x100, measure VmRSS delta.
# Also includes create+set-all-text-fields+free (100x) and clone+free (100x).
# ---------------------------------------------------------------------------
cat > "${TMPDIR}/memory_test.c" << 'EOF'
#include <dlfcn.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void sigalrm_handler(int sig) { (void)sig; }

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
  typedef int (*post_fn)(void*);
  typedef int (*free_fn)(void*);
  typedef void* (*clone_fn)(void*);
  typedef int (*set_text_fn)(void*, int, const char*, const char*, int);

  create_fn noti_create  = (create_fn)dlsym(handle, "notification_create");
  post_fn   noti_post    = (post_fn)dlsym(handle,   "notification_post");
  free_fn   noti_free    = (free_fn)dlsym(handle,   "notification_free");
  clone_fn  noti_clone   = (clone_fn)dlsym(handle,  "notification_clone");
  set_text_fn noti_set_text = (set_text_fn)dlsym(handle, "notification_set_text");

  if (!noti_create || !noti_post || !noti_free) {
    fprintf(stderr, "FAIL: Cannot find core symbols\n");
    dlclose(handle);
    return 1;
  }

  int overall_fail = 0;

  /* Warm up to stabilize RSS before measuring.
   * notification_post may block on IPC if no daemon is running,
   * so use alarm() to interrupt it after 1 second. */
  signal(SIGALRM, sigalrm_handler);

  void* warmup = noti_create(1);
  if (warmup) {
    alarm(1);
    noti_post(warmup);
    alarm(0);
    noti_free(warmup);
  }

  /* --- Test 1: create/post/free x100 --- */
  long rss_before = get_vmrss_kb();
  if (rss_before < 0) {
    fprintf(stderr, "FAIL: Cannot read VmRSS\n");
    dlclose(handle);
    return 1;
  }

  for (int i = 0; i < 100; i++) {
    void* noti = noti_create(1);
    if (noti) {
      alarm(1);
      noti_post(noti);
      alarm(0);
      noti_free(noti);
    }
  }

  long rss_after = get_vmrss_kb();
  long delta_kb = rss_after - rss_before;

  printf("VmRSS before: %ldKB, after: %ldKB, delta: %ldKB (limit: 2048KB)\n",
         rss_before, rss_after, delta_kb);

  if (delta_kb > 2048) {
    printf("FAIL [create/post/free x100]: Memory growth exceeds 2MB\n");
    overall_fail = 1;
  } else {
    printf("PASS [create/post/free x100]: Memory within limits\n");
  }

  /* --- Test 2: create + set all text fields + free (100x cycle) --- */
  if (noti_set_text) {
    long rss2_before = get_vmrss_kb();

    for (int i = 0; i < 100; i++) {
      void* noti = noti_create(1);
      if (noti) {
        /* NOTIFICATION_TEXT_TYPE values 0-9; pass a sample of them */
        for (int t = 0; t <= 9; t++) {
          noti_set_text(noti, t, "Test notification text content", NULL, 0);
        }
        noti_free(noti);
      }
    }

    long rss2_after = get_vmrss_kb();
    long delta2_kb = rss2_after - rss2_before;

    printf("VmRSS before: %ldKB, after: %ldKB, delta: %ldKB (limit: 2048KB)\n",
           rss2_before, rss2_after, delta2_kb);

    if (delta2_kb > 2048) {
      printf("FAIL [create/set-text/free x100]: Memory growth exceeds 2MB\n");
      overall_fail = 1;
    } else {
      printf("PASS [create/set-text/free x100]: Memory within limits\n");
    }
  } else {
    printf("SKIP [create/set-text/free x100]: notification_set_text not resolved\n");
  }

  /* --- Test 3: clone + free cycle (100x) --- */
  if (noti_clone) {
    void* orig = noti_create(1);
    if (orig) {
      long rss3_before = get_vmrss_kb();

      for (int i = 0; i < 100; i++) {
        void* cloned = NULL;
        /* notification_clone signature: (noti_h noti, noti_h *clone) — returns int */
        /* Cast through generic pointer; handle both pointer-return and int-return ABIs */
        typedef int (*clone2_fn)(void*, void**);
        clone2_fn clone2 = (clone2_fn)(void*)noti_clone;
        void* c = NULL;
        int rc = clone2(orig, &c);
        if (rc == 0 && c) {
          noti_free(c);
        } else {
          /* Fallback: try single-arg clone ABI */
          c = noti_clone(orig);
          if (c) noti_free(c);
        }
      }

      long rss3_after = get_vmrss_kb();
      long delta3_kb = rss3_after - rss3_before;

      printf("VmRSS before: %ldKB, after: %ldKB, delta: %ldKB (limit: 2048KB)\n",
             rss3_before, rss3_after, delta3_kb);

      if (delta3_kb > 2048) {
        printf("FAIL [clone/free x100]: Memory growth exceeds 2MB\n");
        overall_fail = 1;
      } else {
        printf("PASS [clone/free x100]: Memory within limits\n");
      }

      noti_free(orig);
    }
  } else {
    printf("SKIP [clone/free x100]: notification_clone not resolved\n");
  }

  dlclose(handle);
  return overall_fail;
}
EOF

gcc -o "${TMPDIR}/memory_test" "${TMPDIR}/memory_test.c" -ldl 2>/dev/null
if [ $? -ne 0 ]; then
  echo "FAIL: Cannot compile memory test"
  rm -rf "$TMPDIR"
  exit 1
fi

"${TMPDIR}/memory_test" "$LIB_PATH"
RSS_RET=$?
if [ "$RSS_RET" -ne 0 ]; then
  OVERALL_FAIL=1
fi

# ---------------------------------------------------------------------------
# Valgrind-based leak check (if valgrind is available)
# ---------------------------------------------------------------------------
if command -v valgrind >/dev/null 2>&1; then
  echo "INFO: valgrind found — running create/free cycle leak check"

  cat > "${TMPDIR}/valgrind_test.c" << 'EOF'
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>

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
  free_fn   noti_free   = (free_fn)dlsym(handle,   "notification_free");

  if (!noti_create || !noti_free) {
    fprintf(stderr, "FAIL: Cannot resolve notification_create/notification_free\n");
    dlclose(handle);
    return 1;
  }

  /* Simple create/free cycle — valgrind will detect heap leaks */
  for (int i = 0; i < 10; i++) {
    void* noti = noti_create(1);
    if (noti) {
      noti_free(noti);
    }
  }

  dlclose(handle);
  return 0;
}
EOF

  gcc -o "${TMPDIR}/valgrind_test" "${TMPDIR}/valgrind_test.c" -ldl 2>/dev/null
  if [ $? -ne 0 ]; then
    echo "WARN: Cannot compile valgrind test program — skipping valgrind check"
  else
    VALGRIND_LOG="${TMPDIR}/valgrind.log"
    valgrind \
      --leak-check=full \
      --error-exitcode=1 \
      --log-file="$VALGRIND_LOG" \
      "${TMPDIR}/valgrind_test" "$LIB_PATH" 2>/dev/null
    VRET=$?

    if [ "$VRET" -eq 0 ]; then
      echo "PASS [valgrind]: No memory leaks detected in create/free cycle"
    else
      echo "FAIL [valgrind]: Memory leaks or errors detected:"
      grep -E "ERROR SUMMARY|definitely lost|indirectly lost|possibly lost" "$VALGRIND_LOG" || true
      OVERALL_FAIL=1
    fi
  fi
else
  echo "INFO: valgrind not available — skipping valgrind leak check"
fi

rm -rf "$TMPDIR"

if [ "$OVERALL_FAIL" -ne 0 ]; then
  exit 1
fi

echo "PASS: All memory checks passed"
exit 0
