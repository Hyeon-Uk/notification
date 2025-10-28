/*
 * Copyright (c) 2000 - 2017 Samsung Electronics Co., Ltd. All rights reserved.
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
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <libintl.h>
#include <gio/gio.h>

#include <aul.h>
#include <tizen.h>
#include <bundle.h>
#include <bundle_internal.h>
#include <app_control.h>
#include <app_control_internal.h>
#include <pkgmgr-info.h>

#include <notification.h>
#include <notification_list.h>
#include <notification_debug.h>
#include <notification_private.h>
#include <notification_noti.h>
#include <notification_ongoing.h>
#include <notification_group.h>
#include <notification_internal.h>
#include <notification_shared_file.h>

#include <notification_tidl.h>

#define REGULAR_UID_MIN 5000

#define NOTIFICATION_CTOR __attribute__((constructor))
#define NOTIFICATION_DTOR __attribute__((destructor))

typedef struct _notification_cb_info notification_cb_info_s;
typedef struct _notification_event_cb_info notification_event_cb_info_s;

typedef enum __notification_cb_type {
	NOTIFICATION_CB_NORMAL = 1,
	NOTIFICATION_CB_DETAILED,
} _notification_cb_type_e;

struct _notification_cb_info {
	_notification_cb_type_e cb_type;
	notification_changed_cb changed_cb;
	notification_detailed_changed_cb detailed_changed_cb;
	void *data;
};

struct _notification_event_cb_info {
	int priv_id;
	event_handler_cb cb;
	void *userdata;
};

static GHashTable *_noti_cb_hash = NULL;
static GList *__noti_event_cb_list = NULL;

static GRecMutex __rec_mutex;

static disturb_cb _disturb_callback = NULL;
static void *_disturb_user_data = NULL;

/* for channel */
typedef struct _notification_channel {
	char *app_id;
	char *channel_name;
	bool blockable;
	bool is_blocked;
} notification_channel_s;

NOTIFICATION_CTOR static void __notification_init(void)
{
	g_rec_mutex_init(&__rec_mutex);
}

NOTIFICATION_DTOR static void __notification_fini(void)
{
	g_rec_mutex_clear(&__rec_mutex);
}

static void __notification_mutex_lock(void)
{
	g_rec_mutex_lock(&__rec_mutex);
}

static void __notification_mutex_unlock(void)
{
	g_rec_mutex_unlock(&__rec_mutex);
}

/* LCOV_EXCL_START */
void notification_reset_event_handler_list(void)
{
	GList *iter;
	notification_event_cb_info_s *info;

	if (!__noti_event_cb_list)
		return;

	iter = g_list_first(__noti_event_cb_list);
	while (iter) {
		info = g_list_nth_data(iter, 0);
		if (info)
			notification_tidl_reset_event_handler(info->priv_id);
		iter = g_list_next(iter);
	}
}

static void __free_changed_cb_info(gpointer data)
{
	notification_cb_info_s *noti_cb_info = (notification_cb_info_s *)data;

	if (noti_cb_info)
		free(noti_cb_info);
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
void notification_call_changed_cb_for_uid(notification_op *op_list, int op_num, uid_t uid)
{
	notification_type_e type = NOTIFICATION_TYPE_NOTI;
	GList *noti_cb_list;
	notification_cb_info_s *noti_cb_info;

	if (_noti_cb_hash == NULL)
		return;

	noti_cb_list = (GList *)g_hash_table_lookup(_noti_cb_hash, GUINT_TO_POINTER(uid));
	if (noti_cb_list == NULL)
		return;

	if (op_list == NULL) {
		ERR("Invalid parameter");
		return;
	}

	noti_cb_list = g_list_first(noti_cb_list);
	notification_get_type(op_list->noti, &type);

	for (; noti_cb_list != NULL; noti_cb_list = noti_cb_list->next) {
		noti_cb_info = noti_cb_list->data;

		if (noti_cb_info->cb_type == NOTIFICATION_CB_NORMAL && noti_cb_info->changed_cb)
			noti_cb_info->changed_cb(noti_cb_info->data, type);

		if (noti_cb_info->cb_type == NOTIFICATION_CB_DETAILED && noti_cb_info->detailed_changed_cb) {
			noti_cb_info->detailed_changed_cb(noti_cb_info->data,
					type, op_list, op_num);
		}
	}
}

static gint __priv_id_compare(gconstpointer a, gconstpointer b)
{
	const notification_event_cb_info_s *info = NULL;

	if (!a)
		return -1;

	info = (notification_event_cb_info_s *)a;
	WARN("src id [%d] dst id [%d]", GPOINTER_TO_INT(b), info->priv_id);

	if (info->priv_id == GPOINTER_TO_INT(b))
		return 0;

	return 1;
}

void notification_call_event_handler_cb(notification_h noti, int event_type)
{
	int ret;
	int priv_id;
	GList *find_list;
	notification_event_cb_info_s *info;
	notification_event_cb_info_s dup_info;

	WARN("call event[%d]", event_type);

	if (__noti_event_cb_list == NULL)
		return;

	ret = notification_get_id(noti, NULL, &priv_id);
	if (ret != NOTIFICATION_ERROR_NONE)
		return;

	WARN("noti id : [%d]", priv_id);

	__notification_mutex_lock();

	__noti_event_cb_list = g_list_first(__noti_event_cb_list);
	find_list = g_list_find_custom(__noti_event_cb_list, GINT_TO_POINTER(priv_id),
				       (GCompareFunc)__priv_id_compare);
	if (find_list == NULL) {
		__notification_mutex_unlock();
		return;
	}

	info = g_list_nth_data(find_list, 0);
	dup_info = *info;

	__notification_mutex_unlock();

	WARN("call event callback");

	dup_info.cb(noti, event_type, dup_info.userdata);

	WARN("done");
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
void notification_delete_event_handler_cb(int priv_id)
{
	GList *delete_list;
	notification_event_cb_info_s *info;

	WARN("delete noti id [%d]", priv_id);

	if (__noti_event_cb_list == NULL)
		return;

	__notification_mutex_lock();

	__noti_event_cb_list = g_list_first(__noti_event_cb_list);
	delete_list = g_list_find_custom(__noti_event_cb_list, GINT_TO_POINTER(priv_id),
					 (GCompareFunc)__priv_id_compare);

	if (delete_list == NULL) {
		__notification_mutex_unlock();
		return;
	}

	info = g_list_nth_data(delete_list, 0);
	__noti_event_cb_list = g_list_remove(g_list_first(__noti_event_cb_list), info);

	if (info)
		free(info);

	__notification_mutex_unlock();

	if (__noti_event_cb_list == NULL && _disturb_callback == NULL)
		notification_tidl_event_monitor_fini();

	WARN("done");
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
EXPORT_API int notification_add_deferred_task(
		void (*deferred_task_cb)(void *data), void *user_data)
{
	if (deferred_task_cb == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	return notification_tidl_add_deffered_task(deferred_task_cb, user_data);
}

EXPORT_API int notification_del_deferred_task(
		void (*deferred_task_cb)(void *data))
{
	if (deferred_task_cb == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	return notification_tidl_del_deffered_task(deferred_task_cb);
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
EXPORT_API int notification_resister_changed_cb_for_uid(
		notification_changed_cb callback, void *user_data, uid_t uid)
{
	GList *noti_cb_list;
	notification_cb_info_s *noti_cb_info_new;

	if (callback == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	if (_noti_cb_hash == NULL)
		_noti_cb_hash = g_hash_table_new(g_direct_hash, g_direct_equal);

	noti_cb_info_new = (notification_cb_info_s *)malloc(sizeof(notification_cb_info_s));
	if (noti_cb_info_new == NULL) {
		ERR("Failed to alloc memory");
		return NOTIFICATION_ERROR_OUT_OF_MEMORY;
	}

	noti_cb_info_new->cb_type = NOTIFICATION_CB_NORMAL;
	noti_cb_info_new->changed_cb = callback;
	noti_cb_info_new->detailed_changed_cb = NULL;
	noti_cb_info_new->data = user_data;

	noti_cb_list = g_hash_table_lookup(_noti_cb_hash, GUINT_TO_POINTER(uid));

	if (noti_cb_list == NULL) {
		noti_cb_list = g_list_append(noti_cb_list, noti_cb_info_new);
		g_hash_table_insert(_noti_cb_hash, GUINT_TO_POINTER(uid), noti_cb_list);
	} else {
		noti_cb_list = g_list_append(noti_cb_list, noti_cb_info_new);
	}

	if (notification_tidl_monitor_init() != NOTIFICATION_ERROR_NONE) {
		notification_unresister_changed_cb_for_uid(callback, uid);
		return NOTIFICATION_ERROR_IO_ERROR;
	}

	return NOTIFICATION_ERROR_NONE;
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
EXPORT_API int notification_resister_changed_cb(
		notification_changed_cb callback, void *user_data)
{
	return notification_resister_changed_cb_for_uid(callback, user_data, aul_getuid());
}

static gint _noti_changed_compare(gconstpointer a, gconstpointer b)
{
	const struct _notification_cb_info *info = NULL;

	if (!a)
		return -1;
	info = (notification_cb_info_s *)a;

	if (info->changed_cb == b)
		return 0;

	return 1;
}

EXPORT_API int notification_unresister_changed_cb_for_uid(
		notification_changed_cb callback, uid_t uid)
{
	notification_cb_info_s *noti_cb_info;
	GList *noti_cb_list;
	GList *delete_cb_list;

	if (callback == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	if (_noti_cb_hash == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	noti_cb_list = (GList *)g_hash_table_lookup(_noti_cb_hash, GUINT_TO_POINTER(uid));
	if (noti_cb_list == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	noti_cb_list = g_list_first(noti_cb_list);
	delete_cb_list = g_list_find_custom(noti_cb_list, (gconstpointer)callback,
					    (GCompareFunc)_noti_changed_compare);
	if (delete_cb_list) {
		noti_cb_info = g_list_nth_data(delete_cb_list, 0);
		noti_cb_list = g_list_delete_link(noti_cb_list, delete_cb_list);
		__free_changed_cb_info(noti_cb_info);

		if (noti_cb_list == NULL) {
			g_hash_table_steal(_noti_cb_hash, GUINT_TO_POINTER(uid));
		} else {
			noti_cb_list = g_list_first(noti_cb_list);
			g_hash_table_replace(_noti_cb_hash, GUINT_TO_POINTER(uid), noti_cb_list);
		}

	} else {
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	if (g_hash_table_size(_noti_cb_hash) == 0)
		notification_tidl_monitor_fini();

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_unresister_changed_cb(
		notification_changed_cb callback)
{
	return notification_unresister_changed_cb_for_uid(callback, aul_getuid());
}

EXPORT_API int notification_update_progress(notification_h noti,
		int priv_id,
		double progress)
{
	int ret;
	int input_priv_id;
	char *caller_app_id;
	double input_progress;

	if (priv_id <= NOTIFICATION_PRIV_ID_NONE) {
		if (noti == NULL)
			return NOTIFICATION_ERROR_INVALID_PARAMETER;

		input_priv_id = noti->priv_id;
	} else {
		input_priv_id = priv_id;
	}

	if (noti == NULL)
		caller_app_id = notification_get_app_id_by_pid(getpid());
	else
		caller_app_id = strdup(noti->caller_app_id);

	if (progress < 0.0)
		input_progress = 0.0;
	else if (progress > 1.0)
		input_progress = 1.0;
	else
		input_progress = progress;

	ret = notification_ongoing_update_progress(caller_app_id, input_priv_id,
			input_progress);

	if (caller_app_id)
		free(caller_app_id);

	return ret;
}

EXPORT_API int notification_update_size(notification_h noti,
		int priv_id,
		double size)
{
	int ret;
	int input_priv_id;
	char *caller_app_id;
	double input_size;

	if (priv_id <= NOTIFICATION_PRIV_ID_NONE) {
		if (noti == NULL)
			return NOTIFICATION_ERROR_INVALID_PARAMETER;

		input_priv_id = noti->priv_id;
	} else {
		input_priv_id = priv_id;
	}

	if (noti == NULL)
		caller_app_id = notification_get_app_id_by_pid(getpid());
	else
		caller_app_id = strdup(noti->caller_app_id);

	if (size < 0.0)
		input_size = 0.0;
	else
		input_size = size;

	ret = notification_ongoing_update_size(caller_app_id, input_priv_id,
			input_size);

	if (caller_app_id)
		free(caller_app_id);

	return ret;
}

EXPORT_API int notification_update_content(notification_h noti,
		int priv_id,
		const char *content)
{
	char *caller_app_id;
	int input_priv_id;
	int ret;

	if (priv_id <= NOTIFICATION_PRIV_ID_NONE) {
		if (noti == NULL)
			return NOTIFICATION_ERROR_INVALID_PARAMETER;

		input_priv_id = noti->priv_id;
	} else {
		input_priv_id = priv_id;
	}

	if (noti == NULL)
		caller_app_id = notification_get_app_id_by_pid(getpid());
	else
		caller_app_id = strdup(noti->caller_app_id);

	ret = notification_ongoing_update_content(caller_app_id, input_priv_id,
			content);

	if (caller_app_id)
		free(caller_app_id);

	return ret;
}
/* LCOV_EXCL_STOP */

/* notification_set_icon will be removed */
/* LCOV_EXCL_START */
EXPORT_API int notification_set_icon(notification_h noti,
		const char *icon_path)
{
	return notification_set_image(noti, NOTIFICATION_IMAGE_TYPE_ICON, icon_path);
}
/* LCOV_EXCL_STOP */

/* notification_get_icon will be removed */
/* LCOV_EXCL_START */
EXPORT_API int notification_get_icon(notification_h noti,
		char **icon_path)
{
	int ret = NOTIFICATION_ERROR_NONE;
	char *ret_image_path = NULL;

	ret = notification_get_image(noti, NOTIFICATION_IMAGE_TYPE_ICON,
				&ret_image_path);

	if (ret == NOTIFICATION_ERROR_NONE && icon_path != NULL)
		*icon_path = ret_image_path;

	return ret;
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
EXPORT_API int notification_translate_localized_text(notification_h noti)
{
	int ret = NOTIFICATION_ERROR_NONE;
	char *ret_text = NULL;
	char buf_key[32];
	char *bundle_val = NULL;
	char *new_text;
	bundle *b;
	notification_text_type_e type = NOTIFICATION_TEXT_TYPE_TITLE;

	noti->is_translation = false;

	for (; type <= NOTIFICATION_TEXT_TYPE_MAX; type++) {
		ret = notification_get_text(noti, type, &ret_text);
		if (ret == NOTIFICATION_ERROR_NONE && ret_text) {
			if (noti->b_text == NULL) {
				noti->b_text = bundle_create();
				if (noti->b_text == NULL)
					return NOTIFICATION_ERROR_OUT_OF_MEMORY;
			}

			b = noti->b_text;

			new_text = strdup(ret_text);

			snprintf(buf_key, sizeof(buf_key), "%d", type);
			bundle_get_str(b, buf_key, &bundle_val);
			if (bundle_val != NULL)
				bundle_del(b, buf_key);

			bundle_add_str(b, buf_key, new_text);
			free(new_text);
			new_text = NULL;

			noti->num_format_args = 0;
			bundle_val = NULL;
		}
	}

	noti->is_translation = true;

	return ret;
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
EXPORT_API int notification_set_title(notification_h noti,
		const char *title,
		const char *loc_title)
{
	return notification_set_text(noti, NOTIFICATION_TEXT_TYPE_TITLE,
				title, loc_title,
				NOTIFICATION_VARIABLE_TYPE_NONE);
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
EXPORT_API int notification_get_title(notification_h noti,
		char **title,
		char **loc_title)
{
	int ret;
	char *ret_text = NULL;

	ret = notification_get_text(noti, NOTIFICATION_TEXT_TYPE_TITLE,
				&ret_text);

	if (title != NULL)
		*title = ret_text;

	if (loc_title != NULL)
		*loc_title = NULL;

	return ret;
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
EXPORT_API int notification_set_content(notification_h noti,
		const char *content,
		const char *loc_content)
{
	return notification_set_text(noti, NOTIFICATION_TEXT_TYPE_CONTENT,
				content, loc_content,
				NOTIFICATION_VARIABLE_TYPE_NONE);
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
EXPORT_API int notification_get_content(notification_h noti,
		char **content,
		char **loc_content)
{
	int ret;
	char *ret_text = NULL;

	ret = notification_get_text(noti, NOTIFICATION_TEXT_TYPE_CONTENT,
				&ret_text);

	if (content != NULL)
		*content = ret_text;

	if (loc_content != NULL)
		*loc_content = NULL;

	return ret;
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
EXPORT_API int notification_set_application(notification_h noti,
		const char *app_id)
{
	if (noti == NULL || app_id == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	if (noti->launch_app_id)
		free(noti->launch_app_id);

	noti->launch_app_id = strdup(app_id);

	return NOTIFICATION_ERROR_NONE;
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
EXPORT_API int notification_get_application(notification_h noti,
		char **app_id)
{
	if (noti == NULL || app_id == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	if (noti->launch_app_id)
		*app_id = noti->launch_app_id;
	else
		*app_id = noti->caller_app_id;

	return NOTIFICATION_ERROR_NONE;
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
EXPORT_API int notification_set_args(notification_h noti, bundle *args,
		bundle *group_args)
{
	if (noti == NULL || args == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	if (noti->args)
		bundle_free(noti->args);

	noti->args = bundle_dup(args);

	if (noti->group_args) {
		bundle_free(noti->group_args);
		noti->group_args = NULL;
	}

	if (group_args != NULL)
		noti->group_args = bundle_dup(group_args);

	return NOTIFICATION_ERROR_NONE;
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
EXPORT_API int notification_get_args(notification_h noti,
		bundle **args,
		bundle **group_args)
{
	if (noti == NULL || args == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	if (noti->args)
		*args = noti->args;
	else
		*args = NULL;

	if (group_args != NULL && noti->group_args)
		*group_args = noti->group_args;

	return NOTIFICATION_ERROR_NONE;
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
int notification_get_grouping_list_for_uid(notification_type_e type, int count,
		notification_list_h *list, uid_t uid)
{
	notification_list_h get_list = NULL;
	int ret = 0;

	if (list == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	ret = notification_noti_get_grouping_list(type, 1, count, &get_list, NULL, uid);
	if (ret != NOTIFICATION_ERROR_NONE)
		return ret;

	*list = get_list;

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_get_grouping_list(notification_type_e type, int count,
		notification_list_h *list)
{
	return notification_get_grouping_list_for_uid(type, count, list, aul_getuid());
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
EXPORT_API int notification_delete_group_by_group_id(const char *app_id,
		notification_type_e type,
		int group_id)
{
	int ret;
	char *caller_app_id;

	if (app_id == NULL)
		caller_app_id = notification_get_app_id_by_pid(getpid());
	else
		caller_app_id = strdup(app_id);

	ret = notification_tidl_request_delete_multiple(type, caller_app_id, aul_getuid());

	if (caller_app_id)
		free(caller_app_id);

	return ret;
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
int notification_delete_group_by_priv_id_for_uid(const char *app_id,
		notification_type_e type,
		int priv_id, uid_t uid)
{
	int ret;
	char *caller_app_id;

	if (app_id == NULL)
		caller_app_id = notification_get_app_id_by_pid(getpid());
	else
		caller_app_id = strdup(app_id);

	ret = notification_tidl_request_delete_single(caller_app_id, priv_id, uid);

	if (caller_app_id)
		free(caller_app_id);

	return ret;
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
EXPORT_API int notification_delete_group_by_priv_id(const char *app_id,
		notification_type_e type,
		int priv_id)
{
	return notification_delete_group_by_priv_id_for_uid(app_id, type, priv_id, aul_getuid());
}

int notification_get_count_for_uid(notification_type_e type,
		const char *app_id,
		int group_id,
		int priv_id, int *count,
		uid_t uid)
{
	int ret;
	char *caller_app_id;

	if (count == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	if (app_id == NULL)
		caller_app_id = notification_get_app_id_by_pid(getpid());
	else
		caller_app_id = strdup(app_id);

	ret = notification_tidl_request_get_count(
			type,
			caller_app_id,
			group_id,
			priv_id,
			count,
			uid);

	if (caller_app_id)
		free(caller_app_id);

	return ret;
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
EXPORT_API int notification_get_count(notification_type_e type,
		const char *app_id,
		int group_id,
		int priv_id, int *count)
{
	return notification_get_count_for_uid(type, app_id, group_id, priv_id, count, aul_getuid());
}

int notification_clear_for_uid(notification_type_e type, uid_t uid)
{
	int ret;

	if (type <= NOTIFICATION_TYPE_NONE || type > NOTIFICATION_TYPE_MAX)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	ret = notification_tidl_request_delete_multiple(type, NULL, uid);

	return ret;
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
EXPORT_API int notification_clear(notification_type_e type)
{
	return notification_clear_for_uid(type, aul_getuid());
}

EXPORT_API int notification_op_get_data(notification_op *noti_op, notification_op_data_type_e type,
		void *data)
{
	if (noti_op == NULL || data == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	switch (type) {
	case NOTIFICATION_OP_DATA_TYPE:
		*((int *)data) = noti_op->type;
		break;
	case NOTIFICATION_OP_DATA_PRIV_ID:
		*((int *)data) = noti_op->priv_id;
		break;
	case NOTIFICATION_OP_DATA_NOTI:
		*((notification_h *)data) = noti_op->noti;
		break;
	case NOTIFICATION_OP_DATA_EXTRA_INFO_1:
		*((int *)data) = noti_op->extra_info_1;
		break;
	case NOTIFICATION_OP_DATA_EXTRA_INFO_2:
		*((int *)data) = noti_op->extra_info_2;
		break;
	default:
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	return NOTIFICATION_ERROR_NONE;
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
EXPORT_API int notification_set_pkgname(notification_h noti,
		const char *pkgname)
{
	return notification_set_app_id(noti, pkgname);
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
EXPORT_API int notification_set_app_id(notification_h noti,
		const char *app_id)
{
	if (noti == NULL || app_id == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	/* Remove previous caller app_id */
	if (noti->caller_app_id) {
		free(noti->caller_app_id);
		noti->caller_app_id = NULL;
	}

	noti->caller_app_id = strdup(app_id);

	return NOTIFICATION_ERROR_NONE;
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
int notification_delete_all_by_type_for_uid(const char *app_id,
		notification_type_e type, uid_t uid)
{
	int ret;
	char *caller_app_id;

	if (type <= NOTIFICATION_TYPE_NONE || type > NOTIFICATION_TYPE_MAX)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	if (app_id == NULL)
		caller_app_id = notification_get_app_id_by_pid(getpid());
	else
		caller_app_id = strdup(app_id);

	ret = notification_tidl_request_delete_multiple(type, caller_app_id, uid);

	if (caller_app_id)
		free(caller_app_id);

	return ret;
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
EXPORT_API int notification_delete_all_by_type(const char *app_id,
		notification_type_e type)
{
	return notification_delete_all_by_type_for_uid(app_id, type, aul_getuid());
}

int notification_delete_by_priv_id_for_uid(const char *app_id,
		notification_type_e type,
		int priv_id,
		uid_t uid)
{
	int ret;
	char *caller_app_id;

	if (priv_id <= NOTIFICATION_PRIV_ID_NONE)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	if (app_id == NULL)
		caller_app_id = notification_get_app_id_by_pid(getpid());
	else
		caller_app_id = strdup(app_id);

	ret = notification_tidl_request_delete_single(caller_app_id, priv_id, uid);

	if (caller_app_id)
		free(caller_app_id);

	return ret;
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
EXPORT_API int notification_delete_by_priv_id(const char *app_id,
		notification_type_e type,
		int priv_id)
{
	return notification_delete_by_priv_id_for_uid(app_id, type, priv_id, aul_getuid());
}

EXPORT_API int notification_set_execute_option(notification_h noti,
		notification_execute_type_e type,
		const char *text,
		const char *key,
		bundle *service_handle)
{
	char buf_key[32] = { 0, };
	char *ret_val = NULL;
	bundle *b = NULL;

	if (noti == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	if (type <= NOTIFICATION_EXECUTE_TYPE_NONE
			|| type > NOTIFICATION_EXECUTE_TYPE_MAX)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	if (noti->b_execute_option == NULL)
		noti->b_execute_option = bundle_create();

	b = noti->b_execute_option;

	if (text != NULL) {
		snprintf(buf_key, sizeof(buf_key), "text%d", type);

		bundle_get_str(b, buf_key, &ret_val);
		if (ret_val != NULL)
			bundle_del(b, buf_key);

		bundle_add_str(b, buf_key, text);
	}

	if (key != NULL) {
		snprintf(buf_key, sizeof(buf_key), "key%d", type);

		bundle_get_str(b, buf_key, &ret_val);
		if (ret_val != NULL)
			bundle_del(b, buf_key);

		bundle_add_str(b, buf_key, key);
	}

	switch ((int)type) {
	case NOTIFICATION_EXECUTE_TYPE_RESPONDING:
		if (noti->b_service_responding != NULL) {
			bundle_free(noti->b_service_responding);
			noti->b_service_responding = NULL;
		}

		if (service_handle != NULL)
			noti->b_service_responding = bundle_dup(service_handle);

		break;
	case NOTIFICATION_EXECUTE_TYPE_SINGLE_LAUNCH:
		if (noti->b_service_single_launch != NULL) {
			bundle_free(noti->b_service_single_launch);
			noti->b_service_single_launch = NULL;
		}

		if (service_handle != NULL)
			noti->b_service_single_launch =
				bundle_dup(service_handle);

		break;
	case NOTIFICATION_EXECUTE_TYPE_MULTI_LAUNCH:
		if (noti->b_service_multi_launch != NULL) {
			bundle_free(noti->b_service_multi_launch);
			noti->b_service_multi_launch = NULL;
		}

		if (service_handle != NULL)
			noti->b_service_multi_launch =
				bundle_dup(service_handle);

		break;
	}

	return NOTIFICATION_ERROR_NONE;
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
EXPORT_API int notification_get_id(notification_h noti,
		int *group_id, int *priv_id)
{
	if (noti == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	if (group_id) {
		if (noti->group_id < NOTIFICATION_GROUP_ID_NONE)
			*group_id = NOTIFICATION_GROUP_ID_NONE;
		else
			*group_id = noti->group_id;
	}

	if (priv_id)
		*priv_id = noti->priv_id;

	return NOTIFICATION_ERROR_NONE;
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
EXPORT_API int notification_set_priv_id(notification_h noti, int priv_id)
{
	if (noti == NULL || priv_id <= 0)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	noti->priv_id = priv_id;

	return NOTIFICATION_ERROR_NONE;
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
notification_h notification_load_for_uid(char *app_id,
		int priv_id, uid_t uid)
{
	int ret;
	notification_h noti;

	ret = notification_tidl_request_load_noti_by_priv_id(&noti, app_id, priv_id, uid);
	if (ret != NOTIFICATION_ERROR_NONE) {
		set_last_result(ret);
		__notification_mutex_unlock();
		return NULL;
	}

	set_last_result(NOTIFICATION_ERROR_NONE);

	return noti;
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
EXPORT_API notification_h notification_load(char *app_id,
		int priv_id)
{
	return notification_load_for_uid(app_id, priv_id, aul_getuid());
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
EXPORT_API notification_h notification_new(notification_type_e type,
		int group_id, int priv_id)
{
	return notification_create(type);
}

static void _notification_get_text_domain(notification_h noti)
{
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
EXPORT_API int notification_get_execute_option(notification_h noti,
		notification_execute_type_e type,
		const char **text,
		bundle **service_handle)
{
	char buf_key[32] = { 0, };
	char *ret_val = NULL;
	char *get_str = NULL;
	bundle *b = NULL;

	if (noti == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	if (type <= NOTIFICATION_EXECUTE_TYPE_NONE
			|| type > NOTIFICATION_EXECUTE_TYPE_MAX)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	switch (type) {
	case NOTIFICATION_EXECUTE_TYPE_RESPONDING:
		b = noti->b_service_responding;
		break;
	case NOTIFICATION_EXECUTE_TYPE_SINGLE_LAUNCH:
		b = noti->b_service_single_launch;
		break;
	case NOTIFICATION_EXECUTE_TYPE_MULTI_LAUNCH:
		b = noti->b_service_multi_launch;
		break;
	default:
		break;
	}

	if (b != NULL) {
		if (text != NULL) {
			if (noti->domain == NULL || noti->dir == NULL)
				_notification_get_text_domain(noti);

			snprintf(buf_key, sizeof(buf_key), "key%d", type);

			bundle_get_str(b, buf_key, &ret_val);
			if (ret_val != NULL && noti->domain != NULL
					&& noti->dir != NULL) {
				bindtextdomain(noti->domain, noti->dir);

				get_str = dgettext(noti->domain, ret_val);

				*text = get_str;
			} else if (ret_val != NULL) {
				get_str = dgettext("sys_string", ret_val);

				*text = get_str;
			} else {
				snprintf(buf_key, sizeof(buf_key), "text%d",
						type);

				bundle_get_str(b, buf_key, &ret_val);

				*text = ret_val;
			}
		}
	}

	if (service_handle != NULL)
		*service_handle = b;

	return NOTIFICATION_ERROR_NONE;
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
EXPORT_API int notification_insert_for_uid(notification_h noti,
		int *priv_id, uid_t uid)
{
	int ret;
	int id;
	bool event_flag;

	if (noti == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	if (noti->type <= NOTIFICATION_TYPE_NONE
			|| noti->type > NOTIFICATION_TYPE_MAX)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	noti->uid = uid;
	noti->insert_time = time(NULL);

	ret = notification_tidl_request_insert(noti, &id);
	if (ret != NOTIFICATION_ERROR_NONE)
		return ret;

	noti->priv_id = id;

	/* If priv_id is valid data, set priv_id */
	if (priv_id != NULL)
		*priv_id = noti->priv_id;

	ret = notification_get_event_flag(noti, &event_flag);
	if (ret != NOTIFICATION_ERROR_NONE)
		return ret;

	if (event_flag == true) {
		ret = notification_tidl_event_monitor_init();
		if (ret != NOTIFICATION_ERROR_NONE)
			return ret;
	}

	return ret;
}

EXPORT_API int notification_insert(notification_h noti,
		int *priv_id)
{
	return notification_insert_for_uid(noti, priv_id, aul_getuid());
}

EXPORT_API int notification_update_async_for_uid(notification_h noti,
		void (*result_cb)(int priv_id, int result, void *data), void *user_data, uid_t uid)
{
	int ret;

	if (noti == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	noti->uid = uid;
	/* Update insert time ? */
	noti->insert_time = time(NULL);

	ret = notification_tidl_request_update(noti);

	return ret;
}

EXPORT_API int notification_update_async(notification_h noti,
		void (*result_cb)(int priv_id, int result, void *data), void *user_data)
{
	return notification_update_async_for_uid(noti, result_cb, user_data, aul_getuid());
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
EXPORT_API int notification_register_detailed_changed_cb_for_uid(
		notification_detailed_changed_cb callback,
		void *user_data, uid_t uid)
{
	GList *noti_cb_list = NULL;
	notification_cb_info_s *noti_cb_info_new = NULL;

	if (callback == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	if (_noti_cb_hash == NULL)
		_noti_cb_hash = g_hash_table_new(g_direct_hash, g_direct_equal);

	noti_cb_info_new = (notification_cb_info_s *)malloc(sizeof(notification_cb_info_s));
	if (noti_cb_info_new == NULL) {
		ERR("Failed to alloc memory");
		return NOTIFICATION_ERROR_OUT_OF_MEMORY;
	}

	noti_cb_info_new->cb_type = NOTIFICATION_CB_DETAILED;
	noti_cb_info_new->changed_cb = NULL;
	noti_cb_info_new->detailed_changed_cb = callback;
	noti_cb_info_new->data = user_data;

	noti_cb_list = g_hash_table_lookup(_noti_cb_hash, GUINT_TO_POINTER(uid));

	if (noti_cb_list == NULL) {
		noti_cb_list = g_list_append(noti_cb_list, noti_cb_info_new);
		g_hash_table_insert(_noti_cb_hash, GUINT_TO_POINTER(uid), noti_cb_list);
	} else {
		noti_cb_list = g_list_append(noti_cb_list, noti_cb_info_new);
	}

	if (notification_tidl_monitor_init() != NOTIFICATION_ERROR_NONE) {
		notification_unregister_detailed_changed_cb_for_uid(callback, user_data, uid);
		return NOTIFICATION_ERROR_IO_ERROR;
	}

	return NOTIFICATION_ERROR_NONE;
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
EXPORT_API int notification_register_detailed_changed_cb(
		notification_detailed_changed_cb callback, void *user_data)
{
	return notification_register_detailed_changed_cb_for_uid(callback, user_data, aul_getuid());
}

static gint _noti_detailed_changed_compare(gconstpointer a, gconstpointer b)
{
	const struct _notification_cb_info *info = NULL;

	if (!a)
		return -1;
	info = (notification_cb_info_s *)a;

	if (info->detailed_changed_cb == b)
		return 0;

	return 1;
}

EXPORT_API int notification_unregister_detailed_changed_cb_for_uid(
		notification_detailed_changed_cb callback, void *user_data, uid_t uid)
{
	notification_cb_info_s *noti_cb_info;
	GList *noti_cb_list;
	GList *delete_cb_list;

	if (callback == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	if (_noti_cb_hash == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	noti_cb_list = (GList *)g_hash_table_lookup(_noti_cb_hash, GUINT_TO_POINTER(uid));

	if (noti_cb_list == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	noti_cb_list = g_list_first(noti_cb_list);
	delete_cb_list = g_list_find_custom(noti_cb_list, (gconstpointer)callback,
					    (GCompareFunc)_noti_detailed_changed_compare);

	if (delete_cb_list) {
		noti_cb_info = (notification_cb_info_s *)g_list_nth_data(delete_cb_list, 0);
		noti_cb_list = g_list_delete_link(noti_cb_list, delete_cb_list);
		__free_changed_cb_info(noti_cb_info);
		if (noti_cb_list == NULL) {
			g_hash_table_steal(_noti_cb_hash, GUINT_TO_POINTER(uid));
		} else {
			noti_cb_list = g_list_first(noti_cb_list);
			g_hash_table_replace(_noti_cb_hash, GUINT_TO_POINTER(uid), noti_cb_list);
		}

	} else {
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	if (g_hash_table_size(_noti_cb_hash) == 0)
		notification_tidl_monitor_fini();

	return NOTIFICATION_ERROR_NONE;

}

EXPORT_API int notification_unregister_detailed_changed_cb(
		notification_detailed_changed_cb callback, void *user_data)
{
	return notification_unregister_detailed_changed_cb_for_uid(
			callback, user_data, aul_getuid());
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
EXPORT_API int notification_is_service_ready(void)
{
	return notification_tidl_is_master_ready();
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
EXPORT_API int notification_set_uid(notification_h noti,
		uid_t uid)
{
	if (noti == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	noti->uid = uid;
	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_get_uid(notification_h noti,
		uid_t *uid)
{
	if (noti == NULL || uid == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	*uid = noti->uid;
	return NOTIFICATION_ERROR_NONE;
}

static GList *__copy_private_file(notification_h noti)
{
	bundle *dst_b;
	bundle *src_b;
	char buf_key[32];
	char *src_path;
	char *dst_path;
	int i;
	GList *file_list = NULL;
	int ret;

	if (noti->b_priv_image_path && noti->b_image_path) {
		dst_b = noti->b_priv_image_path;
		src_b = noti->b_image_path;
		for (i = NOTIFICATION_IMAGE_TYPE_ICON; i <= NOTIFICATION_IMAGE_TYPE_MAX; i++) {
			src_path = NULL;
			dst_path = NULL;

			snprintf(buf_key, sizeof(buf_key), "%d", i);

			bundle_get_str(dst_b, buf_key, &dst_path);
			if (dst_path == NULL)
				continue;

			bundle_get_str(src_b, buf_key, &src_path);
			if (src_path == NULL)
				continue;

			ret = notification_copy_private_file(src_path, dst_path);
			if (ret == NOTIFICATION_ERROR_NONE)
				file_list = g_list_append(file_list, strdup(dst_path));
		}
	}
	if (noti->sound_path && noti->priv_sound_path) {
		ret = notification_copy_private_file(noti->sound_path,
						     noti->priv_sound_path);
		if (ret == NOTIFICATION_ERROR_NONE)
			file_list = g_list_append(file_list,
						  strdup(noti->priv_sound_path));
	}

	if (noti->vibration_path && noti->priv_vibration_path) {
		ret = notification_copy_private_file(noti->vibration_path,
						     noti->priv_vibration_path);
		if (ret == NOTIFICATION_ERROR_NONE)
			file_list = g_list_append(file_list,
						  strdup(noti->priv_vibration_path));
	}

	return file_list;
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
static void __remove_private_file(gpointer data, gpointer user_data)
{
	GFile *src = NULL;
	char *path = (char *)data;

	src = g_file_new_for_path(path);
	if (src) {
		g_file_delete(src, NULL, NULL);
		g_object_unref(src);
	}
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
EXPORT_API int notification_post_for_uid(notification_h noti, uid_t uid)
{
	int ret = 0;
	int id = 0;
	GList *file_list;
	bool event_flag;

	if (noti == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	if (noti->type <= NOTIFICATION_TYPE_NONE
			|| noti->type > NOTIFICATION_TYPE_MAX)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	/* Save insert time */
	noti->insert_time = time(NULL);
	noti->uid = uid;

	INFO("notification post [%d]", (int)uid);

	file_list = __copy_private_file(noti);
	ret = notification_tidl_request_insert(noti, &id);
	if (ret == NOTIFICATION_ERROR_NONE) {
		noti->priv_id = id;
		INFO("Posted notification id[%d]", id);

		ret = notification_get_event_flag(noti, &event_flag);
		if (ret != NOTIFICATION_ERROR_NONE)
			goto out;

		if (event_flag == true) {
			ret = notification_tidl_event_monitor_init();
			if (ret != NOTIFICATION_ERROR_NONE)
				goto out;
		}
	} else {
		g_list_foreach(file_list, __remove_private_file, NULL);
	}

out:
	if (file_list)
		g_list_free_full(file_list, free);

	return ret;
}

EXPORT_API int notification_update_for_uid(notification_h noti, uid_t uid)
{
	int ret;

	if (noti == NULL) {
		WARN("updated notification noti null, uid : %d", uid);
		notification_tidl_request_refresh(uid);
		__notification_mutex_unlock();
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	noti->uid = uid;
	/* Update insert time ? */
	noti->insert_time = time(NULL);
	WARN("updated notification id[%d]", noti->priv_id);

	ret = notification_tidl_request_update(noti);

	return ret;
}

EXPORT_API int notification_delete_for_uid(notification_h noti, uid_t uid)
{
	if (noti == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	int ret;

	ret = notification_tidl_request_delete_single(noti->caller_app_id, noti->priv_id, uid);

	return ret;
}

EXPORT_API int notification_delete_all_for_uid(notification_type_e type, uid_t uid)
{
	int ret = 0;
	char *caller_app_id = NULL;

	if (type <= NOTIFICATION_TYPE_NONE || type > NOTIFICATION_TYPE_MAX)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	caller_app_id = notification_get_app_id_by_pid(getpid());

	ret = notification_tidl_request_delete_multiple(type, caller_app_id, uid);

	if (caller_app_id)
		free(caller_app_id);

	return ret;
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
EXPORT_API notification_h notification_load_by_tag_for_uid(const char *tag, uid_t uid)
{
	int ret;
	notification_h noti;
	char *caller_app_id;

	if (tag == NULL) {
		ERR("Invalid tag");
		set_last_result(NOTIFICATION_ERROR_INVALID_PARAMETER);
		return NULL;
	}

	caller_app_id = notification_get_app_id_by_pid(getpid());
	if (!caller_app_id) {
		ERR("Failed to get a package name");
		set_last_result(NOTIFICATION_ERROR_OUT_OF_MEMORY);
		return NULL;
	}

	ret = notification_tidl_request_load_noti_by_tag(&noti, caller_app_id, tag, uid);
	free(caller_app_id);
	set_last_result(ret);
	if (ret != NOTIFICATION_ERROR_NONE) {
		__notification_mutex_unlock();
		return NULL;
	}

	return noti;
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
EXPORT_API notification_h notification_create_from_package_template(const char *app_id, const char *template_name)
{
	int ret;
	notification_h noti = NULL;

	if (app_id == NULL || template_name == NULL) {
		ERR("Invalid parameter");
		set_last_result(NOTIFICATION_ERROR_INVALID_PARAMETER);
		return NULL;
	}

	ret = notification_tidl_request_create_from_package_template(&noti, app_id, template_name);
	set_last_result(ret);
	if (ret != NOTIFICATION_ERROR_NONE) {
		notification_free(noti);
		return NULL;
	}

	return noti;
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
EXPORT_API int notification_set_default_button(notification_h noti, notification_button_index_e index)
{
	if (noti == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	if (index < 0 || index > NOTIFICATION_BUTTON_6)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	noti->default_button_index = index;

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_get_default_button(notification_h noti, notification_button_index_e *index)
{
	if (noti == NULL || index == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	*index = noti->default_button_index;

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_get_ongoing_value_type(notification_h noti, notification_ongoing_value_type_e *type)
{
	if (noti == NULL || type == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	*type = noti->ongoing_value_type;

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_set_ongoing_value_type(notification_h noti, notification_ongoing_value_type_e type)
{
	if (noti == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	if (type < NOTIFICATION_ONGOING_VALUE_TYPE_PERCENT || type > NOTIFICATION_ONGOING_VALUE_TYPE_TIME)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	noti->ongoing_value_type = type;

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_get_ongoing_time(notification_h noti, int *current, int *duration)
{
	if (noti == NULL || current == NULL || duration == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	*current = noti->ongoing_current;
	*duration = noti->ongoing_duration;

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_set_ongoing_time(notification_h noti, int current, int duration)
{
	if (noti == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	if (current < 0 || duration < 0 || current > duration)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	noti->ongoing_current = current;
	noti->ongoing_duration = duration;

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_get_hide_timeout(notification_h noti, int *timeout)
{
	if (noti == NULL || timeout == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	*timeout = noti->hide_timeout;

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_set_hide_timeout(notification_h noti, int timeout)
{
	if (noti == NULL || timeout < 0)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	noti->hide_timeout = timeout;

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_get_delete_timeout(notification_h noti, int *timeout)
{
	if (noti == NULL || timeout == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	*timeout = noti->delete_timeout;

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_set_delete_timeout(notification_h noti, int timeout)
{
	if (noti == NULL || timeout < 0)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	noti->delete_timeout = timeout;

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_get_text_input_max_length(notification_h noti, int *text_input_max_length)
{
	if (noti == NULL || text_input_max_length == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	*text_input_max_length = noti->text_input_max_length;

	return NOTIFICATION_ERROR_NONE;
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
EXPORT_API int notification_post_with_event_cb_for_uid(notification_h noti, event_handler_cb cb,
							void *userdata, uid_t uid)
{
	int ret;
	int priv_id;
	notification_event_cb_info_s *info = NULL;
	GList *find_list;
	GList *file_list;

	if (noti == NULL || cb == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	if (noti->type <= NOTIFICATION_TYPE_NONE || noti->type > NOTIFICATION_TYPE_MAX)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	noti->insert_time = time(NULL);
	noti->event_flag = true;
	noti->uid = uid;

	file_list = __copy_private_file(noti);
	ret = notification_tidl_request_insert(noti, &priv_id);
	if (ret != NOTIFICATION_ERROR_NONE) {
		ERR("Failed to post notification id[%d]", priv_id);
		g_list_foreach(file_list, __remove_private_file, NULL);
		goto out;
	}

	noti->priv_id = priv_id;

	WARN("Posted notification id[%d]", priv_id);

	ret = notification_tidl_event_monitor_init();
	if (ret != NOTIFICATION_ERROR_NONE)
		goto out;

	__notification_mutex_lock();

	__noti_event_cb_list = g_list_first(__noti_event_cb_list);
	find_list = g_list_find_custom(__noti_event_cb_list, GINT_TO_POINTER(priv_id),
				       (GCompareFunc)__priv_id_compare);

	if (find_list) {
		info = g_list_nth_data(find_list, 0);
		info->cb = cb;
		info->userdata = userdata;
		WARN("event cb is existed id[%d]", priv_id);
	} else {
		info = (notification_event_cb_info_s *)malloc(sizeof(notification_event_cb_info_s));
		if (info == NULL) {
			ERR("Failed to alloc memory");
			ret = NOTIFICATION_ERROR_OUT_OF_MEMORY;
			goto out;
		}
		info->priv_id = priv_id;
		info->cb = cb;
		info->userdata = userdata;
		__noti_event_cb_list = g_list_append(__noti_event_cb_list, info);
		WARN("new event cb appended id[%d]", priv_id);
	}

out:
	if (file_list)
		g_list_free_full(file_list, free);

	__notification_mutex_unlock();

	return ret;
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
EXPORT_API int notification_post_with_event_cb(notification_h noti, event_handler_cb cb, void *userdata)
{
	return notification_post_with_event_cb_for_uid(noti, cb, userdata, aul_getuid());
}

EXPORT_API int notification_send_event(notification_h noti, int event_type)
{
	int ret;
	bool event_flag;

	if (noti == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	if (!((event_type >= NOTIFICATION_EVENT_TYPE_CLICK_ON_BUTTON_1
		&& event_type <= NOTIFICATION_EVENT_TYPE_MAX) ||
		(event_type >= NOTIFICATION_EVENT_TYPE_HIDDEN_BY_USER
		&& event_type <= NOTIFICATION_EVENT_TYPE_HIDDEN_BY_EXTERNAL) ||
		(event_type >= NOTIFICATION_EVENT_TYPE_PRESSED
		&& event_type <= NOTIFICATION_EVENT_TYPE_DELETED) ||
		(event_type == NOTIFICATION_EVENT_TYPE_CHECK_BOX)))
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	ret = notification_get_event_flag(noti, &event_flag);
	if (ret != NOTIFICATION_ERROR_NONE || event_flag == false)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	ret = notification_tidl_send_event(noti, event_type, -1);

	return ret;
}

EXPORT_API int notification_send_event_by_priv_id(int priv_id, int event_type)
{
	int ret;

	if (priv_id <= 0)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	if (!((event_type >= NOTIFICATION_EVENT_TYPE_CLICK_ON_BUTTON_1
		&& event_type <= NOTIFICATION_EVENT_TYPE_MAX) ||
		(event_type >= NOTIFICATION_EVENT_TYPE_HIDDEN_BY_USER
		&& event_type <= NOTIFICATION_EVENT_TYPE_HIDDEN_BY_EXTERNAL) ||
		(event_type >= NOTIFICATION_EVENT_TYPE_PRESSED
		&& event_type <= NOTIFICATION_EVENT_TYPE_DELETED) ||
		(event_type == NOTIFICATION_EVENT_TYPE_CHECK_BOX)))
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	ret = notification_tidl_send_event(NULL, event_type, priv_id);

	return ret;
}

EXPORT_API int notification_get_event_flag(notification_h noti, bool *flag)
{
	if (noti == NULL || flag == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	*flag = noti->event_flag;

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_check_event_receiver_available(notification_h noti, bool *available)
{
	int ret;
	int priv_id;

	if (noti == NULL || available == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	ret = notification_get_id(noti, NULL, &priv_id);
	if (ret != NOTIFICATION_ERROR_NONE) {
		ERR("Failed to get priv id");
		return ret;
	}

	ret = notification_tidl_check_event_receiver(priv_id, available);

	return ret;
}

static bundle *_create_bundle_from_bundle_raw(bundle_raw *string)
{
	if (string == NULL || string[0] == '\0')
		return NULL;

	return bundle_decode(string, strlen((char *)string));
}

EXPORT_API int notification_set_extention_data(notification_h noti, const char *key, bundle *value)
{
	return notification_set_extension_data(noti, key, value);
}

EXPORT_API int notification_set_extension_data(notification_h noti, const char *key, bundle *value)
{
	int ret;
	int len = 0;
	char *del = NULL;
	bundle_raw *raw = NULL;

	if (noti == NULL || key == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	if (noti->args == NULL)
		noti->args = bundle_create();

	if (value == NULL) {
		ret = bundle_del(noti->args, key);
		if (ret == BUNDLE_ERROR_NONE)
			return NOTIFICATION_ERROR_NONE;
		else
			return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	bundle_get_str(noti->args, key, &del);
	if (del != NULL) {
		bundle_del(noti->args, key);
		del = NULL;
	}

	bundle_encode(value, &raw, &len);
	bundle_add_str(noti->args, key, (const char *)raw);
	bundle_free_encoded_rawdata(&raw);

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_get_extention_data(notification_h noti, const char *key, bundle **value)
{
	return notification_get_extension_data(noti, key, value);
}

EXPORT_API int notification_get_extension_data(notification_h noti, const char *key, bundle **value)
{
	char *ret_str;
	bundle *args;
	int ret;

	if (noti == NULL || key == NULL || value == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	if (noti->args == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	args = noti->args;

	ret = bundle_get_str(args, key, &ret_str);
	if (ret != BUNDLE_ERROR_NONE)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	*value = _create_bundle_from_bundle_raw((bundle_raw *)ret_str);
	if (*value == NULL)
		return NOTIFICATION_ERROR_IO_ERROR;

	return NOTIFICATION_ERROR_NONE;
}
/* LCOV_EXCL_STOP */

#define KEY_LEN 40
#define EXTENSION_EVENT_KEY "_NOTIFICATION_EXTENSION_EVENT_"

/* LCOV_EXCL_START */
EXPORT_API int notification_set_extension_event_handler(notification_h noti,
					notification_event_type_extension_e event,
					app_control_h event_handler)
{
	int err;
	int ret = NOTIFICATION_ERROR_NONE;
	int len;
	bundle *app_control_bundle = NULL;
	bundle_raw *b_raw = NULL;
	char key[KEY_LEN];
	char *del = NULL;

	if (noti == NULL || event_handler == NULL) {
		ERR("Invalid parameter");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	if (event < NOTIFICATION_EVENT_TYPE_HIDDEN_BY_USER ||
	    event > NOTIFICATION_EVENT_TYPE_HIDDEN_BY_EXTERNAL) {
		ERR("Invalid event [%d]", event);
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	if (noti->args == NULL)
		noti->args = bundle_create();

	snprintf(key, sizeof(key), "%s%d", EXTENSION_EVENT_KEY, event);
	bundle_get_str(noti->args, key, &del);
	if (del != NULL) {
		bundle_del(noti->args, key);
		del = NULL;
	}

	err = app_control_export_as_bundle(event_handler, &app_control_bundle);
	if (err != APP_CONTROL_ERROR_NONE) {
		ERR("Failed to export app_control to bundle [%d]", err);
		ret = NOTIFICATION_ERROR_IO_ERROR;
		goto out;
	}

	err = bundle_encode(app_control_bundle, &b_raw, &len);
	if (err != BUNDLE_ERROR_NONE) {
		ERR("Failed to encode bundle [%d]", err);
		ret = NOTIFICATION_ERROR_IO_ERROR;
		goto out;
	}

	err = bundle_add_str(noti->args, key, (const char *)b_raw);
	if (err != BUNDLE_ERROR_NONE) {
		ERR("Failed to add str to bundle [%d]", err);
		ret = NOTIFICATION_ERROR_IO_ERROR;
		goto out;
	}

out:
	if (b_raw)
		bundle_free_encoded_rawdata(&b_raw);
	if (app_control_bundle)
		bundle_free(app_control_bundle);

	return ret;
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
EXPORT_API int notification_get_extension_event_handler(notification_h noti,
					notification_event_type_extension_e event,
					app_control_h *event_handler)
{
	int err;
	int ret = NOTIFICATION_ERROR_NONE;
	char *ret_str = NULL;
	char key[KEY_LEN];
	bundle *app_control_bundle = NULL;
	app_control_h ret_app_control = NULL;

	if (noti == NULL || event_handler == NULL) {
		ERR("Invalid parameter");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	if (event < NOTIFICATION_EVENT_TYPE_HIDDEN_BY_USER ||
	    event > NOTIFICATION_EVENT_TYPE_HIDDEN_BY_EXTERNAL) {
		ERR("Invalid event [%d]", event);
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	snprintf(key, sizeof(key), "%s%d", EXTENSION_EVENT_KEY, event);

	bundle_get_str(noti->args, key, &ret_str);
	if (ret_str == NULL) {
		ERR("No handler, Invalid event[%d]", event);
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	app_control_bundle = _create_bundle_from_bundle_raw((bundle_raw *)ret_str);
	if (app_control_bundle == NULL) {
		ERR("Failed to create bundle");
		return NOTIFICATION_ERROR_IO_ERROR;
	}

	err = app_control_create(&ret_app_control);
	if (err != APP_CONTROL_ERROR_NONE) {
		ERR("Failed to create app control [%d]", err);
		ret = NOTIFICATION_ERROR_IO_ERROR;
		goto out;
	}

	err = app_control_import_from_bundle(ret_app_control, app_control_bundle);
	if (err != APP_CONTROL_ERROR_NONE) {
		ERR("Failed to import app control from bundle [%d]", err);
		app_control_destroy(ret_app_control);
		ret = NOTIFICATION_ERROR_IO_ERROR;
		goto out;
	}

	*event_handler = ret_app_control;

out:
	if (app_control_bundle)
		bundle_free(app_control_bundle);

	return ret;
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
EXPORT_API int notification_get_all_count_for_uid(notification_type_e type, int *count, uid_t uid)
{
	int ret;

	if (count == NULL) {
		ERR("Invalid parameter - count is null");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	if (type < NOTIFICATION_TYPE_NONE || type > NOTIFICATION_TYPE_MAX) {
		ERR("Invalid parameter - wrong type");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	ret = notification_tidl_request_get_all_count(type, count, uid);
	if (ret != NOTIFICATION_ERROR_NONE) {
		ERR("Failed to get count [%d]", ret);
		return ret;
	}

	return ret;
}

EXPORT_API int notification_get_all_count(notification_type_e type, int *count)
{
	return notification_get_all_count_for_uid(type, count, aul_getuid());
}

EXPORT_API int notification_set_app_label(notification_h noti, char *label)
{
	if (noti == NULL || label == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	if (noti->app_label)
		free(noti->app_label);

	noti->app_label = strdup(label);

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_get_app_label(notification_h noti, char **label)
{
	if (noti == NULL || label == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	if (noti->app_label)
		*label = noti->app_label;

	return NOTIFICATION_ERROR_NONE;
}

static void __set_caller_info(bundle *b, const char *appid, uid_t uid)
{
	pkgmgrinfo_appinfo_h handle;
	char buf[12];
	char *pkgid = NULL;
	int r;

	snprintf(buf, sizeof(buf), "%u", uid);
	bundle_del(b, AUL_K_ORG_CALLER_UID);
	bundle_add(b, AUL_K_ORG_CALLER_UID, buf);

	bundle_del(b, AUL_K_ORG_CALLER_APPID);
	bundle_add(b, AUL_K_ORG_CALLER_APPID, appid);

	r = pkgmgrinfo_appinfo_get_usr_appinfo(appid, uid, &handle);
	if (r != PMINFO_R_OK)
		return;

	pkgmgrinfo_appinfo_get_pkgid(handle, &pkgid);
	if (pkgid) {
		bundle_del(b, AUL_K_ORG_CALLER_PKGID);
		bundle_add(b, AUL_K_ORG_CALLER_PKGID, pkgid);
	}
	pkgmgrinfo_appinfo_destroy_appinfo(handle);
}

static void __set_indirect_request(bundle *b)
{
	bundle_del(b, AUL_K_REQUEST_TYPE);
	bundle_add(b, AUL_K_REQUEST_TYPE, "indirect-request");
}

EXPORT_API int notification_set_indirect_request(notification_h noti,
		pid_t pid, uid_t uid)
{
	char appid[256] = { 0, };
	int r;
	int i;

	if (noti == NULL || pid <= 1 || uid < REGULAR_UID_MIN)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	r = aul_app_get_appid_bypid(pid, appid, sizeof(appid));
	if (r != AUL_R_OK)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	if (noti->b_service_responding) {
		__set_caller_info(noti->b_service_responding, appid, uid);
		__set_indirect_request(noti->b_service_responding);
	}

	if (noti->b_service_single_launch) {
		__set_caller_info(noti->b_service_single_launch, appid, uid);
		__set_indirect_request(noti->b_service_single_launch);
	}

	if (noti->b_service_multi_launch) {
		__set_caller_info(noti->b_service_multi_launch, appid, uid);
		__set_indirect_request(noti->b_service_multi_launch);
	}

	for (i = 0; i <= NOTIFICATION_EVENT_TYPE_MAX; i++) {
		if (noti->b_event_handler[i]) {
			__set_caller_info(noti->b_event_handler[i], appid, uid);
			__set_indirect_request(noti->b_event_handler[i]);
		}
	}

	return NOTIFICATION_ERROR_NONE;
}


int notification_delete_by_display_applist_for_uid(int display_applist, uid_t uid)
{
	int ret;

	if (display_applist < NOTIFICATION_DISPLAY_APP_NOTIFICATION_TRAY)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	ret = notification_tidl_request_delete_by_display_applist(display_applist, uid);

	return ret;
}

EXPORT_API int notification_delete_by_display_applist(int display_applist)
{
	return notification_delete_by_display_applist_for_uid(display_applist, aul_getuid());
}

EXPORT_API int notification_set_check_box(notification_h noti,
		bool flag, bool checked)
{
	if (noti == NULL) {
		ERR("Invalid parameter");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	noti->check_box = flag;
	noti->check_box_value = checked;

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_get_check_box(notification_h noti,
		bool *flag, bool *checked)
{
	if (noti == NULL || flag == NULL || checked == NULL) {
		ERR("Invalid parameter");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	*flag = noti->check_box;
	*checked = noti->check_box_value;

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_set_check_box_checked(notification_h noti,
		bool checked)
{
	if (noti == NULL) {
		ERR("Invalid parameter");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	noti->check_box_value = checked;

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_get_check_box_checked(notification_h noti,
		bool *checked)
{
	if (noti == NULL || checked == NULL) {
		ERR("Invalid parameter");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	if (noti->check_box)
		*checked = noti->check_box_value;
	else
		*checked = false;

	return NOTIFICATION_ERROR_NONE;
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
EXPORT_API int notification_register_do_not_disturb_app(disturb_cb callback, void *user_data)
{
	int ret;

	WARN("register dnd app");

	if (callback == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	ret = notification_tidl_request_register_dnd_app(aul_getuid(), getpid());
	if (ret != NOTIFICATION_ERROR_NONE) {
		ERR("failed to register dnd app");
		return ret;
	}

	ret = notification_tidl_event_monitor_init();
	if (ret != NOTIFICATION_ERROR_NONE) {
		ERR("failed to event monitor init");
		return ret;
	}

	_disturb_callback = callback;
	_disturb_user_data = user_data;

	WARN("register dnd app done");

	return ret;
}

EXPORT_API int notification_unregister_do_not_disturb_app(void)
{
	int ret;

	WARN("unregister dnd app");

	ret = notification_tidl_request_unregister_dnd_app(aul_getuid(), getpid());
	if (ret != NOTIFICATION_ERROR_NONE) {
		ERR("failed to unregister dnd app");
		return ret;
	}

	_disturb_callback = NULL;

	if (__noti_event_cb_list == NULL)
		notification_tidl_event_monitor_fini();

	WARN("unregister dnd app done");

	return ret;
}

void notification_call_disturb_cb(void)
{
	WARN("call disturb callback");

	if (_disturb_callback == NULL) {
		ERR("no disturb callback");
		return;
	}

	_disturb_callback(_disturb_user_data);

	WARN("done");
}
/* LCOV_EXCL_STOP */

#define PAIRING_TYPE_KEY "_NOTIFICATION_TYPE_PAIRING_"

/* LCOV_EXCL_START */
EXPORT_API int notification_set_pairing_type(notification_h noti, bool pairing)
{
	char *del = NULL;

	WARN("set pairing");

	if (noti == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	if (noti->args == NULL)
		noti->args = bundle_create();

	bundle_get_str(noti->args, PAIRING_TYPE_KEY, &del);
	if (del != NULL) {
		bundle_del(noti->args, PAIRING_TYPE_KEY);
		del = NULL;
	}

	if (pairing)
		bundle_add_str(noti->args, PAIRING_TYPE_KEY, "true");
	else
		bundle_add_str(noti->args, PAIRING_TYPE_KEY, "false");

	WARN("set pairing done");

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_get_pairing_type(notification_h noti, bool *pairing)
{
	int ret;
	char *value = NULL;

	WARN("get pairing");

	if (noti == NULL || noti->args == NULL || pairing == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	ret = bundle_get_str(noti->args, PAIRING_TYPE_KEY, &value);
	if (ret != BUNDLE_ERROR_NONE || value == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	if (strcmp("true", value) == 0)
		*pairing = true;
	else
		*pairing = false;

	WARN("get pairing done");

	return NOTIFICATION_ERROR_NONE;
}
/* LCOV_EXCL_STOP */

/* for channel */
EXPORT_API int notification_set_channel_name(notification_h noti,
		const char *channel_name)
{
	if (noti == NULL || channel_name == NULL) {
		ERR("Invalid parameter");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	if (noti->channel_name)
		free(noti->channel_name);

	noti->channel_name = strdup(channel_name);

	WARN("notification_set_channel_name done");

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_get_channel_name(notification_h noti,
		const char **channel_name)
{
	if (noti == NULL || channel_name == NULL) {
		ERR("Invalid parameter");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	*channel_name = noti->channel_name;

	WARN("notification_set_channel_name done");

	return NOTIFICATION_ERROR_NONE;
}

int notification_channel_create_with_info(const char *app_id,
		const char *channel_name, int blockable, int is_blocked,
		notification_channel_h *channel)
{
	notification_channel_s *noti_channel = NULL;

	if (app_id == NULL || channel_name == NULL) {
		ERR("Invalid parameter");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	noti_channel = (notification_channel_s *)calloc(1,
			sizeof(notification_channel_s));
	if (noti_channel == NULL) {
		ERR("Out of memory");
		return NOTIFICATION_ERROR_OUT_OF_MEMORY;
	}
	noti_channel->app_id = strdup(app_id);
	noti_channel->channel_name = strdup(channel_name);
	noti_channel->blockable = (blockable == 1) ? true : false;
	noti_channel->is_blocked = (is_blocked == 1) ? true : false;

	*channel = noti_channel;

	WARN("notification_channel_create_with_info done");

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_channel_create(const char *channel_name,
		notification_channel_h *channel)
{
	notification_channel_s *noti_channel = NULL;

	if (channel == NULL || channel_name == NULL) {
		ERR("Invalid parameter");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	noti_channel = (notification_channel_s *)calloc(1,
			sizeof(notification_channel_s));
	if (noti_channel == NULL) {
		ERR("Out of memory");
		return NOTIFICATION_ERROR_OUT_OF_MEMORY;
	}
	noti_channel->app_id = notification_get_app_id_by_pid(getpid());
	noti_channel->channel_name = strdup(channel_name);
	noti_channel->blockable = false;
	noti_channel->is_blocked = false;

	*channel = noti_channel;

	WARN("notification_channel_create done [%s]", noti_channel->app_id);

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API void notification_channel_free(notification_channel_h channel)
{
	notification_channel_s *noti_channel = channel;

	if (noti_channel == NULL) {
		ERR("Invalid parameter");
		return;
	}

	if (noti_channel->app_id)
		free(noti_channel->app_id);

	if (noti_channel->channel_name)
		free(noti_channel->channel_name);

	free(noti_channel);

	WARN("notification_channel_free done");
}

EXPORT_API int notification_channel_add(notification_channel_h channel)
{
	int ret;
	notification_channel_s *noti_channel = channel;

	WARN("notification_channel_add");

	if (noti_channel == NULL || noti_channel->app_id == NULL
		|| noti_channel->channel_name == NULL) {
		ERR("Invalid parameter");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	ret = notification_tidl_request_insert_channel(noti_channel->app_id,
			noti_channel->channel_name, noti_channel->blockable,
			noti_channel->is_blocked, aul_getuid());
	if (ret != NOTIFICATION_ERROR_NONE) {
		ERR("failed to insert channel");
		return ret;
	}

	WARN("notification_channel_add done");

	return ret;
}

EXPORT_API int notification_channel_remove(notification_channel_h channel)
{
	int ret;
	notification_channel_s *noti_channel = channel;

	WARN("notification_channel_remove");

	if (noti_channel == NULL || noti_channel->app_id == NULL
		|| noti_channel->channel_name == NULL) {
		ERR("Invalid parameter");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	ret = notification_tidl_request_delete_channel(noti_channel->app_id,
			noti_channel->channel_name, aul_getuid());
	if (ret != NOTIFICATION_ERROR_NONE) {
		ERR("failed to remove channel");
		return ret;
	}

	WARN("notification_channel_remove done");

	return ret;
}

EXPORT_API int notification_channel_update(notification_channel_h channel)
{
	int ret;
	notification_channel_s *noti_channel = channel;

	WARN("notification_channel_update");

	if (noti_channel == NULL || noti_channel->app_id == NULL
		|| noti_channel->channel_name == NULL) {
		ERR("Invalid parameter");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	ret = notification_tidl_request_update_channel(noti_channel->app_id,
			noti_channel->channel_name, noti_channel->blockable,
			noti_channel->is_blocked, aul_getuid());
	if (ret != NOTIFICATION_ERROR_NONE) {
		ERR("failed to update channel");
		return ret;
	}

	WARN("notification_channel_update done");

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_channel_get_by_name(const char *channel_name,
		notification_channel_h *channel)
{
	int ret;
	char *app_id;
	bool blockable;
	bool is_blocked;
	notification_channel_s *noti_channel = NULL;

	WARN("notification_channel_get_by_name");

	app_id = notification_get_app_id_by_pid(getpid());
	if (channel_name == NULL || channel == NULL || app_id == NULL) {
		ERR("Invalid parameter");
		ret = NOTIFICATION_ERROR_INVALID_PARAMETER;
		goto out;
	}

	noti_channel = (notification_channel_s *)calloc(1,
			sizeof(notification_channel_s));
	if (noti_channel == NULL) {
		ERR("Out of memory");
		ret = NOTIFICATION_ERROR_OUT_OF_MEMORY;
		goto out;
	}

	ret = notification_tidl_request_get_channel(app_id, channel_name,
			&blockable, &is_blocked, aul_getuid());
	if (ret != NOTIFICATION_ERROR_NONE) {
		ERR("failed to update channel");
		free(noti_channel);
		goto out;
	}
	noti_channel->app_id = strdup(app_id);
	noti_channel->channel_name = strdup(channel_name);
	noti_channel->blockable = blockable;
	noti_channel->is_blocked = is_blocked;

	*channel = noti_channel;

out:
	if (app_id)
		free(app_id);

	WARN("notification_channel_get_by_name done : %d", ret);

	return ret;
}

EXPORT_API int notification_channel_set_blockable(
		notification_channel_h channel,bool blockable)
{
	notification_channel_s *noti_channel = channel;

	if (noti_channel == NULL) {
		ERR("Invalid parameter");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	noti_channel->blockable = blockable;

	WARN("notification_channel_set_blockable done");

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_channel_get_blockable(
		notification_channel_h channel, bool *blockable)
{
	notification_channel_s *noti_channel = channel;

	if (noti_channel == NULL) {
		ERR("Invalid parameter");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	*blockable = noti_channel->blockable;

	WARN("notification_channel_get_blockable done");

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_channel_set_block(
		notification_channel_h channel, bool block)
{
	notification_channel_s *noti_channel = channel;

	if (noti_channel == NULL) {
		ERR("Invalid parameter");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	noti_channel->is_blocked = block;

	WARN("notification_channel_set_block done");

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_channel_get_block(
		notification_channel_h channel, bool *block)
{
	notification_channel_s *noti_channel = channel;

	if (noti_channel == NULL) {
		ERR("Invalid parameter");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	*block = noti_channel->is_blocked;

	WARN("notification_channel_get_block done");

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_channel_get_name(notification_channel_h channel,
		const char **channel_name)
{
	notification_channel_s *noti_channel = channel;

	if (noti_channel == NULL || channel_name == NULL) {
		ERR("Invalid parameter");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	*channel_name = noti_channel->channel_name;

	WARN("notification_channel_get_name done");

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_channel_clone(notification_channel_h channel,
		notification_channel_h *clone)
{
	notification_channel_s *new_channel = NULL;
	notification_channel_s *noti_channel = channel;

	if (noti_channel == NULL || noti_channel->app_id == NULL
		|| noti_channel->channel_name == NULL || clone == NULL ) {
		ERR("Invalid parameter");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	new_channel = (notification_channel_s *)calloc(1,
			sizeof(notification_channel_s));
	if (new_channel == NULL) {
		ERR("Out of memory");
		return NOTIFICATION_ERROR_OUT_OF_MEMORY;
	}
	new_channel->app_id = strdup(noti_channel->app_id);
	new_channel->channel_name = strdup(noti_channel->channel_name);
	new_channel->blockable = noti_channel->blockable;
	new_channel->is_blocked = noti_channel->is_blocked;

	*clone = new_channel;

	WARN("notification_channel_clone done");

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_channel_foreach(const char *app_id,
		notification_channel_foreach_cb cb, void *user_data)
{
	int ret;
	char *_app_id = NULL;

	if (cb == NULL) {
		ERR("Invalid parameter");
		ret = NOTIFICATION_ERROR_INVALID_PARAMETER;
		goto out;
	}
	if (app_id == NULL) {
		_app_id = notification_get_app_id_by_pid(getpid());
		if (_app_id == NULL) {
			ERR("failed to get app id");
			ret = NOTIFICATION_ERROR_IO_ERROR;
			goto out;
		}
	}
	WARN("notification_channel_foreach : %s", (app_id != NULL) ? app_id : _app_id);

	ret = notification_tidl_request_channel_get_list(
			(app_id != NULL) ? app_id : _app_id,
			cb, user_data, aul_getuid());
	if (ret != NOTIFICATION_ERROR_NONE) {
		ERR("failed to get is blocked");
		goto out;
	}

out:
	if (_app_id)
		free(_app_id);

	WARN("notification_channel_foreach done : %d", ret);

	return ret;
}
