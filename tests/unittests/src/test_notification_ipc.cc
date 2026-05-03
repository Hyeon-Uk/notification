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
 * Phase 1.5-E.3.  notification_ipc.c only exposes one EXPORT_API
 * symbol (notification_ipc_make_noti_from_gvariant); the rest of the
 * surface is internal helpers for unmarshalling GVariants from
 * D-Bus.  This test asserts the two NULL-arg early-returns at
 * lines 377-385 of the source -- both fire before any helper is
 * invoked, so no fixture / mock is needed.
 */

#include <gtest/gtest.h>
#include <glib.h>

#include "notification/include/notification_error.h"

extern "C" {
#include <notification.h>
#include <notification_ipc.h>
}

namespace {

class NotificationIpcTest : public ::testing::Test {};

TEST_F(NotificationIpcTest, make_noti_from_gvariant_rejects_null_noti) {
  GVariant* variant = g_variant_new("(s)", "dummy");
  EXPECT_EQ(NOTIFICATION_ERROR_INVALID_PARAMETER,
      notification_ipc_make_noti_from_gvariant(nullptr, variant));
  g_variant_unref(variant);
}

TEST_F(NotificationIpcTest, make_noti_from_gvariant_rejects_null_variant) {
  // The NULL-noti early return fires first (line 377-380); the
  // NULL-variant branch (line 382-385) is reached only when noti is
  // non-null.  We pass an arbitrary non-null pointer cast to
  // notification_h since the function only inspects pointer non-null
  // before checking the variant.  This avoids notification_create()
  // which depends on aul_app_get_appid_bypid -- mocked via AulMock,
  // not registered in this fixture.
  notification_h dummy_noti = reinterpret_cast<notification_h>(0xDEADBEEF);
  EXPECT_EQ(NOTIFICATION_ERROR_INVALID_PARAMETER,
      notification_ipc_make_noti_from_gvariant(dummy_noti, nullptr));
}

}  // namespace
