/*
 * Copyright (c) 2026 Samsung Electronics Co., Ltd. All rights reserved.
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
 * N2-T6: Shared file helper implementations.
 *
 * Compiled into a CMake OBJECT library and linked into both
 * libnotification.so (C consumers) and libnotification-ex.so (C++
 * consumers). See shared_file_common.h for the contract of each helper.
 *
 * Design notes
 *   - Logging goes through dlog (LOGE/LOGD) so both libraries see the
 *     same diagnostic trail.  The original two implementations logged via
 *     "NOTIFICATION" (C) and "NOTIFICATION_EX" (C++); we pick a neutral
 *     "NOTIFICATION_SHARED_FILE" tag to keep the origin visible in logs
 *     regardless of which .so invoked us.
 *   - Return codes use the notification_error enum values directly for
 *     the I/O helpers so callers can forward them unchanged.
 *   - We intentionally keep every helper a pure function without module
 *     state: the private-sharing bookkeeping (GList of uid_info_s /
 *     SharedFile::SharingData) stays inside each caller's .c/.cc file.
 *     Only the mechanical "walk a path / copy a file / check RO" logic
 *     collapses here.
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <utime.h>
#include <limits.h>

#include <sys/stat.h>
#include <sys/statvfs.h>
#include <sys/types.h>

#include <gio/gio.h>
#include <glib.h>
#include <glib/gstdio.h>

#include <aul.h>
#include <dlog.h>
#include <tzplatform_config.h>

#include <notification_error.h>

#include "shared_file_common.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "NOTIFICATION_SHARED_FILE"

/* LCOV_EXCL_START */
const char *shared_file_common_last_index_of(const char *path,
                                             const char *search)
{
	int search_len;
	const char *index;

	if (path == NULL || search == NULL)
		return NULL;

	search_len = (int)strlen(search);
	if (search_len == 0)
		return path + strlen(path);

	if ((int)strlen(path) < search_len)
		return NULL;

	index = path + strlen(path) - search_len;

	while (index >= path) {
		int i;
		for (i = 0; i < search_len; i++) {
			if (index[i] != search[i])
				break;
		}

		if (i == search_len)
			return index;

		index--;
	}

	return NULL;
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
bool shared_file_common_make_dir(const char *dir)
{
	GFile *noti_dir;
	GError *g_err = NULL;

	if (dir == NULL)
		return false;

	if (access(dir, R_OK) == 0)
		return true;

	noti_dir = g_file_new_for_path(dir);
	if (noti_dir == NULL)
		return false;

	if (g_file_make_directory(noti_dir, NULL, &g_err) == FALSE) {
		if (g_err) {
			LOGE("Failed to make sharing dir[%s]", g_err->message);
			g_error_free(g_err);
		}
		g_object_unref(noti_dir);
		return false;
	}

	g_object_unref(noti_dir);
	return true;
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
bool shared_file_common_is_private_path(const char *file_path)
{
	struct statvfs stat_info;

	if (file_path == NULL)
		return false;

	if (access(file_path, R_OK) != 0) {
		LOGE("failed to access file : %s", file_path);
		return false;
	}

	if (statvfs(file_path, &stat_info) != 0) {
		LOGE("failed to get stat : %s", file_path);
		return false;
	}

	return (stat_info.f_flag & ST_RDONLY) ? true : false;
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
int shared_file_common_copy_file(const char *src_path, const char *dst_path)
{
	GFile *src = NULL;
	GFile *dst = NULL;
	GError *g_err = NULL;
	struct utimbuf ut = { 0, };
	int ret = NOTIFICATION_ERROR_NONE;

	if (src_path == NULL || dst_path == NULL) {
		LOGE("Invalid parameter");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	dst = g_file_new_for_path(dst_path);
	if (dst == NULL) {
		LOGE("dst path is wrong [%s]", dst_path);
		ret = NOTIFICATION_ERROR_IO_ERROR;
		goto out;
	}

	if (g_file_query_exists(dst, NULL) == TRUE) {
		ret = NOTIFICATION_ERROR_ALREADY_EXIST_ID;
		goto out;
	}

	src = g_file_new_for_path(src_path);
	if (src == NULL) {
		LOGE("src path is wrong [%s]", src_path);
		ret = NOTIFICATION_ERROR_IO_ERROR;
		goto out;
	}

	if (!g_file_copy(src, dst, G_FILE_COPY_NOFOLLOW_SYMLINKS, NULL, NULL,
				NULL, &g_err)) {
		if (g_err) {
			LOGE("Copying [%s] -> [%s] failed [%s]", src_path,
					dst_path, g_err->message);
			g_error_free(g_err);
		}
		ret = NOTIFICATION_ERROR_IO_ERROR;
		goto out;
	}

	ut.modtime = time(NULL);
	if (g_utime(dst_path, &ut) != 0)
		LOGD("Failed to set g_utime %d", errno);

out:
	if (src)
		g_object_unref(src);
	if (dst)
		g_object_unref(dst);

	return ret;
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
char *shared_file_common_get_data_path(const char *app_id,
                                       const char *file_path,
                                       const char *subdir)
{
	const char *user_app;
	const char *base;
	char *result;
	size_t len;

	if (app_id == NULL || file_path == NULL || subdir == NULL) {
		LOGE("Invalid parameter");
		return NULL;
	}

	tzplatform_set_user(aul_getuid());
	user_app = tzplatform_getenv(TZ_USER_APP);
	tzplatform_reset_user();

	if (user_app == NULL)
		return NULL;

	base = shared_file_common_last_index_of(file_path, "/");
	base = (base != NULL) ? base + 1 : file_path;

	/* <user_app>/<app_id>/<subdir>/<base>  +  3 slashes  +  NUL */
	len = strlen(user_app) + 1
	    + strlen(app_id)   + 1
	    + strlen(subdir)   + 1
	    + strlen(base)     + 1;
	if (len > PATH_MAX)
		return NULL;

	result = (char *)malloc(len);
	if (result == NULL)
		return NULL;

	snprintf(result, len, "%s/%s/%s/%s", user_app, app_id, subdir, base);
	return result;
}
/* LCOV_EXCL_STOP */
