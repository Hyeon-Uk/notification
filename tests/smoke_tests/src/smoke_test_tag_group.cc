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

#include <string>

#include "common/smoke_utils.h"

namespace smoke {

class TagGroupSmokeTest : public ::testing::Test {
 public:
  void SetUp() override { ResetNotifications(); }
  void TearDown() override { ResetNotifications(); }
};

/* TC-SMK-TG-01 — tag round-trip on local handle */
TEST_F(TagGroupSmokeTest, TagRoundTrip) {
  notification_h n = MakeNoti(NOTIFICATION_TYPE_NOTI, "t", "c");
  EXPECT_EQ(notification_set_tag(n, "smoke_tag_001"),
            NOTIFICATION_ERROR_NONE);

  const char* got = nullptr;
  EXPECT_EQ(notification_get_tag(n, &got), NOTIFICATION_ERROR_NONE);
  ASSERT_NE(got, nullptr);
  EXPECT_STREQ(got, "smoke_tag_001");
  notification_free(n);
}

/* TC-SMK-TG-02 — post a tagged noti, then load_by_tag from the daemon */
TEST_F(TagGroupSmokeTest, PostThenLoadByTag) {
  const char* tag = "smoke_loadable_tag";
  notification_h n = MakeNoti(NOTIFICATION_TYPE_NOTI, "Tagged", "body");
  notification_set_tag(n, tag);
  ASSERT_EQ(notification_post(n), NOTIFICATION_ERROR_NONE);

  notification_h loaded = notification_load_by_tag(tag);
  EXPECT_NE(loaded, nullptr);
  if (loaded != nullptr) notification_free(loaded);
  notification_free(n);
}

/* TC-SMK-TG-03 — load_by_tag for an unknown tag returns NULL */
TEST_F(TagGroupSmokeTest, LoadByUnknownTagReturnsNull) {
  notification_h loaded = notification_load_by_tag("no_such_tag_smoke");
  EXPECT_EQ(loaded, nullptr);
}

/* TC-SMK-TG-04 — priv_id is assigned by the daemon on post.
 * Pre-post the field is 0 (uninitialized in this build's struct
 * convention rather than the documented NOTIFICATION_PRIV_ID_NONE
 * sentinel). Post-post it must be a positive id. */
TEST_F(TagGroupSmokeTest, PrivIdAssignedOnPost) {
  notification_h n = MakeNoti(NOTIFICATION_TYPE_NOTI, "g", "c");
  ASSERT_EQ(notification_post(n), NOTIFICATION_ERROR_NONE);
  int priv_after = 0;
  notification_get_id(n, nullptr, &priv_after);
  EXPECT_GT(priv_after, 0);
  notification_free(n);
}

/* TC-SMK-TG-05 — update only the title preserves other fields */
TEST_F(TagGroupSmokeTest, UpdateTitleOnly) {
  notification_h n = MakeNoti(NOTIFICATION_TYPE_NOTI, "Old", "stable");
  ASSERT_EQ(notification_post(n), NOTIFICATION_ERROR_NONE);

  notification_set_text(n, NOTIFICATION_TEXT_TYPE_TITLE,
      "New", nullptr, NOTIFICATION_VARIABLE_TYPE_NONE);
  EXPECT_EQ(notification_update(n), NOTIFICATION_ERROR_NONE);

  notification_list_h list = nullptr;
  notification_get_list(NOTIFICATION_TYPE_NOTI, -1, &list);
  ASSERT_NE(list, nullptr);
  notification_h fetched = notification_list_get_data(
      notification_list_get_head(list));
  char* title = nullptr;
  char* content = nullptr;
  notification_get_text(fetched, NOTIFICATION_TEXT_TYPE_TITLE, &title);
  notification_get_text(fetched, NOTIFICATION_TEXT_TYPE_CONTENT, &content);
  EXPECT_STREQ(title, "New");
  EXPECT_STREQ(content, "stable");

  notification_free_list(list);
  notification_free(n);
}

/* TC-SMK-TG-06 — clear changes by re-posting wipes the noti */
TEST_F(TagGroupSmokeTest, PostThenRepostCreatesSecond) {
  /* Two separate posts create two separate notifications even with
   * the same content (no tag dedup). */
  notification_h a = MakeNoti(NOTIFICATION_TYPE_NOTI, "X", "Y");
  notification_h b = MakeNoti(NOTIFICATION_TYPE_NOTI, "X", "Y");
  EXPECT_EQ(notification_post(a), NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(notification_post(b), NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(CountByType(NOTIFICATION_TYPE_NOTI), 2);
  notification_free(a);
  notification_free(b);
}

/* TC-SMK-TG-07 — tagged dedup: re-post with same tag updates instead of
 * creating a second row */
TEST_F(TagGroupSmokeTest, TaggedDedupOnRepost) {
  const char* tag = "smoke_dedup";
  notification_h a = MakeNoti(NOTIFICATION_TYPE_NOTI, "v1", "x");
  notification_set_tag(a, tag);
  notification_h b = MakeNoti(NOTIFICATION_TYPE_NOTI, "v2", "y");
  notification_set_tag(b, tag);

  ASSERT_EQ(notification_post(a), NOTIFICATION_ERROR_NONE);
  ASSERT_EQ(notification_post(b), NOTIFICATION_ERROR_NONE);
  /* Daemon's notification_noti_check_tag dedups on (tag, app_id) so
   * the second post should update the first row, not insert. */
  EXPECT_EQ(CountByType(NOTIFICATION_TYPE_NOTI), 1);

  notification_free(a);
  notification_free(b);
}

}  // namespace smoke
