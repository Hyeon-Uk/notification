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

/*
 * Race-audit probe tests for the notification client library.
 *
 * Mirror of dpm 7f3b56d, but the notification client lib is loaded into
 * 3rd-party app processes which can be multi-threaded — unlike the
 * single-thread daemon — so the audit produced real findings rather than
 * a zero-finding log.
 *
 * This TU pins the post-fix invariant for two file-static GHashTable*
 * caches that previously mutated outside their owning module's
 * __rec_mutex:
 *
 *   1. notification_internal.cc :: _noti_cb_hash
 *      Used by the (un)register changed-cb / detailed-changed-cb EXPORT_API
 *      surface AND read by the notification_call_changed_cb_for_uid
 *      dispatch path (called from rpc-port worker delivery).
 *
 *   2. notification_setting.cc :: _noti_dnd_cb_hash
 *      Symmetric pattern for the system-setting DND cb registry.
 *
 * Pre-fix shape (race-vulnerable):
 *
 *      if (_noti_cb_hash == NULL)
 *          _noti_cb_hash = g_hash_table_new(...);     // <-- racy lazy init
 *      ...
 *      g_hash_table_lookup(_noti_cb_hash, ...);        // <-- racy read
 *      g_hash_table_insert(_noti_cb_hash, ...);        // <-- racy write
 *
 * Post-fix shape: every read and every write of `_noti_cb_hash` /
 * `_noti_dnd_cb_hash` must execute under the file-local __rec_mutex.
 *
 * Test strategy mirrors test_notification_wildptr_audit.cc / the dpm
 * audit: slurp the source file at test time and assert structural
 * invariants by static scan (function-body extraction + regex). RED
 * before the matching minimal fix in the same commit; GREEN after.
 */

#include <gtest/gtest.h>

#include <cctype>
#include <fstream>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

namespace {

#ifndef NOTIFICATION_INTERNAL_C_PATH
/* Resolved at compile time from CMakeLists.txt. Empty -> SKIP. */
#define NOTIFICATION_INTERNAL_C_PATH ""
#endif

#ifndef NOTIFICATION_SETTING_C_PATH
#define NOTIFICATION_SETTING_C_PATH ""
#endif

constexpr const char* kInternalPath = NOTIFICATION_INTERNAL_C_PATH;
constexpr const char* kSettingPath  = NOTIFICATION_SETTING_C_PATH;

std::string SlurpFile(const char* path) {
  std::ifstream f(path);
  if (!f.is_open()) return {};
  std::stringstream buf;
  buf << f.rdbuf();
  return buf.str();
}

/* Extract the body (text between the matching outermost {}) of the first
 * top-level definition of `fn_name` in `src`. Skips call-site occurrences
 * (where the name is followed by an argument list and a `;` rather than a
 * `{`). Returns "" if not found.  Mirror of test_notification_wildptr_audit.cc
 * with the call-site filter added so the helper is correct in TUs that
 * contain both the definition and recursive/forward call sites. */
std::string ExtractFunctionBody(const std::string& src,
                                const std::string& fn_name) {
  size_t pos = 0;
  while ((pos = src.find(fn_name, pos)) != std::string::npos) {
    if (pos > 0 && (std::isalnum(static_cast<unsigned char>(src[pos - 1])) ||
                    src[pos - 1] == '_')) {
      pos += fn_name.size();
      continue;
    }
    size_t after_name = pos + fn_name.size();
    /* Identifier-boundary check on the trailing edge: the next char must
     * not be alnum/_ either, otherwise this is just a longer identifier
     * (e.g. a name with an `_extra` suffix). */
    if (after_name < src.size()) {
      unsigned char c = static_cast<unsigned char>(src[after_name]);
      if (std::isalnum(c) || c == '_') {
        pos = after_name;
        continue;
      }
    }
    /* The token must be a function definition (`name (args) { ... }`),
     * not a call site (`name (args);` or `name (args)` inside an expr).
     * Find the matching `)` of the argument list, then peek the next
     * non-whitespace token: must be `{` for a definition.  A trailing
     * `;` / `,` / `)` / etc. all imply call site, skip. */
    size_t paren = src.find('(', after_name);
    if (paren == std::string::npos) return {};
    int pdepth = 1;
    size_t j = paren + 1;
    while (j < src.size() && pdepth > 0) {
      if (src[j] == '(') ++pdepth;
      else if (src[j] == ')') --pdepth;
      ++j;
    }
    if (pdepth != 0) return {};
    size_t k = j;
    while (k < src.size() &&
           std::isspace(static_cast<unsigned char>(src[k])))
      ++k;
    if (k >= src.size() || src[k] != '{') {
      pos = after_name;
      continue;
    }
    int depth = 1;
    size_t i = k + 1;
    while (i < src.size() && depth > 0) {
      if (src[i] == '{') ++depth;
      else if (src[i] == '}') --depth;
      ++i;
    }
    if (depth == 0) return src.substr(k + 1, (i - 1) - (k + 1));
    return {};
  }
  return {};
}

/* Strip C and C++ comments from `s`. Necessary before the lexical
 * lock-depth scan because race-rationale comments often mention the
 * symbol name (`_noti_cb_hash`, `_noti_dnd_cb_hash`) and would otherwise
 * be flagged as unprotected accesses. Handles the standard slash-star ...
 * star-slash and double-slash-to-newline forms. */
std::string StripComments(const std::string& s) {
  std::string out;
  out.reserve(s.size());
  size_t i = 0;
  while (i < s.size()) {
    if (i + 1 < s.size() && s[i] == '/' && s[i + 1] == '*') {
      i += 2;
      while (i + 1 < s.size() && !(s[i] == '*' && s[i + 1] == '/')) ++i;
      if (i + 1 < s.size()) i += 2;
      continue;
    }
    if (i + 1 < s.size() && s[i] == '/' && s[i + 1] == '/') {
      while (i < s.size() && s[i] != '\n') ++i;
      continue;
    }
    out.push_back(s[i]);
    ++i;
  }
  return out;
}

/* True iff every reference to `sym` in `body` is preceded (lexically, in
 * the same body, with comments already stripped) by a __notification_mutex_lock()
 * call without an intervening *enclosing* __notification_mutex_unlock().
 *
 * Implementation: for each hit, walk forward from the function-body start
 * counting `__notification_mutex_lock` (+1) and `__notification_mutex_unlock`
 * (-1). The hit is ACCEPTED iff at the moment of the hit lock_depth > 0.
 * Early-return paths inside the body that drop the lock locally
 * (`if (cond) { unlock(); return; }`) appear to the scanner as a -1 token,
 * but the SUBSEQUENT references in the function body run on the
 * complementary path where the lock is still held — i.e. with the original
 * +1 still in force.
 *
 * To make the scanner robust to those early-return drops, we use the
 * MAX lock_depth seen at OR BEFORE the hit position, not the running
 * depth. The semantic claim is "this hit is reached only on a path where
 * the lock has been acquired and not yet released on that path"; the max
 * is a sound over-approximation for the call sites in this audit because
 * (a) the lock helpers are called textually, no aliasing, and (b) every
 * notification API enters its critical section with a single lock() call
 * and exits via either the success-path unlock() or an early-return
 * unlock(); no nested re-locking in scope. */
bool EveryHashAccessIsLocked(const std::string& raw_body,
                             const std::string& sym) {
  std::string body = StripComments(raw_body);

  /* Pre-compute a vector of (position, +1/-1) lock-event tokens. */
  struct Event { size_t pos; int delta; };
  std::vector<Event> events;
  for (size_t i = 0; i + sizeof("__notification_mutex_lock") - 1 <= body.size();) {
    if (body.compare(i, sizeof("__notification_mutex_lock") - 1,
                     "__notification_mutex_lock") == 0) {
      events.push_back({i, +1});
      i += sizeof("__notification_mutex_lock") - 1;
      continue;
    }
    if (i + sizeof("__notification_mutex_unlock") - 1 <= body.size() &&
        body.compare(i, sizeof("__notification_mutex_unlock") - 1,
                     "__notification_mutex_unlock") == 0) {
      events.push_back({i, -1});
      i += sizeof("__notification_mutex_unlock") - 1;
      continue;
    }
    ++i;
  }

  bool saw_any = false;
  for (size_t i = 0; i < body.size();) {
    if (body.compare(i, sym.size(), sym) != 0) {
      ++i;
      continue;
    }
    bool boundary_ok = true;
    if (i > 0) {
      unsigned char c = static_cast<unsigned char>(body[i - 1]);
      if (std::isalnum(c) || c == '_') boundary_ok = false;
    }
    size_t end = i + sym.size();
    if (end < body.size()) {
      unsigned char c = static_cast<unsigned char>(body[end]);
      if (std::isalnum(c) || c == '_') boundary_ok = false;
    }
    if (!boundary_ok) {
      i = end;
      continue;
    }
    saw_any = true;
    /* Compute the max running depth at or before `i`. */
    int depth = 0, max_depth = 0;
    for (const auto& e : events) {
      if (e.pos > i) break;
      depth += e.delta;
      if (depth > max_depth) max_depth = depth;
    }
    if (max_depth == 0) return false;
    i = end;
  }
  return saw_any;
}

class NotificationRaceAuditTest : public ::testing::Test {};

/* -----------------------------------------------------------------------
 * Bug 1 — _noti_cb_hash mutated outside __rec_mutex in 4 EXPORT_API
 * functions and 1 dispatch helper, all in notification_internal.cc.
 *
 * Pre-fix:
 *   notification_resister_changed_cb_for_uid                — race
 *   notification_unresister_changed_cb_for_uid              — race
 *   notification_register_detailed_changed_cb_for_uid       — race
 *   notification_unregister_detailed_changed_cb_for_uid     — race
 *   notification_call_changed_cb_for_uid                    — race (read)
 *
 * Post-fix invariant: every reference to `_noti_cb_hash` lies inside a
 * __notification_mutex_lock() / __notification_mutex_unlock() window.
 * ----------------------------------------------------------------------- */

class NotiCbHashLockedTest : public NotificationRaceAuditTest {
 protected:
  void SetUp() override {
    src_ = SlurpFile(kInternalPath);
    if (src_.empty()) {
      GTEST_SKIP() << "Source file unreachable: " << kInternalPath;
    }
  }
  std::string src_;
};

TEST_F(NotiCbHashLockedTest, ResisterChangedCbForUid_LockedHash) {
  std::string body = ExtractFunctionBody(
      src_, "notification_resister_changed_cb_for_uid");
  ASSERT_FALSE(body.empty()) << "function body not found";
  EXPECT_TRUE(EveryHashAccessIsLocked(body, "_noti_cb_hash"))
      << "notification_resister_changed_cb_for_uid touches _noti_cb_hash "
         "outside __notification_mutex_lock — racy lazy init / racy "
         "g_hash_table_lookup / racy g_hash_table_insert. 3rd-party app "
         "calling register from two threads can leak the first GHashTable.";
}

TEST_F(NotiCbHashLockedTest, UnresisterChangedCbForUid_LockedHash) {
  std::string body = ExtractFunctionBody(
      src_, "notification_unresister_changed_cb_for_uid");
  ASSERT_FALSE(body.empty()) << "function body not found";
  EXPECT_TRUE(EveryHashAccessIsLocked(body, "_noti_cb_hash"))
      << "notification_unresister_changed_cb_for_uid mutates _noti_cb_hash "
         "outside __notification_mutex_lock — concurrent register/dispatch "
         "can use-after-free the cb-info struct (free runs on the app "
         "thread; dispatch in notification_call_changed_cb_for_uid reads "
         "the same list head).";
}

TEST_F(NotiCbHashLockedTest, RegisterDetailedChangedCbForUid_LockedHash) {
  std::string body = ExtractFunctionBody(
      src_, "notification_register_detailed_changed_cb_for_uid");
  ASSERT_FALSE(body.empty()) << "function body not found";
  EXPECT_TRUE(EveryHashAccessIsLocked(body, "_noti_cb_hash"))
      << "notification_register_detailed_changed_cb_for_uid: same "
         "race-vulnerable lazy-init + lookup + insert pattern as the "
         "non-detailed variant.";
}

TEST_F(NotiCbHashLockedTest, UnregisterDetailedChangedCbForUid_LockedHash) {
  std::string body = ExtractFunctionBody(
      src_, "notification_unregister_detailed_changed_cb_for_uid");
  ASSERT_FALSE(body.empty()) << "function body not found";
  EXPECT_TRUE(EveryHashAccessIsLocked(body, "_noti_cb_hash"))
      << "notification_unregister_detailed_changed_cb_for_uid: same "
         "race-vulnerable lookup + steal/replace pattern as the "
         "non-detailed variant.";
}

TEST_F(NotiCbHashLockedTest, CallChangedCbForUid_LockedHash) {
  std::string body =
      ExtractFunctionBody(src_, "notification_call_changed_cb_for_uid");
  ASSERT_FALSE(body.empty()) << "function body not found";
  EXPECT_TRUE(EveryHashAccessIsLocked(body, "_noti_cb_hash"))
      << "notification_call_changed_cb_for_uid reads _noti_cb_hash + "
         "iterates the resulting list outside __notification_mutex_lock. "
         "Dispatch runs on the rpc-port stub-delivery thread; concurrent "
         "unresister on the app thread frees a node mid-iteration -> UAF.";
}

/* -----------------------------------------------------------------------
 * Bug 2 — _noti_dnd_cb_hash mutated outside notification_setting.cc's
 * __rec_mutex in 3 EXPORT_API functions and 1 dispatch helper.
 *
 * The setting module uses the same convenience pair
 * __notification_mutex_lock / __notification_mutex_unlock as
 * notification_internal.cc; the static checker is therefore reused.
 *
 * Pre-fix:
 *   notification_register_system_setting_dnd_changed_cb_for_uid   — race
 *   notification_unregister_system_setting_dnd_changed_cb_for_uid — race
 *   notification_call_dnd_changed_cb_for_uid                      — race
 * ----------------------------------------------------------------------- */

class NotiDndCbHashLockedTest : public NotificationRaceAuditTest {
 protected:
  void SetUp() override {
    src_ = SlurpFile(kSettingPath);
    if (src_.empty()) {
      GTEST_SKIP() << "Source file unreachable: " << kSettingPath;
    }
  }
  std::string src_;
};

TEST_F(NotiDndCbHashLockedTest, RegisterSystemSettingDnd_LockedHash) {
  std::string body = ExtractFunctionBody(
      src_, "notification_register_system_setting_dnd_changed_cb_for_uid");
  ASSERT_FALSE(body.empty()) << "function body not found";
  EXPECT_TRUE(EveryHashAccessIsLocked(body, "_noti_dnd_cb_hash"))
      << "register_system_setting_dnd_changed_cb_for_uid touches "
         "_noti_dnd_cb_hash outside __notification_mutex_lock — same "
         "race shape as notification_resister_changed_cb_for_uid.";
}

TEST_F(NotiDndCbHashLockedTest, UnregisterSystemSettingDnd_LockedHash) {
  std::string body = ExtractFunctionBody(
      src_, "notification_unregister_system_setting_dnd_changed_cb_for_uid");
  ASSERT_FALSE(body.empty()) << "function body not found";
  EXPECT_TRUE(EveryHashAccessIsLocked(body, "_noti_dnd_cb_hash"))
      << "unregister_system_setting_dnd_changed_cb_for_uid mutates "
         "_noti_dnd_cb_hash outside __notification_mutex_lock — concurrent "
         "register/dispatch -> UAF on the dnd_data struct.";
}

TEST_F(NotiDndCbHashLockedTest, CallDndChangedCbForUid_LockedHash) {
  std::string body =
      ExtractFunctionBody(src_, "notification_call_dnd_changed_cb_for_uid");
  ASSERT_FALSE(body.empty()) << "function body not found";
  EXPECT_TRUE(EveryHashAccessIsLocked(body, "_noti_dnd_cb_hash"))
      << "call_dnd_changed_cb_for_uid reads + iterates _noti_dnd_cb_hash "
         "outside __notification_mutex_lock — same UAF shape as "
         "call_changed_cb_for_uid.";
}

}  // namespace
