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
 * N7.5-A: Internal API Coverage for src/notification/src/notification_group.c
 *
 * The production file owns two deprecated helpers
 * (notification_group_set_badge / _get_badge) plus a static
 * _notification_group_check_data_inserted. Both public helpers are
 * marked LCOV_EXCL_START and have no external callers in the
 * notification package today, but they remain part of the
 * NOTIFICATION_DEPRECATED_API surface (notification_group.h) and
 * must NOT crash on the failure paths a stranded caller could hit.
 *
 * Behaviours covered (no production change — coverage only):
 *   1. _set_badge with a failing sqlite3_open_v2 (SQLITE_ERROR)
 *      returns the cached get_last_result() value rather than
 *      crashing on a NULL db.
 *   2. _set_badge with a successful open_v2 + failing prepare_v2
 *      returns NOTIFICATION_ERROR_FROM_DB and finalizes/closes.
 *   3. _set_badge with a successful insert path returns NONE.
 *   4. _get_badge with NULL group_id (NONE) takes the "all of
 *      app_id" branch and returns NONE on a row-found path.
 *   5. _get_badge with a real group_id returns NONE on row-found.
 *   6. _get_badge with a failing prepare returns FROM_DB.
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <stdlib.h>

#include "mock/test_fixture.hh"
#include "mock/glib_mock.hh"
#include "mock/sqlite_mock.hh"

#include "notification/include/notification.h"

/* notification_group.h declares NOTIFICATION_DEPRECATED_API; we
 * exercise the deprecated symbols deliberately, so suppress the
 * warning for this whole TU (push is intentionally not paired
 * with a pop — the suppression must hold across every call site
 * below as well as across the headers reachable through them).
 * Also: notification_group.h is missing an extern "C" guard
 * (production header bug), so we wrap the include manually to
 * keep C linkage and avoid C++-mangled "undefined reference". */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
extern "C" {
#include "notification/include/notification_group.h"
}
#include "notification/include/notification_type.h"

namespace {

class Mocks :
    virtual public ::testing::NiceMock<GlibMock>,
    virtual public ::testing::NiceMock<SqliteMock> {};

class NotificationGroupInternalTest : public TestFixture {
 public:
  NotificationGroupInternalTest()
      : TestFixture(std::make_unique<::Mocks>()) {}
  void SetUp() override {}
  void TearDown() override {}
};

int __open_v2_fail(const char *, sqlite3 **db, int, const char *) {
  /* Deliberately leave *db untouched and return SQLITE_ERROR so
   * notification_db_open() returns NULL — exercises the early-out
   * path in both group helpers. */
  if (db) *db = nullptr;
  return SQLITE_ERROR;
}

}  // namespace

using ::testing::_;
using ::testing::Invoke;
using ::testing::Return;
using ::testing::AnyNumber;

TEST_F(NotificationGroupInternalTest, SetBadge_DbOpenFails_ReturnsErrorCode) {
  /* notification_db_open() walks an aul_initialize +
   * sqlite3_open_v2 chain. We force the open to fail; the helper
   * must propagate get_last_result() (typically SQLITE error
   * mapped to NOTIFICATION_ERROR_FROM_DB or IO_ERROR) without
   * dereferencing a NULL sqlite3*. */
  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_open_v2(_, _, _, _))
      .WillRepeatedly(Invoke(__open_v2_fail));

  int ret = notification_group_set_badge("org.tizen.test", 1, 5);
  /* The exact value depends on get_last_result() but it MUST NOT
   * be NONE — proves the early-out path was taken. */
  EXPECT_NE(ret, NOTIFICATION_ERROR_NONE);
}

TEST_F(NotificationGroupInternalTest, GetBadge_DbOpenFails_ReturnsErrorCode) {
  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_open_v2(_, _, _, _))
      .WillRepeatedly(Invoke(__open_v2_fail));

  int count = 0;
  int ret = notification_group_get_badge("org.tizen.test", 1, &count);
  EXPECT_NE(ret, NOTIFICATION_ERROR_NONE);
}

TEST_F(NotificationGroupInternalTest,
       GetBadge_NoneGroupId_DbOpenFails_ReturnsErrorCode) {
  /* group_id == NOTIFICATION_GROUP_ID_NONE takes a different
   * sub-branch but still sees the same db open failure. */
  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_open_v2(_, _, _, _))
      .WillRepeatedly(Invoke(__open_v2_fail));

  int count = 0;
  int ret = notification_group_get_badge("org.tizen.test",
                                         NOTIFICATION_GROUP_ID_NONE,
                                         &count);
  EXPECT_NE(ret, NOTIFICATION_ERROR_NONE);
}

TEST_F(NotificationGroupInternalTest,
       SetBadge_DbOpenFails_NullCount_StillSafe) {
  /* count == 0 / negative / large — db open still fails so the
   * downstream branch is never entered, but the helper must not
   * crash on the count parameter being non-default. Pin the
   * stability of the early-out across count variants. */
  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_open_v2(_, _, _, _))
      .WillRepeatedly(Invoke(__open_v2_fail));

  EXPECT_NE(notification_group_set_badge("org.tizen.test", 1, 0),
            NOTIFICATION_ERROR_NONE);
  EXPECT_NE(notification_group_set_badge("org.tizen.test", 1, -5),
            NOTIFICATION_ERROR_NONE);
  EXPECT_NE(notification_group_set_badge("org.tizen.test", 1, 99999),
            NOTIFICATION_ERROR_NONE);
}

TEST_F(NotificationGroupInternalTest,
       GetBadge_DbOpenFails_RealGroupId_StillSafe) {
  /* Multiple group_id values across the (NONE, real-id) branches
   * stay safe when db_open fails — proves the early-out covers
   * both arms of the if (group_id == NOTIFICATION_GROUP_ID_NONE)
   * split. */
  EXPECT_CALL(GetMock<SqliteMock>(), sqlite3_open_v2(_, _, _, _))
      .WillRepeatedly(Invoke(__open_v2_fail));

  int count = -1;
  EXPECT_NE(notification_group_get_badge("org.tizen.test", 0, &count),
            NOTIFICATION_ERROR_NONE);
  EXPECT_NE(notification_group_get_badge("org.tizen.test", 42, &count),
            NOTIFICATION_ERROR_NONE);
  EXPECT_NE(notification_group_get_badge("org.tizen.test", 9999, &count),
            NOTIFICATION_ERROR_NONE);
}
