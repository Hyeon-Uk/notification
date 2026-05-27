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

#include <glib.h>
#include <gtest/gtest.h>
#include <notification.h>
#include <notification_internal.h>

static constexpr const char* TEST_CHANNEL_NAME = "integ_test_channel";
static constexpr const char* TEST_APP_ID = "org.tizen.notification.integtest";

class NotificationChannelIntegTest : public ::testing::Test {
 public:
  void SetUp() override {
    /* Remove any leftover channel from a previous run */
    notification_channel_h ch = nullptr;
    if (notification_channel_get_by_name(TEST_CHANNEL_NAME, &ch) ==
        NOTIFICATION_ERROR_NONE && ch != nullptr) {
      notification_channel_remove(ch);
      notification_channel_free(ch);
    }
  }

  void TearDown() override {
    notification_channel_h ch = nullptr;
    if (notification_channel_get_by_name(TEST_CHANNEL_NAME, &ch) ==
        NOTIFICATION_ERROR_NONE && ch != nullptr) {
      notification_channel_remove(ch);
      notification_channel_free(ch);
    }
  }
};

/* TC-INT-025: channel_add + channel_get_by_name round-trip */
TEST_F(NotificationChannelIntegTest, channel_add_and_get) {
  notification_channel_h channel = nullptr;
  int ret = notification_channel_create(TEST_CHANNEL_NAME, &channel);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  ASSERT_NE(channel, nullptr);

  ret = notification_channel_add(channel);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  notification_channel_free(channel);

  notification_channel_h fetched = nullptr;
  ret = notification_channel_get_by_name(TEST_CHANNEL_NAME, &fetched);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  EXPECT_NE(fetched, nullptr);

  if (fetched != nullptr)
    notification_channel_free(fetched);
}

/* TC-INT-025b: channel_remove → get_by_name returns NOT_EXIST */
TEST_F(NotificationChannelIntegTest, channel_remove) {
  notification_channel_h channel = nullptr;
  int ret = notification_channel_create(TEST_CHANNEL_NAME, &channel);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  ASSERT_NE(channel, nullptr);
  EXPECT_EQ(notification_channel_add(channel), NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(notification_channel_remove(channel), NOTIFICATION_ERROR_NONE);
  notification_channel_free(channel);

  notification_channel_h fetched = nullptr;
  ret = notification_channel_get_by_name(TEST_CHANNEL_NAME, &fetched);
  EXPECT_NE(ret, NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(fetched, nullptr);
}

/* TC-INT-025c: channel_set_blockable + channel_update round-trip */
TEST_F(NotificationChannelIntegTest, channel_set_blockable_update) {
  notification_channel_h channel = nullptr;
  EXPECT_EQ(notification_channel_create(TEST_CHANNEL_NAME, &channel),
            NOTIFICATION_ERROR_NONE);
  ASSERT_NE(channel, nullptr);
  EXPECT_EQ(notification_channel_add(channel), NOTIFICATION_ERROR_NONE);

  int ret = notification_channel_set_blockable(channel, true);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  ret = notification_channel_update(channel);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  notification_channel_free(channel);
}

/* TC-INT-026: save_as_template + create_from_template round-trip */
TEST_F(NotificationChannelIntegTest, template_round_trip) {
  static constexpr const char* TEMPLATE_NAME = "integ_test_template";

  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);
  notification_set_text(noti, NOTIFICATION_TEXT_TYPE_TITLE,
      "Template Title", nullptr, NOTIFICATION_VARIABLE_TYPE_NONE);
  notification_set_app_id(noti, TEST_APP_ID);

  int ret = notification_save_as_template(noti, TEMPLATE_NAME);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  notification_free(noti);

  notification_h from_tpl = notification_create_from_template(TEMPLATE_NAME);
  EXPECT_NE(from_tpl, nullptr);

  if (from_tpl != nullptr) {
    char* title = nullptr;
    notification_get_text(from_tpl, NOTIFICATION_TEXT_TYPE_TITLE, &title);
    EXPECT_NE(title, nullptr);
    if (title) {
      EXPECT_STREQ(title, "Template Title");
    }
    notification_free(from_tpl);
  }
}

/* TC-INT-025: channel_create with NULL name → error */
TEST_F(NotificationChannelIntegTest, channel_create_null_name_n) {
  notification_channel_h channel = nullptr;
  int ret = notification_channel_create(nullptr, &channel);
  EXPECT_NE(ret, NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(channel, nullptr);
}
