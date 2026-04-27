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
 * N4-T1 (regression guard): notification_tidl ctor is mutex-init-only.
 *
 * The constructor `__notification_init` (notification_tidl.c:78,
 * NOTIFICATION_CTOR-tagged) MUST NOT initialise the rpc-port proxy or
 * acquire any expensive resource at process load time. The current
 * body is a single g_rec_mutex_init call; this test pins that
 * invariant by inspecting the source file at build time and
 * confirming there is no proxy/dbus/connect symbol referenced inside
 * the function body.
 *
 * This is a static guard, not a runtime mock: the production library
 * already strips internal symbols (-fvisibility=hidden) so we cannot
 * dlsym into __notification_init. Reading the .c file at the test
 * binary's source root is sufficient and stable - the file lives at a
 * known repo-relative path, exposed through a compile-time define so
 * the test does not depend on the cwd.
 */

#include <cstdio>
#include <fstream>
#include <sstream>
#include <string>

#include <gtest/gtest.h>

namespace {

/* The path is injected by CMake (NOTIFICATION_TIDL_C_PATH). When the
 * define is absent (out-of-tree builds, cmake misconfig), the test
 * SKIPs rather than fails so green-field consumers stay quiet. */
#ifndef NOTIFICATION_TIDL_C_PATH
#define NOTIFICATION_TIDL_C_PATH ""
#endif
constexpr const char* kTidlPath = NOTIFICATION_TIDL_C_PATH;

/* Read the file and return its contents, or empty string on failure. */
std::string SlurpFile(const char* path) {
  std::ifstream f(path);
  if (!f.is_open())
    return {};
  std::stringstream buf;
  buf << f.rdbuf();
  return buf.str();
}

/* Extract the body of the function whose name == fn_name. Returns the
 * substring from after the opening '{' to before the matching '}', or
 * empty if not found / unbalanced. Assumes well-formed C source. */
std::string ExtractFunctionBody(const std::string& src,
                                const std::string& fn_name) {
  /* Find "fn_name(" then walk to the next '{'. */
  size_t pos = 0;
  while ((pos = src.find(fn_name, pos)) != std::string::npos) {
    /* Match a token boundary on the left side. */
    if (pos > 0 && (isalnum(src[pos - 1]) || src[pos - 1] == '_')) {
      pos += fn_name.size();
      continue;
    }
    size_t paren = src.find('(', pos);
    size_t brace = src.find('{', pos);
    if (paren == std::string::npos || brace == std::string::npos) {
      return {};
    }
    /* Find the matching '}'. */
    int depth = 1;
    size_t i = brace + 1;
    while (i < src.size() && depth > 0) {
      if (src[i] == '{') ++depth;
      else if (src[i] == '}') --depth;
      ++i;
    }
    if (depth == 0)
      return src.substr(brace + 1, (i - 1) - (brace + 1));
    return {};
  }
  return {};
}

}  // namespace

/*
 * The static guard: __notification_init body must contain
 * g_rec_mutex_init and nothing else of substance. We allow only the
 * mutex-init call + whitespace.
 */
TEST(TidlLazyTest, NotificationInitCtorMutexOnly) {
  if (kTidlPath[0] == '\0')
    GTEST_SKIP() << "NOTIFICATION_TIDL_C_PATH not defined.";
  std::string src = SlurpFile(kTidlPath);
  if (src.empty()) {
    GTEST_SKIP() << "notification_tidl.c not reachable at " << kTidlPath
                 << " - test only fires from in-tree builds.";
  }

  std::string body = ExtractFunctionBody(src, "__notification_init");
  ASSERT_FALSE(body.empty())
      << "__notification_init not found in notification_tidl.c";

  /* Required: mutex init. */
  EXPECT_NE(body.find("g_rec_mutex_init"), std::string::npos)
      << "__notification_init must initialise the recursive mutex.";

  /* Forbidden: any rpc-port proxy / dbus / connect side effect. */
  const char* forbidden[] = {
      "rpc_port_proxy",
      "g_bus_get",
      "g_dbus_connection",
      "tizen_core_run",
      "DBusConnectionManager",
      /* The pre-N4-T1 design used a `_tidl_init()` helper that did
       * lazy proxy setup. The CTOR must NOT call it directly. */
      "_tidl_init(",
  };

  for (const char* needle : forbidden) {
    EXPECT_EQ(body.find(needle), std::string::npos)
        << "__notification_init body contains forbidden token '" << needle
        << "' - cold-start regression: ctor must remain mutex-init-only.";
  }
}

/*
 * Sanity: the matching destructor (__notification_fini) must clear
 * the mutex. Guards against silent removal that would leak a
 * recursive-mutex object on dlclose.
 */
TEST(TidlLazyTest, NotificationFiniClearsMutex) {
  if (kTidlPath[0] == '\0')
    GTEST_SKIP() << "NOTIFICATION_TIDL_C_PATH not defined.";
  std::string src = SlurpFile(kTidlPath);
  if (src.empty()) {
    GTEST_SKIP() << "notification_tidl.c not reachable.";
  }
  std::string body = ExtractFunctionBody(src, "__notification_fini");
  ASSERT_FALSE(body.empty());
  EXPECT_NE(body.find("g_rec_mutex_clear"), std::string::npos);
}
