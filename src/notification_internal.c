/*
 * Copyright (c) 2000 - 2016 Samsung Electronics Co., Ltd. All rights reserved.
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
#include <dbus/dbus.h>
#include <dbus/dbus-glib-lowlevel.h>
#include <gio/gio.h>

#include <app_control.h>
#include <app_control_internal.h>
#include <aul.h>
#include <tizen.h>
#include <vconf-keys.h>
#include <vconf.h>
#include <bundle.h>
#include <bundle_internal.h>

#include <notification.h>
#include <notification_list.h>
#include <notification_debug.h>
#include <notification_private.h>
#include <notification_noti.h>
#include <notification_ongoing.h>
#include <notification_group.h>
#include <notification_ipc.h>
#include <notification_internal.h>

typedef struct _notification_cb_info notification_cb_info_s;
typedef struct _notification_event_cb_info notification_event_cb_info_s;

typedef enum __notification_cb_type {
	NOTIFICATION_CB_NORMAL = 1,
	NOTIFICATION_CB_DETAILED,
} _notification_cb_type_e;

struct _notification_cb_info {
	_notification_cb_type_e cb_type;
	void (*changed_cb) (void *data, notification_type_e type);
	void (*detailed_changed_cb) (void *data, notification_type_e type, notification_op *op_list, int num_op);
	void *data;
};

struct _notification_event_cb_info {
	int priv_id;
	event_handler_cb cb;
	void *userdata;
};

static GHashTable *_noti_cb_hash = NULL;
static GList *__noti_event_cb_list = NULL;

void notification_reset_event_handler_list(void)
{
	GList *iter;
	notification_event_cb_info_s *info = NULL;

	if (!__noti_event_cb_list)
		return;

	iter = g_list_first(__noti_event_cb_list);
	while (iter) {
		info = g_list_nth_data(iter, 0);
		if (info)
			notification_ipc_reset_event_handler(info->priv_id);
		iter = g_list_next(iter);
	}
}

/* LCOV_EXCL_START */
static void __free_changed_cb_info(gpointer data)
{
	notification_cb_info_s *noti_cb_info = (notification_cb_info_s *)data;
	if (noti_cb_info)
		free(noti_cb_info);
}
/* LCOV_EXCL_STOP */

void notification_call_changed_cb_for_uid(notification_op *op_list, int op_num, uid_t uid)
{
	notification_type_e type = 0;
	GList *noti_cb_list = NULL;
	notification_cb_info_s *noti_cb_info = NULL;

	if (_noti_cb_hash == NULL)
		return;

	noti_cb_list = (GList *)g_hash_table_lookup(_noti_cb_hash, GUINT_TO_POINTER(uid));

	if (noti_cb_list == NULL)
		return;

	if (op_list == NULL) {
		NOTIFICATION_ERR("invalid data");
		return;
	}

	noti_cb_list = g_list_first(noti_cb_list);
	notification_get_type(op_list->noti, &type);

	for (; noti_cb_list != NULL; noti_cb_list = noti_cb_list->next) {
		noti_cb_info = noti_cb_list->data;

		if (noti_cb_info->cb_type == NOTIFICATION_CB_NORMAL && noti_cb_info->changed_cb) {
			noti_cb_info->changed_cb(noti_cb_info->data, type);
		}
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

	if (__noti_event_cb_list == NULL)
		return;

	ret = notification_get_id(noti, NULL, &priv_id);
	if (ret != NOTIFICATION_ERROR_NONE)
		return;

	__noti_event_cb_list = g_list_first(__noti_event_cb_list);
	find_list = g_list_find_custom(__noti_event_cb_list, GINT_TO_POINTER(priv_id),
				       (GCompareFunc)__priv_id_compare);
	if (find_list == NULL)
		return;

	info = g_list_nth_data(find_list, 0);
	info->cb(noti, event_type, info->userdata);
}

void notification_delete_event_handler_cb(int priv_id)
{
	GList *delete_list;
	notification_event_cb_info_s *info = NULL;

	if (__noti_event_cb_list == NULL)
		return;

	__noti_event_cb_list = g_list_first(__noti_event_cb_list);
	delete_list = g_list_find_custom(__noti_event_cb_list, GINT_TO_POINTER(priv_id),
					 (GCompareFunc)__priv_id_compare);

	if (delete_list == NULL)
		return;

	info = g_list_nth_data(delete_list, 0);
	__noti_event_cb_list = g_list_remove(g_list_first(__noti_event_cb_list), info);

	if (info)
		free(info);

	if (__noti_event_cb_list == NULL)
		notification_ipc_event_monitor_fini();
}

EXPORT_API int notification_add_deferred_task(
		void (*deferred_task_cb)(void *data), void *user_data)
{
	if (deferred_task_cb == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	return notification_ipc_add_deffered_task(deferred_task_cb, user_data);
}

EXPORT_API int notification_del_deferred_task(
		void (*deferred_task_cb)(void *data))
{
	if (deferred_task_cb == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	return notification_ipc_del_deffered_task(deferred_task_cb);
}

EXPORT_API int notification_resister_changed_cb_for_uid(
		void (*changed_cb)(void *data, notification_type_e type),
		void *user_data, uid_t uid)
{
	GList *noti_cb_list = NULL;
	notification_cb_info_s *noti_cb_info_new = NULL;

	if (changed_cb == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	if (notification_ipc_monitor_init(uid) != NOTIFICATION_ERROR_NONE)
		return NOTIFICATION_ERROR_IO_ERROR;

	if (_noti_cb_hash == NULL)
		_noti_cb_hash = g_hash_table_new(g_direct_hash, g_direct_equal);

	noti_cb_info_new = (notification_cb_info_s *)malloc(sizeof(notification_cb_info_s));
	if (noti_cb_info_new == NULL) {
		NOTIFICATION_ERR("malloc failed");
		return NOTIFICATION_ERROR_OUT_OF_MEMORY;
	}

	noti_cb_info_new->cb_type = NOTIFICATION_CB_NORMAL;
	noti_cb_info_new->changed_cb = changed_cb;
	noti_cb_info_new->detailed_changed_cb = NULL;
	noti_cb_info_new->data = user_data;

	noti_cb_list = g_hash_table_lookup(_noti_cb_hash, GUINT_TO_POINTER(uid));

	if (noti_cb_list == NULL) {
		noti_cb_list = g_list_append(noti_cb_list, noti_cb_info_new);
		g_hash_table_insert(_noti_cb_hash, GUINT_TO_POINTER(uid), noti_cb_list);
	} else {
		noti_cb_list = g_list_append(noti_cb_list, noti_cb_info_new);
	}

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_resister_changed_cb(
		void (*changed_cb)(void *data, notification_type_e type),
		void *user_data)
{
	return notification_resister_changed_cb_for_uid(changed_cb, user_data, getuid());
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
		void (*changed_cb)(void *data, notification_type_e type), uid_t uid)
{
	notification_cb_info_s *noti_cb_info = NULL;
	GList *noti_cb_list = NULL;
	GList *delete_cb_list = NULL;

	if (changed_cb == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	if (_noti_cb_hash == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	noti_cb_list = (GList *)g_hash_table_lookup(_noti_cb_hash, GUINT_TO_POINTER(uid));

	if (noti_cb_list == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	noti_cb_list = g_list_first(noti_cb_list);
	delete_cb_list = g_list_find_custom(noti_cb_list, (gconstpointer)changed_cb,
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
		notification_ipc_monitor_fini();

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_unresister_changed_cb(
		void (*changed_cb)(void *data, notification_type_e type))
{
	return notification_unresister_changed_cb_for_uid(changed_cb, getuid());
}

EXPORT_API int notification_update_progress(notification_h noti,
		int priv_id,
		double progress)
{
	char *caller_pkgname = NULL;
	int input_priv_id = 0;
	int ret = 0;
	double input_progress = 0.0;

	if (priv_id <= NOTIFICATION_PRIV_ID_NONE) {
		if (noti == NULL)
			return NOTIFICATION_ERROR_INVALID_PARAMETER;
		else
			input_priv_id = noti->priv_id;

	} else {
		input_priv_id = priv_id;
	}

	if (noti == NULL)
		caller_pkgname = notification_get_pkgname_by_pid();
	else
		caller_pkgname = strdup(noti->caller_pkgname);

	if (progress < 0.0)
		input_progress = 0.0;
	else if (progress > 1.0)
		input_progress = 1.0;
	else
		input_progress = progress;

	ret = notification_ongoing_update_progress(caller_pkgname, input_priv_id,
			input_progress);

	if (caller_pkgname)
		free(caller_pkgname);

	return ret;
}

EXPORT_API int notification_update_size(notification_h noti,
		int priv_id,
		double size)
{
	char *caller_pkgname = NULL;
	int input_priv_id = 0;
	int ret = 0;
	double input_size = 0.0;

	if (priv_id <= NOTIFICATION_PRIV_ID_NONE) {
		if (noti == NULL)
			return NOTIFICATION_ERROR_INVALID_PARAMETER;
		else
			input_priv_id = noti->priv_id;
	} else {
		input_priv_id = priv_id;
	}

	if (noti == NULL)
		caller_pkgname = notification_get_pkgname_by_pid();
	else
		caller_pkgname = strdup(noti->caller_pkgname);

	if (size < 0.0)
		input_size = 0.0;
	else
		input_size = size;

	ret = notification_ongoing_update_size(caller_pkgname, input_priv_id,
			input_size);

	if (caller_pkgname)
		free(caller_pkgname);

	return ret;
}

EXPORT_API int notification_update_content(notification_h noti,
		int priv_id,
		const char *content)
{
	char *caller_pkgname = NULL;
	int input_priv_id = 0;
	int ret = 0;

	if (priv_id <= NOTIFICATION_PRIV_ID_NONE) {
		if (noti == NULL)
			return NOTIFICATION_ERROR_INVALID_PARAMETER;
		else
			input_priv_id = noti->priv_id;
	} else {
		input_priv_id = priv_id;
	}

	if (noti == NULL)
		caller_pkgname = notification_get_pkgname_by_pid();
	else
		caller_pkgname = strdup(noti->caller_pkgname);

	ret = notification_ongoing_update_content(caller_pkgname, input_priv_id,
			content);

	if (caller_pkgname)
		free(caller_pkgname);

	return ret;
}

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

/* LCOV_EXCL_START */
EXPORT_API int notification_set_title(notification_h noti,
		const char *title,
		const char *loc_title)
{
	return notification_set_text(noti, NOTIFICATION_TEXT_TYPE_TITLE,
				title, loc_title,
				NOTIFICATION_VARIABLE_TYPE_NONE);;
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
		const char *pkgname)
{
	if (noti == NULL || pkgname == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	if (noti->launch_pkgname)
		free(noti->launch_pkgname);

	noti->launch_pkgname = strdup(pkgname);

	return NOTIFICATION_ERROR_NONE;
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
EXPORT_API int notification_get_application(notification_h noti,
		char **pkgname)
{
	if (noti == NULL || pkgname == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	if (noti->launch_pkgname)
		*pkgname = noti->launch_pkgname;
	else
		*pkgname = noti->caller_pkgname;

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

	ret = notification_noti_get_grouping_list(type, count, &get_list, uid);
	if (ret != NOTIFICATION_ERROR_NONE)
		return ret;

	*list = get_list;

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_get_grouping_list(notification_type_e type, int count,
		notification_list_h *list)
{
	return notification_get_grouping_list_for_uid(type, count, list, getuid());
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
EXPORT_API int notification_delete_group_by_group_id(const char *pkgname,
		notification_type_e type,
		int group_id)
{
	int ret = 0;
	char *caller_pkgname = NULL;

	if (pkgname == NULL)
		caller_pkgname = notification_get_pkgname_by_pid();
	else
		caller_pkgname = strdup(pkgname);

	ret = notification_ipc_request_delete_multiple(type, caller_pkgname, getuid());

	if (caller_pkgname)
		free(caller_pkgname);

	return ret;
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
int notification_delete_group_by_priv_id_for_uid(const char *pkgname,
		notification_type_e type,
		int priv_id, uid_t uid)
{
	int ret = 0;
	char *caller_pkgname = NULL;

	if (pkgname == NULL)
		caller_pkgname = notification_get_pkgname_by_pid();
	else
		caller_pkgname = strdup(pkgname);

	ret = notification_ipc_request_delete_single(type, caller_pkgname, priv_id, uid);

	if (caller_pkgname)
		free(caller_pkgname);

	return ret;
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
EXPORT_API int notification_delete_group_by_priv_id(const char *pkgname,
		notification_type_e type,
		int priv_id)
{
	return notification_delete_group_by_priv_id_for_uid(pkgname, type, priv_id, getuid());
}

int notification_get_count_for_uid(notification_type_e type,
		const char *pkgname,
		int group_id,
		int priv_id, int *count,
		uid_t uid)
{
	int ret = 0;
	char *caller_pkgname = NULL;

	if (count == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	if (pkgname == NULL)
		caller_pkgname = notification_get_pkgname_by_pid();
	else
		caller_pkgname = strdup(pkgname);

	ret = notification_ipc_request_get_count(
			type,
			caller_pkgname,
			group_id,
			priv_id,
			count,
			uid);

	if (caller_pkgname)
		free(caller_pkgname);

	return ret;
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
EXPORT_API int notification_get_count(notification_type_e type,
		const char *pkgname,
		int group_id,
		int priv_id, int *count)
{
	return notification_get_count_for_uid(type, pkgname, group_id, priv_id, count, getuid());
}

int notification_clear_for_uid(notification_type_e type, uid_t uid)
{
	if (type <= NOTIFICATION_TYPE_NONE || type > NOTIFICATION_TYPE_MAX)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	return notification_ipc_request_delete_multiple(type, NULL, uid);
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
EXPORT_API int notification_clear(notification_type_e type)
{
	return notification_clear_for_uid(type, getuid());
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
		break;
	}

	return NOTIFICATION_ERROR_NONE;
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
EXPORT_API int notification_set_pkgname(notification_h noti,
		const char *pkgname)
{
	if (noti == NULL || pkgname == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	/* Remove previous caller pkgname */
	if (noti->caller_pkgname) {
		free(noti->caller_pkgname);
		noti->caller_pkgname = NULL;
	}

	noti->caller_pkgname = strdup(pkgname);

	return NOTIFICATION_ERROR_NONE;
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
int notification_delete_all_by_type_for_uid(const char *pkgname,
		notification_type_e type, uid_t uid)
{
	int ret = 0;
	char *caller_pkgname = NULL;

	if (type <= NOTIFICATION_TYPE_NONE || type > NOTIFICATION_TYPE_MAX)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	if (pkgname == NULL)
		caller_pkgname = notification_get_pkgname_by_pid();
	else
		caller_pkgname = strdup(pkgname);

	ret = notification_ipc_request_delete_multiple(type, caller_pkgname, uid);

	if (caller_pkgname)
		free(caller_pkgname);

	return ret;
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
EXPORT_API int notification_delete_all_by_type(const char *pkgname,
		notification_type_e type)
{
	return notification_delete_all_by_type_for_uid(pkgname, type, getuid());
}

int notification_delete_by_priv_id_for_uid(const char *pkgname,
		notification_type_e type,
		int priv_id,
		uid_t uid)
{
	int ret = 0;
	char *caller_pkgname = NULL;

	if (priv_id <= NOTIFICATION_PRIV_ID_NONE)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	if (pkgname == NULL)
		caller_pkgname = notification_get_pkgname_by_pid();
	else
		caller_pkgname = strdup(pkgname);

	ret = notification_ipc_request_delete_single(type, caller_pkgname, priv_id, uid);

	if (caller_pkgname)
		free(caller_pkgname);

	return ret;
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
EXPORT_API int notification_delete_by_priv_id(const char *pkgname,
		notification_type_e type,
		int priv_id)
{
	return notification_delete_by_priv_id_for_uid(pkgname, type, priv_id, getuid());
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

EXPORT_API int notification_set_priv_id(notification_h noti, int priv_id)
{
	if (noti == NULL || priv_id <= 0)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	noti->priv_id = priv_id;

	return NOTIFICATION_ERROR_NONE;
}

/* LCOV_EXCL_START */
notification_h notification_load_for_uid(char *pkgname,
		int priv_id, uid_t uid)
{
	int ret = 0;
	notification_h noti = NULL;

	noti = (notification_h)calloc(1, sizeof(struct _notification));
	if (noti == NULL) {
		NOTIFICATION_ERR("NO MEMORY : noti == NULL");
		return NULL;
	}

	ret = notification_ipc_request_load_noti_by_priv_id(noti, pkgname, priv_id, uid);
	if (ret != NOTIFICATION_ERROR_NONE) {
		notification_free(noti);
		return NULL;
	}

	return noti;
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
EXPORT_API notification_h notification_load(char *pkgname,
		int priv_id)
{
	return notification_load_for_uid(pkgname, priv_id, getuid());
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

EXPORT_API int notification_insert_for_uid(notification_h noti,
		int *priv_id, uid_t uid)
{
	int ret = 0;
	int id = 0;

	if (noti == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	if (noti->type <= NOTIFICATION_TYPE_NONE
			|| noti->type > NOTIFICATION_TYPE_MAX)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	noti->uid = uid;
	noti->insert_time = time(NULL);

	ret = notification_ipc_request_insert(noti, &id);
	if (ret != NOTIFICATION_ERROR_NONE)
		return ret;

	noti->priv_id = id;
	NOTIFICATION_DBG("from master:%d", id);

	/* If priv_id is valid data, set priv_id */
	if (priv_id != NULL)
		*priv_id = noti->priv_id;

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_insert(notification_h noti,
		int *priv_id)
{
	return notification_insert_for_uid(noti, priv_id, getuid());
}

EXPORT_API int notification_update_async_for_uid(notification_h noti,
		void (*result_cb)(int priv_id, int result, void *data), void *user_data, uid_t uid)
{
	if (noti == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	noti->uid = uid;
	/* Update insert time ? */
	noti->insert_time = time(NULL);

	return notification_ipc_request_update_async(noti, result_cb, user_data);
}

EXPORT_API int notification_update_async(notification_h noti,
		void (*result_cb)(int priv_id, int result, void *data), void *user_data)
{
	return notification_update_async_for_uid(noti, result_cb, user_data, getuid());
}

EXPORT_API int notification_register_detailed_changed_cb_for_uid(
		void (*detailed_changed_cb)(void *data, notification_type_e type, notification_op *op_list, int num_op),
		void *user_data, uid_t uid)
{
	GList *noti_cb_list = NULL;
	notification_cb_info_s *noti_cb_info_new = NULL;

	if (detailed_changed_cb == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	if (notification_ipc_monitor_init(uid) != NOTIFICATION_ERROR_NONE)
		return NOTIFICATION_ERROR_IO_ERROR;

	if (_noti_cb_hash == NULL)
		_noti_cb_hash = g_hash_table_new(g_direct_hash, g_direct_equal);

	noti_cb_info_new = (notification_cb_info_s *)malloc(sizeof(notification_cb_info_s));
	if (noti_cb_info_new == NULL) {
		NOTIFICATION_ERR("malloc failed");
		return NOTIFICATION_ERROR_OUT_OF_MEMORY;
	}

	noti_cb_info_new->cb_type = NOTIFICATION_CB_DETAILED;
	noti_cb_info_new->changed_cb = NULL;
	noti_cb_info_new->detailed_changed_cb = detailed_changed_cb;
	noti_cb_info_new->data = user_data;

	noti_cb_list = g_hash_table_lookup(_noti_cb_hash, GUINT_TO_POINTER(uid));

	if (noti_cb_list == NULL) {
		noti_cb_list = g_list_append(noti_cb_list, noti_cb_info_new);
		g_hash_table_insert(_noti_cb_hash, GUINT_TO_POINTER(uid), noti_cb_list);
	} else {
		noti_cb_list = g_list_append(noti_cb_list, noti_cb_info_new);
	}

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_register_detailed_changed_cb(
		void (*detailed_changed_cb)(void *data, notification_type_e type, notification_op *op_list, int num_op),
		void *user_data)
{
	return notification_register_detailed_changed_cb_for_uid(detailed_changed_cb, user_data, getuid());
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
		void (*detailed_changed_cb)(void *data, notification_type_e type, notification_op *op_list, int num_op),
		void *user_data, uid_t uid)
{
	notification_cb_info_s *noti_cb_info = NULL;
	GList *noti_cb_list = NULL;
	GList *delete_cb_list = NULL;

	if (detailed_changed_cb == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	if (_noti_cb_hash == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	noti_cb_list = (GList *)g_hash_table_lookup(_noti_cb_hash, GUINT_TO_POINTER(uid));

	if (noti_cb_list == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	noti_cb_list = g_list_first(noti_cb_list);
	delete_cb_list = g_list_find_custom(noti_cb_list, (gconstpointer)detailed_changed_cb,
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
		notification_ipc_monitor_fini();

	return NOTIFICATION_ERROR_NONE;

}

EXPORT_API int notification_unregister_detailed_changed_cb(
		void (*detailed_changed_cb)(void *data, notification_type_e type, notification_op *op_list, int num_op),
		void *user_data)
{
	return notification_unregister_detailed_changed_cb_for_uid(detailed_changed_cb, user_data, getuid());
}

/* LCOV_EXCL_START */
EXPORT_API int notification_is_service_ready(void)
{
	return notification_ipc_is_master_ready();
}
/* LCOV_EXCL_STOP */

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

EXPORT_API int notification_post_for_uid(notification_h noti, uid_t uid)
{
	int ret = 0;
	int id = 0;

	if (noti == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	if (noti->type <= NOTIFICATION_TYPE_NONE
			|| noti->type > NOTIFICATION_TYPE_MAX)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	/* Save insert time */
	noti->insert_time = time(NULL);
	noti->uid = uid;

	ret = notification_ipc_request_insert(noti, &id);
	if (ret != NOTIFICATION_ERROR_NONE)
		return ret;

	noti->priv_id = id;
	NOTIFICATION_DBG("from master:%d", id);

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_update_for_uid(notification_h noti, uid_t uid)
{
	if (noti == NULL) {
		notification_ipc_request_refresh(uid);
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	noti->uid = uid;
	/* Update insert time ? */
	noti->insert_time = time(NULL);

	return notification_ipc_request_update(noti);
}

EXPORT_API int notification_delete_for_uid(notification_h noti, uid_t uid)
{

	if (noti == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	return notification_ipc_request_delete_single(NOTIFICATION_TYPE_NONE,
				noti->caller_pkgname, noti->priv_id, uid);
}

EXPORT_API int notification_delete_all_for_uid(notification_type_e type, uid_t uid)
{
	int ret = 0;
	char *caller_pkgname = NULL;

	if (type <= NOTIFICATION_TYPE_NONE || type > NOTIFICATION_TYPE_MAX)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	caller_pkgname = notification_get_pkgname_by_pid();

	ret = notification_ipc_request_delete_multiple(type, caller_pkgname, uid);

	if (caller_pkgname)
		free(caller_pkgname);

	return ret;
}

EXPORT_API notification_h notification_load_by_tag_for_uid(const char *tag, uid_t uid)
{
	int ret = 0;
	notification_h noti = NULL;
	char *caller_pkgname = NULL;

	if (tag == NULL) {
		NOTIFICATION_ERR("Invalid parameter");
		set_last_result(NOTIFICATION_ERROR_INVALID_PARAMETER);
		return NULL;
	}

	caller_pkgname = notification_get_pkgname_by_pid();
	if (!caller_pkgname) {
		/* LCOV_EXCL_START */
		NOTIFICATION_ERR("Failed to get a package name");
		set_last_result(NOTIFICATION_ERROR_OUT_OF_MEMORY);
		return NULL;
		/* LCOV_EXCL_STOP */
	}

	noti = (notification_h)calloc(1, sizeof(struct _notification));
	if (noti == NULL) {
		/* LCOV_EXCL_START */
		NOTIFICATION_ERR("Failed to alloc a new notification");
		set_last_result(NOTIFICATION_ERROR_OUT_OF_MEMORY);
		free(caller_pkgname);

		return NULL;
		/* LCOV_EXCL_STOP */
	}

	ret = notification_ipc_request_load_noti_by_tag(noti, caller_pkgname, (char *)tag, uid);
	free(caller_pkgname);

	set_last_result(ret);
	if (ret != NOTIFICATION_ERROR_NONE) {
		notification_free(noti);
		return NULL;
	}

	return noti;
}

EXPORT_API notification_h notification_create_from_package_template(const char *pkgname, const char *template_name)
{
	int ret = 0;
	notification_h noti = NULL;

	if (pkgname == NULL || template_name == NULL) {
		NOTIFICATION_ERR("Invalid parameter");
		set_last_result(NOTIFICATION_ERROR_INVALID_PARAMETER);
		return NULL;
	}

	noti = (notification_h)calloc(1, sizeof(struct _notification));
	if (noti == NULL) {
		NOTIFICATION_ERR("Failed to alloc a new notification");
		set_last_result(NOTIFICATION_ERROR_OUT_OF_MEMORY);
		return NULL;
	}

	ret = notification_ipc_request_create_from_package_template(noti, pkgname, template_name);

	set_last_result(ret);
	if (ret != NOTIFICATION_ERROR_NONE) {
		notification_free(noti);
		return NULL;
	}

	return noti;
}

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

EXPORT_API int notification_post_with_event_cb_for_uid(notification_h noti, event_handler_cb cb,
							void *userdata, uid_t uid)
{
	int ret;
	int priv_id;
	notification_event_cb_info_s *info = NULL;
	GList *find_list;

	if (noti == NULL || cb == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	if (noti->type <= NOTIFICATION_TYPE_NONE || noti->type > NOTIFICATION_TYPE_MAX)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	noti->insert_time = time(NULL);
	noti->event_flag = true;
	noti->uid = uid;

	ret = notification_ipc_request_insert(noti, &priv_id);
	if (ret != NOTIFICATION_ERROR_NONE)
		return ret;

	noti->priv_id = priv_id;

	__noti_event_cb_list = g_list_first(__noti_event_cb_list);
	find_list = g_list_find_custom(__noti_event_cb_list, GINT_TO_POINTER(priv_id),
				       (GCompareFunc)__priv_id_compare);

	if (find_list) {
		info = g_list_nth_data(find_list, 0);
		info->cb = cb;
		info->userdata = userdata;
	} else {
		info = (notification_event_cb_info_s *)malloc(sizeof(notification_cb_info_s));
		if (info == NULL) {
			NOTIFICATION_ERR("malloc failed");
			return NOTIFICATION_ERROR_OUT_OF_MEMORY;
		}
		info->priv_id = priv_id;
		info->cb = cb;
		info->userdata = userdata;
		__noti_event_cb_list = g_list_append(__noti_event_cb_list, info);
	}

	return ret;
}

EXPORT_API int notification_post_with_event_cb(notification_h noti, event_handler_cb cb, void *userdata)
{
	return notification_post_with_event_cb_for_uid(noti, cb, userdata, getuid());
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
		&& event_type <= NOTIFICATION_EVENT_TYPE_HIDDEN_BY_TIMEOUT) ||
		(event_type >= NOTIFICATION_EVENT_TYPE_PRESSED
		&& event_type <= NOTIFICATION_EVENT_TYPE_DELETED)))
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	ret = notification_get_event_flag(noti, &event_flag);
	if (ret != NOTIFICATION_ERROR_NONE || event_flag == false)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	ret = notification_ipc_send_event(noti, event_type, -1);

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
		&& event_type <= NOTIFICATION_EVENT_TYPE_HIDDEN_BY_TIMEOUT) ||
		(event_type >= NOTIFICATION_EVENT_TYPE_PRESSED
		&& event_type <= NOTIFICATION_EVENT_TYPE_DELETED)))
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	ret = notification_ipc_send_event(NULL, event_type, priv_id);
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
		NOTIFICATION_ERR("failed notification_get_id");
		return ret;
	}

	ret = notification_ipc_check_event_receiver(priv_id, available);

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
	char *ret_str;
	bundle *args;

	if (noti == NULL || key == NULL || value == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	if (noti->args == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	args = noti->args;

	bundle_get_str(args, key, &ret_str);
	if (ret_str == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	*value = _create_bundle_from_bundle_raw((bundle_raw *)ret_str);
	if (*value == NULL)
		return NOTIFICATION_ERROR_IO_ERROR;

	return NOTIFICATION_ERROR_NONE;
}
