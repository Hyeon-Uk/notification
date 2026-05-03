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
 * Phase 1.5-B seed test for the internal-API target.
 *
 * Calls the non-EXPORT_API helper notification_reset_event_handler_list
 * (declared in notification_internal.h, hidden in libnotification.so via
 * the global -fvisibility=hidden) to prove that the
 * notification-internal-static archive built with
 * -fvisibility=default actually exposes such helpers to a unit-test
 * translation unit.
 *
 * The helper is documented behavior: returns void; early-returns when
 * the global event-cb list is empty.  No setup needed; calling it on
 * an unprimed process must not crash.
 */

#include <gtest/gtest.h>

extern "C" {
#include <notification_internal.h>
}

namespace {

class InternalSeedTest : public ::testing::Test {};

TEST_F(InternalSeedTest, ResetEventHandlerListIsCallableWithEmptyList) {
  // Pre-condition: the per-process __noti_event_cb_list global is
  // unset, so the helper takes its early-return branch.  We are
  // asserting no crash and confirming linker resolution against the
  // visibility=default static archive.
  ASSERT_NO_FATAL_FAILURE(notification_reset_event_handler_list());
  // Idempotent: a second call must also early-return cleanly.
  ASSERT_NO_FATAL_FAILURE(notification_reset_event_handler_list());
}

}  // namespace
