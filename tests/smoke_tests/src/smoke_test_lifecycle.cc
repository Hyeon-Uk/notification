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

#include "common/smoke_utils.h"

namespace smoke {

class LifecycleSmokeTest : public ::testing::Test {
 public:
  void SetUp() override { ResetNotifications(); }
  void TearDown() override { ResetNotifications(); }
};

/* TC-SMK-LC-01 */
TEST_F(LifecycleSmokeTest, PostMinimal) {
  notification_h n = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(n, nullptr);
  notification_set_app_id(n, kSmokeAppId);
  EXPECT_EQ(notification_post(n), NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(CountByType(NOTIFICATION_TYPE_NOTI), 1);
  notification_free(n);
}

/* TC-SMK-LC-02 */
TEST_F(LifecycleSmokeTest, PostWithTitleContent) {
  notification_h n = MakeNoti(NOTIFICATION_TYPE_NOTI,
                              "Smoke Title", "Smoke Content");
  ASSERT_NE(n, nullptr);
  EXPECT_EQ(notification_post(n), NOTIFICATION_ERROR_NONE);

  /* Read it back through the daemon and verify the text round-tripped. */
  notification_list_h list = nullptr;
  ASSERT_EQ(notification_get_list(NOTIFICATION_TYPE_NOTI, -1, &list),
            NOTIFICATION_ERROR_NONE);
  ASSERT_NE(list, nullptr);
  notification_h fetched = notification_list_get_data(
      notification_list_get_head(list));
  ASSERT_NE(fetched, nullptr);
  char* title = nullptr;
  notification_get_text(fetched, NOTIFICATION_TEXT_TYPE_TITLE, &title);
  ASSERT_NE(title, nullptr);
  EXPECT_STREQ(title, "Smoke Title");

  notification_free_list(list);
  notification_free(n);
}

/* TC-SMK-LC-03 */
TEST_F(LifecycleSmokeTest, PostOngoingType) {
  notification_h n = MakeNoti(NOTIFICATION_TYPE_ONGOING,
                              "Downloading", "10%");
  ASSERT_NE(n, nullptr);
  EXPECT_EQ(notification_post(n), NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(CountByType(NOTIFICATION_TYPE_ONGOING), 1);
  EXPECT_EQ(CountByType(NOTIFICATION_TYPE_NOTI), 0);
  notification_free(n);
}

/* TC-SMK-LC-04 */
TEST_F(LifecycleSmokeTest, PostBothTypesCoexist) {
  notification_h a = MakeNoti(NOTIFICATION_TYPE_NOTI, "A", "alpha");
  notification_h b = MakeNoti(NOTIFICATION_TYPE_ONGOING, "B", "beta");
  EXPECT_EQ(notification_post(a), NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(notification_post(b), NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(CountByType(NOTIFICATION_TYPE_NOTI), 1);
  EXPECT_EQ(CountByType(NOTIFICATION_TYPE_ONGOING), 1);
  notification_free(a);
  notification_free(b);
}

/* TC-SMK-LC-05 */
TEST_F(LifecycleSmokeTest, PostThenDelete) {
  notification_h n = MakeNoti(NOTIFICATION_TYPE_NOTI, "Bye", "later");
  EXPECT_EQ(notification_post(n), NOTIFICATION_ERROR_NONE);
  ASSERT_EQ(CountByType(NOTIFICATION_TYPE_NOTI), 1);
  EXPECT_EQ(notification_delete(n), NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(CountByType(NOTIFICATION_TYPE_NOTI), 0);
  notification_free(n);
}

/* TC-SMK-LC-06 */
TEST_F(LifecycleSmokeTest, DeleteAllByType) {
  for (int i = 0; i < 3; i++) {
    notification_h n = MakeNoti(NOTIFICATION_TYPE_NOTI, "x", "y");
    notification_post(n);
    notification_free(n);
  }
  ASSERT_EQ(CountByType(NOTIFICATION_TYPE_NOTI), 3);
  EXPECT_EQ(notification_delete_all_by_type(kSmokeAppId,
            NOTIFICATION_TYPE_NOTI), NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(CountByType(NOTIFICATION_TYPE_NOTI), 0);
}

/* TC-SMK-LC-07 */
TEST_F(LifecycleSmokeTest, UpdateContent) {
  notification_h n = MakeNoti(NOTIFICATION_TYPE_NOTI, "before", "v1");
  EXPECT_EQ(notification_post(n), NOTIFICATION_ERROR_NONE);

  notification_set_text(n, NOTIFICATION_TEXT_TYPE_CONTENT,
      "v2", nullptr, NOTIFICATION_VARIABLE_TYPE_NONE);
  EXPECT_EQ(notification_update(n), NOTIFICATION_ERROR_NONE);

  notification_list_h list = nullptr;
  notification_get_list(NOTIFICATION_TYPE_NOTI, -1, &list);
  ASSERT_NE(list, nullptr);
  notification_h fetched = notification_list_get_data(
      notification_list_get_head(list));
  char* content = nullptr;
  notification_get_text(fetched, NOTIFICATION_TEXT_TYPE_CONTENT, &content);
  EXPECT_STREQ(content, "v2");

  notification_free_list(list);
  notification_free(n);
}

/* TC-SMK-LC-08 */
TEST_F(LifecycleSmokeTest, UpdateProgress) {
  notification_h n = MakeNoti(NOTIFICATION_TYPE_ONGOING, "DL", "0%");
  EXPECT_EQ(notification_post(n), NOTIFICATION_ERROR_NONE);

  int priv_id = 0;
  notification_get_id(n, nullptr, &priv_id);
  ASSERT_GT(priv_id, 0);

  for (double p : {0.25, 0.5, 0.75, 1.0}) {
    EXPECT_EQ(notification_update_progress(nullptr, priv_id, p),
              NOTIFICATION_ERROR_NONE);
  }
  notification_free(n);
}

}  // namespace smoke
