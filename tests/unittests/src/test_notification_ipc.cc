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

// =====================================================================
// Lock-in for notification_ipc.h (task 03c).
// 2 declared functions, both exported:
//   - notification_ipc_make_gvariant_from_noti
//   - notification_ipc_make_noti_from_gvariant
//
// R-FINDINGS:
// F-task03c-make-gvariant-segfault: passing nullptr for noti
// causes the impl to dereference the handle and SIGSEGV. Same
// shape as F-task02h-validate-segfault. Future fix task should
// add a NULL guard. Until then we don't call with nullptr in
// the test binary (would crash the whole gtest run).
//
// F-task03c-make-noti-from-gvariant-debt: needs valid gvariant
// data + a real notification_h to roundtrip; lock-in deferred
// to task 19's integration suite.
// =====================================================================

namespace {
class NotificationIpcTest : public ::testing::Test {};
}

TEST_F(NotificationIpcTest, ipc_symbols_reachable) {
  // Smoke: both symbols are linked. We do NOT call them with
  // null inputs (would SIGSEGV per F-task03c-* findings above).
  void (*sym1)() = reinterpret_cast<void(*)()>(&notification_ipc_make_gvariant_from_noti);
  void (*sym2)() = reinterpret_cast<void(*)()>(&notification_ipc_make_noti_from_gvariant);
  EXPECT_NE(sym1, nullptr);
  EXPECT_NE(sym2, nullptr);
}
