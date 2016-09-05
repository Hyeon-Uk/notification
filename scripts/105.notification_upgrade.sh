#!/bin/sh

#------------------------------------------#
# notification patch for upgrade (2.4 -> 3.0) #
#------------------------------------------#

# Macro
PATH=/bin:/usr/bin:/sbin:/usr/sbin

DB_DIR=/opt/dbspace
DB_NOTIFICATION=$DB_DIR/.notification.db

sqlite3 $DB_NOTIFICATION << EOF

DROP TABLE IF EXISTS noti_list_temp;
CREATE TABLE noti_list_temp (
	type INTEGER NOT NULL,
	layout INTEGER NOT NULL default 0,
	caller_pkgname TEXT NOT NULL,
	launch_pkgname TEXT,
	image_path TEXT,
	group_id INTEGER default 0,
	internal_group_id INTEGER default 0,
	priv_id INTEGER PRIMARY KEY AUTOINCREMENT,
	title_key TEXT,
	b_text TEXT,
	b_key TEXT,
	tag TEXT,
	b_format_args TEXT,
	num_format_args INTEGER default 0,
	text_domain TEXT,
	text_dir TEXT,
	time INTEGER default 0,
	insert_time INTEGER default 0,
	args TEXT,
	group_args TEXT,
	b_execute_option TEXT,
	b_service_responding TEXT,
	b_service_single_launch TEXT,
	b_service_multi_launch TEXT,
	b_event_handler_click_on_button_1 TEXT,
	b_event_handler_click_on_button_2 TEXT,
	b_event_handler_click_on_button_3 TEXT,
	b_event_handler_click_on_button_4 TEXT,
	b_event_handler_click_on_button_5 TEXT,
	b_event_handler_click_on_button_6 TEXT,
	b_event_handler_click_on_icon TEXT,
	b_event_handler_click_on_thumbnail TEXT,
	sound_type INTEGER default 0,
	sound_path TEXT,
	vibration_type INTEGER default 0,
	vibration_path TEXT,
	led_operation INTEGER default 0,
	led_argb INTEGER default 0,
	led_on_ms INTEGER default -1,
	led_off_ms INTEGER default -1,
	flags_for_property INTEGER default 0,
	flag_simmode INTEGER default 0,
	display_applist INTEGER,
	progress_size DOUBLE default 0,
	progress_percentage DOUBLE default 0,
	ongoing_flag INTEGER default 0,
	auto_remove INTEGER default 1,
	uid INTEGER
);
INSERT INTO noti_list_temp (type, layout, caller_pkgname, launch_pkgname, image_path, group_id, internal_group_id, priv_id, title_key, b_text, b_key, tag, b_format_args, num_format_args, text_domain, text_dir, time, insert_time, args, group_args, b_execute_option, b_service_responding, b_service_single_launch, b_service_multi_launch, b_event_handler_click_on_button_1, b_event_handler_click_on_button_2, b_event_handler_click_on_button_3, b_event_handler_click_on_button_4, b_event_handler_click_on_button_5, b_event_handler_click_on_button_6, b_event_handler_click_on_icon, b_event_handler_click_on_thumbnail, sound_type, sound_path, vibration_type, vibration_path, led_operation, led_argb, led_on_ms, led_off_ms, flags_for_property, flag_simmode, display_applist, progress_size, progress_percentage, ongoing_flag, auto_remove) \
SELECT type, layout, caller_pkgname, launch_pkgname, image_path, group_id, internal_group_id, priv_id, title_key, b_text, b_key, tag, b_format_args, num_format_args, text_domain, text_dir, time, insert_time, args, group_args, b_execute_option, b_service_responding, b_service_single_launch, b_service_multi_launch, b_event_handler_click_on_button_1, b_event_handler_click_on_button_2, b_event_handler_click_on_button_3, b_event_handler_click_on_button_4, b_event_handler_click_on_button_5, b_event_handler_click_on_button_6, b_event_handler_click_on_icon, b_event_handler_click_on_thumbnail, sound_type, sound_path, vibration_type, vibration_path, led_operation, led_argb, led_on_ms, led_off_ms, flags_for_property, flag_simmode, display_applist, progress_size, progress_percentage, ongoing_flag, auto_remove FROM noti_list;
UPDATE noti_list_temp SET uid=5001;
DROP TABLE noti_list;
ALTER TABLE noti_list_temp RENAME TO noti_list;

DROP TABLE IF EXISTS notification_setting_temp;
CREATE TABLE notification_setting_temp (
	uid INTEGER,
	package_name TEXT NOT NULL,
	allow_to_notify INTEGER DEFAULT 1,
	do_not_disturb_except INTEGER DEFAULT 0,
	visibility_class INTEGER DEFAULT 0,
	pop_up_notification INTEGER DEFAULT 1,
	lock_screen_content_level INTEGER DEFAULT 0,
	UNIQUE (uid, package_name)
);
INSERT INTO notification_setting_temp (package_name, allow_to_notify, do_not_disturb_except, visibility_class) \
SELECT package_name, allow_to_notify, do_not_disturb_except, visibility_class FROM notification_setting;
UPDATE notification_setting_temp SET uid=5001;
DROP TABLE notification_setting;
ALTER TABLE notification_setting_temp RENAME TO notification_setting;


DROP TABLE IF EXISTS notification_system_setting_temp;
CREATE TABLE notification_system_setting_temp (
	uid INTEGER,
	do_not_disturb INTEGER DEFAULT 0,
	visibility_class INTEGER DEFAULT 0,
	dnd_schedule_enabled INTEGER DEFAULT 0,
	dnd_schedule_day INTEGER DEFAULT 0,
	dnd_start_hour INTEGER DEFAULT 0,
	dnd_start_min INTEGER DEFAULT 0,
	dnd_end_hour INTEGER DEFAULT 0,
	dnd_end_min INTEGER DEFAULT 0,
	lock_screen_content_level INTEGER DEFAULT 0,
	UNIQUE (uid)
);
INSERT INTO notification_system_setting_temp (do_not_disturb, visibility_class) \
SELECT do_not_disturb, visibility_class FROM notification_system_setting;
UPDATE notification_system_setting_temp SET uid=5001;
DROP TABLE notification_system_setting;
ALTER TABLE notification_system_setting_temp RENAME TO notification_system_setting;

DROP TABLE IF EXISTS noti_template;
CREATE TABLE noti_template (
	type INTEGER NOT NULL,
	layout INTEGER NOT NULL default 0,
	caller_pkgname TEXT NOT NULL,
	launch_pkgname TEXT,
	image_path TEXT,
	group_id INTEGER default 0,
	internal_group_id INTEGER default 0,
	priv_id INTEGER PRIMARY KEY AUTOINCREMENT,
	title_key TEXT,
	b_text TEXT,
	b_key TEXT,
	tag TEXT,
	b_format_args TEXT,
	num_format_args INTEGER default 0,
	text_domain TEXT,
	text_dir TEXT,
	time INTEGER default 0,
	insert_time INTEGER default 0,
	args TEXT,
	group_args TEXT,
	b_execute_option TEXT,
	b_service_responding TEXT,
	b_service_single_launch TEXT,
	b_service_multi_launch TEXT,
	b_event_handler_click_on_button_1 TEXT,
	b_event_handler_click_on_button_2 TEXT,
	b_event_handler_click_on_button_3 TEXT,
	b_event_handler_click_on_button_4 TEXT,
	b_event_handler_click_on_button_5 TEXT,
	b_event_handler_click_on_button_6 TEXT,
	b_event_handler_click_on_icon TEXT,
	b_event_handler_click_on_thumbnail TEXT,
	sound_type INTEGER default 0,
	sound_path TEXT,
	vibration_type INTEGER default 0,
	vibration_path TEXT,
	led_operation INTEGER default 0,
	led_argb INTEGER default 0,
	led_on_ms INTEGER default -1,
	led_off_ms INTEGER default -1,
	flags_for_property INTEGER default 0,
	flag_simmode INTEGER default 0,
	display_applist INTEGER,
	progress_size DOUBLE default 0,
	progress_percentage DOUBLE default 0,
	ongoing_flag INTEGER default 0,
	auto_remove INTEGER default 1,
	uid INTEGER,
	template_name TEXT,
	UNIQUE (caller_pkgname, template_name)
);
EOF

chown owner:users $DB_NOTIFICATION
chown owner:users $DB_NOTIFICATION-journal

chsmack -a User::Home $DB_NOTIFICATION
chsmack -a User::Home $DB_NOTIFICATION-journal
