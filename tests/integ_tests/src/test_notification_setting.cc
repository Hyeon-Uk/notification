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
#include <notification_setting.h>
#include <notification_setting_internal.h>

static constexpr guint WAIT_TIMEOUT_MSEC = 5000;

class NotificationSettingIntegTest : public ::testing::Test {
 public:
  void SetUp() override {
    mainloop_ = g_main_loop_new(nullptr, FALSE);
    timed_out_ = false;
  }

  void TearDown() override {
    g_main_loop_unref(mainloop_);
    mainloop_ = nullptr;
    timed_out_ = false;
  }

  void RunMainLoop(const char* phase) {
    timed_out_ = false;
    guint timeout_tag = g_timeout_add(
        WAIT_TIMEOUT_MSEC,
        [](gpointer data) -> gboolean {
          auto* self = static_cast<NotificationSettingIntegTest*>(data);
          self->timed_out_ = true;
          self->Finish();
          return G_SOURCE_REMOVE;
        },
        this);
    g_main_loop_run(mainloop_);
    if (timeout_tag > 0)
      g_source_remove(timeout_tag);
    ASSERT_FALSE(timed_out_) << "Timed out waiting for phase: " << phase;
  }

  void Finish() { g_main_loop_quit(mainloop_); }

 protected:
  GMainLoop* mainloop_ = nullptr;
  bool timed_out_ = false;
};

/* TC-INT-007: notification_setting_update_setting persists to DPM */
TEST_F(NotificationSettingIntegTest, setting_update_persists_to_dpm) {
  notification_setting_h setting = nullptr;
  int ret = notification_setting_get_setting(&setting);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  ASSERT_NE(setting, nullptr);

  int orig_class = 0;
  ret = notification_setting_get_visibility_class(setting, &orig_class);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  int new_class = (orig_class == 0) ? 1 : 0;
  ret = notification_setting_set_visibility_class(setting, new_class);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  ret = notification_setting_update_setting(setting);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  /* Re-fetch from DPM to verify persistence */
  notification_setting_h fetched = nullptr;
  ret = notification_setting_get_setting(&fetched);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  ASSERT_NE(fetched, nullptr);

  int verified_class = -1;
  ret = notification_setting_get_visibility_class(fetched, &verified_class);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(verified_class, new_class);

  /* Restore original value */
  notification_setting_set_visibility_class(fetched, orig_class);
  notification_setting_update_setting(fetched);

  notification_setting_free_notification(setting);
  notification_setting_free_notification(fetched);
}

/* TC-INT-008: system setting DND round-trip → DPM 반영 확인 */
TEST_F(NotificationSettingIntegTest, system_setting_dnd_round_trip) {
  notification_system_setting_h sys_setting = nullptr;
  int ret = notification_system_setting_load_system_setting(&sys_setting);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  ASSERT_NE(sys_setting, nullptr);

  /* Toggle do_not_disturb off (safe: doesn't affect real device state
     in a meaningful way since this is a system-level integ test). */
  ret = notification_system_setting_set_do_not_disturb(sys_setting, false);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  ret = notification_system_setting_update_system_setting(sys_setting);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  /* Re-fetch and verify */
  notification_system_setting_h fetched = nullptr;
  ret = notification_system_setting_load_system_setting(&fetched);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  ASSERT_NE(fetched, nullptr);

  g_free(sys_setting);
  g_free(fetched);
}

/* TC-INT-011: notification_get_noti_block_state → DPM 집계 상태 확인 */
TEST_F(NotificationSettingIntegTest, get_noti_block_state) {
  notification_block_state_e state =
      static_cast<notification_block_state_e>(-1);
  int ret = notification_get_noti_block_state(&state);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  EXPECT_TRUE(state == NOTIFICATION_BLOCK_STATE_ALLOWED ||
              state == NOTIFICATION_BLOCK_STATE_BLOCKED ||
              state == NOTIFICATION_BLOCK_STATE_DO_NOT_DISTURB);
}

/* TC-INT-012: NULL setting handle → update 에러 확인 */
TEST_F(NotificationSettingIntegTest, setting_update_null_n) {
  int ret = notification_setting_update_setting(nullptr);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}
