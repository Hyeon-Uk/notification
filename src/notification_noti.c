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
#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <vconf.h>
#include <pkgmgr-info.h>
#include <package_manager.h>
#include <app_control_internal.h>
#include <bundle_internal.h>
#include <iniparser.h>

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
#include "notification_db_query.h"

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

static void __free_encoded_data(char *encoded_data)
{
	if (encoded_data)
		bundle_free_encoded_rawdata((bundle_raw **)&encoded_data);
}

static void __notification_noti_populate_from_stmt(sqlite3_stmt *stmt, notification_h noti)
{
	int col = 0;
	int i = 0;

	if (stmt == NULL || noti == NULL)
		return ;

	noti->type = sqlite3_column_int(stmt, col++);
	noti->layout = sqlite3_column_int(stmt, col++);
	__free_and_set((void **)&(noti->pkg_id), notification_db_column_text(stmt, col++));
	__free_and_set((void **)&(noti->caller_app_id), notification_db_column_text(stmt, col++));
	__free_and_set((void **)&(noti->launch_app_id), notification_db_column_text(stmt, col++));
	noti->b_image_path = notification_db_column_bundle(stmt, col++);
	noti->group_id = sqlite3_column_int(stmt, col++);
	noti->internal_group_id = 0;
	noti->priv_id = sqlite3_column_int(stmt, col++);
	noti->b_text = notification_db_column_bundle(stmt, col++);
	noti->b_key = notification_db_column_bundle(stmt, col++);
	__free_and_set((void **)&(noti->tag), notification_db_column_text(stmt, col++));
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

	for (i = 0; i <= NOTIFICATION_EVENT_TYPE_MAX; i++)
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
	noti->hide_timeout = sqlite3_column_int(stmt, col++);
	noti->delete_timeout = sqlite3_column_int(stmt, col++);
	noti->text_input_max_length = sqlite3_column_int(stmt, col++);
	noti->event_flag = sqlite3_column_int(stmt, col++);
	noti->extension_image_size = sqlite3_column_int(stmt, col++);
	noti->uid = sqlite3_column_int(stmt, col++);

	noti->app_icon_path = NULL;
	noti->app_name = NULL;
	noti->temp_title = NULL;
	noti->temp_content = NULL;
}

static int _notification_noti_check_priv_id(notification_h noti, sqlite3 *db)
{
	sqlite3_stmt *stmt = NULL;
	char *query = NULL;
	int result = 0;
	int ret = NOTIFICATION_ERROR_NONE;

	query = sqlite3_mprintf("SELECT count(*) FROM noti_list "
				"WHERE caller_app_id = %Q AND priv_id = %d",
				noti->caller_app_id, noti->priv_id);
	if (query == NULL) {
		NOTIFICATION_ERR("fail to alloc query"); /* LCOV_EXCL_LINE */
		ret = NOTIFICATION_ERROR_OUT_OF_MEMORY;
		goto err;
	}

	ret = sqlite3_prepare_v2(db, query, strlen(query), &stmt, NULL);
	if (ret != SQLITE_OK) {
		NOTIFICATION_ERR("sqlite3_prepare_v2 failed [%d][%s]", ret,
				sqlite3_errmsg(db));
		ret = NOTIFICATION_ERROR_FROM_DB;
		goto err;
	}

	ret = sqlite3_step(stmt);
	if (ret == SQLITE_ROW)
		result = sqlite3_column_int(stmt, 0);
	else
		result = 0;

	/* If result > 0, there is priv_id in DB */
	if (result > 0)
		ret = NOTIFICATION_ERROR_ALREADY_EXIST_ID;

err:
	if (stmt)
		sqlite3_finalize(stmt);

	if (query)
		sqlite3_free(query);

	return ret;
}

static int _notification_noti_get_internal_group_id_by_priv_id(const char *app_id,
							int priv_id,
							sqlite3 *db)
{
	sqlite3_stmt *stmt = NULL;
	char *query = NULL;
	int ret = NOTIFICATION_ERROR_NONE;
	int result = 0;

	query = sqlite3_mprintf("SELECT internal_group_id FROM noti_list "
				"WHERE caller_app_id = %Q AND priv_id = %d",
				app_id, priv_id);
	if (query == NULL) {
		NOTIFICATION_ERR("fail to alloc query"); /* LCOV_EXCL_LINE */
		ret = NOTIFICATION_ERROR_OUT_OF_MEMORY;
		goto err;
	}

	ret = sqlite3_prepare_v2(db, query, strlen(query), &stmt, NULL);
	if (ret != SQLITE_OK) {
		NOTIFICATION_ERR("sqlite3_prepare_v2 failed [%d][%s]", ret,
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

static int _create_insertion_query(sqlite3 *db, notification_h noti, sqlite3_stmt *stmt, int *index)
{
	char buf_key[32] = { 0, };
	char *title_key = NULL;
	char *b_text = NULL;
	char *b_key = NULL;
	char *b_format_args = NULL;
	char *args = NULL;
	char *group_args = NULL;
	char *b_image_path = NULL;
	char *b_execute_option = NULL;
	char *b_service_responding = NULL;
	char *b_service_single_launch = NULL;
	char *b_service_multi_launch = NULL;
	char *b_event_handler[NOTIFICATION_EVENT_TYPE_MAX+1] = { NULL , };
	int flag_simmode = 0;
	int idx = 1;
	int i = 0;
	int b_encode_len = 0;
	int ret = NOTIFICATION_ERROR_NONE;

	if (noti == NULL || stmt == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	if (noti->b_image_path)
		bundle_encode(noti->b_image_path, (bundle_raw **)&b_image_path,
				&b_encode_len);

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
		title_key = noti->caller_app_id;

	if (noti->b_text)
		bundle_encode(noti->b_text, (bundle_raw **)&b_text, &b_encode_len);

	if (noti->b_key)
		bundle_encode(noti->b_key, (bundle_raw **)&b_key, &b_encode_len);

	if (noti->b_format_args)
		bundle_encode(noti->b_format_args,
			(bundle_raw **)&b_format_args, &b_encode_len);

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


	for (i = 0; i <= NOTIFICATION_EVENT_TYPE_MAX; i++) {
		if (noti->b_event_handler[i])
			bundle_encode(noti->b_event_handler[i],
				(bundle_raw **)&b_event_handler[i], &b_encode_len);
	}

	/* Check only simmode property is enable */
	if (noti->flags_for_property & NOTIFICATION_PROP_DISPLAY_ONLY_SIMMODE)
		flag_simmode = 1;

	__BIND_INT(db, stmt, idx++, noti->type, ret, out);
	__BIND_INT(db, stmt, idx++, noti->layout, ret, out);
	__BIND_TEXT(db, stmt, idx++, NOTIFICATION_CHECK_STR(noti->pkg_id), ret, out);
	__BIND_TEXT(db, stmt, idx++, NOTIFICATION_CHECK_STR(noti->caller_app_id), ret, out);
	__BIND_TEXT(db, stmt, idx++, NOTIFICATION_CHECK_STR(noti->launch_app_id), ret, out);
	__BIND_TEXT(db, stmt, idx++, NOTIFICATION_CHECK_STR(b_image_path), ret, out);
	__BIND_INT(db, stmt, idx++, noti->group_id, ret, out);
	__BIND_INT(db, stmt, idx++, noti->internal_group_id, ret, out);
	__BIND_TEXT_STATIC(db, stmt, idx++, NOTIFICATION_CHECK_STR(title_key), ret, out);
	__BIND_TEXT(db, stmt, idx++, NOTIFICATION_CHECK_STR(b_text), ret, out);
	__BIND_TEXT(db, stmt, idx++, NOTIFICATION_CHECK_STR(b_key), ret, out);
	__BIND_TEXT_STATIC(db, stmt, idx++, NOTIFICATION_CHECK_STR(noti->tag), ret, out);
	__BIND_TEXT(db, stmt, idx++, NOTIFICATION_CHECK_STR(b_format_args), ret, out);
	__BIND_INT(db, stmt, idx++, noti->num_format_args, ret, out);
	__BIND_TEXT(db, stmt, idx++, NOTIFICATION_CHECK_STR(noti->domain), ret, out);
	__BIND_TEXT(db, stmt, idx++, NOTIFICATION_CHECK_STR(noti->dir), ret, out);
	__BIND_INT(db, stmt, idx++, (int)noti->time, ret, out);
	__BIND_INT(db, stmt, idx++, (int)noti->insert_time, ret, out);
	__BIND_TEXT(db, stmt, idx++, NOTIFICATION_CHECK_STR(args), ret, out);
	__BIND_TEXT(db, stmt, idx++, NOTIFICATION_CHECK_STR(group_args), ret, out);
	__BIND_TEXT(db, stmt, idx++, NOTIFICATION_CHECK_STR(b_execute_option), ret, out);
	__BIND_TEXT(db, stmt, idx++, NOTIFICATION_CHECK_STR(b_service_responding), ret, out);
	__BIND_TEXT(db, stmt, idx++, NOTIFICATION_CHECK_STR(b_service_single_launch), ret, out);
	__BIND_TEXT(db, stmt, idx++, NOTIFICATION_CHECK_STR(b_service_multi_launch), ret, out);
	__BIND_TEXT(db, stmt, idx++, NOTIFICATION_CHECK_STR(b_event_handler[NOTIFICATION_EVENT_TYPE_CLICK_ON_BUTTON_1]), ret, out);
	__BIND_TEXT(db, stmt, idx++, NOTIFICATION_CHECK_STR(b_event_handler[NOTIFICATION_EVENT_TYPE_CLICK_ON_BUTTON_2]), ret, out);
	__BIND_TEXT(db, stmt, idx++, NOTIFICATION_CHECK_STR(b_event_handler[NOTIFICATION_EVENT_TYPE_CLICK_ON_BUTTON_3]), ret, out);
	__BIND_TEXT(db, stmt, idx++, NOTIFICATION_CHECK_STR(b_event_handler[NOTIFICATION_EVENT_TYPE_CLICK_ON_BUTTON_4]), ret, out);
	__BIND_TEXT(db, stmt, idx++, NOTIFICATION_CHECK_STR(b_event_handler[NOTIFICATION_EVENT_TYPE_CLICK_ON_BUTTON_5]), ret, out);
	__BIND_TEXT(db, stmt, idx++, NOTIFICATION_CHECK_STR(b_event_handler[NOTIFICATION_EVENT_TYPE_CLICK_ON_BUTTON_6]), ret, out);
	__BIND_TEXT(db, stmt, idx++, NOTIFICATION_CHECK_STR(b_event_handler[NOTIFICATION_EVENT_TYPE_CLICK_ON_ICON]), ret, out);
	__BIND_TEXT(db, stmt, idx++, NOTIFICATION_CHECK_STR(b_event_handler[NOTIFICATION_EVENT_TYPE_CLICK_ON_THUMBNAIL]), ret, out);
	__BIND_TEXT(db, stmt, idx++, NOTIFICATION_CHECK_STR(b_event_handler[NOTIFICATION_EVENT_TYPE_CLICK_ON_TEXT_INPUT_BUTTON]), ret, out);
	__BIND_INT(db, stmt, idx++, noti->sound_type, ret, out);
	__BIND_TEXT(db, stmt, idx++, NOTIFICATION_CHECK_STR(noti->sound_path), ret, out);
	__BIND_INT(db, stmt, idx++, noti->vibration_type, ret, out);
	__BIND_TEXT(db, stmt, idx++, NOTIFICATION_CHECK_STR(noti->vibration_path), ret, out);
	__BIND_INT(db, stmt, idx++, noti->led_operation, ret, out);
	__BIND_INT(db, stmt, idx++, noti->led_argb, ret, out);
	__BIND_INT(db, stmt, idx++, noti->led_on_ms, ret, out);
	__BIND_INT(db, stmt, idx++, noti->led_off_ms, ret, out);
	__BIND_INT(db, stmt, idx++, noti->flags_for_property, ret, out);
	__BIND_INT(db, stmt, idx++, flag_simmode, ret, out);
	__BIND_INT(db, stmt, idx++, noti->display_applist, ret, out);
	__BIND_DOUBLE(db, stmt, idx++, noti->progress_size, ret, out);
	__BIND_DOUBLE(db, stmt, idx++, noti->progress_percentage, ret, out);
	__BIND_INT(db, stmt, idx++, noti->ongoing_flag, ret, out);
	__BIND_INT(db, stmt, idx++, noti->ongoing_value_type, ret, out);
	__BIND_INT(db, stmt, idx++, noti->ongoing_current, ret, out);
	__BIND_INT(db, stmt, idx++, noti->ongoing_duration, ret, out);
	__BIND_INT(db, stmt, idx++, noti->auto_remove, ret, out);
	__BIND_INT(db, stmt, idx++, noti->default_button_index, ret, out);
	__BIND_INT(db, stmt, idx++, noti->hide_timeout, ret, out);
	__BIND_INT(db, stmt, idx++, noti->delete_timeout, ret, out);
	__BIND_INT(db, stmt, idx++, noti->text_input_max_length, ret, out);
	__BIND_INT(db, stmt, idx++, noti->event_flag, ret, out);
	__BIND_INT(db, stmt, idx++, noti->extension_image_size, ret, out);
	__BIND_INT(db, stmt, idx++, noti->uid, ret, out);

out:
	__free_encoded_data(b_image_path);
	__free_encoded_data(b_text);
	__free_encoded_data(b_key);
	__free_encoded_data(b_format_args);
	__free_encoded_data(args);
	__free_encoded_data(group_args);
	__free_encoded_data(b_execute_option);
	__free_encoded_data(b_service_responding);
	__free_encoded_data(b_service_single_launch);
	__free_encoded_data(b_service_multi_launch);

	for (i = 0; i <= NOTIFICATION_EVENT_TYPE_MAX; i++)
		__free_encoded_data(b_event_handler[i]);

	if (index != NULL && ret == NOTIFICATION_ERROR_NONE)
		*index = idx;

	return ret;
}

static int _create_update_query(sqlite3 *db, notification_h noti, sqlite3_stmt *stmt)
{
	char *b_image_path = NULL;
	char *b_text = NULL;
	char *b_key = NULL;
	char *b_format_args = NULL;
	char *args = NULL;
	char *group_args = NULL;
	char *b_execute_option = NULL;
	char *b_service_responding = NULL;
	char *b_service_single_launch = NULL;
	char *b_service_multi_launch = NULL;
	char *b_event_handler[NOTIFICATION_EVENT_TYPE_MAX+1] = { NULL , };
	int flag_simmode = 0;
	int idx = 1;
	int i = 0;
	int b_encode_len = 0;
	int ret = NOTIFICATION_ERROR_NONE;

	if (noti == NULL || stmt == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	/* Decode bundle to update DB */
	if (noti->b_image_path)
		bundle_encode(noti->b_image_path,
			(bundle_raw **)&b_image_path, &b_encode_len);

	if (noti->b_text)
		bundle_encode(noti->b_text, (bundle_raw **)&b_text, &b_encode_len);

	if (noti->b_key)
		bundle_encode(noti->b_key, (bundle_raw **)&b_key, &b_encode_len);

	if (noti->b_format_args)
		bundle_encode(noti->b_format_args,
			(bundle_raw **)&b_format_args, &b_encode_len);

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

	for (i = 0; i <= NOTIFICATION_EVENT_TYPE_MAX; i++) {
		if (noti->b_event_handler[i])
			bundle_encode(noti->b_event_handler[i],
					(bundle_raw **)&b_event_handler[i], &b_encode_len);
	}

	/* Check only simmode property is enable */
	if (noti->flags_for_property & NOTIFICATION_PROP_DISPLAY_ONLY_SIMMODE)
		flag_simmode = 1;

	__BIND_INT(db, stmt, idx++, noti->type, ret, out);
	__BIND_INT(db, stmt, idx++, noti->layout, ret, out);
	__BIND_TEXT(db, stmt, idx++, NOTIFICATION_CHECK_STR(noti->launch_app_id), ret, out);
	__BIND_TEXT(db, stmt, idx++, NOTIFICATION_CHECK_STR(b_image_path), ret, out);
	__BIND_TEXT(db, stmt, idx++, NOTIFICATION_CHECK_STR(b_text), ret, out);
	__BIND_TEXT(db, stmt, idx++, NOTIFICATION_CHECK_STR(b_key), ret, out);
	__BIND_TEXT_STATIC(db, stmt, idx++, NOTIFICATION_CHECK_STR(noti->tag), ret, out);
	__BIND_TEXT(db, stmt, idx++, NOTIFICATION_CHECK_STR(b_format_args), ret, out);
	__BIND_INT(db, stmt, idx++, noti->num_format_args, ret, out);
	__BIND_TEXT(db, stmt, idx++, NOTIFICATION_CHECK_STR(noti->domain), ret, out);
	__BIND_TEXT(db, stmt, idx++, NOTIFICATION_CHECK_STR(noti->dir), ret, out);
	__BIND_INT(db, stmt, idx++, (int)noti->time, ret, out);
	__BIND_INT(db, stmt, idx++, (int)noti->insert_time, ret, out);
	__BIND_TEXT(db, stmt, idx++, NOTIFICATION_CHECK_STR(args), ret, out);
	__BIND_TEXT(db, stmt, idx++, NOTIFICATION_CHECK_STR(group_args), ret, out);
	__BIND_TEXT(db, stmt, idx++, NOTIFICATION_CHECK_STR(b_execute_option), ret, out);
	__BIND_TEXT(db, stmt, idx++, NOTIFICATION_CHECK_STR(b_service_responding), ret, out);
	__BIND_TEXT(db, stmt, idx++, NOTIFICATION_CHECK_STR(b_service_single_launch), ret, out);
	__BIND_TEXT(db, stmt, idx++, NOTIFICATION_CHECK_STR(b_service_multi_launch), ret, out);
	__BIND_TEXT(db, stmt, idx++, NOTIFICATION_CHECK_STR(b_event_handler[NOTIFICATION_EVENT_TYPE_CLICK_ON_BUTTON_1]), ret, out);
	__BIND_TEXT(db, stmt, idx++, NOTIFICATION_CHECK_STR(b_event_handler[NOTIFICATION_EVENT_TYPE_CLICK_ON_BUTTON_2]), ret, out);
	__BIND_TEXT(db, stmt, idx++, NOTIFICATION_CHECK_STR(b_event_handler[NOTIFICATION_EVENT_TYPE_CLICK_ON_BUTTON_3]), ret, out);
	__BIND_TEXT(db, stmt, idx++, NOTIFICATION_CHECK_STR(b_event_handler[NOTIFICATION_EVENT_TYPE_CLICK_ON_BUTTON_4]), ret, out);
	__BIND_TEXT(db, stmt, idx++, NOTIFICATION_CHECK_STR(b_event_handler[NOTIFICATION_EVENT_TYPE_CLICK_ON_BUTTON_5]), ret, out);
	__BIND_TEXT(db, stmt, idx++, NOTIFICATION_CHECK_STR(b_event_handler[NOTIFICATION_EVENT_TYPE_CLICK_ON_BUTTON_6]), ret, out);
	__BIND_TEXT(db, stmt, idx++, NOTIFICATION_CHECK_STR(b_event_handler[NOTIFICATION_EVENT_TYPE_CLICK_ON_ICON]), ret, out);
	__BIND_TEXT(db, stmt, idx++, NOTIFICATION_CHECK_STR(b_event_handler[NOTIFICATION_EVENT_TYPE_CLICK_ON_THUMBNAIL]), ret, out);
	__BIND_TEXT(db, stmt, idx++, NOTIFICATION_CHECK_STR(b_event_handler[NOTIFICATION_EVENT_TYPE_CLICK_ON_TEXT_INPUT_BUTTON]), ret, out);
	__BIND_INT(db, stmt, idx++, noti->sound_type, ret, out);
	__BIND_TEXT(db, stmt, idx++, NOTIFICATION_CHECK_STR(noti->sound_path), ret, out);
	__BIND_INT(db, stmt, idx++, noti->vibration_type, ret, out);
	__BIND_TEXT(db, stmt, idx++, NOTIFICATION_CHECK_STR(noti->vibration_path), ret, out);
	__BIND_INT(db, stmt, idx++, noti->led_operation, ret, out);
	__BIND_INT(db, stmt, idx++, noti->led_argb, ret, out);
	__BIND_INT(db, stmt, idx++, noti->led_on_ms, ret, out);
	__BIND_INT(db, stmt, idx++, noti->led_off_ms, ret, out);
	__BIND_INT(db, stmt, idx++, noti->flags_for_property, ret, out);
	__BIND_INT(db, stmt, idx++, flag_simmode, ret, out);
	__BIND_INT(db, stmt, idx++, noti->display_applist, ret, out);
	__BIND_DOUBLE(db, stmt, idx++, noti->progress_size, ret, out);
	__BIND_DOUBLE(db, stmt, idx++, noti->progress_percentage, ret, out);
	__BIND_INT(db, stmt, idx++, noti->ongoing_flag, ret, out);
	__BIND_INT(db, stmt, idx++, noti->ongoing_value_type, ret, out);
	__BIND_INT(db, stmt, idx++, noti->ongoing_current, ret, out);
	__BIND_INT(db, stmt, idx++, noti->ongoing_duration, ret, out);
	__BIND_INT(db, stmt, idx++, noti->auto_remove, ret, out);
	__BIND_INT(db, stmt, idx++, noti->default_button_index, ret, out);
	__BIND_INT(db, stmt, idx++, noti->hide_timeout, ret, out);
	__BIND_INT(db, stmt, idx++, noti->delete_timeout, ret, out);
	__BIND_INT(db, stmt, idx++, noti->text_input_max_length, ret, out);
	__BIND_INT(db, stmt, idx++, noti->event_flag, ret, out);
	__BIND_INT(db, stmt, idx++, noti->extension_image_size, ret, out);

out:
	__free_encoded_data(b_image_path);
	__free_encoded_data(b_text);
	__free_encoded_data(b_key);
	__free_encoded_data(b_format_args);
	__free_encoded_data(args);
	__free_encoded_data(group_args);
	__free_encoded_data(b_execute_option);
	__free_encoded_data(b_service_responding);
	__free_encoded_data(b_service_single_launch);
	__free_encoded_data(b_service_multi_launch);

	for (i = 0; i <= NOTIFICATION_EVENT_TYPE_MAX; i++)
		__free_encoded_data(b_event_handler[i]);

	return ret;
}

static int _get_notification(char *query_where, notification_h noti)
{
	int ret;
	char *query = NULL;
	sqlite3 *db = NULL;
	sqlite3_stmt *stmt = NULL;

	if (query_where == NULL || noti == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	db = notification_db_open(DBPATH);
	if (!db)
		return get_last_result();

	query = sqlite3_mprintf("SELECT %s FROM %s",
				NOTI_LIST_DB_ATTRIBUTES_SELECT, query_where);
	if (query == NULL) {
		ret = NOTIFICATION_ERROR_OUT_OF_MEMORY;
		goto err;
	}

	ret = sqlite3_prepare_v2(db, query, -1, &stmt, NULL);
	if (ret != SQLITE_OK) {
		NOTIFICATION_ERR("sqlite3_prepare_v2 failed [%d][%s]", ret,
				sqlite3_errmsg(db));
		ret = NOTIFICATION_ERROR_FROM_DB;
		goto err;
	}

	ret = sqlite3_step(stmt);
	if (ret == SQLITE_ROW) {
		__notification_noti_populate_from_stmt(stmt, noti);
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

static int _get_notification_list(char *query_where, notification_list_h *list, int count)
{
	int ret;
	int internal_count = 0;
	char *query = NULL;
	sqlite3 *db = NULL;
	sqlite3_stmt *stmt = NULL;
	notification_list_h get_list = NULL;
	notification_h noti = NULL;

	if (query_where == NULL || list == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	db = notification_db_open(DBPATH);
	if (!db)
		return get_last_result();

	query = sqlite3_mprintf("SELECT %s FROM %s",
				NOTI_LIST_DB_ATTRIBUTES_SELECT, query_where);
	if (query == NULL) {
		ret = NOTIFICATION_ERROR_OUT_OF_MEMORY;
		goto err;
	}

	ret = sqlite3_prepare_v2(db, query, -1, &stmt, NULL);
	if (ret != SQLITE_OK) {
		NOTIFICATION_ERR("sqlite3_prepare_v2 failed [%d][%s]", ret,
				sqlite3_errmsg(db));
		ret = NOTIFICATION_ERROR_FROM_DB;
		goto err;
	}

	while (sqlite3_step(stmt) == SQLITE_ROW) {
		noti = (notification_h)calloc(1, sizeof(struct _notification));
		if (noti == NULL) {
			ret = NOTIFICATION_ERROR_OUT_OF_MEMORY;
			goto err;
		}

		__notification_noti_populate_from_stmt(stmt, noti);
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

err:
	if (stmt)
		sqlite3_finalize(stmt);

	if (db != NULL)
		notification_db_close(&db);

	if (get_list != NULL)
		*list = notification_list_get_head(get_list);

	return ret;
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

/* LCOV_EXCL_START */
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
		NOTIFICATION_WARN("Can't get or no the excute option [%x]", ret);
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
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

	ret = noti_setting_service_get_setting_by_app_id(app_id, &setting_new, noti->uid);
	if (ret != APP_CONTROL_ERROR_NONE || setting == NULL) {
		NOTIFICATION_ERR("noti_setting_service_get_setting_by_app_id failed [%x]", ret);
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
/* LCOV_EXCL_STOP */

static bool _is_allowed_to_notify(notification_h noti)
{
	notification_setting_h setting = NULL;
	bool allow_to_notify = true;
	bool app_disabled = false;
	bool ret = true;
	int err;

	err = noti_setting_service_get_setting_by_app_id(noti->caller_app_id,
							&setting, noti->uid);
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

	err = notification_system_setting_get_do_not_disturb(system_setting,
							&do_not_disturb);
	if (err != NOTIFICATION_ERROR_NONE) {
		NOTIFICATION_ERR("_get_do_not_disturb failed [%d]", err);
		goto out;
	}

	NOTIFICATION_DBG("do_not_disturb [%d]", do_not_disturb);

	if (do_not_disturb) {
		/* Check exception option of the caller app_id */
		err = noti_setting_service_get_setting_by_app_id(noti->caller_app_id, &setting, noti->uid);
		if (err != NOTIFICATION_ERROR_NONE) {
			NOTIFICATION_ERR("_get_setting_by_app_id failed [%d]", err);
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
			noti->display_applist = noti->display_applist &
						(NOTIFICATION_DISPLAY_APP_NOTIFICATION_TRAY |
						NOTIFICATION_DISPLAY_APP_INDICATOR);
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

static bool _is_pop_up_notification(const char *app_id, uid_t uid)
{
	int err;
	bool ret = true;
	notification_setting_h setting = NULL;

	err = noti_setting_service_get_setting_by_app_id(app_id, &setting, uid);
	if (err != NOTIFICATION_ERROR_NONE) {
		NOTIFICATION_WARN("Can't get the setting for %s [%x]", app_id, err);
		goto out;
	}

	err = notification_setting_get_pop_up_notification(setting, &ret);
	if (err != NOTIFICATION_ERROR_NONE) {
		NOTIFICATION_ERR("notification_setting_get_pop_up_notification failed");
		goto out;
	}

	if (ret != true)
		NOTIFICATION_DBG("[%s] is not allowed Pop-up notification", app_id);

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

	if (noti == NULL) {
		NOTIFICATION_ERR("NOTIFICATION_ERROR_INVALID_PARAMETER");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	if (_is_allowed_to_notify(noti) == false) {
		NOTIFICATION_DBG("[%s] is not allowed to notify", noti->caller_app_id);
		return NOTIFICATION_ERROR_PERMISSION_DENIED;
	}

	if (_handle_do_not_disturb_option(noti) != NOTIFICATION_ERROR_NONE)
		NOTIFICATION_WARN("_handle_do_not_disturb_option failed");

	if (_is_pop_up_notification((const char *)noti->caller_app_id, noti->uid) == false) {
		noti->display_applist = (noti->display_applist & (~NOTIFICATION_DISPLAY_APP_ACTIVE));
		NOTIFICATION_DBG("notification display applist - app_id [%s], applist [%d]",
				noti->caller_app_id, noti->display_applist);
	}

	if (_check_text_input(noti) != NOTIFICATION_ERROR_NONE)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	/* Initialize private ID */
	noti->group_id = NOTIFICATION_GROUP_ID_NONE;
	noti->internal_group_id = NOTIFICATION_GROUP_ID_NONE;

	db = notification_db_open(DBPATH);
	if (!db)
		return get_last_result();

	query = sqlite3_mprintf("INSERT INTO noti_list (%s) VALUES (%s)",
			NOTI_LIST_DB_ATTRIBUTES_INSERT, NOTI_LIST_INSERT_VALUES);
	if (query == NULL) {
		NOTIFICATION_ERR("sqlite3_mprintf failed");
		ret = NOTIFICATION_ERROR_OUT_OF_MEMORY;
		goto err;
	}

	ret = sqlite3_prepare_v2(db, query, -1, &stmt, NULL);
	if (ret != SQLITE_OK) {
		NOTIFICATION_ERR("sqlite3_prepare_v2 failed [%d][%s]", ret,
				 sqlite3_errmsg(db));
		ret = NOTIFICATION_ERROR_FROM_DB;
		goto err;
	}

	ret = _create_insertion_query(db, noti, stmt, NULL);
	if (ret != NOTIFICATION_ERROR_NONE)
		goto err;

	ret = sqlite3_step(stmt);
	if (ret == SQLITE_OK || ret == SQLITE_DONE) {
		noti->priv_id = (int)sqlite3_last_insert_rowid(db);
		ret = NOTIFICATION_ERROR_NONE;
	} else {
		ret = NOTIFICATION_ERROR_FROM_DB;
	}

err:
	if (stmt)
		sqlite3_finalize(stmt);

	if (query)
		sqlite3_free(query);

	if (db)
		notification_db_close(&db);

	return ret;
}

/* LCOV_EXCL_START */
EXPORT_API int notification_noti_get_by_priv_id(notification_h noti, int priv_id)
{
	int ret = NOTIFICATION_ERROR_NONE;
	char *query_where = NULL;

	if (priv_id < 0 || noti == NULL) {
		NOTIFICATION_ERR("NOTIFICATION_ERROR_INVALID_PARAMETER");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	query_where = sqlite3_mprintf("noti_list WHERE priv_id = %d", priv_id);
	if (query_where == NULL) {
		ret = NOTIFICATION_ERROR_OUT_OF_MEMORY;
		goto err;
	}

	ret = _get_notification(query_where, noti);

err:
	if (query_where)
		sqlite3_free(query_where);

	return ret;
}
/* LCOV_EXCL_STOP */

EXPORT_API int notification_noti_get_by_tag(notification_h noti, char *app_id, char *tag, uid_t uid)
{
	int ret = NOTIFICATION_ERROR_NONE;
	char *query_where;

	if (tag == NULL || noti == NULL) {
		NOTIFICATION_ERR("NOTIFICATION_ERROR_INVALID_PARAMETER");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	query_where = sqlite3_mprintf("noti_list WHERE caller_app_id = %Q "
			"AND tag = %Q AND uid = %d", app_id, tag, uid);
	if (query_where == NULL) {
		ret = NOTIFICATION_ERROR_OUT_OF_MEMORY;
		goto err;
	}

	ret = _get_notification(query_where, noti);

err:
	if (query_where)
		sqlite3_free(query_where);

	return ret;
}

EXPORT_API int notification_noti_update(notification_h noti)
{
	int ret = NOTIFICATION_ERROR_NONE;
	sqlite3 *db;
	sqlite3_stmt *stmt = NULL;
	char *query = NULL;

	if (noti == NULL) {
		NOTIFICATION_ERR("NOTIFICATION_ERROR_INVALID_PARAMETER");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	if (_is_allowed_to_notify(noti) == false) {
		NOTIFICATION_DBG("[%s] is not allowed to notify", noti->caller_app_id);
		return NOTIFICATION_ERROR_PERMISSION_DENIED;
	}

	if (_handle_do_not_disturb_option(noti) != NOTIFICATION_ERROR_NONE)
		NOTIFICATION_WARN("_handle_do_not_disturb_option failed");

	if (_is_pop_up_notification((const char *)noti->caller_app_id, noti->uid) == false) {
		noti->display_applist = (noti->display_applist & (~NOTIFICATION_DISPLAY_APP_ACTIVE));
		NOTIFICATION_DBG("notification display applist - app_id [%s], applist [%d]",
				noti->caller_app_id, noti->display_applist);
	}

	db = notification_db_open(DBPATH);
	if (!db)
		return get_last_result();

	/* Check private ID is exist */
	ret = _notification_noti_check_priv_id(noti, db);
	if (ret != NOTIFICATION_ERROR_ALREADY_EXIST_ID) {
		NOTIFICATION_ERR("NOTIFICATION_ERROR_NOT_EXIST_ID");
		ret = NOTIFICATION_ERROR_NOT_EXIST_ID;
		goto err;
	}

	query = sqlite3_mprintf("UPDATE noti_list SET %s WHERE priv_id = %d ",
				NOTI_LIST_DB_ATTRIBUTES_UPDATE, noti->priv_id);
	if (query == NULL) {
		ret = NOTIFICATION_ERROR_OUT_OF_MEMORY;
		goto err;
	}

	ret = sqlite3_prepare_v2(db, query, -1, &stmt, NULL);
	if (ret != SQLITE_OK) {
		NOTIFICATION_ERR("sqlite3_prepare_v2 failed [%d][%s]", ret,
				 sqlite3_errmsg(db));
		ret = NOTIFICATION_ERROR_FROM_DB;
		goto err;
	}

	ret = _create_update_query(db, noti, stmt);
	if (ret != NOTIFICATION_ERROR_NONE)
		goto err;

	ret = sqlite3_step(stmt);
	if (ret == SQLITE_OK || ret == SQLITE_DONE)
		ret = NOTIFICATION_ERROR_NONE;
	else
		ret = NOTIFICATION_ERROR_FROM_DB;

err:
	if (stmt)
		sqlite3_finalize(stmt);

	if (query)
		sqlite3_free(query);

	if (db)
		notification_db_close(&db);

	return ret;
}

EXPORT_API int notification_noti_delete_all(notification_type_e type,
					const char *app_id, int *num_deleted,
					int **list_deleted_rowid, uid_t uid)
{
	sqlite3 *db = NULL;
	sqlite3_stmt *stmt = NULL;
	char buf[128] = { 0, };
	char *query = NULL;
	char *query_base = NULL;
	char *query_where = NULL;
	char err_buf[ERR_BUFFER_SIZE];
	int ret = NOTIFICATION_ERROR_NONE;
	int ret_tmp = NOTIFICATION_ERROR_NONE;
	int i = 0;
	int data_cnt = 0;

	db = notification_db_open(DBPATH);
	if (!db)
		return get_last_result();

	query_base = sqlite3_mprintf("DELETE FROM noti_list");
	if (query_base == NULL) {
		ret = NOTIFICATION_ERROR_OUT_OF_MEMORY;
		goto err;
	}

	if (app_id == NULL || strlen(app_id) == 0) {
		if (type != NOTIFICATION_TYPE_NONE) {
			query_where = sqlite3_mprintf("WHERE type = %d "
						"AND uid = %d", type, uid);
			if (query_where == NULL) {
				ret = NOTIFICATION_ERROR_OUT_OF_MEMORY;
				goto err;
			}
		}
	} else {
		if (type == NOTIFICATION_TYPE_NONE)
			query_where = sqlite3_mprintf("WHERE caller_app_id = %Q "
						"AND uid = %d", app_id, uid);
		else
			query_where = sqlite3_mprintf("WHERE caller_app_id = %Q "
						"AND type = %d AND uid = %d",
						app_id, type, uid);

		if (query_where == NULL) {
			ret = NOTIFICATION_ERROR_OUT_OF_MEMORY;
			goto err;
		}
	}

	if (num_deleted != NULL)
		*num_deleted = 0;

	if (list_deleted_rowid != NULL) {
		*list_deleted_rowid = NULL;

		query = sqlite3_mprintf("SELECT priv_id FROM noti_list %s ", query_where);
		if (query == NULL) {
			ret = NOTIFICATION_ERROR_OUT_OF_MEMORY;
			goto err;
		}

		ret = sqlite3_prepare_v2(db, query, strlen(query), &stmt, NULL);
		if (ret != SQLITE_OK) {
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

		if (data_cnt > 0) {
			query_where[0] = '\0';

			for (i = 0; i < data_cnt ; i++) {
				if (i % NOTI_BURST_DELETE_UNIT == 0 && i != 0) {
					query = sqlite3_mprintf("%s WHERE priv_id in (%s)", query_base, query_where);
					if (query == NULL) {
						ret = NOTIFICATION_ERROR_OUT_OF_MEMORY;
						goto err;
					}
					ret_tmp = notification_db_exec(db, query, NULL);
					query_where[0] = '\0';
					if (ret == NOTIFICATION_ERROR_NONE)
						ret = ret_tmp;
				}
				snprintf(buf, sizeof(buf) - 1, "%s%d", (i % NOTI_BURST_DELETE_UNIT == 0) ? "" : ",", *((*list_deleted_rowid) + i));
				query_where = sqlite3_mprintf("%s%s", query_where, buf);
				if (query_where == NULL) {
					ret = NOTIFICATION_ERROR_OUT_OF_MEMORY;
					goto err;
				}
			}

			if ((i <= NOTI_BURST_DELETE_UNIT) || ((i % NOTI_BURST_DELETE_UNIT) > 0)) {
				query = sqlite3_mprintf("%s WHERE priv_id in (%s)", query_base, query_where);
				if (query == NULL) {
					ret = NOTIFICATION_ERROR_OUT_OF_MEMORY;
					goto err;
				}

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
		query = sqlite3_mprintf("%s %s", query_base, query_where);
		if (query == NULL) {
			ret = NOTIFICATION_ERROR_OUT_OF_MEMORY;
			goto err;
		}

		ret = notification_db_exec(db, query, num_deleted);
	}

err:
	if (stmt)
		sqlite3_finalize(stmt);

	if (query)
		sqlite3_free(query);

	if (query_base)
		sqlite3_free(query_base);

	if (query_where)
		sqlite3_free(query_where);

	if (db)
		notification_db_close(&db);

	return ret;
}

/* LCOV_EXCL_START */
EXPORT_API int notification_noti_delete_by_priv_id(const char *app_id, int priv_id)
{
	sqlite3 *db = NULL;
	char *query = NULL;
	int ret = NOTIFICATION_ERROR_NONE;

	if (app_id == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	db = notification_db_open(DBPATH);
	if (!db)
		return get_last_result();

	query = sqlite3_mprintf("DELETE FROM noti_list WHERE caller_app_id = %Q "
				"AND priv_id = %d", app_id, priv_id);
	if (query == NULL) {
		ret = NOTIFICATION_ERROR_OUT_OF_MEMORY;
		goto err;
	}

	ret = notification_db_exec(db, query, NULL);

err:
	if (query)
		sqlite3_free(query);

	if (db)
		notification_db_close(&db);

	return ret;
}
/* LCOV_EXCL_STOP */

EXPORT_API int notification_noti_delete_by_priv_id_get_changes(const char *app_id,
							int priv_id,
							int *num_changes,
							uid_t uid)
{
	sqlite3 *db = NULL;
	char *query = NULL;
	int ret = NOTIFICATION_ERROR_NONE;

	db = notification_db_open(DBPATH);
	if (!db)
		return get_last_result();

	query = sqlite3_mprintf("DELETE FROM noti_list WHERE caller_app_id = %Q "
				"AND priv_id = %d AND uid = %d",
				app_id, priv_id, uid);
	if (query == NULL) {
		ret = NOTIFICATION_ERROR_OUT_OF_MEMORY;
		goto err;
	}

	ret = notification_db_exec(db, query, num_changes);

err:
	if (query)
		sqlite3_free(query);

	if (db)
		notification_db_close(&db);

	return ret;
}

/* todo refactoring */
/* LCOV_EXCL_START */
EXPORT_API int notification_noti_get_count(notification_type_e type,
					const char *app_id,
					int group_id, int priv_id,
					int *count, uid_t uid)
{
	sqlite3 *db = NULL;
	sqlite3_stmt *stmt = NULL;
	char *query = NULL;
	char *query_base = NULL;
	char *query_where = NULL;
	char *query_where_more = NULL;
	int ret = NOTIFICATION_ERROR_NONE;
	int get_count = 0;
	int internal_group_id = 0;
	int status = VCONFKEY_TELEPHONY_SIM_UNKNOWN;

	/* Check current sim status */
	ret = vconf_get_int(VCONFKEY_TELEPHONY_SIM_SLOT, &status);
	if (ret < 0)
		return NOTIFICATION_ERROR_IO_ERROR;

	db = notification_db_open(DBPATH);
	if (!db)
		return get_last_result();

	query_base = sqlite3_mprintf("SELECT count(*) FROM noti_list ");
	if (query_base == NULL) {
		ret = NOTIFICATION_ERROR_OUT_OF_MEMORY;
		goto err;
	}

	internal_group_id =
		_notification_noti_get_internal_group_id_by_priv_id(app_id, priv_id, db);

	if (group_id == NOTIFICATION_GROUP_ID_NONE) {
		if (priv_id == NOTIFICATION_PRIV_ID_NONE)
			query_where = sqlite3_mprintf("WHERE caller_app_id = %Q "
					"AND uid = %d ", app_id, uid);
		else
			query_where = sqlite3_mprintf("WHERE caller_app_id = %Q "
					"AND internal_group_id = %d AND uid = %d ",
					app_id, internal_group_id, uid);
	} else {
		if (priv_id == NOTIFICATION_PRIV_ID_NONE)
			query_where = sqlite3_mprintf("WHERE caller_app_id = %Q "
					"AND group_id = %d AND uid = %d ",
					app_id, group_id, uid);
		else
			query_where = sqlite3_mprintf("WHERE caller_app_id = %Q "
					"AND internal_group_id = %d AND uid = %d ",
					app_id, internal_group_id, uid);
	}

	if (query_where == NULL) {
		ret = NOTIFICATION_ERROR_OUT_OF_MEMORY;
		goto err;
	}

	if (status == VCONFKEY_TELEPHONY_SIM_INSERTED) {
		if (type != NOTIFICATION_TYPE_NONE) {
			query_where_more = sqlite3_mprintf("type = %d ", type);
			if (query_where_more == NULL) {
				ret = NOTIFICATION_ERROR_OUT_OF_MEMORY;
				goto err;
			}
		}
	} else {
		if (type != NOTIFICATION_TYPE_NONE)
			query_where_more = sqlite3_mprintf("type = %d AND "
					"flag_simmode = 0 ", type);
		else
			query_where_more = sqlite3_mprintf("flag_simmode = 0 ");

		if (query_where_more == NULL) {
			ret = NOTIFICATION_ERROR_OUT_OF_MEMORY;
			goto err;
		}
	}

	if (query_where_more) {
		query = sqlite3_mprintf("%s %s AND %s", query_base, query_where,
				query_where_more);
		if (query == NULL) {
			ret = NOTIFICATION_ERROR_OUT_OF_MEMORY;
			goto err;
		}
	} else {
		query = sqlite3_mprintf("%s %s", query_base, query_where);
		if (query == NULL) {
			ret = NOTIFICATION_ERROR_OUT_OF_MEMORY;
			goto err;
		}
	}

	ret = sqlite3_prepare_v2(db, query, -1, &stmt, NULL);
	if (ret != SQLITE_OK) {
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

	if (query)
		sqlite3_free(query);

	if (query_base)
		sqlite3_free(query_base);

	if (query_where)
		sqlite3_free(query_where);

	if (query_where_more)
		sqlite3_free(query_where_more);

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
	char *query = NULL;
	char *query_uid = NULL;
	char *query_where = NULL;
	int ret = NOTIFICATION_ERROR_NONE;
	int status;

	/* Check current sim status */
	ret = vconf_get_int(VCONFKEY_TELEPHONY_SIM_SLOT, &status);
	if (ret < 0)
		ret = NOTIFICATION_ERROR_IO_ERROR;

	if (status == VCONFKEY_TELEPHONY_SIM_INSERTED) {
		if (type != NOTIFICATION_TYPE_NONE) {
			query_where = sqlite3_mprintf(" AND type = %d ", type);
			if (query_where == NULL)
				return NOTIFICATION_ERROR_OUT_OF_MEMORY;
		}
	} else {
		if (type != NOTIFICATION_TYPE_NONE)
			query_where = sqlite3_mprintf(" AND type = %d AND "
					"flag_simmode = 0 ", type);
		else
			query_where = sqlite3_mprintf(" AND flag_simmode = 0 ");

		if (query_where == NULL)
			return NOTIFICATION_ERROR_OUT_OF_MEMORY;
	}

	if (uid != NOTIFICATION_GLOBAL_UID) {
		query_uid = sqlite3_mprintf(" AND uid = %d ", uid);
		if (query_uid == NULL) {
			ret = NOTIFICATION_ERROR_OUT_OF_MEMORY;
			goto err;
		}
	}

	query = sqlite3_mprintf("noti_list WHERE 1 > 0 %s %s "
			"ORDER BY rowid DESC, time DESC", query_where, query_uid);
	if (query == NULL) {
		ret = NOTIFICATION_ERROR_OUT_OF_MEMORY;
		goto err;
	}

	ret = _get_notification_list(query, list, count);

err:
	if (query_where)
		sqlite3_free(query_where);

	if (query_uid)
		sqlite3_free(query_uid);

	if (query)
		sqlite3_free(query);

	return ret;
}

EXPORT_API int notification_noti_get_detail_list(const char *app_id,
						int group_id,
						int priv_id, int count,
						notification_list_h *list,
						uid_t uid)
{
	char *query_where = NULL;
	char *query = NULL;
	int ret = NOTIFICATION_ERROR_NONE;
	int internal_group_id = 0;
	int status = 0;
	sqlite3 *db = NULL;

	/* Check current sim status */
	ret = vconf_get_int(VCONFKEY_TELEPHONY_SIM_SLOT, &status);
	if (ret < 0)
		return NOTIFICATION_ERROR_IO_ERROR;

	db = notification_db_open(DBPATH);
	if (!db)
		return get_last_result();

	if (priv_id == NOTIFICATION_PRIV_ID_NONE &&
		group_id == NOTIFICATION_GROUP_ID_NONE) {
		if (status == VCONFKEY_TELEPHONY_SIM_INSERTED)
			query_where = sqlite3_mprintf("WHERE  caller_app_id = %Q "
				"AND uid = %d ", app_id, uid);
		else
			query_where = sqlite3_mprintf("WHERE  caller_app_id = %Q "
				"AND flag_simmode = 0 AND uid = %d ", app_id, uid);

	} else {
		internal_group_id =
			_notification_noti_get_internal_group_id_by_priv_id(app_id,
								priv_id, db);

		if (status == VCONFKEY_TELEPHONY_SIM_INSERTED)
			query_where = sqlite3_mprintf("WHERE  caller_app_id = %Q "
				"AND internal_group_id = %d AND uid = %d ",
				 app_id, internal_group_id, uid);
		else
			query_where = sqlite3_mprintf("WHERE  caller_app_id = %Q "
				"AND internal_group_id = %d AND flag_simmode = 0 "
				"AND uid = %d ",
				 app_id, internal_group_id, uid);
	}

	if (query_where == NULL) {
		ret = NOTIFICATION_ERROR_OUT_OF_MEMORY;
		goto err;
	}

	query = sqlite3_mprintf("noti_list %s ORDER BY rowid DESC, time DESC",
			query_where);
	if (query == NULL) {
		ret = NOTIFICATION_ERROR_OUT_OF_MEMORY;
		goto err;
	}

	ret = _get_notification_list(query, list, count);

err:
	if (query_where)
		sqlite3_free(query_where);

	if (query)
		sqlite3_free(query);

	if (db)
		notification_db_close(&db);

	return ret;
}

EXPORT_API int notification_noti_check_tag(notification_h noti)
{
	sqlite3 *db;
	sqlite3_stmt *stmt = NULL;
	char *query = NULL;
	int ret = NOTIFICATION_ERROR_NONE;
	int result = 0;

	if (noti->tag == NULL)
		return NOTIFICATION_ERROR_NOT_EXIST_ID;

	db = notification_db_open(DBPATH);
	if (!db)
		return get_last_result();

	query = sqlite3_mprintf("SELECT priv_id FROM noti_list "
			"WHERE caller_app_id = %Q AND tag = %Q",
			NOTIFICATION_CHECK_STR(noti->caller_app_id),
			NOTIFICATION_CHECK_STR(noti->tag));
	if (query == NULL) {
		ret = NOTIFICATION_ERROR_OUT_OF_MEMORY;
		goto err;
	}

	ret = sqlite3_prepare_v2(db, query, strlen(query), &stmt, NULL);
	if (ret != SQLITE_OK) {
		NOTIFICATION_ERR("sqlite3_prepare_v2 failed [%d][%s]", ret,
			sqlite3_errmsg(db));
		goto err;
	}

	ret = sqlite3_step(stmt);
	if (ret == SQLITE_ROW)
		result = sqlite3_column_int(stmt, 0);
	else
		result = 0;

	/* If result > 0, there is priv_id in DB */
	if (result > 0) {
		noti->priv_id = result;
		ret = NOTIFICATION_ERROR_ALREADY_EXIST_ID;
	} else {
		ret = NOTIFICATION_ERROR_NOT_EXIST_ID;
	}

err:
	if (stmt)
		sqlite3_finalize(stmt);

	if (query)
		sqlite3_free(query);

	if (db)
		notification_db_close(&db);

	return ret;
}

EXPORT_API int notification_noti_check_count_for_template(notification_h noti, int *count)
{
	sqlite3 *db;
	sqlite3_stmt *stmt = NULL;
	char *query = NULL;
	int ret = NOTIFICATION_ERROR_NONE;
	int result = 0;

	if (noti == NULL || count == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	db = notification_db_open(DBPATH);
	if (!db)
		return get_last_result();

	query = sqlite3_mprintf("SELECT COUNT(caller_app_id) FROM noti_template "
				"WHERE caller_app_id = %Q",
				NOTIFICATION_CHECK_STR(noti->caller_app_id));
	if (query == NULL) {
		ret = NOTIFICATION_ERROR_OUT_OF_MEMORY;
		goto err;
	}

	ret = sqlite3_prepare_v2(db, query, strlen(query), &stmt, NULL);
	if (ret != SQLITE_OK) {
		NOTIFICATION_ERR("sqlite3_prepare_v2 failed [%d][%s]", ret,
			sqlite3_errmsg(db));
		ret = NOTIFICATION_ERROR_FROM_DB;
		goto err;
	}

	ret = sqlite3_step(stmt);
	if (ret == SQLITE_ROW)
		result = sqlite3_column_int(stmt, 0);
	else
		result = 0;

	*count = result;

err:
	if (stmt)
		sqlite3_finalize(stmt);

	if (query)
		sqlite3_free(query);

	if (db)
		notification_db_close(&db);

	return ret;
}

EXPORT_API int notification_noti_add_template(notification_h noti, char *template_name)
{
	sqlite3 *db = NULL;
	sqlite3_stmt *stmt = NULL;
	char *query = NULL;
	int idx = 1;
	int ret = NOTIFICATION_ERROR_NONE;

	if (noti == NULL || template_name == NULL) {
		NOTIFICATION_ERR("NOTIFICATION_ERROR_INVALID_PARAMETER");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	/* Initialize private ID */
	noti->group_id = NOTIFICATION_GROUP_ID_NONE;
	noti->internal_group_id = NOTIFICATION_GROUP_ID_NONE;

	db = notification_db_open(DBPATH);
	if (!db)
		return get_last_result();

	query = sqlite3_mprintf("INSERT OR REPLACE INTO noti_template "
				"(%s, template_name) VALUES (%s, %Q)",
				NOTI_LIST_DB_ATTRIBUTES_INSERT,
				NOTI_LIST_INSERT_VALUES,
				template_name);
	if (query == NULL) {
		ret = NOTIFICATION_ERROR_OUT_OF_MEMORY;
		goto err;
	}

	ret = sqlite3_prepare_v2(db, query, -1, &stmt, NULL);
	if (ret != SQLITE_OK) {
		NOTIFICATION_ERR("sqlite3_prepare_v2 failed [%d][%s]", ret,
				sqlite3_errmsg(db));
		ret = NOTIFICATION_ERROR_FROM_DB;
		goto err;
	}

	ret = _create_insertion_query(db, noti, stmt, &idx);
	if (ret != NOTIFICATION_ERROR_NONE)
		goto err;

	ret = sqlite3_step(stmt);
	if (ret == SQLITE_OK || ret == SQLITE_DONE)
		ret = NOTIFICATION_ERROR_NONE;
	else
		ret = NOTIFICATION_ERROR_FROM_DB;

err:
	if (stmt)
		sqlite3_finalize(stmt);

	if (query)
		sqlite3_free(query);

	if (db)
		notification_db_close(&db);

	return ret;
}

EXPORT_API int notification_noti_get_package_template(notification_h noti, char *app_id, char *template_name)
{
	int ret = NOTIFICATION_ERROR_NONE;
	char *query_where = NULL;

	if (noti == NULL || app_id == NULL || template_name == NULL) {
		NOTIFICATION_ERR("NOTIFICATION_ERROR_INVALID_PARAMETER");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	query_where = sqlite3_mprintf("noti_template WHERE caller_app_id = %Q "
			"AND template_name = %Q", app_id, template_name);
	if (query_where == NULL)
		return NOTIFICATION_ERROR_OUT_OF_MEMORY;

	ret = _get_notification(query_where, noti);
	if (ret != NOTIFICATION_ERROR_NONE)
		NOTIFICATION_ERR("_get_notification failed, %d", ret);

	if (query_where)
		sqlite3_free(query_where);

	return ret;
}

EXPORT_API int notification_noti_delete_template(const char *pkg_id)
{
	sqlite3 *db = NULL;
	char *query = NULL;
	int ret = NOTIFICATION_ERROR_NONE;

	if (pkg_id == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	db = notification_db_open(DBPATH);
	if (!db)
		return get_last_result();

	query = sqlite3_mprintf("DELETE FROM noti_template WHERE pkg_id = %Q",
				NOTIFICATION_CHECK_STR(pkg_id));
	if (query == NULL) {
		ret = NOTIFICATION_ERROR_OUT_OF_MEMORY;
		goto err;
	}

	ret = notification_db_exec(db, query, NULL);

err:
	if (query)
		sqlite3_free(query);

	if (db)
		notification_db_close(&db);

	return ret;
}
