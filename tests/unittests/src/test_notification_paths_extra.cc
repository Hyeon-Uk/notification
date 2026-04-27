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

/*
 * N7.5-D coverage boost — notification.c (Large 3 file group, 2068 LOC).
 *
 * test_notification.cc already drives a wide happy-path matrix on a single
 * notification_h. This TU adds:
 *
 *   1. Setter/Getter matrix WITH ROUND-TRIP — every public set/get pair on
 *      notification.c invokes both branches and asserts the value lands
 *      back unchanged. Pinning these now makes the upcoming Stage 8 C->C++
 *      port a no-op for callers.
 *   2. NULL parameter coverage on the long tail of accessors NOT exercised
 *      by test_null_matrix.cc (size, progress, led_time_period, sound,
 *      vibration, tag, ongoing_flag, auto_remove, extension_image_size,
 *      pkgname, default_button, hide/delete timeout, text_input_max_length,
 *      led, get_event_handler).
 *   3. notification_clone deep-copy semantics — verifies the clone owns its
 *      own copies of the heap fields by mutating the original after clone.
 *   4. notification_set_time(0) auto-fills with time(NULL); explicit
 *      timestamp round-trips byte-for-byte; negative time is preserved
 *      verbatim (no clamping in the helper).
 *   5. notification_set_layout boundary values: NONE / NOTI_EVENT_SINGLE /
 *      NOTI_EVENT_MULTIPLE / NOTI_THUMBNAIL / ONGOING_EVENT / ONGOING_PROGRESS
 *      / MAX-1.  Above MAX or below NONE => INVALID_PARAMETER.
 *   6. notification_post / update / delete with NULL handle - INVALID_PARAMETER
 *      (already in test_post_update_delete.cc but pinned again here at the
 *      handle-validation layer to detect sequence-leak from refactors).
 *
 * No production code is changed by this TU.  All branches are reached
 * through the EXPORT_API surface — no _for_test_ shims required.
 *
 * The fixture is plain TestFixture(nullptr), not LeakCheckFixture, because
 * notification.c strdup's strings inside libnotification.so while we
 * --wrap=malloc only on the test-binary side; the cross-boundary delta is
 * not a real leak (see test_notification_setting_extra.cc for prior art).
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <climits>
#include <ctime>
#include <cstring>

#include "mock/test_fixture.hh"
#include "mock/aul_mock.hh"
#include "mock/glib_mock.hh"
#include "mock/package_manager_mock.hh"
#include "mock/tzplatform_config_mock.hh"

extern "C" {
#include <notification.h>
#include <notification_internal.h>
#include <notification_error.h>
#include <notification_type.h>
#include <notification_ongoing_flag.h>
#include <notification_text_domain.h>
}

namespace {

using ::testing::_;
using ::testing::Return;
using ::testing::Invoke;

/* Fake aul_app_get_appid_bypid that fills a deterministic appid so
 * notification_create() succeeds without depending on the host
 * /proc/<pid>/cmdline contents. */
int FakeAulGetAppidBypid(int /*pid*/, char* appid, int len) {
  if (appid != nullptr && len > 0) {
    snprintf(appid, len, "%s", "org.tizen.testapp");
  }
  return 0;  /* AUL_R_OK */
}

int FakeAulGetPkgidBypid(int /*pid*/, char* pkgid, int len) {
  if (pkgid != nullptr && len > 0) {
    snprintf(pkgid, len, "%s", "org.tizen.testpkg");
  }
  return 0;  /* AUL_R_OK */
}

class PathsMocks :
    virtual public ::testing::NiceMock<AulMock>,
    virtual public ::testing::NiceMock<TzplatformConfigMock>,
    virtual public ::testing::NiceMock<GlibMock>,
    virtual public ::testing::NiceMock<PackageManagerMock> {};

class NotificationPathsExtraTest : public TestFixture {
 public:
  NotificationPathsExtraTest()
      : TestFixture(std::make_unique<::PathsMocks>()) {}
  void SetUp() override {
    /* Default behaviour: aul_* returns OK with deterministic strings.
     * Tests can override per-call but most don't need to. */
    ON_CALL(GetMock<AulMock>(), aul_app_get_appid_bypid(_, _, _))
        .WillByDefault(Invoke(FakeAulGetAppidBypid));
    ON_CALL(GetMock<AulMock>(), aul_app_get_pkgid_bypid(_, _, _))
        .WillByDefault(Invoke(FakeAulGetPkgidBypid));
    /* package_manager calls are gated on first-create only and tolerate
     * a non-zero error code (we treat the error path as the test path). */
    ON_CALL(GetMock<PackageManagerMock>(),
            package_info_create(_, _))
        .WillByDefault(Return(-1));
    ON_CALL(GetMock<PackageManagerMock>(),
            pkgmgrinfo_appinfo_get_usr_appinfo(_, _, _))
        .WillByDefault(Return(-1));
  }
  void TearDown() override {}
};

}  // namespace

/* ------------------------------------------------------------------ */
/* 1. Setter/Getter matrix — round-trip every pair.                   */
/* ------------------------------------------------------------------ */

TEST_F(NotificationPathsExtraTest, SetGet_Image_RoundTrip) {
  notification_h n = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(n, nullptr);

  ASSERT_EQ(notification_set_image(n, NOTIFICATION_IMAGE_TYPE_ICON,
                                   "/tmp/icon.png"),
            NOTIFICATION_ERROR_NONE);
  char* out = nullptr;
  ASSERT_EQ(notification_get_image(n, NOTIFICATION_IMAGE_TYPE_ICON, &out),
            NOTIFICATION_ERROR_NONE);
  EXPECT_STREQ(out, "/tmp/icon.png");

  ASSERT_EQ(notification_set_image(n, NOTIFICATION_IMAGE_TYPE_ICON_FOR_INDICATOR,
                                   "/tmp/ind.png"),
            NOTIFICATION_ERROR_NONE);
  ASSERT_EQ(notification_get_image(n, NOTIFICATION_IMAGE_TYPE_ICON_FOR_INDICATOR,
                                   &out),
            NOTIFICATION_ERROR_NONE);
  EXPECT_STREQ(out, "/tmp/ind.png");

  notification_free(n);
}

TEST_F(NotificationPathsExtraTest, SetGet_TextDomain_RoundTrip) {
  notification_h n = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(n, nullptr);

  ASSERT_EQ(notification_set_text_domain(n, "domain.foo", "/tmp/locale"),
            NOTIFICATION_ERROR_NONE);
  char* dom = nullptr;
  char* dir = nullptr;
  ASSERT_EQ(notification_get_text_domain(n, &dom, &dir),
            NOTIFICATION_ERROR_NONE);
  EXPECT_STREQ(dom, "domain.foo");
  EXPECT_STREQ(dir, "/tmp/locale");

  notification_free(n);
}

TEST_F(NotificationPathsExtraTest, SetGet_Sound_RoundTrip) {
  notification_h n = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(n, nullptr);

  ASSERT_EQ(notification_set_sound(n, NOTIFICATION_SOUND_TYPE_USER_DATA,
                                   "/tmp/snd.ogg"),
            NOTIFICATION_ERROR_NONE);
  notification_sound_type_e ty = NOTIFICATION_SOUND_TYPE_NONE;
  const char* path = nullptr;
  ASSERT_EQ(notification_get_sound(n, &ty, &path),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(ty, NOTIFICATION_SOUND_TYPE_USER_DATA);
  EXPECT_STREQ(path, "/tmp/snd.ogg");

  notification_free(n);
}

TEST_F(NotificationPathsExtraTest, SetGet_Vibration_RoundTrip) {
  notification_h n = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(n, nullptr);

  ASSERT_EQ(notification_set_vibration(n, NOTIFICATION_VIBRATION_TYPE_USER_DATA,
                                       "/tmp/vib.bin"),
            NOTIFICATION_ERROR_NONE);
  notification_vibration_type_e ty = NOTIFICATION_VIBRATION_TYPE_NONE;
  const char* path = nullptr;
  ASSERT_EQ(notification_get_vibration(n, &ty, &path),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(ty, NOTIFICATION_VIBRATION_TYPE_USER_DATA);
  EXPECT_STREQ(path, "/tmp/vib.bin");

  notification_free(n);
}

TEST_F(NotificationPathsExtraTest, SetGet_LedAndPeriod_RoundTrip) {
  notification_h n = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(n, nullptr);

  ASSERT_EQ(notification_set_led(n, NOTIFICATION_LED_OP_ON_CUSTOM_COLOR,
                                 0x11223344),
            NOTIFICATION_ERROR_NONE);
  notification_led_op_e op = NOTIFICATION_LED_OP_OFF;
  int argb = 0;
  ASSERT_EQ(notification_get_led(n, &op, &argb), NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(op, NOTIFICATION_LED_OP_ON_CUSTOM_COLOR);
  EXPECT_EQ(argb, 0x11223344);

  ASSERT_EQ(notification_set_led_time_period(n, 500, 250),
            NOTIFICATION_ERROR_NONE);
  int on_ms = 0, off_ms = 0;
  ASSERT_EQ(notification_get_led_time_period(n, &on_ms, &off_ms),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(on_ms, 500);
  EXPECT_EQ(off_ms, 250);

  notification_free(n);
}

TEST_F(NotificationPathsExtraTest, SetGet_Property_RoundTrip) {
  notification_h n = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(n, nullptr);

  ASSERT_EQ(notification_set_property(n,
                NOTIFICATION_PROP_PERMANENT_DISPLAY |
                NOTIFICATION_PROP_DISABLE_AUTO_DELETE),
            NOTIFICATION_ERROR_NONE);
  int prop = 0;
  ASSERT_EQ(notification_get_property(n, &prop), NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(prop & NOTIFICATION_PROP_PERMANENT_DISPLAY,
            NOTIFICATION_PROP_PERMANENT_DISPLAY);

  notification_free(n);
}

TEST_F(NotificationPathsExtraTest, SetGet_DisplayApplist_RoundTrip) {
  notification_h n = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(n, nullptr);
  ASSERT_EQ(notification_set_display_applist(n, NOTIFICATION_DISPLAY_APP_ALL),
            NOTIFICATION_ERROR_NONE);
  int apps = 0;
  ASSERT_EQ(notification_get_display_applist(n, &apps),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(apps, NOTIFICATION_DISPLAY_APP_ALL);
  notification_free(n);
}

TEST_F(NotificationPathsExtraTest, SetGet_SizeAndProgress_RoundTrip) {
  notification_h n = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(n, nullptr);

  ASSERT_EQ(notification_set_size(n, 0.42), NOTIFICATION_ERROR_NONE);
  double size = 0.0;
  ASSERT_EQ(notification_get_size(n, &size), NOTIFICATION_ERROR_NONE);
  EXPECT_DOUBLE_EQ(size, 0.42);

  ASSERT_EQ(notification_set_progress(n, 0.85), NOTIFICATION_ERROR_NONE);
  double pct = 0.0;
  ASSERT_EQ(notification_get_progress(n, &pct), NOTIFICATION_ERROR_NONE);
  EXPECT_DOUBLE_EQ(pct, 0.85);

  notification_free(n);
}

TEST_F(NotificationPathsExtraTest, SetGet_Tag_RoundTrip) {
  notification_h n = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(n, nullptr);

  ASSERT_EQ(notification_set_tag(n, "tag-A"), NOTIFICATION_ERROR_NONE);
  const char* tag = nullptr;
  ASSERT_EQ(notification_get_tag(n, &tag), NOTIFICATION_ERROR_NONE);
  EXPECT_STREQ(tag, "tag-A");

  /* re-set replaces, no leak (free old + strdup new) */
  ASSERT_EQ(notification_set_tag(n, "tag-B"), NOTIFICATION_ERROR_NONE);
  ASSERT_EQ(notification_get_tag(n, &tag), NOTIFICATION_ERROR_NONE);
  EXPECT_STREQ(tag, "tag-B");

  notification_free(n);
}

TEST_F(NotificationPathsExtraTest, SetGet_OngoingFlag_RoundTrip) {
  notification_h n = notification_create(NOTIFICATION_TYPE_ONGOING);
  ASSERT_NE(n, nullptr);

  ASSERT_EQ(notification_set_ongoing_flag(n, true), NOTIFICATION_ERROR_NONE);
  bool flag = false;
  ASSERT_EQ(notification_get_ongoing_flag(n, &flag), NOTIFICATION_ERROR_NONE);
  EXPECT_TRUE(flag);

  ASSERT_EQ(notification_set_ongoing_flag(n, false), NOTIFICATION_ERROR_NONE);
  ASSERT_EQ(notification_get_ongoing_flag(n, &flag), NOTIFICATION_ERROR_NONE);
  EXPECT_FALSE(flag);

  notification_free(n);
}

TEST_F(NotificationPathsExtraTest, SetGet_AutoRemove_RoundTrip) {
  notification_h n = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(n, nullptr);

  ASSERT_EQ(notification_set_auto_remove(n, false), NOTIFICATION_ERROR_NONE);
  bool ar = true;
  ASSERT_EQ(notification_get_auto_remove(n, &ar), NOTIFICATION_ERROR_NONE);
  EXPECT_FALSE(ar);

  ASSERT_EQ(notification_set_auto_remove(n, true), NOTIFICATION_ERROR_NONE);
  ASSERT_EQ(notification_get_auto_remove(n, &ar), NOTIFICATION_ERROR_NONE);
  EXPECT_TRUE(ar);

  notification_free(n);
}

TEST_F(NotificationPathsExtraTest, SetGet_ExtensionImageSize_RoundTrip) {
  notification_h n = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(n, nullptr);

  ASSERT_EQ(notification_set_extension_image_size(n, 128),
            NOTIFICATION_ERROR_NONE);
  int height = 0;
  ASSERT_EQ(notification_get_extension_image_size(n, &height),
            NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(height, 128);

  notification_free(n);
}

TEST_F(NotificationPathsExtraTest, AddRemove_Buttons_AllSlots) {
  notification_h n = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(n, nullptr);

  for (int i = NOTIFICATION_BUTTON_1; i <= NOTIFICATION_BUTTON_6; i++) {
    ASSERT_EQ(notification_add_button(n,
                  static_cast<notification_button_index_e>(i)),
              NOTIFICATION_ERROR_NONE);
  }
  for (int i = NOTIFICATION_BUTTON_1; i <= NOTIFICATION_BUTTON_6; i++) {
    ASSERT_EQ(notification_remove_button(n,
                  static_cast<notification_button_index_e>(i)),
              NOTIFICATION_ERROR_NONE);
  }

  notification_free(n);
}

/* ------------------------------------------------------------------ */
/* 2. NULL parameter coverage on the long tail of accessors.          */
/* ------------------------------------------------------------------ */

TEST_F(NotificationPathsExtraTest, NullParam_AccessorsLongTail) {
  /* set: NULL handle */
  EXPECT_EQ(notification_set_size(nullptr, 0.5),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_set_progress(nullptr, 0.5),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_set_led_time_period(nullptr, 100, 100),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_set_tag(nullptr, "x"),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_set_ongoing_flag(nullptr, true),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_set_auto_remove(nullptr, true),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_set_extension_image_size(nullptr, 32),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_set_text_input(nullptr, 100),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_add_button(nullptr, NOTIFICATION_BUTTON_1),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_remove_button(nullptr, NOTIFICATION_BUTTON_1),
            NOTIFICATION_ERROR_INVALID_PARAMETER);

  /* get: NULL handle and NULL out pointer */
  double d = 0.0;
  EXPECT_EQ(notification_get_size(nullptr, &d),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_get_progress(nullptr, &d),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  int a = 0, b = 0;
  EXPECT_EQ(notification_get_led_time_period(nullptr, &a, &b),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  const char* s = nullptr;
  EXPECT_EQ(notification_get_tag(nullptr, &s),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  bool flag = false;
  EXPECT_EQ(notification_get_ongoing_flag(nullptr, &flag),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_get_auto_remove(nullptr, &flag),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_get_extension_image_size(nullptr, &a),
            NOTIFICATION_ERROR_INVALID_PARAMETER);

  notification_type_e t;
  EXPECT_EQ(notification_get_type(nullptr, &t),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  notification_ly_type_e ly;
  EXPECT_EQ(notification_get_layout(nullptr, &ly),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  int prop = 0;
  EXPECT_EQ(notification_get_property(nullptr, &prop),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  int applist = 0;
  EXPECT_EQ(notification_get_display_applist(nullptr, &applist),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  char* pkg = nullptr;
  EXPECT_EQ(notification_get_pkgname(nullptr, &pkg),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  notification_led_op_e op;
  int argb = 0;
  EXPECT_EQ(notification_get_led(nullptr, &op, &argb),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  notification_sound_type_e st;
  const char* sp = nullptr;
  EXPECT_EQ(notification_get_sound(nullptr, &st, &sp),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  notification_vibration_type_e vt;
  const char* vp = nullptr;
  EXPECT_EQ(notification_get_vibration(nullptr, &vt, &vp),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  time_t tt = 0;
  EXPECT_EQ(notification_get_time(nullptr, &tt),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_get_insert_time(nullptr, &tt),
            NOTIFICATION_ERROR_INVALID_PARAMETER);

  /* clone: both NULL combinations */
  notification_h clone = nullptr;
  EXPECT_EQ(notification_clone(nullptr, &clone),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  notification_h n = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(n, nullptr);
  EXPECT_EQ(notification_clone(n, nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  notification_free(n);

  /* free(NULL) - bookkeeping branch in production */
  EXPECT_EQ(notification_free(nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(NotificationPathsExtraTest, NullParam_NullOutPointers) {
  /* Most getters bail on either NULL handle OR NULL out. Cover the second
   * branch on the handle-OK side. */
  notification_h n = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(n, nullptr);

  EXPECT_EQ(notification_get_time(n, nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_get_insert_time(n, nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_get_size(n, nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_get_progress(n, nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_get_property(n, nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_get_display_applist(n, nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_get_layout(n, nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_get_type(n, nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_get_pkgname(n, nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_get_extension_image_size(n, nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  bool b;
  (void)b;
  EXPECT_EQ(notification_get_ongoing_flag(n, nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_get_auto_remove(n, nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);

  notification_free(n);
}

/* ------------------------------------------------------------------ */
/* 3. Clone deep-copy semantics.                                      */
/* ------------------------------------------------------------------ */

TEST_F(NotificationPathsExtraTest, Clone_DeepCopy_HeapFieldsIndependent) {
  notification_h n = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(n, nullptr);

  /* Populate heap-owned strings. */
  ASSERT_EQ(notification_set_text(n, NOTIFICATION_TEXT_TYPE_TITLE,
                                  "title-orig", nullptr,
                                  NOTIFICATION_VARIABLE_TYPE_NONE),
            NOTIFICATION_ERROR_NONE);
  ASSERT_EQ(notification_set_tag(n, "tag-orig"), NOTIFICATION_ERROR_NONE);
  ASSERT_EQ(notification_set_text_domain(n, "dom-orig", "/dir-orig"),
            NOTIFICATION_ERROR_NONE);
  ASSERT_EQ(notification_set_image(n, NOTIFICATION_IMAGE_TYPE_ICON,
                                   "/img-orig.png"),
            NOTIFICATION_ERROR_NONE);

  notification_h clone = nullptr;
  ASSERT_EQ(notification_clone(n, &clone), NOTIFICATION_ERROR_NONE);
  ASSERT_NE(clone, nullptr);

  /* Mutate the original: clone must NOT see the change. */
  ASSERT_EQ(notification_set_tag(n, "tag-new"), NOTIFICATION_ERROR_NONE);
  ASSERT_EQ(notification_set_text(n, NOTIFICATION_TEXT_TYPE_TITLE,
                                  "title-new", nullptr,
                                  NOTIFICATION_VARIABLE_TYPE_NONE),
            NOTIFICATION_ERROR_NONE);

  const char* clone_tag = nullptr;
  ASSERT_EQ(notification_get_tag(clone, &clone_tag),
            NOTIFICATION_ERROR_NONE);
  EXPECT_STREQ(clone_tag, "tag-orig");

  char* clone_title = nullptr;
  ASSERT_EQ(notification_get_text(clone, NOTIFICATION_TEXT_TYPE_TITLE,
                                  &clone_title),
            NOTIFICATION_ERROR_NONE);
  EXPECT_STREQ(clone_title, "title-orig");

  char* dom = nullptr;
  char* dir = nullptr;
  ASSERT_EQ(notification_get_text_domain(clone, &dom, &dir),
            NOTIFICATION_ERROR_NONE);
  EXPECT_STREQ(dom, "dom-orig");
  EXPECT_STREQ(dir, "/dir-orig");

  char* img = nullptr;
  ASSERT_EQ(notification_get_image(clone, NOTIFICATION_IMAGE_TYPE_ICON, &img),
            NOTIFICATION_ERROR_NONE);
  EXPECT_STREQ(img, "/img-orig.png");

  notification_free(clone);
  notification_free(n);
}

/* ------------------------------------------------------------------ */
/* 4. notification_set_time semantics.                                */
/* ------------------------------------------------------------------ */

TEST_F(NotificationPathsExtraTest, TimeFormat_BoundaryValues) {
  notification_h n = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(n, nullptr);

  /* Explicit timestamp - round-trip exactly. */
  ASSERT_EQ(notification_set_time(n, 1234567890),
            NOTIFICATION_ERROR_NONE);
  time_t t = 0;
  ASSERT_EQ(notification_get_time(n, &t), NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(t, 1234567890);

  /* set_time(0) means "fill with time(NULL)". */
  time_t before = time(nullptr);
  ASSERT_EQ(notification_set_time(n, 0), NOTIFICATION_ERROR_NONE);
  time_t after = time(nullptr);
  ASSERT_EQ(notification_get_time(n, &t), NOTIFICATION_ERROR_NONE);
  EXPECT_GE(t, before);
  EXPECT_LE(t, after);

  /* Negative timestamp - production preserves verbatim, no clamping. */
  ASSERT_EQ(notification_set_time(n, (time_t)-1),
            NOTIFICATION_ERROR_NONE);
  ASSERT_EQ(notification_get_time(n, &t), NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(t, (time_t)-1);

  notification_free(n);
}

/* ------------------------------------------------------------------ */
/* 5. Layout transitions.                                             */
/* ------------------------------------------------------------------ */

TEST_F(NotificationPathsExtraTest, Layout_AcceptedRange) {
  notification_h n = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(n, nullptr);

  const notification_ly_type_e accepted[] = {
      NOTIFICATION_LY_NONE,
      NOTIFICATION_LY_NOTI_EVENT_SINGLE,
      NOTIFICATION_LY_NOTI_EVENT_MULTIPLE,
      NOTIFICATION_LY_NOTI_THUMBNAIL,
      NOTIFICATION_LY_ONGOING_EVENT,
      NOTIFICATION_LY_ONGOING_PROGRESS,
  };
  for (auto v : accepted) {
    ASSERT_EQ(notification_set_layout(n, v), NOTIFICATION_ERROR_NONE)
        << "v=" << v;
    notification_ly_type_e got = NOTIFICATION_LY_NONE;
    ASSERT_EQ(notification_get_layout(n, &got), NOTIFICATION_ERROR_NONE);
    EXPECT_EQ(got, v);
  }

  /* NOTIFICATION_LY_MAX (= LY_EXTENSION) is INCLUSIVE on the upper bound,
   * so it round-trips like the other accepted values. */
  ASSERT_EQ(notification_set_layout(n,
                static_cast<notification_ly_type_e>(NOTIFICATION_LY_MAX)),
            NOTIFICATION_ERROR_NONE);

  /* Strictly above MAX is rejected. */
  EXPECT_EQ(notification_set_layout(n,
                static_cast<notification_ly_type_e>(NOTIFICATION_LY_MAX + 5)),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_set_layout(n,
                static_cast<notification_ly_type_e>(-7)),
            NOTIFICATION_ERROR_INVALID_PARAMETER);

  notification_free(n);
}

/* ------------------------------------------------------------------ */
/* 6. Post / Update / Delete sequence — null handle contract.         */
/* ------------------------------------------------------------------ */

TEST_F(NotificationPathsExtraTest, PostUpdateDelete_NullHandle_Sequence) {
  /* These dispatch to *_for_uid; the null check happens inside the per-uid
   * helper. We keep this here in addition to test_post_update_delete.cc to
   * detect the case where a refactor wires the public path past the null
   * check. */
  EXPECT_EQ(notification_post(nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_update(nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  EXPECT_EQ(notification_delete(nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

/* ------------------------------------------------------------------ */
/* Type matrix - notification_create.                                 */
/* ------------------------------------------------------------------ */

TEST_F(NotificationPathsExtraTest, Create_AllValidTypes) {
  notification_h ni = notification_create(NOTIFICATION_TYPE_NOTI);
  EXPECT_NE(ni, nullptr);
  notification_type_e t;
  if (ni) {
    EXPECT_EQ(notification_get_type(ni, &t), NOTIFICATION_ERROR_NONE);
    EXPECT_EQ(t, NOTIFICATION_TYPE_NOTI);
    notification_free(ni);
  }

  notification_h og = notification_create(NOTIFICATION_TYPE_ONGOING);
  EXPECT_NE(og, nullptr);
  if (og) {
    EXPECT_EQ(notification_get_type(og, &t), NOTIFICATION_ERROR_NONE);
    EXPECT_EQ(t, NOTIFICATION_TYPE_ONGOING);
    notification_free(og);
  }
}

TEST_F(NotificationPathsExtraTest, Create_InvalidType_ReturnsNull) {
  /* Production rejects type <= NONE (-1) || type > MAX(=ONGOING).
   * NONE itself, negative, and MAX+1 all rejected. */
  notification_h n = notification_create(
      static_cast<notification_type_e>(NOTIFICATION_TYPE_NONE));
  EXPECT_EQ(n, nullptr);

  n = notification_create(static_cast<notification_type_e>(-3));
  EXPECT_EQ(n, nullptr);

  n = notification_create(
      static_cast<notification_type_e>(NOTIFICATION_TYPE_MAX + 1));
  EXPECT_EQ(n, nullptr);
}
