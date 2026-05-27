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

#include <atomic>
#include <thread>
#include <vector>

static constexpr guint WAIT_TIMEOUT_MSEC = 10000;  /* longer for concurrency */
static constexpr const char* TEST_APP_ID = "org.tizen.notification.integtest";
static constexpr int RAPID_POST_COUNT = 50;

class NotificationConcurrencyIntegTest : public ::testing::Test {
 public:
  void SetUp() override {
    mainloop_ = g_main_loop_new(nullptr, FALSE);
    notification_delete_all_by_type(TEST_APP_ID, NOTIFICATION_TYPE_NOTI);
    timed_out_ = false;
    cb_count_ = 0;
    target_count_ = 0;
  }

  void TearDown() override {
    notification_delete_all_by_type(TEST_APP_ID, NOTIFICATION_TYPE_NOTI);
    notification_unresister_changed_cb(on_changed_);
    g_main_loop_unref(mainloop_);
    mainloop_ = nullptr;
    timed_out_ = false;
    cb_count_ = 0;
    target_count_ = 0;
  }

  void RunMainLoop(const char* phase) {
    timed_out_ = false;
    guint timeout_tag = g_timeout_add(
        WAIT_TIMEOUT_MSEC,
        [](gpointer data) -> gboolean {
          auto* self = static_cast<NotificationConcurrencyIntegTest*>(data);
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
    auto* self = static_cast<NotificationConcurrencyIntegTest*>(data);
    self->cb_count_++;
    if (self->cb_count_ >= self->target_count_)
      self->Finish();
  }

 protected:
  GMainLoop* mainloop_ = nullptr;
  bool timed_out_ = false;
  std::atomic<int> cb_count_{0};
  int target_count_ = 0;
};

/* TC-INT-016: changed_cb fires on post from a background thread */
TEST_F(NotificationConcurrencyIntegTest, changed_cb_fires_from_thread) {
  bool cb_fired = false;
  static GMainLoop* loop_ptr;
  loop_ptr = mainloop_;

  /* Lambda is bound to a named variable so it can be unregistered by the
   * same function pointer. Passing a freshly-written lambda to the
   * unregister call would resolve to a different function and leave the
   * real cb installed — the next test would then receive the event,
   * dispatch into this lambda with a now-dangling &cb_fired stack
   * pointer, and corrupt whatever sits there (heap UAF). */
  notification_changed_cb thread_cb =
      [](void* data, notification_type_e /*type*/) {
        *static_cast<bool*>(data) = true;
        g_main_loop_quit(loop_ptr);
      };
  int ret = notification_resister_changed_cb(thread_cb, &cb_fired);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  int post_ret = NOTIFICATION_ERROR_NONE;
  std::thread poster([&]() {
    notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
    if (noti == nullptr) {
      post_ret = NOTIFICATION_ERROR_OUT_OF_MEMORY;
      g_main_loop_quit(loop_ptr);
      return;
    }
    notification_set_app_id(noti, TEST_APP_ID);
    post_ret = notification_post(noti);
    notification_free(noti);
    if (post_ret != NOTIFICATION_ERROR_NONE)
      g_main_loop_quit(loop_ptr);
  });

  RunMainLoop("cross_thread_post_cb");
  poster.join();

  EXPECT_EQ(post_ret, NOTIFICATION_ERROR_NONE);
  EXPECT_TRUE(cb_fired);

  notification_unresister_changed_cb(thread_cb);
}

/* TC-INT-018: rapid post storm — 50 posts, all get unique priv_ids */
TEST_F(NotificationConcurrencyIntegTest, rapid_post_storm_unique_priv_ids) {
  std::vector<int> ids;
  ids.reserve(RAPID_POST_COUNT);

  for (int i = 0; i < RAPID_POST_COUNT; i++) {
    notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
    ASSERT_NE(noti, nullptr);
    notification_set_app_id(noti, TEST_APP_ID);

    int ret = notification_post(noti);
    EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

    if (ret == NOTIFICATION_ERROR_NONE) {
      int priv_id = 0;
      notification_get_id(noti, nullptr, &priv_id);
      ids.push_back(priv_id);
    }
    notification_free(noti);
  }

  /* All priv_ids must be unique */
  for (size_t i = 0; i < ids.size(); i++) {
    for (size_t j = i + 1; j < ids.size(); j++) {
      EXPECT_NE(ids[i], ids[j])
          << "Duplicate priv_id " << ids[i] << " at positions " << i << " and " << j;
    }
  }

  /* Count must match */
  int count = 0;
  notification_get_all_count(NOTIFICATION_TYPE_NOTI, &count);
  EXPECT_EQ(count, RAPID_POST_COUNT);
}

/* TC-INT-018b: rapid post + delete storm — count returns to zero */
TEST_F(NotificationConcurrencyIntegTest, rapid_post_then_delete_all) {
  for (int i = 0; i < RAPID_POST_COUNT; i++) {
    notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
    ASSERT_NE(noti, nullptr);
    notification_set_app_id(noti, TEST_APP_ID);
    EXPECT_EQ(notification_post(noti), NOTIFICATION_ERROR_NONE);
    notification_free(noti);
  }

  int before = 0;
  notification_get_all_count(NOTIFICATION_TYPE_NOTI, &before);
  EXPECT_EQ(before, RAPID_POST_COUNT);

  /* Use _by_type because each posted noti has caller_app_id pinned to
   * TEST_APP_ID, which doesn't match the pid-derived id that the bare
   * notification_delete_all() would use. */
  EXPECT_EQ(notification_delete_all_by_type(TEST_APP_ID, NOTIFICATION_TYPE_NOTI),
            NOTIFICATION_ERROR_NONE);

  int after = 0;
  notification_get_all_count(NOTIFICATION_TYPE_NOTI, &after);
  EXPECT_EQ(after, 0);
}

/* TC-INT-016b: multiple changed_cb registrations all fire */
TEST_F(NotificationConcurrencyIntegTest, multiple_changed_cb_all_fire) {
  static bool cb1_fired = false;
  static bool cb2_fired = false;
  static GMainLoop* loop_ptr;
  loop_ptr = mainloop_;

  auto cb1 = [](void* /*data*/, notification_type_e /*type*/) {
    cb1_fired = true;
    if (cb1_fired && cb2_fired)
      g_main_loop_quit(loop_ptr);
  };
  auto cb2 = [](void* /*data*/, notification_type_e /*type*/) {
    cb2_fired = true;
    if (cb1_fired && cb2_fired)
      g_main_loop_quit(loop_ptr);
  };

  cb1_fired = false;
  cb2_fired = false;

  notification_resister_changed_cb(cb1, nullptr);
  notification_resister_changed_cb(cb2, nullptr);

  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);
  notification_set_app_id(noti, TEST_APP_ID);
  EXPECT_EQ(notification_post(noti), NOTIFICATION_ERROR_NONE);

  RunMainLoop("two_cbs_fire");
  EXPECT_TRUE(cb1_fired);
  EXPECT_TRUE(cb2_fired);

  notification_unresister_changed_cb(cb1);
  notification_unresister_changed_cb(cb2);
  notification_free(noti);
}
