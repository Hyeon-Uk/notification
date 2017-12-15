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

#define NOTIFICATION_DB_TABLE "noti_list"
#define NOTIFICATION_SETTING_DB_TABLE "notification_setting"
#define NOTIFICATION_SYSTEM_SETTING_DB_TABLE "notification_system_setting"
#define NOTIFICATION_DND_ALLOW_EXCEPTION "dnd_allow_exception"

#define CREATE_NOTIFICATION_TABLE \
	"PRAGMA journal_mode = PERSIST;\n" \
	"PRAGMA synchronous = FULL;\n" \
	"CREATE TABLE IF NOT EXISTS noti_list (\n" \
	"  type INTEGER NOT NULL,\n" \
	"  layout INTEGER NOT NULL DEFAULT 0,\n" \
	"  pkg_id TEXT NOT NULL,\n" \
	"  caller_app_id TEXT NOT NULL,\n" \
	"  launch_app_id TEXT,\n" \
	"  app_label TEXT,\n" \
	"  image_path TEXT,\n" \
	"  priv_image_path TEXT,\n" \
	"  group_id INTEGER DEFAULT 0,\n" \
	"  internal_group_id INTEGER DEFAULT 0,\n" \
	"  priv_id INTEGER PRIMARY KEY AUTOINCREMENT,\n" \
	"  title_key TEXT,\n" \
	"  b_text TEXT,\n" \
	"  b_key TEXT,\n" \
	"  tag TEXT,\n" \
	"  b_format_args TEXT,\n" \
	"  num_format_args INTEGER DEFAULT 0,\n" \
	"  text_domain TEXT,\n" \
	"  text_dir TEXT,\n" \
	"  time INTEGER DEFAULT 0,\n" \
	"  insert_time INTEGER DEFAULT 0,\n" \
	"  args TEXT,\n" \
	"  group_args TEXT,\n" \
	"  b_execute_option TEXT,\n" \
	"  b_service_responding TEXT,\n" \
	"  b_service_single_launch TEXT,\n" \
	"  b_service_multi_launch TEXT,\n" \
	"  b_event_handler_click_on_button_1 TEXT,\n" \
	"  b_event_handler_click_on_button_2 TEXT,\n" \
	"  b_event_handler_click_on_button_3 TEXT,\n" \
	"  b_event_handler_click_on_button_4 TEXT,\n" \
	"  b_event_handler_click_on_button_5 TEXT,\n" \
	"  b_event_handler_click_on_button_6 TEXT,\n" \
	"  b_event_handler_click_on_icon TEXT,\n" \
	"  b_event_handler_click_on_thumbnail TEXT,\n" \
	"  b_event_handler_click_on_text_input_button TEXT,\n" \
	"  sound_type INTEGER DEFAULT 0,\n" \
	"  sound_path TEXT,\n" \
	"  priv_sound_path TEXT,\n" \
	"  vibration_type INTEGER DEFAULT 0,\n" \
	"  vibration_path TEXT,\n" \
	"  priv_vibration_path TEXT,\n" \
	"  led_operation INTEGER DEFAULT 0,\n" \
	"  led_argb INTEGER DEFAULT 0,\n" \
	"  led_on_ms INTEGER DEFAULT -1,\n" \
	"  led_off_ms INTEGER DEFAULT -1,\n" \
	"  flags_for_property INTEGER DEFAULT 0,\n" \
	"  flag_simmode INTEGER DEFAULT 0,\n" \
	"  display_applist INTEGER,\n" \
	"  progress_size DOUBLE DEFAULT 0,\n" \
	"  progress_percentage DOUBLE DEFAULT 0,\n" \
	"  ongoing_flag INTEGER DEFAULT 0,\n" \
	"  ongoing_value_type INTEGER DEFAULT 0,\n" \
	"  ongoing_current INTEGER DEFAULT 0,\n" \
	"  ongoing_duration INTEGER DEFAULT 0,\n" \
	"  auto_remove INTEGER DEFAULT 1,\n" \
	"  default_button_index INTEGER DEFAULT 0,\n" \
	"  hide_timeout INTEGER DEFAULT 0,\n" \
	"  delete_timeout INTEGER DEFAULT 0,\n" \
	"  text_input_max_length INTEGER DEFAULT 0,\n" \
	"  event_flag INTEGER DEFAULT 0,\n" \
	"  extension_image_size INTEGER DEFAULT 0,\n" \
	"  uid INTEGER );\n" \
	"CREATE TABLE IF NOT EXISTS noti_group_data (\n" \
	"  caller_app_id TEXT NOT NULL,\n" \
	"  group_id INTEGER DEFAULT 0,\n" \
	"  badge INTEGER DEFAULT 0,\n" \
	"  title TEXT,\n" \
	"  content TEXT,\n" \
	"  loc_title TEXT,\n" \
	"  loc_content TEXT,\n" \
	"  count_display_title INTEGER,\n" \
	"  count_display_content INTEGER,\n" \
	"  rowid INTEGER PRIMARY KEY AUTOINCREMENT,\n" \
	"  UNIQUE (caller_app_id, group_id));\n" \
	"CREATE TABLE IF NOT EXISTS ongoing_list (\n" \
	"  caller_app_id TEXT NOT NULL,\n" \
	"  launch_app_id TEXT,\n" \
	"  icon_path TEXT,\n" \
	"  group_id INTEGER DEFAULT 0,\n" \
	"  internal_group_id INTEGER DEFAULT 0,\n" \
	"  priv_id INTERGER NOT NULL,\n" \
	"  title TEXT,\n" \
	"  content TEXT,\n" \
	"  default_content TEXT,\n" \
	"  loc_title TEXT,\n" \
	"  loc_content TEXT,\n" \
	"  loc_default_content TEXT,\n" \
	"  text_domain TEXT,\n" \
	"  text_dir TEXT,\n" \
	"  args TEXT,\n" \
	"  group_args TEXT,\n" \
	"  flag INTEGER DEFAULT 0,\n" \
	"  progress_size DOUBLE DEFAULT 0,\n" \
	"  progress_percentage DOUBLE DEFAULT 0,\n" \
	"  rowid INTEGER PRIMARY KEY AUTOINCREMENT,\n" \
	"  UNIQUE (caller_app_id, priv_id));\n" \
	"CREATE TABLE IF NOT EXISTS notification_setting (\n" \
	"  uid INTEGER,\n" \
	"  package_name TEXT NOT NULL,\n" \
	"  app_id TEXT NOT NULL,\n" \
	"  allow_to_notify INTEGER DEFAULT 1,\n" \
	"  do_not_disturb_except INTEGER DEFAULT 0,\n" \
	"  visibility_class INTEGER DEFAULT 0,\n" \
	"  pop_up_notification INTEGER DEFAULT 1,\n" \
	"  lock_screen_content_level INTEGER DEFAULT 0,\n" \
	"  app_disabled INTEGER DEFAULT 0,\n" \
	"  UNIQUE (uid, package_name, app_id));\n" \
	"CREATE TABLE IF NOT EXISTS notification_system_setting (\n" \
	"  uid INTEGER,\n" \
	"  do_not_disturb INTEGER DEFAULT 0,\n" \
	"  visibility_class INTEGER DEFAULT 0,\n" \
	"  dnd_schedule_enabled INTEGER DEFAULT 0,\n" \
	"  dnd_schedule_day INTEGER DEFAULT 62,\n" \
	"  dnd_start_hour INTEGER DEFAULT 22,\n" \
	"  dnd_start_min INTEGER DEFAULT 0,\n" \
	"  dnd_end_hour INTEGER DEFAULT 8,\n" \
	"  dnd_end_min INTEGER DEFAULT 0,\n" \
	"  lock_screen_content_level INTEGER DEFAULT 0,\n" \
	"  UNIQUE (uid));\n"\
	"CREATE TABLE IF NOT EXISTS dnd_allow_exception (\n" \
	"  uid INTEGER,\n" \
	"  type INTEGER DEFAULT 0,\n" \
	"  value INTEGER DEFAULT 0,\n" \
	"  UNIQUE (uid, type));\n" \
	"CREATE TABLE IF NOT EXISTS noti_template (\n" \
	"  type INTEGER NOT NULL,\n" \
	"  layout INTEGER NOT NULL DEFAULT 0,\n" \
	"  pkg_id TEXT NOT NULL,\n" \
	"  caller_app_id TEXT NOT NULL,\n" \
	"  launch_app_id TEXT,\n" \
	"  app_label TEXT,\n" \
	"  image_path TEXT,\n" \
	"  priv_image_path TEXT,\n" \
	"  group_id INTEGER DEFAULT 0,\n" \
	"  internal_group_id INTEGER DEFAULT 0,\n" \
	"  priv_id INTEGER PRIMARY KEY AUTOINCREMENT,\n" \
	"  title_key TEXT,\n" \
	"  b_text TEXT,\n" \
	"  b_key TEXT,\n" \
	"  tag TEXT,\n" \
	"  b_format_args TEXT,\n" \
	"  num_format_args INTEGER DEFAULT 0,\n" \
	"  text_domain TEXT,\n" \
	"  text_dir TEXT,\n" \
	"  time INTEGER DEFAULT 0,\n" \
	"  insert_time INTEGER DEFAULT 0,\n" \
	"  args TEXT,\n" \
	"  group_args TEXT,\n" \
	"  b_execute_option TEXT,\n" \
	"  b_service_responding TEXT,\n" \
	"  b_service_single_launch TEXT,\n" \
	"  b_service_multi_launch TEXT,\n" \
	"  b_event_handler_click_on_button_1 TEXT,\n" \
	"  b_event_handler_click_on_button_2 TEXT,\n" \
	"  b_event_handler_click_on_button_3 TEXT,\n" \
	"  b_event_handler_click_on_button_4 TEXT,\n" \
	"  b_event_handler_click_on_button_5 TEXT,\n" \
	"  b_event_handler_click_on_button_6 TEXT,\n" \
	"  b_event_handler_click_on_icon TEXT,\n" \
	"  b_event_handler_click_on_thumbnail TEXT,\n" \
	"  b_event_handler_click_on_text_input_button TEXT,\n" \
	"  sound_type INTEGER DEFAULT 0,\n" \
	"  sound_path TEXT,\n" \
	"  priv_sound_path TEXT,\n" \
	"  vibration_type INTEGER DEFAULT 0,\n" \
	"  vibration_path TEXT,\n" \
	"  priv_vibration_path TEXT,\n" \
	"  led_operation INTEGER DEFAULT 0,\n" \
	"  led_argb INTEGER DEFAULT 0,\n" \
	"  led_on_ms INTEGER DEFAULT -1,\n" \
	"  led_off_ms INTEGER DEFAULT -1,\n" \
	"  flags_for_property INTEGER DEFAULT 0,\n" \
	"  flag_simmode INTEGER DEFAULT 0,\n" \
	"  display_applist INTEGER,\n" \
	"  progress_size DOUBLE DEFAULT 0,\n" \
	"  progress_percentage DOUBLE DEFAULT 0,\n" \
	"  ongoing_flag INTEGER DEFAULT 0,\n" \
	"  ongoing_value_type INTEGER DEFAULT 0,\n" \
	"  ongoing_current INTEGER DEFAULT 0,\n" \
	"  ongoing_duration INTEGER DEFAULT 0,\n" \
	"  auto_remove INTEGER DEFAULT 1,\n" \
	"  default_button_index INTEGER DEFAULT 0,\n" \
	"  hide_timeout INTEGER DEFAULT 0,\n" \
	"  delete_timeout INTEGER DEFAULT 0,\n" \
	"  text_input_max_length INTEGER DEFAULT 0,\n" \
	"  event_flag INTEGER DEFAULT 0,\n" \
	"  extension_image_size INTEGER DEFAULT 0,\n" \
	"  uid INTEGER,\n" \
	"  template_name TEXT,\n" \
	"  UNIQUE (caller_app_id, template_name));\n"

#define NOTI_LIST_DB_ATTRIBUTES_SELECT \
	"type, layout, pkg_id, caller_app_id, launch_app_id, app_label, "\
	"image_path, priv_image_path, group_id, priv_id, b_text, b_key, " \
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
	"type, layout, pkg_id, caller_app_id, launch_app_id, app_label, " \
	"image_path, priv_image_path, group_id, internal_group_id, title_key, " \
	"b_text, b_key, tag, b_format_args, num_format_args, text_domain, " \
	"text_dir, time, insert_time, args, group_args, b_execute_option, " \
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
	"?, ? "

#define NOTI_LIST_DB_ATTRIBUTES_UPDATE \
	"type = ?, layout = ?, launch_app_id = ?, app_label = ?, " \
	"image_path = ?, priv_image_path = ?, b_text = ?, b_key = ?, tag = ?, " \
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
	"text_input_max_length = ?, event_flag = ?, extension_image_size = ? "

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
