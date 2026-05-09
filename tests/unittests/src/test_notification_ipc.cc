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

#include <gtest/gtest.h>
#include <glib.h>

#include "notification/include/notification.h"
#include "notification/include/notification_ipc.h"

namespace {
class NotificationIpcTest : public ::testing::Test {};
}

TEST_F(NotificationIpcTest, ipc_symbols_reachable) {
  // Smoke: both symbols are linked. We do NOT call them with
  void (*sym1)() = reinterpret_cast<void(*)()>(&notification_ipc_make_gvariant_from_noti);
  void (*sym2)() = reinterpret_cast<void(*)()>(&notification_ipc_make_noti_from_gvariant);
  EXPECT_NE(sym1, nullptr);
  EXPECT_NE(sym2, nullptr);
}
