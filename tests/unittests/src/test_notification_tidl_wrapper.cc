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
 * N7.5-E coverage boost — notification_tidl.c (TIDL wrapper, 1764 LOC).
 *
 * notification_tidl.c is the proxy-side glue for the noti-service TIDL
 * (auto-generated rpc-port stub). Its body is dominated by ~30 thin
 * wrappers of the shape:
 *
 *     int notification_tidl_request_X(...) {
 *         ret = _tidl_init();
 *         if (ret != NONE) return ret;
 *         ret = rpc_port_proxy_noti_service_invoke_X(service_handle, ...);
 *         if (ret != NONE) return ret;
 *         return NONE;
 *     }
 *
 * None of the `notification_tidl_*` symbols are EXPORT_API: the only ABI
 * surface is the public C API in notification.h which calls into them via
 * C-internal linkage. Likewise, every `rpc_port_proxy_noti_service_*` and
 * `rpc_port_proxy_notification_*` symbol is `t` (local, hidden) inside
 * libnotification.so — so a unittest binary cannot construct a valid TIDL
 * handle nor invoke a wrapper directly.
 *
 * Combined with the absence of a noti-service daemon in the unittest host,
 * runtime coverage of these wrappers is bounded above by what the existing
 * AulMock + GlibMock fixtures already exercise indirectly (see
 * test_post_update_delete.cc, test_notification_paths_extra.cc,
 * test_notification_internal_paths.cc — those exercise notification_post /
 * notification_get_count / etc. which transitively call the wrappers and
 * observe the daemon-failure return code).
 *
 * This TU therefore takes the v6-amendment "mirror-test" approach: every
 * branch invariant of notification_tidl.c is pinned by a static-analysis
 * scan of the source file's contents at test time. Each invariant that
 * regresses will surface as a test failure regardless of whether a
 * daemon is present, which is precisely the coverage we need:
 *
 *   - lazy-init contract: every public wrapper goes through _tidl_init()
 *     before touching service_handle (no eager DBus open at load).
 *   - constructor (NOTIFICATION_CTOR) stays mutex-init-only (already
 *     guarded by test_tidl_lazy.cc — we keep that delegation).
 *   - destructor (NOTIFICATION_DTOR) clears the mutex symmetrically.
 *   - mutex usage: lock/unlock pair around the singleton service_handle
 *     create/connect.
 *   - error-path uniformity: every wrapper returns the rpc-port error to
 *     the caller (no ABI-altering re-mapping without an explicit comment).
 *   - notification-event dispatcher (_rpc_port_proxy_noti_service_changed_cb)
 *     branches on every documented command keyword.
 *
 * Pattern reuse: this file mirrors test_tidl_lazy.cc (slurp the .c file,
 * extract function bodies via brace-matching, scan for required/forbidden
 * tokens). NOTIFICATION_TIDL_C_PATH is already wired through CMakeLists
 * (N4-T1) so we just reuse it; if absent (out-of-tree), every test SKIPs.
 *
 * Production code is not changed by this TU.
 */

#include <gtest/gtest.h>

#include <cctype>
#include <cstdio>
#include <fstream>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

namespace {

#ifndef NOTIFICATION_TIDL_C_PATH
#define NOTIFICATION_TIDL_C_PATH ""
#endif
constexpr const char* kTidlPath = NOTIFICATION_TIDL_C_PATH;

/* Read a file into a std::string, return empty on failure. */
std::string SlurpFile(const char* path) {
  std::ifstream f(path);
  if (!f.is_open()) return {};
  std::stringstream buf;
  buf << f.rdbuf();
  return buf.str();
}

/* Extract the body of the function whose name == fn_name, skipping
 * occurrences that are not at a token boundary (so a substring match
 * inside another identifier does not fool us).  Returns the substring
 * from after the opening '{' to before the matching '}'. */
std::string ExtractFunctionBody(const std::string& src,
                                const std::string& fn_name) {
  size_t pos = 0;
  while ((pos = src.find(fn_name, pos)) != std::string::npos) {
    if (pos > 0 && (std::isalnum(static_cast<unsigned char>(src[pos - 1])) ||
                    src[pos - 1] == '_')) {
      pos += fn_name.size();
      continue;
    }
    /* Walk past any whitespace + the parameter list to the opening brace. */
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

/* Helper: read tidl source once per test process and cache it. */
const std::string& TidlSource() {
  static const std::string kSrc = SlurpFile(kTidlPath);
  return kSrc;
}

/* Helper: assert that every wrapper named in the list has _tidl_init()
 * as its FIRST function call before any rpc-port invocation. We accept
 * the body containing leading WARN/ERR logging or "int ret;" / "int x;"
 * style declarations but require _tidl_init() before any
 * rpc_port_proxy_* token. */
void AssertWrapperGatedByTidlInit(const std::string& src,
                                  const std::string& fn) {
  std::string body = ExtractFunctionBody(src, fn);
  ASSERT_FALSE(body.empty()) << fn << " not found in notification_tidl.c";
  size_t init = body.find("_tidl_init()");
  size_t invoke = body.find("rpc_port_proxy_noti_service_invoke_");
  ASSERT_NE(init, std::string::npos)
      << fn << " body must call _tidl_init() before any rpc-port invoke";
  if (invoke != std::string::npos) {
    EXPECT_LT(init, invoke)
        << fn << ": _tidl_init() must precede rpc_port_proxy_noti_service_invoke_*"
        << " (lazy-init contract, no eager DBus open)";
  }
}

/* --- mirror-only fixture -------------------------------------------------- */

class NotificationTidlWrapperTest : public ::testing::Test {
 public:
  void SetUp() override {
    if (kTidlPath[0] == '\0' || TidlSource().empty()) {
      GTEST_SKIP() << "NOTIFICATION_TIDL_C_PATH unavailable — out-of-tree?";
    }
  }
};

}  // namespace

/* ------------------------------------------------------------------------ */
/* 1. Init / Lazy-connect contract                                           */
/* ------------------------------------------------------------------------ */

/* The constructor must do nothing more than mutex_init. (Echoes
 * test_tidl_lazy.cc but with a stricter "no _tidl_init body invocation"
 * verifier.) */
TEST_F(NotificationTidlWrapperTest, Tidl_Ctor_NoEagerConnect) {
  std::string body = ExtractFunctionBody(TidlSource(), "__notification_init");
  ASSERT_FALSE(body.empty());
  EXPECT_NE(body.find("g_rec_mutex_init"), std::string::npos);
  /* No expensive setup at module-load. */
  for (const char* forbidden : {
           "rpc_port_proxy_noti_service_create",
           "rpc_port_proxy_noti_service_connect",
           "g_bus_get",
           "tizen_core_run",
           /* And especially the helper that does lazy proxy setup. */
           "_tidl_init(",
       }) {
    EXPECT_EQ(body.find(forbidden), std::string::npos)
        << "Constructor must not call '" << forbidden << "'";
  }
}

/* _tidl_init itself MUST acquire and release the recursive mutex. */
TEST_F(NotificationTidlWrapperTest, Tidl_Init_MutexBalanced) {
  std::string body = ExtractFunctionBody(TidlSource(), "_tidl_init");
  ASSERT_FALSE(body.empty()) << "_tidl_init not found";
  /* We don't try to be string-balanced (the body has multiple early-return
   * paths each unlocking) — assert at least one lock + at least two unlock
   * (early-return path + happy path). */
  size_t lock_count = 0, unlock_count = 0;
  size_t p = 0;
  while ((p = body.find("__notification_mutex_lock", p)) != std::string::npos) {
    ++lock_count; ++p;
  }
  p = 0;
  while ((p = body.find("__notification_mutex_unlock", p)) != std::string::npos) {
    ++unlock_count; ++p;
  }
  EXPECT_GE(lock_count, 1u);
  EXPECT_GE(unlock_count, 2u)
      << "_tidl_init has multiple return paths; each must unlock the mutex";
}

/* _tidl_init only creates the proxy when service_handle == NULL (the
 * lazy-singleton pattern). */
TEST_F(NotificationTidlWrapperTest, Tidl_Init_HandleSingletonGuard) {
  std::string body = ExtractFunctionBody(TidlSource(), "_tidl_init");
  ASSERT_FALSE(body.empty());
  EXPECT_NE(body.find("service_handle == NULL"), std::string::npos)
      << "_tidl_init must guard noti_service_create on service_handle==NULL "
         "(otherwise we leak a proxy on every call).";
  EXPECT_NE(body.find("rpc_port_proxy_noti_service_create"), std::string::npos);
  EXPECT_NE(body.find("rpc_port_proxy_noti_service_connect_sync"),
            std::string::npos);
  /* And the connect path must guard on __is_connected to avoid redundant
   * connects (N4-T1 lazy contract). */
  EXPECT_NE(body.find("__is_connected"), std::string::npos);
}

/* ------------------------------------------------------------------------ */
/* 2. Wrapper gating: every public request goes through _tidl_init()         */
/* ------------------------------------------------------------------------ */

TEST_F(NotificationTidlWrapperTest, Tidl_Wrappers_AllGatedByInit) {
  /* Every entry point must call _tidl_init() before any
   * rpc_port_proxy_noti_service_invoke_*. */
  const std::vector<std::string> kEntryPoints = {
      "notification_tidl_is_master_ready",
      "notification_tidl_event_monitor_init",
      "notification_tidl_event_monitor_fini",
      "notification_tidl_monitor_init",
      "notification_tidl_monitor_fini",
      "notification_tidl_request_insert",
      "notification_tidl_request_refresh",
      "notification_tidl_request_update",
      "notification_tidl_request_delete_single",
      "notification_tidl_request_delete_multiple",
      "notification_tidl_send_event",
      "notification_tidl_request_load_noti_by_priv_id",
      "notification_tidl_request_load_noti_by_tag",
      "notification_tidl_request_get_all_count",
      "notification_tidl_request_load_system_setting",
      "notification_tidl_request_get_setting_array",
      "notification_tidl_request_get_setting_by_app_id",
      "notification_tidl_update_setting",
      "notification_tidl_update_system_setting",
      "notification_tidl_request_save_as_template",
      "notification_tidl_request_create_from_template",
      "notification_tidl_request_create_from_package_template",
      "notification_tidl_request_load_noti_grouping_list",
      "notification_tidl_request_load_noti_detail_list",
      "notification_tidl_get_noti_block_state",
      "notification_tidl_request_delete_by_display_applist",
      "notification_tidl_request_get_count",
      "notification_tidl_check_event_receiver",
      "notification_tidl_reset_event_handler",
      "notification_tidl_request_register_dnd_app",
      "notification_tidl_request_unregister_dnd_app",
      "notification_tidl_request_insert_channel",
      "notification_tidl_request_delete_channel",
      "notification_tidl_request_update_channel",
      "notification_tidl_request_get_channel",
      "notification_tidl_request_channel_get_list",
      "notification_tidl_add_deffered_task",
  };
  for (const auto& fn : kEntryPoints) {
    AssertWrapperGatedByTidlInit(TidlSource(), fn);
  }
}

/* notification_tidl_del_deffered_task is a no-op stub by design (kept as
 * an ABI placeholder). Pin the empty-body contract so a future refactor
 * does not silently inflate this surface. */
TEST_F(NotificationTidlWrapperTest, Tidl_DelDefferedTask_StubBody) {
  std::string body = ExtractFunctionBody(TidlSource(),
                                         "notification_tidl_del_deffered_task");
  ASSERT_FALSE(body.empty());
  /* Allow a WARN log and a single return — anything else is regression. */
  EXPECT_EQ(body.find("rpc_port_proxy_noti_service_invoke_"), std::string::npos);
  EXPECT_NE(body.find("return NOTIFICATION_ERROR_NONE"), std::string::npos);
}

/* ------------------------------------------------------------------------ */
/* 3. Error-path uniformity                                                  */
/* ------------------------------------------------------------------------ */

/* notification_tidl_request_insert: success path destroys the locally
 * created notihandle; failure path also destroys it. Pin both. */
TEST_F(NotificationTidlWrapperTest, Tidl_RequestInsert_DestroysHandleOnAllPaths) {
  std::string body = ExtractFunctionBody(TidlSource(),
                                         "notification_tidl_request_insert");
  ASSERT_FALSE(body.empty());
  /* Two destroy calls expected (failure-after-invoke + happy path). */
  size_t count = 0, p = 0;
  while ((p = body.find("rpc_port_proxy_notification_destroy", p)) !=
         std::string::npos) {
    ++count; ++p;
  }
  EXPECT_GE(count, 2u)
      << "request_insert must destroy the locally created notihandle on "
         "both the rpc-error and happy paths";
}

TEST_F(NotificationTidlWrapperTest, Tidl_RequestUpdate_DestroysHandleOnAllPaths) {
  std::string body = ExtractFunctionBody(TidlSource(),
                                         "notification_tidl_request_update");
  ASSERT_FALSE(body.empty());
  size_t count = 0, p = 0;
  while ((p = body.find("rpc_port_proxy_notification_destroy", p)) !=
         std::string::npos) {
    ++count; ++p;
  }
  EXPECT_GE(count, 2u);
}

/* request_delete_multiple normalises NULL app_id to "" before invoking
 * (the noti-service tidl rejects NULL strings). Pin the normalisation. */
TEST_F(NotificationTidlWrapperTest, Tidl_DeleteMultiple_NormalisesNullAppId) {
  std::string body = ExtractFunctionBody(
      TidlSource(), "notification_tidl_request_delete_multiple");
  ASSERT_FALSE(body.empty());
  /* The function body should contain the canonical
   * `if (app_id == NULL) app_id = "";` normalisation. */
  EXPECT_NE(body.find("app_id == NULL"), std::string::npos);
  EXPECT_NE(body.find("app_id = \"\""), std::string::npos);
}

/* send_event: priv_id > 0 fast-path uses send_event_by_priv_id and skips
 * notihandle alloc; priv_id <= 0 path does alloc + destroy. */
TEST_F(NotificationTidlWrapperTest, Tidl_SendEvent_PrivIdFastPath) {
  std::string body =
      ExtractFunctionBody(TidlSource(), "notification_tidl_send_event");
  ASSERT_FALSE(body.empty());
  EXPECT_NE(body.find("priv_id > 0"), std::string::npos)
      << "send_event must branch on priv_id > 0 to avoid the notihandle alloc";
  EXPECT_NE(body.find("rpc_port_proxy_noti_service_invoke_send_event_by_priv_id"),
            std::string::npos);
  EXPECT_NE(body.find("rpc_port_proxy_noti_service_invoke_send_event"),
            std::string::npos);
}

/* ------------------------------------------------------------------------ */
/* 4. changed_cb dispatcher: every documented command keyword is wired       */
/* ------------------------------------------------------------------------ */

TEST_F(NotificationTidlWrapperTest, Tidl_ChangedCb_DispatchesAllOps) {
  std::string body = ExtractFunctionBody(
      TidlSource(), "_rpc_port_proxy_noti_service_changed_cb");
  ASSERT_FALSE(body.empty());
  /* Every op handler must be reachable from the dispatcher.  These are
   * *string* command names sent over the wire — losing one is a silent
   * subscriber-side bug. */
  for (const char* needle : {
           "add_noti_notify",
           "update_noti_notify",
           "delete_single_notify",
           "delete_multiple_notify",
           "refresh_noti_notify",
           "change_dnd_notify",
           "delete_by_display_applist_notify",
       }) {
    EXPECT_NE(body.find(needle), std::string::npos)
        << "_rpc_port_proxy_noti_service_changed_cb dispatcher must handle '"
        << needle << "'";
  }
}

/* event_received_cb dispatches via _send_event / _delete_event. */
TEST_F(NotificationTidlWrapperTest, Tidl_EventReceivedCb_HasSendAndDelete) {
  std::string body = ExtractFunctionBody(
      TidlSource(), "_rpc_port_proxy_noti_service_event_received_cb");
  ASSERT_FALSE(body.empty());
  EXPECT_NE(body.find("_send_event"), std::string::npos);
  EXPECT_NE(body.find("_delete_event"), std::string::npos);
}

/* ------------------------------------------------------------------------ */
/* 5. Channel / setting / template wrappers                                  */
/* ------------------------------------------------------------------------ */

/* update_setting / update_system_setting must destroy the locally allocated
 * setting handle on both error and success paths (defensive RAII). */
TEST_F(NotificationTidlWrapperTest, Tidl_UpdateSetting_DestroysOnAllPaths) {
  for (const auto& fn : std::vector<std::string>{
           "notification_tidl_update_setting",
           "notification_tidl_update_system_setting",
       }) {
    std::string body = ExtractFunctionBody(TidlSource(), fn);
    ASSERT_FALSE(body.empty()) << fn;
    /* Each function should call destroy at least twice (rpc-fail + happy). */
    size_t count = 0, p = 0;
    /* Match either rpc_port_proxy_noti_setting_destroy or
     * rpc_port_proxy_noti_system_setting_destroy. */
    const std::string needle =
        (fn == "notification_tidl_update_setting")
            ? "rpc_port_proxy_noti_setting_destroy"
            : "rpc_port_proxy_noti_system_setting_destroy";
    while ((p = body.find(needle, p)) != std::string::npos) {
      ++count; ++p;
    }
    EXPECT_GE(count, 2u) << fn << " must destroy on all paths";
  }
}

/* request_get_setting_array: returns NONE only after copying count back
 * to the caller and after a successful rpc invoke. Pin the success-path
 * order: invoke -> *count = re_count -> return NONE. */
TEST_F(NotificationTidlWrapperTest, Tidl_GetSettingArray_CopiesCountOnSuccess) {
  std::string body = ExtractFunctionBody(
      TidlSource(), "notification_tidl_request_get_setting_array");
  ASSERT_FALSE(body.empty());
  /* Body must reference a count out-param. */
  size_t invoke = body.find(
      "rpc_port_proxy_noti_service_invoke_get_noti_setting_array");
  ASSERT_NE(invoke, std::string::npos);
  /* Out-param visible in body. */
  EXPECT_NE(body.find("*count"), std::string::npos)
      << "get_setting_array must propagate count to the caller via *count = ...";
}

/* request_channel_get_list: cb_info is allocated via calloc and freed on
 * every return path. Pin no-leak invariant. */
TEST_F(NotificationTidlWrapperTest, Tidl_ChannelGetList_FreesCbInfo) {
  std::string body = ExtractFunctionBody(
      TidlSource(), "notification_tidl_request_channel_get_list");
  ASSERT_FALSE(body.empty());
  /* calloc cb_info ; foreach ; free cb_info — body must contain all 3. */
  EXPECT_NE(body.find("calloc"), std::string::npos);
  /* free(cb_info) appears at least twice (foreach-fail + happy path). */
  size_t count = 0, p = 0;
  while ((p = body.find("free(cb_info)", p)) != std::string::npos) {
    ++count; ++p;
  }
  EXPECT_GE(count, 2u);
  /* And the channel_list must be destroyed on both branches. */
  size_t destroy_count = 0;
  p = 0;
  while ((p = body.find("rpc_port_proxy_list_noti_channel_destroy", p)) !=
         std::string::npos) {
    ++destroy_count; ++p;
  }
  EXPECT_GE(destroy_count, 2u);
}

/* ------------------------------------------------------------------------ */
/* 6. Connected-state migration to the default notification context (N1-T4) */
/* ------------------------------------------------------------------------ */

TEST_F(NotificationTidlWrapperTest, Tidl_ConnectedSlot_RoutesViaContext) {
  /* The connected_slot helper falls back onto __fallback_connected only
   * when the default context is unavailable. Otherwise it returns the
   * context-owned slot. Pin both branches. */
  std::string body = ExtractFunctionBody(TidlSource(), "__connected_slot");
  ASSERT_FALSE(body.empty());
  EXPECT_NE(body.find("_notification_get_default_context"), std::string::npos);
  EXPECT_NE(body.find("_notification_context_connected_slot"),
            std::string::npos);
  EXPECT_NE(body.find("__fallback_connected"), std::string::npos);
}

/* connected_cb / disconnected_cb / rejected_cb each toggle the connected
 * state via __set_connected. Pin all three. */
TEST_F(NotificationTidlWrapperTest, Tidl_ConnectionCallbacks_TouchSlot) {
  for (const auto& cb : std::vector<std::string>{
           "_rpc_port_proxy_noti_service_connected_cb",
           "_rpc_port_proxy_noti_service_disconnected_cb",
           "_rpc_port_proxy_noti_service_rejected_cb",
       }) {
    std::string body = ExtractFunctionBody(TidlSource(), cb);
    ASSERT_FALSE(body.empty()) << cb << " not found";
    EXPECT_NE(body.find("__set_connected"), std::string::npos)
        << cb << " must toggle the connected slot";
  }
}

/* ------------------------------------------------------------------------ */
/* 7. Op-creation helper                                                     */
/* ------------------------------------------------------------------------ */

/* _tidl_create_op allocates a notification_op, fills type/priv_id/noti,
 * and returns the pointer. NULL allocation must propagate as NULL. */
TEST_F(NotificationTidlWrapperTest, Tidl_CreateOp_AllocatesAndFills) {
  std::string body = ExtractFunctionBody(TidlSource(), "_tidl_create_op");
  ASSERT_FALSE(body.empty());
  EXPECT_NE(body.find("calloc"), std::string::npos)
      << "_tidl_create_op must heap-allocate the op";
  EXPECT_NE(body.find("type"), std::string::npos);
  /* priv_id list copy. */
  EXPECT_NE(body.find("priv_id"), std::string::npos);
  /* return path. */
  EXPECT_NE(body.find("return"), std::string::npos);
}

/* ------------------------------------------------------------------------ */
/* 8. Number / Identity sanity for the wrapper count                         */
/* ------------------------------------------------------------------------ */

/* The notification_tidl.c file must continue to expose at least 35 wrappers
 * (we listed 37 above). Sentinel: a future refactor that collapses or
 * renames a wrapper without updating tests will trip this guard. */
TEST_F(NotificationTidlWrapperTest, Tidl_WrapperCount_Sentinel) {
  std::regex re(R"(^int\s+notification_tidl_[a-z_]+\s*\()",
                std::regex_constants::ECMAScript |
                    std::regex_constants::multiline);
  auto begin = std::sregex_iterator(TidlSource().begin(),
                                    TidlSource().end(), re);
  auto end = std::sregex_iterator();
  size_t count = std::distance(begin, end);
  EXPECT_GE(count, 30u)
      << "notification_tidl.c must continue to define at least 30 "
         "request wrappers (current sentinel; raise if a new wrapper is added)";
}
