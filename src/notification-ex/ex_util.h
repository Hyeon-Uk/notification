/*
 * Copyright (c) 2019 Samsung Electronics Co., Ltd.
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

#ifndef NOTIFICATION_EX_EX_UTIL_H_
#define NOTIFICATION_EX_EX_UTIL_H_

#include <gmodule.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <bundle_cpp.h>

#include <string>
#include <list>

#ifndef EXPORT_API
#define EXPORT_API __attribute__((visibility("default")))
#endif

namespace notification {
namespace util {
  // N2-T5: util:: helpers are referenced from within the .so and from the
  // unittest executable, so they must stay at default visibility once the
  // project-wide -fvisibility=hidden kicks in on C++ TUs.
  EXPORT_API std::string GetAppId();
  EXPORT_API std::string GetAppId(pid_t pid);
  EXPORT_API int GetRequestId();
  EXPORT_API std::string GetPkgId();
  EXPORT_API std::string GetDomainName();
  EXPORT_API std::string GetLocaleDirectory();
  EXPORT_API unsigned int GetSocketBufferSize(int fd, int optname);
  EXPORT_API unsigned int ReadSocket(int fd, char* buffer, unsigned int nbytes);
  EXPORT_API int WriteSocket(int fd, const char* buffer, unsigned int nbytes);
  EXPORT_API std::list<tizen_base::Bundle> ReadBundleList(int recvfd, int noti_cnt);
  EXPORT_API int ConvertDBusError(int error);

  /*
   * N1-T7: Thread-local scratch GVariantBuilder.
   *
   * Rationale:
   *   The DBusSender hot path (Notify / MethodCall) allocates a fresh
   *   GVariantBuilder via g_variant_builder_new() on every call. The builder
   *   is consumed by g_variant_new("a(s)", ...) which takes ownership of the
   *   resulting child values but the builder itself still has to be unref'd.
   *   Across 100 posts in the bench harness this accounts for 200+ heap allocs
   *   that are trivially reusable (same GVariantType, same reset-on-end
   *   semantics).
   *
   * Design:
   *   - One scratch GVariantBuilder per thread, stored via g_autoptr so the
   *     thread's exit path naturally runs g_variant_builder_unref().
   *   - ScratchBuilder(type) returns a pointer to the re-initialised builder.
   *     Semantically equivalent to g_variant_builder_new(type), but after the
   *     first call on a given thread all subsequent calls reuse the same
   *     heap object. The previous builder state (if any) is cleared via
   *     g_variant_builder_clear + g_variant_builder_init.
   *   - The caller does NOT own the returned pointer; it MUST NOT call
   *     g_variant_builder_unref or retain the pointer across thread exits.
   *   - Calling g_variant_builder_end() on the returned pointer is safe: it
   *     resets the builder so the next ScratchBuilder() call can init-reuse.
   *
   * Thread-safety:
   *   Each thread has its own instance (thread_local inside the .cc). No
   *   locking, no contention.
   *
   * ABI: file-static. Not EXPORT_API. Internal hot-path helper only.
   */
  EXPORT_API GVariantBuilder* ScratchBuilder(const GVariantType* type);

  /* Test-only: reset the current thread's scratch builder. Allows unit
   * tests to isolate builder-reuse allocation counts across TESTs without
   * relying on test execution order. */
  EXPORT_API void ResetScratchBuilderForTest();

  /* Test-only: returns how many times ScratchBuilder() has had to
   * g_variant_builder_new() on the current thread (i.e. cold path count).
   * On a reused-thread hot path this should stay at 1. */
  EXPORT_API int GetScratchBuilderAllocCountForTest();

  /*
   * N3-T3 C3: per-process cached aul_getuid() for notification-ex.
   *
   * Same rationale as the C-side notification_cached_uid() (see
   * notification_private.h): aul_getuid() is called from AbstractItem and
   * IEventInfo constructors on every item / event instantiation, yet the
   * answer is a fixed per-process value. Cache it once in a file-static
   * (protected by std::call_once) and hand out the cached uid_t on every
   * subsequent call.
   *
   * The cache MUST be reset between unit TEST_F runs so the per-test
   * EXPECT_CALL(aul_getuid).Times(1) assertion is scoped correctly —
   * ResetCachedUidForTest() provides that hook.
   */
  EXPORT_API uid_t GetCachedUid();

  /* Test-only: drop the cached uid so the next GetCachedUid() re-invokes
   * aul_getuid(). */
  EXPORT_API void ResetCachedUidForTest();

  /*
   * N3-T1 D6: thread-local scratch receive buffer for ReadBundleList().
   *
   * Problem this solves:
   *   ReadBundleList() historically called calloc(SO_RCVBUF, 1) on EVERY
   *   invocation and, when a message exceeded that size, calloc()d again.
   *   In the notification-ex receive hot path each incoming batch of
   *   bundles therefore triggered at least one heap alloc + free even
   *   when the per-thread high-watermark was already satisfied.
   *
   * Design:
   *   - A thread_local RecvScratch struct holds {char* buf; size_t cap}.
   *     The dtor frees buf when the thread exits.
   *   - EnsureRecvBuf(size) realloc()s only when size > cap (grow path).
   *     On every subsequent call with size <= cap the cached buffer is
   *     reused as-is (high-watermark retained).
   *   - The buffer is intentionally NOT zeroed between calls; callers
   *     that need a cleared buffer can memset() the exact prefix they
   *     will use. ReadBundleList() zeros via memset(buf, 0, need) inside
   *     the loop body.
   *
   * Thread-safety:
   *   Each thread has its own RecvScratch. No locking, no contention.
   *
   * Lifetime note:
   *   The returned pointer is valid only until the next EnsureRecvBuf()
   *   call on the SAME thread (which may realloc and invalidate it).
   *   Callers must not retain the pointer across thread boundaries.
   */
  EXPORT_API char* EnsureRecvBuf(size_t size);

  /* Returns the current high-watermark capacity of the thread's scratch
   * buffer. Returns 0 if EnsureRecvBuf() has never been called on this
   * thread. Used by ReadBundleList() to avoid re-reading SO_RCVBUF on
   * the hot path. */
  EXPORT_API size_t RecvBufCapacity();

  /* Test-only: release the thread-local scratch buffer. Lets a test
   * exercise the cold/grow paths deterministically regardless of whether
   * a previous TEST on the same thread warmed the cache. */
  EXPORT_API void ResetRecvBufForTest();
}  // namespace util
}  // namespace notification

#endif  // NOTIFICATION_EX_EX_UTIL_H_
