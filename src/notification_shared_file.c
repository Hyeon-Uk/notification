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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <gio/gio.h>
#include <glib/gstdio.h>

#include <tzplatform_config.h>
#include <security-manager.h>
#include <sys/smack.h>
#include <sys/stat.h>
#include <linux/xattr.h>
#include <sys/types.h>
#include <utime.h>
#include <sys/time.h>
#include <package_manager.h>

#include "notification.h"
#include "notification_debug.h"
#include "notification_shared_file.h"
#include <notification_private.h>


#define DUMMY_PARAM
#define __OOM_CHECK(value, ret_value, free_fun) \
do { \
	if (value == NULL) { \
		NOTIFICATION_ERR("out of memory"); \
		free_fun; \
		return ret_value; \
	} \
} while (0);

GList *__uid_list;
typedef struct uid_info {
	uid_t uid;
	GList *sharing_req_list;
	GList *target_app_list;
} uid_info_s;

typedef struct sharing_req_data {
	char *app_id;
	char *dir;
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
	char *dbus_sender_name;
} target_app_info_s;

static bool __make_sharing_dir(const char *dir)
{
	GFile *noti_dir = NULL;
	GError *g_err = NULL;

	if (access(dir, R_OK) != 0) {
		noti_dir = g_file_new_for_path(dir);
		if (g_file_make_directory(noti_dir, NULL, &g_err) == false) {
			if (g_err) {
				NOTIFICATION_ERR("make_directory fail (%s)",
					g_err->message);
				g_error_free(g_err);
			}
			g_object_unref(noti_dir);
			return false;
		}
		g_object_unref(noti_dir);
	}

	return true;
}

static char *__get_data_path_by_pkg_id(const char *pkg_id,
					const char *file_name, uid_t uid)
{
#define NOTI_PRIV_DATA_DIR "data/.notification"

	const char *path;
	char dir[PATH_MAX];

	tzplatform_set_user(uid);
	path = tzplatform_getenv(TZ_USER_APP);
	tzplatform_reset_user();

	if (pkg_id == NULL || path == NULL)
		return NULL;

	snprintf(dir, sizeof(dir), "%s/%s/%s", path, pkg_id, NOTI_PRIV_DATA_DIR);

	if (__make_sharing_dir(dir) == false)
		return NULL;

	snprintf(dir, sizeof(dir), "%s/%s/%s/%s", path, pkg_id,
						NOTI_PRIV_DATA_DIR, file_name);

	return strdup(dir);
}

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
		for (i = 0; i < search_len; i++)
			if (index[i] != search[i])
				break;

		if (i == search_len)
			return index;
		index--;
	}

	return NULL;
}

/* Check path that include res directory */
static bool __is_private_file(const char *smack_label, const char *pkg_id)
{
	const char *smack_index;
	bool ret = false;

	if (smack_label == NULL || pkg_id == NULL)
		return ret;

	smack_index = __last_index_of(smack_label, pkg_id);
	if (smack_index != NULL && (strlen(smack_index) == strlen(pkg_id)))
		ret = true;

	return ret;
}

static bool __is_RO_file(const char *smack_label)
{
#define CHECK_LABEL "::RO"

	const char *smack_index;
	bool ret = false;

	smack_index = __last_index_of(smack_label, CHECK_LABEL);
	if (smack_index != NULL && (strlen(smack_index) == strlen(CHECK_LABEL)))
		ret = true;

	return ret;
}

/* file copy from /res to /data */
int notification_copy_private_file(const char* src_path,
					const char* dst_path)
{
	int ret = NOTIFICATION_ERROR_NONE;
	GFile *src = NULL;
	GFile *dst = NULL;
	GError *g_err = NULL;
	struct utimbuf ut = {0,};

	dst = g_file_new_for_path(dst_path);
	if (dst == NULL) {
		NOTIFICATION_ERR("des is wrong path [%s]", dst_path);
		ret = NOTIFICATION_ERROR_IO_ERROR;
		goto out;
	}

	if (g_file_query_exists(dst, NULL) == true) {
		ret = NOTIFICATION_ERROR_ALREADY_EXIST_ID;
		NOTIFICATION_INFO("dst_path existed [%s]", dst_path);
		goto out;
	}

	src = g_file_new_for_path(src_path);
	if (src == NULL) {
		NOTIFICATION_ERR("src is wrong path [%s]", src_path);
		ret = NOTIFICATION_ERROR_IO_ERROR;
		goto out;
	}

	if (!g_file_copy(src, dst, G_FILE_COPY_NOFOLLOW_SYMLINKS,
			NULL, NULL, NULL, &g_err)) {
		if (g_err) {
			NOTIFICATION_ERR(
				"Copying file from [%s] to [%s] is failed [%s]",
					 src_path, dst_path, g_err->message);
			g_error_free(g_err);
		}
		ret = NOTIFICATION_ERROR_IO_ERROR;
		goto out;
	}

	ut.modtime = time(NULL);
	if (g_utime(dst_path, &ut) != 0)
		NOTIFICATION_DBG("failed to set g_utime %d ", errno);

out:
	if (src)
		g_object_unref(src);
	if (dst)
		g_object_unref(dst);

	return ret;
}

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

static void __free_req_info(gpointer data)
{
	sharing_req_data_s * info = (sharing_req_data_s *)data;

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

	free(info);
}

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

static gint __comp_file_info(gconstpointer a, gconstpointer b)
{
	sharing_file_info_s *file = (sharing_file_info_s *)a;
	sharing_file_info_s *new_file = (sharing_file_info_s *)b;

	if (!file || !new_file || !new_file->dst_path || !file->dst_path)
		return -1;

	return strcmp(new_file->dst_path, file->dst_path);
}

static gint __comp_dst_path(gconstpointer a, gconstpointer b)
{
	sharing_file_info_s *info = (sharing_file_info_s *)a;
	char *path = (char *)b;

	if (!info || !path || !info->dst_path)
		return -1;

	return strcmp(info->dst_path, path);
}

static sharing_file_info_s * __dup_file_info(sharing_file_info_s *src)
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

	/*
	* If the file copy is successful,
	* ignore the shared_file_list check for private sharing
	*/
	ret = notification_copy_private_file(src_path, dst_path);

	if (ret == NOTIFICATION_ERROR_ALREADY_EXIST_ID) {
		tmp = g_list_find_custom(shared_file_list, dst_path,
				__comp_dst_path);
		if (tmp == NULL) {
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
				NOTIFICATION_ERR("fail to get stat info");
			file_info->modification_time = stat_buf.st_mtime;

			*new_file_list = g_list_append(*new_file_list, file_info);
		} else {
			file_info = (sharing_file_info_s *)tmp->data;

			if (stat(file_info->dst_path, &stat_buf) != 0)
				NOTIFICATION_ERR("fail to get stat info");

			if (file_info->modification_time != stat_buf.st_mtime) {
				dup_file_info = __dup_file_info(file_info);
				__OOM_CHECK(dup_file_info, DUMMY_PARAM, DUMMY_PARAM);

				*new_file_list =  g_list_append(*new_file_list,
						dup_file_info);
				*is_overlapping = true;
			}
		}
	} else if (ret == NOTIFICATION_ERROR_NONE) {
		*is_overlapping = true;
	}

}

static GList* __get_new_file_list(notification_h noti,
				GList *shared_file_list,
				bool *is_overlapping)
{
	GList *new_file_list = NULL;
	char *src_path, *dst_path, *dup_str;
	char buf_key[32] = { 0, };
	int i = NOTIFICATION_IMAGE_TYPE_ICON;


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

	if (noti->b_priv_image_path == NULL)
		return new_file_list;

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
		}
	}

	return new_file_list;
}

static char* __get_shared_dir(notification_h noti)
{
	char *path = NULL;
	char *dir;
	const char *index;
	char buf_key[32] = { 0, };
	int i = NOTIFICATION_IMAGE_TYPE_ICON, dir_len;

	if (noti->b_priv_image_path != NULL) {
		for (; i <= NOTIFICATION_IMAGE_TYPE_MAX; i++) {
			snprintf(buf_key, sizeof(buf_key), "%d", i);
			bundle_get_str(noti->b_priv_image_path, buf_key, &path);
			if (path != NULL)
				break;
		}
	}

	if (path == NULL && noti->priv_sound_path != NULL)
		path = noti->priv_sound_path;
	else if (path == NULL && noti->priv_vibration_path != NULL)
		path = noti->priv_vibration_path;

	if (path == NULL) {
		NOTIFICATION_DBG("failed to find priv path");
		return NULL;
	}

	index =  __last_index_of(path, "/");
	if (index == NULL) {
		NOTIFICATION_ERR("failed to find directory separator");
		return NULL;
	}

	dir_len = index - path + 1;
	if (dir_len <= 0 || dir_len > PATH_MAX)
		return NULL;

	dir = (char *)calloc(dir_len, sizeof(char));
	__OOM_CHECK(dir, NULL, DUMMY_PARAM);

	snprintf(dir, dir_len, "%s", path);

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

	if (!target || !sender || !target->dbus_sender_name)
		return -1;

	return strcmp(sender, target->dbus_sender_name);
}

EXPORT_API void notification_remove_private_sharing_target_id(
				const char *sender, uid_t uid)
{
	target_app_info_s *target_info;
	uid_info_s *uid_info;
	GList *sharing_req_list, *target_app;

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
			free(target_info->dbus_sender_name);
			free(target_info);
			target_info = NULL;
		}
	}
}

EXPORT_API void notification_add_private_sharing_target_id(pid_t pid,
					const char *sender, uid_t uid)
{
	char *app_id;
	target_app_info_s *target_info;
	uid_info_s *uid_info;
	GList *sharing_req_list, *target_app;

	sharing_req_list = g_list_find_custom(__uid_list, GINT_TO_POINTER(uid),
						__comp_uid_info_list);
	if (sharing_req_list == NULL) {
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
		app_id = notification_get_app_id_by_pid((int)pid);
		if (app_id == NULL) {
			NOTIFICATION_ERR("out of memory");
			return;
		}

		target_info = (target_app_info_s *)calloc(1, sizeof(target_app_info_s));
		if (target_info == NULL) {
			NOTIFICATION_ERR("out of memory");
			free(app_id);
			return;
		}

		target_info->app_id = app_id;
		target_info->dbus_sender_name = strdup(sender);
		if (target_info->dbus_sender_name == NULL) {
			NOTIFICATION_ERR("out of memory");
			free(target_info);
			free(app_id);
			return;
		}

		uid_info->target_app_list = g_list_append(
			uid_info->target_app_list, target_info);
	}
}

char *notification_check_file_path_is_private(const char *pkg_id,
							const char *file_path)
{
	char *smack_label = NULL;
	char *dst_path = NULL;
	int size;
	uid_t uid = getuid();

	size = smack_new_label_from_path(file_path, XATTR_NAME_SMACK,
							TRUE, &smack_label);
	if (size <= 0) {
		NOTIFICATION_ERR("fail to get smack info");
		return NULL;
	}

	if (__is_RO_file(smack_label))
		dst_path = __get_data_path_by_pkg_id(pkg_id,
				__last_index_of(file_path, "/") + 1, uid);

	if (dst_path == NULL && __is_private_file(smack_label, pkg_id)) {
		dst_path = strdup(file_path);
		if (dst_path == NULL)
			NOTIFICATION_ERR("out of memory");
	}

	free(smack_label);
	return dst_path;
}

EXPORT_API bool notification_validate_private_sharing(
						notification_h updated_noti)
{
	char *updated_path = NULL;
	char *private_path;
	char buf_key[32] = { 0, };
	int i = NOTIFICATION_IMAGE_TYPE_ICON;

	if (updated_noti->b_image_path && updated_noti->b_priv_image_path) {
		for (; i <= NOTIFICATION_IMAGE_TYPE_MAX; i++) {
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

			updated_path = NULL;
		}
	}

	if (updated_noti->sound_path && updated_noti->priv_sound_path) {
		if (strcmp(updated_noti->sound_path,
					updated_noti->priv_sound_path) == 0)
			return false;
	}

	if (updated_noti->vibration_path && updated_noti->priv_vibration_path) {
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
	char *source_path;
	char *private_path;
	char buf_key[32] = { 0, };
	int i = NOTIFICATION_IMAGE_TYPE_ICON;

	if (updated_noti->b_image_path && updated_noti->b_priv_image_path) {
		for (; i <= NOTIFICATION_IMAGE_TYPE_MAX; i++) {
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
				bundle_get_str(source_noti->b_image_path,
						buf_key, &source_path);
				bundle_del(updated_noti->b_image_path, buf_key);
				bundle_add_str(updated_noti->b_image_path,
						buf_key, source_path);
			}
			updated_path = NULL;
			private_path = NULL;
		}
	}

	if (updated_noti->sound_path && updated_noti->priv_sound_path) {
		if (strcmp(updated_noti->sound_path,
				updated_noti->priv_sound_path) == 0) {
			free(updated_noti->sound_path);
			updated_noti->sound_path = strdup(source_noti->sound_path);
			if (updated_noti->sound_path == NULL)
				NOTIFICATION_ERR("out of memory");
		}
	}

	if (updated_noti->vibration_path && updated_noti->priv_vibration_path) {
		if (strcmp(updated_noti->vibration_path,
				updated_noti->priv_vibration_path) == 0) {
			free(updated_noti->vibration_path);
			updated_noti->vibration_path =
				strdup(source_noti->priv_vibration_path);
			if (updated_noti->vibration_path == NULL)
				NOTIFICATION_ERR("out of memory");
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
	char *app_info;

	iter = target_app_list;
	for (; iter != NULL; iter = g_list_next(iter)) {
		target_info = (target_app_info_s *)iter->data;
		tmp = g_list_find_custom(req_data->target_app_table,
					target_info->app_id, __comp_str);
		if (tmp != NULL)
			continue;

		app_info = strdup(target_info->app_id);
		if (app_info == NULL) {
			NOTIFICATION_ERR("out of memory");
			goto out;
		}

		if (handle == NULL) {
			ret = security_manager_private_sharing_req_new(&handle);
			if (ret != SECURITY_MANAGER_SUCCESS) {
				ret = NOTIFICATION_ERROR_IO_ERROR;
				NOTIFICATION_ERR("Failed to create PS handle");
				goto out;
			}

			ret = security_manager_private_sharing_req_set_owner_appid(
						handle, req_data->app_id);
			if (ret != SECURITY_MANAGER_SUCCESS) {
				ret = NOTIFICATION_ERROR_IO_ERROR;
				NOTIFICATION_ERR(
					"Failed to set owner appid(%s) %d",
						req_data->app_id, ret);
				goto out;
			}
			path_array = __convert_list_to_array(
					req_data->shared_file_list, &len);
			if (path_array == NULL) {
				ret = NOTIFICATION_ERROR_OUT_OF_MEMORY;
				goto out;
			}

			ret = security_manager_private_sharing_req_add_paths(
					handle, (const char **)path_array, len);
			if (ret != SECURITY_MANAGER_SUCCESS) {
				ret = NOTIFICATION_ERROR_IO_ERROR;
				NOTIFICATION_ERR("Failed to add paths %d", ret);
				goto out;
			}
		}

		ret = security_manager_private_sharing_req_set_target_appid(
					handle, target_info->app_id);
		if (ret != SECURITY_MANAGER_SUCCESS) {
			ret = NOTIFICATION_ERROR_IO_ERROR;
			NOTIFICATION_ERR("Failed to set target appid(%s)",
				(const char *)iter->data);
			goto out;
		}

		ret = security_manager_private_sharing_apply(handle);
		if (ret != SECURITY_MANAGER_SUCCESS) {
			ret = NOTIFICATION_ERROR_IO_ERROR;
			NOTIFICATION_ERR("Failed to apply PS %d", ret);
			goto out;
		}

		req_data->target_app_table = g_list_append(
				req_data->target_app_table,
				app_info);
	}

out:
	if (handle != NULL)
		security_manager_private_sharing_req_free(handle);
	if (path_array != NULL)
		free(path_array);

	return ret;
}

int __set_sharing_for_new_file(sharing_req_data_s *req_data,
				GList *new_file_list, bool is_overlapping)
{
	char **path_array = NULL;
	int ret = NOTIFICATION_ERROR_NONE;
	int len;
	private_sharing_req *handle = NULL;
	GList *iter;

	path_array = __convert_list_to_array(new_file_list, &len);
	if (path_array == NULL) {
		ret = NOTIFICATION_ERROR_OUT_OF_MEMORY;
		goto out;
	}

	ret = security_manager_private_sharing_req_new(&handle);
	if (ret != SECURITY_MANAGER_SUCCESS) {
		ret = NOTIFICATION_ERROR_IO_ERROR;
		NOTIFICATION_ERR("Failed to create private sharing request handle");
		goto out;
	}

	ret = security_manager_private_sharing_req_set_owner_appid(handle,
					req_data->app_id);
	if (ret != SECURITY_MANAGER_SUCCESS) {
		NOTIFICATION_ERR("Failed to set owner appid(%s) %d",
				req_data->app_id, ret);
		ret = NOTIFICATION_ERROR_IO_ERROR;
		goto out;
	}

	ret = security_manager_private_sharing_req_add_paths(handle,
						(const char **)path_array, len);
	if (ret != SECURITY_MANAGER_SUCCESS) {
		NOTIFICATION_ERR("Failed to add paths %d", ret);
		ret = NOTIFICATION_ERROR_IO_ERROR;
		goto out;
	}

	if (is_overlapping == true) {
		iter = req_data->target_app_table;
		for (; iter != NULL; iter = g_list_next(iter)) {
			ret = security_manager_private_sharing_req_set_target_appid(
							handle, (const char *)iter->data);
			if (ret != SECURITY_MANAGER_SUCCESS) {
				NOTIFICATION_ERR("Failed to set target appid(%s)",
					(const char *)iter->data);
				ret = NOTIFICATION_ERROR_IO_ERROR;
				goto out;
			}

			ret = security_manager_private_sharing_drop(handle);
			if (ret != SECURITY_MANAGER_SUCCESS) {
				NOTIFICATION_ERR("Failed to drop %d", ret);
				ret = NOTIFICATION_ERROR_IO_ERROR;
				goto out;
			}
		}
	}

	iter = req_data->target_app_table;
	for (; iter != NULL; iter = g_list_next(iter)) {
		ret = security_manager_private_sharing_req_set_target_appid(handle,
							(const char *)iter->data);
		if (ret != SECURITY_MANAGER_SUCCESS) {
				NOTIFICATION_ERR("Failed to set target appid(%s)",
					(const char *)iter->data);
				ret = NOTIFICATION_ERROR_IO_ERROR;
				goto out;
		}

		ret = security_manager_private_sharing_apply(handle);
		if (ret != SECURITY_MANAGER_SUCCESS) {
			NOTIFICATION_ERR("Failed to apply PS %d", ret);
			ret = NOTIFICATION_ERROR_IO_ERROR;
			goto out;
		}
	}

out:
	if (handle != NULL)
		security_manager_private_sharing_req_free(handle);
	if (path_array != NULL)
		free(path_array);

	return ret;
}

EXPORT_API int notification_set_private_sharing(notification_h noti,
						uid_t uid)
{
	int ret = NOTIFICATION_ERROR_NONE;
	char *shared_dir;
	bool is_overlapping = false;
	sharing_req_data_s *req_data;
	sharing_file_info_s *file_info, *dup_file_info;
	uid_info_s *uid_info;
	GList *req_list, *iter, *tmp;
	GList *new_file_list = NULL;

	if (noti == NULL || noti->caller_app_id == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	shared_dir = __get_shared_dir(noti);
	if (shared_dir == NULL)
		return NOTIFICATION_ERROR_NONE;

	tmp = g_list_find_custom(__uid_list, GINT_TO_POINTER(uid),
						__comp_uid_info_list);
	if (tmp == NULL) {
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
		req_data = (sharing_req_data_s *)calloc(1, sizeof(sharing_req_data_s));
		__OOM_CHECK(req_data, NOTIFICATION_ERROR_OUT_OF_MEMORY, DUMMY_PARAM);

		req_data->app_id = strdup(noti->caller_app_id);
		__OOM_CHECK(req_data->app_id, NOTIFICATION_ERROR_OUT_OF_MEMORY,
						__free_req_info(req_data));

		req_data->dir = strdup(shared_dir);
		__OOM_CHECK(req_data->dir, NOTIFICATION_ERROR_OUT_OF_MEMORY,
						__free_req_info(req_data));

		uid_info->sharing_req_list = g_list_append(
			uid_info->sharing_req_list, req_data);
	} else {
		req_data = (sharing_req_data_s *)req_list->data;
	}

	__make_sharing_dir(shared_dir);
	free(shared_dir);

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
			ret = NOTIFICATION_ERROR_IO_ERROR;
			goto out;
		}
	}

	for (iter = new_file_list; iter != NULL; iter = g_list_next(iter)) {
		tmp = NULL;
		file_info = iter->data;
		if (is_overlapping == true)
			tmp = g_list_find_custom(
				req_data->shared_file_list, file_info,
				__comp_file_info);

		if (tmp == NULL) {
			dup_file_info = __dup_file_info(file_info);
			req_data->shared_file_list = g_list_append(
				req_data->shared_file_list, dup_file_info);
		}
	}

	if (__set_sharing_for_new_target(req_data, uid_info->target_app_list)
						 != NOTIFICATION_ERROR_NONE) {
		ret = NOTIFICATION_ERROR_IO_ERROR;
		goto out;
	}
	NOTIFICATION_INFO("PS success [%d] [%d] [%d]", noti->priv_id,
				g_list_length(req_data->shared_file_list),
				g_list_length(req_data->target_app_table));

out:
	if (new_file_list != NULL)
		g_list_free_full(new_file_list, __free_file_info);
	if (ret != NOTIFICATION_ERROR_NONE)
		req_data->priv_id_list = g_list_remove(req_data->priv_id_list,
						GINT_TO_POINTER(noti->priv_id));
	return ret;
}

EXPORT_API void notification_remove_private_sharing(
				const char *src_app_id, int priv_id, uid_t uid)
{
	char **path_array = NULL;
	sharing_req_data_s *req_data;
	private_sharing_req *handle = NULL;
	uid_info_s *uid_info;
	GList *req_list;
	GList *iter;
	GList *priv_id_info = NULL;
	GList *tmp_list = NULL;
	int len, ret;

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

	len = g_list_length(req_data->priv_id_list);
	if (len > 1) {
		req_data->priv_id_list = g_list_remove(req_data->priv_id_list,
					priv_id_info->data);
		return;
	}

	/* If there is no shared file, the private sharing is not dropped. */
	__make_sharing_dir(req_data->dir);
	iter = req_data->shared_file_list;
	for (; iter != NULL; iter = g_list_next(iter))
		notification_copy_private_file(
			((sharing_file_info_s *)(iter->data))->src_path,
			((sharing_file_info_s *)(iter->data))->dst_path);

	if (g_list_length(req_data->target_app_table) > 0) {
		ret = security_manager_private_sharing_req_new(&handle);
		if (ret != SECURITY_MANAGER_SUCCESS) {
			NOTIFICATION_ERR("Failed to create PS request handle");
			goto out;
		}

		ret = security_manager_private_sharing_req_set_owner_appid(
					handle, src_app_id);
		if (ret != SECURITY_MANAGER_SUCCESS) {
			NOTIFICATION_ERR("Failed to set owner appid(%s) %d",
					req_data->app_id, ret);
			goto out;
		}

		path_array = __convert_list_to_array(req_data->shared_file_list, &len);
		if (path_array == NULL) {
			NOTIFICATION_ERR("path_array is null %d",
				g_list_length(req_data->shared_file_list));
			goto out;
		}

		ret = security_manager_private_sharing_req_add_paths(handle,
						(const char **)path_array, len);
		if (ret != SECURITY_MANAGER_SUCCESS) {
			NOTIFICATION_ERR("Failed to add paths %d", ret);
			goto out;
		}

		iter = req_data->target_app_table;
		for (; iter != NULL; iter = g_list_next(iter)) {
			ret = security_manager_private_sharing_req_set_target_appid(
						handle, (const char *)iter->data);
			if (ret != SECURITY_MANAGER_SUCCESS) {
				NOTIFICATION_ERR("Failed to set target appid(%s)",
					(const char *)iter->data);
				goto out;
			}

			ret = security_manager_private_sharing_drop(handle);
			if (ret != SECURITY_MANAGER_SUCCESS) {
				NOTIFICATION_ERR("Failed to drop %d", ret);
				goto out;
			}
		}
	}

	iter = req_data->shared_file_list;
	for (; iter != NULL; iter = g_list_next(iter)) {
		if (g_remove(((sharing_file_info_s *)(iter->data))->dst_path) != 0)
			NOTIFICATION_ERR("failed [%s] [%d]", iter->data, errno);
	}

	g_rmdir(req_data->dir);
	req_data->priv_id_list = g_list_remove(req_data->priv_id_list,
						priv_id_info->data);

	uid_info->sharing_req_list = g_list_remove(uid_info->sharing_req_list,
							req_data);
	__free_req_info(req_data);

out:
	if (handle != NULL)
		security_manager_private_sharing_req_free(handle);

	if (path_array)
		free(path_array);
}

