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

#include "common/smoke_utils.h"

namespace smoke {

constexpr const char* kCh = "smoke_channel";
constexpr const char* kCh2 = "smoke_channel_secondary";

class ChannelSmokeTest : public ::testing::Test {
 public:
  void SetUp() override {
    ResetNotifications();
    /* Drop any leftover channels from prior runs. */
    for (auto name : {kCh, kCh2}) {
      notification_channel_h ch = nullptr;
      if (notification_channel_get_by_name(name, &ch) ==
          NOTIFICATION_ERROR_NONE && ch != nullptr) {
        notification_channel_remove(ch);
        notification_channel_free(ch);
      }
    }
  }
  void TearDown() override { SetUp(); }
};

/* TC-SMK-CH-01 — create, add, and look up succeed */
TEST_F(ChannelSmokeTest, CreateAddGet) {
  notification_channel_h ch = nullptr;
  ASSERT_EQ(notification_channel_create(kCh, &ch), NOTIFICATION_ERROR_NONE);
  ASSERT_NE(ch, nullptr);
  EXPECT_EQ(notification_channel_add(ch), NOTIFICATION_ERROR_NONE);

  notification_channel_h fetched = nullptr;
  ASSERT_EQ(notification_channel_get_by_name(kCh, &fetched),
            NOTIFICATION_ERROR_NONE);
  EXPECT_NE(fetched, nullptr);

  notification_channel_free(ch);
  notification_channel_free(fetched);
}

/* TC-SMK-CH-02 — remove makes a subsequent lookup fail */
TEST_F(ChannelSmokeTest, RemoveMakesGetFail) {
  notification_channel_h ch = nullptr;
  notification_channel_create(kCh, &ch);
  notification_channel_add(ch);

  EXPECT_EQ(notification_channel_remove(ch), NOTIFICATION_ERROR_NONE);
  notification_channel_free(ch);

  notification_channel_h fetched = nullptr;
  int ret = notification_channel_get_by_name(kCh, &fetched);
  EXPECT_NE(ret, NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(fetched, nullptr);
}

/* TC-SMK-CH-03 — setting blockable + updating does not error */
TEST_F(ChannelSmokeTest, BlockableUpdateSucceeds) {
  notification_channel_h ch = nullptr;
  notification_channel_create(kCh, &ch);
  ASSERT_EQ(notification_channel_add(ch), NOTIFICATION_ERROR_NONE);

  EXPECT_EQ(notification_channel_set_blockable(ch, true),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(notification_channel_update(ch), NOTIFICATION_ERROR_NONE);

  /* The channel still exists after update. */
  notification_channel_h fetched = nullptr;
  EXPECT_EQ(notification_channel_get_by_name(kCh, &fetched),
            NOTIFICATION_ERROR_NONE);
  EXPECT_NE(fetched, nullptr);

  notification_channel_free(ch);
  notification_channel_free(fetched);
}

/* TC-SMK-CH-04 — two distinct channels coexist */
TEST_F(ChannelSmokeTest, TwoChannelsCoexist) {
  notification_channel_h a = nullptr, b = nullptr;
  notification_channel_create(kCh, &a);
  notification_channel_create(kCh2, &b);
  EXPECT_EQ(notification_channel_add(a), NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(notification_channel_add(b), NOTIFICATION_ERROR_NONE);

  notification_channel_h fa = nullptr, fb = nullptr;
  EXPECT_EQ(notification_channel_get_by_name(kCh, &fa),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(notification_channel_get_by_name(kCh2, &fb),
            NOTIFICATION_ERROR_NONE);
  ASSERT_NE(fa, nullptr);
  ASSERT_NE(fb, nullptr);

  notification_channel_free(a);
  notification_channel_free(b);
  notification_channel_free(fa);
  notification_channel_free(fb);
}

/* TC-SMK-CH-05 — create + free without add is safe (no daemon hit) */
TEST_F(ChannelSmokeTest, CreateAndFreeWithoutAdd) {
  notification_channel_h ch = nullptr;
  ASSERT_EQ(notification_channel_create(kCh, &ch), NOTIFICATION_ERROR_NONE);
  ASSERT_NE(ch, nullptr);
  notification_channel_free(ch);
}

}  // namespace smoke
