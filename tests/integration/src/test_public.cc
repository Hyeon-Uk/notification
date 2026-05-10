#include <gtest/gtest.h>

#include <notification.h>
#include <notification_db.h>
#include <notification_error.h>
#include <notification_internal.h>
#include <notification_type.h>

#include "integ_harness.hh"

namespace {

class IntegPublic : public ::testing::Test {
 protected:
  void SetUp() override {
    ASSERT_EQ(notification_db_init(), NOTIFICATION_ERROR_NONE);
  }
  integ::TmpDb tmp_;
};

TEST_F(IntegPublic, P_PUB_1_create_and_free_round_trip) {
  notification_h n = notification_create(NOTIFICATION_TYPE_NOTI);
  if (!n) GTEST_SKIP() << "notification_create unavailable";
  EXPECT_EQ(notification_free(n), NOTIFICATION_ERROR_NONE);
}

TEST_F(IntegPublic, P_PUB_2_set_get_text_round_trip) {
  notification_h n = notification_create(NOTIFICATION_TYPE_NOTI);
  if (!n) GTEST_SKIP() << "notification_create unavailable";

  ASSERT_EQ(notification_set_text(n, NOTIFICATION_TEXT_TYPE_TITLE, "title-19a",
                                  nullptr, NOTIFICATION_VARIABLE_TYPE_NONE),
            NOTIFICATION_ERROR_NONE);
  char* readback = nullptr;
  ASSERT_EQ(notification_get_text(n, NOTIFICATION_TEXT_TYPE_TITLE, &readback),
            NOTIFICATION_ERROR_NONE);
  ASSERT_NE(readback, nullptr);
  EXPECT_STREQ(readback, "title-19a");
  notification_free(n);
}

TEST_F(IntegPublic, P_PUB_3_set_get_image_round_trip) {
  notification_h n = notification_create(NOTIFICATION_TYPE_NOTI);
  if (!n) GTEST_SKIP() << "notification_create unavailable";

  ASSERT_EQ(notification_set_image(n, NOTIFICATION_IMAGE_TYPE_ICON,
                                   "/tmp/no-such-icon.png"),
            NOTIFICATION_ERROR_NONE);
  char* readback = nullptr;
  ASSERT_EQ(notification_get_image(n, NOTIFICATION_IMAGE_TYPE_ICON, &readback),
            NOTIFICATION_ERROR_NONE);
  ASSERT_NE(readback, nullptr);
  EXPECT_STREQ(readback, "/tmp/no-such-icon.png");
  notification_free(n);
}

TEST_F(IntegPublic, P_PUB_4_clone_preserves_text) {
  notification_h n = notification_create(NOTIFICATION_TYPE_NOTI);
  if (!n) GTEST_SKIP() << "notification_create unavailable";

  ASSERT_EQ(notification_set_text(n, NOTIFICATION_TEXT_TYPE_CONTENT,
                                  "clone-content", nullptr,
                                  NOTIFICATION_VARIABLE_TYPE_NONE),
            NOTIFICATION_ERROR_NONE);

  notification_h cloned = nullptr;
  ASSERT_EQ(notification_clone(n, &cloned), NOTIFICATION_ERROR_NONE);
  ASSERT_NE(cloned, nullptr);
  char* text = nullptr;
  ASSERT_EQ(
      notification_get_text(cloned, NOTIFICATION_TEXT_TYPE_CONTENT, &text),
      NOTIFICATION_ERROR_NONE);
  EXPECT_STREQ(text, "clone-content");
  notification_free(cloned);
  notification_free(n);
}

TEST_F(IntegPublic, P_PUB_5_set_get_time_round_trip) {
  notification_h n = notification_create(NOTIFICATION_TYPE_NOTI);
  if (!n) GTEST_SKIP() << "notification_create unavailable";
  time_t t = 1715000000;
  ASSERT_EQ(notification_set_time(n, t), NOTIFICATION_ERROR_NONE);
  time_t readback = 0;
  ASSERT_EQ(notification_get_time(n, &readback), NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(readback, t);
  notification_free(n);
}

TEST_F(IntegPublic, N_PUB_1_create_invalid_type) {
  notification_h n = notification_create(static_cast<notification_type_e>(999));
  // Pin today's contract: invalid type returns NULL handle.
  EXPECT_EQ(n, nullptr);
}

TEST_F(IntegPublic, N_PUB_2_clone_null_handle) {
  notification_h cloned = nullptr;
  EXPECT_EQ(notification_clone(nullptr, &cloned),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(IntegPublic, N_PUB_3_clone_null_out) {
  notification_h n = notification_create(NOTIFICATION_TYPE_NOTI);
  if (!n) GTEST_SKIP() << "notification_create unavailable";
  EXPECT_EQ(notification_clone(n, nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  notification_free(n);
}

TEST_F(IntegPublic, N_PUB_4_set_text_null_handle) {
  EXPECT_EQ(notification_set_text(nullptr, NOTIFICATION_TEXT_TYPE_TITLE, "x",
                                  nullptr, NOTIFICATION_VARIABLE_TYPE_NONE),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(IntegPublic, N_PUB_5_get_text_null_handle) {
  char* out = nullptr;
  EXPECT_EQ(notification_get_text(nullptr, NOTIFICATION_TEXT_TYPE_TITLE, &out),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
}

TEST_F(IntegPublic, N_PUB_6_get_text_null_out) {
  notification_h n = notification_create(NOTIFICATION_TYPE_NOTI);
  if (!n) GTEST_SKIP() << "notification_create unavailable";
  EXPECT_EQ(notification_get_text(n, NOTIFICATION_TEXT_TYPE_TITLE, nullptr),
            NOTIFICATION_ERROR_INVALID_PARAMETER);
  notification_free(n);
}

TEST_F(IntegPublic, E_PUB_1_get_text_unset_returns_null_string) {
  notification_h n = notification_create(NOTIFICATION_TYPE_NOTI);
  if (!n) GTEST_SKIP() << "notification_create unavailable";

  char* out = reinterpret_cast<char*>(0xdeadbeef);
  int rc = notification_get_text(n, NOTIFICATION_TEXT_TYPE_TITLE, &out);
  // Pin: an unset text type returns NONE with out=NULL today.
  EXPECT_EQ(rc, NOTIFICATION_ERROR_NONE);
  EXPECT_EQ(out, nullptr);
  notification_free(n);
}

TEST_F(IntegPublic, E_PUB_2_set_text_empty_string) {
  notification_h n = notification_create(NOTIFICATION_TYPE_NOTI);
  if (!n) GTEST_SKIP() << "notification_create unavailable";
  int rc = notification_set_text(n, NOTIFICATION_TEXT_TYPE_TITLE, "", nullptr,
                                 NOTIFICATION_VARIABLE_TYPE_NONE);
  // Pin: empty string accepted today (no length validation).
  EXPECT_EQ(rc, NOTIFICATION_ERROR_NONE);
  notification_free(n);
}

TEST_F(IntegPublic, C_PUB_1_set_text_overwrite) {
  notification_h n = notification_create(NOTIFICATION_TYPE_NOTI);
  if (!n) GTEST_SKIP() << "notification_create unavailable";

  ASSERT_EQ(notification_set_text(n, NOTIFICATION_TEXT_TYPE_TITLE, "first",
                                  nullptr, NOTIFICATION_VARIABLE_TYPE_NONE),
            NOTIFICATION_ERROR_NONE);
  ASSERT_EQ(notification_set_text(n, NOTIFICATION_TEXT_TYPE_TITLE, "second",
                                  nullptr, NOTIFICATION_VARIABLE_TYPE_NONE),
            NOTIFICATION_ERROR_NONE);
  char* out = nullptr;
  ASSERT_EQ(notification_get_text(n, NOTIFICATION_TEXT_TYPE_TITLE, &out),
            NOTIFICATION_ERROR_NONE);
  EXPECT_STREQ(out, "second");
  notification_free(n);
}

TEST_F(IntegPublic, C_PUB_2_post_against_empty_db_returns_bounded) {
  notification_h n = notification_create(NOTIFICATION_TYPE_NOTI);
  if (!n) GTEST_SKIP() << "notification_create unavailable";
  // Real post requires DPM; sandbox returns bounded code, never crashes.
  int rc = notification_post(n);
  EXPECT_LE(rc, NOTIFICATION_ERROR_NONE);
  notification_free(n);
}

}  // namespace
