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
 * Wild-pointer / uninitialized-handle audit (mirror of dpm 0ec37bb).
 *
 * The dpm-side bug class:
 *
 *     notification_h noti;                       // uninitialized stack slot
 *     ret = make_noti_from_notification(&noti, h);
 *     // helper has an OOM branch that returns without writing *noti
 *     ...
 *     notification_free(noti);                   // wild-pointer free
 *
 * In the notification client EXPORT_API surface this audit found one
 * mirror occurrence: notification_tidl_request_load_system_setting in
 * src/notification/src/notification_tidl.cc.  The shape there:
 *
 *     notification_system_setting_h _setting;    // uninitialized
 *     ...
 *     make_setting_from_noti_system_setting(&_setting, system_setting);
 *     // ^^^ return value IGNORED.  Helper has an OOM branch that returns
 *     //     NOTIFICATION_ERROR_OUT_OF_MEMORY without writing *setting.
 *     *setting = _setting;                       // wild pointer leaks out
 *
 * The caller (noti_system_setting_load_system_setting -> _handle_do_not_disturb_option
 * in notification_noti.cc) initializes its local to NULL but then unconditionally
 * overwrites it with the wild *setting on the silent-OOM path; the next access
 * is either a deref of garbage (line 862) or a notification_system_setting_free
 * on garbage (line 907).
 *
 * Test strategy — same as the existing test_notification_tidl_wrapper.cc /
 * test_notification_internal_tidl_wrapper.cc mirror tests: slurp the source
 * file at test time and pin the post-fix invariant by static scan. The fix
 * either (a) initializes _setting = NULL at declaration AND propagates the
 * helper's return code, or (b) checks the helper's return code and bails
 * before the *setting = _setting; assignment.
 *
 * The probe pattern is simple: for every uninitialized opaque-pointer local
 * declaration in notification_tidl.cc, if a helper-out-arg call follows
 * with the same variable, the function body must either contain the variable
 * being initialized to NULL on declaration OR check the helper's return code.
 *
 * Production code is not changed by this TU; the matching minimal patch
 * lives in the same commit.
 */

#include <gtest/gtest.h>

#include <cctype>
#include <fstream>
#include <regex>
#include <sstream>
#include <string>

namespace {

#ifndef NOTIFICATION_TIDL_C_PATH
#define NOTIFICATION_TIDL_C_PATH ""
#endif
constexpr const char* kTidlPath = NOTIFICATION_TIDL_C_PATH;

std::string SlurpFile(const char* path) {
  std::ifstream f(path);
  if (!f.is_open()) return {};
  std::stringstream buf;
  buf << f.rdbuf();
  return buf.str();
}

std::string ExtractFunctionBody(const std::string& src,
                                const std::string& fn_name) {
  size_t pos = 0;
  while ((pos = src.find(fn_name, pos)) != std::string::npos) {
    if (pos > 0 && (std::isalnum(static_cast<unsigned char>(src[pos - 1])) ||
                    src[pos - 1] == '_')) {
      pos += fn_name.size();
      continue;
    }
    size_t paren = src.find('(', pos);
    size_t brace = src.find('{', pos);
    if (paren == std::string::npos || brace == std::string::npos) return {};
    int depth = 1;
    size_t i = brace + 1;
    while (i < src.size() && depth > 0) {
      if (src[i] == '{') ++depth;
      else if (src[i] == '}') --depth;
      ++i;
    }
    if (depth == 0) return src.substr(brace + 1, (i - 1) - (brace + 1));
    return {};
  }
  return {};
}

class NotificationWildPtrAuditTest : public ::testing::Test {
 protected:
  void SetUp() override {
    src_ = SlurpFile(kTidlPath);
    if (src_.empty()) {
      GTEST_SKIP() << "Source file unreachable: " << kTidlPath;
    }
  }
  std::string src_;
};

/* Post-fix invariant: notification_tidl_request_load_system_setting must
 * either declare _setting = NULL OR check the return code of
 * make_setting_from_noti_system_setting before writing *setting = _setting.
 *
 * RED before the fix: declaration is plain `notification_system_setting_h _setting;`
 * and the call is `make_setting_from_noti_system_setting(&_setting, system_setting);`
 * with the return value dropped. */
TEST_F(NotificationWildPtrAuditTest, LoadSystemSetting_HelperReturnChecked) {
  std::string body =
      ExtractFunctionBody(src_, "notification_tidl_request_load_system_setting");
  ASSERT_FALSE(body.empty())
      << "could not find notification_tidl_request_load_system_setting body";

  /* Either the declaration is NULL-initialized... */
  bool null_init = std::regex_search(
      body,
      std::regex(R"(notification_system_setting_h\s+_setting\s*=\s*NULL\s*;)"));

  /* ...or the helper return is checked before the *setting = _setting
   * assignment. We grep for "ret = make_setting_from_noti_system_setting"
   * and verify a NOTIFICATION_ERROR_NONE comparison appears between that
   * call and the next "*setting = _setting" assignment. */
  bool ret_checked = false;
  std::smatch m;
  if (std::regex_search(
          body, m,
          std::regex(R"(ret\s*=\s*make_setting_from_noti_system_setting\s*\()"))) {
    size_t after = m.position(0) + m.length(0);
    size_t assign = body.find("*setting = _setting", after);
    if (assign != std::string::npos) {
      std::string between = body.substr(after, assign - after);
      ret_checked = std::regex_search(
          between,
          std::regex(R"(if\s*\(\s*ret\s*!=\s*NOTIFICATION_ERROR_NONE)"));
    }
  }

  EXPECT_TRUE(null_init || ret_checked)
      << "notification_tidl_request_load_system_setting still has the "
         "wild-pointer pattern: _setting is declared uninitialized AND "
         "make_setting_from_noti_system_setting's return value is dropped, "
         "so a silent OOM in the helper propagates an uninitialized stack "
         "slot to *setting (mirror of dpm 0ec37bb).";
}

/* Defense-in-depth: pin that on the OOM/error path nothing reaches
 * `*setting = _setting`.  The fix may also wrap the assignment in a
 * success-only branch, which is equally acceptable. */
TEST_F(NotificationWildPtrAuditTest,
       LoadSystemSetting_OutAssignmentGuarded) {
  std::string body =
      ExtractFunctionBody(src_, "notification_tidl_request_load_system_setting");
  ASSERT_FALSE(body.empty());

  /* The fix: either NULL-initialize _setting, or guard the *setting = _setting
   * assignment with a success check.  Pre-fix: neither holds. */
  bool null_init = std::regex_search(
      body,
      std::regex(R"(notification_system_setting_h\s+_setting\s*=\s*NULL\s*;)"));
  bool guarded_assign = std::regex_search(
      body,
      std::regex(R"(if\s*\(\s*ret\s*==\s*NOTIFICATION_ERROR_NONE\s*\)\s*\{?[^}]*\*setting\s*=\s*_setting)"));

  EXPECT_TRUE(null_init || guarded_assign)
      << "Either NULL-init _setting or guard *setting = _setting on success.";
}

}  // namespace
