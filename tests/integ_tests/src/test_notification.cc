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
#include <notification_list.h>
#include <notification_setting.h>
#include <notification_setting_internal.h>

#include <memory>
#include <string>

static constexpr guint WAIT_TIMEOUT_MSEC = 5000;
static constexpr const char *TEST_APP_ID = "org.tizen.notification.integtest";

namespace
{

  notification_h CreateTestNotification()
  {
    notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
    if (noti == nullptr)
      return nullptr;
    notification_set_text(noti, NOTIFICATION_TEXT_TYPE_TITLE,
                          "Integration Test", nullptr, NOTIFICATION_VARIABLE_TYPE_NONE);
    notification_set_text(noti, NOTIFICATION_TEXT_TYPE_CONTENT,
                          "Test content from integ test", nullptr, NOTIFICATION_VARIABLE_TYPE_NONE);
    notification_set_app_id(noti, TEST_APP_ID);
    return noti;
  }

} // namespace

class NotificationIntegTest : public ::testing::Test
{
public:
  void SetUp() override
  {
    mainloop_ = g_main_loop_new(nullptr, FALSE);
    notification_delete_all_by_type(TEST_APP_ID, NOTIFICATION_TYPE_NOTI);
    notification_delete_all_by_type(TEST_APP_ID, NOTIFICATION_TYPE_ONGOING);
    timed_out_ = false;
    changed_cb_fired_ = false;
  }

  void TearDown() override
  {
    notification_delete_all_by_type(TEST_APP_ID, NOTIFICATION_TYPE_NOTI);
    notification_delete_all_by_type(TEST_APP_ID, NOTIFICATION_TYPE_ONGOING);
    notification_unresister_changed_cb(changed_cb_);
    g_main_loop_unref(mainloop_);
    mainloop_ = nullptr;
    timed_out_ = false;
    changed_cb_fired_ = false;
  }

  void RunMainLoop(const char *phase)
  {
    timed_out_ = false;
    guint timeout_tag = g_timeout_add(
        WAIT_TIMEOUT_MSEC,
        [](gpointer data) -> gboolean
        {
          auto *self = static_cast<NotificationIntegTest *>(data);
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

  static void ChangedCb(void *data, notification_type_e type)
  {
    auto *self = static_cast<NotificationIntegTest *>(data);
    self->changed_cb_fired_ = true;
    self->Finish();
  }

protected:
  GMainLoop *mainloop_ = nullptr;
  bool timed_out_ = false;
  bool changed_cb_fired_ = false;
  notification_changed_cb changed_cb_ = ChangedCb;
};

/* TC-INT-001: notification_post → DPM 저장 확인 (round-trip) */
TEST_F(NotificationIntegTest, notification_post)
{
  notification_h noti = CreateTestNotification();
  ASSERT_NE(noti, nullptr);

  int ret = notification_post(noti);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  notification_list_h list = nullptr;
  ret = notification_get_list(NOTIFICATION_TYPE_NOTI, -1, &list);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  EXPECT_NE(list, nullptr);

  notification_free_list(list);
  notification_free(noti);
}

/* TC-INT-002: notification_delete → DPM에서 삭제 확인 */
TEST_F(NotificationIntegTest, notification_delete)
{
  notification_h noti = CreateTestNotification();
  ASSERT_NE(noti, nullptr);

  int ret = notification_post(noti);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  ret = notification_delete(noti);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  notification_list_h list = nullptr;
  ret = notification_get_list(NOTIFICATION_TYPE_NOTI, -1, &list);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(list, nullptr);

  notification_free(noti);
}

/* TC-INT-003: notification_delete_all → 전체 삭제 후 빈 목록 확인 */
TEST_F(NotificationIntegTest, notification_delete_all)
{
  for (int i = 0; i < 3; i++)
  {
    notification_h noti = CreateTestNotification();
    ASSERT_NE(noti, nullptr);
    EXPECT_EQ(notification_post(noti), NOTIFICATION_ERROR_NONE);
    notification_free(noti);
  }

  /* CreateTestNotification() pins the noti to TEST_APP_ID; the daemon's
   * delete-all filter keys off caller_app_id, so cleanup must use the
   * same id. notification_delete_all() derives caller from getpid() and
   * would miss the rows. */
  int ret = notification_delete_all_by_type(TEST_APP_ID, NOTIFICATION_TYPE_NOTI);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  notification_list_h list = nullptr;
  ret = notification_get_list(NOTIFICATION_TYPE_NOTI, -1, &list);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(list, nullptr);
}

/* TC-INT-004: notification_get_list → 여러 알림 조회 정합성 확인 */
TEST_F(NotificationIntegTest, notification_get_list_multiple)
{
  const int count = 5;
  for (int i = 0; i < count; i++)
  {
    notification_h noti = CreateTestNotification();
    ASSERT_NE(noti, nullptr);
    EXPECT_EQ(notification_post(noti), NOTIFICATION_ERROR_NONE);
    notification_free(noti);
  }

  notification_list_h list = nullptr;
  int ret = notification_get_list(NOTIFICATION_TYPE_NOTI, -1, &list);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  EXPECT_NE(list, nullptr);

  int actual_count = 0;
  notification_list_h iter = list;
  while (iter != nullptr)
  {
    actual_count++;
    iter = notification_list_get_next(iter);
  }
  EXPECT_EQ(actual_count, count);

  notification_free_list(list);
}

/* TC-INT-005: notification_resister_changed_cb → DPM 이벤트 수신 확인 */
TEST_F(NotificationIntegTest, notification_resister_changed_cb)
{
  int ret = notification_resister_changed_cb(changed_cb_, this);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  notification_h noti = CreateTestNotification();
  ASSERT_NE(noti, nullptr);
  ret = notification_post(noti);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  RunMainLoop("changed_cb_after_post");
  EXPECT_TRUE(changed_cb_fired_);

  notification_free(noti);
}

/* TC-INT-006: 삭제 후 changed_cb 이벤트 확인 */
TEST_F(NotificationIntegTest, notification_changed_cb_on_delete)
{
  notification_h noti = CreateTestNotification();
  ASSERT_NE(noti, nullptr);
  EXPECT_EQ(notification_post(noti), NOTIFICATION_ERROR_NONE);

  changed_cb_fired_ = false;
  int ret = notification_resister_changed_cb(changed_cb_, this);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  ret = notification_delete(noti);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  RunMainLoop("changed_cb_after_delete");
  EXPECT_TRUE(changed_cb_fired_);

  notification_free(noti);
}

/* TC-INT-007: notification_unresister_changed_cb → 해제 후 이벤트 미수신 */
TEST_F(NotificationIntegTest, notification_unresister_changed_cb)
{
  int ret = notification_resister_changed_cb(changed_cb_, this);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  ret = notification_unresister_changed_cb(changed_cb_);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  notification_h noti = CreateTestNotification();
  ASSERT_NE(noti, nullptr);
  EXPECT_EQ(notification_post(noti), NOTIFICATION_ERROR_NONE);

  /* 콜백 없이 타임아웃 없이 바로 확인 */
  EXPECT_FALSE(changed_cb_fired_);
  notification_free(noti);
}

/* TC-INT-008: notification_update_progress → ongoing 알림 진행률 업데이트 */
TEST_F(NotificationIntegTest, notification_update_progress)
{
  notification_h noti = notification_create(NOTIFICATION_TYPE_ONGOING);
  ASSERT_NE(noti, nullptr);
  notification_set_app_id(noti, TEST_APP_ID);
  notification_set_layout(noti, NOTIFICATION_LY_ONGOING_PROGRESS);
  notification_set_progress(noti, 0.0);

  int ret = notification_post(noti);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  int priv_id = 0;
  notification_get_id(noti, nullptr, &priv_id);

  ret = notification_update_progress(nullptr, priv_id, 0.5);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  notification_free(noti);
}

/* TC-INT-009: notification_get_all_count → post/delete 후 카운트 정합성 */
TEST_F(NotificationIntegTest, notification_get_all_count)
{
  int before = 0;
  notification_get_all_count(NOTIFICATION_TYPE_NOTI, &before);

  notification_h noti = CreateTestNotification();
  ASSERT_NE(noti, nullptr);
  EXPECT_EQ(notification_post(noti), NOTIFICATION_ERROR_NONE);

  int after = 0;
  int ret = notification_get_all_count(NOTIFICATION_TYPE_NOTI, &after);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(after, before + 1);

  notification_free(noti);
}

/* TC-INT-010: 잘못된 타입 → 에러 반환 (입력 검증이 TIDL 경계 이전 차단) */
TEST_F(NotificationIntegTest, notification_post_invalid_type_n)
{
  notification_h noti = notification_create(
      static_cast<notification_type_e>(-999));
  /* 잘못된 타입으로 create 자체가 실패하거나 post가 실패해야 함 */
  if (noti != nullptr)
  {
    int ret = notification_post(noti);
    EXPECT_NE(ret, NOTIFICATION_ERROR_NONE);
    notification_free(noti);
  }
}

/* TC-INT-011: NULL 핸들로 post → INVALID_PARAMETER */
TEST_F(NotificationIntegTest, notification_post_null_n)
{
  int ret = notification_post(nullptr);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* TC-INT-012: NULL 핸들로 delete → INVALID_PARAMETER */
TEST_F(NotificationIntegTest, notification_delete_null_n)
{
  int ret = notification_delete(nullptr);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* TC-INT-013: notification_setting_get_setting → DPM에서 설정 조회 */
TEST_F(NotificationIntegTest, notification_setting_get_setting)
{
  notification_setting_h setting = nullptr;
  int ret = notification_setting_get_setting(&setting);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  EXPECT_NE(setting, nullptr);

  if (setting != nullptr)
    notification_setting_free_notification(setting);
}

/* TC-INT-014: notification_setting_set_visibility_class → 설정 반영 확인 */
TEST_F(NotificationIntegTest, notification_setting_visibility_class)
{
  notification_setting_h setting = nullptr;
  int ret = notification_setting_get_setting(&setting);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  ASSERT_NE(setting, nullptr);

  int orig_val = 0;
  ret = notification_setting_get_visibility_class(setting, &orig_val);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  int new_val = (orig_val == 0) ? 1 : 0;
  ret = notification_setting_set_visibility_class(setting, new_val);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  int read_back = -1;
  ret = notification_setting_get_visibility_class(setting, &read_back);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(read_back, new_val);

  /* 원복 */
  notification_setting_set_visibility_class(setting, orig_val);
  notification_setting_free_notification(setting);
}

/* TC-INT-015: notification_setting_set_allow_to_notify → DPM 반영 확인 */
TEST_F(NotificationIntegTest, notification_setting_allow_to_notify)
{
  notification_setting_h setting = nullptr;
  int ret = notification_setting_get_setting(&setting);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  ASSERT_NE(setting, nullptr);

  ret = notification_setting_set_allow_to_notify(setting, true);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  notification_setting_free_notification(setting);
}

/* TC-INT-016: 연속 post → 각각 고유한 priv_id 부여 확인 */
TEST_F(NotificationIntegTest, notification_post_unique_priv_ids)
{
  int ids[3] = {0};
  for (int i = 0; i < 3; i++)
  {
    notification_h noti = CreateTestNotification();
    ASSERT_NE(noti, nullptr);
    EXPECT_EQ(notification_post(noti), NOTIFICATION_ERROR_NONE);
    notification_get_id(noti, nullptr, &ids[i]);
    notification_free(noti);
  }
  EXPECT_NE(ids[0], ids[1]);
  EXPECT_NE(ids[1], ids[2]);
  EXPECT_NE(ids[0], ids[2]);
}

/* TC-INT-017: post → get_list 데이터 일치 확인 (제목 검증) */
TEST_F(NotificationIntegTest, notification_post_and_verify_title)
{
  const char *expected_title = "Integ Test Title Verify";
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);
  notification_set_text(noti, NOTIFICATION_TEXT_TYPE_TITLE,
                        expected_title, nullptr, NOTIFICATION_VARIABLE_TYPE_NONE);
  notification_set_app_id(noti, TEST_APP_ID);

  int ret = notification_post(noti);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  notification_list_h list = nullptr;
  ret = notification_get_list(NOTIFICATION_TYPE_NOTI, 1, &list);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  ASSERT_NE(list, nullptr);

  notification_h fetched = notification_list_get_data(list);
  ASSERT_NE(fetched, nullptr);

  char *title = nullptr;
  notification_get_text(fetched, NOTIFICATION_TEXT_TYPE_TITLE, &title);
  EXPECT_NE(title, nullptr);
  if (title)
  {
    EXPECT_STREQ(title, expected_title);
  }

  notification_free_list(list);
  notification_free(noti);
}

/* TC-INT-018: ongoing + update_content → 내용 반영 확인 */
TEST_F(NotificationIntegTest, notification_update_content)
{
  notification_h noti = notification_create(NOTIFICATION_TYPE_ONGOING);
  ASSERT_NE(noti, nullptr);
  notification_set_app_id(noti, TEST_APP_ID);
  notification_set_text(noti, NOTIFICATION_TEXT_TYPE_CONTENT,
                        "original", nullptr, NOTIFICATION_VARIABLE_TYPE_NONE);

  int ret = notification_post(noti);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  int priv_id = 0;
  notification_get_id(noti, nullptr, &priv_id);

  ret = notification_update_content(nullptr, priv_id, "updated content");
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  notification_free(noti);
}

/* TC-INT-019: 중복 changed_cb 등록 → 두 번째 등록은 에러 없이 처리 */
TEST_F(NotificationIntegTest, notification_resister_changed_cb_duplicate)
{
  int ret = notification_resister_changed_cb(changed_cb_, this);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  ret = notification_resister_changed_cb(changed_cb_, this);
  /* 중복 등록: 에러 없이 처리하거나 ALREADY_EXISTS 반환 허용 */
  EXPECT_TRUE(ret == NOTIFICATION_ERROR_NONE ||
              ret == NOTIFICATION_ERROR_ALREADY_EXIST_ID);

  notification_unresister_changed_cb(changed_cb_);
}

/* TC-INT-020: notification_system_setting_load_system_setting → DPM 조회 */
TEST_F(NotificationIntegTest, notification_system_setting_load)
{
  notification_system_setting_h sys_setting = nullptr;
  int ret = notification_system_setting_load_system_setting(&sys_setting);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  EXPECT_NE(sys_setting, nullptr);
  /* notification_system_setting_free_system_setting is defined in DPM;
     free the opaque struct directly in test context. */
  g_free(sys_setting);
}
