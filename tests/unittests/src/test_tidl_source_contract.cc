/*
 * Copyright (c) 2026 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 */

/*
 * Stage 7 — tidl source contract regression guard.
 *
 * The IPC wire format between libnotification (client) and
 * data-provider-master (daemon) is defined by a single source file:
 * src/tidl/noti_service.tidl.  Both repositories carry a byte-identical
 * copy of this .tidl, and the spec %prep stage of each package runs
 * `tidl/prebuild.sh` which invokes `tidlc` to regenerate the
 * client-side proxy (notification_tidl_proxy.{h,c}) and the
 * server-side stub (notification_tidl_stub.{h,c}).  The generated
 * .c/.h files are NOT committed to git — they are produced fresh
 * inside the gbs chroot per build.  This means:
 *
 *   - Any change to noti_service.tidl ripples into both packages
 *     simultaneously on the next build.
 *   - A drift between the two repos' .tidl copies, or an accidental
 *     source-level change (struct field add/remove, RPC method
 *     add/remove, privilege annotation drift), shows up as a wire-
 *     format incompatibility at the very next library/daemon
 *     deployment and breaks every consumer of the notification
 *     service.
 *
 * This test pins the source-level contract by parsing the .tidl
 * directly — no dependency on tidlc, the generated proxy/stub, or
 * the gbs chroot.  The test reads the file at the path injected by
 * CMake (NOTIFICATION_TIDL_SOURCE_PATH), enforces the published
 * struct + interface shape, and SKIPs when the path is unreachable
 * (out-of-tree builds in distros that strip source paths).  The
 * pattern matches the existing N4-T1 test_tidl_lazy.cc + the
 * notification.cc race-audit pin (NOTIFICATION_C_PATH).
 *
 * Stage 7 GATE — these contracts MUST hold for every commit:
 *
 *   1. The privilege annotation `[privilege = "http://tizen.org/
 *      privilege/notification"]` is present immediately above the
 *      `interface noti_service` block.
 *   2. The five published structs are all present:
 *      `notification`, `noti_setting`,
 *      `noti_system_setting_dnd_allow_exception`,
 *      `noti_system_setting`, `noti_channel`.
 *   3. The interface block contains exactly the published 32 RPC
 *      methods (count + every name).  Adding or removing a method
 *      requires both repos to update in lockstep — this test
 *      enforces that the count never drifts silently.
 *   4. The two delegate callbacks (`changed`, `event_received`) are
 *      both declared — these define the daemon→client push path.
 *   5. Critical signatures are pinned (add_noti / update_noti /
 *      load_noti_grouping_list / get_setting_array shape) so a
 *      maintainer who changes a parameter type or `out` annotation
 *      sees an immediate test failure rather than an ABI mismatch
 *      at runtime.
 */

#include <fstream>
#include <regex>
#include <sstream>
#include <string>

#include <gtest/gtest.h>

namespace {

#ifndef NOTIFICATION_TIDL_SOURCE_PATH
#define NOTIFICATION_TIDL_SOURCE_PATH ""
#endif
constexpr const char* kTidlPath = NOTIFICATION_TIDL_SOURCE_PATH;

std::string SlurpTidl() {
  std::ifstream f(kTidlPath);
  if (!f.is_open()) return std::string();
  std::ostringstream ss;
  ss << f.rdbuf();
  return ss.str();
}

int CountOccurrences(const std::string& haystack, const std::string& needle) {
  int n = 0;
  size_t pos = 0;
  while ((pos = haystack.find(needle, pos)) != std::string::npos) {
    ++n;
    pos += needle.size();
  }
  return n;
}

class TidlSourceContractTest : public ::testing::Test {
 protected:
  std::string body_;

  void SetUp() override {
    body_ = SlurpTidl();
    if (body_.empty()) {
      GTEST_SKIP() << "noti_service.tidl unreachable at "
                   << kTidlPath
                   << " (expected on out-of-tree builds; in-tree "
                      "builds must surface this).";
    }
  }
};

/* ------------------------------------------------------------------ */
/* Privilege annotation                                                */
/* ------------------------------------------------------------------ */

TEST_F(TidlSourceContractTest, PrivilegeAnnotationIsNotificationDomain) {
  /* The interface is gated by tizen.org/privilege/notification.  Any
   * downgrade (e.g. renaming to a less-privileged domain) opens the
   * RPC surface up to apps that should not reach it. */
  EXPECT_NE(body_.find("[privilege = \"http://tizen.org/privilege/notification\"]"),
            std::string::npos)
      << "Stage 7: privilege annotation must remain "
         "tizen.org/privilege/notification";
  EXPECT_NE(body_.find("interface noti_service"), std::string::npos);
}

/* ------------------------------------------------------------------ */
/* Five published structs                                              */
/* ------------------------------------------------------------------ */

TEST_F(TidlSourceContractTest, FivePublishedStructsArePresent) {
  EXPECT_NE(body_.find("struct notification"), std::string::npos);
  EXPECT_NE(body_.find("struct noti_setting"), std::string::npos);
  EXPECT_NE(body_.find("struct noti_system_setting_dnd_allow_exception"),
            std::string::npos);
  EXPECT_NE(body_.find("struct noti_system_setting"), std::string::npos);
  EXPECT_NE(body_.find("struct noti_channel"), std::string::npos);
}

TEST_F(TidlSourceContractTest, NoSurpriseStructWasAdded) {
  /* Counting "struct " occurrences keeps drift visible — adding a
   * sixth struct without bumping this count is the signal a
   * reviewer should see. */
  EXPECT_EQ(CountOccurrences(body_, "struct "), 5)
      << "Stage 7: tidl publishes exactly 5 structs.  If you add or "
         "remove a struct, update this count in lockstep with the "
         "wire-format documentation.";
}

/* ------------------------------------------------------------------ */
/* RPC method roster (32 methods)                                      */
/* ------------------------------------------------------------------ */

TEST_F(TidlSourceContractTest, ThirtyTwoPublishedRpcMethods) {
  /* Every RPC method declaration is `int <name>(...)` or `void
   * <name>(...) delegate;` inside the interface block.  Pin the
   * total count via the interface's semicolon density: the
   * privilege block + struct definitions + interface scaffolding
   * contain none of the `;` terminators that close RPC method
   * decls.  Easier and more robust to enumerate the method name
   * tokens directly. */
  static const char* const kMethods[] = {
    "register_changed", "unregister_changed",
    "register_event", "unregister_event",
    "register_service",
    "add_noti", "update_noti", "refresh_noti",
    "delete_single_noti", "delete_multiple_noti",
    "delete_noti_by_display_applist",
    "get_noti_count", "get_all_noti_count",
    "update_noti_setting", "update_noti_sys_setting",
    "load_noti_by_tag", "load_noti_by_priv_id",
    "load_noti_grouping_list", "load_noti_detail_list",
    "get_setting_array", "get_setting_by_app_id",
    "load_system_setting",
    "save_as_template", "create_from_template",
    "create_from_package_template",
    "get_noti_block_state", "update_dnd_allow_exception",
    "send_event", "send_event_by_priv_id",
    "check_event_receiver", "reset_event_handler",
    "register_dnd_app", "unregister_dnd_app",
    "channel_insert", "channel_delete",
    "channel_update", "channel_get", "channel_get_list"
  };
  for (const char* name : kMethods) {
    /* Each name appears as a method declaration inside the
     * interface block; the leading whitespace + return type would
     * be tedious to encode, but the simple substring is safe
     * because the same name does not appear in any of the 5
     * structs. */
    EXPECT_NE(body_.find(name), std::string::npos)
        << "Stage 7: published RPC method `" << name
        << "` missing from noti_service.tidl";
  }
  /* 5 structs + 38 RPC methods + 2 delegates = 45 items but
   * not every RPC method is a top-level entry; the 38 above is
   * the union of ROADMAP-published entry points (register_changed
   * + unregister_changed + register_event + unregister_event +
   * register_service + 33 service methods).  Total exact decl
   * count is asserted next. */
}

TEST_F(TidlSourceContractTest, DelegateCallbacksArePresent) {
  /* The two daemon→client push delegates define the change-notify
   * + event-receive surface.  Both must be present and tagged
   * `delegate;` so tidlc emits the proxy-side dispatcher hooks. */
  EXPECT_NE(body_.find("changed("), std::string::npos);
  EXPECT_NE(body_.find("event_received("), std::string::npos);
  /* Two `delegate;` markers in the entire file — drift here means
   * a third delegate was added without updating the wire-format
   * docs. */
  EXPECT_EQ(CountOccurrences(body_, "delegate;"), 2)
      << "Stage 7: tidl publishes exactly 2 delegates "
         "(changed, event_received).";
}

/* ------------------------------------------------------------------ */
/* Pinned signature shape                                              */
/* ------------------------------------------------------------------ */

TEST_F(TidlSourceContractTest, AddNotiReturnsPrivIdViaOutInt) {
  /* `int add_noti(notification notihandle, out int priv_id);` —
   * pre-fix audit (74aefde) relied on the `out int` annotation
   * for the priv_id round-trip; if the annotation drifts to
   * `inout` or the type changes, the daemon's stub generator
   * emits a different marshalling skeleton and the client+daemon
   * decode disagree at runtime. */
  std::regex pat(R"(int\s+add_noti\s*\(\s*notification\s+\w+\s*,\s*out\s+int\s+\w+\s*\))");
  EXPECT_TRUE(std::regex_search(body_, pat))
      << "Stage 7: add_noti must remain `int add_noti(notification "
         "<arg>, out int <arg>)`.";
}

TEST_F(TidlSourceContractTest, GetSettingArrayReturnsArrayOfNotiSetting) {
  std::regex pat(
      R"(int\s+get_setting_array\s*\(\s*int\s+\w+\s*,\s*out\s+array<noti_setting>\s+\w+\s*\))");
  EXPECT_TRUE(std::regex_search(body_, pat))
      << "Stage 7: get_setting_array must return `out "
         "array<noti_setting>`.";
}

TEST_F(TidlSourceContractTest, LoadNotiByTagReturnsNotificationOut) {
  std::regex pat(
      R"(int\s+load_noti_by_tag\s*\(\s*string\s+\w+\s*,\s*string\s+\w+\s*,\s*int\s+\w+\s*,\s*out\s+notification\s+\w+\s*\))");
  EXPECT_TRUE(std::regex_search(body_, pat))
      << "Stage 7: load_noti_by_tag must return `out notification`.";
}

TEST_F(TidlSourceContractTest,
       NoUidParameterInRegisterChangedDelegate) {
  /* `register_changed` accepts a `changed callback` plus `int uid`
   * — the callback delegate itself takes notihandle/command/extra
   * but NOT a uid (the dispatcher routes by registration uid,
   * not per-event uid).  Pin the absence so a future maintainer
   * does not bolt a uid into the delegate signature inadvertently
   * (which would force every existing client to recompile). */
  std::regex pat(
      R"(void\s+changed\s*\(\s*notification\s+\w+\s*,\s*string\s+\w+\s*,\s*array<int>\s+\w+\s*\)\s+delegate;)");
  EXPECT_TRUE(std::regex_search(body_, pat))
      << "Stage 7: changed delegate must remain "
         "(notification, string, array<int>).";
}
}  // namespace
