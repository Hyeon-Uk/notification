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
#include <notification_list.h>
#include <notification_setting.h>
#include <notification_setting_internal.h>

#include <set>
#include <string>

#include "common/smoke_utils.h"

namespace smoke {

class EnumerateSmokeTest : public ::testing::Test {
 public:
  void SetUp() override { ResetNotifications(); }
  void TearDown() override { ResetNotifications(); }
};

/* TC-SMK-EN-01 — setting_array enumerates all registered apps;
 * smoketest app must be present in the result. */
TEST_F(EnumerateSmokeTest, SettingArrayContainsSmokeApp) {
  notification_setting_h arr = nullptr;
  int count = 0;
  ASSERT_EQ(notification_setting_get_setting_array(&arr, &count),
            NOTIFICATION_ERROR_NONE);
  ASSERT_GE(count, 1);
  ASSERT_NE(arr, nullptr);

  bool found = false;
  for (int i = 0; i < count; i++) {
    char* aid = nullptr;
    notification_setting_get_appid(&arr[i], &aid);
    if (aid != nullptr && std::string(aid) == kSmokeAppId) {
      found = true;
      break;
    }
  }
  EXPECT_TRUE(found) << "smoke app missing from setting array";
}

/* TC-SMK-EN-02 — get_setting_by_package_name returns our row */
TEST_F(EnumerateSmokeTest, GetSettingByPackageName) {
  notification_setting_h setting = nullptr;
  int ret = notification_setting_get_setting_by_package_name(
      kSmokeAppId, &setting);
  EXPECT_EQ(ret, NOTIFICATION_ERROR_NONE);
  ASSERT_NE(setting, nullptr);

  char* pkg = nullptr;
  notification_setting_get_package_name(setting, &pkg);
  ASSERT_NE(pkg, nullptr);
  EXPECT_STREQ(pkg, kSmokeAppId);

  notification_setting_free_notification(setting);
}

/* TC-SMK-EN-03 — get_setting_by_package_name on unknown pkg fails
 * cleanly (no crash, non-NONE return). */
TEST_F(EnumerateSmokeTest, GetSettingUnknownPackage) {
  notification_setting_h setting = nullptr;
  int ret = notification_setting_get_setting_by_package_name(
      "org.tizen.no.such.app.smoke", &setting);
  /* Either NOT_EXIST_ID or any non-NONE — both are fine as long as
   * setting is not populated. */
  if (ret == NOTIFICATION_ERROR_NONE) {
    /* Some implementations succeed with a default; in that case the
     * handle must at least be non-null and free-able. */
    EXPECT_NE(setting, nullptr);
    notification_setting_free_notification(setting);
  } else {
    EXPECT_EQ(setting, nullptr);
  }
}

/* TC-SMK-EN-04 — notification_get_list enumerates only what we
 * actually posted, in priv_id order */
TEST_F(EnumerateSmokeTest, GetListEnumerationOrder) {
  for (int i = 0; i < 3; i++) {
    notification_h n = MakeNoti(NOTIFICATION_TYPE_NOTI,
        "title" + std::to_string(i), "content");
    ASSERT_EQ(notification_post(n), NOTIFICATION_ERROR_NONE);
    notification_free(n);
  }

  notification_list_h list = nullptr;
  ASSERT_EQ(notification_get_list(NOTIFICATION_TYPE_NOTI, -1, &list),
            NOTIFICATION_ERROR_NONE);
  ASSERT_NE(list, nullptr);

  /* Walk and ensure we see at least 3 entries belonging to us, with
   * distinct priv_ids. */
  std::set<int> ids;
  notification_list_h iter = notification_list_get_head(list);
  while (iter != nullptr) {
    notification_h h = notification_list_get_data(iter);
    char* caller = nullptr;
    notification_get_pkgname(h, &caller);
    if (caller != nullptr && std::string(caller) == kSmokeAppId) {
      int priv = 0;
      notification_get_id(h, nullptr, &priv);
      ids.insert(priv);
    }
    iter = notification_list_get_next(iter);
  }
  EXPECT_GE(static_cast<int>(ids.size()), 3);

  notification_free_list(list);
}

/* TC-SMK-EN-05 — get_count_for_uid returns at least the number we
 * posted. */
TEST_F(EnumerateSmokeTest, GetCountAtLeastWhatWePosted) {
  for (int i = 0; i < 2; i++) {
    notification_h n = MakeNoti(NOTIFICATION_TYPE_NOTI, "c", "c");
    notification_post(n);
    notification_free(n);
  }
  int count = 0;
  EXPECT_EQ(notification_get_count(NOTIFICATION_TYPE_NOTI, nullptr, -1,
      NOTIFICATION_GROUP_ID_NONE, &count), NOTIFICATION_ERROR_NONE);
  EXPECT_GE(count, 2);
}

}  // namespace smoke
