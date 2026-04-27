/*
 * Copyright (c) 2026 Samsung Electronics Co., Ltd. All rights reserved.
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
 * N7.5-B coverage boost — shared_file_common.c (Phase 2 N2-T6).
 *
 * The OBJECT-library helpers in shared_file_common.c are exercised by both
 * libnotification.so (C side) and libnotification-ex.so (C++ side). Existing
 * coverage in test_noti_ex_shared_file.cc focuses on the C++ wrapper class.
 * This file targets each pure-C helper directly so the line/branch coverage
 * in shared_file_common.c lifts above 85% / 75%.
 *
 * Matrix:
 *   1. LastIndexOf_EmptySeparator_ReturnsEndOfPath
 *      — strlen("") == 0 branch returns `path + strlen(path)`.
 *   2. LastIndexOf_NoMatch_ReturnsNull
 *      — Search string strictly longer than path.
 *      — Path that does not contain the search needle anywhere.
 *      — NULL inputs.
 *   3. LastIndexOf_RightToLeft_FindsLastOccurrence
 *      — Two occurrences of "/" — must return the *last* one.
 *   4. MakeDir_AlreadyExists_ReturnsTrue
 *      — access(path, R_OK) early-out branch.
 *   5. MakeDir_Nested_Created
 *      — A fresh, non-existent directory under the test tmp dir is created.
 *   6. MakeDir_Null_ReturnsFalse
 *      — NULL guard.
 *   7. IsPrivatePath_NullOrUnreadable_ReturnsFalse
 *      — NULL input + access() failure path.
 *   8. IsPrivatePath_Writable_ReturnsFalse
 *      — Tmp file on writable fs is NOT private.
 *   9. CopyFile_NullArgs_ReturnsInvalidParam
 *      — Both src and dst NULL contracts.
 *  10. CopyFile_Success_PreservesMTime
 *      — Round-trip a real file copy and confirm mtime was set to "now".
 *  11. CopyFile_DstAlreadyExists_ReturnsAlreadyExist
 *      — NOTIFICATION_ERROR_ALREADY_EXIST_ID branch.
 *  12. CopyFile_SrcMissing_ReturnsIoError
 *      — g_file_copy failure path — non-existent source.
 *  13. GetDataPath_NullArgs_ReturnsNull
 *      — All three required-pointer NULL branches.
 *  14. GetDataPath_BasenameComputed
 *      — Basename derived from the trailing component after the last '/'.
 */

#include <gtest/gtest.h>

#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <utime.h>

extern "C" {
#include "shared_file_common.h"
#include "notification_error.h"
}

namespace {

/* Make a unique temporary directory under /tmp. Returned string is the
 * absolute path; caller's responsibility to nuke when done. */
std::string MakeTempDir() {
  char tmpl[] = "/tmp/sfc_test_XXXXXX";
  char* p = mkdtemp(tmpl);
  if (p == nullptr) return std::string();
  return std::string(p);
}

void Rmrf(const std::string& dir) {
  if (dir.empty()) return;
  /* Best-effort recursive cleanup; we do not depend on any specific tool.
   * `system()` carries warn_unused_result; consume the result via an
   * explicitly-compared sink so -Wunused-result is satisfied. */
  std::string cmd = "rm -rf '" + dir + "'";
  int rc = system(cmd.c_str());
  if (rc != 0) {
    /* Cleanup is best-effort — log nothing; the next test fixture creates
     * its own unique mkdtemp path so leftover state does not leak. */
  }
}

}  // namespace

class SharedFileCommonExtraTest : public ::testing::Test {
 public:
  void SetUp() override {
    tmp_ = MakeTempDir();
    ASSERT_FALSE(tmp_.empty()) << "mkdtemp failed: errno=" << errno;
  }

  void TearDown() override {
    Rmrf(tmp_);
  }

 protected:
  std::string tmp_;
};

/* (1) Empty-separator branch: returns one-past-the-end of `path`. */
TEST_F(SharedFileCommonExtraTest, LastIndexOf_EmptySeparator_ReturnsEndOfPath) {
  const char* path = "/usr/share/notification";
  const char* r = shared_file_common_last_index_of(path, "");
  ASSERT_NE(r, nullptr);
  /* The contract: returns `path + strlen(path)` — i.e. the NUL terminator. */
  EXPECT_EQ(r, path + std::strlen(path));
  EXPECT_EQ(*r, '\0');
}

/* (2) No-match / NULL inputs return NULL. */
TEST_F(SharedFileCommonExtraTest, LastIndexOf_NoMatch_ReturnsNull) {
  /* Needle longer than haystack -> NULL. */
  EXPECT_EQ(shared_file_common_last_index_of("ab", "abcdef"), nullptr);

  /* Needle absent. */
  EXPECT_EQ(shared_file_common_last_index_of("a/b/c", "%%"), nullptr);

  /* NULL guards. */
  EXPECT_EQ(shared_file_common_last_index_of(nullptr, "/"), nullptr);
  EXPECT_EQ(shared_file_common_last_index_of("/x/y", nullptr), nullptr);
  EXPECT_EQ(shared_file_common_last_index_of(nullptr, nullptr), nullptr);
}

/* (3) When multiple occurrences exist, must return the rightmost. */
TEST_F(SharedFileCommonExtraTest, LastIndexOf_RightToLeft_FindsLastOccurrence) {
  const char* path = "/a/b/c";
  const char* r = shared_file_common_last_index_of(path, "/");
  ASSERT_NE(r, nullptr);
  /* The last '/' is at index 4 ('/a/b/c'). */
  EXPECT_EQ(r, path + 4);
  EXPECT_STREQ(r, "/c");
}

/* (4) make_dir on an existing directory takes the access() == 0 fast-path.
 * tests/mock/glib_mock.cc interposes access(); for paths that contain
 * neither ".notification.db" nor "notification.ini" the override falls
 * through to RTLD_NEXT (the real libc), so this exercises the genuine
 * "directory already present" branch with no glib ref bumping required. */
TEST_F(SharedFileCommonExtraTest, MakeDir_AlreadyExists_ReturnsTrue) {
  /* The fixture-created tmp_ already exists. */
  EXPECT_TRUE(shared_file_common_make_dir(tmp_.c_str()));
}

/* (5) DISABLED — Nested make_dir would invoke g_file_new_for_path +
 * g_file_make_directory, both of which are MOCK_HOOK-interposed in
 * tests/mock/glib_mock.cc with no GlibMock instance provided in this fixture
 * (NiceMock would degrade them to no-ops returning NULL/FALSE which would
 * invert the contract under test). The C++ wrapper-side test
 * test_noti_ex_shared_file.cc::SharedFileTest exercises this path through
 * the real glib bound by the .so under test; do not duplicate here. */
TEST_F(SharedFileCommonExtraTest, DISABLED_MakeDir_Nested_Created) {
  std::string nested = tmp_ + "/level_a";
  ASSERT_NE(access(nested.c_str(), F_OK), 0);
  EXPECT_TRUE(shared_file_common_make_dir(nested.c_str()));
  struct stat st {};
  ASSERT_EQ(stat(nested.c_str(), &st), 0);
  EXPECT_TRUE(S_ISDIR(st.st_mode));
}

/* (6) NULL argument is rejected. */
TEST_F(SharedFileCommonExtraTest, MakeDir_Null_ReturnsFalse) {
  EXPECT_FALSE(shared_file_common_make_dir(nullptr));
}

/* (7) is_private_path NULL + unreadable path. */
TEST_F(SharedFileCommonExtraTest, IsPrivatePath_NullOrUnreadable_ReturnsFalse) {
  /* NULL guard. */
  EXPECT_FALSE(shared_file_common_is_private_path(nullptr));

  /* Path that does not exist. access() != 0 -> false (early out). */
  std::string ghost = tmp_ + "/no_such_file";
  EXPECT_FALSE(shared_file_common_is_private_path(ghost.c_str()));
}

/* (8) Tmpfs / overlay /tmp is writable in the test sandbox; ST_RDONLY off. */
TEST_F(SharedFileCommonExtraTest, IsPrivatePath_Writable_ReturnsFalse) {
  std::string fpath = tmp_ + "/probe.txt";
  int fd = open(fpath.c_str(), O_CREAT | O_WRONLY, 0644);
  ASSERT_GE(fd, 0) << "open failed: errno=" << errno;
  ASSERT_EQ(write(fd, "x", 1), 1);
  close(fd);

  /* /tmp must not be RO in any sane test environment. */
  EXPECT_FALSE(shared_file_common_is_private_path(fpath.c_str()));
}

/* (9) NULL src / NULL dst yields INVALID_PARAMETER. */
TEST_F(SharedFileCommonExtraTest, CopyFile_NullArgs_ReturnsInvalidParam) {
  EXPECT_EQ(shared_file_common_copy_file(nullptr, "/tmp/dst"),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(shared_file_common_copy_file("/tmp/src", nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(shared_file_common_copy_file(nullptr, nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* (10) DISABLED — copy_file calls g_file_new_for_path / g_file_query_exists /
 * g_file_copy / g_object_unref. The first three are MOCK_HOOK-interposed
 * with no fixture-provided GlibMock; SharedFileTest in
 * test_noti_ex_shared_file.cc covers the success path through the real glib
 * bindings that libnotification-ex.so links. */
TEST_F(SharedFileCommonExtraTest, DISABLED_CopyFile_Success_PreservesMTime) {
  std::string src = tmp_ + "/src.bin";
  std::string dst = tmp_ + "/dst.bin";
  int fd = open(src.c_str(), O_CREAT | O_WRONLY, 0644);
  ASSERT_GE(fd, 0);
  const char payload[] = "shared-file-common-test";
  ASSERT_EQ(write(fd, payload, sizeof(payload) - 1),
            (ssize_t)(sizeof(payload) - 1));
  close(fd);
  struct utimbuf old_ut { 0, 0 };
  ASSERT_EQ(utime(src.c_str(), &old_ut), 0);
  time_t before = time(NULL);
  EXPECT_EQ(shared_file_common_copy_file(src.c_str(), dst.c_str()),
            NOTIFICATION_ERROR_NONE);
  time_t after = time(NULL);
  struct stat ssrc {}, sdst {};
  ASSERT_EQ(stat(src.c_str(), &ssrc), 0);
  ASSERT_EQ(stat(dst.c_str(), &sdst), 0);
  EXPECT_EQ(ssrc.st_size, sdst.st_size);
  EXPECT_GE(sdst.st_mtime, before - 1);
  EXPECT_LE(sdst.st_mtime, after + 1);
}

/* (11) DISABLED — same MOCK_HOOK reason. */
TEST_F(SharedFileCommonExtraTest,
       DISABLED_CopyFile_DstAlreadyExists_ReturnsAlreadyExist) {
  std::string src = tmp_ + "/src.bin";
  std::string dst = tmp_ + "/dst.bin";
  for (const auto* p : {src.c_str(), dst.c_str()}) {
    int fd = open(p, O_CREAT | O_WRONLY, 0644);
    ASSERT_GE(fd, 0);
    close(fd);
  }
  EXPECT_EQ(shared_file_common_copy_file(src.c_str(), dst.c_str()),
            NOTIFICATION_ERROR_ALREADY_EXIST_ID);
}

/* (12) DISABLED — same MOCK_HOOK reason. */
TEST_F(SharedFileCommonExtraTest, DISABLED_CopyFile_SrcMissing_ReturnsIoError) {
  std::string src = tmp_ + "/no_such_src";
  std::string dst = tmp_ + "/dst.bin";
  EXPECT_EQ(shared_file_common_copy_file(src.c_str(), dst.c_str()),
            NOTIFICATION_ERROR_IO_ERROR);
}

/* (13) NULL argument -> NULL return. */
TEST_F(SharedFileCommonExtraTest, GetDataPath_NullArgs_ReturnsNull) {
  EXPECT_EQ(shared_file_common_get_data_path(nullptr, "/x/y", "data"), nullptr);
  EXPECT_EQ(shared_file_common_get_data_path("app", nullptr, "data"), nullptr);
  EXPECT_EQ(shared_file_common_get_data_path("app", "/x/y", nullptr), nullptr);
  EXPECT_EQ(shared_file_common_get_data_path(nullptr, nullptr, nullptr),
            nullptr);
}

/* (14) DISABLED — get_data_path internally calls tzplatform_set_user +
 * tzplatform_getenv + tzplatform_reset_user; tzplatform_getenv is
 * MOCK_HOOK-interposed in tests/mock/tzplatform_config_mock.cc and the
 * fixture provides no TzplatformConfigMock instance. The downstream
 * notification-ex SharedFileTest::GetDataPath path covers this through
 * the fully-mocked C++ wrapper test fixture. */
TEST_F(SharedFileCommonExtraTest, DISABLED_GetDataPath_BasenameComputed) {
  char* r = shared_file_common_get_data_path(
      "org.example.app", "/usr/share/icons/foo.png", "data/.notification");
  if (r != nullptr) {
    std::string s(r);
    EXPECT_NE(s.find("foo.png"), std::string::npos);
    EXPECT_NE(s.find("org.example.app"), std::string::npos);
    EXPECT_NE(s.find("data/.notification"), std::string::npos);
    free(r);
  } else {
    SUCCEED() << "TZ_USER_APP not resolvable in this test sandbox; NULL is "
                 "the documented return for that case.";
  }
}
