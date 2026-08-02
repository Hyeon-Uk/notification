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

#include <aul.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <tizen_type.h>
#include <tzplatform_config.h>

#include <notification.h>
#include <notification_list.h>
#include <notification_debug.h>
#include <notification_private.h>
#include <notification_setting.h>
#include <notification_setting_internal.h>
#include <notification_tidl.h>

typedef struct _noti_dnd_cb_info noti_dnd_cb_info_s;

struct _noti_dnd_cb_info {
	dnd_changed_cb callback;
	void *user_data;
};

static GHashTable *_noti_dnd_cb_hash = NULL;

/* LCOV_EXCL_START */
EXPORT_API int notification_setting_get_setting_array_for_uid(notification_setting_h *setting_array, int *count, uid_t uid)
{
	if (setting_array == NULL || count == NULL) {
		ERR("Invalid parameter");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	return notification_tidl_request_get_setting_array(setting_array, count, uid);
}

EXPORT_API int notification_setting_get_setting_array(notification_setting_h *setting_array, int *count)
{
	return notification_setting_get_setting_array_for_uid(setting_array, count, aul_getuid());
}

EXPORT_API int notification_setting_get_setting_by_appid_for_uid(const char *app_id, notification_setting_h *setting, uid_t uid)
{
	if (app_id == NULL || setting == NULL) {
		ERR("Invalid parameter");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	return notification_tidl_request_get_setting_by_app_id(app_id, setting, uid);
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
EXPORT_API int notification_setting_get_setting_by_package_name(const char *package_name, notification_setting_h *setting)
{
	return notification_setting_get_setting_by_appid_for_uid(package_name, setting, aul_getuid());
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
EXPORT_API int notification_setting_get_setting(notification_setting_h *setting)
{
	int ret;
	char *app_id = NULL;

	app_id = notification_get_app_id_by_pid(getpid());
	if (app_id == NULL)
		return NOTIFICATION_ERROR_NOT_EXIST_ID;

	ret = notification_setting_get_setting_by_package_name(app_id, setting);

	free(app_id);

	return ret;
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
EXPORT_API int notification_setting_get_package_name(notification_setting_h setting, char **value)
{
	if (setting == NULL || value == NULL) {
		ERR("Invalid parameter");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	if (setting->package_name == NULL) {
		ERR("setting->package_name is null");
		return NOTIFICATION_ERROR_NOT_EXIST_ID;
	}

	*value = SAFE_STRDUP(setting->package_name);

	return NOTIFICATION_ERROR_NONE;
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
EXPORT_API int notification_setting_get_appid(notification_setting_h setting, char **app_id)
{
	if (setting == NULL || app_id == NULL) {
		ERR("Invalid parameter");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	if (setting->app_id == NULL) {
		ERR("setting->app_id is null");
		return NOTIFICATION_ERROR_NOT_EXIST_ID;
	}

	*app_id = SAFE_STRDUP(setting->app_id);

	return NOTIFICATION_ERROR_NONE;
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
EXPORT_API int notification_setting_get_allow_to_notify(notification_setting_h setting, bool *value)
{
	if (setting == NULL || value == NULL) {
		ERR("Invalid parameter");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	*value = setting->allow_to_notify;

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_setting_set_allow_to_notify(notification_setting_h setting, bool value)
{
	if (setting == NULL) {
		ERR("Invalid parameter");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	setting->allow_to_notify = value;

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_setting_get_do_not_disturb_except(notification_setting_h setting, bool *value)
{
	if (setting == NULL || value == NULL) {
		ERR("Invalid parameter");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	*value = setting->do_not_disturb_except;

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_setting_set_do_not_disturb_except(notification_setting_h setting, bool value)
{
	if (setting == NULL) {
		ERR("Invalid parameter");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	setting->do_not_disturb_except = value;

	return NOTIFICATION_ERROR_NONE;
}
/* LCOV_EXCL_STOP */

EXPORT_API int notification_setting_get_visibility_class(notification_setting_h setting, int *value)
{
	if (setting == NULL || value == NULL) {
		ERR("Invalid parameter");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	*value = setting->visibility_class;

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_setting_set_visibility_class(notification_setting_h setting, int value)
{
	if (setting == NULL) {
		ERR("Invalid parameter");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	setting->visibility_class = value;

	return NOTIFICATION_ERROR_NONE;
}

/* LCOV_EXCL_START */
EXPORT_API int notification_setting_get_pop_up_notification(notification_setting_h setting, bool *value)
{
	if (setting == NULL || value == NULL) {
		ERR("Invalid parameter");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	*value = setting->pop_up_notification;
	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_setting_set_pop_up_notification(notification_setting_h setting, bool value)
{
	if (setting == NULL) {
		ERR("Invalid parameter");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	setting->pop_up_notification = value;
	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_setting_get_lock_screen_content(notification_setting_h setting, lock_screen_content_level_e *level)
{
	if (setting == NULL || level == NULL) {
		ERR("Invalid parameter");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	*level = setting->lock_screen_content_level;

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_setting_set_lock_screen_content(notification_setting_h setting, lock_screen_content_level_e level)
{
	if (setting == NULL) {
		ERR("Invalid parameter");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	setting->lock_screen_content_level = level;

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_setting_get_app_disabled(notification_setting_h setting, bool *value)
{
	if (setting == NULL || value == NULL) {
		ERR("Invalid parameter");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	*value = setting->app_disabled;

	return NOTIFICATION_ERROR_NONE;
}
/* LCOV_EXCL_STOP */

EXPORT_API int notification_setting_update_setting_for_uid(notification_setting_h setting, uid_t uid)
{
	if (setting == NULL) {
		ERR("Invalid parameter");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	return notification_tidl_update_setting(setting, uid);
}

EXPORT_API int notification_setting_update_setting(notification_setting_h setting)
{
	return notification_setting_update_setting_for_uid(setting, aul_getuid());
}

EXPORT_API int notification_setting_free_notification(notification_setting_h setting)
{
	if (setting == NULL) {
		ERR("Invalid parameter");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	SAFE_FREE(setting->package_name);
	SAFE_FREE(setting->app_id);

	/* add codes to free all properties */

	SAFE_FREE(setting);

	return NOTIFICATION_ERROR_NONE;
}

/* LCOV_EXCL_START */
EXPORT_API int notification_system_setting_load_system_setting_for_uid(notification_system_setting_h *system_setting, uid_t uid)
{
	if (system_setting == NULL) {
		ERR("NOTIFICATION_ERROR_INVALID_PARAMETER");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	return notification_tidl_request_load_system_setting(system_setting, uid);
}

EXPORT_API int notification_system_setting_load_system_setting(notification_system_setting_h *system_setting)
{
	return notification_system_setting_load_system_setting_for_uid(system_setting, aul_getuid());
}

EXPORT_API int notification_system_setting_update_system_setting_for_uid(notification_system_setting_h system_setting, uid_t uid)
{
	if (system_setting == NULL) {
		ERR("Invalid parameter");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	return notification_tidl_update_system_setting(system_setting, uid);
}

EXPORT_API int notification_system_setting_update_system_setting(notification_system_setting_h system_setting)
{
	return notification_system_setting_update_system_setting_for_uid(system_setting, aul_getuid());
}

EXPORT_API int notification_system_setting_free_system_setting(notification_system_setting_h system_setting)
{
	if (system_setting == NULL) {
		ERR("Invalid parameter");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	/* add codes to free all properties */

	if (system_setting->dnd_allow_exceptions != NULL)
		g_list_free_full(system_setting->dnd_allow_exceptions, free);

	SAFE_FREE(system_setting);

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_system_setting_get_do_not_disturb(notification_system_setting_h system_setting, bool *value)
{
	if (system_setting == NULL || value == NULL) {
		ERR("Invalid parameter");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	*value = system_setting->do_not_disturb;

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_system_setting_set_do_not_disturb(notification_system_setting_h system_setting, bool value)
{
	if (system_setting == NULL) {
		ERR("Invalid parameter");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	system_setting->do_not_disturb = value;

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_system_setting_get_visibility_class(notification_system_setting_h system_setting, int *value)
{
	if (system_setting == NULL || value == NULL) {
		ERR("Invalid parameter");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	*value = system_setting->visibility_class;

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_system_setting_set_visibility_class(notification_system_setting_h system_setting, int value)
{
	if (system_setting == NULL) {
		ERR("Invalid parameter");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	system_setting->visibility_class = value;

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_system_setting_dnd_schedule_get_enabled(notification_system_setting_h system_setting, bool *enabled)
{
	if (system_setting == NULL || enabled == NULL) {
		ERR("Invalid parameter");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	*enabled = system_setting->dnd_schedule_enabled;

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_system_setting_dnd_schedule_set_enabled(notification_system_setting_h system_setting, bool enabled)
{
	if (system_setting == NULL) {
		ERR("Invalid parameter");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	system_setting->dnd_schedule_enabled = enabled;

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_system_setting_dnd_schedule_get_day(notification_system_setting_h system_setting, int *day)
{
	if (system_setting == NULL || day == NULL) {
		ERR("Invalid parameter");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	*day = system_setting->dnd_schedule_day;

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_system_setting_dnd_schedule_set_day(notification_system_setting_h system_setting, int day)
{
	if (system_setting == NULL) {
		ERR("Invalid parameter");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	system_setting->dnd_schedule_day = day;

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_system_setting_dnd_schedule_get_start_time(notification_system_setting_h system_setting, int *hour, int *min)
{
	if (system_setting == NULL || hour == NULL || min == NULL) {
		ERR("Invalid parameter");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	*hour = system_setting->dnd_start_hour;
	*min = system_setting->dnd_start_min;

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_system_setting_dnd_schedule_set_start_time(notification_system_setting_h system_setting, int hour, int min)
{
	if (system_setting == NULL) {
		ERR("Invalid parameter");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	system_setting->dnd_start_hour = hour;
	system_setting->dnd_start_min = min;

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_system_setting_dnd_schedule_get_end_time(notification_system_setting_h system_setting, int *hour, int *min)
{
	if (system_setting == NULL || hour == NULL || min == NULL) {
		ERR("Invalid parameter");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	*hour = system_setting->dnd_end_hour;
	*min = system_setting->dnd_end_min;

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_system_setting_dnd_schedule_set_end_time(notification_system_setting_h system_setting, int hour, int min)
{
	if (system_setting == NULL) {
		ERR("Invalid parameter");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	system_setting->dnd_end_hour = hour;
	system_setting->dnd_end_min = min;

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_system_setting_get_lock_screen_content(notification_system_setting_h system_setting, lock_screen_content_level_e *level)
{
	if (system_setting == NULL || level == NULL) {
		ERR("Invalid parameter");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	*level = system_setting->lock_screen_content_level;

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_system_setting_set_lock_screen_content(notification_system_setting_h system_setting, lock_screen_content_level_e level)
{
	if (system_setting == NULL) {
		ERR("Invalid parameter");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	system_setting->lock_screen_content_level = level;

	return NOTIFICATION_ERROR_NONE;
}

static gint _dnd_allow_exception_compare(gconstpointer a, gconstpointer b)
{
	dnd_allow_exception_h dnd_allow_exception_data_a;

	if (a == NULL)
		return -1;

	dnd_allow_exception_data_a = (dnd_allow_exception_h)a;

	if (dnd_allow_exception_data_a->type == GPOINTER_TO_INT(b))
		return 0;

	return -1;
}

EXPORT_API int notification_system_setting_get_dnd_allow_exceptions(notification_system_setting_h system_setting, dnd_allow_exception_type_e type, int *value)
{
	dnd_allow_exception_h dnd_allow_exception_data;
	GList *list;

	if (system_setting == NULL || value == NULL) {
		ERR("Invalid parameter");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	list = g_list_find_custom(system_setting->dnd_allow_exceptions, GINT_TO_POINTER(type), (GCompareFunc)_dnd_allow_exception_compare);
	if (list) {
		dnd_allow_exception_data = (dnd_allow_exception_h)list->data;
		*value = dnd_allow_exception_data->value;
	} else {
		ERR("Invalid type");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_system_setting_set_dnd_allow_exceptions(notification_system_setting_h system_setting, dnd_allow_exception_type_e type, int value)
{
	dnd_allow_exception_h dnd_allow_exception_data;
	GList *list = NULL;

	if (system_setting == NULL) {
		ERR("Invalid parameter");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	list = g_list_first(system_setting->dnd_allow_exceptions);
	list = g_list_find_custom(list, GINT_TO_POINTER(type), (GCompareFunc)_dnd_allow_exception_compare);

	if (list) {
		dnd_allow_exception_data = (dnd_allow_exception_h)list->data;
		dnd_allow_exception_data->value = value;
	} else {
		dnd_allow_exception_data = (dnd_allow_exception_h)malloc(sizeof(struct notification_system_setting_dnd_allow_exception));
		if (dnd_allow_exception_data == NULL)
			return NOTIFICATION_ERROR_OUT_OF_MEMORY;

		dnd_allow_exception_data->type = type;
		dnd_allow_exception_data->value = value;
		system_setting->dnd_allow_exceptions = g_list_append(
				system_setting->dnd_allow_exceptions, dnd_allow_exception_data);
	}

	return NOTIFICATION_ERROR_NONE;
}

static gint _noti_dnd_cb_compare(gconstpointer a, gconstpointer b)
{
	noti_dnd_cb_info_s *info = NULL;

	if (a == NULL)
		return -1;

	info = (noti_dnd_cb_info_s *)a;
	if (info->callback == b)
		return 0;

	return 1;
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
void notification_call_dnd_changed_cb_for_uid(int do_not_disturb, uid_t uid)
{
	GList *noti_dnd_cb_list = NULL;
	noti_dnd_cb_info_s *dnd_data = NULL;

	if (_noti_dnd_cb_hash == NULL)
		return;

	noti_dnd_cb_list = (GList *)g_hash_table_lookup(_noti_dnd_cb_hash, GUINT_TO_POINTER(uid));
	if (noti_dnd_cb_list == NULL) {
		ERR("Invalid data");
		return;
	}

	noti_dnd_cb_list = g_list_first(noti_dnd_cb_list);

	for (; noti_dnd_cb_list != NULL; noti_dnd_cb_list = noti_dnd_cb_list->next) {
		dnd_data = noti_dnd_cb_list->data;

		if (dnd_data != NULL && dnd_data->callback != NULL)
			dnd_data->callback(dnd_data->user_data, do_not_disturb);
	}
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
EXPORT_API int notification_register_system_setting_dnd_changed_cb_for_uid(dnd_changed_cb callback, void *user_data, uid_t uid)
{
	GList *noti_dnd_list = NULL;
	GList *noti_dnd_found_list = NULL;
	noti_dnd_cb_info_s *dnd_data = NULL;

	if (callback == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	if (notification_tidl_monitor_init() != NOTIFICATION_ERROR_NONE) {
		ERR("Failed to init monitor");
		return NOTIFICATION_ERROR_IO_ERROR;
	}

	if (_noti_dnd_cb_hash == NULL)
		_noti_dnd_cb_hash = g_hash_table_new(g_direct_hash, g_direct_equal);

	dnd_data = (noti_dnd_cb_info_s *)malloc(sizeof(noti_dnd_cb_info_s));
	if (dnd_data == NULL)
		return NOTIFICATION_ERROR_OUT_OF_MEMORY;

	dnd_data->callback = callback;
	dnd_data->user_data = user_data;

	noti_dnd_list = (GList *)g_hash_table_lookup(_noti_dnd_cb_hash, GUINT_TO_POINTER(uid));

	if (noti_dnd_list == NULL) {
		noti_dnd_list = g_list_append(noti_dnd_list, dnd_data);
		g_hash_table_insert(_noti_dnd_cb_hash, GUINT_TO_POINTER(uid), noti_dnd_list);
	} else {
		noti_dnd_found_list = g_list_find_custom(noti_dnd_list, (gconstpointer)callback,
							 (GCompareFunc)_noti_dnd_cb_compare);
		if (noti_dnd_found_list) {
			ERR("Already existing callback");
			free(dnd_data);
			return NOTIFICATION_ERROR_INVALID_PARAMETER;
		}

		noti_dnd_list = g_list_append(noti_dnd_list, dnd_data);
	}

	return NOTIFICATION_ERROR_NONE;
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
EXPORT_API int notification_register_system_setting_dnd_changed_cb(dnd_changed_cb callback, void *user_data)
{
	return notification_register_system_setting_dnd_changed_cb_for_uid(callback, user_data, aul_getuid());
}

EXPORT_API int notification_unregister_system_setting_dnd_changed_cb_for_uid(dnd_changed_cb callback, uid_t uid)
{
	GList *noti_dnd_cb_list = NULL;
	GList *noti_dnd_del_list = NULL;
	noti_dnd_cb_info_s *dnd_data = NULL;

	if (callback == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	if (_noti_dnd_cb_hash == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	noti_dnd_cb_list = (GList *)g_hash_table_lookup(_noti_dnd_cb_hash, GUINT_TO_POINTER(uid));

	if (noti_dnd_cb_list == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	noti_dnd_del_list = g_list_find_custom(noti_dnd_cb_list, (gconstpointer)callback,
					(GCompareFunc)_noti_dnd_cb_compare);

	if (noti_dnd_del_list) {
		dnd_data = g_list_nth_data(noti_dnd_del_list, 0);
		noti_dnd_cb_list = g_list_delete_link(noti_dnd_cb_list, noti_dnd_del_list);
		free(dnd_data);
	} else {
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	if (noti_dnd_cb_list == NULL) {
		g_hash_table_steal(_noti_dnd_cb_hash, GUINT_TO_POINTER(uid));
	} else {
		noti_dnd_cb_list = g_list_first(noti_dnd_cb_list);
		g_hash_table_replace(_noti_dnd_cb_hash, GUINT_TO_POINTER(uid), noti_dnd_cb_list);
	}

	if (g_hash_table_size(_noti_dnd_cb_hash) == 0)
		notification_tidl_monitor_fini();

	return NOTIFICATION_ERROR_NONE;
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
EXPORT_API int notification_unregister_system_setting_dnd_changed_cb(dnd_changed_cb callback)
{
	return notification_unregister_system_setting_dnd_changed_cb_for_uid(callback, aul_getuid());
}
/* LCOV_EXCL_STOP */
