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
 * N7.5-D follow-up: production-bug NULL-safety regression tests.
 *
 * test_notification_noti_internal.cc explicitly documented two known crash
 * sites that it could NOT pin without first fixing production code:
 *
 *   1. notification_noti_get_count(..., int *count, ...) — dereferences
 *      `*count = ...` after a vconf+sqlite walk. Passing NULL crashes.
 *
 *   2. notification_noti_check_tag(notification_h noti) — first statement
 *      is `WARN("@@ check tag : %s", noti->tag);`, which reads `noti->tag`
 *      BEFORE any NULL guard. Passing a NULL handle crashes (and any code
 *      path that swapped the WARN-after-guard ordering would still UB on
 *      `noti == NULL` because the explicit `if (noti->tag == NULL)` check
 *      itself derefs `noti`).
 *
 * Production fix (same commit):
 *   - get_count: add `if (count == NULL) return INVALID_PARAMETER;` as the
 *     first line of the function body.
 *   - check_tag: add `if (noti == NULL) return INVALID_PARAMETER;` and move
 *     the `WARN("@@ check tag : %s", noti->tag)` to AFTER the
 *     `noti->tag == NULL` guard so the format-string walker never sees a
 *     stale pointer.
 *
 * Contract aligned with the rest of the EXPORT_API surface: NULL out-args
 * and NULL handles -> NOTIFICATION_ERROR_INVALID_PARAMETER, no crash, no
 * DB I/O performed.
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <cstring>
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
#include <notification_noti.h>
#include <notification_error.h>
#include <notification_type.h>
}

namespace {

using ::testing::_;
using ::testing::Invoke;
using ::testing::Return;

int FakeAulGetAppidBypid_NS(int /*pid*/, char* appid, int len) {
  if (appid && len > 0) snprintf(appid, len, "%s", "org.tizen.testapp");
  return 0;  /* AUL_R_OK */
}

int FakeAulGetPkgidBypid_NS(int /*pid*/, char* pkgid, int len) {
  if (pkgid && len > 0) snprintf(pkgid, len, "%s", "org.tizen.testpkg");
  return 0;
}

class NotiNullSafeMocks :
    virtual public ::testing::NiceMock<AulMock>,
    virtual public ::testing::NiceMock<GlibMock>,
    virtual public ::testing::NiceMock<SqliteMock>,
    virtual public ::testing::NiceMock<PackageManagerMock>,
    virtual public ::testing::NiceMock<TzplatformConfigMock> {};

class NotificationNotiNullSafeTest : public TestFixture {
 public:
  NotificationNotiNullSafeTest()
      : TestFixture(std::make_unique<::NotiNullSafeMocks>()) {}

  void SetUp() override {
    /* Provide aul fakes so notification_create() succeeds for tests that
     * need a real handle (CheckTag with non-NULL handle, NULL tag path). */
    ON_CALL(GetMock<AulMock>(), aul_app_get_appid_bypid(_, _, _))
        .WillByDefault(Invoke(FakeAulGetAppidBypid_NS));
    ON_CALL(GetMock<AulMock>(), aul_app_get_pkgid_bypid(_, _, _))
        .WillByDefault(Invoke(FakeAulGetPkgidBypid_NS));
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
/* 1. notification_noti_get_count(NULL count) — NULL out-arg.         */
/* ------------------------------------------------------------------ */

TEST_F(NotificationNotiNullSafeTest, GetCount_NullCount_ReturnsInvalidParam) {
  /* Pre-fix behavior: function ran vconf_get_int + sqlite open + a few
   *   sqlite3_mprintf allocations, then crashed at `*count = get_count`.
   * Post-fix behavior: rejects with INVALID_PARAMETER as the very first
   *   statement; no DB / vconf calls reach the mock.
   *
   * We deliberately do NOT install any sqlite mocks here — if the early
   * NULL guard regresses, the call would attempt sqlite3_open and either
   * crash on NULL handle in the unmocked path or trigger a NiceMock
   * "uninteresting call" warning (still observable in logs). The
   * EXPECT_EQ pins the contract. */
  int *no_out = nullptr;
  EXPECT_EQ(notification_noti_get_count(NOTIFICATION_TYPE_NOTI,
                                        "app", -1, -1,
                                        no_out, 5001),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationNotiNullSafeTest,
       GetCount_NullCount_NoneType_ReturnsInvalidParam) {
  /* Same contract on NOTIFICATION_TYPE_NONE — type bound was the first
   * branch the old code took before reaching the deref. Pin it too. */
  int *no_out = nullptr;
  EXPECT_EQ(notification_noti_get_count(NOTIFICATION_TYPE_NONE,
                                        nullptr, -1, -1,
                                        no_out, 5001),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* ------------------------------------------------------------------ */
/* 2. notification_noti_check_tag(NULL noti) — NULL handle.           */
/* ------------------------------------------------------------------ */

TEST_F(NotificationNotiNullSafeTest, CheckTag_NullNoti_NoCrash) {
  /* Pre-fix behavior: dereferenced `noti->tag` inside WARN() before any
   *   guard -> SIGSEGV (when noti == NULL) or scribble of arbitrary
   *   pointer (UB).
   * Post-fix behavior: returns INVALID_PARAMETER. */
  EXPECT_EQ(notification_noti_check_tag(nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationNotiNullSafeTest, CheckTag_ValidNoti_NullTag_NotExistId) {
  /* Pin the migrated WARN ordering: with a real handle whose `tag` field
   * is NULL (default after notification_create), production must hit the
   * `noti->tag == NULL` branch and return NOT_EXIST_ID — not crash on the
   * format-string walker reading a NULL pointer. */
  notification_h n = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(n, nullptr);
  EXPECT_EQ(notification_noti_check_tag(n),
            NOTIFICATION_ERROR_NOT_EXIST_ID);
  notification_free(n);
}
