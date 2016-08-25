/*
 * Copyright (c) 2016 Samsung Electronics Co., Ltd. All rights reserved.
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
 *
 */

#define _GNU_SOURCE

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

#include <notification_setting.h>

/* For multi-user support */
#include <tzplatform_config.h>

#define OWNER_ROOT 0

#ifdef _E
#undef _E
#endif
#define _E(fmt, arg...) fprintf(stderr, "[NOTIFICATION_INIT][E][%s,%d] "fmt"\n", \
		__FUNCTION__, __LINE__, ##arg);

static int _is_authorized()
{
	/* pkg_init db should be called by as root privilege. */
	uid_t uid = getuid();

	if ((uid_t) OWNER_ROOT == uid)
		return 1;
	else
		return 0;
}

int main(int argc, char *argv[])
{
	int ret;
	uid_t uid = 0;

	if (!_is_authorized()) {
		_E("You are not an authorized user!");
		return -1;
	}

	if (argc > 2)
		uid = (uid_t)atoi(argv[2]);
	ret = notification_setting_refresh_setting_table(uid);
	if (ret != NOTIFICATION_ERROR_NONE) {
		_E("notification setting table refresh fail.");
		return ret;
	}

	ret = notification_system_setting_init_system_setting_table(uid);
	if (ret != NOTIFICATION_ERROR_NONE) {
		_E("notification system setting table init fail.");
		return ret;
	}

	return ret;
}
