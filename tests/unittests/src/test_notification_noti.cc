/*
 * Copyright (c) 2023 Samsung Electronics Co., Ltd.
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
 * The notification_noti.{c,h} unit was relocated to data-provider-master
 * because all its public APIs were used only by the DPM server side. The
 * original tests for it should be re-authored under
 * data-provider-master/tests/unit_tests/ against the DPM test fixture.
 *
 * This stub keeps the translation unit valid so the notification build still
 * picks up the surrounding test sources via AUX_SOURCE_DIRECTORY.
 */

#include <gtest/gtest.h>

TEST(NotificationNotiTest, MovedToDataProviderMaster) {
	GTEST_SKIP() <<
		"notification_noti.c was moved to data-provider-master; "
		"tests should be re-authored under data-provider-master/tests/.";
}
