/*
 * Copyright (c) 2026 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#include <glib.h>
#include <gtest/gtest.h>
#include <notification.h>
#include <notification_internal.h>
#include <notification_setting.h>
#include <notification_setting_internal.h>

#include "common/smoke_utils.h"

namespace smoke {

class SettingSmokeTest : public ::testing::Test {
 public:
  void SetUp() override { ResetNotifications(); }
  void TearDown() override { ResetNotifications(); }
};

/* TC-SMK-ST-01 — get_setting returns a populated handle */
TEST_F(SettingSmokeTest, GetSettingReturnsHandle) {
  notification_setting_h setting = nullptr;
  ASSERT_EQ(notification_setting_get_setting(&setting),
            NOTIFICATION_ERROR_NONE);
  ASSERT_NE(setting, nullptr);
  char* app_id = nullptr;
  notification_setting_get_appid(setting, &app_id);
  ASSERT_NE(app_id, nullptr);
  EXPECT_STREQ(app_id, kSmokeAppId);
  notification_setting_free_notification(setting);
}

/* TC-SMK-ST-02 — visibility_class round-trip persists */
TEST_F(SettingSmokeTest, VisibilityClassRoundTrip) {
  notification_setting_h setting = nullptr;
  notification_setting_get_setting(&setting);
  int orig = 0;
  notification_setting_get_visibility_class(setting, &orig);

  int target = (orig == 0) ? 1 : 0;
  notification_setting_set_visibility_class(setting, target);
  ASSERT_EQ(notification_setting_update_setting(setting),
            NOTIFICATION_ERROR_NONE);

  notification_setting_h fetched = nullptr;
  notification_setting_get_setting(&fetched);
  int got = -1;
  notification_setting_get_visibility_class(fetched, &got);
  EXPECT_EQ(got, target);

  /* restore */
  notification_setting_set_visibility_class(fetched, orig);
  notification_setting_update_setting(fetched);
  notification_setting_free_notification(setting);
  notification_setting_free_notification(fetched);
}

/* TC-SMK-ST-03 — set/update of write-only fields does not error */
TEST_F(SettingSmokeTest, WriteOnlyFieldsUpdateSucceeds) {
  notification_setting_h setting = nullptr;
  ASSERT_EQ(notification_setting_get_setting(&setting),
            NOTIFICATION_ERROR_NONE);

  /* These fields have no public getter; the smoke check is that the
   * setter + update path does not error out. */
  EXPECT_EQ(notification_setting_set_allow_to_notify(setting, true),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(notification_setting_set_do_not_disturb_except(setting, false),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(notification_setting_update_setting(setting),
            NOTIFICATION_ERROR_NONE);

  notification_setting_free_notification(setting);
}

/* TC-SMK-ST-04 — block-state aggregation is queryable */
TEST_F(SettingSmokeTest, BlockStateQueryable) {
  notification_block_state_e state =
      static_cast<notification_block_state_e>(-1);
  ASSERT_EQ(notification_get_noti_block_state(&state),
            NOTIFICATION_ERROR_NONE);
  EXPECT_TRUE(state == NOTIFICATION_BLOCK_STATE_ALLOWED ||
              state == NOTIFICATION_BLOCK_STATE_BLOCKED ||
              state == NOTIFICATION_BLOCK_STATE_DO_NOT_DISTURB);
}

/* TC-SMK-ST-05 — system DND set/update round-trip succeeds */
TEST_F(SettingSmokeTest, SystemDnDSetUpdate) {
  notification_system_setting_h sys = nullptr;
  ASSERT_EQ(notification_system_setting_load_system_setting(&sys),
            NOTIFICATION_ERROR_NONE);
  ASSERT_NE(sys, nullptr);

  EXPECT_EQ(notification_system_setting_set_do_not_disturb(sys, true),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(notification_system_setting_update_system_setting(sys),
            NOTIFICATION_ERROR_NONE);

  /* visibility_class is the one system field with a getter — round-trip it
   * to confirm system_setting persistence too. */
  int orig = 0;
  notification_system_setting_get_visibility_class(sys, &orig);
  int target = (orig == 0) ? 1 : 0;
  notification_system_setting_set_visibility_class(sys, target);
  EXPECT_EQ(notification_system_setting_update_system_setting(sys),
            NOTIFICATION_ERROR_NONE);

  notification_system_setting_h fetched = nullptr;
  notification_system_setting_load_system_setting(&fetched);
  int got = -1;
  notification_system_setting_get_visibility_class(fetched, &got);
  EXPECT_EQ(got, target);

  /* restore */
  notification_system_setting_set_visibility_class(fetched, orig);
  notification_system_setting_set_do_not_disturb(fetched, false);
  notification_system_setting_update_system_setting(fetched);

  g_free(sys);
  g_free(fetched);
}

/* --- Callback-driven scenarios share a tiny harness --- */
struct CbCounter {
  int fired = 0;
  MainLoopRunner* runner = nullptr;
  static void Cb(void* data, notification_type_e /*type*/) {
    auto* c = static_cast<CbCounter*>(data);
    c->fired++;
    if (c->runner) c->runner->Quit();
  }
};

class CbSmokeTest : public ::testing::Test {
 public:
  void SetUp() override { ResetNotifications(); }
  void TearDown() override {
    notification_unresister_changed_cb(CbCounter::Cb);
    ResetNotifications();
  }
};

/* TC-SMK-CB-01 — changed cb fires on post */
TEST_F(CbSmokeTest, FiresOnPost) {
  CbCounter c;
  MainLoopRunner runner;
  c.runner = &runner;
  ASSERT_EQ(notification_resister_changed_cb(CbCounter::Cb, &c),
            NOTIFICATION_ERROR_NONE);

  notification_h n = MakeNoti(NOTIFICATION_TYPE_NOTI, "t", "c");
  ASSERT_EQ(notification_post(n), NOTIFICATION_ERROR_NONE);
  EXPECT_TRUE(runner.RunUntil(kCbTimeoutMs));
  EXPECT_GE(c.fired, 1);

  notification_free(n);
}

/* TC-SMK-CB-02 — changed cb fires on update */
TEST_F(CbSmokeTest, FiresOnUpdate) {
  CbCounter c;
  MainLoopRunner post_runner;
  c.runner = &post_runner;
  ASSERT_EQ(notification_resister_changed_cb(CbCounter::Cb, &c),
            NOTIFICATION_ERROR_NONE);

  notification_h n = MakeNoti(NOTIFICATION_TYPE_NOTI, "t", "v1");
  notification_post(n);
  ASSERT_TRUE(post_runner.RunUntil(kCbTimeoutMs));
  int after_post = c.fired;

  MainLoopRunner update_runner;
  c.runner = &update_runner;
  notification_set_text(n, NOTIFICATION_TEXT_TYPE_CONTENT,
      "v2", nullptr, NOTIFICATION_VARIABLE_TYPE_NONE);
  notification_update(n);
  EXPECT_TRUE(update_runner.RunUntil(kCbTimeoutMs));
  EXPECT_GT(c.fired, after_post);

  notification_free(n);
}

/* TC-SMK-CB-03 — changed cb fires on delete */
TEST_F(CbSmokeTest, FiresOnDelete) {
  CbCounter c;
  MainLoopRunner post_runner;
  c.runner = &post_runner;
  notification_resister_changed_cb(CbCounter::Cb, &c);
  notification_h n = MakeNoti(NOTIFICATION_TYPE_NOTI, "t", "c");
  notification_post(n);
  ASSERT_TRUE(post_runner.RunUntil(kCbTimeoutMs));
  int after_post = c.fired;

  MainLoopRunner del_runner;
  c.runner = &del_runner;
  notification_delete(n);
  EXPECT_TRUE(del_runner.RunUntil(kCbTimeoutMs));
  EXPECT_GT(c.fired, after_post);

  notification_free(n);
}

/* TC-SMK-CB-04 — unregister stops callback delivery */
TEST_F(CbSmokeTest, UnregisterStopsCb) {
  CbCounter c;
  MainLoopRunner first;
  c.runner = &first;
  notification_resister_changed_cb(CbCounter::Cb, &c);

  notification_h n = MakeNoti(NOTIFICATION_TYPE_NOTI, "t", "c");
  notification_post(n);
  ASSERT_TRUE(first.RunUntil(kCbTimeoutMs));
  int before = c.fired;

  ASSERT_EQ(notification_unresister_changed_cb(CbCounter::Cb),
            NOTIFICATION_ERROR_NONE);
  c.runner = nullptr;  // no quit needed; should never fire

  notification_h n2 = MakeNoti(NOTIFICATION_TYPE_NOTI, "t2", "c2");
  notification_post(n2);
  /* Wait briefly — the count must stay flat. */
  MainLoopRunner wait;
  wait.RunUntil(500);
  EXPECT_EQ(c.fired, before);

  notification_free(n);
  notification_free(n2);
}

}  // namespace smoke
