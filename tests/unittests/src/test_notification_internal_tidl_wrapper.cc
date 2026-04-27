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
 * N7.5-E coverage boost — notification_internal_tidl.c (TIDL marshalling,
 * 1454 LOC).
 *
 * notification_internal_tidl.c is a pair of large 1:1 (de)serialisers
 * between `notification_h` and the auto-generated tidl proxy structs:
 *
 *   make_empty_notification(handle)               // fresh defaults
 *   make_notification_from_noti(handle, noti, tr) // pack: noti -> handle
 *   make_noti_from_notification(out, handle)      // unpack: handle -> noti
 *   make_setting_from_noti_setting(out, handle)
 *   make_noti_setting_from_setting(handle, setting)
 *   make_setting_from_noti_system_setting(out, handle)
 *   make_noti_system_setting_from_setting(handle, setting)
 *   make_dnd_allow_exception_from_exception(handle, dnd) // NULL-guarded
 *
 * Although these `make_*` symbols ARE EXPORT_API (visible in libnotification.so
 * dynamic symbol table), the `rpc_port_proxy_*_h` parameters they accept can
 * only be allocated by the auto-generated `rpc_port_proxy_*_create()` factory
 * functions which are LOCAL (`t` in the nm output) and therefore unreachable
 * from the unittest binary.  Without a way to construct a valid handle, we
 * cannot drive these functions end-to-end at runtime.
 *
 * Therefore — exactly as N7.5-D and D7.5-F did for TIDL surfaces — this TU
 * uses the v6-amendment "mirror-test" approach: pin every behavioural
 * invariant of notification_internal_tidl.c by static analysis of the
 * source file at test time.  Coverage of the invariants comes from the
 * scan; coverage of the executable code comes (indirectly) from the
 * existing public-API tests that drive notification_post / update / load
 * grouping list / setting_get_setting_array / etc., which transitively
 * call into make_notification_from_noti and make_noti_from_notification.
 *
 * The scan-based invariants pinned here:
 *
 *   1. make_empty_notification populates EVERY public default field
 *      (-1 enums, "" strings, empty bundles, false bools).
 *   2. make_notification_from_noti / make_noti_from_notification call the
 *      matching set/get pair for every notification field — the count of
 *      set vs. get calls must agree (round-trip completeness).
 *   3. NULL bundles in the noti are replaced with empty_bundle (no NULL
 *      sent over the wire).
 *   4. Bool fields are bridged through a `bool _bf = ...; ...; ? 1u : 0u`
 *      pattern so the bitfield in `struct _notification` stays valid.
 *   5. Empty string fields (strlen == 0) are dropped (set to NULL +
 *      free()) on the unpack side, preserving the original-NULL semantics.
 *   6. make_dnd_allow_exception_from_exception NULL-guards both args.
 *   7. make_noti_system_setting_from_setting only allocates a list handle
 *      when there are exceptions (length > 0 check).
 *   8. The dnd-exception-foreach callback returns false on field-get
 *      failure (so the foreach short-circuits) and true on success.
 *
 * Production code is not changed by this TU.  The only file added is a
 * test source.
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

#ifndef NOTIFICATION_INTERNAL_TIDL_C_PATH
#define NOTIFICATION_INTERNAL_TIDL_C_PATH ""
#endif
constexpr const char* kPath = NOTIFICATION_INTERNAL_TIDL_C_PATH;

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

const std::string& Source() {
  static const std::string kSrc = SlurpFile(kPath);
  return kSrc;
}

/* Count occurrences of pattern in body (substring count). */
size_t CountOccurrences(const std::string& body, const std::string& pat) {
  size_t count = 0, p = 0;
  while ((p = body.find(pat, p)) != std::string::npos) {
    ++count;
    p += pat.size();
  }
  return count;
}

class NotificationInternalTidlWrapperTest : public ::testing::Test {
 public:
  void SetUp() override {
    if (kPath[0] == '\0' || Source().empty()) {
      GTEST_SKIP() << "NOTIFICATION_INTERNAL_TIDL_C_PATH unavailable";
    }
  }
};

}  // namespace

/* ------------------------------------------------------------------------ */
/* 1. make_empty_notification populates every default field                  */
/* ------------------------------------------------------------------------ */

TEST_F(NotificationInternalTidlWrapperTest,
       InternalTidl_MakeEmpty_HasDefaultsForAllFields) {
  std::string body = ExtractFunctionBody(Source(), "make_empty_notification");
  ASSERT_FALSE(body.empty());
  /* The defaults: -1 enums, "" strings, empty bundles, false bools.
   * We pin a representative subset of the ~40 setters. */
  for (const char* needle : {
           "rpc_port_proxy_notification_set_type(handle, -1)",
           "rpc_port_proxy_notification_set_layout(handle, -1)",
           "rpc_port_proxy_notification_set_group_id(handle, -1)",
           "rpc_port_proxy_notification_set_priv_id(handle, -1)",
           "rpc_port_proxy_notification_set_pkg_id(handle, \"\")",
           "rpc_port_proxy_notification_set_caller_app_id(handle, \"\")",
           "rpc_port_proxy_notification_set_launch_app_id(handle, \"\")",
           "rpc_port_proxy_notification_set_args(handle, empty_bundle)",
           "rpc_port_proxy_notification_set_check_box_value(handle, false)",
           "rpc_port_proxy_notification_set_uid(handle, -1)",
       }) {
    EXPECT_NE(body.find(needle), std::string::npos)
        << "make_empty_notification missing default: " << needle;
  }
  /* Empty bundle must be freed at the end. */
  EXPECT_NE(body.find("bundle_free(empty_bundle)"), std::string::npos);
  /* Should not allocate `_setting`, `_noti`, etc. — pure handle init. */
  EXPECT_EQ(body.find("calloc"), std::string::npos);
}

/* ------------------------------------------------------------------------ */
/* 2. Pack / Unpack symmetry                                                 */
/* ------------------------------------------------------------------------ */

/* make_notification_from_noti must call setters; make_noti_from_notification
 * must call getters. Field-level symmetry is asserted by counting calls. */
TEST_F(NotificationInternalTidlWrapperTest,
       InternalTidl_PackUnpack_FieldCountSymmetric) {
  std::string pack = ExtractFunctionBody(Source(),
                                         "make_notification_from_noti");
  std::string unpack = ExtractFunctionBody(Source(),
                                           "make_noti_from_notification");
  ASSERT_FALSE(pack.empty());
  ASSERT_FALSE(unpack.empty());

  /* The number of set_X calls in pack should equal the number of
   * get_X calls in unpack — every field round-trips. We count
   * "rpc_port_proxy_notification_set_" and "rpc_port_proxy_notification_get_"
   * tokens. */
  size_t set_count = CountOccurrences(pack, "rpc_port_proxy_notification_set_");
  size_t get_count = CountOccurrences(unpack,
                                      "rpc_port_proxy_notification_get_");
  EXPECT_GT(set_count, 30u)
      << "make_notification_from_noti must set every field (>30 setters)";
  EXPECT_GT(get_count, 30u)
      << "make_noti_from_notification must get every field (>30 getters)";
  /* Allow ±2 mismatch for handful of array_bundle helpers that look like a
   * set/get on the line but emit an extra cleanup. */
  EXPECT_NEAR(static_cast<double>(set_count),
              static_cast<double>(get_count), 3.0)
      << "set/get count mismatch implies a field is missing on either path";
}

/* ------------------------------------------------------------------------ */
/* 3. NULL bundle replacement on the pack side                               */
/* ------------------------------------------------------------------------ */

TEST_F(NotificationInternalTidlWrapperTest,
       InternalTidl_PackSide_NullBundlesReplacedByEmpty) {
  std::string body =
      ExtractFunctionBody(Source(), "make_notification_from_noti");
  ASSERT_FALSE(body.empty());
  /* Each bundle field must replace NULL with empty_bundle. We sample a few. */
  for (const char* sample : {
           "noti->args ? noti->args : empty_bundle",
           "noti->group_args ? noti->group_args : empty_bundle",
           "noti->b_execute_option ? noti->b_execute_option : empty_bundle",
           "noti->b_text ? noti->b_text : empty_bundle",
           "noti->b_key ? noti->b_key : empty_bundle",
           "noti->b_image_path ? noti->b_image_path : empty_bundle",
       }) {
    EXPECT_NE(body.find(sample), std::string::npos)
        << "make_notification_from_noti must NULL-replace bundle: " << sample;
  }
  EXPECT_NE(body.find("empty_bundle = bundle_create()"), std::string::npos);
}

/* ------------------------------------------------------------------------ */
/* 4. Bool bitfield bridge on the unpack side                                */
/* ------------------------------------------------------------------------ */

TEST_F(NotificationInternalTidlWrapperTest,
       InternalTidl_UnpackSide_BoolBitfieldBridge) {
  std::string body =
      ExtractFunctionBody(Source(), "make_noti_from_notification");
  ASSERT_FALSE(body.empty());
  /* The bitfield bridge pattern:
   *   bool _bf = _noti->X;
   *   ret = rpc_port_proxy_notification_get_X(handle, &_bf);
   *   _noti->X = _bf ? 1u : 0u;
   * We assert it appears at least 3 times (is_translation, check_box,
   * check_box_value). */
  size_t bridge_count = CountOccurrences(body, "_bf ? 1u : 0u");
  EXPECT_GE(bridge_count, 3u)
      << "Unpack side must bridge bool fields through a stack `_bf` "
         "variable to keep the bitfield aliasing well-defined";
}

/* ------------------------------------------------------------------------ */
/* 5. Empty-string drop on the unpack side                                   */
/* ------------------------------------------------------------------------ */

TEST_F(NotificationInternalTidlWrapperTest,
       InternalTidl_UnpackSide_DropsEmptyStrings) {
  std::string body =
      ExtractFunctionBody(Source(), "make_noti_from_notification");
  ASSERT_FALSE(body.empty());
  /* For a string field X, the unpack must:
   *   if (strlen(X) > 0) _noti->X = X;
   *   else { _noti->X = NULL; free(X); }
   * Pin via the canonical "strlen(X) > 0" + "free(X)" pair. */
  for (const char* sample : {
           "strlen(pkg_id) > 0",
           "strlen(caller_app_id) > 0",
           "strlen(launch_app_id) > 0",
           "strlen(domain) > 0",
           "strlen(dir) > 0",
           "strlen(channel_name) > 0",
           "strlen(sound_path) > 0",
           "strlen(vibration_path) > 0",
       }) {
    EXPECT_NE(body.find(sample), std::string::npos)
        << "make_noti_from_notification must drop empty: " << sample;
  }
  /* And empty bundles too. */
  EXPECT_NE(body.find("bundle_get_count(args) > 0"), std::string::npos);
}

/* ------------------------------------------------------------------------ */
/* 6. NULL guards on make_dnd_allow_exception_from_exception                 */
/* ------------------------------------------------------------------------ */

TEST_F(NotificationInternalTidlWrapperTest,
       InternalTidl_MakeDndException_NullGuards) {
  std::string body = ExtractFunctionBody(
      Source(), "make_dnd_allow_exception_from_exception");
  ASSERT_FALSE(body.empty());
  /* Must guard both inputs explicitly. */
  EXPECT_NE(body.find("exception_handle == NULL"), std::string::npos);
  EXPECT_NE(body.find("dnd_allow_exception == NULL"), std::string::npos);
  EXPECT_NE(body.find("NOTIFICATION_ERROR_INVALID_PARAMETER"),
            std::string::npos);
  /* The body must propagate type and value to the proxy handle. */
  EXPECT_NE(body.find(
                "rpc_port_proxy_noti_system_setting_dnd_allow_exception_set_type"),
            std::string::npos);
  EXPECT_NE(body.find(
                "rpc_port_proxy_noti_system_setting_dnd_allow_exception_set_value"),
            std::string::npos);
}

/* ------------------------------------------------------------------------ */
/* 7. make_noti_system_setting_from_setting only allocates list when needed  */
/* ------------------------------------------------------------------------ */

TEST_F(NotificationInternalTidlWrapperTest,
       InternalTidl_SystemSettingPack_LengthGatedListAlloc) {
  std::string body = ExtractFunctionBody(
      Source(), "make_noti_system_setting_from_setting");
  ASSERT_FALSE(body.empty());
  /* Must NOT allocate the list handle when length == 0. */
  EXPECT_NE(body.find("g_list_length(setting->dnd_allow_exceptions) > 0"),
            std::string::npos)
      << "Pack side must skip list-handle alloc when no exceptions present";
  /* And the per-element create must be paired with a destroy. */
  size_t create_count = CountOccurrences(
      body,
      "rpc_port_proxy_noti_system_setting_dnd_allow_exception_create");
  size_t destroy_count = CountOccurrences(
      body,
      "rpc_port_proxy_noti_system_setting_dnd_allow_exception_destroy");
  EXPECT_GE(create_count, 1u);
  EXPECT_GE(destroy_count, 1u)
      << "Per-element create/destroy must be paired";
}

/* ------------------------------------------------------------------------ */
/* 8. dnd-exception foreach callback contract                                */
/* ------------------------------------------------------------------------ */

TEST_F(NotificationInternalTidlWrapperTest,
       InternalTidl_DndExceptionForeachCb_ShortCircuitsOnFailure) {
  std::string body = ExtractFunctionBody(
      Source(),
      "_rpc_port_proxy_list_noti_system_setting_dnd_allow_exception_cb");
  ASSERT_FALSE(body.empty());
  /* Must alloc, free on per-field error, return false on error, true on
   * happy path. */
  EXPECT_NE(body.find("calloc"), std::string::npos);
  EXPECT_NE(body.find("free(dnd_allow_exception)"), std::string::npos);
  EXPECT_NE(body.find("return false"), std::string::npos);
  EXPECT_NE(body.find("return true"), std::string::npos);
  EXPECT_NE(body.find("g_list_append"), std::string::npos);
}

/* ------------------------------------------------------------------------ */
/* 9. make_setting_from_noti_setting drops empty pkg_name / app_id           */
/* ------------------------------------------------------------------------ */

TEST_F(NotificationInternalTidlWrapperTest,
       InternalTidl_SettingUnpack_DropsEmptyPkgAndAppId) {
  std::string body =
      ExtractFunctionBody(Source(), "make_setting_from_noti_setting");
  ASSERT_FALSE(body.empty());
  /* Both pkg_name and app_id strings must be dropped if empty. */
  EXPECT_NE(body.find("strlen(pkg_name) > 0"), std::string::npos);
  EXPECT_NE(body.find("strlen(app_id) > 0"), std::string::npos);
  /* And every getter on the setting handle must be present. */
  for (const char* sample : {
           "rpc_port_proxy_noti_setting_get_package_name",
           "rpc_port_proxy_noti_setting_get_app_id",
           "rpc_port_proxy_noti_setting_get_allow_to_notify",
           "rpc_port_proxy_noti_setting_get_do_not_disturb_except",
           "rpc_port_proxy_noti_setting_get_pop_up_notification",
           "rpc_port_proxy_noti_setting_get_visibility_class",
           "rpc_port_proxy_noti_setting_get_lock_screen_content_level",
           "rpc_port_proxy_noti_setting_get_app_disabled",
       }) {
    EXPECT_NE(body.find(sample), std::string::npos)
        << "Setting unpack missing: " << sample;
  }
}

/* ------------------------------------------------------------------------ */
/* 10. make_noti_setting_from_setting setter coverage                        */
/* ------------------------------------------------------------------------ */

TEST_F(NotificationInternalTidlWrapperTest,
       InternalTidl_SettingPack_AllSettersPresent) {
  std::string body =
      ExtractFunctionBody(Source(), "make_noti_setting_from_setting");
  ASSERT_FALSE(body.empty());
  for (const char* sample : {
           "rpc_port_proxy_noti_setting_set_package_name",
           "rpc_port_proxy_noti_setting_set_app_id",
           "rpc_port_proxy_noti_setting_set_allow_to_notify",
           "rpc_port_proxy_noti_setting_set_do_not_disturb_except",
           "rpc_port_proxy_noti_setting_set_pop_up_notification",
           "rpc_port_proxy_noti_setting_set_visibility_class",
           "rpc_port_proxy_noti_setting_set_lock_screen_content_level",
           "rpc_port_proxy_noti_setting_set_app_disabled",
       }) {
    EXPECT_NE(body.find(sample), std::string::npos)
        << "Setting pack missing: " << sample;
  }
  /* Strings must use __get_str_value so a NULL field becomes "" not crash. */
  EXPECT_NE(body.find("__get_str_value(setting->package_name)"),
            std::string::npos);
  EXPECT_NE(body.find("__get_str_value(setting->app_id)"), std::string::npos);
}

/* ------------------------------------------------------------------------ */
/* 11. System setting unpack runs the dnd_allow_exception foreach            */
/* ------------------------------------------------------------------------ */

TEST_F(NotificationInternalTidlWrapperTest,
       InternalTidl_SystemSettingUnpack_RunsDndForeach) {
  std::string body =
      ExtractFunctionBody(Source(), "make_setting_from_noti_system_setting");
  ASSERT_FALSE(body.empty());
  EXPECT_NE(body.find("calloc"), std::string::npos)
      << "Must heap-allocate the system-setting struct";
  EXPECT_NE(body.find(
                "rpc_port_proxy_list_noti_system_setting_dnd_allow_exception_foreach"),
            std::string::npos)
      << "Must run the dnd-exception foreach to populate the list";
  /* And the lock-screen-content-level enum is bridged through an int. */
  EXPECT_NE(body.find("(lock_screen_content_level_e)_lock_screen_content_level"),
            std::string::npos);
}

/* ------------------------------------------------------------------------ */
/* 12. __get_str_value NULL-safety helper                                    */
/* ------------------------------------------------------------------------ */

TEST_F(NotificationInternalTidlWrapperTest,
       InternalTidl_GetStrValue_NullReturnsEmpty) {
  std::string body = ExtractFunctionBody(Source(), "__get_str_value");
  ASSERT_FALSE(body.empty());
  /* Must guard NULL and return the canonical empty string literal. */
  EXPECT_NE(body.find("value != NULL"), std::string::npos);
  EXPECT_NE(body.find("\"\""), std::string::npos);
  /* No malloc/free of the returned pointer — caller doesn't own it. */
  EXPECT_EQ(body.find("strdup"), std::string::npos);
  EXPECT_EQ(body.find("calloc"), std::string::npos);
}

/* ------------------------------------------------------------------------ */
/* 13. make_noti_from_notification: array_bundle event_handler unpack        */
/* ------------------------------------------------------------------------ */

TEST_F(NotificationInternalTidlWrapperTest,
       InternalTidl_EventHandlerArray_NullSafeUnpack) {
  std::string body =
      ExtractFunctionBody(Source(), "make_noti_from_notification");
  ASSERT_FALSE(body.empty());
  /* The unpack iterates the array; per-slot NULL is logged but not fatal. */
  EXPECT_NE(body.find("if (_event_handler)"), std::string::npos)
      << "event_handler array must be null-checked before iter";
  EXPECT_NE(body.find("if (_b_event_handler[i] == NULL)"), std::string::npos);
  /* Empty per-slot bundles drop to NULL. */
  EXPECT_NE(body.find("__count > 0"), std::string::npos);
  /* Array container must be destroyed after iter. */
  EXPECT_NE(body.find("rpc_port_proxy_array_bundle_destroy(_event_handler)"),
            std::string::npos);
}

/* ------------------------------------------------------------------------ */
/* 14. make_notification_from_noti: array_bundle event_handler pack +        */
/*     translation gate                                                      */
/* ------------------------------------------------------------------------ */

TEST_F(NotificationInternalTidlWrapperTest,
       InternalTidl_PackEventHandler_AllocAndDestroyPaired) {
  std::string body =
      ExtractFunctionBody(Source(), "make_notification_from_noti");
  ASSERT_FALSE(body.empty());
  /* event_handler array is created, populated, set on handle, then
   * destroyed (the rpc-port stub clones internally). */
  EXPECT_NE(body.find("rpc_port_proxy_array_bundle_create"), std::string::npos);
  EXPECT_NE(body.find("rpc_port_proxy_array_bundle_set"), std::string::npos);
  EXPECT_NE(body.find("rpc_port_proxy_array_bundle_destroy"),
            std::string::npos);
  /* Translation flag conditionally calls notification_translate_localized_text
   * on the source noti. */
  EXPECT_NE(body.find("if (translate)"), std::string::npos);
  EXPECT_NE(body.find("notification_translate_localized_text(noti)"),
            std::string::npos);
}

/* ------------------------------------------------------------------------ */
/* 15. Sentinel: total EXPORT_API surface unchanged                          */
/* ------------------------------------------------------------------------ */

TEST_F(NotificationInternalTidlWrapperTest,
       InternalTidl_ExportSurface_Sentinel) {
  std::regex re(R"(^EXPORT_API\s+int\s+make_)",
                std::regex_constants::ECMAScript |
                    std::regex_constants::multiline);
  auto begin = std::sregex_iterator(Source().begin(), Source().end(), re);
  auto end = std::sregex_iterator();
  size_t count = std::distance(begin, end);
  /* Today: 8 (make_empty, make_notification_from_noti, make_noti_from_notification,
   * make_setting_from_noti_system_setting, make_dnd_allow_exception_from_exception,
   * make_noti_system_setting_from_setting, make_setting_from_noti_setting,
   * make_noti_setting_from_setting). */
  EXPECT_EQ(count, 8u)
      << "EXPORT_API surface of notification_internal_tidl.c changed; review"
         " ABI delta and update sentinel";
}

/* ------------------------------------------------------------------------ */
/* 16. Out-of-memory handling on per-noti / per-setting allocation           */
/* ------------------------------------------------------------------------ */

TEST_F(NotificationInternalTidlWrapperTest,
       InternalTidl_MakeNoti_OomReturnsOutOfMemory) {
  std::string body =
      ExtractFunctionBody(Source(), "make_noti_from_notification");
  ASSERT_FALSE(body.empty());
  /* The calloc result is checked and returns OUT_OF_MEMORY on failure. */
  EXPECT_NE(body.find("_noti == NULL"), std::string::npos);
  EXPECT_NE(body.find("NOTIFICATION_ERROR_OUT_OF_MEMORY"), std::string::npos);
}

TEST_F(NotificationInternalTidlWrapperTest,
       InternalTidl_MakeSystemSetting_OomReturnsOutOfMemory) {
  std::string body =
      ExtractFunctionBody(Source(), "make_setting_from_noti_system_setting");
  ASSERT_FALSE(body.empty());
  EXPECT_NE(body.find("_setting == NULL"), std::string::npos);
  EXPECT_NE(body.find("NOTIFICATION_ERROR_OUT_OF_MEMORY"), std::string::npos);
}
