/*
 * Copyright (c) 2026 Samsung Electronics Co., Ltd.
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

#include "notification-ex/appid_lru.h"

#include <aul.h>
#include <dlog.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#include <array>
#include <cstdio>
#include <cstring>
#include <mutex>

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "NOTIFICATION_EX"

/* LCOV_EXCL_START */
namespace notification {
namespace util {
namespace detail {

namespace {

struct Slot {
  pid_t pid = 0;            /* 0 == empty slot sentinel (no real pid is 0) */
  int   len = 0;            /* cached strlen to avoid repeated strlen() */
  char  appid[kAppIdMaxLen] = {0};
};

/*
 * The slots array is indexed 0..kAppIdSlotCount-1 from most-recently-
 * used (index 0) to least-recently-used (index kAppIdSlotCount-1).
 * Every hit moves the resolved slot to index 0 with a single memmove;
 * every miss overwrites index kAppIdSlotCount-1 and then shuffles it
 * to the front. The slot layout is therefore a pure array — no linked
 * list, no heap nodes.
 */
std::array<Slot, kAppIdSlotCount> g_slots;

/* Mutex guards g_slots. The appid cache is shared across threads
 * because the old std::map version was also shared and pre-existing
 * callers (reporter + manager across dbus dispatch threads) rely on
 * that. The critical section is O(kAppIdSlotCount) == O(1). */
std::mutex g_mutex;

constexpr int kMaxPackageStrSize = 512;

/* Move slot at index `idx` to the front of the LRU (index 0). */
void PromoteToFront(int idx) {
  if (idx == 0)
    return;
  Slot promoted = g_slots[idx];
  /* Shift 0..idx-1 down by one, then install promoted at 0. */
  for (int i = idx; i > 0; --i)
    g_slots[i] = g_slots[i - 1];
  g_slots[0] = promoted;
}

/* Copy src (len bytes, NUL-terminated or length-bounded) into dst
 * bounded by kAppIdMaxLen-1. dst[len] is forced to 0. */
int StoreAppId(Slot& slot, pid_t pid, const char* src, int src_len) {
  int copy = src_len;
  if (copy >= kAppIdMaxLen)
    copy = kAppIdMaxLen - 1;
  if (copy < 0)
    copy = 0;
  std::memcpy(slot.appid, src, copy);
  slot.appid[copy] = 0;
  slot.pid = pid;
  slot.len = copy;
  return copy;
}

/* Resolve the appid via aul_app_get_appid_bypid + /proc/<pid>/cmdline
 * fallback exactly like the pre-N3-T1 helper. Writes the result into
 * `out_buf` (a kAppIdMaxLen buffer owned by the caller) and returns
 * the written length (0 on failure). */
int ResolveAppId(pid_t pid, char* out_buf) {
  char appid_buf[kMaxPackageStrSize] = {0};
  int ret = aul_app_get_appid_bypid(pid, appid_buf, sizeof(appid_buf));
  if (ret == AUL_R_OK) {
    int len = static_cast<int>(std::strlen(appid_buf));
    /* "d::org.tizen.appfw.service.esd" → "data-provider-master" rewrite
     * preserved from the original GetAppId(). */
    static const char kEsd[]  = "d::org.tizen.appfw.service.esd";
    static const char kDpm[]  = "data-provider-master";
    if (len == static_cast<int>(sizeof(kEsd) - 1) &&
        std::memcmp(appid_buf, kEsd, sizeof(kEsd) - 1) == 0) {
      std::memcpy(out_buf, kDpm, sizeof(kDpm));
      return static_cast<int>(sizeof(kDpm) - 1);
    }
    int copy = len;
    if (copy >= kAppIdMaxLen)
      copy = kAppIdMaxLen - 1;
    std::memcpy(out_buf, appid_buf, copy);
    out_buf[copy] = 0;
    return copy;
  }

  /* AUL miss → /proc/<pid>/cmdline fallback. */
  char proc_buf[kMaxPackageStrSize] = {0};
  std::snprintf(proc_buf, sizeof(proc_buf), "/proc/%d/cmdline", pid);
  int fd = open(proc_buf, O_RDONLY);
  if (fd < 0) {
    LOGE("Fail to get appid (%d)", errno);
    out_buf[0] = 0;
    return 0;
  }
  int n = read(fd, appid_buf, sizeof(appid_buf) - 1);
  close(fd);
  if (n <= 0) {
    LOGE("Fail to get appid (%d)", errno);
    out_buf[0] = 0;
    return 0;
  }

  int last_slash = 0;
  for (int i = 0; i < n; ++i) {
    if (appid_buf[i] == '/')
      last_slash = i;
  }
  if (last_slash == n - 1) {
    LOGE("Fail to get appid (%s)", appid_buf);
    out_buf[0] = 0;
    return 0;
  }

  const char* start = (last_slash == 0) ? appid_buf
                                        : &appid_buf[last_slash + 1];
  int len = static_cast<int>(std::strlen(start));
  int copy = len;
  if (copy >= kAppIdMaxLen)
    copy = kAppIdMaxLen - 1;
  std::memcpy(out_buf, start, copy);
  out_buf[copy] = 0;
  return copy;
}

}  // namespace

std::string_view LookupOrFillAppId(pid_t pid) {
  std::lock_guard<std::mutex> lk(g_mutex);

  /* Fast path: linear scan ≤ 8 slots. */
  for (int i = 0; i < kAppIdSlotCount; ++i) {
    if (g_slots[i].pid == pid && g_slots[i].pid != 0) {
      PromoteToFront(i);
      return std::string_view(g_slots[0].appid,
                              static_cast<size_t>(g_slots[0].len));
    }
  }

  /* Miss path: resolve into a scratch buffer, then install at the LRU
   * slot (kAppIdSlotCount - 1) and promote to front. Resolving first
   * avoids writing a corrupt slot if the AUL call fails. */
  char scratch[kAppIdMaxLen] = {0};
  int len = ResolveAppId(pid, scratch);
  if (len <= 0) {
    /* Return empty view — callers expect "" on failure. The slot is
     * NOT touched so a transient resolve error does not evict a hot
     * pid. */
    return std::string_view();
  }

  /* Install at the LRU tail, then promote. */
  StoreAppId(g_slots[kAppIdSlotCount - 1], pid, scratch, len);
  PromoteToFront(kAppIdSlotCount - 1);
  LOGI("appid(%s)", g_slots[0].appid);
  return std::string_view(g_slots[0].appid,
                          static_cast<size_t>(g_slots[0].len));
}

void ResetAppidLruForTest() {
  std::lock_guard<std::mutex> lk(g_mutex);
  for (auto& s : g_slots) {
    s.pid = 0;
    s.len = 0;
    s.appid[0] = 0;
  }
}

int OccupiedSlotCountForTest() {
  std::lock_guard<std::mutex> lk(g_mutex);
  int n = 0;
  for (auto& s : g_slots)
    if (s.pid != 0)
      ++n;
  return n;
}

}  // namespace detail
}  // namespace util
}  // namespace notification
/* LCOV_EXCL_STOP */
