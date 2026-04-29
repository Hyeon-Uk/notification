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
 * Race-audit probe tests, pass 2.
 *
 * Sibling of test_notification_race_audit.cc.  That TU covers the
 * 8 _noti_cb_hash / _noti_dnd_cb_hash bugs in commit 5115623.  This
 * TU pins the post-fix invariant for the four file-statics flagged
 * out-of-scope in pass 1 but real on closer reading:
 *
 *   1. notification.cc    — _pkg_id / _locale_directory / _label /
 *                           _cb_registered, init-once-then-cached
 *                           by _notification_create AND mutated by
 *                           the vconf language-change callback that
 *                           runs on a separate GMainContext thread.
 *   2. notification_viewer.cc — _delayed_noti_list head read at the
 *                           top of __launch_default_viewer outside
 *                           __rec_mutex, racing the timer-driven
 *                           __pop_delayed_noti_cb and __push paths.
 *   3. notification_status.cc — file-static `md` struct touched by
 *                           the dbus signal callback (GMainContext
 *                           thread) AND by the EXPORT_API _set /
 *                           _unset entries (app thread).
 *   4. notification_db.cc — `is_db_corrupted` flag never reset
 *                           between calls; sticky `true` re-enters
 *                           recovery on a later healthy invocation.
 *
 * Test strategy: slurp the source file at test time and assert
 * structural invariants by static scan (function-body extraction
 * + lock-depth counter + targeted regex).  Mirror of
 * test_notification_race_audit.cc and test_notification_wildptr_audit.cc.
 * RED before the matching minimal patch in the same commit; GREEN after.
 * SKIP-on-absent contract preserved.
 */

#include <gtest/gtest.h>

#include <cctype>
#include <fstream>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

namespace {

#ifndef NOTIFICATION_C_PATH
#define NOTIFICATION_C_PATH ""
#endif
#ifndef NOTIFICATION_VIEWER_C_PATH
#define NOTIFICATION_VIEWER_C_PATH ""
#endif
#ifndef NOTIFICATION_STATUS_C_PATH
#define NOTIFICATION_STATUS_C_PATH ""
#endif
#ifndef NOTIFICATION_DB_C_PATH
#define NOTIFICATION_DB_C_PATH ""
#endif

constexpr const char* kNotificationPath = NOTIFICATION_C_PATH;
constexpr const char* kViewerPath       = NOTIFICATION_VIEWER_C_PATH;
constexpr const char* kStatusPath       = NOTIFICATION_STATUS_C_PATH;
constexpr const char* kDbPath           = NOTIFICATION_DB_C_PATH;

std::string SlurpFile(const char* path) {
  std::ifstream f(path);
  if (!f.is_open()) return {};
  std::stringstream buf;
  buf << f.rdbuf();
  return buf.str();
}

/* Strip C and C++ comments — race-rationale comments mention the
 * symbol names and would otherwise be flagged as unprotected
 * accesses by the lexical scanner. */
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

/* Extract the body (text between the matching outermost {}) of the
 * first top-level definition of `fn_name` in `src`. Skips call sites. */
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
    if (after_name < src.size()) {
      unsigned char c = static_cast<unsigned char>(src[after_name]);
      if (std::isalnum(c) || c == '_') {
        pos = after_name;
        continue;
      }
    }
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

/* True iff every word-boundary occurrence of `sym` in `body` lies
 * inside a lock window opened by `lock_fn` and closed by `unlock_fn`.
 * Same max-running-depth approach as test_notification_race_audit.cc:
 * tolerant of early-return paths that drop the lock locally before a
 * `goto out / return`. */
bool EveryAccessIsLocked(const std::string& raw_body,
                         const std::string& sym,
                         const std::string& lock_fn,
                         const std::string& unlock_fn) {
  std::string body = StripComments(raw_body);

  struct Event { size_t pos; int delta; };
  std::vector<Event> events;
  for (size_t i = 0; i < body.size();) {
    if (i + lock_fn.size() <= body.size() &&
        body.compare(i, lock_fn.size(), lock_fn) == 0) {
      events.push_back({i, +1});
      i += lock_fn.size();
      continue;
    }
    if (i + unlock_fn.size() <= body.size() &&
        body.compare(i, unlock_fn.size(), unlock_fn) == 0) {
      events.push_back({i, -1});
      i += unlock_fn.size();
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

/* -----------------------------------------------------------------------
 * Bug 1 — notification.cc :: _pkg_id / _locale_directory / _label /
 * _cb_registered file-statics, mutated by both _notification_create
 * (app thread) and _language_key_changed_cb (vconf GMainContext thread).
 *
 * Post-fix invariant: every read AND every write of the four cache
 * fields lies inside a __notification_cache_lock window in both the
 * create-side function and the vconf callback.
 * ----------------------------------------------------------------------- */

class NotificationCacheLockedTest : public ::testing::Test {
 protected:
  void SetUp() override {
    src_ = SlurpFile(kNotificationPath);
    if (src_.empty())
      GTEST_SKIP() << "Source file unreachable: " << kNotificationPath;
  }
  std::string src_;
};

TEST_F(NotificationCacheLockedTest, NotificationCreate_PkgIdLocked) {
  std::string body = ExtractFunctionBody(src_, "_notification_create");
  ASSERT_FALSE(body.empty()) << "function body not found";
  EXPECT_TRUE(EveryAccessIsLocked(body, "_pkg_id",
                                  "__notification_cache_lock",
                                  "__notification_cache_unlock"))
      << "_notification_create touches _pkg_id outside "
         "__notification_cache_lock — racing creators on two app threads "
         "each see _pkg_id == NULL, both strdup, the second leaks the first.";
}

TEST_F(NotificationCacheLockedTest, NotificationCreate_LocaleDirectoryLocked) {
  std::string body = ExtractFunctionBody(src_, "_notification_create");
  ASSERT_FALSE(body.empty()) << "function body not found";
  EXPECT_TRUE(EveryAccessIsLocked(body, "_locale_directory",
                                  "__notification_cache_lock",
                                  "__notification_cache_unlock"))
      << "_notification_create touches _locale_directory outside "
         "__notification_cache_lock — same lazy-init race as _pkg_id.";
}

TEST_F(NotificationCacheLockedTest, NotificationCreate_LabelLocked) {
  std::string body = ExtractFunctionBody(src_, "_notification_create");
  ASSERT_FALSE(body.empty()) << "function body not found";
  EXPECT_TRUE(EveryAccessIsLocked(body, "_label",
                                  "__notification_cache_lock",
                                  "__notification_cache_unlock"))
      << "_notification_create reads _label outside "
         "__notification_cache_lock — concurrent vconf callback runs "
         "free(_label); _label = strdup(...) — UAF on the read side.";
}

TEST_F(NotificationCacheLockedTest, NotificationCreate_CbRegisteredLocked) {
  std::string body = ExtractFunctionBody(src_, "_notification_create");
  ASSERT_FALSE(body.empty()) << "function body not found";
  EXPECT_TRUE(EveryAccessIsLocked(body, "_cb_registered",
                                  "__notification_cache_lock",
                                  "__notification_cache_unlock"))
      << "_notification_create reads/writes _cb_registered outside "
         "__notification_cache_lock — two threads can both pass the "
         "if (_cb_registered) gate and both call vconf_notify_key_changed, "
         "yielding double-registration.";
}

TEST_F(NotificationCacheLockedTest, LanguageKeyChangedCb_LabelLocked) {
  std::string body = ExtractFunctionBody(src_, "_language_key_changed_cb");
  ASSERT_FALSE(body.empty()) << "function body not found";
  EXPECT_TRUE(EveryAccessIsLocked(body, "_label",
                                  "__notification_cache_lock",
                                  "__notification_cache_unlock"))
      << "_language_key_changed_cb writes _label outside "
         "__notification_cache_lock — racing _notification_create on "
         "the app thread can read the freed pointer (UAF) or strdup it.";
}

/* -----------------------------------------------------------------------
 * Bug 2 — notification_viewer.cc :: _delayed_noti_list head read at the
 * top of __launch_default_viewer outside __rec_mutex.
 *
 * Post-fix invariant: every reference to _delayed_noti_list lies inside
 * a __notification_mutex_lock window.
 * ----------------------------------------------------------------------- */

class NotificationViewerLockedTest : public ::testing::Test {
 protected:
  void SetUp() override {
    src_ = SlurpFile(kViewerPath);
    if (src_.empty())
      GTEST_SKIP() << "Source file unreachable: " << kViewerPath;
  }
  std::string src_;
};

TEST_F(NotificationViewerLockedTest, LaunchDefaultViewer_DelayedListLocked) {
  std::string body = ExtractFunctionBody(src_, "__launch_default_viewer");
  ASSERT_FALSE(body.empty()) << "function body not found";
  EXPECT_TRUE(EveryAccessIsLocked(body, "_delayed_noti_list",
                                  "__notification_mutex_lock",
                                  "__notification_mutex_unlock"))
      << "__launch_default_viewer reads _delayed_noti_list outside "
         "__notification_mutex_lock — racing the timer-driven "
         "__pop_delayed_noti_cb (which pops under the lock) on the "
         "tizen-core source-dispatch thread.";
}

/* -----------------------------------------------------------------------
 * Bug 3 — notification_status.cc :: file-static `md` struct touched
 * by the dbus signal callback (GMainContext thread) AND by the two
 * EXPORT_API entry points (_set / _unset).
 *
 * Post-fix invariant: every reference to `md` (the literal struct
 * name, before any `.field` deref) lies inside a
 * __notification_status_lock window in all three functions.
 * ----------------------------------------------------------------------- */

class NotificationStatusLockedTest : public ::testing::Test {
 protected:
  void SetUp() override {
    src_ = SlurpFile(kStatusPath);
    if (src_.empty())
      GTEST_SKIP() << "Source file unreachable: " << kStatusPath;
  }
  std::string src_;
};

TEST_F(NotificationStatusLockedTest, DbusCallback_MdLocked) {
  std::string body = ExtractFunctionBody(
      src_, "__notification_status_message_dbus_callback");
  ASSERT_FALSE(body.empty()) << "function body not found";
  EXPECT_TRUE(EveryAccessIsLocked(body, "md",
                                  "__notification_status_lock",
                                  "__notification_status_unlock"))
      << "__notification_status_message_dbus_callback reads md.callback "
         "/ md.data outside __notification_status_lock — racing the "
         "_set / _unset entry points on the app thread can free "
         "user_data mid-dispatch (UAF in the user callback).";
}

TEST_F(NotificationStatusLockedTest, MonitorCbSet_MdLocked) {
  std::string body = ExtractFunctionBody(
      src_, "notification_status_monitor_message_cb_set");
  ASSERT_FALSE(body.empty()) << "function body not found";
  EXPECT_TRUE(EveryAccessIsLocked(body, "md",
                                  "__notification_status_lock",
                                  "__notification_status_unlock"))
      << "notification_status_monitor_message_cb_set mutates md outside "
         "__notification_status_lock — torn writes observable from the "
         "dbus dispatch thread.";
}

TEST_F(NotificationStatusLockedTest, MonitorCbUnset_MdLocked) {
  std::string body = ExtractFunctionBody(
      src_, "notification_status_monitor_message_cb_unset");
  ASSERT_FALSE(body.empty()) << "function body not found";
  EXPECT_TRUE(EveryAccessIsLocked(body, "md",
                                  "__notification_status_lock",
                                  "__notification_status_unlock"))
      << "notification_status_monitor_message_cb_unset clears md outside "
         "__notification_status_lock — racing the dbus callback can "
         "dispatch through a stale callback / freed user_data.";
}

/* -----------------------------------------------------------------------
 * Bug 4 — notification_db.cc :: `is_db_corrupted` flag never reset
 * between calls.  Sticky `true` from a previous invocation forces the
 * recovery path on a later healthy call.  Lightweight invariant: the
 * function must reset `is_db_corrupted` to false at entry, before any
 * read of the flag.
 * ----------------------------------------------------------------------- */

class NotificationDbStickyFlagTest : public ::testing::Test {
 protected:
  void SetUp() override {
    src_ = SlurpFile(kDbPath);
    if (src_.empty())
      GTEST_SKIP() << "Source file unreachable: " << kDbPath;
  }
  std::string src_;
};

TEST_F(NotificationDbStickyFlagTest, DbInit_ResetsIsCorruptedFlag) {
  std::string body = ExtractFunctionBody(src_, "notification_db_init");
  ASSERT_FALSE(body.empty()) << "function body not found";
  std::string clean = StripComments(body);

  /* The reset must come before any read of the flag.  Find the first
   * write `is_db_corrupted = false;` (whitespace-tolerant) and the
   * first read site that consumes the flag in a boolean context. */
  std::regex reset_re(R"(is_db_corrupted\s*=\s*false\s*;)");
  std::smatch reset_m;
  ASSERT_TRUE(std::regex_search(clean, reset_m, reset_re))
      << "notification_db_init must reset is_db_corrupted = false at "
         "entry — without the reset, a previously corrupted-then-"
         "recovered DB leaves the flag stuck at true and a later "
         "notification_db_init() incorrectly re-enters "
         "__recover_corrupted_db on a healthy DB.";
  size_t reset_pos = static_cast<size_t>(reset_m.position(0));

  /* Find the earliest READ of is_db_corrupted (i.e. occurrence not
   * immediately followed by `=` that is not `==`). */
  size_t first_read = std::string::npos;
  size_t i = 0;
  while (i < clean.size()) {
    size_t hit = clean.find("is_db_corrupted", i);
    if (hit == std::string::npos) break;
    /* word boundary */
    bool ok = true;
    if (hit > 0) {
      unsigned char c = static_cast<unsigned char>(clean[hit - 1]);
      if (std::isalnum(c) || c == '_') ok = false;
    }
    size_t end = hit + sizeof("is_db_corrupted") - 1;
    if (end < clean.size()) {
      unsigned char c = static_cast<unsigned char>(clean[end]);
      if (std::isalnum(c) || c == '_') ok = false;
    }
    if (!ok) {
      i = end;
      continue;
    }
    /* Skip whitespace after the name; if the next non-ws char is `=`
     * AND the char after that is NOT `=`, this is a write — skip. */
    size_t k = end;
    while (k < clean.size() &&
           std::isspace(static_cast<unsigned char>(clean[k])))
      ++k;
    bool is_write = (k < clean.size() && clean[k] == '=' &&
                     (k + 1 >= clean.size() || clean[k + 1] != '='));
    if (!is_write) {
      first_read = hit;
      break;
    }
    i = end;
  }

  if (first_read != std::string::npos) {
    EXPECT_LT(reset_pos, first_read)
        << "notification_db_init reads is_db_corrupted before resetting "
           "it — sticky-true bug remains.";
  }
}

}  // namespace
