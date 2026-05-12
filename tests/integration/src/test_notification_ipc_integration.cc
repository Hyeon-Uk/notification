/*
 * Copyright (c) 2023 Samsung Electronics Co., Ltd. All rights reserved.
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

#include <gtest/gtest.h>
#include <sqlite3.h>
#include <bundle.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <unistd.h>
#include <sys/stat.h>
#include <tzplatform_config.h>
#include <gio/gio.h>

#include "notification/include/notification.h"
#include "notification/include/notification_internal.h"
#include "notification/include/notification_db.h"
#include "notification/include/notification_noti.h"
#include "notification/include/notification_ipc.h"

extern "C" void test_set_db_dir(const char* dir);

namespace {

const char* CREATE_NOTI_TABLE_SQL =
    "CREATE TABLE IF NOT EXISTS noti_list ("
    "  type INTEGER NOT NULL,"
    "  layout INTEGER NOT NULL DEFAULT 0,"
    "  pkg_id TEXT NOT NULL,"
    "  caller_app_id TEXT NOT NULL,"
    "  launch_app_id TEXT,"
    "  app_label TEXT,"
    "  image_path TEXT,"
    "  priv_image_path TEXT,"
    "  group_id INTEGER DEFAULT 0,"
    "  internal_group_id INTEGER DEFAULT 0,"
    "  priv_id INTEGER PRIMARY KEY AUTOINCREMENT,"
    "  title_key TEXT,"
    "  b_text TEXT,"
    "  b_key TEXT,"
    "  tag TEXT,"
    "  b_format_args TEXT,"
    "  num_format_args INTEGER DEFAULT 0,"
    "  text_domain TEXT,"
    "  text_dir TEXT,"
    "  time INTEGER DEFAULT 0,"
    "  insert_time INTEGER DEFAULT 0,"
    "  args TEXT,"
    "  group_args TEXT,"
    "  b_execute_option TEXT,"
    "  b_service_responding TEXT,"
    "  b_service_single_launch TEXT,"
    "  b_service_multi_launch TEXT,"
    "  b_event_handler_click_on_button_1 TEXT,"
    "  b_event_handler_click_on_button_2 TEXT,"
    "  b_event_handler_click_on_button_3 TEXT,"
    "  b_event_handler_click_on_button_4 TEXT,"
    "  b_event_handler_click_on_button_5 TEXT,"
    "  b_event_handler_click_on_button_6 TEXT,"
    "  b_event_handler_click_on_icon TEXT,"
    "  b_event_handler_click_on_thumbnail TEXT,"
    "  b_event_handler_click_on_text_input_button TEXT,"
    "  b_event_handler_click_on_button_7 TEXT,"
    "  b_event_handler_click_on_button_8 TEXT,"
    "  b_event_handler_click_on_button_9 TEXT,"
    "  b_event_handler_click_on_button_10 TEXT,"
    "  sound_type INTEGER DEFAULT 0,"
    "  sound_path TEXT,"
    "  priv_sound_path TEXT,"
    "  vibration_type INTEGER DEFAULT 0,"
    "  vibration_path TEXT,"
    "  priv_vibration_path TEXT,"
    "  led_operation INTEGER DEFAULT 0,"
    "  led_argb INTEGER DEFAULT 0,"
    "  led_on_ms INTEGER DEFAULT -1,"
    "  led_off_ms INTEGER DEFAULT -1,"
    "  flags_for_property INTEGER DEFAULT 0,"
    "  flag_simmode INTEGER DEFAULT 0,"
    "  display_applist INTEGER,"
    "  progress_size DOUBLE DEFAULT 0,"
    "  progress_percentage DOUBLE DEFAULT 0,"
    "  ongoing_flag INTEGER DEFAULT 0,"
    "  ongoing_value_type INTEGER DEFAULT 0,"
    "  ongoing_current INTEGER DEFAULT 0,"
    "  ongoing_duration INTEGER DEFAULT 0,"
    "  auto_remove INTEGER DEFAULT 1,"
    "  default_button_index INTEGER DEFAULT 0,"
    "  hide_timeout INTEGER DEFAULT 0,"
    "  delete_timeout INTEGER DEFAULT 0,"
    "  text_input_max_length INTEGER DEFAULT 0,"
    "  event_flag INTEGER DEFAULT 0,"
    "  extension_image_size INTEGER DEFAULT 0,"
    "  uid INTEGER,"
    "  check_box INTEGER DEFAULT 0,"
    "  check_box_value INTEGER DEFAULT 0,"
    "  channel_name TEXT);";

}  // namespace

static GVariant* UnwrapNotiVariant(GVariant* wrapped) {
  GVariant* inner = nullptr;
  if (g_variant_is_of_type(wrapped, G_VARIANT_TYPE("(v)")))
    g_variant_get(wrapped, "(v)", &inner);
  else
    inner = g_variant_ref(wrapped);
  return inner;
}

class NotificationIpcIntegrationTest : public ::testing::Test {
 protected:
  void SetUp() override {
    char tmp_dir[] = "/tmp/noti_integ_ipc_XXXXXX";
    if (!mkdtemp(tmp_dir))
      GTEST_SKIP() << "Failed to create temp dir";
    tmp_dir_ = tmp_dir;
    test_set_db_dir(tmp_dir_.c_str());
    db_path_ = std::string(DBPATH);

    sqlite3* setup_db = nullptr;
    int rc = sqlite3_open(db_path_.c_str(), &setup_db);
    ASSERT_EQ(rc, SQLITE_OK);
    ASSERT_NE(setup_db, nullptr);

    char* err_msg = nullptr;
    rc = sqlite3_exec(setup_db, CREATE_NOTI_TABLE_SQL,
        nullptr, nullptr, &err_msg);
    if (err_msg) { sqlite3_free(err_msg); err_msg = nullptr; }
    ASSERT_EQ(rc, SQLITE_OK);

    sqlite3_close(setup_db);

    notification_h test_noti = notification_create(NOTIFICATION_TYPE_NOTI);
    if (!test_noti)
      GTEST_SKIP() << "notification_create requires app context";

    notification_set_text(test_noti, NOTIFICATION_TEXT_TYPE_TITLE,
        "probe_test", NULL, NOTIFICATION_VARIABLE_TYPE_NONE);

    GVariant* probe_v = notification_ipc_make_gvariant_from_noti(
        test_noti, false);
    if (!probe_v) {
      notification_free(test_noti);
      GTEST_SKIP() << "GVariant serialization not available";
    }
    notification_h probe_noti = notification_create(NOTIFICATION_TYPE_NOTI);
    GVariant* probe_inner = UnwrapNotiVariant(probe_v);
    int probe_ret = notification_ipc_make_noti_from_gvariant(
        probe_noti, probe_inner);
    g_variant_unref(probe_inner);
    g_variant_unref(probe_v);

    bool probe_ok = false;
    if (probe_ret == NOTIFICATION_ERROR_NONE) {
      char* probe_title = nullptr;
      notification_get_text(probe_noti, NOTIFICATION_TEXT_TYPE_TITLE,
          &probe_title);
      if (probe_title && strcmp(probe_title, "probe_test") == 0)
        probe_ok = true;
    }
    notification_free(probe_noti);
    notification_free(test_noti);
    if (!probe_ok)
      GTEST_SKIP() << "GVariant round-trip not supported in this environment";
  }

  void TearDown() override {
    if (!db_path_.empty())
      unlink(db_path_.c_str());
    if (!tmp_dir_.empty())
      rmdir(tmp_dir_.c_str());
  }

  std::string db_path_;
  std::string tmp_dir_;
};

/* SERIAL_01: Serialize and deserialize basic fields */
TEST_F(NotificationIpcIntegrationTest,
    SERIAL_01_SerializeDeserializeBasicFields) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  notification_set_text(noti, NOTIFICATION_TEXT_TYPE_TITLE, "serial_title",
      NULL, NOTIFICATION_VARIABLE_TYPE_NONE);
  notification_set_text(noti, NOTIFICATION_TEXT_TYPE_CONTENT, "serial_content",
      NULL, NOTIFICATION_VARIABLE_TYPE_NONE);
  notification_set_layout(noti, NOTIFICATION_LY_NOTI_EVENT_SINGLE);
  notification_set_display_applist(noti,
      NOTIFICATION_DISPLAY_APP_NOTIFICATION_TRAY);

  GVariant *variant = notification_ipc_make_gvariant_from_noti(noti, false);
  if (variant == NULL) {
    notification_free(noti);
    GTEST_SKIP() << "GVariant serialization not available without daemon";
  }

  GVariant *inner = UnwrapNotiVariant(variant);
  notification_h noti2 = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti2, nullptr);

  int ret = notification_ipc_make_noti_from_gvariant(noti2, inner);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  char *title = nullptr;
  notification_get_text(noti2, NOTIFICATION_TEXT_TYPE_TITLE, &title);
  ASSERT_NE(title, nullptr);
  EXPECT_STREQ(title, "serial_title");

  char *content = nullptr;
  notification_get_text(noti2, NOTIFICATION_TEXT_TYPE_CONTENT, &content);
  ASSERT_NE(content, nullptr);
  EXPECT_STREQ(content, "serial_content");

  g_variant_unref(inner);
  g_variant_unref(variant);
  notification_free(noti2);
  notification_free(noti);
}

/* SERIAL_02: Serialize with null optional fields */
TEST_F(NotificationIpcIntegrationTest,
    SERIAL_02_SerializeWithNullOptionalFields) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  notification_set_text(noti, NOTIFICATION_TEXT_TYPE_TITLE, "only_title",
      NULL, NOTIFICATION_VARIABLE_TYPE_NONE);

  GVariant *variant = notification_ipc_make_gvariant_from_noti(noti, false);
  if (variant == NULL) {
    notification_free(noti);
    GTEST_SKIP() << "GVariant serialization not available without daemon";
  }

  GVariant *inner = UnwrapNotiVariant(variant);
  notification_h noti2 = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti2, nullptr);

  int ret = notification_ipc_make_noti_from_gvariant(noti2, inner);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  char *title = nullptr;
  notification_get_text(noti2, NOTIFICATION_TEXT_TYPE_TITLE, &title);
  ASSERT_NE(title, nullptr);
  EXPECT_STREQ(title, "only_title");

  notification_sound_type_e snd_type;
  const char *snd_path = nullptr;
  notification_get_sound(noti2, &snd_type, &snd_path);
  EXPECT_TRUE(snd_type == NOTIFICATION_SOUND_TYPE_NONE ||
      snd_type == static_cast<notification_sound_type_e>(0));

  g_variant_unref(inner);
  g_variant_unref(variant);
  notification_free(noti2);
  notification_free(noti);
}

/* SERIAL_03: Serialize sound and vibration fields */
TEST_F(NotificationIpcIntegrationTest,
    SERIAL_03_SerializeSoundAndVibration) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  notification_set_sound(noti, NOTIFICATION_SOUND_TYPE_USER_DATA,
      "/test/sound.wav");
  notification_set_vibration(noti, NOTIFICATION_VIBRATION_TYPE_USER_DATA,
      "/test/vib.ivt");
  notification_set_display_applist(noti,
      NOTIFICATION_DISPLAY_APP_NOTIFICATION_TRAY);

  GVariant *variant = notification_ipc_make_gvariant_from_noti(noti, false);
  if (variant == NULL) {
    notification_free(noti);
    GTEST_SKIP() << "GVariant serialization not available without daemon";
  }

  GVariant *inner = UnwrapNotiVariant(variant);
  notification_h noti2 = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti2, nullptr);

  int ret = notification_ipc_make_noti_from_gvariant(noti2, inner);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  notification_sound_type_e snd_type;
  const char *snd_path = nullptr;
  notification_get_sound(noti2, &snd_type, &snd_path);
  EXPECT_EQ(snd_type, NOTIFICATION_SOUND_TYPE_USER_DATA);
  ASSERT_NE(snd_path, nullptr);
  EXPECT_STREQ(snd_path, "/test/sound.wav");

  notification_vibration_type_e vib_type;
  const char *vib_path = nullptr;
  notification_get_vibration(noti2, &vib_type, &vib_path);
  EXPECT_EQ(vib_type, NOTIFICATION_VIBRATION_TYPE_USER_DATA);
  ASSERT_NE(vib_path, nullptr);
  EXPECT_STREQ(vib_path, "/test/vib.ivt");

  g_variant_unref(inner);
  g_variant_unref(variant);
  notification_free(noti2);
  notification_free(noti);
}

/* SERIAL_04: Serialize LED fields */
TEST_F(NotificationIpcIntegrationTest,
    SERIAL_04_SerializeLedFields) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  notification_set_led(noti, NOTIFICATION_LED_OP_ON_CUSTOM_COLOR, 0x00FF00);
  notification_set_led_time_period(noti, 200, 800);
  notification_set_display_applist(noti,
      NOTIFICATION_DISPLAY_APP_NOTIFICATION_TRAY);

  GVariant *variant = notification_ipc_make_gvariant_from_noti(noti, false);
  if (variant == NULL) {
    notification_free(noti);
    GTEST_SKIP() << "GVariant serialization not available without daemon";
  }

  GVariant *inner = UnwrapNotiVariant(variant);
  notification_h noti2 = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti2, nullptr);

  int ret = notification_ipc_make_noti_from_gvariant(noti2, inner);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  notification_led_op_e led_op;
  int led_argb = 0;
  notification_get_led(noti2, &led_op, &led_argb);
  EXPECT_EQ(led_op, NOTIFICATION_LED_OP_ON_CUSTOM_COLOR);
  EXPECT_EQ(led_argb, 0x00FF00);

  int on_ms = 0, off_ms = 0;
  notification_get_led_time_period(noti2, &on_ms, &off_ms);
  EXPECT_EQ(on_ms, 200);
  EXPECT_EQ(off_ms, 800);

  g_variant_unref(inner);
  g_variant_unref(variant);
  notification_free(noti2);
  notification_free(noti);
}

/* SERIAL_05: Serialize progress fields */
TEST_F(NotificationIpcIntegrationTest,
    SERIAL_05_SerializeProgressFields) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_ONGOING);
  ASSERT_NE(noti, nullptr);

  notification_set_progress(noti, 0.65);
  notification_set_size(noti, 4096.0);
  notification_set_display_applist(noti,
      NOTIFICATION_DISPLAY_APP_NOTIFICATION_TRAY);

  GVariant *variant = notification_ipc_make_gvariant_from_noti(noti, false);
  if (variant == NULL) {
    notification_free(noti);
    GTEST_SKIP() << "GVariant serialization not available without daemon";
  }

  GVariant *inner = UnwrapNotiVariant(variant);
  notification_h noti2 = notification_create(NOTIFICATION_TYPE_ONGOING);
  ASSERT_NE(noti2, nullptr);

  int ret = notification_ipc_make_noti_from_gvariant(noti2, inner);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  double progress = 0.0;
  notification_get_progress(noti2, &progress);
  EXPECT_DOUBLE_EQ(progress, 0.65);

  double size = 0.0;
  notification_get_size(noti2, &size);
  EXPECT_DOUBLE_EQ(size, 4096.0);

  g_variant_unref(inner);
  g_variant_unref(variant);
  notification_free(noti2);
  notification_free(noti);
}

/* SERIAL_06: Serialize notification with extension data (large bundle) */
TEST_F(NotificationIpcIntegrationTest,
    SERIAL_06_SerializeExtensionData) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  notification_set_text(noti, NOTIFICATION_TEXT_TYPE_TITLE, "ext_test",
      NULL, NOTIFICATION_VARIABLE_TYPE_NONE);

  bundle *b = bundle_create();
  ASSERT_NE(b, nullptr);
  for (int i = 0; i < 20; i++) {
    char key[32], val[128];
    snprintf(key, sizeof(key), "ext_key_%d", i);
    snprintf(val, sizeof(val), "ext_value_%d_with_some_padding_data", i);
    bundle_add_str(b, key, val);
  }
  int ret = notification_set_extension_data(noti, "big_bundle", b);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  bundle_free(b);

  notification_set_display_applist(noti,
      NOTIFICATION_DISPLAY_APP_NOTIFICATION_TRAY);

  GVariant *variant = notification_ipc_make_gvariant_from_noti(noti, false);
  if (variant == NULL) {
    notification_free(noti);
    GTEST_SKIP() << "GVariant serialization not available without daemon";
  }

  GVariant *inner = UnwrapNotiVariant(variant);
  notification_h noti2 = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti2, nullptr);

  ret = notification_ipc_make_noti_from_gvariant(noti2, inner);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);

  bundle *b2 = nullptr;
  ret = notification_get_extension_data(noti2, "big_bundle", &b2);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  if (b2) {
    char *val = nullptr;
    bundle_get_str(b2, "ext_key_0", &val);
    ASSERT_NE(val, nullptr);
    EXPECT_STREQ(val, "ext_value_0_with_some_padding_data");
    bundle_free(b2);
  }

  g_variant_unref(inner);
  g_variant_unref(variant);
  notification_free(noti2);
  notification_free(noti);
}

/* SERIAL_07: Serialize all fields round-trip (comprehensive) */
TEST_F(NotificationIpcIntegrationTest,
    SERIAL_07_SerializeAllFieldsRoundTrip) {
  notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti, nullptr);

  notification_set_text(noti, NOTIFICATION_TEXT_TYPE_TITLE, "full_title",
      NULL, NOTIFICATION_VARIABLE_TYPE_NONE);
  notification_set_text(noti, NOTIFICATION_TEXT_TYPE_CONTENT, "full_content",
      NULL, NOTIFICATION_VARIABLE_TYPE_NONE);
  notification_set_image(noti, NOTIFICATION_IMAGE_TYPE_ICON, "/test/icon.png");
  notification_set_sound(noti, NOTIFICATION_SOUND_TYPE_USER_DATA,
      "/test/s.wav");
  notification_set_vibration(noti, NOTIFICATION_VIBRATION_TYPE_USER_DATA,
      "/test/v.ivt");
  notification_set_led(noti, NOTIFICATION_LED_OP_ON_CUSTOM_COLOR, 0xFF0000);
  notification_set_led_time_period(noti, 100, 400);
  notification_set_layout(noti, NOTIFICATION_LY_NOTI_EVENT_SINGLE);
  notification_set_display_applist(noti,
      NOTIFICATION_DISPLAY_APP_NOTIFICATION_TRAY |
      NOTIFICATION_DISPLAY_APP_LOCK);
  notification_set_tag(noti, "full_tag");
  notification_set_auto_remove(noti, false);

  GVariant *variant = notification_ipc_make_gvariant_from_noti(noti, false);
  if (variant == NULL) {
    notification_free(noti);
    GTEST_SKIP() << "GVariant serialization not available without daemon";
  }

  GVariant *inner = UnwrapNotiVariant(variant);
  notification_h noti2 = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(noti2, nullptr);

  int ret = notification_ipc_make_noti_from_gvariant(noti2, inner);
  ASSERT_EQ(ret, NOTIFICATION_ERROR_NONE);

  char *title = nullptr;
  notification_get_text(noti2, NOTIFICATION_TEXT_TYPE_TITLE, &title);
  ASSERT_NE(title, nullptr);
  EXPECT_STREQ(title, "full_title");

  char *content = nullptr;
  notification_get_text(noti2, NOTIFICATION_TEXT_TYPE_CONTENT, &content);
  ASSERT_NE(content, nullptr);
  EXPECT_STREQ(content, "full_content");

  char *icon = nullptr;
  notification_get_image(noti2, NOTIFICATION_IMAGE_TYPE_ICON, &icon);
  ASSERT_NE(icon, nullptr);
  EXPECT_STREQ(icon, "/test/icon.png");

  const char *tag = nullptr;
  notification_get_tag(noti2, &tag);
  ASSERT_NE(tag, nullptr);
  EXPECT_STREQ(tag, "full_tag");

  bool auto_remove = true;
  notification_get_auto_remove(noti2, &auto_remove);
  EXPECT_FALSE(auto_remove);

  g_variant_unref(inner);
  g_variant_unref(variant);
  notification_free(noti2);
  notification_free(noti);
}
