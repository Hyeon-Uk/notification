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

#include <notification_tidl_proxy.h>


static const char *__get_str_value(char *value)
{
	if (value != NULL)
		return (const char *)value;

	return "";
}

/*
 * for TIDL
 */
EXPORT_API int make_empty_notification(void *notihandle)
{
	int ret;
	bundle *empty_bundle;

	if (notihandle == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	empty_bundle = bundle_create();

	WARN("make_empty_notification");

	rpc_port_proxy_notification_h handle = (rpc_port_proxy_notification_h)notihandle;

	ret = rpc_port_proxy_notification_set_type(handle, -1);
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_set_type : %d", ret);

	ret = rpc_port_proxy_notification_set_layout(handle, -1);
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_set_layout : %d", ret);

	ret = rpc_port_proxy_notification_set_group_id(handle, -1);
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_set_group_id : %d", ret);

	ret = rpc_port_proxy_notification_set_internal_group_id(handle, -1);
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_set_internal_group_id : %d", ret);

	ret = rpc_port_proxy_notification_set_priv_id(handle, -1);
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_set_priv_id : %d", ret);

	ret = rpc_port_proxy_notification_set_pkg_id(handle, "");
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_set_pkg_id : %d", ret);

	ret = rpc_port_proxy_notification_set_caller_app_id(handle, "");
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_set_caller_app_id : %d", ret);

	ret = rpc_port_proxy_notification_set_launch_app_id(handle, "");
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_set_launch_app_id : %d", ret);

	ret = rpc_port_proxy_notification_set_args(handle, empty_bundle);
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_set_args : %d", ret);

	ret = rpc_port_proxy_notification_set_group_args(handle, empty_bundle);
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_set_group_args : %d", ret);

	ret = rpc_port_proxy_notification_set_execute_option(handle, empty_bundle);
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_set_execute_option : %d", ret);

	ret = rpc_port_proxy_notification_set_service_responding(handle, empty_bundle);
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_set_service_responding : %d", ret);

	ret = rpc_port_proxy_notification_set_service_single_launch(handle, empty_bundle);
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_set_service_single_launch : %d", ret);

	ret = rpc_port_proxy_notification_set_service_multi_launch(handle, empty_bundle);
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_set_service_multi_launch : %d", ret);

// start
	rpc_port_proxy_array_bundle_h _event_handler;
	rpc_port_proxy_array_bundle_create(&_event_handler);

	ret = rpc_port_proxy_notification_set_event_handler(handle, _event_handler);
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_set_event_handler : %d", ret);

	rpc_port_proxy_array_bundle_destroy(_event_handler);
// end

	ret = rpc_port_proxy_notification_set_domain(handle, "");
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_set_domain : %d", ret);

	ret = rpc_port_proxy_notification_set_dir(handle, "");
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_set_dir : %d", ret);

	ret = rpc_port_proxy_notification_set_text(handle, empty_bundle);
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_set_text : %d", ret);

	ret = rpc_port_proxy_notification_set_key(handle, empty_bundle);
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_set_key : %d", ret);

	ret = rpc_port_proxy_notification_set_format_args(handle, empty_bundle);
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_set_format_args : %d", ret);

	ret = rpc_port_proxy_notification_set_num_format_args(handle, -1);
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_set_num_format_args : %d", ret);

	ret = rpc_port_proxy_notification_set_image_path(handle, empty_bundle);
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_set_image_path : %d", ret);

	ret = rpc_port_proxy_notification_set_priv_image_path(handle, empty_bundle);
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_set_priv_image_path : %d", ret);

	ret = rpc_port_proxy_notification_set_sound_type(handle, -1);
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_set_sound_type : %d", ret);

	ret = rpc_port_proxy_notification_set_sound_path(handle, "");
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_set_sound_path : %d", ret);

	ret = rpc_port_proxy_notification_set_priv_sound_path(handle, "");
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_set_priv_sound_path : %d", ret);

	ret = rpc_port_proxy_notification_set_vibration_type(handle, -1);
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_set_vibration_type : %d", ret);

	ret = rpc_port_proxy_notification_set_vibration_path(handle, "");
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_set_vibration_path : %d", ret);

	ret = rpc_port_proxy_notification_set_priv_vibration_path(handle, "");
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_set_priv_vibration_path : %d", ret);

	ret = rpc_port_proxy_notification_set_led_operation(handle, -1);
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_set_led_operation : %d", ret);

	ret = rpc_port_proxy_notification_set_led_argb(handle, -1);
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_set_led_argb : %d", ret);

	ret = rpc_port_proxy_notification_set_led_on_ms(handle, -1);
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_set_led_on_ms : %d", ret);

	ret = rpc_port_proxy_notification_set_led_off_ms(handle, -1);
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_set_led_off_ms : %d", ret);

	ret = rpc_port_proxy_notification_set_time(handle, -1);
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_set_time : %d", ret);

	ret = rpc_port_proxy_notification_set_insert_time(handle, -1);
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_set_insert_time : %d", ret);

	ret = rpc_port_proxy_notification_set_flags_for_property(handle, -1);
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_set_flags_for_property : %d", ret);

	ret = rpc_port_proxy_notification_set_display_applist(handle, -1);
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_set_display_applist : %d", ret);

	ret = rpc_port_proxy_notification_set_progress_size(handle, 0.0);
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_set_progress_size : %d", ret);

	ret = rpc_port_proxy_notification_set_progress_percentage(handle, 0.0);
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_set_progress_percentage : %d", ret);

	ret = rpc_port_proxy_notification_set_app_icon_path(handle, "");
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_set_app_icon_path : %d", ret);

	ret = rpc_port_proxy_notification_set_app_label(handle, "");
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_set_app_label : %d", ret);

	ret = rpc_port_proxy_notification_set_temp_title(handle, "");
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_set_temp_title : %d", ret);

	ret = rpc_port_proxy_notification_set_temp_content(handle, "");
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_set_temp_content : %d", ret);

	ret = rpc_port_proxy_notification_set_tag(handle, "");
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_set_tag : %d", ret);

	ret = rpc_port_proxy_notification_set_ongoing_flag(handle, false);
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_set_ongoing_flag : %d", ret);

	ret = rpc_port_proxy_notification_set_ongoing_value_type(handle, -1);
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_set_ongoing_value_type : %d", ret);

	ret = rpc_port_proxy_notification_set_ongoing_current(handle, -1);
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_set_ongoing_current : %d", ret);

	ret = rpc_port_proxy_notification_set_ongoing_duration(handle, -1);
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_set_ongoing_duration : %d", ret);

	ret = rpc_port_proxy_notification_set_auto_remove(handle, false);
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_set_auto_remove : %d", ret);

	ret = rpc_port_proxy_notification_set_default_button_index(handle, -1);
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_set_default_button_index : %d", ret);

	ret = rpc_port_proxy_notification_set_hide_timeout(handle, -1);
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_set_hide_timeout : %d", ret);

	ret = rpc_port_proxy_notification_set_delete_timeout(handle, -1);
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_set_delete_timeout : %d", ret);

	ret = rpc_port_proxy_notification_set_text_input_max_length(handle, -1);
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_set_text_input_max_length : %d", ret);

	ret = rpc_port_proxy_notification_set_event_flag(handle, false);
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_set_event_flag : %d", ret);

	ret = rpc_port_proxy_notification_set_is_translation(handle, false);
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_set_is_translation : %d", ret);

	ret = rpc_port_proxy_notification_set_extension_image_size(handle, -1);
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_set_extension_image_size : %d", ret);

	ret = rpc_port_proxy_notification_set_check_box(handle, false);
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_set_check_box : %d", ret);

	ret = rpc_port_proxy_notification_set_check_box_value(handle, false);
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_set_check_box_value : %d", ret);

	ret = rpc_port_proxy_notification_set_channel_name(handle, "");
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_set_channel_name : %d", ret);

	ret = rpc_port_proxy_notification_set_uid(handle, -1);
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_set_uid : %d", ret);

	bundle_free(empty_bundle);

	WARN("make_empty_notification done");

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int make_notification_from_noti(void *notihandle,
		notification_h noti, bool translate)
{
	int ret;
	bundle *empty_bundle;

	if (notihandle == NULL || noti == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	empty_bundle = bundle_create();

	WARN("make_notification_from_noti");

	rpc_port_proxy_notification_h handle = (rpc_port_proxy_notification_h)notihandle;

	if (translate)
		notification_translate_localized_text(noti);

	ret = rpc_port_proxy_notification_set_type(handle, (int)noti->type);
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_set_type : %d", ret);

	ret = rpc_port_proxy_notification_set_layout(handle, (int)noti->layout);
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_set_layout : %d", ret);

	ret = rpc_port_proxy_notification_set_group_id(handle, noti->group_id);
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_set_group_id : %d", ret);

	ret = rpc_port_proxy_notification_set_internal_group_id(handle, noti->internal_group_id);
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_set_internal_group_id : %d", ret);

	ret = rpc_port_proxy_notification_set_priv_id(handle, noti->priv_id);
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_set_priv_id : %d", ret);

	ret = rpc_port_proxy_notification_set_pkg_id(handle, __get_str_value(noti->pkg_id));
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_set_pkg_id : %d", ret);

	ret = rpc_port_proxy_notification_set_caller_app_id(handle, __get_str_value(noti->caller_app_id));
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_set_caller_app_id : %d", ret);

	ret = rpc_port_proxy_notification_set_launch_app_id(handle, __get_str_value(noti->launch_app_id));
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_set_launch_app_id : %d", ret);

	ret = rpc_port_proxy_notification_set_args(handle,
			noti->args ? noti->args : empty_bundle);
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_set_args : %d", ret);

	ret = rpc_port_proxy_notification_set_group_args(handle,
			noti->group_args ? noti->group_args : empty_bundle);
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_set_group_args : %d", ret);

	ret = rpc_port_proxy_notification_set_execute_option(handle,
			noti->b_execute_option ? noti->b_execute_option : empty_bundle);
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_set_execute_option : %d", ret);

	ret = rpc_port_proxy_notification_set_service_responding(handle,
			noti->b_service_responding ? noti->b_service_responding : empty_bundle);
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_set_service_responding : %d", ret);

	ret = rpc_port_proxy_notification_set_service_single_launch(handle,
			noti->b_service_single_launch ? noti->b_service_single_launch : empty_bundle);
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_set_service_single_launch : %d", ret);

	ret = rpc_port_proxy_notification_set_service_multi_launch(handle,
			noti->b_service_multi_launch ? noti->b_service_multi_launch : empty_bundle);
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_set_service_multi_launch : %d", ret);

// start
	rpc_port_proxy_array_bundle_h _event_handler;
	rpc_port_proxy_array_bundle_create(&_event_handler);

	for (int i = 0; i < NOTIFICATION_EVENT_TYPE_MAX+1; i++) {
		// should be changed ?
		if (noti->b_event_handler[i] == NULL)
			noti->b_event_handler[i] = bundle_create();
	}

	rpc_port_proxy_array_bundle_set(_event_handler, noti->b_event_handler, NOTIFICATION_EVENT_TYPE_MAX+1);

	ret = rpc_port_proxy_notification_set_event_handler(handle, _event_handler);
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_set_event_handler : %d", ret);

	rpc_port_proxy_array_bundle_destroy(_event_handler);
// end

	ret = rpc_port_proxy_notification_set_domain(handle, __get_str_value(noti->domain));
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_set_domain : %d", ret);

	ret = rpc_port_proxy_notification_set_dir(handle, __get_str_value(noti->dir));
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_set_dir : %d", ret);

	ret = rpc_port_proxy_notification_set_text(handle,
			noti->b_text ? noti->b_text : empty_bundle);
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_set_text : %d", ret);

	ret = rpc_port_proxy_notification_set_key(handle,
			noti->b_key ? noti->b_key : empty_bundle);
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_set_key : %d", ret);

	ret = rpc_port_proxy_notification_set_format_args(handle,
			noti->b_format_args ? noti->b_format_args : empty_bundle);
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_set_format_args : %d", ret);

	ret = rpc_port_proxy_notification_set_num_format_args(handle, noti->num_format_args);
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_set_num_format_args : %d", ret);

	ret = rpc_port_proxy_notification_set_image_path(handle,
			noti->b_image_path ? noti->b_image_path : empty_bundle);
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_set_image_path : %d", ret);

	ret = rpc_port_proxy_notification_set_priv_image_path(handle,
			noti->b_priv_image_path ? noti->b_priv_image_path : empty_bundle);
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_set_priv_image_path : %d", ret);

	ret = rpc_port_proxy_notification_set_sound_type(handle, (int)noti->sound_type);
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_set_sound_type : %d", ret);

	ret = rpc_port_proxy_notification_set_sound_path(handle, __get_str_value(noti->sound_path));
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_set_sound_path : %d", ret);

	ret = rpc_port_proxy_notification_set_priv_sound_path(handle, __get_str_value(noti->priv_sound_path));
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_set_priv_sound_path : %d", ret);

	ret = rpc_port_proxy_notification_set_vibration_type(handle, (int)noti->vibration_type);
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_set_vibration_type : %d", ret);

	ret = rpc_port_proxy_notification_set_vibration_path(handle, __get_str_value(noti->vibration_path));
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_set_vibration_path : %d", ret);

	ret = rpc_port_proxy_notification_set_priv_vibration_path(handle, __get_str_value(noti->priv_vibration_path));
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_set_priv_vibration_path : %d", ret);

	ret = rpc_port_proxy_notification_set_led_operation(handle, (int)noti->led_operation);
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_set_led_operation : %d", ret);

	ret = rpc_port_proxy_notification_set_led_argb(handle, noti->led_argb);
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_set_led_argb : %d", ret);

	ret = rpc_port_proxy_notification_set_led_on_ms(handle, noti->led_on_ms);
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_set_led_on_ms : %d", ret);

	ret = rpc_port_proxy_notification_set_led_off_ms(handle, noti->led_off_ms);
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_set_led_off_ms : %d", ret);

	ret = rpc_port_proxy_notification_set_time(handle, (long long)noti->time);
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_set_time : %d", ret);

	ret = rpc_port_proxy_notification_set_insert_time(handle, (long long)noti->insert_time);
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_set_insert_time : %d", ret);

	ret = rpc_port_proxy_notification_set_flags_for_property(handle, noti->flags_for_property);
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_set_flags_for_property : %d", ret);

	ret = rpc_port_proxy_notification_set_display_applist(handle, noti->display_applist);
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_set_display_applist : %d", ret);

	ret = rpc_port_proxy_notification_set_progress_size(handle, noti->progress_size);
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_set_progress_size : %d", ret);

	ret = rpc_port_proxy_notification_set_progress_percentage(handle, noti->progress_percentage);
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_set_progress_percentage : %d", ret);

	ret = rpc_port_proxy_notification_set_app_icon_path(handle, __get_str_value(noti->app_icon_path));
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_set_app_icon_path : %d", ret);

	ret = rpc_port_proxy_notification_set_app_label(handle, __get_str_value(noti->app_label));
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_set_app_label : %d", ret);

	ret = rpc_port_proxy_notification_set_temp_title(handle, __get_str_value(noti->temp_title));
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_set_temp_title : %d", ret);

	ret = rpc_port_proxy_notification_set_temp_content(handle, __get_str_value(noti->temp_content));
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_set_temp_content : %d", ret);

	ret = rpc_port_proxy_notification_set_tag(handle, __get_str_value(noti->tag));
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_set_tag : %d", ret);

	ret = rpc_port_proxy_notification_set_ongoing_flag(handle, noti->ongoing_flag);
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_set_ongoing_flag : %d", ret);

	ret = rpc_port_proxy_notification_set_ongoing_value_type(handle, noti->ongoing_value_type);
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_set_ongoing_value_type : %d", ret);

	ret = rpc_port_proxy_notification_set_ongoing_current(handle, noti->ongoing_current);
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_set_ongoing_current : %d", ret);

	ret = rpc_port_proxy_notification_set_ongoing_duration(handle, noti->ongoing_duration);
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_set_ongoing_duration : %d", ret);

	ret = rpc_port_proxy_notification_set_auto_remove(handle, noti->auto_remove);
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_set_auto_remove : %d", ret);

	ret = rpc_port_proxy_notification_set_default_button_index(handle, (int)noti->default_button_index);
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_set_default_button_index : %d", ret);

	ret = rpc_port_proxy_notification_set_hide_timeout(handle, noti->hide_timeout);
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_set_hide_timeout : %d", ret);

	ret = rpc_port_proxy_notification_set_delete_timeout(handle, noti->delete_timeout);
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_set_delete_timeout : %d", ret);

	ret = rpc_port_proxy_notification_set_text_input_max_length(handle, noti->text_input_max_length);
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_set_text_input_max_length : %d", ret);

	ret = rpc_port_proxy_notification_set_event_flag(handle, noti->event_flag);
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_set_event_flag : %d", ret);

	ret = rpc_port_proxy_notification_set_is_translation(handle, noti->is_translation);
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_set_is_translation : %d", ret);

	ret = rpc_port_proxy_notification_set_extension_image_size(handle, noti->extension_image_size);
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_set_extension_image_size : %d", ret);

	ret = rpc_port_proxy_notification_set_check_box(handle, noti->check_box);
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_set_check_box : %d", ret);

	ret = rpc_port_proxy_notification_set_check_box_value(handle, noti->check_box_value);
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_set_check_box_value : %d", ret);

	ret = rpc_port_proxy_notification_set_channel_name(handle, __get_str_value(noti->channel_name));
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_set_channel_name : %d", ret);

	ret = rpc_port_proxy_notification_set_uid(handle, (int)noti->uid);
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_set_uid : %d", ret);

	bundle_free(empty_bundle);

	WARN("make_notification_from_noti done");

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int make_noti_from_notification(notification_h *noti, void *notihandle)
{
	notification_h _noti;
	int ret;

	if (noti == NULL || notihandle == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	WARN("make_noti_from_notification");

	rpc_port_proxy_notification_h handle = (rpc_port_proxy_notification_h)notihandle;

	_noti = (notification_h)calloc(1, sizeof(struct _notification));
	if (_noti == NULL) {
		ERR("Failed to alloc memory");
		return NOTIFICATION_ERROR_OUT_OF_MEMORY;
	}

	int _type;
	ret = rpc_port_proxy_notification_get_type(handle, &_type);
	_noti->type = (notification_type_e)_type;
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_get_type : %d", ret);

	int _layout;
	ret = rpc_port_proxy_notification_get_layout(handle, &_layout);
	_noti->layout = (notification_ly_type_e)_layout;
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_get_layout : %d", ret);

	ret = rpc_port_proxy_notification_get_group_id(handle, &_noti->group_id);
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_get_group_id : %d", ret);

	ret = rpc_port_proxy_notification_get_internal_group_id(handle, &_noti->internal_group_id);
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_get_internal_group_id : %d", ret);

	ret = rpc_port_proxy_notification_get_priv_id(handle, &_noti->priv_id);
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_get_priv_id : %d", ret);

	char *pkg_id;
	ret = rpc_port_proxy_notification_get_pkg_id(handle, &pkg_id);
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_get_pkg_id : %d", ret);

	if (strlen(pkg_id) > 0) {
		_noti->pkg_id = pkg_id;
	} else {
		_noti->pkg_id = NULL;
		free(pkg_id);
	}

	char *caller_app_id;
	ret = rpc_port_proxy_notification_get_caller_app_id(handle, &caller_app_id);
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_get_caller_app_id : %d", ret);

	if (strlen(caller_app_id) > 0) {
		_noti->caller_app_id = caller_app_id;
	} else {
		_noti->caller_app_id = NULL;
		free(caller_app_id);
	}

	char *launch_app_id;
	ret = rpc_port_proxy_notification_get_launch_app_id(handle, &launch_app_id);
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_get_launch_app_id : %d", ret);

	if (strlen(launch_app_id) > 0) {
		_noti->launch_app_id = launch_app_id;
	} else{
		_noti->launch_app_id = NULL;
		free(launch_app_id);
	}

	bundle *args;
	ret = rpc_port_proxy_notification_get_args(handle, &args);
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_get_args : %d", ret);

	if (bundle_get_count(args) > 0) {
		_noti->args = args;
	} else {
		_noti->args = NULL;
		bundle_free(args);
	}

	bundle *group_args;
	ret = rpc_port_proxy_notification_get_group_args(handle, &group_args);
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_get_group_args : %d", ret);

	if (bundle_get_count(group_args) > 0) {
		_noti->group_args = group_args;
	} else {
		_noti->group_args = NULL;
		bundle_free(group_args);
	}

	bundle *b_execute_option;
	ret = rpc_port_proxy_notification_get_execute_option(handle, &b_execute_option);
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_get_execute_option : %d", ret);

	if (bundle_get_count(b_execute_option) > 0) {
		_noti->b_execute_option = b_execute_option;
	} else {
		_noti->b_execute_option = NULL;
		bundle_free(b_execute_option);
	}

	bundle *b_service_responding;
	ret = rpc_port_proxy_notification_get_service_responding(handle, &b_service_responding);
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_get_service_responding : %d", ret);

	if (bundle_get_count(b_service_responding) > 0) {
		_noti->b_service_responding = b_service_responding;
	} else {
		_noti->b_service_responding = NULL;
		bundle_free(b_service_responding);
	}

	bundle *b_service_single_launch;
	ret = rpc_port_proxy_notification_get_service_single_launch(handle, &b_service_single_launch);
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_get_service_single_launch : %d", ret);

	if (bundle_get_count(b_service_single_launch) > 0) {
		_noti->b_service_single_launch = b_service_single_launch;
	} else {
		_noti->b_service_single_launch = NULL;
		bundle_free(b_service_single_launch);
	}

	bundle *b_service_multi_launch;
	ret = rpc_port_proxy_notification_get_service_multi_launch(handle, &b_service_multi_launch);
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_get_service_multi_launch : %d", ret);

	if (bundle_get_count(b_service_multi_launch) > 0) {
		_noti->b_service_multi_launch = b_service_multi_launch;
	} else {
		_noti->b_service_multi_launch = NULL;
		bundle_free(b_service_multi_launch);
	}

// start
	rpc_port_proxy_array_bundle_h _event_handler;
	ret = rpc_port_proxy_notification_get_event_handler(handle, &_event_handler);
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_get_event_handler : %d", ret);

	WARN("make_noti_from_notification event handler");

	int bundle_size;
	bundle **_b_event_handler;

	if (_event_handler) {
		ret = rpc_port_proxy_array_bundle_get(_event_handler, &_b_event_handler, &bundle_size);
		WARN("make_noti_from_notification size : %d", bundle_size);

		if (ret != RPC_PORT_ERROR_NONE) {
			WARN("rpc_port_proxy_array_bundle_get failed : %d", ret);
		} else {
			for (int i = 0; i < bundle_size; i++) {
				if (_b_event_handler[i] == NULL) {
					WARN("make_noti_from_notification event handler : %d null", i);
				} else {
					int __count = 0;
					__count = bundle_get_count(_b_event_handler[i]);

					if (__count > 0)
						_noti->b_event_handler[i] = _b_event_handler[i];
					else
						_noti->b_event_handler[i] = NULL;
				}
			}
		}
		rpc_port_proxy_array_bundle_destroy(_event_handler);
	} else {
		WARN("make_noti_from_notification event handler null");
	}
// end

	char *domain;
	ret = rpc_port_proxy_notification_get_domain(handle, &domain);
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_get_domain : %d", ret);

	if (strlen(domain) > 0) {
		_noti->domain = domain;
	} else {
		_noti->domain = NULL;
		free(domain);
	}

	char *dir;
	ret = rpc_port_proxy_notification_get_dir(handle, &dir);
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_get_dir : %d", ret);

	if (strlen(dir) > 0) {
		_noti->dir = dir;
	} else {
		_noti->dir = NULL;
		free(dir);
	}

	bundle *b_text;
	ret = rpc_port_proxy_notification_get_text(handle, &b_text);
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_get_text : %d", ret);

	if (bundle_get_count(b_text) > 0) {
		_noti->b_text = b_text;
	} else {
		_noti->b_text = NULL;
		bundle_free(b_text);
	}

	bundle *b_key;
	ret = rpc_port_proxy_notification_get_key(handle, &b_key);
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_get_key : %d", ret);

	if (bundle_get_count(b_key) > 0) {
		_noti->b_key = b_key;
	} else {
		_noti->b_key = NULL;
		bundle_free(b_key);
	}

	bundle *b_format_args;
	ret = rpc_port_proxy_notification_get_format_args(handle, &b_format_args);
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_get_format_args : %d", ret);

	if (bundle_get_count(b_format_args) > 0) {
		_noti->b_format_args = b_format_args;
	} else {
		_noti->b_format_args = NULL;
		bundle_free(b_format_args);
	}

	ret = rpc_port_proxy_notification_get_num_format_args(handle, &_noti->num_format_args);
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_get_num_format_args : %d", ret);

	bundle *b_image_path;
	ret = rpc_port_proxy_notification_get_image_path(handle, &b_image_path);
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_get_image_path : %d", ret);

	if (bundle_get_count(b_image_path) > 0) {
		_noti->b_image_path = b_image_path;
	} else {
		_noti->b_image_path = NULL;
		bundle_free(b_image_path);
	}

	bundle *b_priv_image_path;
	ret = rpc_port_proxy_notification_get_priv_image_path(handle, &b_priv_image_path);
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_get_priv_image_path : %d", ret);

	if (bundle_get_count(b_priv_image_path) > 0) {
		_noti->b_priv_image_path = b_priv_image_path;
	} else {
		_noti->b_priv_image_path = NULL;
		bundle_free(b_priv_image_path);
	}

	int _sound_type;
	ret = rpc_port_proxy_notification_get_sound_type(handle, &_sound_type);
	_noti->sound_type = (notification_sound_type_e)_sound_type;
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_get_sound_type : %d", ret);

	char *sound_path;
	ret = rpc_port_proxy_notification_get_sound_path(handle, &sound_path);
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_get_sound_path : %d", ret);

	if (strlen(sound_path) > 0) {
		_noti->sound_path = sound_path;
	} else {
		_noti->sound_path = NULL;
		free(sound_path);
	}

	char *priv_sound_path;
	ret = rpc_port_proxy_notification_get_priv_sound_path(handle, &priv_sound_path);
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_get_priv_sound_path : %d", ret);

	if (strlen(priv_sound_path) > 0) {
		_noti->priv_sound_path = priv_sound_path;
	} else {
		_noti->priv_sound_path = NULL;
		free(priv_sound_path);
	}

	int vibration_type_val;
	ret = rpc_port_proxy_notification_get_vibration_type(handle, &vibration_type_val);
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_get_vibration_type : %d", ret);
	_noti->vibration_type = static_cast<notification_vibration_type_e>(
	    vibration_type_val);

	char *vibration_path;
	ret = rpc_port_proxy_notification_get_vibration_path(handle, &vibration_path);
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_get_vibration_path : %d", ret);

	if (strlen(vibration_path) > 0) {
		_noti->vibration_path = vibration_path;
	} else {
		_noti->vibration_path = NULL;
		free(vibration_path);
	}

	char *priv_vibration_path;
	ret = rpc_port_proxy_notification_get_priv_vibration_path(handle, &priv_vibration_path);
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_get_priv_vibration_path : %d", ret);

	if (strlen(priv_vibration_path) > 0) {
		_noti->priv_vibration_path = priv_vibration_path;
	} else {
		_noti->priv_vibration_path = NULL;
		free(priv_vibration_path);
	}

	int _led_operation;
	ret = rpc_port_proxy_notification_get_led_operation(handle, &_led_operation);
	_noti->led_operation = (notification_led_op_e)_led_operation;
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_get_led_operation : %d", ret);

	ret = rpc_port_proxy_notification_get_led_argb(handle, &_noti->led_argb);
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_get_LedArgb : %d", ret);

	ret = rpc_port_proxy_notification_get_led_on_ms(handle, &_noti->led_on_ms);
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_get_led_on_ms : %d", ret);

	ret = rpc_port_proxy_notification_get_led_off_ms(handle, &_noti->led_off_ms);
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_get_led_off_ms : %d", ret);

	long long _time;
	ret = rpc_port_proxy_notification_get_time(handle, &_time);
	_noti->time = (time_t)_time;
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_get_time : %d", ret);

	long long _insert_time;
	ret = rpc_port_proxy_notification_get_insert_time(handle, &_insert_time);
	_noti->insert_time = (time_t)_insert_time;
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_get_insert_time : %d", ret);

	ret = rpc_port_proxy_notification_get_flags_for_property(handle, &_noti->flags_for_property);
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_get_flags_for_property : %d", ret);

	ret = rpc_port_proxy_notification_get_display_applist(handle, &_noti->display_applist);
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_get_display_applist : %d", ret);

	ret = rpc_port_proxy_notification_get_progress_size(handle, &_noti->progress_size);
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_get_progress_size : %d", ret);

	ret = rpc_port_proxy_notification_get_progress_percentage(handle, &_noti->progress_percentage);
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_get_progress_percentage : %d", ret);

	char *app_icon_path;
	ret = rpc_port_proxy_notification_get_app_icon_path(handle, &app_icon_path);
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_get_app_icon_path : %d", ret);

	if (strlen(app_icon_path) > 0) {
		_noti->app_icon_path = app_icon_path;
	} else {
		_noti->app_icon_path = NULL;
		free(app_icon_path);
	}

	char *app_label;
	ret = rpc_port_proxy_notification_get_app_label(handle, &app_label);
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_get_app_label : %d", ret);

	if (strlen(app_label) > 0) {
		_noti->app_label = app_label;
	} else {
		_noti->app_label = NULL;
		free(app_label);
	}

	char *temp_title;
	ret = rpc_port_proxy_notification_get_temp_title(handle, &temp_title);
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_get_temp_title : %d", ret);

	if (strlen(temp_title) > 0) {
		_noti->temp_title = temp_title;
	} else {
		_noti->temp_title = NULL;
		free(temp_title);
	}

	char *temp_content;
	ret = rpc_port_proxy_notification_get_temp_content(handle, &temp_content);
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_get_temp_content : %d", ret);

	if (strlen(temp_content) > 0) {
		_noti->temp_content = temp_content;
	} else {
		_noti->temp_content = NULL;
		free(temp_content);
	}

	char *tag;
	ret = rpc_port_proxy_notification_get_tag(handle, &tag);
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_get_tag : %d", ret);

	if (strlen(tag) > 0) {
		_noti->tag = tag;
	} else {
		_noti->tag = NULL;
		free(tag);
	}

	ret = rpc_port_proxy_notification_get_ongoing_flag(handle, &_noti->ongoing_flag);
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_get_ongoing_flag : %d", ret);

	ret = rpc_port_proxy_notification_get_ongoing_value_type(handle, &_noti->ongoing_value_type);
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_get_ongoing_value_type : %d", ret);

	ret = rpc_port_proxy_notification_get_ongoing_current(handle, &_noti->ongoing_current);
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_get_ongoing_current : %d", ret);

	ret = rpc_port_proxy_notification_get_ongoing_duration(handle, &_noti->ongoing_duration);
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_get_ongoing_duration : %d", ret);

	ret = rpc_port_proxy_notification_get_auto_remove(handle, &_noti->auto_remove);
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_get_auto_remove : %d", ret);

	int _index;
	ret = rpc_port_proxy_notification_get_default_button_index(handle, &_index);
	_noti->default_button_index = (notification_button_index_e)_index;
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_get_default_button_index : %d", ret);

	ret = rpc_port_proxy_notification_get_hide_timeout(handle, &_noti->hide_timeout);
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_get_hide_timeout : %d", ret);

	ret = rpc_port_proxy_notification_get_delete_timeout(handle, &_noti->delete_timeout);
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_get_delete_timeout : %d", ret);

	ret = rpc_port_proxy_notification_get_text_input_max_length(handle, &_noti->text_input_max_length);
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_get_text_input_max_length : %d", ret);

	ret = rpc_port_proxy_notification_get_event_flag(handle, &_noti->event_flag);
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_get_event_flag : %d", ret);

	ret = rpc_port_proxy_notification_get_is_translation(handle, &_noti->is_translation);
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_get_is_translation : %d", ret);

	ret = rpc_port_proxy_notification_get_extension_image_size(handle, &_noti->extension_image_size);
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_get_extension_image_size : %d", ret);

	ret = rpc_port_proxy_notification_get_check_box(handle, &_noti->check_box);
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_get_check_box : %d", ret);

	ret = rpc_port_proxy_notification_get_check_box_value(handle, &_noti->check_box_value);
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_get_check_box_value : %d", ret);

	char *channel_name;
	ret = rpc_port_proxy_notification_get_channel_name(handle, &channel_name);
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_get_channel_name : %d", ret);

	if (strlen(channel_name) > 0) {
		_noti->channel_name = channel_name;
	} else {
		_noti->channel_name = NULL;
		free(channel_name);
	}

	int _uid;
	ret = rpc_port_proxy_notification_get_uid(handle, &_uid);
	_noti->uid = (uid_t)_uid;
	if (ret != RPC_PORT_ERROR_NONE)
		WARN("rpc_port_proxy_notification_get_uid : %d", ret);

	*noti = _noti;

	WARN("make_noti_from_notification done : %d", _uid);

	return NOTIFICATION_ERROR_NONE;
}

bool _rpc_port_proxy_list_noti_system_setting_dnd_allow_exception_cb (
		rpc_port_proxy_noti_system_setting_dnd_allow_exception_h value,
		void *user_data)
{
	int ret;
	dnd_allow_exception_h dnd_allow_exception;
	notification_system_setting_h _setting =
			(notification_system_setting_h)user_data;

	dnd_allow_exception = (dnd_allow_exception_h)calloc(1,
			sizeof(struct notification_system_setting_dnd_allow_exception));
	if (dnd_allow_exception == NULL) {
		ERR("Out of memory");
		return false;
	}

	ret = rpc_port_proxy_noti_system_setting_dnd_allow_exception_get_type(value,
			&dnd_allow_exception->type);
	if (ret != RPC_PORT_ERROR_NONE) {
		ERR("failed to get type");
		free(dnd_allow_exception);
		return false;
	}

	ret = rpc_port_proxy_noti_system_setting_dnd_allow_exception_get_value(value,
			&dnd_allow_exception->value);
	if (ret != RPC_PORT_ERROR_NONE) {
		ERR("failed to get value");
		free(dnd_allow_exception);
		return false;
	}

	_setting->dnd_allow_exceptions = g_list_append(_setting->dnd_allow_exceptions, dnd_allow_exception);

	return true;
}

EXPORT_API int make_setting_from_noti_system_setting(
		notification_system_setting_h *setting, void *settinghandle)
{
	notification_system_setting_h _setting;
	rpc_port_proxy_noti_system_setting_h handle;

	if (setting == NULL || settinghandle == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	handle = (rpc_port_proxy_noti_system_setting_h)settinghandle;

	WARN("make_setting_from_noti_system_setting");

	_setting = (notification_system_setting_h)calloc(1, sizeof(struct notification_system_setting));
	if (_setting == NULL) {
		ERR("Failed to alloc memory");
		return NOTIFICATION_ERROR_OUT_OF_MEMORY;
	}

	rpc_port_proxy_noti_system_setting_get_do_not_disturb(handle, &_setting->do_not_disturb);

	rpc_port_proxy_noti_system_setting_get_visibility_class(handle, &_setting->visibility_class);

	rpc_port_proxy_noti_system_setting_get_dnd_schedule_enabled(handle, &_setting->dnd_schedule_enabled);

	rpc_port_proxy_noti_system_setting_get_dnd_schedule_day(handle, &_setting->dnd_schedule_day);

	rpc_port_proxy_noti_system_setting_get_dnd_start_hour(handle, &_setting->dnd_start_hour);

	rpc_port_proxy_noti_system_setting_get_dnd_start_min(handle, &_setting->dnd_start_min);

	rpc_port_proxy_noti_system_setting_get_dnd_end_hour(handle, &_setting->dnd_end_hour);

	rpc_port_proxy_noti_system_setting_get_dnd_end_min(handle, &_setting->dnd_end_min);

	int _lock_screen_content_level;
	rpc_port_proxy_noti_system_setting_get_lock_screen_content_level(handle, &_lock_screen_content_level);
	_setting->lock_screen_content_level = (lock_screen_content_level_e)_lock_screen_content_level;

	rpc_port_proxy_list_noti_system_setting_dnd_allow_exception_h dnd_allow_list;
	rpc_port_proxy_noti_system_setting_get_dnd_allow_exceptions(handle, &dnd_allow_list);

	rpc_port_proxy_list_noti_system_setting_dnd_allow_exception_foreach(dnd_allow_list,
			_rpc_port_proxy_list_noti_system_setting_dnd_allow_exception_cb, _setting);

	*setting = _setting;

	WARN("make_setting_from_noti_system_setting done");

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int make_dnd_allow_exception_from_exception(void *exception_handle,
		dnd_allow_exception_h dnd_allow_exception)
{
	int ret;
	rpc_port_proxy_noti_system_setting_dnd_allow_exception_h _exception_handle
			= (rpc_port_proxy_noti_system_setting_dnd_allow_exception_h)exception_handle;

	if (exception_handle == NULL || dnd_allow_exception == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	ret = rpc_port_proxy_noti_system_setting_dnd_allow_exception_set_type(_exception_handle, dnd_allow_exception->type);
	if (ret != RPC_PORT_ERROR_NONE)
		ERR("failed to set dnd allow exception");

	ret = rpc_port_proxy_noti_system_setting_dnd_allow_exception_set_value(_exception_handle, dnd_allow_exception->value);
	if (ret != RPC_PORT_ERROR_NONE)
		ERR("failed to set value");

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int make_noti_system_setting_from_setting(void *settinghandle,
		notification_system_setting_h setting)
{
	int ret;
	dnd_allow_exception_h info = NULL;
	rpc_port_proxy_list_noti_system_setting_dnd_allow_exception_h list_handle;
	rpc_port_proxy_noti_system_setting_dnd_allow_exception_h exception_handle;
	rpc_port_proxy_noti_system_setting_h handle;

	if (settinghandle == NULL || setting == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	handle = (rpc_port_proxy_noti_system_setting_h)settinghandle;

	ret = rpc_port_proxy_noti_system_setting_set_do_not_disturb(handle, setting->do_not_disturb);
	if (ret != RPC_PORT_ERROR_NONE)
		ERR("failed to set do not disturb");

	ret = rpc_port_proxy_noti_system_setting_set_visibility_class(handle, setting->visibility_class);
	if (ret != RPC_PORT_ERROR_NONE)
		ERR("failed to set visibility");

	ret = rpc_port_proxy_noti_system_setting_set_dnd_schedule_enabled(handle, setting->dnd_schedule_enabled);
	if (ret != RPC_PORT_ERROR_NONE)
		ERR("failed to set dnd schedule enabled");

	ret = rpc_port_proxy_noti_system_setting_set_dnd_schedule_day(handle, setting->dnd_schedule_day);
	if (ret != RPC_PORT_ERROR_NONE)
		ERR("failed to set dnd schedule day");

	ret = rpc_port_proxy_noti_system_setting_set_dnd_start_hour(handle, setting->dnd_start_hour);
	if (ret != RPC_PORT_ERROR_NONE)
		ERR("failed to set dnd start hour");

	ret = rpc_port_proxy_noti_system_setting_set_dnd_start_min(handle, setting->dnd_start_min);
	if (ret != RPC_PORT_ERROR_NONE)
		ERR("failed to set dnd start min");

	ret = rpc_port_proxy_noti_system_setting_set_dnd_end_hour(handle, setting->dnd_end_hour);
	if (ret != RPC_PORT_ERROR_NONE)
		ERR("failed to set dnd end hour");

	ret = rpc_port_proxy_noti_system_setting_set_dnd_end_min(handle, setting->dnd_end_min);
	if (ret != RPC_PORT_ERROR_NONE)
		ERR("failed to set dnd end min");

	ret = rpc_port_proxy_noti_system_setting_set_lock_screen_content_level(handle, setting->lock_screen_content_level);
	if (ret != RPC_PORT_ERROR_NONE)
		ERR("failed to set lock screen content level");

	if (g_list_length(setting->dnd_allow_exceptions) > 0) {
		ret = rpc_port_proxy_list_noti_system_setting_dnd_allow_exception_create(&list_handle);
		if (ret != RPC_PORT_ERROR_NONE) {
			ERR("failed to create system setting dnd allow exception list");
			return NOTIFICATION_ERROR_NONE;
		}

		setting->dnd_allow_exceptions = g_list_first(setting->dnd_allow_exceptions);

		for (GList *iter = setting->dnd_allow_exceptions; iter != NULL; iter = iter->next) {
			info = static_cast<dnd_allow_exception_h>(iter->data);

			ret = rpc_port_proxy_noti_system_setting_dnd_allow_exception_create(&exception_handle);
			if (ret == RPC_PORT_ERROR_NONE) {
				make_dnd_allow_exception_from_exception(exception_handle, info);

				ret = rpc_port_proxy_list_noti_system_setting_dnd_allow_exception_add(list_handle, exception_handle);
				if (ret != RPC_PORT_ERROR_NONE)
					ERR("failed to add list");

				ret = rpc_port_proxy_noti_system_setting_dnd_allow_exception_destroy(exception_handle);
				if (ret != RPC_PORT_ERROR_NONE)
					ERR("failed to destroy");
			} else {
				ERR("failed to create system setting dnd allow exception");
			}
		}

		ret = rpc_port_proxy_noti_system_setting_set_dnd_allow_exceptions(handle, list_handle);
		if (ret != RPC_PORT_ERROR_NONE)
			ERR("failed to set dnd allow exception list");
	}

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int make_setting_from_noti_setting(
		notification_setting_h setting, void *settinghandle)
{
	int ret;
	rpc_port_proxy_noti_setting_h handle;
	char *pkg_name;
	char *app_id;
	bool allow_to_notify;
	bool dnd_except;
	bool pop_up_noti;
	int visibility_class;
	int lock_screen_content_level;
	bool app_disabled;

	if (setting == NULL || settinghandle == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	handle = (rpc_port_proxy_noti_setting_h)settinghandle;

	WARN("make_setting_from_noti_setting");

	ret = rpc_port_proxy_noti_setting_get_package_name(handle, &pkg_name);
	if (ret != RPC_PORT_ERROR_NONE)
		ERR("failed to get pkg name");

	ret = rpc_port_proxy_noti_setting_get_app_id(handle, &app_id);
	if (ret != RPC_PORT_ERROR_NONE)
		ERR("failed to get app id");

	ret = rpc_port_proxy_noti_setting_get_allow_to_notify(handle, &allow_to_notify);
	if (ret != RPC_PORT_ERROR_NONE)
		ERR("failed to get allow to notify");

	ret = rpc_port_proxy_noti_setting_get_do_not_disturb_except(handle, &dnd_except);
	if (ret != RPC_PORT_ERROR_NONE)
		ERR("failed to get dnd except");

	ret = rpc_port_proxy_noti_setting_get_pop_up_notification(handle, &pop_up_noti);
	if (ret != RPC_PORT_ERROR_NONE)
		ERR("failed to get pop up noti");

	ret = rpc_port_proxy_noti_setting_get_visibility_class(handle,&visibility_class);
	if (ret != RPC_PORT_ERROR_NONE)
		ERR("failed to get visiblility class");

	ret = rpc_port_proxy_noti_setting_get_lock_screen_content_level(handle, &lock_screen_content_level);
	if (ret != RPC_PORT_ERROR_NONE)
		ERR("failed to get lock screen content level");

	ret = rpc_port_proxy_noti_setting_get_app_disabled(handle, &app_disabled);
	if (ret != RPC_PORT_ERROR_NONE)
		ERR("failed to get app disabled");

	if (strlen(pkg_name) > 0) {
		setting->package_name = pkg_name;
	} else{
		setting->package_name = NULL;
		free(pkg_name);
	}

	if (strlen(app_id) > 0) {
		setting->app_id = app_id;
	} else {
		setting->app_id = NULL;
		free(app_id);
	}

	setting->allow_to_notify = allow_to_notify;
	setting->do_not_disturb_except = dnd_except;
	setting->pop_up_notification = pop_up_noti;
	setting->visibility_class = visibility_class;
	setting->lock_screen_content_level =
	    static_cast<lock_screen_content_level_e>(lock_screen_content_level);
	setting->app_disabled = app_disabled;

	WARN("make_setting_from_noti_setting done");

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int make_noti_setting_from_setting(
		void *settinghandle, notification_setting_h setting)
{
	int ret;
	rpc_port_proxy_noti_setting_h handle;

	if (settinghandle == NULL || setting == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	handle = (rpc_port_proxy_noti_setting_h)settinghandle;

	ret = rpc_port_proxy_noti_setting_set_package_name(handle, __get_str_value(setting->package_name));
	if (ret != RPC_PORT_ERROR_NONE)
		ERR("failed to set package name");

	ret = rpc_port_proxy_noti_setting_set_app_id(handle, __get_str_value(setting->app_id));
	if (ret != RPC_PORT_ERROR_NONE)
		ERR("failed to set app id");

	ret = rpc_port_proxy_noti_setting_set_allow_to_notify(handle, setting->allow_to_notify);
	if (ret != RPC_PORT_ERROR_NONE)
		ERR("failed to set allow to notify");

	ret = rpc_port_proxy_noti_setting_set_do_not_disturb_except(handle, setting->do_not_disturb_except);
	if (ret != RPC_PORT_ERROR_NONE)
		ERR("failed to set do not disturb except");

	ret = rpc_port_proxy_noti_setting_set_pop_up_notification(handle, setting->pop_up_notification);
	if (ret != RPC_PORT_ERROR_NONE)
		ERR("failed to set pop up notification");

	ret = rpc_port_proxy_noti_setting_set_visibility_class(handle, setting->visibility_class);
	if (ret != RPC_PORT_ERROR_NONE)
		ERR("failed to set visibility");

	ret = rpc_port_proxy_noti_setting_set_lock_screen_content_level(handle, (int)setting->lock_screen_content_level);
	if (ret != RPC_PORT_ERROR_NONE)
		ERR("failed to set lock screen content level");

	ret = rpc_port_proxy_noti_setting_set_app_disabled(handle, setting->app_disabled);
	if (ret != RPC_PORT_ERROR_NONE)
		ERR("failed to set app disabled");

	return NOTIFICATION_ERROR_NONE;
}
