/*
 * Copyright (c) 2018 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <dlog.h>
#include <aul.h>
#include <unistd.h>
#include <package_info.h>
#include <package_manager.h>

#include <atomic>
#include <string>
#include <string_view>
#include <memory>

#include "notification-ex/ex_util.h"
#include "notification-ex/appid_lru.h"
#include "notification-ex/socket_handler.h"
#include "notification-ex/common.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "NOTIFICATION_EX"

using namespace std;

/* LCOV_EXCL_START */
namespace {
constexpr int MAX_PACKAGE_STR_SIZE = 512;
constexpr int MAX_RETRY_CNT = 10;
}

namespace notification {
namespace util {

int GetRequestId() {
  static int id = 0;
  g_atomic_int_inc(&id);
  return id;
}

string GetAppId() {
  return GetAppId(getpid());
}

/*
 * N3-T1 D5: delegate to the fixed-slot LRU in appid_lru.cc.
 *
 * The previous implementation backed the per-pid cache with a file-local
 * std::map<pid_t, std::string> and used a clear()-on-overflow policy that
 * dropped every hot pid on the first cold pid past MAX_CACHE_SIZE. The
 * new LRU evicts a single slot on miss, keeping ≤ 8 hot pids resident
 * across any traffic pattern.
 *
 * We still return a std::string (by value) here so the public ABI of
 * util::GetAppId(pid_t) is preserved — the LRU's internal string_view
 * is materialised into a std::string at this boundary because callers
 * hold the result across further cache operations which may reshuffle
 * slots and invalidate the view.
 */
string GetAppId(pid_t pid) {
  std::string_view view = detail::LookupOrFillAppId(pid);
  return string(view);
}

string GetPkgId() {
  static string pkgid = "";
  if (!pkgid.empty()) {
    LOGI("pkgid(%s)", pkgid.c_str());
    return pkgid;
  }

  char pkgid_buf[MAX_PACKAGE_STR_SIZE] = {0, };
  int ret = aul_app_get_pkgid_bypid(getpid(), pkgid_buf, sizeof(pkgid_buf));
  if (ret == AUL_R_OK)
    pkgid = string(pkgid_buf);

  return pkgid;
}

string GetDomainName() {
  static string domain_str = "";
  if (!domain_str.empty()) {
    LOGI("domain(%s)", domain_str.c_str());
    return domain_str;
  }

  string pkgid = GetPkgId();
  if (pkgid.empty())
    return "";

  string::size_type pos = pkgid.rfind('.');
  if (pos != string::npos)
    ++pos;
  else
    return "";

  domain_str = pkgid.substr(pos);
  return domain_str;
}

string GetLocaleDirectory() {
  static string locale_directory = "";
  if (!locale_directory.empty()) {
    LOGI("locale_directory(%s)", locale_directory.c_str());
    return locale_directory;
  }

  string pkgid = GetPkgId();
  if (pkgid.empty())
    return "";

  package_info_h package_info;
  int err = package_info_create(pkgid.c_str(), &package_info);
  if (err != PACKAGE_MANAGER_ERROR_NONE) {
    LOGE("fail to get package info");
    return "";
  }
  unique_ptr<package_info_s, decltype(package_info_destroy)*> pkg_ptr(
      package_info, package_info_destroy);

  char* app_root_path;
  err = package_info_get_root_path(pkg_ptr.get(), &app_root_path);
  if (err != PACKAGE_MANAGER_ERROR_NONE) {
    LOGE("Failed to get root path err[%d] path[%p]",
        err, app_root_path);
    return "";
  }

  string app_root_str(app_root_path);
  free(app_root_path);

  locale_directory = app_root_str + string("/res/locale");
  return locale_directory;
}

unsigned int GetSocketBufferSize(int fd, int optname) {
  unsigned int ret_size = 0;
  socklen_t len = sizeof(ret_size);

  errno = 0;
  if (getsockopt(fd, SOL_SOCKET, optname, &ret_size, &len) < 0) {
    LOGE("read socket size [%d]", errno);
    return 0;
  }
  return ret_size;
}

unsigned int ReadSocket(int fd, char* buffer, unsigned int nbytes) {
  unsigned int left = nbytes;
  ssize_t nb;
  int retry_cnt = 0;
  const struct timespec SLEEP_TIME = { 0, 20 * 1000 * 1000 };
  while (left && (retry_cnt < MAX_RETRY_CNT)) {
    errno = 0;
    nb = read(fd, buffer, left);
    if (nb == 0) {
      LOGE("read socket - EOF, fd close [%d]", fd);
      return -1;
    } else if (nb == -1) {
      if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK) {
        LOGE("errno [%d], sleep and retry", errno);
        retry_cnt++;
        nanosleep(&SLEEP_TIME, 0);
        continue;
      }
      LOGE("errno [%d] fd [%d]", errno, fd);
      return -1;
    }
    left -= nb;
    buffer += nb;
    retry_cnt = 0;
  }

  if (left != 0) {
    LOGE("error fd [%d] retry_cnt [%d]", fd, retry_cnt);
    return -1;
  }
  return 0;
}

/*
 * N3-T1 D6: switched from per-call calloc() to a thread_local scratch
 * buffer (EnsureRecvBuf). The previous implementation called calloc()
 * on every ReadBundleList() invocation plus a second calloc() on the
 * grow path inside the loop. With the scratch buffer the first call on
 * a thread heap-allocs once, subsequent calls reuse the buffer
 * (high-watermark retained), and only a genuine grow past the current
 * capacity triggers realloc().
 */
list<tizen_base::Bundle> ReadBundleList(int recvfd, int noti_cnt) {
  list<tizen_base::Bundle> ret_list;
  list<tizen_base::Bundle> empty_list;
  unsigned int data_size = 0;

  unsigned int initial = util::GetSocketBufferSize(recvfd, SO_RCVBUF);
  char* data = util::EnsureRecvBuf(initial);
  size_t buf_size = (data == nullptr) ? 0 : util::RecvBufCapacity();

  for (int i = 0; i < noti_cnt; i++) {
    int ret = util::ReadSocket(recvfd,
        reinterpret_cast<char*>(&data_size), sizeof(data_size));
    if (ret != 0)
      return empty_list;
    if (data_size > buf_size) {
      data = util::EnsureRecvBuf(data_size);
      buf_size = util::RecvBufCapacity();
    }
    memset(data, 0, data_size);
    ret = util::ReadSocket(recvfd, data, data_size);
    if (ret != 0)
      return empty_list;
    ret_list.emplace_back(data);
  }
  return ret_list;
}

int WriteSocket(int fd, const char* buffer, unsigned int nbytes) {
  int retry_cnt = 0;
  unsigned int left = nbytes;
  ssize_t nb;
  const struct timespec SLEEP_TIME = { 0, 20 * 1000 * 1000 };

  while (left && (retry_cnt < MAX_RETRY_CNT)) {
    errno = 0;
    nb = write(fd, buffer, left);
    if (nb == -1) {
      if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK) {
        LOGE("continue..");
        retry_cnt++;
        nanosleep(&SLEEP_TIME, 0);
        continue;
      }
      LOGE("error fd [%d] errno [%d]", fd, errno);
      return -1;
    }

    left -= nb;
    buffer += nb;
    retry_cnt = 0;
  }

  if (left != 0) {
    LOGE("error fd [%d], retry_cnt [%d]", fd, retry_cnt);
    return -1;
  }
  return 0;
}

int ConvertDBusError(int error) {
  if (error == G_DBUS_ERROR_ACCESS_DENIED)
    return ERROR_PERMISSION_DENIED;

  return ERROR_IO_ERROR;
}

/*
 * N1-T7: Thread-local scratch GVariantBuilder implementation.
 *
 * Implementation note on g_autoptr + thread_local:
 *   The combination is safe IFF the cleanup attribute only runs at thread
 *   exit, not at each function return. In C (the original glib idiom)
 *   g_autoptr is a compiler attribute bound to the variable's enclosing
 *   scope - which for thread_local means thread lifetime. In C++ the TLS
 *   storage class gives us the same semantics through the __thread / TLS
 *   destructor. We express this by storing a raw GVariantBuilder* (not a
 *   g_autoptr) inside a thread_local RAII wrapper whose dtor runs at
 *   thread exit. This avoids the "auto-unref on every function return"
 *   trap called out in the design doc (N1-T7 risks).
 *
 *   The wrapper also tracks the cold-path alloc count for unit tests so
 *   they can prove the second Notify() on the same thread re-uses the
 *   scratch builder with zero g_variant_builder_new() calls.
 */
namespace {

struct ScratchBuilderSlot {
  GVariantBuilder* builder = nullptr;
  int alloc_count = 0;
  bool initialised = false;  // true once g_variant_builder_init ran at least once

  ~ScratchBuilderSlot() {
    if (builder) {
      // The scratch builder was *either* mid-use (in which case the caller
      // leaked it - unit test bug) *or* was fully-drained via
      // g_variant_builder_end() which already cleared it. Unref is always
      // safe on a heap builder.
      g_variant_builder_unref(builder);
      builder = nullptr;
    }
  }
};

static thread_local ScratchBuilderSlot tls_slot;

}  // namespace

GVariantBuilder* ScratchBuilder(const GVariantType* type) {
  if (tls_slot.builder == nullptr) {
    /* Cold path: one-time per-thread heap alloc. g_variant_builder_new
     * returns a builder already initialised for `type`. */
    tls_slot.builder = g_variant_builder_new(type);
    tls_slot.alloc_count++;
    tls_slot.initialised = true;
    return tls_slot.builder;
  }

  /* Hot path: re-init the existing heap builder.
   *
   * After g_variant_new("a(s)", builder) the builder was drained by GLib's
   * internal g_variant_builder_end() and is therefore in an "uninitialised"
   * state ready for g_variant_builder_init(). g_variant_builder_clear() is
   * still safe (idempotent on a drained builder, no-op) and guards against
   * reuse paths that might abort mid-build (defensive on exception paths).
   */
  g_variant_builder_clear(tls_slot.builder);
  g_variant_builder_init(tls_slot.builder, type);
  return tls_slot.builder;
}

void ResetScratchBuilderForTest() {
  if (tls_slot.builder) {
    g_variant_builder_unref(tls_slot.builder);
    tls_slot.builder = nullptr;
  }
  tls_slot.alloc_count = 0;
  tls_slot.initialised = false;
}

int GetScratchBuilderAllocCountForTest() {
  return tls_slot.alloc_count;
}

/*
 * N3-T1 D6: per-thread receive scratch buffer for ReadBundleList().
 *
 * Lifetime:
 *   - RecvScratch::buf is realloc'd on grow; freed at thread exit by
 *     the dtor. Idempotent on a never-used thread.
 *   - Capacity is high-watermark: EnsureRecvBuf(4KB) followed by
 *     EnsureRecvBuf(512) returns the existing 4KB buffer unchanged
 *     (no shrink). This is the whole point of the optimisation — the
 *     old code reallocated on every grow and discarded capacity when
 *     SO_RCVBUF was re-read per call.
 *
 * Alignment:
 *   realloc() returns memory aligned for any scalar type; that matches
 *   what the pre-N3-T1 calloc() path delivered. Callers treating the
 *   buffer as bytes (ReadBundleList) or as bundle_raw (Bundle ctor) are
 *   both well-served.
 */
namespace {

struct RecvScratch {
  char*  buf = nullptr;
  size_t cap = 0;
  ~RecvScratch() {
    if (buf != nullptr) {
      free(buf);
      buf = nullptr;
      cap = 0;
    }
  }
};

static thread_local RecvScratch tls_recv;

}  // namespace

char* EnsureRecvBuf(size_t size) {
  if (size == 0) {
    /* Degenerate request — callers that hit this path (SO_RCVBUF == 0
     * in some emulator builds) previously got a 0-byte calloc which
     * returns NULL or an unwritable pointer. Keep that behaviour. */
    return tls_recv.buf;
  }
  if (size <= tls_recv.cap) {
    /* High-watermark hit — zero realloc, zero free. */
    return tls_recv.buf;
  }
  /* Grow path: realloc keeps previous bytes (we don't rely on them but
   * realloc is still the cheapest path compared to free + calloc). */
  char* grown = static_cast<char*>(realloc(tls_recv.buf, size));
  if (grown == nullptr) {
    /* realloc failure leaves tls_recv.buf intact; return NULL so the
     * caller can bail out without writing past the old capacity. */
    LOGE("RecvScratch grow failed: size=%zu cap=%zu", size, tls_recv.cap);
    return nullptr;
  }
  tls_recv.buf = grown;
  tls_recv.cap = size;
  return tls_recv.buf;
}

size_t RecvBufCapacity() {
  return tls_recv.cap;
}

void ResetRecvBufForTest() {
  if (tls_recv.buf != nullptr) {
    free(tls_recv.buf);
    tls_recv.buf = nullptr;
    tls_recv.cap = 0;
  }
}

/*
 * N3-T3 C3: per-process uid cache for notification-ex.
 *
 * Holds the value alongside an explicit "initialised" flag rather than a
 * sentinel (uid_t)-1 so unit tests that ResetCachedUidForTest() back to
 * 0 do not spuriously keep skipping the aul_getuid() call on the next
 * iteration. std::atomic<bool> plus a simple uid_t suffices: the cache
 * is idempotent, a racing reader either sees a stale `false` (and harmlessly
 * re-calls aul_getuid()) or sees `true` and the already-written uid.
 */
namespace {
static std::atomic<bool> g_cached_uid_set{false};
static uid_t g_cached_uid_value = 0;
}  // namespace

uid_t GetCachedUid() {
  if (g_cached_uid_set.load(std::memory_order_acquire))
    return g_cached_uid_value;

  uid_t local = aul_getuid();
  g_cached_uid_value = local;
  g_cached_uid_set.store(true, std::memory_order_release);
  return local;
}

void ResetCachedUidForTest() {
  g_cached_uid_set.store(false, std::memory_order_release);
  g_cached_uid_value = 0;
}
/* LCOV_EXCL_STOP */

}  // namespace util
}  // namespace notification
