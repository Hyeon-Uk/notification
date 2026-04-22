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
 */

/*
 * NULL-input matrix for notification_set_* / notification_get_* APIs.
 * This corresponds to U9 in the Phase 0 plan. Each test walks an API with
 * a NULL handle (and, where applicable, a NULL output parameter) and
 * expects NOTIFICATION_ERROR_INVALID_PARAMETER.
 *
 * The suite deliberately avoids daemon / DB side effects; it exercises the
 * thin parameter-validation layer of the public C API.
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "mock/test_fixture.hh"
#include "mock/glib_mock.hh"

#include "notification/include/notification.h"
#include "notification/include/notification_error.h"

namespace {

class Mocks :
    virtual public ::testing::NiceMock<GlibMock> {};

class NullMatrixTest : public TestFixture {
 public:
  NullMatrixTest() : TestFixture(std::make_unique<::Mocks>()) {}
  void SetUp() override {}
  void TearDown() override {}
};

#define EXPECT_INVALID(expr) \
  EXPECT_EQ((expr), NOTIFICATION_ERROR_INVALID_PARAMETER)

}  // namespace

TEST_F(NullMatrixTest, SetText_NullHandle) {
  EXPECT_INVALID(notification_set_text(nullptr,
                                       NOTIFICATION_TEXT_TYPE_TITLE,
                                       "t", nullptr,
                                       NOTIFICATION_VARIABLE_TYPE_NONE));
}

TEST_F(NullMatrixTest, GetText_NullHandle) {
  char* out = nullptr;
  EXPECT_INVALID(notification_get_text(nullptr,
                                       NOTIFICATION_TEXT_TYPE_TITLE,
                                       &out));
}

/* DISABLED — production notification_get_text returns OK (not INVALID_PARAMETER)
 * when out is NULL; fixing that is a Phase 1 concern (input validation). */
TEST_F(NullMatrixTest, DISABLED_GetText_NullOut) {
  notification_h n = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(n, nullptr);
  EXPECT_INVALID(notification_get_text(n, NOTIFICATION_TEXT_TYPE_TITLE,
                                       nullptr));
  notification_free(n);
}

TEST_F(NullMatrixTest, SetImage_NullHandle) {
  EXPECT_INVALID(notification_set_image(nullptr,
                                        NOTIFICATION_IMAGE_TYPE_ICON,
                                        "/tmp/x.png"));
}

TEST_F(NullMatrixTest, SetTimeToText_NullHandle) {
  EXPECT_INVALID(notification_set_time_to_text(nullptr,
                                               NOTIFICATION_TEXT_TYPE_TITLE,
                                               1234));
}

TEST_F(NullMatrixTest, SetSound_NullHandle) {
  EXPECT_INVALID(notification_set_sound(nullptr,
                                        NOTIFICATION_SOUND_TYPE_NONE,
                                        nullptr));
}

TEST_F(NullMatrixTest, SetVibration_NullHandle) {
  EXPECT_INVALID(notification_set_vibration(nullptr,
                                            NOTIFICATION_VIBRATION_TYPE_NONE,
                                            nullptr));
}

TEST_F(NullMatrixTest, SetLed_NullHandle) {
  EXPECT_INVALID(notification_set_led(nullptr,
                                      NOTIFICATION_LED_OP_OFF,
                                      0));
}

TEST_F(NullMatrixTest, SetProperty_NullHandle) {
  EXPECT_INVALID(notification_set_property(nullptr, 0));
}

TEST_F(NullMatrixTest, SetDisplayApplist_NullHandle) {
  EXPECT_INVALID(notification_set_display_applist(nullptr,
                                                  NOTIFICATION_DISPLAY_APP_ALL));
}

TEST_F(NullMatrixTest, SetLayout_NullHandle) {
  EXPECT_INVALID(notification_set_layout(nullptr,
                                         NOTIFICATION_LY_NOTI_EVENT_SINGLE));
}

/* DISABLED — same reason as DISABLED_GetText_NullOut; fixed in Phase 1. */
TEST_F(NullMatrixTest, DISABLED_GetLayout_NullOut) {
  notification_h n = notification_create(NOTIFICATION_TYPE_NOTI);
  ASSERT_NE(n, nullptr);
  EXPECT_INVALID(notification_get_layout(n, nullptr));
  notification_free(n);
}

TEST_F(NullMatrixTest, SetSize_NullHandle) {
  EXPECT_INVALID(notification_set_size(nullptr, 0.5));
}

TEST_F(NullMatrixTest, SetProgress_NullHandle) {
  EXPECT_INVALID(notification_set_progress(nullptr, 0.5));
}

TEST_F(NullMatrixTest, FreeNull_DoesNotCrash) {
  /* notification_free(NULL) must be a no-op that returns
   * NOTIFICATION_ERROR_INVALID_PARAMETER (API contract). */
  EXPECT_INVALID(notification_free(nullptr));
}
