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

namespace notification {
namespace util {
  std::string GetAppId();
  std::string GetAppId(pid_t pid);
  int GetRequestId();
  std::string GetPkgId();
  std::string GetDomainName();
  std::string GetLocaleDirectory();
  unsigned int GetSocketBufferSize(int fd, int optname);
  unsigned int ReadSocket(int fd, char* buffer, unsigned int nbytes);
  int WriteSocket(int fd, const char* buffer, unsigned int nbytes);
  std::list<tizen_base::Bundle> ReadBundleList(int recvfd, int noti_cnt);
  int ConvertDBusError(int error);

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
  GVariantBuilder* ScratchBuilder(const GVariantType* type);

  /* Test-only: reset the current thread's scratch builder. Allows unit
   * tests to isolate builder-reuse allocation counts across TESTs without
   * relying on test execution order. */
  void ResetScratchBuilderForTest();

  /* Test-only: returns how many times ScratchBuilder() has had to
   * g_variant_builder_new() on the current thread (i.e. cold path count).
   * On a reused-thread hot path this should stay at 1. */
  int GetScratchBuilderAllocCountForTest();
}  // namespace util
}  // namespace notification

#endif  // NOTIFICATION_EX_EX_UTIL_H_
