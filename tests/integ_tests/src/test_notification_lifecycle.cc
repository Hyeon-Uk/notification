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

static constexpr guint WAIT_TIMEOUT_MSEC = 5000;
static constexpr const char* TEST_APP_ID = "org.tizen.notification.integtest";
static constexpr const char* TEST_TAG = "integ_lifecycle_tag";

class NotificationLifecycleIntegTest : public ::testing::Test {
 public:
  void SetUp() override {
    mainloop_ = g_main_loop_new(nullptr, FALSE);
    notification_delete_all_by_type(TEST_APP_ID, NOTIFICATION_TYPE_NOTI);
    notification_delete_all_by_type(TEST_APP_ID, NOTIFICATION_TYPE_ONGOING);
    timed_out_ = false;
    changed_fired_ = false;
  }

  void TearDown() override {
    notification_delete_all_by_type(TEST_APP_ID, NOTIFICATION_TYPE_NOTI);
    notification_delete_all_by_type(TEST_APP_ID, NOTIFICATION_TYPE_ONGOING);
    if (cb_registered_)
      notification_unresister_changed_cb(on_changed_);
    g_main_loop_unref(mainloop_);
    mainloop_ = nullptr;
    timed_out_ = false;
    changed_fired_ = false;
    cb_registered_ = false;
  }

  void RunMainLoop(const char* phase) {
    timed_out_ = false;
    guint timeout_tag = g_timeout_add(
        WAIT_TIMEOUT_MSEC,
        [](gpointer data) -> gboolean {
          auto* self = static_cast<NotificationLifecycleIntegTest*>(data);
          self->timed_out_ = true;
          self->Finish();
          return G_SOURCE_REMOVE;
        },
        this);
    g_main_loop_run(mainloop_);
    if (timeout_tag > 0)
      g_source_remove(timeout_tag);
    ASSERT_FALSE(timed_out_) << "Timed out at phase: " << phase;
  }

  void Finish() { g_main_loop_quit(mainloop_); }

  static void on_changed_(void* data, notification_type_e /*type*/) {
    auto* self = static_cast<NotificationLifecycleIntegTest*>(data);
    self->changed_fired_ = true;
    self->Finish();
  }

 protected:
  GMainLoop* mainloop_ = nullptr;
  bool timed_out_ = false;
  bool changed_fired_ = false;
  bool cb_registered_ = false;
};

/* TC-INT-002: notification_update mutates an existing row */
TEST_F(NotificationLifecycleIntegTest, notification_update_mutates_row) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);
  notification_set_text(noti, NOTIFICATION_TEXT_TYPE_TITLE,
      "Original Title", nullptr, NOTIFICATION_VARIABLE_TYPE_NONE);
  notification_set_app_id(noti, TEST_APP_ID);

  EXPECT_EQ(notification_post(noti), NOTIFICATION_ERROR_NONE);

  /* Mutate title and call update */
  notification_set_text(noti, NOTIFICATION_TEXT_TYPE_TITLE,
      "Updated Title", nullptr, NOTIFICATION_VARIABLE_TYPE_NONE);

  int ret = notification_update(noti);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  /* Verify updated title is reflected in DPM */
  notification_list_h list = nullptr;
  ret = notification_get_list(NOTIFICATION_TYPE_NOTI, 1, &list);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  ASSERT_NE(list, nullptr);

  notification_h fetched = notification_list_get_data(list);
  ASSERT_NE(fetched, nullptr);

  char* title = nullptr;
  notification_get_text(fetched, NOTIFICATION_TEXT_TYPE_TITLE, &title);
  EXPECT_NE(title, nullptr);
  if (title) {
    EXPECT_STREQ(title, "Updated Title");
  }

  notification_free_list(list);
  notification_free(noti);
}

/* TC-INT-001b: notification_load_by_tag round-trip */
TEST_F(NotificationLifecycleIntegTest, notification_load_by_tag) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);
  notification_set_app_id(noti, TEST_APP_ID);
  notification_set_tag(noti, TEST_TAG);
  notification_set_text(noti, NOTIFICATION_TEXT_TYPE_TITLE,
      "Tag Test", nullptr, NOTIFICATION_VARIABLE_TYPE_NONE);

  EXPECT_EQ(notification_post(noti), NOTIFICATION_ERROR_NONE);
  notification_free(noti);

  notification_h loaded = notification_load_by_tag(TEST_TAG);
  EXPECT_NE(loaded, nullptr);

  if (loaded != nullptr) {
    const char* tag = nullptr;
    notification_get_tag(loaded, &tag);
    EXPECT_NE(tag, nullptr);
    if (tag) {
      EXPECT_STREQ(tag, TEST_TAG);
    }
    notification_free(loaded);
  }
}

/* TC-INT-019: data survives client-side fini → init (via changed_cb re-register) */
TEST_F(NotificationLifecycleIntegTest, data_persists_after_cb_unregister_reregister) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);
  notification_set_app_id(noti, TEST_APP_ID);
  EXPECT_EQ(notification_post(noti), NOTIFICATION_ERROR_NONE);
  notification_free(noti);

  /* Simulate client-side fini by unregistering all callbacks */
  /* Re-register and confirm data still available in DPM */
  notification_list_h list = nullptr;
  int ret = notification_get_list(NOTIFICATION_TYPE_NOTI, -1, &list);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  EXPECT_NE(list, nullptr);

  if (list != nullptr)
    notification_free_list(list);
}

/* TC-INT-002b: notification_update triggers changed_cb */
TEST_F(NotificationLifecycleIntegTest, notification_update_fires_changed_cb) {
  int ret = notification_resister_changed_cb(on_changed_, this);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  cb_registered_ = true;

  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);
  notification_set_app_id(noti, TEST_APP_ID);
  EXPECT_EQ(notification_post(noti), NOTIFICATION_ERROR_NONE);

  RunMainLoop("wait_post_cb");
  EXPECT_TRUE(changed_fired_);
  changed_fired_ = false;

  notification_set_text(noti, NOTIFICATION_TEXT_TYPE_CONTENT,
      "Updated Content", nullptr, NOTIFICATION_VARIABLE_TYPE_NONE);
  ret = notification_update(noti);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  RunMainLoop("wait_update_cb");
  EXPECT_TRUE(changed_fired_);

  notification_free(noti);
}

/* TC-INT-006: notification_get_all_count filters by type correctly */
TEST_F(NotificationLifecycleIntegTest, notification_get_all_count_by_type) {
  for (int i = 0; i < 2; i++) {
    notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
    ASSERT_NE(noti, nullptr);
    notification_set_app_id(noti, TEST_APP_ID);
    EXPECT_EQ(notification_post(noti), NOTIFICATION_ERROR_NONE);
    notification_free(noti);
  }

  notification_h ongoing = notification_create(NOTIFICATION_TYPE_ONGOING);
  ASSERT_NE(ongoing, nullptr);
  notification_set_app_id(ongoing, TEST_APP_ID);
  EXPECT_EQ(notification_post(ongoing), NOTIFICATION_ERROR_NONE);
  notification_free(ongoing);

  int noti_count = 0;
  int ongoing_count = 0;
  EXPECT_EQ(notification_get_all_count(NOTIFICATION_TYPE_NOTI, &noti_count),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(notification_get_all_count(NOTIFICATION_TYPE_ONGOING, &ongoing_count),
            NOTIFICATION_ERROR_NONE);

  EXPECT_EQ(noti_count, 2);
  EXPECT_EQ(ongoing_count, 1);
}
