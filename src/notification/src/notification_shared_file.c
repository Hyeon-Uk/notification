/*
 * Copyright (c) 2017 Samsung Electronics Co., Ltd. All rights reserved.
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

#define _GNU_SOURCE

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <gio/gio.h>
#include <glib/gstdio.h>

#include <sys/stat.h>
#include <sys/statvfs.h>
#include <sys/types.h>
#include <utime.h>
#include <sys/time.h>

#include "notification.h"
#include "notification_debug.h"
#include "notification_shared_file.h"
#include <notification_private.h>
#include "notification_internal.h"

#define NOTI_PRIV_DATA_DIR "data/.notification"

/* LCOV_EXCL_START */
static const char *__last_index_of(const char *path, const char *search)
{
	int i;
	int search_len;
	const char *index;

	if (path == NULL || search == NULL)
		return NULL;

	search_len = strlen(search);
	index = path + strlen(path) - search_len;

	while (index >= path) {
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
static bool __is_res_file(const char *file_path, const char *pkg_id)
{
	const char *path_index;
	bool ret = false;
	char res_path[PATH_MAX];

	if (file_path == NULL || pkg_id == NULL)
		return ret;

	snprintf(res_path, sizeof(res_path), "%s/res", pkg_id);

	path_index = __last_index_of(file_path, pkg_id);
	if (path_index != NULL &&
		(strncmp(path_index, res_path, strlen(res_path)) == 0))
		ret = true;

	return ret;
}

static bool __is_shared_file(const char *file_path, const char *pkg_id)
{
	const char *path_index;
	bool ret = false;
	char shared_path[PATH_MAX];

	if (file_path == NULL || pkg_id == NULL)
		return ret;

	snprintf(shared_path, sizeof(shared_path), "%s/shared", pkg_id);

	path_index = __last_index_of(file_path, pkg_id);
	if (path_index != NULL &&
		(strncmp(path_index, pkg_id, strlen(pkg_id)) == 0)) {
		if (strncmp(path_index, shared_path, strlen(shared_path)) == 0)
			ret = true;
		else
			ret = false;
	}

	return ret;
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
static bool __is_RO_file(const char *file_path)
{
	struct statvfs stat_info;
	bool ret = false;

	if (statvfs(file_path, &stat_info) == 0) {
		if (stat_info.f_flag & ST_RDONLY)
			ret = true;
		else
			ret = false;
	} else {
		ERR("failed to get stat : %s", file_path);
		ret = false;
	}

	return ret;
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
int notification_copy_private_file(const char *src_path,
					const char *dst_path)
{
	int ret = NOTIFICATION_ERROR_NONE;
	GFile *src = NULL;
	GFile *dst = NULL;
	GError *g_err = NULL;
	struct utimbuf ut = {0,};

	dst = g_file_new_for_path(dst_path);
	if (dst == NULL) {
		ERR("dst path is wrong [%s]", dst_path);
		ret = NOTIFICATION_ERROR_IO_ERROR;
		goto out;
	}

	if (g_file_query_exists(dst, NULL) == true) {
		ret = NOTIFICATION_ERROR_ALREADY_EXIST_ID;
		INFO("dst path existed [%s]", dst_path);
		goto out;
	}

	src = g_file_new_for_path(src_path);
	if (src == NULL) {
		ERR("src path is wrong [%s]", src_path);
		ret = NOTIFICATION_ERROR_IO_ERROR;
		goto out;
	}

	if (!g_file_copy(src, dst, G_FILE_COPY_NOFOLLOW_SYMLINKS, NULL, NULL,
			NULL, &g_err)) {
		if (g_err) {
			ERR("Copying file from [%s] to [%s] is failed [%s]",
				src_path, dst_path, g_err->message);
			g_error_free(g_err);
		}
		ret = NOTIFICATION_ERROR_IO_ERROR;
		goto out;
	}

	ut.modtime = time(NULL);
	if (g_utime(dst_path, &ut) != 0)
		DBG("Failed to set g_utime %d ", errno);

out:
	if (src)
		g_object_unref(src);
	if (dst)
		g_object_unref(dst);

	WARN("copy private file done : %d", ret);

	return ret;
}
/* LCOV_EXCL_STOP */

char *notification_check_file_path_is_private(const char *pkg_id,
							const char *file_path)
{
	char *dst_path = NULL;
	// uid_t uid = aul_getuid();

	WARN("check file path [%s][[%s]", file_path, pkg_id);

	if (pkg_id == NULL || file_path == NULL)
		return NULL;

	if (access(file_path, R_OK) != 0) {
		ERR("failed to access file : %s", file_path);
		return NULL;
	}

	// 1. shared in other package
	// 2. shared/[pkgid]/res
	// 3. [pkgid]/shared/res
	// 4. other cases

	if (__is_shared_file(file_path, pkg_id)) {
		/* file path is shared */
	} else if (__is_res_file(file_path, pkg_id)) {
		/* file path is res */
	} else if (__is_RO_file(file_path)) {
		/* file path is read-only */
	}

	// if (__is_RO_file(file_path)) {
	// 	WARN("is RO");
	// 	dst_path = __get_data_path_by_pkg_id(pkg_id, file_path, uid);
	// }

	// if (dst_path == NULL && __is_shared_file(file_path, pkg_id)) {
	// 	WARN("is private");
	// 	if (__is_res_file(file_path, pkg_id)) {
	// 		WARN("path res");
	// 		dst_path = __get_data_path_by_pkg_id(pkg_id, file_path, uid);
	// 	} else {
	// 		dst_path = strdup(file_path);
	// 		if (dst_path == NULL)
	// 			ERR("Failed to strdup");
	// 	}
	// }

	return dst_path;
}

