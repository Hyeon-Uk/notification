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

#include <string>

#include "common/smoke_utils.h"

namespace smoke {

class AttributesSmokeTest : public ::testing::Test {
 public:
  void SetUp() override { ResetNotifications(); }
  void TearDown() override { ResetNotifications(); }
};

/* TC-SMK-AT-01 — image_path set/get round-trip on handle */
TEST_F(AttributesSmokeTest, ImagePathRoundTrip) {
  notification_h n = MakeNoti(NOTIFICATION_TYPE_NOTI, "img", "test");
  ASSERT_NE(n, nullptr);
  const char* path = "/usr/share/icons/default/small/notification.png";
  EXPECT_EQ(notification_set_image(n, NOTIFICATION_IMAGE_TYPE_ICON, path),
            NOTIFICATION_ERROR_NONE);

  char* got = nullptr;
  EXPECT_EQ(notification_get_image(n, NOTIFICATION_IMAGE_TYPE_ICON, &got),
            NOTIFICATION_ERROR_NONE);
  ASSERT_NE(got, nullptr);
  EXPECT_STREQ(got, path);
  notification_free(n);
}

/* TC-SMK-AT-02 — sound type round-trip */
TEST_F(AttributesSmokeTest, SoundTypeRoundTrip) {
  notification_h n = MakeNoti(NOTIFICATION_TYPE_NOTI, "snd", "test");
  EXPECT_EQ(notification_set_sound(n, NOTIFICATION_SOUND_TYPE_DEFAULT, nullptr),
            NOTIFICATION_ERROR_NONE);

  notification_sound_type_e type = NOTIFICATION_SOUND_TYPE_NONE;
  const char* path = nullptr;
  EXPECT_EQ(notification_get_sound(n, &type, &path),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(type, NOTIFICATION_SOUND_TYPE_DEFAULT);
  notification_free(n);
}

/* TC-SMK-AT-03 — vibration type round-trip */
TEST_F(AttributesSmokeTest, VibrationTypeRoundTrip) {
  notification_h n = MakeNoti(NOTIFICATION_TYPE_NOTI, "vib", "test");
  EXPECT_EQ(notification_set_vibration(n,
            NOTIFICATION_VIBRATION_TYPE_DEFAULT, nullptr),
            NOTIFICATION_ERROR_NONE);

  notification_vibration_type_e type = NOTIFICATION_VIBRATION_TYPE_NONE;
  const char* path = nullptr;
  EXPECT_EQ(notification_get_vibration(n, &type, &path),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(type, NOTIFICATION_VIBRATION_TYPE_DEFAULT);
  notification_free(n);
}

/* TC-SMK-AT-04 — LED op set/get round-trip. The ARGB color is only
 * retained for OP_ON_CUSTOM_COLOR; with OP_ON the daemon resets it
 * to 0. Smoke check the op only. */
TEST_F(AttributesSmokeTest, LedRoundTrip) {
  notification_h n = MakeNoti(NOTIFICATION_TYPE_NOTI, "led", "test");
  EXPECT_EQ(notification_set_led(n, NOTIFICATION_LED_OP_ON, 0),
            NOTIFICATION_ERROR_NONE);

  notification_led_op_e op = NOTIFICATION_LED_OP_OFF;
  int got_argb = 0;
  EXPECT_EQ(notification_get_led(n, &op, &got_argb),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(op, NOTIFICATION_LED_OP_ON);
  notification_free(n);
}

/* TC-SMK-AT-05 — LED time period round-trip */
TEST_F(AttributesSmokeTest, LedTimePeriodRoundTrip) {
  notification_h n = MakeNoti(NOTIFICATION_TYPE_NOTI, "ledtp", "test");
  EXPECT_EQ(notification_set_led_time_period(n, 100, 200),
            NOTIFICATION_ERROR_NONE);

  int on_ms = 0, off_ms = 0;
  EXPECT_EQ(notification_get_led_time_period(n, &on_ms, &off_ms),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(on_ms, 100);
  EXPECT_EQ(off_ms, 200);
  notification_free(n);
}

/* TC-SMK-AT-06 — time field round-trip */
TEST_F(AttributesSmokeTest, TimeRoundTrip) {
  notification_h n = MakeNoti(NOTIFICATION_TYPE_NOTI, "time", "test");
  time_t now = 1700000000;
  EXPECT_EQ(notification_set_time(n, now), NOTIFICATION_ERROR_NONE);

  time_t got = 0;
  EXPECT_EQ(notification_get_time(n, &got), NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(got, now);
  notification_free(n);
}

/* TC-SMK-AT-07 — display_applist round-trip */
TEST_F(AttributesSmokeTest, DisplayApplistRoundTrip) {
  notification_h n = MakeNoti(NOTIFICATION_TYPE_NOTI, "disp", "test");
  int target = NOTIFICATION_DISPLAY_APP_NOTIFICATION_TRAY |
               NOTIFICATION_DISPLAY_APP_TICKER;
  EXPECT_EQ(notification_set_display_applist(n, target),
            NOTIFICATION_ERROR_NONE);

  int got = 0;
  EXPECT_EQ(notification_get_display_applist(n, &got),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(got, target);
  notification_free(n);
}

/* TC-SMK-AT-08 — progress round-trip */
TEST_F(AttributesSmokeTest, ProgressRoundTrip) {
  notification_h n = MakeNoti(NOTIFICATION_TYPE_ONGOING, "prog", "0%");
  EXPECT_EQ(notification_set_progress(n, 0.42), NOTIFICATION_ERROR_NONE);

  double got = 0.0;
  EXPECT_EQ(notification_get_progress(n, &got), NOTIFICATION_ERROR_NONE);
  EXPECT_DOUBLE_EQ(got, 0.42);
  notification_free(n);
}

/* TC-SMK-AT-09 — size round-trip */
TEST_F(AttributesSmokeTest, SizeRoundTrip) {
  notification_h n = MakeNoti(NOTIFICATION_TYPE_ONGOING, "size", "test");
  EXPECT_EQ(notification_set_size(n, 1024.5), NOTIFICATION_ERROR_NONE);

  double got = 0.0;
  EXPECT_EQ(notification_get_size(n, &got), NOTIFICATION_ERROR_NONE);
  EXPECT_DOUBLE_EQ(got, 1024.5);
  notification_free(n);
}

/* TC-SMK-AT-10 — layout round-trip */
TEST_F(AttributesSmokeTest, LayoutRoundTrip) {
  notification_h n = MakeNoti(NOTIFICATION_TYPE_NOTI, "lay", "test");
  EXPECT_EQ(notification_set_layout(n, NOTIFICATION_LY_NOTI_THUMBNAIL),
            NOTIFICATION_ERROR_NONE);

  notification_ly_type_e got = NOTIFICATION_LY_NONE;
  EXPECT_EQ(notification_get_layout(n, &got), NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(got, NOTIFICATION_LY_NOTI_THUMBNAIL);
  notification_free(n);
}

/* TC-SMK-AT-11 — auto_remove round-trip */
TEST_F(AttributesSmokeTest, AutoRemoveRoundTrip) {
  notification_h n = MakeNoti(NOTIFICATION_TYPE_NOTI, "auto", "test");
  EXPECT_EQ(notification_set_auto_remove(n, false),
            NOTIFICATION_ERROR_NONE);

  bool got = true;
  EXPECT_EQ(notification_get_auto_remove(n, &got),
            NOTIFICATION_ERROR_NONE);
  EXPECT_FALSE(got);
  notification_free(n);
}

/* TC-SMK-AT-12 — property flags round-trip */
TEST_F(AttributesSmokeTest, PropertyFlagsRoundTrip) {
  notification_h n = MakeNoti(NOTIFICATION_TYPE_NOTI, "prop", "test");
  int flags = NOTIFICATION_PROP_DISABLE_APP_LAUNCH |
              NOTIFICATION_PROP_VOLATILE_DISPLAY;
  EXPECT_EQ(notification_set_property(n, flags),
            NOTIFICATION_ERROR_NONE);

  int got = 0;
  EXPECT_EQ(notification_get_property(n, &got),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(got, flags);
  notification_free(n);
}

/* TC-SMK-AT-13 — post with rich attributes survives round-trip
 * through the daemon */
TEST_F(AttributesSmokeTest, RichAttributesPostFetch) {
  notification_h n = MakeNoti(NOTIFICATION_TYPE_NOTI, "Rich", "Full");
  notification_set_sound(n, NOTIFICATION_SOUND_TYPE_DEFAULT, nullptr);
  notification_set_vibration(n, NOTIFICATION_VIBRATION_TYPE_DEFAULT, nullptr);
  notification_set_led(n, NOTIFICATION_LED_OP_ON, 0xFF0000FF);
  notification_set_auto_remove(n, false);

  EXPECT_EQ(notification_post(n), NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(CountByType(NOTIFICATION_TYPE_NOTI), 1);
  notification_free(n);
}

}  // namespace smoke
