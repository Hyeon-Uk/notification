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
#include <notification.h>
#include <notification_internal.h>
#include <notification_list.h>

#include <set>
#include <thread>
#include <vector>

#include "common/smoke_utils.h"

namespace smoke {

class ErrorSmokeTest : public ::testing::Test {
 public:
  void SetUp() override { ResetNotifications(); }
  void TearDown() override { ResetNotifications(); }
};

/* TC-SMK-ER-01 */
TEST_F(ErrorSmokeTest, PostNullHandleRejected) {
  EXPECT_EQ(notification_post(nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* TC-SMK-ER-02 */
TEST_F(ErrorSmokeTest, DeleteNullHandleRejected) {
  EXPECT_EQ(notification_delete(nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* TC-SMK-ER-03 */
TEST_F(ErrorSmokeTest, CreateInvalidTypeReturnsNullOrFails) {
  /* The library may return NULL on create or accept then reject on
   * post. Either is acceptable as long as we never crash. */
  notification_h n = notification_create(
      static_cast<notification_type_e>(-999));
  if (n != nullptr) {
    EXPECT_NE(notification_post(n), NOTIFICATION_ERROR_NONE);
    notification_free(n);
  }
}

/* TC-SMK-ER-04 */
TEST_F(ErrorSmokeTest, GetListWhenEmpty) {
  notification_list_h list = nullptr;
  int ret = notification_get_list(NOTIFICATION_TYPE_NOTI, -1, &list);
  /* Empty list path: NONE + list==NULL is the contract. */
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(list, nullptr);
}

/* TC-SMK-ER-05 — deleting an id that was never posted must not crash
 * and must return a well-formed return code (idempotent semantics:
 * either NONE if the delete is a no-op, or NOT_EXIST_ID). The
 * smoke check is "no process death + reasonable return code". */
TEST_F(ErrorSmokeTest, DeleteNonexistentPrivIdIsBenign) {
  int ret = notification_delete_by_priv_id(kSmokeAppId,
      NOTIFICATION_TYPE_NOTI, 9999999);
  EXPECT_TRUE(ret == NOTIFICATION_ERROR_NONE ||
              ret == NOTIFICATION_ERROR_NOT_EXIST_ID)
      << "unexpected ret=" << ret;
}

class StressSmokeTest : public ::testing::Test {
 public:
  void SetUp() override { ResetNotifications(); }
  void TearDown() override { ResetNotifications(); }
};

/* TC-SMK-ST-01 — rapid post + delete cycle */
TEST_F(StressSmokeTest, RapidPostDeleteCycle) {
  for (int i = 0; i < 10; i++) {
    notification_h n = MakeNoti(NOTIFICATION_TYPE_NOTI, "x", "y");
    ASSERT_EQ(notification_post(n), NOTIFICATION_ERROR_NONE);
    ASSERT_EQ(notification_delete(n), NOTIFICATION_ERROR_NONE);
    notification_free(n);
  }
  EXPECT_EQ(CountByType(NOTIFICATION_TYPE_NOTI), 0);
}

/* TC-SMK-ST-02 — burst post: all priv_ids unique */
TEST_F(StressSmokeTest, BurstPostUniqueIds) {
  constexpr int kBurst = 25;
  std::set<int> ids;
  std::vector<notification_h> handles;
  for (int i = 0; i < kBurst; i++) {
    notification_h n = MakeNoti(NOTIFICATION_TYPE_NOTI, "burst", "i");
    ASSERT_EQ(notification_post(n), NOTIFICATION_ERROR_NONE);
    int priv = 0;
    notification_get_id(n, nullptr, &priv);
    ASSERT_GT(priv, 0);
    ids.insert(priv);
    handles.push_back(n);
  }
  EXPECT_EQ(static_cast<int>(ids.size()), kBurst);
  EXPECT_EQ(CountByType(NOTIFICATION_TYPE_NOTI), kBurst);
  for (auto* h : handles) notification_free(h);
}

/* TC-SMK-ST-03 — multiple handles alive simultaneously */
TEST_F(StressSmokeTest, MultipleHandlesParallel) {
  std::vector<notification_h> handles;
  for (int i = 0; i < 5; i++)
    handles.push_back(MakeNoti(NOTIFICATION_TYPE_NOTI, "p", "h"));
  for (auto* h : handles)
    EXPECT_EQ(notification_post(h), NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(CountByType(NOTIFICATION_TYPE_NOTI), 5);
  for (auto* h : handles) notification_free(h);
}

/* TC-SMK-ST-04 — cross-thread post */
TEST_F(StressSmokeTest, PostFromBackgroundThread) {
  int ret = NOTIFICATION_ERROR_NONE;
  std::thread t([&] {
    notification_h n = MakeNoti(NOTIFICATION_TYPE_NOTI, "th", "x");
    ret = notification_post(n);
    notification_free(n);
  });
  t.join();
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(CountByType(NOTIFICATION_TYPE_NOTI), 1);
}

/* TC-SMK-ST-05 — count_by_type matches enumerated */
TEST_F(StressSmokeTest, CountByTypeMatchesEnumeration) {
  for (int i = 0; i < 4; i++) {
    notification_h n = MakeNoti(NOTIFICATION_TYPE_NOTI, "n", "i");
    notification_post(n);
    notification_free(n);
  }
  for (int i = 0; i < 2; i++) {
    notification_h n = MakeNoti(NOTIFICATION_TYPE_ONGOING, "o", "i");
    notification_post(n);
    notification_free(n);
  }

  int noti_count = 0, ongoing_count = 0;
  notification_get_count(NOTIFICATION_TYPE_NOTI, nullptr, -1,
      NOTIFICATION_GROUP_ID_NONE, &noti_count);
  notification_get_count(NOTIFICATION_TYPE_ONGOING, nullptr, -1,
      NOTIFICATION_GROUP_ID_NONE, &ongoing_count);

  /* These counts include other apps' notifications too — assert lower
   * bounds, not exact equality. */
  EXPECT_GE(noti_count, 4);
  EXPECT_GE(ongoing_count, 2);
  EXPECT_EQ(CountByType(NOTIFICATION_TYPE_NOTI), 4);
  EXPECT_EQ(CountByType(NOTIFICATION_TYPE_ONGOING), 2);
}

}  // namespace smoke
