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
#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <vconf.h>
#include <pkgmgr-info.h>
#include <package_manager.h>
#include <app_control_internal.h>

#include <notification.h>
#include <notification_internal.h>
#include <notification_db.h>
#include <notification_list.h>
#include <notification_noti.h>
#include <notification_debug.h>
#include <notification_private.h>
#include <notification_setting.h>
#include <notification_setting_internal.h>
#include <notification_setting_service.h>

#define NOTI_BURST_DELETE_UNIT 10
#define ERR_BUFFER_SIZE		1024

static void __free_and_set(void **target_ptr, void *new_ptr)
{
	if (target_ptr != NULL) {
		if (*target_ptr != NULL)
			free(*target_ptr);

		*target_ptr = new_ptr;
	}
}

static int _notification_noti_bind_query_text(sqlite3_stmt *stmt, const char *name,
					 const char *str)
{
	int ret = 0;
	int index = 0;

	index = sqlite3_bind_parameter_index(stmt, name);
	if (index == 0) {
		NOTIFICATION_ERR("Insert : invalid column name");
		return NOTIFICATION_ERROR_FROM_DB;
	}

	ret = sqlite3_bind_text(stmt, index, NOTIFICATION_CHECK_STR(str), -1,
			      SQLITE_STATIC);
	if (ret != SQLITE_OK) {
		NOTIFICATION_ERR("Insert text : %s",
				 NOTIFICATION_CHECK_STR(str));
		return NOTIFICATION_ERROR_FROM_DB;
	}

	return NOTIFICATION_ERROR_NONE;
}

static int _notification_noti_bind_query_double(sqlite3_stmt *stmt, const char *name,
					 double val)
{
	int ret = 0;
	int index = 0;

	index = sqlite3_bind_parameter_index(stmt, name);
	if (index == 0) {
		NOTIFICATION_ERR("Insert : invalid column name");
		return NOTIFICATION_ERROR_FROM_DB;
	}

	ret = sqlite3_bind_double(stmt, index, val);
	if (ret != SQLITE_OK) {
		NOTIFICATION_ERR("Insert double : %f", val);
		return NOTIFICATION_ERROR_FROM_DB;
	}

	return NOTIFICATION_ERROR_NONE;
}

static int _notification_noti_check_priv_id(notification_h noti, sqlite3 *db)
{
	int result = 0;
	int ret = NOTIFICATION_ERROR_NONE;
	char *query = NULL;
	sqlite3_stmt *stmt = NULL;

	/* Make query to check priv_id exist */
	query = sqlite3_mprintf("SELECT count(*) FROM noti_list WHERE caller_pkgname = '%s' AND priv_id = %d",
		 noti->caller_pkgname, noti->priv_id);
	if (query == NULL) {
		ret = NOTIFICATION_ERROR_OUT_OF_MEMORY;
		goto err;
	}

	ret = sqlite3_prepare(db, query, strlen(query), &stmt, NULL);
	if (ret != SQLITE_OK) {
		NOTIFICATION_ERR("Get count DB err(%d) : %s", ret,
				 sqlite3_errmsg(db));
		ret = NOTIFICATION_ERROR_FROM_DB;
		goto err;
	}

	ret = sqlite3_step(stmt);
	if (ret == SQLITE_ROW)
		result = sqlite3_column_int(stmt, 0);
	else
		result = 0;

	sqlite3_finalize(stmt);

	/* If result > 0, there is priv_id in DB */
	if (result > 0)
		ret = NOTIFICATION_ERROR_ALREADY_EXIST_ID;

err:
	if (query)
		sqlite3_free(query);

	return ret;
}

static int _notification_noti_get_internal_group_id_by_priv_id(const char *pkgname,
							       int priv_id,
							       sqlite3 *db)
{
	char *query = NULL;
	sqlite3_stmt *stmt = NULL;
	int ret = NOTIFICATION_ERROR_NONE, result = 0;

	query = sqlite3_mprintf("SELECT internal_group_id FROM noti_list WHERE caller_pkgname = '%s' AND priv_id = %d",
		 pkgname, priv_id);
	if (query == NULL) {
		ret = NOTIFICATION_ERROR_OUT_OF_MEMORY;
		goto err;
	}

	ret = sqlite3_prepare(db, query, strlen(query), &stmt, NULL);
	if (ret != SQLITE_OK) {
		NOTIFICATION_ERR("Get count DB err(%d) : %s", ret,
				 sqlite3_errmsg(db));
		ret = NOTIFICATION_ERROR_FROM_DB;
		goto err;
	}

	ret = sqlite3_step(stmt);
	if (ret == SQLITE_ROW)
		result = sqlite3_column_int(stmt, 0);
	else
		result = 0;

err:
	if (stmt)
		sqlite3_finalize(stmt);

	if (query)
		sqlite3_free(query);

	if (ret != NOTIFICATION_ERROR_NONE)
		NOTIFICATION_ERR("failed to internal group ID:%d", ret);

	return result;
}

static int _insertion_query_create(notification_h noti, char **query)
{
	int i = 0;
	int b_encode_len = 0;
	char *args = NULL;
	char *group_args = NULL;
	char *b_image_path = NULL;
	char *b_execute_option = NULL;
	char *b_service_responding = NULL;
	char *b_service_single_launch = NULL;
	char *b_service_multi_launch = NULL;
	char *b_event_handler[NOTIFICATION_EVENT_TYPE_MAX] = { NULL , };
	char *b_text = NULL;
	char *b_key = NULL;
	char *b_format_args = NULL;
	int flag_simmode = 0;

	if (query == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	/* Decode bundle to insert DB */
	if (noti->args)
		bundle_encode(noti->args, (bundle_raw **)&args, &b_encode_len);

	if (noti->group_args)
		bundle_encode(noti->group_args, (bundle_raw **)&group_args,
			      &b_encode_len);

	if (noti->b_execute_option)
		bundle_encode(noti->b_execute_option,
			      (bundle_raw **)&b_execute_option, &b_encode_len);

	if (noti->b_service_responding)
		bundle_encode(noti->b_service_responding,
			      (bundle_raw **)&b_service_responding, &b_encode_len);

	if (noti->b_service_single_launch)
		bundle_encode(noti->b_service_single_launch,
			      (bundle_raw **)&b_service_single_launch, &b_encode_len);

	if (noti->b_service_multi_launch)
		bundle_encode(noti->b_service_multi_launch,
			      (bundle_raw **)&b_service_multi_launch, &b_encode_len);


	for (i = 0; i < NOTIFICATION_EVENT_TYPE_MAX; i++) {
		if (noti->b_event_handler[i])
			bundle_encode(noti->b_event_handler[i],
					(bundle_raw **)&b_event_handler[i], &b_encode_len);
	}

	if (noti->b_text)
		bundle_encode(noti->b_text, (bundle_raw **)&b_text, &b_encode_len);

	if (noti->b_key)
		bundle_encode(noti->b_key, (bundle_raw **)&b_key, &b_encode_len);

	if (noti->b_format_args)
		bundle_encode(noti->b_format_args,
			      (bundle_raw **)&b_format_args, &b_encode_len);

	if (noti->b_image_path)
		bundle_encode(noti->b_image_path,
			      (bundle_raw **)&b_image_path, &b_encode_len);

	/* Check only simmode property is enable */
	if (noti->flags_for_property & NOTIFICATION_PROP_DISPLAY_ONLY_SIMMODE)
		flag_simmode = 1;

	*query = sqlite3_mprintf("INSERT INTO noti_list ("
		"type, "
		"layout, "
		"caller_pkgname, launch_pkgname, "
		"image_path, "
		"group_id, internal_group_id,"
		"title_key, "
		"b_text, b_key, tag, b_format_args, num_format_args, "
		"text_domain, text_dir, "
		"time, insert_time, "
		"args, group_args, "
		"b_execute_option, "
		"b_service_responding, b_service_single_launch, b_service_multi_launch, "
		"b_event_handler_click_on_button_1, b_event_handler_click_on_button_2, b_event_handler_click_on_button_3, "
		"b_event_handler_click_on_button_4, b_event_handler_click_on_button_5, b_event_handler_click_on_button_6, "
		"b_event_handler_click_on_icon, b_event_handler_click_on_thumbnail, b_event_handler_click_on_text_input_button, "
		"sound_type, sound_path, vibration_type, vibration_path, led_operation, led_argb, led_on_ms, led_off_ms, "
		"flags_for_property, flag_simmode, display_applist, "
		"progress_size, progress_percentage, "
		"ongoing_flag, ongoing_value_type, ongoing_current, ongoing_duration, "
		"auto_remove, default_button_index, timeout, text_input_max_length, event_flag, uid) values ("
		"%d, "
		"%d, "
		"'%s', '%s', "
		"'%s', "
		"%d, %d,"
		"$title_key, "
		"'%s', '%s', $tag, '%s', %d, "
		"'%s', '%s', "
		"%d, %d, "
		"'%s', '%s', "
		"'%s', "
		"'%s', '%s', '%s', "
		"'%s', '%s', '%s', "
		"'%s', '%s', '%s', "
		"'%s', '%s', '%s', "
		"%d, '%s', %d, '%s', %d, %d, %d, %d,"
		"%d, %d, %d, "
		"$progress_size, $progress_percentage, "
		"%d, %d, %d, %d, "
		"%d, %d, %d, %d, %d, %d)",
		noti->type,
		noti->layout,
		NOTIFICATION_CHECK_STR(noti->caller_pkgname),
		NOTIFICATION_CHECK_STR(noti->launch_pkgname),
		NOTIFICATION_CHECK_STR(b_image_path), noti->group_id,
		noti->internal_group_id,
		NOTIFICATION_CHECK_STR(b_text), NOTIFICATION_CHECK_STR(b_key),
		NOTIFICATION_CHECK_STR(b_format_args), noti->num_format_args,
		NOTIFICATION_CHECK_STR(noti->domain),
		NOTIFICATION_CHECK_STR(noti->dir), (int)noti->time,
		(int)noti->insert_time, NOTIFICATION_CHECK_STR(args),
		NOTIFICATION_CHECK_STR(group_args),
		NOTIFICATION_CHECK_STR(b_execute_option),
		NOTIFICATION_CHECK_STR(b_service_responding),
		NOTIFICATION_CHECK_STR(b_service_single_launch),
		NOTIFICATION_CHECK_STR(b_service_multi_launch),
		NOTIFICATION_CHECK_STR(b_event_handler[NOTIFICATION_EVENT_TYPE_CLICK_ON_BUTTON_1]),
		NOTIFICATION_CHECK_STR(b_event_handler[NOTIFICATION_EVENT_TYPE_CLICK_ON_BUTTON_2]),
		NOTIFICATION_CHECK_STR(b_event_handler[NOTIFICATION_EVENT_TYPE_CLICK_ON_BUTTON_3]),
		NOTIFICATION_CHECK_STR(b_event_handler[NOTIFICATION_EVENT_TYPE_CLICK_ON_BUTTON_4]),
		NOTIFICATION_CHECK_STR(b_event_handler[NOTIFICATION_EVENT_TYPE_CLICK_ON_BUTTON_5]),
		NOTIFICATION_CHECK_STR(b_event_handler[NOTIFICATION_EVENT_TYPE_CLICK_ON_BUTTON_6]),
		NOTIFICATION_CHECK_STR(b_event_handler[NOTIFICATION_EVENT_TYPE_CLICK_ON_ICON]),
		NOTIFICATION_CHECK_STR(b_event_handler[NOTIFICATION_EVENT_TYPE_CLICK_ON_THUMBNAIL]),
		NOTIFICATION_CHECK_STR(b_event_handler[NOTIFICATION_EVENT_TYPE_CLICK_ON_TEXT_INPUT_BUTTON]),
		noti->sound_type, NOTIFICATION_CHECK_STR(noti->sound_path),
		noti->vibration_type,
		NOTIFICATION_CHECK_STR(noti->vibration_path),
		noti->led_operation,
		noti->led_argb,
		noti->led_on_ms,
		noti->led_off_ms,
		noti->flags_for_property, flag_simmode, noti->display_applist,
		noti->ongoing_flag,
		noti->ongoing_value_type,
		noti->ongoing_current,
		noti->ongoing_duration,
		noti->auto_remove,
		noti->default_button_index,
		noti->timeout,
		noti->text_input_max_length,
		noti->event_flag,
		noti->uid);

	/* Free decoded data */
	if (args)
		free(args);

	if (group_args)
		free(group_args);

	if (b_execute_option)
		free(b_execute_option);

	if (b_service_responding)
		free(b_service_responding);

	if (b_service_single_launch)
		free(b_service_single_launch);

	if (b_service_multi_launch)
		free(b_service_multi_launch);

	if (b_text)
		free(b_text);

	if (b_key)
		free(b_key);

	if (b_format_args)
		free(b_format_args);

	if (b_image_path)
		free(b_image_path);

	if (*query == NULL)
		return NOTIFICATION_ERROR_OUT_OF_MEMORY;

	return NOTIFICATION_ERROR_NONE;
}

static int _update_query_create(notification_h noti, char **query)
{
	int i = 0;
	int b_encode_len = 0;
	char *args = NULL;
	char *group_args = NULL;
	char *b_image_path = NULL;
	char *b_execute_option = NULL;
	char *b_service_responding = NULL;
	char *b_service_single_launch = NULL;
	char *b_service_multi_launch = NULL;
	char *b_event_handler[NOTIFICATION_EVENT_TYPE_MAX] = { NULL , };
	char *b_text = NULL;
	char *b_key = NULL;
	char *b_format_args = NULL;
	int flag_simmode = 0;

	if (query == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	/* Decode bundle to update DB */
	if (noti->args)
		bundle_encode(noti->args, (bundle_raw **)&args, &b_encode_len);

	if (noti->group_args)
		bundle_encode(noti->group_args, (bundle_raw **)&group_args,
			      &b_encode_len);

	if (noti->b_execute_option)
		bundle_encode(noti->b_execute_option,
			      (bundle_raw **)&b_execute_option, &b_encode_len);

	if (noti->b_service_responding)
		bundle_encode(noti->b_service_responding,
			      (bundle_raw **)&b_service_responding, &b_encode_len);

	if (noti->b_service_single_launch)
		bundle_encode(noti->b_service_single_launch,
			      (bundle_raw **)&b_service_single_launch, &b_encode_len);

	if (noti->b_service_multi_launch)
		bundle_encode(noti->b_service_multi_launch,
			      (bundle_raw **)&b_service_multi_launch, &b_encode_len);

	for (i = 0; i < NOTIFICATION_EVENT_TYPE_MAX; i++) {
		if (noti->b_event_handler[i])
			bundle_encode(noti->b_event_handler[i],
					(bundle_raw **)&b_event_handler[i], &b_encode_len);
	}

	if (noti->b_text)
		bundle_encode(noti->b_text, (bundle_raw **)&b_text, &b_encode_len);

	if (noti->b_key)
		bundle_encode(noti->b_key, (bundle_raw **)&b_key, &b_encode_len);

	if (noti->b_format_args)
		bundle_encode(noti->b_format_args,
			      (bundle_raw **)&b_format_args, &b_encode_len);

	if (noti->b_image_path)
		bundle_encode(noti->b_image_path,
			      (bundle_raw **)&b_image_path, &b_encode_len);

	/* Check only simmode property is enable */
	if (noti->flags_for_property & NOTIFICATION_PROP_DISPLAY_ONLY_SIMMODE)
		flag_simmode = 1;

	*query = sqlite3_mprintf("UPDATE noti_list SET "
		"type = %d, "
		"layout = %d, "
		"launch_pkgname = '%s', "
		"image_path = '%s', "
		"b_text = '%s', b_key = '%s', tag = $tag, "
		"b_format_args = '%s', num_format_args = %d, "
		"text_domain = '%s', text_dir = '%s', "
		"time = %d, insert_time = %d, "
		"args = '%s', group_args = '%s', "
		"b_execute_option = '%s', "
		"b_service_responding = '%s', "
		"b_service_single_launch = '%s', "
		"b_service_multi_launch = '%s', "
		"b_event_handler_click_on_button_1 = '%s', "
		"b_event_handler_click_on_button_2= '%s', "
		"b_event_handler_click_on_button_3= '%s', "
		"b_event_handler_click_on_button_4= '%s', "
		"b_event_handler_click_on_button_5= '%s', "
		"b_event_handler_click_on_button_6= '%s', "
		"b_event_handler_click_on_icon= '%s', "
		"b_event_handler_click_on_thumbnail= '%s', "
		"b_event_handler_click_on_text_input_button= '%s', "
		"sound_type = %d, sound_path = '%s', "
		"vibration_type = %d, vibration_path = '%s', "
		"led_operation = %d, led_argb = %d, "
		"led_on_ms = %d, led_off_ms = %d, "
		"flags_for_property = %d, flag_simmode = %d, "
		"display_applist = %d, "
		"progress_size = $progress_size, progress_percentage = $progress_percentage, "
		"ongoing_flag = %d, ongoing_value_type = %d, ongoing_current = %d, ongoing_duration = %d, "
		"auto_remove = %d, default_button_index = %d, timeout = %d, text_input_max_length = %d, event_flag = %d "
		"where priv_id = %d ",
		noti->type,
		noti->layout,
		NOTIFICATION_CHECK_STR(noti->launch_pkgname),
		NOTIFICATION_CHECK_STR(b_image_path),
		NOTIFICATION_CHECK_STR(b_text), NOTIFICATION_CHECK_STR(b_key),
		NOTIFICATION_CHECK_STR(b_format_args), noti->num_format_args,
		NOTIFICATION_CHECK_STR(noti->domain),
		NOTIFICATION_CHECK_STR(noti->dir),
		(int)noti->time, (int)noti->insert_time,
		NOTIFICATION_CHECK_STR(args), NOTIFICATION_CHECK_STR(group_args),
		NOTIFICATION_CHECK_STR(b_execute_option),
		NOTIFICATION_CHECK_STR(b_service_responding),
		NOTIFICATION_CHECK_STR(b_service_single_launch),
		NOTIFICATION_CHECK_STR(b_service_multi_launch),
		NOTIFICATION_CHECK_STR(b_event_handler[NOTIFICATION_EVENT_TYPE_CLICK_ON_BUTTON_1]),
		NOTIFICATION_CHECK_STR(b_event_handler[NOTIFICATION_EVENT_TYPE_CLICK_ON_BUTTON_2]),
		NOTIFICATION_CHECK_STR(b_event_handler[NOTIFICATION_EVENT_TYPE_CLICK_ON_BUTTON_3]),
		NOTIFICATION_CHECK_STR(b_event_handler[NOTIFICATION_EVENT_TYPE_CLICK_ON_BUTTON_4]),
		NOTIFICATION_CHECK_STR(b_event_handler[NOTIFICATION_EVENT_TYPE_CLICK_ON_BUTTON_5]),
		NOTIFICATION_CHECK_STR(b_event_handler[NOTIFICATION_EVENT_TYPE_CLICK_ON_BUTTON_6]),
		NOTIFICATION_CHECK_STR(b_event_handler[NOTIFICATION_EVENT_TYPE_CLICK_ON_ICON]),
		NOTIFICATION_CHECK_STR(b_event_handler[NOTIFICATION_EVENT_TYPE_CLICK_ON_THUMBNAIL]),
		NOTIFICATION_CHECK_STR(b_event_handler[NOTIFICATION_EVENT_TYPE_CLICK_ON_TEXT_INPUT_BUTTON]),
		noti->sound_type, NOTIFICATION_CHECK_STR(noti->sound_path),
		noti->vibration_type,
		NOTIFICATION_CHECK_STR(noti->vibration_path),
		noti->led_operation,
		noti->led_argb,
		noti->led_on_ms,
		noti->led_off_ms,
		noti->flags_for_property, flag_simmode, noti->display_applist,
		noti->ongoing_flag, noti->ongoing_value_type,
		noti->ongoing_current, noti->ongoing_duration,
		noti->auto_remove, noti->default_button_index, noti->timeout,
		noti->text_input_max_length, noti->event_flag, noti->priv_id);

	/* Free decoded data */
	if (args)
		free(args);

	if (group_args)
		free(group_args);

	if (b_execute_option)
		free(b_execute_option);

	if (b_service_responding)
		free(b_service_responding);

	if (b_service_single_launch)
		free(b_service_single_launch);

	if (b_service_multi_launch)
		free(b_service_multi_launch);

	if (b_text)
		free(b_text);

	if (b_key)
		free(b_key);

	if (b_format_args)
		free(b_format_args);

	if (b_image_path)
		free(b_image_path);

	if (*query == NULL)
		return NOTIFICATION_ERROR_OUT_OF_MEMORY;

	return NOTIFICATION_ERROR_NONE;
}

static void _notification_noti_populate_from_stmt(sqlite3_stmt *stmt, notification_h noti)
{
	int col = 0;
	int i = 0;

	if (stmt == NULL || noti == NULL)
		return ;

	noti->type = sqlite3_column_int(stmt, col++);
	noti->layout = sqlite3_column_int(stmt, col++);
	__free_and_set((void **)&(noti->caller_pkgname), notification_db_column_text(stmt, col++));
	__free_and_set((void **)&(noti->launch_pkgname), notification_db_column_text(stmt, col++));
	noti->b_image_path = notification_db_column_bundle(stmt, col++);
	noti->group_id = sqlite3_column_int(stmt, col++);
	noti->internal_group_id = 0;
	noti->priv_id = sqlite3_column_int(stmt, col++);
	__free_and_set((void **)&(noti->tag), notification_db_column_text(stmt, col++));

	noti->b_text = notification_db_column_bundle(stmt, col++);
	noti->b_key = notification_db_column_bundle(stmt, col++);
	noti->b_format_args = notification_db_column_bundle(stmt, col++);
	noti->num_format_args = sqlite3_column_int(stmt, col++);

	__free_and_set((void **)&(noti->domain), notification_db_column_text(stmt, col++));
	__free_and_set((void **)&(noti->dir), notification_db_column_text(stmt, col++));
	noti->time = sqlite3_column_int(stmt, col++);
	noti->insert_time = sqlite3_column_int(stmt, col++);
	noti->args = notification_db_column_bundle(stmt, col++);
	noti->group_args = notification_db_column_bundle(stmt, col++);

	noti->b_execute_option = notification_db_column_bundle(stmt, col++);
	noti->b_service_responding = notification_db_column_bundle(stmt, col++);
	noti->b_service_single_launch =
	    notification_db_column_bundle(stmt, col++);
	noti->b_service_multi_launch =
	    notification_db_column_bundle(stmt, col++);

	for (i = 0; i < NOTIFICATION_EVENT_TYPE_MAX; i++)
		noti->b_event_handler[i] = notification_db_column_bundle(stmt, col++);


	noti->sound_type = sqlite3_column_int(stmt, col++);
	__free_and_set((void **)&(noti->sound_path), notification_db_column_text(stmt, col++));
	noti->vibration_type = sqlite3_column_int(stmt, col++);
	__free_and_set((void **)&(noti->vibration_path), notification_db_column_text(stmt, col++));
	noti->led_operation = sqlite3_column_int(stmt, col++);
	noti->led_argb = sqlite3_column_int(stmt, col++);
	noti->led_on_ms = sqlite3_column_int(stmt, col++);
	noti->led_off_ms = sqlite3_column_int(stmt, col++);

	noti->flags_for_property = sqlite3_column_int(stmt, col++);
	noti->display_applist = sqlite3_column_int(stmt, col++);
	noti->progress_size = sqlite3_column_double(stmt, col++);
	noti->progress_percentage = sqlite3_column_double(stmt, col++);

	noti->ongoing_flag = sqlite3_column_int(stmt, col++);
	noti->ongoing_value_type = sqlite3_column_int(stmt, col++);
	noti->ongoing_current = sqlite3_column_int(stmt, col++);
	noti->ongoing_duration = sqlite3_column_int(stmt, col++);
	noti->auto_remove = sqlite3_column_int(stmt, col++);
	noti->default_button_index = sqlite3_column_int(stmt, col++);
	noti->timeout = sqlite3_column_int(stmt, col++);
	noti->text_input_max_length = sqlite3_column_int(stmt, col++);
	noti->event_flag = sqlite3_column_int(stmt, col++);
	noti->uid = sqlite3_column_int(stmt, col++);

	noti->app_icon_path = NULL;
	noti->app_name = NULL;
	noti->temp_title = NULL;
	noti->temp_content = NULL;
}

static notification_h _notification_noti_get_item(sqlite3_stmt *stmt)
{
	notification_h noti = NULL;

	noti = (notification_h) calloc(1, sizeof(struct _notification));
	if (noti == NULL)
		return NULL;

	_notification_noti_populate_from_stmt(stmt, noti);

	return noti;
}

int notification_noti_set_tag(const char *tag, char *value, char *buf, int buf_len)
{
	int len_total = 0;

	len_total += (strlen(tag) * 2) + 5 + strlen(value) + 1;

	if (buf_len <= len_total)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	snprintf(buf, buf_len, "<%s>%s</%s>", tag, value, tag);

	return NOTIFICATION_ERROR_NONE;
}

char *notification_noti_strip_tag(const char *tagged_str)
{
	if (tagged_str == NULL)
		return NULL;

	int len_total = strlen(tagged_str);

	if (len_total == 0)
		return NULL;

	char *b_f_e = strstr(tagged_str, ">");
	char *b_e_s = strstr(tagged_str, "</");

	if (b_f_e == NULL || b_e_s == NULL || (b_e_s - b_f_e - 1) <= 0)
		return NULL;

	return strndup(b_f_e + 1, b_e_s - b_f_e - 1);
}

int notification_noti_get_tag_type(const char *tagged_str)
{
	if (tagged_str == NULL)
		return TAG_TYPE_INVALID;

	if (strlen(tagged_str) == 0)
		return TAG_TYPE_INVALID;

	char *b_f_s = strstr(tagged_str, "<");
	char *b_f_e = strstr(tagged_str, ">");

	if (b_f_s == NULL || b_f_e == NULL || (b_f_e - b_f_s - 1) <= 0)
		return TAG_TYPE_INVALID;

	char *start = b_f_s + 1;
	int len_tag = b_f_e - b_f_s - 1;

	if (strncmp(start, TAG_TIME, len_tag) == 0)
		return TAG_TYPE_TIME;

	return TAG_TYPE_INVALID;
}

static int _notification_noti_update_priv_id(sqlite3 *db, int rowid)
{
	int ret = NOTIFICATION_ERROR_NONE;
	char *query = NULL;

	if (db == NULL) {
		NOTIFICATION_ERR("NOTIFICATION_ERROR_INVALID_PARAMETER");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	query = sqlite3_mprintf("UPDATE noti_list SET "
			"priv_id = %d, internal_group_id = %d WHERE rowid = %d",
			rowid, rowid, rowid);
	if (query == NULL) {
		ret = NOTIFICATION_ERROR_OUT_OF_MEMORY;
		goto err;
	}

	ret = notification_db_exec(db, query, NULL);

err:
	if (query)
		sqlite3_free(query);

	return ret;
}

static int __get_setting_from_app_control(notification_h noti, notification_setting_h *setting)
{
	notification_setting_h setting_new = NULL;
	app_control_h app_control = NULL;
	bundle *b = NULL;
	char *app_id = NULL;
	int ret;

	ret = notification_get_execute_option(noti,
				NOTIFICATION_EXECUTE_TYPE_SINGLE_LAUNCH,
				NULL,
				&b);
	if (ret != NOTIFICATION_ERROR_NONE || b == NULL) {
		NOTIFICATION_ERR("notification_get_execute_option failed [%x]", ret);
		return ret;
	}

	ret = app_control_create(&app_control);
	if (ret != APP_CONTROL_ERROR_NONE) {
		NOTIFICATION_ERR("app_control_create failed [%x]", ret);
		goto out;
	}

	ret = app_control_import_from_bundle(app_control, b);
	if (ret != APP_CONTROL_ERROR_NONE) {
		NOTIFICATION_ERR("app_control_import_from_bundle failed [%x]", ret);
		goto out;
	}

	ret = app_control_get_app_id(app_control, &app_id);
	if (ret != APP_CONTROL_ERROR_NONE || app_id == NULL) {
		NOTIFICATION_ERR("app_control_get_app_id failed [%x]", ret);
		goto out;
	}

	ret = noti_setting_service_get_setting_by_appid(app_id, &setting_new, noti->uid);
	if (ret != APP_CONTROL_ERROR_NONE || setting == NULL) {
		NOTIFICATION_ERR("noti_setting_service_get_setting_by_appid failed [%x]", ret);
		goto out;
	}

	*setting = setting_new;

out:
	if (app_id)
		free(app_id);

	if (app_control)
		app_control_destroy(app_control);

	return ret;
}

static bool _is_allowed_to_notify(notification_h noti)
{
	notification_setting_h setting = NULL;
	bool allow_to_notify = true;
	bool app_disabled = false;
	bool ret = true;
	int err;

	err = noti_setting_service_get_setting_by_appid(noti->caller_pkgname, &setting, noti->uid);
	if (err != NOTIFICATION_ERROR_NONE) {
		err = __get_setting_from_app_control(noti, &setting);
		if (err != NOTIFICATION_ERROR_NONE)
			return ret;
	}

	err = notification_setting_get_allow_to_notify(setting, &allow_to_notify);
	if (err != NOTIFICATION_ERROR_NONE) {
		NOTIFICATION_ERR("get_allow_to_notify failed [%x]", err);
		goto out;
	}

	err = notification_setting_get_app_disabled(setting, &app_disabled);
	if (err != NOTIFICATION_ERROR_NONE) {
		NOTIFICATION_ERR("get_app_disabled failed [%x]", err);
		goto out;
	}

	if (!allow_to_notify || app_disabled)
		ret = false;

out:
	if (setting)
		notification_setting_free_notification(setting);

	return ret;
}

static int _handle_do_not_disturb_option(notification_h noti)
{
	int err = NOTIFICATION_ERROR_NONE;
	bool do_not_disturb = false;
	bool do_not_disturb_exception = false;
	notification_setting_h setting = NULL;
	notification_system_setting_h system_setting = NULL;

	if (noti == NULL) {
		NOTIFICATION_ERR("NOTIFICATION_ERROR_INVALID_PARAMETER");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	/* Get system setting */
	err = noti_system_setting_load_system_setting(&system_setting, noti->uid);
	if (err != NOTIFICATION_ERROR_NONE) {
		NOTIFICATION_ERR("_load_system_setting failed [%d]", err);
		goto out;
	}

	err = notification_system_setting_get_do_not_disturb(system_setting, &do_not_disturb);
	if (err != NOTIFICATION_ERROR_NONE) {
		NOTIFICATION_ERR("_get_do_not_disturb failed [%d]", err);
		goto out;
	}

	NOTIFICATION_DBG("do_not_disturb [%d]", do_not_disturb);

	if (do_not_disturb) {
		/* Check exception option of the caller package */
		err = noti_setting_service_get_setting_by_appid(noti->caller_pkgname, &setting, noti->uid);
		if (err != NOTIFICATION_ERROR_NONE) {
			NOTIFICATION_ERR("_get_setting_by_package_name failed [%d]", err);
			goto out;
		}

		err = notification_setting_get_do_not_disturb_except(setting, &do_not_disturb_exception);
		if (err != NOTIFICATION_ERROR_NONE) {
			NOTIFICATION_ERR("notification_setting_get_allow_to_notify failed [%d]", err);
			goto out;
		}

		if (do_not_disturb_exception == false) {
			/* do_not_disturb is ON and do_not_disturb_exception is OFF */
			/* Then add this notification only on quick panel and indicator */
			noti->display_applist = noti->display_applist & (NOTIFICATION_DISPLAY_APP_NOTIFICATION_TRAY | NOTIFICATION_DISPLAY_APP_INDICATOR);
			/* and reset all sound and vibration and led options */
			noti->sound_type = NOTIFICATION_SOUND_TYPE_NONE;
			SAFE_FREE(noti->sound_path);
			noti->vibration_type = NOTIFICATION_VIBRATION_TYPE_NONE;
			SAFE_FREE(noti->vibration_path);
			noti->led_operation = NOTIFICATION_LED_OP_OFF;
			noti->led_argb = 0;
			noti->led_on_ms = 0;
			noti->led_off_ms = 0;
		}
	}

out:
	if (system_setting)
		notification_system_setting_free_system_setting(system_setting);

	if (setting)
		notification_setting_free_notification(setting);

	return err;
}

static bool _is_pop_up_notification(const char *appid, uid_t uid)
{
	int err;
	bool ret = true;
	notification_setting_h setting = NULL;

	err = noti_setting_service_get_setting_by_appid(appid, &setting, uid);
	if (err != NOTIFICATION_ERROR_NONE) {
		NOTIFICATION_ERR("noti_setting_service_get_setting_by_appid failed [%d]", err);
		goto out;
	}

	err = notification_setting_get_pop_up_notification(setting, &ret);
	if (err != NOTIFICATION_ERROR_NONE) {
		NOTIFICATION_ERR("notification_setting_get_pop_up_notification failed");
		goto out;
	}

	if (ret != true)
		NOTIFICATION_DBG("[%s] is not allowed Pop-up notification", appid);

out:
	if (setting)
		notification_setting_free_notification(setting);

	return ret;
}

static int _check_text_input(notification_h noti)
{
	int err;
	int text_input_max_length;
	app_control_h app_control = NULL;

	err = notification_get_text_input_max_length(noti, &text_input_max_length);
	if (err == NOTIFICATION_ERROR_NONE && text_input_max_length != 0) {
		err = notification_get_event_handler(noti, NOTIFICATION_EVENT_TYPE_CLICK_ON_TEXT_INPUT_BUTTON, &app_control);
		if (err != NOTIFICATION_ERROR_NONE || app_control == NULL) {
			NOTIFICATION_ERR("Event handler for text_input is not set");
			return -1;
		}
	}

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_noti_insert(notification_h noti)
{
	int ret = 0;
	sqlite3 *db = NULL;
	sqlite3_stmt *stmt = NULL;
	char *query = NULL;
	char buf_key[32] = { 0, };
	char *title_key = NULL;

	if (noti == NULL) {
		NOTIFICATION_ERR("NOTIFICATION_ERROR_INVALID_PARAMETER");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	if (_is_allowed_to_notify(noti) == false) {
		NOTIFICATION_DBG("[%s] is not allowed to notify", noti->caller_pkgname);
		return NOTIFICATION_ERROR_PERMISSION_DENIED;
	}

	if (_handle_do_not_disturb_option(noti) != NOTIFICATION_ERROR_NONE)
		NOTIFICATION_WARN("_handle_do_not_disturb_option failed");

	if (_is_pop_up_notification((const char *)noti->caller_pkgname, noti->uid) == false) {
		noti->display_applist = (noti->display_applist & (~NOTIFICATION_DISPLAY_APP_ACTIVE));
		NOTIFICATION_DBG("notification display applist - pkgname [%s], applist [%d]", noti->caller_pkgname, noti->display_applist);
	}

	if (_check_text_input(noti) != NOTIFICATION_ERROR_NONE)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	db = notification_db_open(DBPATH);
	if (!db)
		return get_last_result();

	/* Initialize private ID */
	noti->group_id = NOTIFICATION_GROUP_ID_NONE;
	noti->internal_group_id = NOTIFICATION_GROUP_ID_NONE;

	ret = _insertion_query_create(noti, &query);
	if (ret != NOTIFICATION_ERROR_NONE)
		goto err;

	ret = sqlite3_prepare_v2(db, query, -1, &stmt, NULL);
	if (ret != SQLITE_OK) {
		NOTIFICATION_ERR("Insert Query : %s", query);
		NOTIFICATION_ERR("Insert DB error(%d) : %s", ret,
				 sqlite3_errmsg(db));
		ret = NOTIFICATION_ERROR_FROM_DB;
		goto err;
	}

	/* Get title key */
	if (noti->b_key != NULL) {
		snprintf(buf_key, sizeof(buf_key), "%d",
			 NOTIFICATION_TEXT_TYPE_TITLE);

		bundle_get_str(noti->b_key, buf_key, &title_key);
	}

	if (title_key == NULL && noti->b_text != NULL) {
		snprintf(buf_key, sizeof(buf_key), "%d",
			 NOTIFICATION_TEXT_TYPE_TITLE);

		bundle_get_str(noti->b_text, buf_key, &title_key);
	}

	if (title_key == NULL)
		title_key = noti->caller_pkgname;

	/* Bind query */
	ret = _notification_noti_bind_query_text(stmt, "$tag", noti->tag);
	if (ret != NOTIFICATION_ERROR_NONE) {
		NOTIFICATION_ERR("Bind error : %s", sqlite3_errmsg(db));
		goto err;
	}
	ret = _notification_noti_bind_query_text(stmt, "$title_key", title_key);
	if (ret != NOTIFICATION_ERROR_NONE) {
		NOTIFICATION_ERR("Bind error : %s", sqlite3_errmsg(db));
		goto err;
	}
	ret = _notification_noti_bind_query_double(stmt, "$progress_size", noti->progress_size);
	if (ret != NOTIFICATION_ERROR_NONE) {
		NOTIFICATION_ERR("Bind error : %s", sqlite3_errmsg(db));
		goto err;
	}
	ret = _notification_noti_bind_query_double(stmt, "$progress_percentage", noti->progress_percentage);
	if (ret != NOTIFICATION_ERROR_NONE) {
		NOTIFICATION_ERR("Bind error : %s", sqlite3_errmsg(db));
		goto err;
	}

	ret = sqlite3_step(stmt);
	if (ret == SQLITE_OK || ret == SQLITE_DONE) {
		noti->priv_id = (int)sqlite3_last_insert_rowid(db);

		if (_notification_noti_update_priv_id(db, noti->priv_id) == 0)
			ret = NOTIFICATION_ERROR_NONE;
		else
			ret = NOTIFICATION_ERROR_FROM_DB;

	} else {
		ret = NOTIFICATION_ERROR_FROM_DB;
	}
err:
	if (stmt)
		sqlite3_finalize(stmt);

	if (db)
		notification_db_close(&db);

	if (query)
		sqlite3_free(query);

	return ret;
}

/* LCOV_EXCL_START */
EXPORT_API int notification_noti_get_by_priv_id(notification_h noti, char *pkgname, int priv_id, uid_t uid)
{
	int ret = 0;
	char *query = NULL;
	sqlite3 *db = NULL;
	sqlite3_stmt *stmt = NULL;

	if (priv_id < 0 || noti == NULL) {
		ret = NOTIFICATION_ERROR_INVALID_PARAMETER;
		goto err;
	}

	db = notification_db_open(DBPATH);
	if (!db)
		return get_last_result();

	char *base_query = "select "
			 "type, layout, caller_pkgname, launch_pkgname, image_path, group_id, priv_id, "
			 "tag, b_text, b_key, b_format_args, num_format_args, "
			 "text_domain, text_dir, time, insert_time, args, group_args, "
			 "b_execute_option, b_service_responding, b_service_single_launch, b_service_multi_launch, "
			 "b_event_handler_click_on_button_1, b_event_handler_click_on_button_2, b_event_handler_click_on_button_3, "
			 "b_event_handler_click_on_button_4, b_event_handler_click_on_button_5, b_event_handler_click_on_button_6, "
			 "b_event_handler_click_on_icon, b_event_handler_click_on_thumbnail, b_event_handler_click_on_text_input_button, "
			 "sound_type, sound_path, vibration_type, vibration_path, led_operation, led_argb, led_on_ms, led_off_ms, "
			 "flags_for_property, display_applist, progress_size, progress_percentage, ongoing_flag, ongoing_value_type, "
			 "ongoing_current, ongoing_duration, auto_remove, default_button_index, timeout, text_input_max_length, event_flag, uid "
			 "from noti_list ";

	if (pkgname != NULL && strlen(pkgname) != 0)
		query = sqlite3_mprintf("%s where caller_pkgname = '%s' and priv_id = %d and uid = %d",
				base_query, pkgname, priv_id, uid);
	else
		query = sqlite3_mprintf("%s where priv_id = %d and uid = %d", base_query, priv_id, uid);

	if (query == NULL) {
		ret = NOTIFICATION_ERROR_OUT_OF_MEMORY;
		goto err;
	}

	ret = sqlite3_prepare_v2(db, query, -1, &stmt, NULL);
	if (ret != SQLITE_OK) {
		NOTIFICATION_ERR("select Query : %s", query);
		NOTIFICATION_ERR("select DB error(%d) : %s", ret,
				 sqlite3_errmsg(db));
		ret = NOTIFICATION_ERROR_FROM_DB;
		goto err;
	}

	ret = sqlite3_step(stmt);
	if (ret == SQLITE_ROW) {
		_notification_noti_populate_from_stmt(stmt, noti);
		ret = NOTIFICATION_ERROR_NONE;
	} else {
		ret = NOTIFICATION_ERROR_FROM_DB;
	}

err:
	if (query)
		sqlite3_free(query);

	if (stmt)
		sqlite3_finalize(stmt);

	if (db != NULL)
		notification_db_close(&db);

	return ret;
}
/* LCOV_EXCL_STOP */

EXPORT_API int notification_noti_get_by_tag(notification_h noti, char *pkgname, char *tag, uid_t uid)
{
	int ret = 0;
	sqlite3 *db = NULL;
	sqlite3_stmt *stmt = NULL;

	if (tag == NULL || noti == NULL) {
		NOTIFICATION_ERR("NOTIFICATION_ERROR_INVALID_PARAMETER");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	db = notification_db_open(DBPATH);
	if (!db)
		return get_last_result();

	if (pkgname != NULL && strlen(pkgname) != 0) {
		ret = sqlite3_prepare_v2(db, "select "
			 "type, layout, caller_pkgname, launch_pkgname, image_path, group_id, priv_id, "
			 "tag, b_text, b_key, b_format_args, num_format_args, "
			 "text_domain, text_dir, time, insert_time, args, group_args, "
			 "b_execute_option, b_service_responding, b_service_single_launch, b_service_multi_launch, "
			 "b_event_handler_click_on_button_1, b_event_handler_click_on_button_2, b_event_handler_click_on_button_3, "
			 "b_event_handler_click_on_button_4, b_event_handler_click_on_button_5, b_event_handler_click_on_button_6, "
			 "b_event_handler_click_on_icon, b_event_handler_click_on_thumbnail, b_event_handler_click_on_text_input_button, "
			 "sound_type, sound_path, vibration_type, vibration_path, led_operation, led_argb, led_on_ms, led_off_ms, "
			 "flags_for_property, display_applist, progress_size, progress_percentage, ongoing_flag, ongoing_value_type, "
			 "ongoing_current, ongoing_duration, auto_remove, default_button_index, timeout, text_input_max_length, event_flag "
			 "from noti_list where caller_pkgname = ? and tag = ? and uid = ?", -1, &stmt, NULL);
		if (ret != SQLITE_OK) {
			NOTIFICATION_ERR("Error: %s\n", sqlite3_errmsg(db));
			return NOTIFICATION_ERROR_OUT_OF_MEMORY;
		}

		ret = sqlite3_bind_text(stmt, 1, pkgname, -1, SQLITE_TRANSIENT);
		if (ret != SQLITE_OK) {
			NOTIFICATION_ERR("Error: %s\n", sqlite3_errmsg(db));
			goto err;
		}

		ret = sqlite3_bind_text(stmt, 2,  tag, -1, SQLITE_TRANSIENT);
		if (ret != SQLITE_OK) {
			NOTIFICATION_ERR("Error: %s\n", sqlite3_errmsg(db));
			goto err;
		}

		ret = sqlite3_bind_int(stmt, 3, uid);
		if (ret != SQLITE_OK) {
			NOTIFICATION_ERR("Error: %s\n", sqlite3_errmsg(db));
			goto err;
		}

	} else {
		ret = sqlite3_prepare_v2(db, "select "
			 "type, layout, caller_pkgname, launch_pkgname, image_path, group_id, priv_id, "
			 "tag, b_text, b_key, b_format_args, num_format_args, "
			 "text_domain, text_dir, time, insert_time, args, group_args, "
			 "b_execute_option, b_service_responding, b_service_single_launch, b_service_multi_launch, "
			 "b_event_handler_click_on_button_1, b_event_handler_click_on_button_2, b_event_handler_click_on_button_3, "
			 "b_event_handler_click_on_button_4, b_event_handler_click_on_button_5, b_event_handler_click_on_button_6, "
			 "b_event_handler_click_on_icon, b_event_handler_click_on_thumbnail, b_event_handler_click_on_text_input_button, "
			 "sound_type, sound_path, vibration_type, vibration_path, led_operation, led_argb, led_on_ms, led_off_ms, "
			 "flags_for_property, display_applist, progress_size, progress_percentage, ongoing_flag, ongoing_value_type, "
			 "ongoing_current, ongoing_duration, auto_remove, default_button_index, timeout, text_input_max_length, event_flag, uid "
			 "from noti_list where  tag = ? and uid = ?", -1, &stmt, NULL);
		if (ret != SQLITE_OK) {
			NOTIFICATION_ERR("Error: %s\n", sqlite3_errmsg(db));
			return NOTIFICATION_ERROR_OUT_OF_MEMORY;
		}

		ret = sqlite3_bind_text(stmt, 1, tag, -1, SQLITE_TRANSIENT);
		if (ret != SQLITE_OK) {
			NOTIFICATION_ERR("Error: %s\n", sqlite3_errmsg(db));
			goto err;
		}

		ret = sqlite3_bind_int(stmt, 2, uid);
		if (ret != SQLITE_OK) {
			NOTIFICATION_ERR("Error: %s\n", sqlite3_errmsg(db));
			goto err;
		}
	}

	ret = sqlite3_step(stmt);
	if (ret == SQLITE_ROW) {
		_notification_noti_populate_from_stmt(stmt, noti);
		ret = NOTIFICATION_ERROR_NONE;
	} else {
		ret = NOTIFICATION_ERROR_FROM_DB;
	}

err:

	if (stmt)
		sqlite3_finalize(stmt);

	if (db != NULL)
		notification_db_close(&db);

	return ret;
}

EXPORT_API int notification_noti_update(notification_h noti)
{
	int ret = 0;
	sqlite3 *db;
	sqlite3_stmt *stmt = NULL;
	char *query = NULL;

	if (noti == NULL) {
		NOTIFICATION_ERR("NOTIFICATION_ERROR_INVALID_PARAMETER");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	db = notification_db_open(DBPATH);
	if (!db)
		return get_last_result();

	if (_is_allowed_to_notify(noti) == false) {
		NOTIFICATION_DBG("[%s] is not allowed to notify", noti->caller_pkgname);
		return NOTIFICATION_ERROR_PERMISSION_DENIED;
	}

	if (_handle_do_not_disturb_option(noti) != NOTIFICATION_ERROR_NONE)
		NOTIFICATION_WARN("_handle_do_not_disturb_option failed");

	if (_is_pop_up_notification((const char *)noti->caller_pkgname, noti->uid) == false) {
		noti->display_applist = (noti->display_applist & (~NOTIFICATION_DISPLAY_APP_ACTIVE));
		NOTIFICATION_DBG("notification display applist - pkgname [%s], applist [%d]", noti->caller_pkgname, noti->display_applist);
	}

	/* Check private ID is exist */
	ret = _notification_noti_check_priv_id(noti, db);
	if (ret != NOTIFICATION_ERROR_ALREADY_EXIST_ID) {
		ret = NOTIFICATION_ERROR_NOT_EXIST_ID;
		goto err;
	}

	/* make update query */
	ret = _update_query_create(noti, &query);
	if (ret != NOTIFICATION_ERROR_NONE)
		goto err;

	ret = sqlite3_prepare_v2(db, query, -1, &stmt, NULL);
	if (ret != SQLITE_OK) {
		NOTIFICATION_ERR("Insert Query : %s", query);
		NOTIFICATION_ERR("Insert DB error(%d) : %s", ret,
				 sqlite3_errmsg(db));
		ret = NOTIFICATION_ERROR_FROM_DB;
		goto err;
	}

	ret = _notification_noti_bind_query_text(stmt, "$tag", noti->tag);
	if (ret != NOTIFICATION_ERROR_NONE) {
		NOTIFICATION_ERR("Bind error : %s", sqlite3_errmsg(db));
		goto err;
	}

	ret = _notification_noti_bind_query_double(stmt, "$progress_size", noti->progress_size);
	if (ret != NOTIFICATION_ERROR_NONE) {
		NOTIFICATION_ERR("Bind error : %s", sqlite3_errmsg(db));
		goto err;
	}

	ret = _notification_noti_bind_query_double(stmt, "$progress_percentage", noti->progress_percentage);
	if (ret != NOTIFICATION_ERROR_NONE) {
		NOTIFICATION_ERR("Bind error : %s", sqlite3_errmsg(db));
		goto err;
	}

	ret = sqlite3_step(stmt);
	if (ret == SQLITE_OK || ret == SQLITE_DONE)
		ret = NOTIFICATION_ERROR_NONE;
	else
		ret = NOTIFICATION_ERROR_FROM_DB;

err:
	if (stmt)
		sqlite3_finalize(stmt);

	if (db)
		notification_db_close(&db);

	if (query)
		sqlite3_free(query);

	return ret;
}

EXPORT_API int notification_noti_delete_all(notification_type_e type, const char *pkgname, int *num_deleted, int **list_deleted_rowid, uid_t uid)
{
	int ret = NOTIFICATION_ERROR_NONE;
	int ret_tmp = NOTIFICATION_ERROR_NONE;
	int i = 0;
	int data_cnt = 0;
	sqlite3 *db = NULL;
	sqlite3_stmt *stmt = NULL;
	char buf[128] = { 0, };
	char query[NOTIFICATION_QUERY_MAX] = { 0, };
	char query_base[NOTIFICATION_QUERY_MAX] = { 0, };
	char query_where[NOTIFICATION_QUERY_MAX] = { 0, };
	char err_buf[ERR_BUFFER_SIZE];

	db = notification_db_open(DBPATH);
	if (!db)
		return get_last_result();

	if (pkgname == NULL || strlen(pkgname) == 0) {
		if (type != NOTIFICATION_TYPE_NONE)
			snprintf(query_where, sizeof(query_where),
				 "where type = %d and uid = %d", type, uid);
	} else {
		if (type == NOTIFICATION_TYPE_NONE)
			snprintf(query_where, sizeof(query_where),
				 "where caller_pkgname = '%s' and uid = %d", pkgname, uid);
		else
			snprintf(query_where, sizeof(query_where),
				 "where caller_pkgname = '%s' and type = %d and uid = %d",
				 pkgname, type, uid);
	}

	if (num_deleted != NULL)
		*num_deleted = 0;

	if (list_deleted_rowid != NULL) {
		*list_deleted_rowid = NULL;
		snprintf(query, sizeof(query),
				"select priv_id from noti_list %s ", query_where);

		ret = sqlite3_prepare(db, query, strlen(query), &stmt, NULL);
		if (ret != SQLITE_OK) {
			NOTIFICATION_ERR("Select Query : %s", query);
			NOTIFICATION_ERR("Select DB error(%d) : %s", ret,
					 sqlite3_errmsg(db));

			ret = NOTIFICATION_ERROR_FROM_DB;
			goto err;
		}

		while (sqlite3_step(stmt) == SQLITE_ROW) {
			if (data_cnt % 8 == 0) {
				int *tmp;

				tmp = (int *)realloc(*list_deleted_rowid, sizeof(int) * (data_cnt + 8 + 1));
				if (tmp) {
					*list_deleted_rowid = tmp;
				} else {
					NOTIFICATION_ERR("Heap: %s\n", strerror_r(errno, err_buf, sizeof(err_buf)));
					/*!
					 * \TODO
					 * How can I handle this?
					 */
					free(*list_deleted_rowid);
					*list_deleted_rowid = NULL;
					ret = NOTIFICATION_ERROR_OUT_OF_MEMORY;
					goto err;
				}
			}
			*((*list_deleted_rowid) + data_cnt) = sqlite3_column_int(stmt, 0);
			data_cnt++;
		}

		if (stmt) {
			sqlite3_finalize(stmt);
			stmt = NULL;
		}

		if (data_cnt > 0) {
			query_where[0] = '\0';
			snprintf(query_base, sizeof(query_base) - 1, "delete from noti_list");
			for (i = 0; i < data_cnt ; i++) {
				if (i % NOTI_BURST_DELETE_UNIT == 0 && i != 0) {
					snprintf(query, sizeof(query) - 1, "%s where priv_id in (%s)", query_base, query_where);
					ret_tmp = notification_db_exec(db, query, NULL);
					query_where[0] = '\0';
					if (ret == NOTIFICATION_ERROR_NONE)
						ret = ret_tmp;
				}
				snprintf(buf, sizeof(buf) - 1, "%s%d", (i % NOTI_BURST_DELETE_UNIT == 0) ? "" : ",", *((*list_deleted_rowid) + i));
				strncat(query_where, buf, sizeof(query_where) - strlen(query_where) - 1);
			}
			if ((i <= NOTI_BURST_DELETE_UNIT) || ((i % NOTI_BURST_DELETE_UNIT) > 0)) {
				snprintf(query, sizeof(query) - 1, "%s where priv_id in (%s)", query_base, query_where);
				ret_tmp = notification_db_exec(db, query, NULL);
				if (ret == NOTIFICATION_ERROR_NONE)
					ret = ret_tmp;
			}
		} else {
			free(*list_deleted_rowid);
			*list_deleted_rowid = NULL;
		}

		if (num_deleted != NULL)
			*num_deleted = data_cnt;
	} else {
		/* Make main query */
		snprintf(query_base, sizeof(query_base), "delete from noti_list ");
		snprintf(query, sizeof(query), "%s %s", query_base, query_where);

		ret = notification_db_exec(db, query, NULL);

		if (num_deleted != NULL)
			*num_deleted = sqlite3_changes(db);
	}

err:
	if (stmt)
		sqlite3_finalize(stmt);

	if (db)
		notification_db_close(&db);

	return ret;
}

/* LCOV_EXCL_START */
EXPORT_API int notification_noti_delete_by_priv_id(const char *pkgname, int priv_id)
{
	sqlite3 *db = NULL;
	char query[NOTIFICATION_QUERY_MAX] = { 0, };
	int ret;

	if (pkgname == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	db = notification_db_open(DBPATH);
	if (!db)
		return get_last_result();

	snprintf(query, sizeof(query), "delete from noti_list "
		 "where caller_pkgname = '%s' and priv_id = %d", pkgname,
		 priv_id);

	ret = notification_db_exec(db, query, NULL);

	if (db)
		notification_db_close(&db);

	return ret;
}
/* LCOV_EXCL_STOP */

EXPORT_API int notification_noti_delete_by_priv_id_get_changes(const char *pkgname, int priv_id, int *num_changes, uid_t uid)
{
	sqlite3 *db = NULL;
	char query[NOTIFICATION_QUERY_MAX] = { 0, };
	int ret;

	db = notification_db_open(DBPATH);
	if (!db)
		return get_last_result();

	if (pkgname == NULL || strlen(pkgname) == 0) {
		snprintf(query, sizeof(query), "delete from noti_list "
			"where priv_id = %d and uid = %d", priv_id, uid);
	} else {
		snprintf(query, sizeof(query), "delete from noti_list "
			"where caller_pkgname = '%s' and priv_id = %d and uid = %d",
			pkgname, priv_id, uid);
	}

	ret = notification_db_exec(db, query, num_changes);

	if (num_changes != NULL)
		NOTIFICATION_DBG("deleted num:%d", *num_changes);

	if (db)
		notification_db_close(&db);

	return ret;
}

/* todo refactoring */
/* LCOV_EXCL_START */
EXPORT_API int notification_noti_get_count(notification_type_e type,
						 const char *pkgname,
						 int group_id, int priv_id,
						 int *count, uid_t uid)
{
	sqlite3 *db = NULL;
	sqlite3_stmt *stmt = NULL;
	char query[NOTIFICATION_QUERY_MAX] = { 0, };
	char query_base[NOTIFICATION_QUERY_MAX] = { 0, };
	char query_where[NOTIFICATION_QUERY_MAX] = { 0, };
	char query_where_more[NOTIFICATION_QUERY_MAX] = { 0, };

	int ret = 0, get_count = 0, internal_group_id = 0;
	int status = VCONFKEY_TELEPHONY_SIM_UNKNOWN;
	int flag_where_more = 0;
	int ret_vconf = 0;

	db = notification_db_open(DBPATH);
	if (!db)
		return get_last_result();

	/* Check current sim status */
	ret_vconf = vconf_get_int(VCONFKEY_TELEPHONY_SIM_SLOT, &status);

	snprintf(query_base, sizeof(query_base),
		 "select count(*) from noti_list ");

	if (pkgname != NULL && strlen(pkgname) != 0) {
		if (group_id == NOTIFICATION_GROUP_ID_NONE) {
			if (priv_id == NOTIFICATION_PRIV_ID_NONE) {
				snprintf(query_where, sizeof(query_where),
					 "where caller_pkgname = '%s' and uid = %d ",
					 pkgname, uid);
			} else {
				internal_group_id =
				    _notification_noti_get_internal_group_id_by_priv_id
				    (pkgname, priv_id, db);
				snprintf(query_where, sizeof(query_where),
					 "where caller_pkgname = '%s' and internal_group_id = %d and uid = %d ",
					 pkgname, internal_group_id, uid);
			}
		} else {
			if (priv_id == NOTIFICATION_PRIV_ID_NONE) {
				snprintf(query_where, sizeof(query_where),
					 "where caller_pkgname = '%s' and group_id = %d and uid = %d ",
					 pkgname, group_id, uid);
			} else {
				internal_group_id =
				    _notification_noti_get_internal_group_id_by_priv_id
				    (pkgname, priv_id, db);
				snprintf(query_where, sizeof(query_where),
					 "where caller_pkgname = '%s' and internal_group_id = %d and uid = %d ",
					 pkgname, internal_group_id, uid);
			}
		}
	} else {
		snprintf(query_where, sizeof(query_where), "where uid = %d", uid);
	}

	if (ret_vconf == 0 && status == VCONFKEY_TELEPHONY_SIM_INSERTED) {
		if (type != NOTIFICATION_TYPE_NONE) {
			snprintf(query_where_more, sizeof(query_where_more),
				 "type = %d ", type);
			flag_where_more = 1;
		}
	} else {
		if (type != NOTIFICATION_TYPE_NONE) {
			snprintf(query_where_more, sizeof(query_where_more),
				 "type = %d and flag_simmode = 0 ", type);
			flag_where_more = 1;
		} else {
			snprintf(query_where_more, sizeof(query_where_more),
				 "flag_simmode = 0 ");
			flag_where_more = 1;
		}
	}

	if (flag_where_more == 1) {
		snprintf(query, sizeof(query), "%s %s and %s",
			 query_base, query_where, query_where_more);
	} else {
		snprintf(query, sizeof(query), "%s %s", query_base,
			 query_where);
	}

	ret = sqlite3_prepare(db, query, strlen(query), &stmt, NULL);
	if (ret != SQLITE_OK) {
		NOTIFICATION_ERR("Select Query : %s", query);
		NOTIFICATION_ERR("Select DB error(%d) : %s", ret,
				 sqlite3_errmsg(db));

		ret = NOTIFICATION_ERROR_FROM_DB;
		goto err;
	}

	ret = sqlite3_step(stmt);
	if (ret == SQLITE_ROW)
		get_count = sqlite3_column_int(stmt, 0);

	ret = NOTIFICATION_ERROR_NONE;

err:
	if (stmt)
		sqlite3_finalize(stmt);

	if (db)
		notification_db_close(&db);

	*count = get_count;

	return ret;
}
/* LCOV_EXCL_STOP */

EXPORT_API int notification_noti_get_grouping_list(notification_type_e type,
							 int count,
							 notification_list_h *list,
							 uid_t uid)
{
	sqlite3 *db = NULL;
	sqlite3_stmt *stmt = NULL;
	char query[NOTIFICATION_QUERY_MAX] = { 0, };
	char query_base[NOTIFICATION_QUERY_MAX] = { 0, };
	char query_uid[NOTIFICATION_QUERY_MAX] = { 0, };
	char query_where[NOTIFICATION_QUERY_MAX] = { 0, };

	int ret = 0;
	notification_list_h get_list = NULL;
	notification_h noti = NULL;
	int internal_count = 0;
	int status;

	db = notification_db_open(DBPATH);
	if (!db)
		return get_last_result();

	/* Check current sim status */
	ret = vconf_get_int(VCONFKEY_TELEPHONY_SIM_SLOT, &status);

	snprintf(query_base, sizeof(query_base), "select "
		 "type, layout, caller_pkgname, launch_pkgname, image_path, group_id, priv_id, "
		 "tag, b_text, b_key, b_format_args, num_format_args, "
		 "text_domain, text_dir, time, insert_time, args, group_args, "
		 "b_execute_option, b_service_responding, b_service_single_launch, b_service_multi_launch, "
		 "b_event_handler_click_on_button_1, b_event_handler_click_on_button_2, b_event_handler_click_on_button_3, "
		 "b_event_handler_click_on_button_4, b_event_handler_click_on_button_5, b_event_handler_click_on_button_6, "
		 "b_event_handler_click_on_icon, b_event_handler_click_on_thumbnail, b_event_handler_click_on_text_input_button, "
		 "sound_type, sound_path, vibration_type, vibration_path, led_operation, led_argb, led_on_ms, led_off_ms, "
		 "flags_for_property, display_applist, progress_size, progress_percentage, ongoing_flag, ongoing_value_type, "
		 "ongoing_current, ongoing_duration, auto_remove, default_button_index, timeout, text_input_max_length, event_flag, uid "
		 "from noti_list where 1 > 0 ");

	if (status == VCONFKEY_TELEPHONY_SIM_INSERTED) {
		if (type != NOTIFICATION_TYPE_NONE)
			snprintf(query_where, sizeof(query_where),
				 " and type = %d ", type);
	} else {
		if (type != NOTIFICATION_TYPE_NONE)
			snprintf(query_where, sizeof(query_where),
				 " and type = %d and flag_simmode = 0 ", type);
		else
			snprintf(query_where, sizeof(query_where),
				 " and flag_simmode = 0 ");
	}

	if (uid != NOTIFICATION_GLOBAL_UID)
		snprintf(query_uid, sizeof(query_uid), " and uid = %d ", uid);

	snprintf(query, sizeof(query),
		 "%s %s %s "
		 "group by internal_group_id "
		 "order by rowid desc, time desc", query_base, query_where, query_uid);

	ret = sqlite3_prepare(db, query, strlen(query), &stmt, NULL);
	if (ret != SQLITE_OK) {
		NOTIFICATION_ERR("Select Query : %s", query);
		NOTIFICATION_ERR("Select DB error(%d) : %s", ret,
				 sqlite3_errmsg(db));

		ret = NOTIFICATION_ERROR_FROM_DB;
		goto err;
	}

	while (sqlite3_step(stmt) == SQLITE_ROW) {
		/* Make notification list */
		noti = _notification_noti_get_item(stmt);
		if (noti != NULL) {
			internal_count++;

			get_list = notification_list_append(get_list, noti);

			if (count != -1 && internal_count >= count) {
				NOTIFICATION_INFO
				    ("internal count %d >= count %d",
				     internal_count, count);
				break;
			}
		}
	}

	ret = NOTIFICATION_ERROR_NONE;

err:
	if (stmt)
		sqlite3_finalize(stmt);

	if (db)
		notification_db_close(&db);

	if (get_list != NULL)
		*list = notification_list_get_head(get_list);

	return ret;
}

EXPORT_API int notification_noti_get_detail_list(const char *pkgname,
						       int group_id,
						       int priv_id, int count,
						       notification_list_h *list,
						       uid_t uid)
{
	sqlite3 *db = NULL;
	sqlite3_stmt *stmt = NULL;
	char query_base[NOTIFICATION_QUERY_MAX] = { 0, };
	char query_where[NOTIFICATION_QUERY_MAX] = { 0, };

	char query[NOTIFICATION_QUERY_MAX] = { 0, };
	int ret = 0;
	notification_list_h get_list = NULL;
	notification_h noti = NULL;
	int internal_count = 0;
	int internal_group_id = 0;
	int status = 0; /* If the vconf_get_int failed, the status will be the garbage value */

	db = notification_db_open(DBPATH);
	if (!db)
		return get_last_result();

	/* Check current sim status */
	ret = vconf_get_int(VCONFKEY_TELEPHONY_SIM_SLOT, &status);

	snprintf(query_base, sizeof(query_base), "select "
		 "type, layout, caller_pkgname, launch_pkgname, image_path, group_id, priv_id, "
		 "tag, b_text, b_key, b_format_args, num_format_args, "
		 "text_domain, text_dir, time, insert_time, args, group_args, "
		 "b_execute_option, b_service_responding, b_service_single_launch, b_service_multi_launch, "
		 "b_event_handler_click_on_button_1, b_event_handler_click_on_button_2, b_event_handler_click_on_button_3, "
		 "b_event_handler_click_on_button_4, b_event_handler_click_on_button_5, b_event_handler_click_on_button_6, "
		 "b_event_handler_click_on_icon, b_event_handler_click_on_thumbnail, b_event_handler_click_on_text_input_button, "
		 "sound_type, sound_path, vibration_type, vibration_path, led_operation, led_argb, led_on_ms, led_off_ms, "
		 "flags_for_property, display_applist, progress_size, progress_percentage, ongoing_flag, ongoing_value_type, "
		 "ongoing_current, ongoing_duration, auto_remove, default_button_index, timeout, text_input_max_length, event_flag, uid "
		 "from noti_list ");

	if (priv_id == NOTIFICATION_PRIV_ID_NONE && group_id == NOTIFICATION_GROUP_ID_NONE) {
		if (status == VCONFKEY_TELEPHONY_SIM_INSERTED)
			snprintf(query_where, sizeof(query_where),
				 "where  caller_pkgname = '%s' and uid = %d ", pkgname, uid);
		else
			snprintf(query_where, sizeof(query_where),
				 "where  caller_pkgname = '%s' and flag_simmode = 0 and uid = %d ", pkgname, uid);

	} else {
		internal_group_id =
		    _notification_noti_get_internal_group_id_by_priv_id(pkgname,
									priv_id, db);

		if (status == VCONFKEY_TELEPHONY_SIM_INSERTED)
			snprintf(query_where, sizeof(query_where),
				 "where  caller_pkgname = '%s' and internal_group_id = %d and uid = %d ",
				 pkgname, internal_group_id, uid);
		else
			snprintf(query_where, sizeof(query_where),
				 "where  caller_pkgname = '%s' and internal_group_id = %d and flag_simmode = 0 and uid = %d ",
				 pkgname, internal_group_id, uid);
	}

	snprintf(query, sizeof(query),
		 "%s %s "
		 "order by rowid desc, time desc", query_base, query_where);

	ret = sqlite3_prepare(db, query, strlen(query), &stmt, NULL);
	if (ret != SQLITE_OK) {
		NOTIFICATION_ERR("Select Query : %s", query);
		NOTIFICATION_ERR("Select DB error(%d) : %s", ret,
				 sqlite3_errmsg(db));

		ret = NOTIFICATION_ERROR_FROM_DB;
		goto err;
	}

	while (sqlite3_step(stmt) == SQLITE_ROW) {
		/* Make notification list */
		noti = _notification_noti_get_item(stmt);
		if (noti != NULL) {
			internal_count++;

			get_list = notification_list_append(get_list, noti);

			if (count != -1 && internal_count >= count) {
				NOTIFICATION_INFO
				    ("internal count %d >= count %d",
				     internal_count, count);
				break;
			}
		}
	}

	ret = NOTIFICATION_ERROR_NONE;

err:
	if (stmt)
		sqlite3_finalize(stmt);

	if (db)
		notification_db_close(&db);

	if (get_list != NULL)
		*list = notification_list_get_head(get_list);

	return ret;
}

EXPORT_API int notification_noti_check_tag(notification_h noti)
{
	int result = 0;
	int ret = NOTIFICATION_ERROR_NONE;
	sqlite3 *db;
	sqlite3_stmt *stmt = NULL;

	if (noti->tag == NULL)
		return NOTIFICATION_ERROR_NOT_EXIST_ID;

	db = notification_db_open(DBPATH);
	if (!db)
		return get_last_result();

	ret = sqlite3_prepare_v2(db, "SELECT priv_id FROM noti_list WHERE caller_pkgname = ? and tag = ?", -1, &stmt, NULL);
	if (ret != SQLITE_OK) {
		NOTIFICATION_ERR("Error: %s\n", sqlite3_errmsg(db));
		if (db)
			notification_db_close(&db);
		return NOTIFICATION_ERROR_OUT_OF_MEMORY;
	}

	ret = sqlite3_bind_text(stmt, 1, noti->caller_pkgname, -1, SQLITE_TRANSIENT);
	if (ret != SQLITE_OK) {
		NOTIFICATION_ERR("Error: %s\n", sqlite3_errmsg(db));
		goto err;
	}

	ret = sqlite3_bind_text(stmt, 2, noti->tag, -1, SQLITE_TRANSIENT);
	if (ret != SQLITE_OK) {
		NOTIFICATION_ERR("Error: %s\n", sqlite3_errmsg(db));
		goto err;
	}

	ret = sqlite3_step(stmt);
	if (ret == SQLITE_ROW)
		result = sqlite3_column_int(stmt, 0);
	else
		result = 0;

	sqlite3_finalize(stmt);

	/* If result > 0, there is priv_id in DB */
	if (result > 0) {
		noti->priv_id = result;
		ret = NOTIFICATION_ERROR_ALREADY_EXIST_ID;
	} else {
		ret = NOTIFICATION_ERROR_NOT_EXIST_ID;
	}

err:
	if (db)
		notification_db_close(&db);

	return ret;
}

EXPORT_API int notification_noti_check_count_for_template(notification_h noti, int *count)
{
	int result = 0;
	int ret = NOTIFICATION_ERROR_NONE;
	sqlite3 *db;
	sqlite3_stmt *stmt = NULL;

	if (noti == NULL || count == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	db = notification_db_open(DBPATH);
	if (!db)
		return get_last_result();

	ret = sqlite3_prepare_v2(db, "SELECT COUNT(caller_pkgname) FROM noti_template WHERE caller_pkgname = ?", -1, &stmt, NULL);
	if (ret != SQLITE_OK) {
		NOTIFICATION_ERR("Error: %s\n", sqlite3_errmsg(db));
		if (db)
			notification_db_close(&db);
		return NOTIFICATION_ERROR_OUT_OF_MEMORY;
	}

	ret = sqlite3_bind_text(stmt, 1, noti->caller_pkgname, -1, SQLITE_TRANSIENT);
	if (ret != SQLITE_OK) {
		NOTIFICATION_ERR("Error: %s\n", sqlite3_errmsg(db));
		goto err;
	}

	ret = sqlite3_step(stmt);
	if (ret == SQLITE_ROW)
		result = sqlite3_column_int(stmt, 0);
	else
		result = 0;

	*count = result;

	sqlite3_finalize(stmt);
err:
	if (db)
		notification_db_close(&db);

	return ret;
}

static int _template_query_create(notification_h noti, char *template_name, char **query)
{
	int i = 0;
	int b_encode_len = 0;
	char *args = NULL;
	char *group_args = NULL;
	char *b_image_path = NULL;
	char *b_execute_option = NULL;
	char *b_service_responding = NULL;
	char *b_service_single_launch = NULL;
	char *b_service_multi_launch = NULL;
	char *b_event_handler[NOTIFICATION_EVENT_TYPE_MAX] = { NULL , };
	char *b_text = NULL;
	char *b_key = NULL;
	char *b_format_args = NULL;
	int flag_simmode = 0;

	if (query == NULL || template_name == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	/* Decode bundle to insert DB */
	if (noti->args)
		bundle_encode(noti->args, (bundle_raw **)&args, &b_encode_len);

	if (noti->group_args)
		bundle_encode(noti->group_args, (bundle_raw **)&group_args,
			      &b_encode_len);

	if (noti->b_execute_option)
		bundle_encode(noti->b_execute_option,
			      (bundle_raw **)&b_execute_option, &b_encode_len);

	if (noti->b_service_responding)
		bundle_encode(noti->b_service_responding,
			      (bundle_raw **)&b_service_responding, &b_encode_len);

	if (noti->b_service_single_launch)
		bundle_encode(noti->b_service_single_launch,
			      (bundle_raw **)&b_service_single_launch, &b_encode_len);

	if (noti->b_service_multi_launch)
		bundle_encode(noti->b_service_multi_launch,
			      (bundle_raw **)&b_service_multi_launch, &b_encode_len);


	for (i = 0; i < NOTIFICATION_EVENT_TYPE_MAX; i++) {
		if (noti->b_event_handler[i])
			bundle_encode(noti->b_event_handler[i],
					(bundle_raw **)&b_event_handler[i], &b_encode_len);
	}

	if (noti->b_text)
		bundle_encode(noti->b_text, (bundle_raw **)&b_text, &b_encode_len);

	if (noti->b_key)
		bundle_encode(noti->b_key, (bundle_raw **)&b_key, &b_encode_len);

	if (noti->b_format_args)
		bundle_encode(noti->b_format_args,
			      (bundle_raw **)&b_format_args, &b_encode_len);

	if (noti->b_image_path)
		bundle_encode(noti->b_image_path,
			      (bundle_raw **)&b_image_path, &b_encode_len);

	/* Check only simmode property is enable */
	if (noti->flags_for_property & NOTIFICATION_PROP_DISPLAY_ONLY_SIMMODE)
		flag_simmode = 1;

	*query = sqlite3_mprintf("INSERT OR REPLACE INTO noti_template ("
		"type, "
		"layout, "
		"caller_pkgname, launch_pkgname, "
		"image_path, "
		"group_id, internal_group_id,"
		"title_key, "
		"b_text, b_key, tag, b_format_args, num_format_args, "
		"text_domain, text_dir, "
		"time, insert_time, "
		"args, group_args, "
		"b_execute_option, "
		"b_service_responding, b_service_single_launch, b_service_multi_launch, "
		"b_event_handler_click_on_button_1, b_event_handler_click_on_button_2, b_event_handler_click_on_button_3, "
		"b_event_handler_click_on_button_4, b_event_handler_click_on_button_5, b_event_handler_click_on_button_6, "
		"b_event_handler_click_on_icon, b_event_handler_click_on_thumbnail, b_event_handler_click_on_text_input_button, "
		"sound_type, sound_path, vibration_type, vibration_path, led_operation, led_argb, led_on_ms, led_off_ms, "
		"flags_for_property, flag_simmode, display_applist, progress_size, progress_percentage, "
		"ongoing_flag, ongoing_value_type, ongoing_current, ongoing_duration, "
		"auto_remove, default_button_index, timeout, text_input_max_length, "
		"event_flag, uid, template_name) values ("
		"%d, "
		"%d, "
		"'%s', '%s', "
		"'%s', "
		"%d, %d,"
		"$title_key, "
		"'%s', '%s', $tag, '%s', %d, "
		"'%s', '%s', "
		"%d, %d, "
		"'%s', '%s', "
		"'%s', "
		"'%s', '%s', '%s', "
		"'%s', '%s', '%s', "
		"'%s', '%s', '%s', "
		"'%s', '%s', '%s', "
		"%d, '%s', %d, '%s', %d, %d, %d, %d, "
		"%d, %d, %d, $progress_size, $progress_percentage, "
		"%d, %d, %d, %d, "
		"%d, %d, %d, %d, "
		"%d, %d, '%s')",
		noti->type,
		noti->layout,
		NOTIFICATION_CHECK_STR(noti->caller_pkgname),
		NOTIFICATION_CHECK_STR(noti->launch_pkgname),
		NOTIFICATION_CHECK_STR(b_image_path), noti->group_id,
		noti->internal_group_id,
		NOTIFICATION_CHECK_STR(b_text), NOTIFICATION_CHECK_STR(b_key),
		NOTIFICATION_CHECK_STR(b_format_args), noti->num_format_args,
		NOTIFICATION_CHECK_STR(noti->domain),
		NOTIFICATION_CHECK_STR(noti->dir), (int)noti->time,
		(int)noti->insert_time, NOTIFICATION_CHECK_STR(args),
		NOTIFICATION_CHECK_STR(group_args),
		NOTIFICATION_CHECK_STR(b_execute_option),
		NOTIFICATION_CHECK_STR(b_service_responding),
		NOTIFICATION_CHECK_STR(b_service_single_launch),
		NOTIFICATION_CHECK_STR(b_service_multi_launch),
		NOTIFICATION_CHECK_STR(b_event_handler[NOTIFICATION_EVENT_TYPE_CLICK_ON_BUTTON_1]),
		NOTIFICATION_CHECK_STR(b_event_handler[NOTIFICATION_EVENT_TYPE_CLICK_ON_BUTTON_2]),
		NOTIFICATION_CHECK_STR(b_event_handler[NOTIFICATION_EVENT_TYPE_CLICK_ON_BUTTON_3]),
		NOTIFICATION_CHECK_STR(b_event_handler[NOTIFICATION_EVENT_TYPE_CLICK_ON_BUTTON_4]),
		NOTIFICATION_CHECK_STR(b_event_handler[NOTIFICATION_EVENT_TYPE_CLICK_ON_BUTTON_5]),
		NOTIFICATION_CHECK_STR(b_event_handler[NOTIFICATION_EVENT_TYPE_CLICK_ON_BUTTON_6]),
		NOTIFICATION_CHECK_STR(b_event_handler[NOTIFICATION_EVENT_TYPE_CLICK_ON_ICON]),
		NOTIFICATION_CHECK_STR(b_event_handler[NOTIFICATION_EVENT_TYPE_CLICK_ON_THUMBNAIL]),
		NOTIFICATION_CHECK_STR(b_event_handler[NOTIFICATION_EVENT_TYPE_CLICK_ON_TEXT_INPUT_BUTTON]),
		noti->sound_type, NOTIFICATION_CHECK_STR(noti->sound_path),
		noti->vibration_type,
		NOTIFICATION_CHECK_STR(noti->vibration_path),
		noti->led_operation,
		noti->led_argb,
		noti->led_on_ms,
		noti->led_off_ms,
		noti->flags_for_property, flag_simmode, noti->display_applist,
		noti->ongoing_flag,
		noti->ongoing_value_type,
		noti->ongoing_current,
		noti->ongoing_duration,
		noti->auto_remove,
		noti->default_button_index,
		noti->timeout,
		noti->text_input_max_length,
		noti->event_flag,
		noti->uid,
		template_name);

	/* Free decoded data */
	if (args)
		free(args);

	if (group_args)
		free(group_args);

	if (b_execute_option)
		free(b_execute_option);

	if (b_service_responding)
		free(b_service_responding);

	if (b_service_single_launch)
		free(b_service_single_launch);

	if (b_service_multi_launch)
		free(b_service_multi_launch);

	if (b_text)
		free(b_text);

	if (b_key)
		free(b_key);

	if (b_format_args)
		free(b_format_args);

	if (b_image_path)
		free(b_image_path);

	if (*query == NULL)
		return NOTIFICATION_ERROR_OUT_OF_MEMORY;

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_noti_add_template(notification_h noti, char *template_name)
{
	int ret = 0;
	sqlite3 *db = NULL;
	sqlite3_stmt *stmt = NULL;
	char *query = NULL;
	char buf_key[32] = { 0, };
	char *title_key = NULL;

	if (noti == NULL || template_name == NULL) {
		NOTIFICATION_ERR("NOTIFICATION_ERROR_INVALID_PARAMETER");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	db = notification_db_open(DBPATH);
	if (!db)
		return get_last_result();

	/* Initialize private ID */
	noti->group_id = NOTIFICATION_GROUP_ID_NONE;
	noti->internal_group_id = NOTIFICATION_GROUP_ID_NONE;

	ret = _template_query_create(noti, template_name, &query);
	if (ret != NOTIFICATION_ERROR_NONE)
		goto err;

	ret = sqlite3_prepare_v2(db, query, -1, &stmt, NULL);
	if (ret != SQLITE_OK) {
		NOTIFICATION_ERR("Insert Query : %s", query);
		NOTIFICATION_ERR("Insert DB error(%d) : %s", ret,
				 sqlite3_errmsg(db));
		ret = NOTIFICATION_ERROR_FROM_DB;
		goto err;
	}

	/* Get title key */
	if (noti->b_key != NULL) {
		snprintf(buf_key, sizeof(buf_key), "%d",
			 NOTIFICATION_TEXT_TYPE_TITLE);

		bundle_get_str(noti->b_key, buf_key, &title_key);
	}

	if (title_key == NULL && noti->b_text != NULL) {
		snprintf(buf_key, sizeof(buf_key), "%d",
			 NOTIFICATION_TEXT_TYPE_TITLE);

		bundle_get_str(noti->b_text, buf_key, &title_key);
	}

	if (title_key == NULL)
		title_key = noti->caller_pkgname;

	/* Bind query */
	ret = _notification_noti_bind_query_text(stmt, "$tag", noti->tag);
	if (ret != NOTIFICATION_ERROR_NONE) {
		NOTIFICATION_ERR("Bind error : %s", sqlite3_errmsg(db));
		goto err;
	}
	ret = _notification_noti_bind_query_text(stmt, "$title_key", title_key);
	if (ret != NOTIFICATION_ERROR_NONE) {
		NOTIFICATION_ERR("Bind error : %s", sqlite3_errmsg(db));
		goto err;
	}
	ret = _notification_noti_bind_query_double(stmt, "$progress_size", noti->progress_size);
	if (ret != NOTIFICATION_ERROR_NONE) {
		NOTIFICATION_ERR("Bind error : %s", sqlite3_errmsg(db));
		goto err;
	}
	ret = _notification_noti_bind_query_double(stmt, "$progress_percentage", noti->progress_percentage);
	if (ret != NOTIFICATION_ERROR_NONE) {
		NOTIFICATION_ERR("Bind error : %s", sqlite3_errmsg(db));
		goto err;
	}

	ret = sqlite3_step(stmt);
	if (ret == SQLITE_OK || ret == SQLITE_DONE) {
		noti->priv_id = (int)sqlite3_last_insert_rowid(db);

		if (_notification_noti_update_priv_id(db, noti->priv_id) == 0)
			ret = NOTIFICATION_ERROR_NONE;
		else
			ret = NOTIFICATION_ERROR_FROM_DB;

	} else {
		ret = NOTIFICATION_ERROR_FROM_DB;
	}
err:
	if (stmt)
		sqlite3_finalize(stmt);

	if (db)
		notification_db_close(&db);

	if (query)
		sqlite3_free(query);

	return ret;
}

EXPORT_API int notification_noti_get_package_template(notification_h noti, char *pkgname, char *template_name)
{
	int ret = 0;
	sqlite3 *db = NULL;
	sqlite3_stmt *stmt = NULL;

	if (noti == NULL || pkgname == NULL || template_name == NULL) {
		NOTIFICATION_ERR("NOTIFICATION_ERROR_INVALID_PARAMETER");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	db = notification_db_open(DBPATH);
	if (!db)
		return get_last_result();

	ret = sqlite3_prepare_v2(db, "select "
		"type, layout, caller_pkgname, launch_pkgname, image_path, group_id, priv_id, "
		"tag, b_text, b_key, b_format_args, num_format_args, "
		"text_domain, text_dir, time, insert_time, args, group_args, "
		"b_execute_option, b_service_responding, b_service_single_launch, b_service_multi_launch, "
		"b_event_handler_click_on_button_1, b_event_handler_click_on_button_2, b_event_handler_click_on_button_3, "
		"b_event_handler_click_on_button_4, b_event_handler_click_on_button_5, b_event_handler_click_on_button_6, "
		"b_event_handler_click_on_icon, b_event_handler_click_on_thumbnail, b_event_handler_click_on_text_input_button, "
		"sound_type, sound_path, vibration_type, vibration_path, led_operation, led_argb, led_on_ms, led_off_ms, "
		"flags_for_property, display_applist, progress_size, progress_percentage, ongoing_flag, ongoing_value_type, "
		"ongoing_current, ongoing_duration, auto_remove, default_button_index, timeout, text_input_max_length, event_flag, uid "
		"from noti_template where caller_pkgname = ? and template_name = ?", -1, &stmt, NULL);
	if (ret != SQLITE_OK) {
		NOTIFICATION_ERR("Error: %s\n", sqlite3_errmsg(db));
		return NOTIFICATION_ERROR_OUT_OF_MEMORY;
	}

	ret = sqlite3_bind_text(stmt, 1,  pkgname, -1, SQLITE_TRANSIENT);
	if (ret != SQLITE_OK) {
		NOTIFICATION_ERR("Error: %s\n", sqlite3_errmsg(db));
		goto err;
	}

	ret = sqlite3_bind_text(stmt, 2,  template_name, -1, SQLITE_TRANSIENT);
	if (ret != SQLITE_OK) {
		NOTIFICATION_ERR("Error: %s\n", sqlite3_errmsg(db));
		goto err;
	}

	ret = sqlite3_step(stmt);
	if (ret == SQLITE_ROW) {
		_notification_noti_populate_from_stmt(stmt, noti);
		ret = NOTIFICATION_ERROR_NONE;
	} else {
		NOTIFICATION_ERR("NOTIFICATION_ERROR_FROM_DB");
		ret = NOTIFICATION_ERROR_FROM_DB;
	}

err:

	if (stmt)
		sqlite3_finalize(stmt);

	if (db != NULL)
		notification_db_close(&db);

	return ret;
}

EXPORT_API int notification_noti_delete_template(const char *pkgname)
{
	sqlite3 *db = NULL;
	char query[NOTIFICATION_QUERY_MAX] = { 0, };
	int ret;

	if (pkgname == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	db = notification_db_open(DBPATH);
	if (!db)
		return get_last_result();

	snprintf(query, sizeof(query), "delete from noti_template "
		 "where caller_pkgname = '%s'", pkgname);

	ret = notification_db_exec(db, query, NULL);

	if (db)
		notification_db_close(&db);

	return ret;
}
