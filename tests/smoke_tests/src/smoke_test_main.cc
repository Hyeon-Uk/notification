/*
 * Copyright (c) 2026 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#include <gtest/gtest.h>

#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <vector>

/* app_launcher detaches stdout/stderr from the launching shell and drops
 * the calling shell's env. Mirror the integtests pattern: redirect
 * gtest output to a known file, and optionally honor a filter file. */
static constexpr const char* kOutputLog = "/tmp/notification-smoketests.log";
static constexpr const char* kFilterFile = "/tmp/smoke_gtest_filter.txt";

int main(int argc, char** argv) {
  freopen(kOutputLog, "w", stdout);
  freopen(kOutputLog, "a", stderr);

  char filter_buf[512] = {0};
  char filter_arg[600] = {0};
  std::vector<char*> argv_vec(argv, argv + argc);
  if (FILE* f = fopen(kFilterFile, "r")) {
    if (fgets(filter_buf, sizeof(filter_buf), f) != nullptr) {
      size_t n = strlen(filter_buf);
      if (n > 0 && filter_buf[n - 1] == '\n') filter_buf[n - 1] = '\0';
      snprintf(filter_arg, sizeof(filter_arg), "--gtest_filter=%s", filter_buf);
      argv_vec.push_back(filter_arg);
    }
    fclose(f);
  }
  int new_argc = static_cast<int>(argv_vec.size());
  char** new_argv = argv_vec.data();

  testing::InitGoogleTest(&new_argc, new_argv);
  int ret = RUN_ALL_TESTS();
  fflush(stdout);
  fflush(stderr);
  return ret;
}
