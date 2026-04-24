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
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <aul.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <gio/gio.h>
#include <glib/gstdio.h>

#include <tizen_core.h>
#include <tzplatform_config.h>
#include <security-manager.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <linux/xattr.h>
#include <sys/types.h>
#include <utime.h>
#include <sys/time.h>
#include <package_manager.h>

#include "notification.h"
#include "notification_debug.h"
#include "notification_shared_file.h"
#include <notification_private.h>
#include "notification_internal.h"
#include "shared_file_common.h"

#define NOTI_PRIV_DATA_DIR "data/.notification"
#define MAX_TIMEOUT 5000
#define MAX_RETRY_CNT 3
#define ERR_BUFFER_SIZE 1024

#define DUMMY_PARAM
#define __OOM_CHECK(value, ret_value, free_fun) \
do { \
	if (value == NULL) { \
		ERR("out of memory"); \
		free_fun; \
		return ret_value; \
	} \
} while (0)

GList *__uid_list;
typedef struct uid_info {
	uid_t uid;
	GList *sharing_req_list;
	GList *target_app_list;
} uid_info_s;

typedef struct sharing_req_data {
	char *app_id;
	char *dir;
	tizen_core_source_h timer;
	int drop_retry_count;
	uid_t uid;
	GList *priv_id_list;
	GList *shared_file_list;
	GList *target_app_table;
} sharing_req_data_s;

typedef struct sharing_file_info {
	char *src_path;
	char *dst_path;
	time_t modification_time;
} sharing_file_info_s;

typedef struct target_app_info {
	char *app_id;
	char *tidl_sender_name;
} target_app_info_s;

/* N2-T6: __last_index_of / __make_sharing_dir collapsed onto
 * shared_file_common_last_index_of / shared_file_common_make_dir.
 * Thin file-static wrappers keep the existing call sites untouched and
 * preserve the legacy WARN logs local callers depend on. */
#define __last_index_of(path, search) \
	shared_file_common_last_index_of((path), (search))

/* LCOV_EXCL_START */
static bool __make_sharing_dir(const char *dir)
{
	bool ok = shared_file_common_make_dir(dir);
	if (ok)
		WARN("__make_sharing_dir done : %s", dir);
	return ok;
}
/* LCOV_EXCL_STOP */

/* Check path that include res directory */
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

/* N2-T6: __is_RO_file delegates to shared_file_common_is_private_path,
 * which performs the same statvfs(ST_RDONLY) probe (with an added R_OK
 * access check for the symmetric C++ callsite). The extra access() call
 * is harmless here: the caller only hits __is_RO_file with paths that
 * already passed access(R_OK) inside notification_check_file_path_is_private. */
#define __is_RO_file(file_path) \
	shared_file_common_is_private_path((file_path))

/* file copy from /res to /data */
/* LCOV_EXCL_START */
/*
 * N2-T6: copy body moved to shared_file_common_copy_file. This entry
 * point stays an EXPORT_API passthrough so external callers (notification
 * setting, test-app) keep the same ABI.
 */
int notification_copy_private_file(const char *src_path,
					const char *dst_path)
{
	int ret = shared_file_common_copy_file(src_path, dst_path);
	WARN("copy private file done : %d", ret);
	return ret;
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
static void __free_file_info(gpointer data)
{
	sharing_file_info_s *info = (sharing_file_info_s *)data;

	if (info != NULL) {
		if (info->src_path)
			free(info->src_path);
		if (info->dst_path)
			free(info->dst_path);
		free(info);
	}
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
static void __free_req_info(gpointer data)
{
	sharing_req_data_s *info = (sharing_req_data_s *)data;

	if (info == NULL)
		return;
	if (info->app_id)
		free(info->app_id);
	if (info->dir)
		free(info->dir);
	if (info->shared_file_list)
		g_list_free_full(info->shared_file_list, __free_file_info);
	if (info->target_app_table)
		g_list_free_full(info->target_app_table, free);

	WARN("__free_req_info done");

	free(info);
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
static char **__convert_list_to_array(GList *list, int *length)
{
	int len, i;
	char **array;
	GList *iter;

	if (list == NULL)
		return NULL;

	len = g_list_length(list);
	if (len == 0)
		return NULL;

	array = (char **)calloc(len + 1, sizeof(char *));
	__OOM_CHECK(array, NULL, DUMMY_PARAM);

	for (i = 0, iter = list; iter != NULL; i++, iter = g_list_next(iter))
		array[i] = ((sharing_file_info_s *)iter->data)->dst_path;

	*length = len;

	return array;
}

static gint __comp_str(gconstpointer a, gconstpointer b)
{
	char *new_file = (char *)a;
	char *file = (char *)b;

	if (new_file == NULL || file == NULL)
		return -1;

	return strcmp(new_file, file);
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
static gint __comp_file_info(gconstpointer a, gconstpointer b)
{
	sharing_file_info_s *file = (sharing_file_info_s *)a;
	sharing_file_info_s *new_file = (sharing_file_info_s *)b;

	if (!file || !new_file || !new_file->dst_path || !file->dst_path)
		return -1;

	return strcmp(new_file->dst_path, file->dst_path);
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
static gint __comp_dst_path(gconstpointer a, gconstpointer b)
{
	sharing_file_info_s *info = (sharing_file_info_s *)a;
	char *path = (char *)b;

	if (!info || !path || !info->dst_path)
		return -1;

	return strcmp(info->dst_path, path);
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
static sharing_file_info_s *__dup_file_info(sharing_file_info_s *src)
{
	sharing_file_info_s *file_info;

	file_info = (sharing_file_info_s *)calloc(1, sizeof(sharing_file_info_s));
	__OOM_CHECK(file_info, NULL, DUMMY_PARAM);

	file_info->dst_path = strdup(src->dst_path);
	__OOM_CHECK(file_info->dst_path, NULL, __free_file_info(file_info));

	file_info->src_path = strdup(src->src_path);
	__OOM_CHECK(file_info->src_path, NULL, __free_file_info(file_info));

	file_info->modification_time = src->modification_time;

	return file_info;

}

static void __make_file_info(char *src_path, char *dst_path,
	GList **new_file_list, GList *shared_file_list, bool *is_overlapping)
{
	int ret;
	GList *tmp;
	sharing_file_info_s *file_info;
	sharing_file_info_s *dup_file_info;
	struct stat stat_buf = {0,};

	WARN("__make_file_info : %s", src_path);
	WARN("__make_file_info : %s", dst_path);

	/*
	* If the file copy is successful,
	* ignore the shared_file_list check for private sharing
	*/
	ret = notification_copy_private_file(src_path, dst_path);
	if (ret == NOTIFICATION_ERROR_ALREADY_EXIST_ID) {
		tmp = g_list_find_custom(shared_file_list, dst_path,
				__comp_dst_path);
		if (tmp == NULL) {
			WARN("__make_file_info tmp null");

			file_info = (sharing_file_info_s *)calloc(1,
					sizeof(sharing_file_info_s));
			__OOM_CHECK(file_info, DUMMY_PARAM, DUMMY_PARAM);

			file_info->dst_path = strdup(dst_path);
			__OOM_CHECK(file_info->dst_path, DUMMY_PARAM,
				__free_file_info(file_info));

			file_info->src_path = strdup(src_path);
			__OOM_CHECK(file_info->src_path, DUMMY_PARAM,
				__free_file_info(file_info));

			if (stat(dst_path, &stat_buf) != 0)
				ERR("Failed to get stat info");
			file_info->modification_time = stat_buf.st_mtime;

			/* N3-T4 A1: prepend; caller __get_new_file_list
			 * reverses once all helper calls are done. */
			*new_file_list = g_list_prepend(*new_file_list, file_info);
		} else {
			WARN("__make_file_info tmp not null");

			file_info = (sharing_file_info_s *)tmp->data;

			if (stat(file_info->dst_path, &stat_buf) != 0)
				ERR("Failed to get stat info");

			if (file_info->modification_time != stat_buf.st_mtime) {
				dup_file_info = __dup_file_info(file_info);
				__OOM_CHECK(dup_file_info, DUMMY_PARAM, DUMMY_PARAM);

				*new_file_list = g_list_prepend(*new_file_list,
						dup_file_info);
				*is_overlapping = true;
			}
		}
	} else if (ret == NOTIFICATION_ERROR_NONE) {
		*is_overlapping = true;
	}
	WARN("__make_file_info done");
}

static GList *__get_new_file_list(notification_h noti,
				GList *shared_file_list,
				bool *is_overlapping)
{
	GList *new_file_list = NULL;
	char *src_path, *dst_path, *dup_str;
	char buf_key[32] = { 0, };
	int i = NOTIFICATION_IMAGE_TYPE_ICON;

	WARN("__get_new_file_list");

	if (noti->priv_sound_path != NULL &&  noti->sound_path != NULL) {
		dst_path = noti->priv_sound_path;
		src_path = noti->sound_path;

		dup_str = strdup(dst_path);
		__OOM_CHECK(dup_str, NULL, DUMMY_PARAM);

		__make_file_info(src_path, dst_path, &new_file_list,
				shared_file_list, is_overlapping);

		free(noti->sound_path);
		noti->sound_path = dup_str;
	}

	if (noti->priv_vibration_path != NULL && noti->vibration_path != NULL) {
		dst_path = noti->priv_vibration_path;
		src_path = noti->vibration_path;

		dup_str = strdup(dst_path);
		__OOM_CHECK(dup_str, NULL,
			g_list_free_full(new_file_list, __free_file_info));

		__make_file_info(src_path, dst_path, &new_file_list,
						shared_file_list, is_overlapping);

		free(noti->vibration_path);
		noti->vibration_path = dup_str;
	}

	if (noti->b_priv_image_path == NULL) {
		/* N3-T4 A1: restore insertion order after prepend chain. */
		new_file_list = g_list_reverse(new_file_list);
		return new_file_list;
	}

	for (; i <= NOTIFICATION_IMAGE_TYPE_MAX; i++) {
		src_path = NULL;
		dst_path = NULL;
		snprintf(buf_key, sizeof(buf_key), "%d", i);
		bundle_get_str(noti->b_priv_image_path, buf_key, &dst_path);
		bundle_get_str(noti->b_image_path, buf_key, &src_path);
		if (dst_path != NULL && src_path != NULL) {
			__make_file_info(src_path, dst_path, &new_file_list,
					shared_file_list, is_overlapping);

			bundle_del(noti->b_image_path, buf_key);
			bundle_add_str(noti->b_image_path, buf_key, dst_path);
			WARN("__get_new_file_list changed path %d : %s", i, dst_path);
		}
	}
	/* N3-T4 A1: restore insertion order after prepend chain. */
	new_file_list = g_list_reverse(new_file_list);
	WARN("__get_new_file_list done");

	return new_file_list;
}

static char *__get_shared_dir(notification_h noti)
{
	char *path;
	char *dir;
	char *shared_path = NULL;
	const char *index;
	char buf_key[32] = { 0, };
	int ret;
	int i = NOTIFICATION_IMAGE_TYPE_ICON, dir_len;

	if (noti->b_priv_image_path != NULL) {
		for (; i <= NOTIFICATION_IMAGE_TYPE_MAX; i++) {
			snprintf(buf_key, sizeof(buf_key), "%d", i);
			ret = bundle_get_str(noti->b_priv_image_path, buf_key, &path);
			if (ret == BUNDLE_ERROR_NONE
					&& __last_index_of(path, NOTI_PRIV_DATA_DIR) != NULL) {
				shared_path = path;
				break;
			}
		}
	}

	if (shared_path == NULL && noti->priv_sound_path != NULL
			&& __last_index_of(noti->priv_sound_path, NOTI_PRIV_DATA_DIR) != NULL)
		shared_path = noti->priv_sound_path;
	else if (shared_path == NULL && noti->priv_vibration_path != NULL
			&& __last_index_of(noti->priv_vibration_path, NOTI_PRIV_DATA_DIR) != NULL)
		shared_path = noti->priv_vibration_path;

	if (shared_path == NULL) {
		DBG("No private resource");
		return NULL;
	}

	index =  __last_index_of(shared_path, "/");
	if (index == NULL) {
		ERR("Failed to find directory separator");
		return NULL;
	}

	dir_len = index - shared_path + 1;
	if (dir_len <= 0 || dir_len > PATH_MAX)
		return NULL;

	dir = (char *)calloc(dir_len, sizeof(char));
	__OOM_CHECK(dir, NULL, DUMMY_PARAM);

	snprintf(dir, dir_len, "%s", shared_path);

	WARN("__get_shared_dir : %s", dir);

	return dir;
}

static gint __comp_sharing_req_list(gconstpointer a, gconstpointer b)
{
	sharing_req_data_s *req_data = (sharing_req_data_s *)a;
	char *app_id = (char *)b;

	if (!req_data || !app_id || !req_data->app_id)
		return -1;

	if (!strcmp(app_id, req_data->app_id))
		return 0;

	return -1;
}

static gint __comp_uid_info_list(gconstpointer a, gconstpointer b)
{
	uid_info_s *uid_info = (uid_info_s *)a;

	if (!a || !b)
		return -1;

	if (uid_info->uid == GPOINTER_TO_INT(b))
		return 0;

	return -1;
}

static gint __comp_priv_id(gconstpointer a, gconstpointer b)
{
	if (!a || !b)
		return -1;

	if (GPOINTER_TO_INT(a) == GPOINTER_TO_INT(b))
		return 0;

	return 1;
}

static gint __comp_target_app(gconstpointer a, gconstpointer b)
{
	target_app_info_s *target = (target_app_info_s *)a;
	char *sender = (char *)b;

	if (!target || !sender || !target->tidl_sender_name)
		return -1;

	return strcmp(sender, target->tidl_sender_name);
}

EXPORT_API void notification_remove_private_sharing_target_id(
				const char *sender, uid_t uid)
{
	target_app_info_s *target_info;
	uid_info_s *uid_info;
	GList *sharing_req_list, *target_app;

	WARN("notification_remove_private_sharing_target_id : %s", sender);

	sharing_req_list = g_list_find_custom(__uid_list, GINT_TO_POINTER(uid),
						__comp_uid_info_list);
	if (sharing_req_list == NULL)
		return;

	uid_info = sharing_req_list->data;
	target_app = g_list_find_custom(uid_info->target_app_list, sender,
						__comp_target_app);

	if (target_app != NULL) {
		target_info = target_app->data;
		if (target_info) {
			uid_info->target_app_list = g_list_remove(
					uid_info->target_app_list, target_info);

			free(target_info->app_id);
			free(target_info->tidl_sender_name);
			free(target_info);
			target_info = NULL;
		}
	}
	WARN("notification_remove_private_sharing_target_id done");
}

EXPORT_API void notification_add_private_sharing_target_id(pid_t pid,
					const char *sender, uid_t uid)
{
	char *app_id;
	target_app_info_s *target_info;
	uid_info_s *uid_info;
	GList *sharing_req_list, *target_app;

	WARN("notification_add_private_sharing_target_id : [%s]", sender);

	sharing_req_list = g_list_find_custom(__uid_list, GINT_TO_POINTER(uid),
						__comp_uid_info_list);
	if (sharing_req_list == NULL) {
		WARN("sharing list null");

		uid_info = (uid_info_s *)calloc(1, sizeof(uid_info_s));
		__OOM_CHECK(uid_info, DUMMY_PARAM, DUMMY_PARAM);
		uid_info->uid = uid;

		__uid_list = g_list_append(__uid_list, uid_info);
	} else {
		uid_info = sharing_req_list->data;
	}

	target_app = g_list_find_custom(uid_info->target_app_list, sender,
						__comp_target_app);
	if (target_app == NULL) {
		WARN("target app null");

		app_id = notification_get_app_id_by_pid((int)pid);
		if (app_id == NULL) {
			ERR("Failed to get app id by pid");
			return;
		}

		target_info = (target_app_info_s *)calloc(1, sizeof(target_app_info_s));
		if (target_info == NULL) {
			ERR("Failed to alloc memory");
			free(app_id);
			return;
		}

		target_info->app_id = app_id;
		target_info->tidl_sender_name = strdup(sender);
		if (target_info->tidl_sender_name == NULL) {
			ERR("Failed to alloc memory");
			free(target_info);
			free(app_id);
			return;
		}

		uid_info->target_app_list = g_list_append(
			uid_info->target_app_list, target_info);
		WARN("target app [%s]", app_id);
	}
	WARN("notification_add_private_sharing_target_id done");
}

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
		WARN("is shared");
	} else if (__is_res_file(file_path, pkg_id)) {
		WARN("path res");
	} else if (__is_RO_file(file_path)) {
		WARN("is RO");
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

EXPORT_API bool notification_validate_private_sharing(
						notification_h updated_noti)
{
	char *updated_path = NULL;
	char *private_path = NULL;
	char buf_key[32] = { 0, };
	int i = NOTIFICATION_IMAGE_TYPE_ICON;

	if (updated_noti->b_image_path && updated_noti->b_priv_image_path) {
		for (; i <= NOTIFICATION_IMAGE_TYPE_MAX; i++) {
			updated_path = NULL;
			private_path = NULL;

			snprintf(buf_key, sizeof(buf_key), "%d", i);
			bundle_get_str(updated_noti->b_image_path, buf_key,
							&updated_path);
			if (updated_path == NULL)
				continue;

			bundle_get_str(updated_noti->b_priv_image_path,
							buf_key, &private_path);
			if (private_path == NULL)
				continue;

			if (strcmp(updated_path, private_path) == 0)
				return false;

			WARN("validate private sharing image path : %d", i);
		}
	}

	if (updated_noti->sound_path && updated_noti->priv_sound_path) {
		WARN("validate private sharing sound path");
		if (strcmp(updated_noti->sound_path,
					updated_noti->priv_sound_path) == 0)
			return false;
	}

	if (updated_noti->vibration_path && updated_noti->priv_vibration_path) {
		WARN("validate private sharing vibration path");
		if (strcmp(updated_noti->vibration_path,
					updated_noti->priv_vibration_path) == 0)
			return false;
	}

	return true;
}

EXPORT_API void notification_calibrate_private_sharing(
			notification_h updated_noti, notification_h source_noti)
{
	char *updated_path = NULL;
	char *source_path = NULL;
	char *private_path = NULL;
	char buf_key[32] = { 0, };
	int i = NOTIFICATION_IMAGE_TYPE_ICON;

	if (updated_noti->b_image_path && updated_noti->b_priv_image_path) {
		for (; i <= NOTIFICATION_IMAGE_TYPE_MAX; i++) {
			source_path = NULL;
			updated_path = NULL;
			private_path = NULL;

			snprintf(buf_key, sizeof(buf_key), "%d", i);
			bundle_get_str(updated_noti->b_image_path, buf_key,
						&updated_path);
			if (updated_path == NULL)
				continue;

			bundle_get_str(updated_noti->b_priv_image_path,
						buf_key, &private_path);
			if (private_path == NULL)
				continue;

			if (strcmp(updated_path, private_path) == 0) {
				if (bundle_get_str(source_noti->b_image_path,
					buf_key, &source_path) == BUNDLE_ERROR_NONE) {
					bundle_del(updated_noti->b_image_path,
						buf_key);
					bundle_add_str(updated_noti->b_image_path,
						buf_key, source_path);
					WARN("calibrate private sharing set path : %s", source_path);
				}
			}
			WARN("calibrate private sharing image path : %d", i);
		}
	}

	if (updated_noti->sound_path && updated_noti->priv_sound_path) {
		WARN("calibrate private sharing sound path");
		if (strcmp(updated_noti->sound_path,
				updated_noti->priv_sound_path) == 0) {
			free(updated_noti->sound_path);
			updated_noti->sound_path = NULL;

			if (source_noti->sound_path) {
				updated_noti->sound_path = strdup(source_noti->sound_path);
				if (updated_noti->sound_path == NULL)
					ERR("out of memory");
			}
		}
	}

	if (updated_noti->vibration_path && updated_noti->priv_vibration_path) {
		WARN("calibrate private sharing vibration path");
		if (strcmp(updated_noti->vibration_path,
				updated_noti->priv_vibration_path) == 0) {
			free(updated_noti->vibration_path);
			updated_noti->vibration_path = NULL;

			if (source_noti->priv_vibration_path) {
				updated_noti->vibration_path =
					strdup(source_noti->priv_vibration_path);
				if (updated_noti->vibration_path == NULL)
					ERR("out of memory");
			}
		}
	}
}


int __set_sharing_for_new_target(sharing_req_data_s *req_data,
					GList *target_app_list)
{
	char **path_array = NULL;
	int ret = NOTIFICATION_ERROR_NONE;
	int len;
	private_sharing_req *handle = NULL;
	GList *iter, *tmp;
	target_app_info_s *target_info;
	char *app_info = NULL;
	/* N3-T4 A1: accumulate successful entries in a local prepend list
	 * and splice once after the loop so per-iteration tail walk of
	 * req_data->target_app_table is avoided. */
	GList *new_targets = NULL;

	WARN("__set_sharing_for_new_target : %d",
			g_list_length(target_app_list));

	iter = target_app_list;
	for (; iter != NULL; iter = g_list_next(iter)) {
		target_info = (target_app_info_s *)iter->data;
		tmp = g_list_find_custom(req_data->target_app_table,
					target_info->app_id, __comp_str);
		if (tmp != NULL) {
			ERR("tmp not null continue");
			continue;
		}
		/* Also skip if we already queued it in this invocation. */
		if (g_list_find_custom(new_targets, target_info->app_id,
					__comp_str) != NULL) {
			continue;
		}

		app_info = strdup(target_info->app_id);
		if (app_info == NULL) {
			ERR("out of memory");
			ret = NOTIFICATION_ERROR_OUT_OF_MEMORY;
			goto out;
		}

		if (handle == NULL) {
			ret = security_manager_private_sharing_req_new(&handle);
			if (ret != SECURITY_MANAGER_SUCCESS) {
				ret = NOTIFICATION_ERROR_IO_ERROR;
				ERR("Failed to create PS handle");
				goto out;
			}

			ret = security_manager_private_sharing_req_set_owner_appid(
						handle, req_data->app_id);
			if (ret != SECURITY_MANAGER_SUCCESS) {
				ret = NOTIFICATION_ERROR_IO_ERROR;
				ERR("Failed to set owner appid(%s) %d",
						req_data->app_id, ret);
				goto out;
			}
			path_array = __convert_list_to_array(
					req_data->shared_file_list, &len);
			if (path_array == NULL) {
				ret = NOTIFICATION_ERROR_OUT_OF_MEMORY;
				ERR("path array is null");
				goto out;
			}

			ret = security_manager_private_sharing_req_add_paths(
					handle, (const char **)path_array, len);
			if (ret != SECURITY_MANAGER_SUCCESS) {
				ret = NOTIFICATION_ERROR_IO_ERROR;
				ERR("Failed to add paths %d", ret);
				goto out;
			}
		}

		ret = security_manager_private_sharing_req_set_target_appid(
					handle, target_info->app_id);
		if (ret != SECURITY_MANAGER_SUCCESS) {
			ret = NOTIFICATION_ERROR_IO_ERROR;
			ERR("Failed to set target appid(%s)",
				(const char *)iter->data);
			goto out;
		}

		WARN("set private sharing start [%s]", target_info->app_id);
		ret = security_manager_private_sharing_apply(handle);
		if (ret != SECURITY_MANAGER_SUCCESS) {
			ret = NOTIFICATION_ERROR_IO_ERROR;
			ERR("Failed to apply PS %d", ret);
			goto out;
		}
		WARN("set private sharing end");

		new_targets = g_list_prepend(new_targets, app_info);
	}

	/* Splice successful adds in original traversal order. */
	new_targets = g_list_reverse(new_targets);
	req_data->target_app_table =
			g_list_concat(req_data->target_app_table, new_targets);
	new_targets = NULL;

out:
	if (new_targets != NULL) {
		/* Error path: we allocated strdup'd entries but did not
		 * publish them. free each strdup before discarding. */
		g_list_free_full(new_targets, free);
	}

	if (ret != NOTIFICATION_ERROR_NONE && app_info)
		free(app_info);

	if (handle != NULL)
		security_manager_private_sharing_req_free(handle);

	if (path_array != NULL)
		free(path_array);

	WARN("__set_sharing_for_new_target done : %d", ret);

	return ret;
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
int __set_sharing_for_new_file(sharing_req_data_s *req_data,
				GList *new_file_list, bool is_overlapping)
{
	char **path_array = NULL;
	int ret = NOTIFICATION_ERROR_NONE;
	int len;
	private_sharing_req *handle = NULL;
	GList *iter;

	WARN("__set_sharing_for_new_file %d", (int)is_overlapping);

	path_array = __convert_list_to_array(new_file_list, &len);
	if (path_array == NULL) {
		ERR("path array is null");
		ret = NOTIFICATION_ERROR_OUT_OF_MEMORY;
		goto out;
	}

	ret = security_manager_private_sharing_req_new(&handle);
	if (ret != SECURITY_MANAGER_SUCCESS) {
		ERR("Failed to create private sharing request handle[%d]", ret);
		ret = NOTIFICATION_ERROR_IO_ERROR;
		goto out;
	}

	ret = security_manager_private_sharing_req_set_owner_appid(handle,
					req_data->app_id);
	if (ret != SECURITY_MANAGER_SUCCESS) {
		ERR("Failed to set owner appid[%s][%d]", req_data->app_id, ret);
		ret = NOTIFICATION_ERROR_IO_ERROR;
		goto out;
	}

	ret = security_manager_private_sharing_req_add_paths(handle,
						(const char **)path_array, len);
	if (ret != SECURITY_MANAGER_SUCCESS) {
		ERR("Failed to add paths [%d]", ret);
		ret = NOTIFICATION_ERROR_IO_ERROR;
		goto out;
	}

	if (is_overlapping == true) {
		iter = req_data->target_app_table;
		for (; iter != NULL; iter = g_list_next(iter)) {
			ret = security_manager_private_sharing_req_set_target_appid(
							handle, (const char *)iter->data);
			if (ret != SECURITY_MANAGER_SUCCESS) {
				ERR("Failed to set target appid [%s]",
					(const char *)iter->data);
				ret = NOTIFICATION_ERROR_IO_ERROR;
				goto out;
			}

			WARN("drop sharing [%s]", (const char *)iter->data);
			ret = security_manager_private_sharing_drop(handle);
			if (ret != SECURITY_MANAGER_SUCCESS) {
				ERR("Failed to drop [%d]", ret);
				ret = NOTIFICATION_ERROR_IO_ERROR;
				goto out;
			}
		}
	}
	WARN("taget app table : %d", g_list_length(req_data->target_app_table));

	iter = req_data->target_app_table;
	for (; iter != NULL; iter = g_list_next(iter)) {
		ret = security_manager_private_sharing_req_set_target_appid(handle,
							(const char *)iter->data);
		if (ret != SECURITY_MANAGER_SUCCESS) {
			ERR("Failed to set target appid [%s]",
				(const char *)iter->data);
			ret = NOTIFICATION_ERROR_IO_ERROR;
			goto out;
		}

		WARN("set private sharing start [%s]", (const char *)iter->data);
		ret = security_manager_private_sharing_apply(handle);
		if (ret != SECURITY_MANAGER_SUCCESS) {
			ERR("Failed to apply PS [%d]", ret);
			ret = NOTIFICATION_ERROR_IO_ERROR;
			goto out;
		}
		WARN("set private sharing end");
	}

out:
	if (handle != NULL)
		security_manager_private_sharing_req_free(handle);

	if (path_array != NULL)
		free(path_array);

	WARN("__set_sharing_for_new_file done");

	return ret;
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
EXPORT_API int notification_set_private_sharing(notification_h noti,
						uid_t uid)
{
	int ret = NOTIFICATION_ERROR_NONE;
	bool is_overlapping = false;
	sharing_req_data_s *req_data;
	sharing_file_info_s *file_info, *dup_file_info;
	uid_info_s *uid_info;
	GList *req_list, *iter, *tmp;
	GList *new_file_list = NULL;

	if (noti == NULL || noti->caller_app_id == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	tmp = g_list_find_custom(__uid_list, GINT_TO_POINTER(uid),
						__comp_uid_info_list);
	if (tmp == NULL) {
		WARN("set private sharing uid info null");

		uid_info = (uid_info_s *)calloc(1, sizeof(uid_info_s));
		__OOM_CHECK(uid_info, NOTIFICATION_ERROR_OUT_OF_MEMORY, DUMMY_PARAM);

		uid_info->uid = uid;
		__uid_list = g_list_append(__uid_list, uid_info);
	} else {
		uid_info = tmp->data;
	}

	req_list = g_list_find_custom(uid_info->sharing_req_list,
				noti->caller_app_id, __comp_sharing_req_list);
	if (req_list == NULL) {
		WARN("set private sharing req list null");

		req_data = (sharing_req_data_s *)calloc(1, sizeof(sharing_req_data_s));
		__OOM_CHECK(req_data, NOTIFICATION_ERROR_OUT_OF_MEMORY, DUMMY_PARAM);

		req_data->app_id = strdup(noti->caller_app_id);
		__OOM_CHECK(req_data->app_id, NOTIFICATION_ERROR_OUT_OF_MEMORY,
						__free_req_info(req_data));

		req_data->dir = __get_shared_dir(noti);
		req_data->uid = uid;

		uid_info->sharing_req_list = g_list_append(
			uid_info->sharing_req_list, req_data);
	} else {
		WARN("set private sharing req list exist");

		req_data = (sharing_req_data_s *)req_list->data;
		if (req_data->dir == NULL)
			req_data->dir = __get_shared_dir(noti);

		if (req_data->timer) {
			tizen_core_source_destroy(req_data->timer);
			req_data->timer = NULL;
		}
	}

	if (req_data->dir != NULL)
		__make_sharing_dir(req_data->dir);

	tmp = g_list_find_custom(req_data->priv_id_list,
				GINT_TO_POINTER(noti->priv_id), __comp_priv_id);
	if (tmp == NULL)
		req_data->priv_id_list = g_list_append(req_data->priv_id_list,
						GINT_TO_POINTER(noti->priv_id));

	new_file_list = __get_new_file_list(noti, req_data->shared_file_list,
							&is_overlapping);
	if (new_file_list != NULL) {
		if (__set_sharing_for_new_file(req_data, new_file_list,
				is_overlapping) != NOTIFICATION_ERROR_NONE) {
			ERR("__set_sharing_for_new_file out");
			ret = NOTIFICATION_ERROR_IO_ERROR;
			goto out;
		}
	}

	/* N3-T4 A1: accumulate dup'd file_info in a local prepend list,
	 * reverse once, and splice into shared_file_list in O(m) instead
	 * of per-iteration O(n) tail walk. */
	{
		GList *new_shared = NULL;

		for (iter = new_file_list; iter != NULL;
				iter = g_list_next(iter)) {
			tmp = NULL;
			file_info = iter->data;
			if (is_overlapping) {
				tmp = g_list_find_custom(
						req_data->shared_file_list,
						file_info, __comp_file_info);
			}

			if (tmp == NULL) {
				dup_file_info = __dup_file_info(file_info);
				if (dup_file_info != NULL) {
					new_shared = g_list_prepend(new_shared,
							dup_file_info);
				}
			}
		}
		new_shared = g_list_reverse(new_shared);
		req_data->shared_file_list = g_list_concat(
				req_data->shared_file_list, new_shared);
	}

	if (__set_sharing_for_new_target(req_data, uid_info->target_app_list)
						!= NOTIFICATION_ERROR_NONE) {
		WARN("__set_sharing_for_new_target out");
		ret = NOTIFICATION_ERROR_IO_ERROR;
		goto out;
	}
	INFO("PS success priv id[%d] shared file count[%d] target app count[%d]",
				noti->priv_id,
				g_list_length(req_data->shared_file_list),
				g_list_length(req_data->target_app_table));

out:
	if (new_file_list != NULL)
		g_list_free_full(new_file_list, __free_file_info);

	if (ret != NOTIFICATION_ERROR_NONE)
		req_data->priv_id_list = g_list_remove(req_data->priv_id_list,
						GINT_TO_POINTER(noti->priv_id));

	WARN("notification_set_private_sharing done");

	return ret;
}

static bool __timeout_handler(void *data)
{
	char **path_array = NULL;
	char *dst_path;
	char *target_appid;
	sharing_req_data_s *req_data;
	private_sharing_req *handle = NULL;
	GList *iter;
	GList *tmp_list;
	uid_info_s *uid_info;
	int ret = SECURITY_MANAGER_SUCCESS;
	int len;

	WARN("__timeout_handler");

	req_data = (sharing_req_data_s *)data;
	if (g_list_length(req_data->priv_id_list) > 0) {
		req_data->timer = NULL;
		return false;
	}

	req_data->drop_retry_count++;

	/* If there is no shared file, the private sharing is not dropped. */
	if (req_data->dir != NULL) {
		__make_sharing_dir(req_data->dir);
		iter = req_data->shared_file_list;
		for (; iter != NULL; iter = g_list_next(iter)) {
			notification_copy_private_file(
				((sharing_file_info_s *)(iter->data))->src_path,
				((sharing_file_info_s *)(iter->data))->dst_path);
		}
	}

	if (g_list_length(req_data->target_app_table) > 0) {
		ret = security_manager_private_sharing_req_new(&handle);
		if (ret != SECURITY_MANAGER_SUCCESS) {
			ERR("Failed to create PS request handle");
			goto out;
		}

		ret = security_manager_private_sharing_req_set_owner_appid(
				handle, req_data->app_id);
		if (ret != SECURITY_MANAGER_SUCCESS) {
			ERR("Failed to set owner appid(%s) %d",
					req_data->app_id, ret);
			goto out;
		}

		path_array = __convert_list_to_array(req_data->shared_file_list, &len);
		if (path_array == NULL) {
			ERR("path_array is null %d",
				g_list_length(req_data->shared_file_list));
			ret = SECURITY_MANAGER_ERROR_MEMORY;
			goto out;
		}

		ret = security_manager_private_sharing_req_add_paths(handle,
						(const char **)path_array, len);
		if (ret != SECURITY_MANAGER_SUCCESS) {
			ERR("Failed to add paths %d", ret);
			goto out;
		}

		iter = req_data->target_app_table;
		while(iter != NULL) {
			target_appid = (char *)iter->data;
			iter = g_list_next(iter);

			ret = security_manager_private_sharing_req_set_target_appid(
						handle, target_appid);
			if (ret != SECURITY_MANAGER_SUCCESS) {
				ERR("Failed to set target appid(%s)", target_appid);
				goto out;
			}

			WARN("drop sharing [%s]", target_appid);
			ret = security_manager_private_sharing_drop(handle);
			if (ret != SECURITY_MANAGER_SUCCESS) {
				ERR("Failed to drop %d", ret);
				goto out;
			}

			req_data->target_app_table = g_list_remove(
					req_data->target_app_table, target_appid);
			free(target_appid);
		}
	}

out:
	if (handle != NULL)
		security_manager_private_sharing_req_free(handle);

	if (path_array)
		free(path_array);

	if (ret != SECURITY_MANAGER_SUCCESS && req_data->drop_retry_count < MAX_RETRY_CNT) {
		ERR("drop_retry_count %d", req_data->drop_retry_count);
		return true;
	}

	if (req_data->dir != NULL) {
		iter = req_data->shared_file_list;
		for (; iter != NULL; iter = g_list_next(iter)) {
			dst_path = ((sharing_file_info_s *)(iter->data))->dst_path;
			if (strncmp(req_data->dir, dst_path, strlen(req_data->dir)) != 0)
				continue;

			if (g_remove(dst_path) != 0)
				ERR("Failed [%s] [%d]", dst_path, errno);
		}
		g_rmdir(req_data->dir);
	}

	tmp_list = g_list_find_custom(__uid_list, GINT_TO_POINTER(req_data->uid),
			__comp_uid_info_list);

	if (tmp_list != NULL) {
		uid_info = tmp_list->data;
		uid_info->sharing_req_list = g_list_remove(
				uid_info->sharing_req_list, req_data);
	}
	__free_req_info(req_data);

	WARN("__timeout_handler done");

	return false;
}

EXPORT_API void notification_remove_private_sharing(
		const char *src_app_id, int priv_id, uid_t uid)
{
	sharing_req_data_s *req_data;
	uid_info_s *uid_info;
	GList *req_list;
	GList *priv_id_info = NULL;
	GList *tmp_list = NULL;
	tizen_core_h core = NULL;
	tizen_core_source_h source = NULL;
	int len;
	int ret;

	WARN("notification_remove_private_sharing [%d]", priv_id);

	tmp_list = g_list_find_custom(__uid_list, GINT_TO_POINTER(uid),
			__comp_uid_info_list);
	if (tmp_list == NULL)
		return;

	uid_info = tmp_list->data;

	req_list = g_list_find_custom(uid_info->sharing_req_list, src_app_id,
			__comp_sharing_req_list);
	if (req_list == NULL)
		return;

	req_data = (sharing_req_data_s *)req_list->data;

	priv_id_info = g_list_find_custom(req_data->priv_id_list,
			GINT_TO_POINTER(priv_id), __comp_priv_id);
	if (priv_id_info == NULL)
		return;

	req_data->priv_id_list = g_list_remove(req_data->priv_id_list,
			priv_id_info->data);
	len = g_list_length(req_data->priv_id_list);
	if (len > 0)
		return;

	if (req_data->timer)
		tizen_core_source_destroy(req_data->timer);

	req_data->timer = NULL;
	req_data->drop_retry_count = 0;
	tizen_core_find_from_this_thread(&core);
	ret = tizen_core_add_timer(core, MAX_TIMEOUT, __timeout_handler,
			req_data, &source);

	req_data->timer = source;
	if (ret != TIZEN_CORE_ERROR_NONE) {
		ERR("Failed to add timer");
		__timeout_handler(req_data);
	}

	WARN("notification_remove_private_sharing done");
}
/* LCOV_EXCL_STOP */
