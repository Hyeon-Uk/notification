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

#define NOTIFICATION_DB_NAME ".notification.db"
#define NOTIFICATION_SETTING_DB_TABLE "notification_setting"
#define NOTIFICATION_SYSTEM_SETTING_DB_TABLE "notification_system_setting"
#define NOTIFICATION_DND_ALLOW_EXCEPTION "dnd_allow_exception"

#define CREATE_NOTIFICATION_TABLE "\
PRAGMA journal_mode = PERSIST; \
PRAGMA synchronous = FULL; \
CREATE TABLE IF NOT EXISTS noti_list ( \
	type INTEGER NOT NULL, \
	layout INTEGER NOT NULL DEFAULT 0, \
	pkg_id TEXT NOT NULL, \
	caller_app_id TEXT NOT NULL, \
	launch_app_id TEXT, \
	image_path TEXT, \
	priv_image_path TEXT, \
	group_id INTEGER DEFAULT 0, \
	internal_group_id INTEGER DEFAULT 0, \
	priv_id INTEGER PRIMARY KEY AUTOINCREMENT, \
	title_key TEXT, \
	b_text TEXT, \
	b_key TEXT, \
	tag TEXT, \
	b_format_args TEXT, \
	num_format_args INTEGER DEFAULT 0, \
	text_domain TEXT, \
	text_dir TEXT, \
	time INTEGER DEFAULT 0, \
	insert_time INTEGER DEFAULT 0, \
	args TEXT, \
	group_args TEXT, \
	b_execute_option TEXT, \
	b_service_responding TEXT, \
	b_service_single_launch TEXT, \
	b_service_multi_launch TEXT, \
	b_event_handler_click_on_button_1 TEXT, \
	b_event_handler_click_on_button_2 TEXT, \
	b_event_handler_click_on_button_3 TEXT, \
	b_event_handler_click_on_button_4 TEXT, \
	b_event_handler_click_on_button_5 TEXT, \
	b_event_handler_click_on_button_6 TEXT, \
	b_event_handler_click_on_icon TEXT, \
	b_event_handler_click_on_thumbnail TEXT, \
	b_event_handler_click_on_text_input_button TEXT, \
	sound_type INTEGER DEFAULT 0, \
	sound_path TEXT, \
	priv_sound_path TEXT, \
	vibration_type INTEGER DEFAULT 0, \
	vibration_path TEXT, \
	priv_vibration_path TEXT, \
	led_operation INTEGER DEFAULT 0, \
	led_argb INTEGER DEFAULT 0, \
	led_on_ms INTEGER DEFAULT -1, \
	led_off_ms INTEGER DEFAULT -1, \
	flags_for_property INTEGER DEFAULT 0, \
	flag_simmode INTEGER DEFAULT 0, \
	display_applist INTEGER, \
	progress_size DOUBLE DEFAULT 0, \
	progress_percentage DOUBLE DEFAULT 0, \
	ongoing_flag INTEGER DEFAULT 0, \
	ongoing_value_type INTEGER DEFAULT 0, \
	ongoing_current INTEGER DEFAULT 0, \
	ongoing_duration INTEGER DEFAULT 0, \
	auto_remove INTEGER DEFAULT 1, \
	default_button_index INTEGER DEFAULT 0, \
	hide_timeout INTEGER DEFAULT 0, \
	delete_timeout INTEGER DEFAULT 0, \
	text_input_max_length INTEGER DEFAULT 0, \
	event_flag INTEGER DEFAULT 0, \
	extension_image_size INTEGER DEFAULT 0, \
	uid INTEGER \
); \
CREATE TABLE IF NOT EXISTS noti_group_data ( \
	caller_app_id TEXT NOT NULL, \
	group_id INTEGER DEFAULT 0, \
	badge INTEGER DEFAULT 0, \
	title TEXT, \
	content TEXT, \
	loc_title TEXT, \
	loc_content TEXT, \
	count_display_title INTEGER, \
	count_display_content INTEGER, \
	rowid INTEGER PRIMARY KEY AUTOINCREMENT, \
	UNIQUE (caller_app_id, group_id) \
); \
CREATE TABLE IF NOT EXISTS ongoing_list ( \
	caller_app_id TEXT NOT NULL, \
	launch_app_id TEXT, \
	icon_path TEXT, \
	group_id INTEGER DEFAULT 0, \
	internal_group_id INTEGER DEFAULT 0, \
	priv_id INTERGER NOT NULL, \
	title TEXT, \
	content TEXT, \
	default_content TEXT, \
	loc_title TEXT, \
	loc_content TEXT, \
	loc_default_content TEXT, \
	text_domain TEXT, \
	text_dir TEXT, \
	args TEXT, \
	group_args TEXT, \
	flag INTEGER DEFAULT 0, \
	progress_size DOUBLE DEFAULT 0, \
	progress_percentage DOUBLE DEFAULT 0, \
	rowid INTEGER PRIMARY KEY AUTOINCREMENT, \
	UNIQUE (caller_app_id, priv_id) \
); \
CREATE TABLE IF NOT EXISTS notification_setting ( \
	uid INTEGER, \
	package_name TEXT NOT NULL, \
	app_id TEXT NOT NULL, \
	allow_to_notify INTEGER DEFAULT 1, \
	do_not_disturb_except INTEGER DEFAULT 0, \
	visibility_class INTEGER DEFAULT 0, \
	pop_up_notification INTEGER DEFAULT 1, \
	lock_screen_content_level INTEGER DEFAULT 0, \
	app_disabled INTEGER DEFAULT 0, \
	UNIQUE (uid, package_name, app_id) \
); \
CREATE TABLE IF NOT EXISTS notification_system_setting ( \
	uid INTEGER, \
	do_not_disturb INTEGER DEFAULT 0, \
	visibility_class INTEGER DEFAULT 0, \
	dnd_schedule_enabled INTEGER DEFAULT 0, \
	dnd_schedule_day INTEGER DEFAULT 62, \
	dnd_start_hour INTEGER DEFAULT 22, \
	dnd_start_min INTEGER DEFAULT 0, \
	dnd_end_hour INTEGER DEFAULT 8, \
	dnd_end_min INTEGER DEFAULT 0, \
	lock_screen_content_level INTEGER DEFAULT 0, \
	UNIQUE (uid) \
); \
CREATE TABLE IF NOT EXISTS dnd_allow_exception ( \
	uid INTEGER, \
	type INTEGER DEFAULT 0, \
	value INTEGER DEFAULT 0, \
	UNIQUE (uid, type) \
); \
CREATE TABLE IF NOT EXISTS noti_template ( \
	type INTEGER NOT NULL, \
	layout INTEGER NOT NULL DEFAULT 0, \
	pkg_id TEXT NOT NULL, \
	caller_app_id TEXT NOT NULL, \
	launch_app_id TEXT, \
	image_path TEXT, \
	priv_image_path TEXT, \
	group_id INTEGER DEFAULT 0, \
	internal_group_id INTEGER DEFAULT 0, \
	priv_id INTEGER PRIMARY KEY AUTOINCREMENT, \
	title_key TEXT, \
	b_text TEXT, \
	b_key TEXT, \
	tag TEXT, \
	b_format_args TEXT, \
	num_format_args INTEGER DEFAULT 0, \
	text_domain TEXT, \
	text_dir TEXT, \
	time INTEGER DEFAULT 0, \
	insert_time INTEGER DEFAULT 0, \
	args TEXT, \
	group_args TEXT, \
	b_execute_option TEXT, \
	b_service_responding TEXT, \
	b_service_single_launch TEXT, \
	b_service_multi_launch TEXT, \
	b_event_handler_click_on_button_1 TEXT, \
	b_event_handler_click_on_button_2 TEXT, \
	b_event_handler_click_on_button_3 TEXT, \
	b_event_handler_click_on_button_4 TEXT, \
	b_event_handler_click_on_button_5 TEXT, \
	b_event_handler_click_on_button_6 TEXT, \
	b_event_handler_click_on_icon TEXT, \
	b_event_handler_click_on_thumbnail TEXT, \
	b_event_handler_click_on_text_input_button TEXT, \
	sound_type INTEGER DEFAULT 0, \
	sound_path TEXT, \
	priv_sound_path TEXT, \
	vibration_type INTEGER DEFAULT 0, \
	vibration_path TEXT, \
	priv_vibration_path TEXT, \
	led_operation INTEGER DEFAULT 0, \
	led_argb INTEGER DEFAULT 0, \
	led_on_ms INTEGER DEFAULT -1, \
	led_off_ms INTEGER DEFAULT -1, \
	flags_for_property INTEGER DEFAULT 0, \
	flag_simmode INTEGER DEFAULT 0, \
	display_applist INTEGER, \
	progress_size DOUBLE DEFAULT 0, \
	progress_percentage DOUBLE DEFAULT 0, \
	ongoing_flag INTEGER DEFAULT 0, \
	ongoing_value_type INTEGER DEFAULT 0, \
	ongoing_current INTEGER DEFAULT 0, \
	ongoing_duration INTEGER DEFAULT 0, \
	auto_remove INTEGER DEFAULT 1, \
	default_button_index INTEGER DEFAULT 0, \
	hide_timeout INTEGER DEFAULT 0, \
	delete_timeout INTEGER DEFAULT 0, \
	text_input_max_length INTEGER DEFAULT 0, \
	event_flag INTEGER DEFAULT 0, \
	extension_image_size INTEGER DEFAULT 0, \
	uid INTEGER, \
	template_name TEXT, \
	UNIQUE (caller_app_id, template_name) \
);"

#define NOTI_LIST_DB_ATTRIBUTES_SELECT \
	"type, layout, pkg_id, caller_app_id, launch_app_id, image_path, " \
	"priv_image_path, group_id, priv_id, b_text, b_key, " \
	"tag, b_format_args, num_format_args, text_domain, text_dir, " \
	"time, insert_time, args, group_args, b_execute_option, " \
	"b_service_responding, b_service_single_launch, b_service_multi_launch, " \
	"b_event_handler_click_on_button_1, b_event_handler_click_on_button_2, " \
	"b_event_handler_click_on_button_3, b_event_handler_click_on_button_4, " \
	"b_event_handler_click_on_button_5, b_event_handler_click_on_button_6, " \
	"b_event_handler_click_on_icon, b_event_handler_click_on_thumbnail, " \
	"b_event_handler_click_on_text_input_button, " \
	"sound_type, sound_path, priv_sound_path, " \
	"vibration_type, vibration_path, priv_vibration_path, " \
	"led_operation, led_argb, led_on_ms, led_off_ms, " \
	"flags_for_property, display_applist, " \
	"progress_size, progress_percentage, ongoing_flag, ongoing_value_type, " \
	"ongoing_current, ongoing_duration, auto_remove, default_button_index, " \
	"hide_timeout, delete_timeout, text_input_max_length, event_flag, "\
	"extension_image_size, uid"

#define NOTI_LIST_DB_ATTRIBUTES_INSERT \
	"type, layout, pkg_id, caller_app_id, launch_app_id, image_path, " \
	"priv_image_path, group_id, internal_group_id, title_key, b_text, b_key, " \
	"tag, b_format_args, num_format_args, text_domain, text_dir, " \
	"time, insert_time, args, group_args, b_execute_option, " \
	"b_service_responding, b_service_single_launch, b_service_multi_launch, " \
	"b_event_handler_click_on_button_1, b_event_handler_click_on_button_2, " \
	"b_event_handler_click_on_button_3, b_event_handler_click_on_button_4, " \
	"b_event_handler_click_on_button_5, b_event_handler_click_on_button_6, " \
	"b_event_handler_click_on_icon, b_event_handler_click_on_thumbnail, " \
	"b_event_handler_click_on_text_input_button, " \
	"sound_type, sound_path, priv_sound_path, " \
	"vibration_type, vibration_path, priv_vibration_path, " \
	"led_operation, led_argb, led_on_ms, led_off_ms, " \
	"flags_for_property, flag_simmode, display_applist, " \
	"progress_size, progress_percentage, ongoing_flag, ongoing_value_type, " \
	"ongoing_current, ongoing_duration, auto_remove, default_button_index, " \
	"hide_timeout, delete_timeout, text_input_max_length, event_flag, " \
	"extension_image_size, uid"

#define NOTI_LIST_INSERT_VALUES \
	"?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, " \
	"?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, " \
	"?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, " \
	"? "

#define NOTI_LIST_DB_ATTRIBUTES_UPDATE \
	"type = ?, layout = ?, launch_app_id = ?, image_path = ?, " \
	"priv_image_path = ?, b_text = ?, b_key = ?, tag = ?, " \
	"b_format_args = ?, num_format_args = ?, text_domain = ?, text_dir = ?, " \
	"time = ?, insert_time = ?, args = ?, group_args = ?, " \
	"b_execute_option = ?, b_service_responding = ?, " \
	"b_service_single_launch = ?, b_service_multi_launch = ?, " \
	"b_event_handler_click_on_button_1 = ?, " \
	"b_event_handler_click_on_button_2 = ?, " \
	"b_event_handler_click_on_button_3 = ?, " \
	"b_event_handler_click_on_button_4 = ?, " \
	"b_event_handler_click_on_button_5 = ?, " \
	"b_event_handler_click_on_button_6 = ?, " \
	"b_event_handler_click_on_icon = ?, " \
	"b_event_handler_click_on_thumbnail = ?, " \
	"b_event_handler_click_on_text_input_button = ?, " \
	"sound_type = ?, sound_path = ?, priv_sound_path = ?, " \
	"vibration_type = ?, vibration_path = ?, priv_vibration_path = ?, " \
	"led_operation = ?, led_argb = ?, led_on_ms = ?, led_off_ms = ?, " \
	"flags_for_property = ?, flag_simmode = ?, display_applist = ?, " \
	"progress_size = ?, progress_percentage = ?, " \
	"ongoing_flag = ?, ongoing_value_type = ?, " \
	"ongoing_current = ?, ongoing_duration = ?, " \
	"auto_remove = ?, default_button_index = ?, " \
	"hide_timeout = ?, delete_timeout = ?, " \
	"text_input_max_length = ?, event_flag = ?, extension_image_size = ?"

#define NOTIFICATION_SETTING_DB_ATTRIBUTES \
	"package_name, app_id, allow_to_notify, do_not_disturb_except, "\
	"visibility_class, pop_up_notification, lock_screen_content_level, "\
	"app_disabled"

#define NOTIFICATION_SYSTEM_SETTING_DB_ATTRIBUTES \
	"do_not_disturb, visibility_class, dnd_schedule_enabled, " \
	"dnd_schedule_day, dnd_start_hour, dnd_start_min, dnd_end_hour, " \
	"dnd_end_min, lock_screen_content_level"

#define __BIND_TEXT(db, stmt, i, text, ret, label) \
do { \
	ret = sqlite3_bind_text(stmt, i, text, -1, SQLITE_TRANSIENT); \
	if (ret != SQLITE_OK) { \
		NOTIFICATION_ERR("bind error(index %d): [%d][%s]", i, ret, \
				sqlite3_errmsg(db)); \
		ret = NOTIFICATION_ERROR_FROM_DB; \
		goto label; \
	} \
} while (0)

#define __BIND_TEXT_STATIC(db, stmt, i, text, ret, label) \
do { \
	ret = sqlite3_bind_text(stmt, i, text, -1, SQLITE_STATIC); \
	if (ret != SQLITE_OK) { \
		NOTIFICATION_ERR("bind error(index %d): [%d][%s]", i, ret, \
				sqlite3_errmsg(db)); \
		ret = NOTIFICATION_ERROR_FROM_DB; \
		goto label; \
	} \
} while (0)

#define __BIND_INT(db, stmt, i, int, ret, label) \
do { \
	ret = sqlite3_bind_int(stmt, i, int); \
	if (ret != SQLITE_OK) { \
		NOTIFICATION_ERR("bind error(index %d): [%d][%s]", i, ret, \
				sqlite3_errmsg(db)); \
		ret = NOTIFICATION_ERROR_FROM_DB; \
		goto label; \
	} \
} while (0)

#define __BIND_DOUBLE(db, stmt, i, double, ret, label) \
do { \
	ret = sqlite3_bind_double(stmt, i, double); \
	if (ret != SQLITE_OK) { \
		NOTIFICATION_ERR("bind error(index %d): [%d][%s]", i, ret, \
				sqlite3_errmsg(db)); \
		ret = NOTIFICATION_ERROR_FROM_DB; \
		goto label; \
	} \
} while (0)
