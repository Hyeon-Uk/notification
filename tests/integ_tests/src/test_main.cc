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

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <vector>

/* When launched via app_launcher (-s <appid>) the binary's stdout/stderr
 * is detached from the launching shell. Redirect both to a known file so
 * the gtest output can be retrieved via `sdb pull /tmp/notification-integtests.log`.
 * Direct invocation still works — output is mirrored to the log only.
 *
 * app_launcher also drops the calling shell's environment, so GTEST_FILTER
 * can't be set that way. Instead, if /tmp/gtest_filter.txt exists, its
 * first line becomes --gtest_filter=<line>.
 */
static constexpr const char* kOutputLog = "/tmp/notification-integtests.log";
static constexpr const char* kFilterFile = "/tmp/gtest_filter.txt";

int main(int argc, char** argv) {
  freopen(kOutputLog, "w", stdout);
  freopen(kOutputLog, "a", stderr);

  /* Inject --gtest_filter from /tmp/gtest_filter.txt for isolated runs
   * under app_launcher. */
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

  int ret = -1;

  try {
    testing::InitGoogleTest(&new_argc, new_argv);
  } catch(...) {
    std::cout << "Exception occurred" << std::endl;
  }

  try {
    ret = RUN_ALL_TESTS();
  } catch (const ::testing::internal::GoogleTestFailureException& e) {
    ret = -1;
    std::cout << "GoogleTestFailureException was thrown:" << e.what() << std::endl;
  }

  fflush(stdout);
  fflush(stderr);
  return ret;
}
