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
 * N8 NULL-safety pass 2: parallel audit to a5932a8 / c064fdc.
 *
 * Background:
 *   - a5932a8 fixed two NULL-deref crashes in notification_noti.cc:
 *       (1) `*count_out = 0` write before NULL guard on count out-arg
 *       (2) WARN("...[%s]", tag) BEFORE NULL guard on the handle / member.
 *   - c064fdc (dpm-restriction) found 4 additional bugs of the same shape
 *     across that lib's EXPORT_API surface in a parallel pass.
 *
 * This audit (notification side) covers the 14 EXPORT_API source files
 * NOT touched by a5932a8. The crash-class bugs found and fixed:
 *
 *   1. notification_shared_file.cc / notification_add_private_sharing_target_id
 *        WARN("...[%s]", sender) BEFORE NULL check on sender. (Suspect
 *        explicitly called out in the audit prompt.)
 *
 *   2. notification_shared_file.cc / notification_remove_private_sharing_target_id
 *        Same shape — WARN("...: %s", sender) BEFORE NULL check.
 *
 *   3. notification_shared_file.cc / notification_validate_private_sharing
 *        First statement derefs `updated_noti->b_image_path` with no
 *        NULL guard on the handle.
 *
 *   4. notification_shared_file.cc / notification_calibrate_private_sharing
 *        Same — derefs `updated_noti->b_image_path` and later
 *        `source_noti->sound_path` with no guard on either handle.
 *
 *   5. notification_group.cc / notification_group_set_badge
 *        snprintf(query, ..., "%s", app_id) with no NULL guard ->
 *        glibc UB / crash on x86_64.
 *
 *   6. notification_group.cc / notification_group_get_badge
 *        Same a5932a8 bug 1 shape: writes `*count = sqlite3_column_int(...)`
 *        with no NULL guard on count out-arg, AND no guard on app_id
 *        before snprintf.
 *
 *   7. notification_ipc.cc / notification_ipc_make_gvariant_from_noti
 *        Derefs `noti->type` immediately after the entry WARN, with no
 *        NULL guard on the handle. The mirror function
 *        notification_ipc_make_noti_from_gvariant() already had this
 *        guard; the encode side never grew one.
 *
 *   8. notification_setting_service.cc /
 *      notification_system_setting_get_dnd_schedule_enabled_uid
 *        Same a5932a8 bug 1 shape on both out-args (`*uids = ...; *count = ...`).
 *
 *   9. notification_setting_service.cc /
 *      notification_system_setting_load_dnd_allow_exception
 *        Original guard checked `dnd_allow_exception` but missed `count`,
 *        which is also written via `*count = row_count` on success.
 *
 * Contract aligned with the rest of the EXPORT_API surface:
 *   - int-returning APIs: NULL handle / NULL out-arg ->
 *       NOTIFICATION_ERROR_INVALID_PARAMETER, no DB / IPC reached.
 *   - GVariant *-returning API: NULL handle -> NULL.
 *   - void-returning APIs: silent no-op, no crash.
 *
 * The tests below are TDD-red: each EXPECT_EQ pins the post-fix contract
 * and was written to fail on the pre-fix code (where the call would have
 * either SIGSEGV'd or returned NOTIFICATION_ERROR_NONE while reading
 * uninitialised memory).
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <bundle.h>
#include <bundle_internal.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <gio/gio.h>
#include <unistd.h>

#include "mock/test_fixture.hh"
#include "mock/aul_mock.hh"
#include "mock/glib_mock.hh"
#include "mock/sqlite_mock.hh"
#include "mock/package_manager_mock.hh"
#include "mock/tzplatform_config_mock.hh"

extern "C" {
#include <notification.h>
#include <notification_internal.h>
#include <notification_ipc.h>
#include <notification_private.h>
#include <notification_setting_service.h>
#include <notification_shared_file.h>
#include <notification_type.h>
}

/* notification_group.h is missing an extern "C" guard (production header
 * issue, see comments in test_notification_group_internal.cc), so wrap
 * manually. NOTIFICATION_DEPRECATED_API would also flag the includes
 * below; suppress for this TU. */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
extern "C" {
#include <notification_group.h>
}

namespace {

using ::testing::_;
using ::testing::Invoke;
using ::testing::Return;

int FakeAulGetAppidBypid_NS2(int /*pid*/, char* appid, int len) {
  if (appid && len > 0) snprintf(appid, len, "%s", "org.tizen.testapp.ns2");
  return 0;
}

int FakeAulGetPkgidBypid_NS2(int /*pid*/, char* pkgid, int len) {
  if (pkgid && len > 0) snprintf(pkgid, len, "%s", "org.tizen.testpkg.ns2");
  return 0;
}

int FakeOpenV2_Fail(const char *, sqlite3 **db, int, const char *) {
  if (db) *db = nullptr;
  return SQLITE_ERROR;
}

class NullSafe2Mocks :
    virtual public ::testing::NiceMock<AulMock>,
    virtual public ::testing::NiceMock<GlibMock>,
    virtual public ::testing::NiceMock<SqliteMock>,
    virtual public ::testing::NiceMock<PackageManagerMock>,
    virtual public ::testing::NiceMock<TzplatformConfigMock> {};

class NotificationNullSafePass2Test : public TestFixture {
 public:
  NotificationNullSafePass2Test()
      : TestFixture(std::make_unique<::NullSafe2Mocks>()) {}

  void SetUp() override {
    /* aul fakes for tests that need notification_create() to succeed. */
    ON_CALL(GetMock<AulMock>(), aul_app_get_appid_bypid(_, _, _))
        .WillByDefault(Invoke(FakeAulGetAppidBypid_NS2));
    ON_CALL(GetMock<AulMock>(), aul_app_get_pkgid_bypid(_, _, _))
        .WillByDefault(Invoke(FakeAulGetPkgidBypid_NS2));
    ON_CALL(GetMock<PackageManagerMock>(),
            package_info_create(_, _))
        .WillByDefault(Return(-1));
    ON_CALL(GetMock<PackageManagerMock>(),
            pkgmgrinfo_appinfo_get_usr_appinfo(_, _, _))
        .WillByDefault(Return(-1));
  }
};

}  // namespace

/* ------------------------------------------------------------------ */
/* shared_file.cc                                                     */
/* ------------------------------------------------------------------ */

/* Bug 1: notification_add_private_sharing_target_id(NULL sender) */
TEST_F(NotificationNullSafePass2Test, AddPrivateSharingTargetId_NullSender_NoCrash) {
  /* Pre-fix: WARN("...[%s]", sender) walks NULL pointer for %s -> UB
   * (typically SIGSEGV in __strchrnul on x86_64 glibc).
   * Post-fix: silent no-op (function returns void). */
  notification_add_private_sharing_target_id((pid_t)1234, nullptr, getuid());
  /* Reaching this point without a SIGSEGV proves the guard fired. */
  SUCCEED();
}

/* Bug 2: notification_remove_private_sharing_target_id(NULL sender) */
TEST_F(NotificationNullSafePass2Test, RemovePrivateSharingTargetId_NullSender_NoCrash) {
  /* Pre-fix: WARN("...: %s", sender) walks NULL -> UB / SIGSEGV.
   * Post-fix: silent no-op. */
  notification_remove_private_sharing_target_id(nullptr, getuid());
  SUCCEED();
}

/* Bug 3: notification_validate_private_sharing(NULL noti) */
TEST_F(NotificationNullSafePass2Test, ValidatePrivateSharing_NullNoti_ReturnsTrue) {
  /* Pre-fix: derefs updated_noti->b_image_path -> SIGSEGV.
   * Post-fix: return true (matches "no images to compare -> valid"
   * documented in test_notification_shared_file_extra
   * SharedFile_Validate_BothNull_BundleSafe). */
  EXPECT_EQ(notification_validate_private_sharing(nullptr), true);
}

/* Bug 4: notification_calibrate_private_sharing(NULL, valid) and
 *        notification_calibrate_private_sharing(valid, NULL) */
TEST_F(NotificationNullSafePass2Test, CalibratePrivateSharing_NullUpdated_NoCrash) {
  notification_h n = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(n, nullptr);
  /* Pre-fix: derefs updated_noti->b_image_path on first stmt -> SIGSEGV.
   * Post-fix: silent no-op. */
  notification_calibrate_private_sharing(nullptr, n);
  notification_free(n);
  SUCCEED();
}

TEST_F(NotificationNullSafePass2Test, CalibratePrivateSharing_NullSource_NoCrash) {
  notification_h n = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(n, nullptr);
  /* Pre-fix: would deref source_noti->sound_path inside the second
   * branch on a noti with sound paths set; even with empty noti the
   * single-handle deref fired. Post-fix guards both. */
  notification_calibrate_private_sharing(n, nullptr);
  notification_free(n);
  SUCCEED();
}

/* ------------------------------------------------------------------ */
/* group.cc                                                           */
/* ------------------------------------------------------------------ */

/* Bug 5: notification_group_set_badge(NULL app_id) */
TEST_F(NotificationNullSafePass2Test, GroupSetBadge_NullAppId_ReturnsInvalidParam) {
  /* Pre-fix: snprintf(... "%s" ..., app_id, ...) is glibc UB / SIGSEGV
   *   on NULL %s (verified on x86_64).
   * Post-fix: rejects with INVALID_PARAMETER without opening DB. */
  EXPECT_EQ(notification_group_set_badge(nullptr, /*group_id=*/1, /*count=*/5),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* Bug 6a: notification_group_get_badge(NULL app_id) */
TEST_F(NotificationNullSafePass2Test, GroupGetBadge_NullAppId_ReturnsInvalidParam) {
  int count = -1;
  EXPECT_EQ(notification_group_get_badge(nullptr, /*group_id=*/1, &count),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* Bug 6b: notification_group_get_badge(NULL count) */
TEST_F(NotificationNullSafePass2Test, GroupGetBadge_NullCount_ReturnsInvalidParam) {
  /* Pre-fix: would walk through to `*count = sqlite3_column_int(...)` and
   *   crash. Same shape as a5932a8 bug 1. */
  EXPECT_EQ(notification_group_get_badge("org.tizen.test", /*group_id=*/1,
                                         /*count=*/nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* ------------------------------------------------------------------ */
/* ipc.cc                                                             */
/* ------------------------------------------------------------------ */

/* Bug 7: notification_ipc_make_gvariant_from_noti(NULL noti) */
TEST_F(NotificationNullSafePass2Test, IpcMakeGVariantFromNoti_NullNoti_ReturnsNull) {
  /* Pre-fix: WARN entry, then immediate g_variant_builder_add(... noti->type)
   *   -> SIGSEGV reading from NULL.
   * Post-fix: rejects with NULL return; mirrors the existing
   *   make_noti_from_gvariant guard. */
  EXPECT_EQ(notification_ipc_make_gvariant_from_noti(nullptr, /*translate=*/false),
            nullptr);
}

/* ------------------------------------------------------------------ */
/* setting_service.cc                                                 */
/* ------------------------------------------------------------------ */

/* Bug 8a: notification_system_setting_get_dnd_schedule_enabled_uid(NULL uids) */
TEST_F(NotificationNullSafePass2Test,
       SystemSettingGetDndScheduleEnabledUid_NullUids_ReturnsInvalidParam) {
  /* Pre-fix: success path writes `*uids = result_uids` with no guard.
   *   Even on the early-out (db open failure) the symbolic guard was
   *   missing, so any trace through the success branch crashed. */
  int count = 0;
  EXPECT_EQ(notification_system_setting_get_dnd_schedule_enabled_uid(
                /*uids=*/nullptr, &count),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* Bug 8b: notification_system_setting_get_dnd_schedule_enabled_uid(NULL count) */
TEST_F(NotificationNullSafePass2Test,
       SystemSettingGetDndScheduleEnabledUid_NullCount_ReturnsInvalidParam) {
  uid_t *uids = nullptr;
  EXPECT_EQ(notification_system_setting_get_dnd_schedule_enabled_uid(
                &uids, /*count=*/nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* Bug 9: notification_system_setting_load_dnd_allow_exception(NULL count) */
TEST_F(NotificationNullSafePass2Test,
       SystemSettingLoadDndAllowException_NullCount_ReturnsInvalidParam) {
  /* Pre-fix: original guard only covered `dnd_allow_exception`; the
   * success path also writes `*count = row_count`. Mirror the
   * a5932a8 bug 1 fix on the second out-arg. */
  dnd_allow_exception_h dnd = nullptr;
  EXPECT_EQ(notification_system_setting_load_dnd_allow_exception(
                &dnd, /*count=*/nullptr, getuid()),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

#pragma GCC diagnostic pop
