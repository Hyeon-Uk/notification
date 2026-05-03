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
 * Phase 1.5-E.7.  notification_viewer.c surface.
 *
 * The three EXPORT_API entries (init / launch / launch-without-
 * candidate) are LCOV_EXCL_START/STOP-wrapped at source.  The two
 * launch entries route through a shared __launch_default_viewer
 * helper which short-circuits with NOTIFICATION_ERROR_NONE when
 * the static `_default_viewer` global is NULL -- which is the
 * default state on a fresh process where init_default_viewer has
 * not been called.  This is the cleanly testable behavior.
 */

#include <gtest/gtest.h>

#include "notification/include/notification_error.h"

extern "C" {
#include <notification.h>
#include <notification_viewer.h>
}

namespace {

class NotificationViewerTest : public ::testing::Test {};

TEST_F(NotificationViewerTest,
       launch_default_viewer_returns_none_on_fresh_process) {
  // _default_viewer is NULL before init runs; the launch path
  // (notification_viewer.c line 258-259) short-circuits with NONE.
  EXPECT_EQ(NOTIFICATION_ERROR_NONE,
      notification_launch_default_viewer(1, NOTIFICATION_OP_INSERT, 0));
}

TEST_F(NotificationViewerTest,
       launch_default_viewer_without_candidate_returns_none_on_fresh_process) {
  // Same short-circuit, candidate = false branch.
  EXPECT_EQ(NOTIFICATION_ERROR_NONE,
      notification_launch_default_viewer_without_candidate_process(
          1, NOTIFICATION_OP_INSERT, 0));
}

}  // namespace
