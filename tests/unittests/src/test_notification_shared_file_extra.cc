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
 * N7.5-C coverage boost — notification_shared_file.c (Mid 6 file group).
 *
 * Most of notification_shared_file.c is wrapped in LCOV_EXCL because it
 * either (a) lives behind security_manager_*, tizen_core_*, package_manager
 * or (b) is exercised end-to-end through the notification-test-app
 * integration tests. The slim hermetic surface we cover is
 * notification_validate_private_sharing(noti) — pure C logic comparing the
 * image / sound / vibration paths between the b_image_path bundle and the
 * b_priv_image_path bundle. No IPC.
 *
 * The internal helper notification_check_file_path_is_private() is declared
 * in notification_shared_file.h but is NOT EXPORT_API in the .c — therefore
 * it is hidden under -fvisibility=hidden and not reachable from the test
 * binary. We deliberately do not flip its visibility here (v6 §N7.5-C says
 * "minimal production change") and leave it for a follow-up "contract fix"
 * commit if a future reader wants to test it directly.
 *
 * Matrix:
 *   1. SharedFile_Validate_BothNull_BundleSafe
 *      — Validate on a bare notification_h returns true (no images to
 *        compare).
 *   2. SharedFile_Validate_DistinctSoundPaths_ReturnsTrue
 *      — sound_path != priv_sound_path -> true
 *   3. SharedFile_Validate_SameSoundPath_ReturnsFalse
 *      — sound_path == priv_sound_path -> false
 *   4. SharedFile_Validate_DistinctVibrationPaths_ReturnsTrue
 *   5. SharedFile_Validate_SameVibrationPath_ReturnsFalse
 *   6. SharedFile_Validate_DistinctImagePaths_ReturnsTrue
 *   7. SharedFile_Validate_SameImagePath_ReturnsFalse
 *   8. SharedFile_Validate_OnlySound_NoImage_NoVib_ReturnsTrue
 *      — Setting only sound paths leaves image/vib branches untouched.
 *   9. SharedFile_Validate_PartialImageBundle_NoMatchingKey_ReturnsTrue
 *      — When b_image_path/b_priv_image_path have non-overlapping keys,
 *        the inner loop continues without comparing — must return true.
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <bundle.h>
#include <bundle_internal.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <string>
#include <sys/stat.h>
#include <unistd.h>

#include <aul.h>

#include "mock/test_fixture.hh"
#include "mock/aul_mock.hh"
#include "mock/glib_mock.hh"

extern "C" {
#include <notification.h>
#include <notification_internal.h>
#include <notification_private.h>
#include <notification_shared_file.h>
#include <notification_type.h>
}

namespace {

class Mocks :
    virtual public ::testing::NiceMock<AulMock>,
    virtual public ::testing::NiceMock<GlibMock> {};

int FakeAulAppGetAppidByPid(int /*pid*/, char* appid, int len) {
  snprintf(appid, len, "%s", "org.tizen.unittest.app");
  return AUL_R_OK;
}

}  // namespace

using ::testing::_;
using ::testing::Invoke;

class NotificationSharedFileExtraTest : public TestFixture {
 public:
  NotificationSharedFileExtraTest()
      : TestFixture(std::make_unique<::Mocks>()) {}
  void SetUp() override {
    EXPECT_CALL(GetMock<AulMock>(), aul_app_get_appid_bypid(_, _, _))
        .WillRepeatedly(Invoke(FakeAulAppGetAppidByPid));
    char tmpl[] = "/tmp/noti_sf_XXXXXX";
    char* dir = mkdtemp(tmpl);
    ASSERT_NE(dir, nullptr);
    tmpdir_ = dir;
  }
  void TearDown() override {
    if (!real_file_.empty()) unlink(real_file_.c_str());
    if (!tmpdir_.empty()) rmdir(tmpdir_.c_str());
  }

  std::string MakeRealFile(const char* basename, const char* contents) {
    std::string path = tmpdir_ + "/" + basename;
    int fd = open(path.c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if (fd < 0) return {};
    if (contents) {
      ssize_t n = write(fd, contents, strlen(contents));
      (void)n;
    }
    close(fd);
    real_file_ = path;
    return path;
  }

  std::string tmpdir_;
  std::string real_file_;
};

/* (1) validate_private_sharing on a bare notification (no image/sound/
 *     vibration set). Must return true (no fields to compare -> nothing
 *     conflicts). */
TEST_F(NotificationSharedFileExtraTest, SharedFile_Validate_BothNull_BundleSafe) {
  notification_h n = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(n, nullptr);
  EXPECT_TRUE(notification_validate_private_sharing(n));
  notification_free(n);
}

/* (2) Distinct sound paths -> true. */
TEST_F(NotificationSharedFileExtraTest,
       SharedFile_Validate_DistinctSoundPaths_ReturnsTrue) {
  notification_h n = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(n, nullptr);
  n->sound_path = strdup("/opt/usr/data/sound.ogg");
  n->priv_sound_path = strdup("/opt/usr/data/.notification/sound.ogg");
  EXPECT_TRUE(notification_validate_private_sharing(n));
  notification_free(n);
}

/* (3) Same sound path twice -> false (no actual private sharing). */
TEST_F(NotificationSharedFileExtraTest,
       SharedFile_Validate_SameSoundPath_ReturnsFalse) {
  notification_h n = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(n, nullptr);
  n->sound_path = strdup("/same/path");
  n->priv_sound_path = strdup("/same/path");
  EXPECT_FALSE(notification_validate_private_sharing(n));
  notification_free(n);
}

/* (4) Distinct vibration paths -> true. */
TEST_F(NotificationSharedFileExtraTest,
       SharedFile_Validate_DistinctVibrationPaths_ReturnsTrue) {
  notification_h n = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(n, nullptr);
  n->vibration_path = strdup("/opt/usr/data/vib.iv");
  n->priv_vibration_path = strdup("/opt/usr/data/.notification/vib.iv");
  EXPECT_TRUE(notification_validate_private_sharing(n));
  notification_free(n);
}

/* (5) Same vibration path twice -> false. */
TEST_F(NotificationSharedFileExtraTest,
       SharedFile_Validate_SameVibrationPath_ReturnsFalse) {
  notification_h n = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(n, nullptr);
  n->vibration_path = strdup("/same/vib");
  n->priv_vibration_path = strdup("/same/vib");
  EXPECT_FALSE(notification_validate_private_sharing(n));
  notification_free(n);
}

/* (6) Distinct image paths -> true. We have to populate b_image_path /
 *     b_priv_image_path bundles with at least one matching key for the
 *     image branch to be exercised. */
TEST_F(NotificationSharedFileExtraTest,
       SharedFile_Validate_DistinctImagePaths_ReturnsTrue) {
  notification_h n = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(n, nullptr);

  n->b_image_path = bundle_create();
  n->b_priv_image_path = bundle_create();
  ASSERT_NE(n->b_image_path, nullptr);
  ASSERT_NE(n->b_priv_image_path, nullptr);

  /* Use NOTIFICATION_IMAGE_TYPE_ICON (== 1) as the bundle key. */
  bundle_add_str(n->b_image_path, "1", "/opt/usr/data/icon.png");
  bundle_add_str(n->b_priv_image_path, "1", "/opt/usr/data/.notification/icon.png");

  EXPECT_TRUE(notification_validate_private_sharing(n));
  notification_free(n);
}

/* (7) Same image path in both bundles -> false. */
TEST_F(NotificationSharedFileExtraTest,
       SharedFile_Validate_SameImagePath_ReturnsFalse) {
  notification_h n = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(n, nullptr);

  n->b_image_path = bundle_create();
  n->b_priv_image_path = bundle_create();
  ASSERT_NE(n->b_image_path, nullptr);
  ASSERT_NE(n->b_priv_image_path, nullptr);

  bundle_add_str(n->b_image_path, "1", "/same/icon.png");
  bundle_add_str(n->b_priv_image_path, "1", "/same/icon.png");

  EXPECT_FALSE(notification_validate_private_sharing(n));
  notification_free(n);
}

/* (8) Only sound paths set, no image / vibration. The image and vibration
 *     branches stay untouched and the function still returns true (the
 *     distinct-sound branch alone does not flag as a sharing conflict). */
TEST_F(NotificationSharedFileExtraTest,
       SharedFile_Validate_OnlySound_NoImage_NoVib_ReturnsTrue) {
  notification_h n = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(n, nullptr);
  n->sound_path = strdup("/sound.ogg");
  n->priv_sound_path = strdup("/data/sound.ogg");
  EXPECT_TRUE(notification_validate_private_sharing(n));
  notification_free(n);
}

/* (9) Image bundles have non-overlapping keys: the loop probes the same
 *     buf_key against both, the inner bundle_get_str on the priv side
 *     fails (private_path stays NULL) so the loop continues without
 *     comparing. The function returns true. */
TEST_F(NotificationSharedFileExtraTest,
       SharedFile_Validate_PartialImageBundle_NoMatchingKey_ReturnsTrue) {
  notification_h n = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(n, nullptr);
  n->b_image_path = bundle_create();
  n->b_priv_image_path = bundle_create();
  ASSERT_NE(n->b_image_path, nullptr);
  ASSERT_NE(n->b_priv_image_path, nullptr);
  /* Only the public bundle has key "1"; the priv bundle is empty. */
  bundle_add_str(n->b_image_path, "1", "/icon.png");
  EXPECT_TRUE(notification_validate_private_sharing(n));
  notification_free(n);
}
