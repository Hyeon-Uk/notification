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

#ifndef __NOTIFICATION_IPC_H__
#define __NOTIFICATION_IPC_H__

#include <gio/gio.h>
#include <sys/types.h>

#include <notification.h>
#include <notification_setting.h>
#include <notification_setting_internal.h>
#include <notification_list.h>

#ifdef __cplusplus
extern "C" {
#endif

GVariant *notification_ipc_make_gvariant_from_noti(notification_h noti, bool translate);
int notification_ipc_make_noti_from_gvariant(notification_h noti,
		GVariant *variant);

GVariant *notification_ipc_make_gvariant_from_setting(
		struct notification_setting *noti_setting);
int notification_ipc_make_setting_from_gvariant(
		struct notification_setting *noti_setting, GVariant *variant);

GVariant *notification_ipc_make_gvariant_from_system_setting(
		struct notification_system_setting *noti_setting);
int notification_ipc_make_system_setting_from_gvariant(
		struct notification_system_setting *noti_setting,
		GVariant *variant);

int notification_dbus_init();
int notification_ipc_monitor_init(uid_t uid);
int notification_ipc_monitor_fini(void);
void notification_ipc_event_monitor_fini(void);

int notification_ipc_is_master_ready(void);
int notification_ipc_add_deffered_task(void (*deferred_task_cb)(void *data),
		void *user_data);
int notification_ipc_del_deffered_task(void (*deferred_task_cb)(void *data));
int notification_ipc_request_insert(notification_h noti, int *priv_id);
int notification_ipc_request_update(notification_h noti);
int notification_ipc_request_update_async(notification_h noti,
		void (*result_cb)(int priv_id, int result, void *data),
		void *user_data);
int notification_ipc_request_refresh(uid_t uid);
int notification_ipc_request_delete_multiple(notification_type_e type,
		char *app_id, uid_t uid);
int notification_ipc_request_delete_multiple_by_viewer(int viewer, uid_t uid);
int notification_ipc_request_delete_single(notification_type_e type,
		char *app_id, int priv_id, uid_t uid);
int notification_ipc_update_setting(notification_setting_h setting, uid_t uid);
int notification_ipc_update_system_setting(
		notification_system_setting_h system_setting, uid_t uid);
int notification_ipc_request_load_noti_by_tag(notification_h noti,
		const char *app_id, const char *tag, uid_t uid);
int notification_ipc_request_load_noti_grouping_list(notification_type_e type,
		int count, int count_per_page, notification_list_h *list, uid_t uid);
int notification_ipc_request_get_setting_array(
		notification_setting_h *setting_array, int *count, uid_t uid);
int notification_ipc_request_get_setting_by_app_id(
		const char *app_id, notification_setting_h *setting, uid_t uid);
int notification_ipc_request_load_system_setting(
		notification_system_setting_h *setting, uid_t uid);
int notification_ipc_request_get_count(notification_type_e type,
		const char *app_id, int group_id, int priv_id, int *count, uid_t uid);
int notification_ipc_request_load_noti_by_priv_id(notification_h noti,
		const char *app_id, int priv_id, uid_t uid);
int notification_ipc_request_load_noti_detail_list(const char *app_id,
		int group_id, int priv_id, int count,
		notification_list_h *list, uid_t uid);
int notification_ipc_request_save_as_template(notification_h noti, const char *template_name);
int notification_ipc_request_create_from_template(notification_h noti, const char *template_name);
int notification_ipc_request_create_from_package_template(notification_h noti,
		const char *app_id, const char *template_name);
int notification_ipc_get_noti_block_state(const char *app_id, int *do_not_disturb, int *do_not_disturb_except,
					  int *allow_to_notify, uid_t uid);
GVariant *notification_ipc_make_gvariant_from_dnd_allow_exception(
		struct notification_system_setting_dnd_allow_exception *dnd_allow_exception);
int notification_ipc_make_dnd_allow_exception_from_gvariant(
		struct notification_system_setting_dnd_allow_exception *dnd_allow_exception,
		GVariant *variant);
int notification_ipc_send_event(notification_h noti, int event_type, int priv_id);
int notification_ipc_check_event_receiver(int priv_id, bool *available);
void notification_ipc_reset_event_handler(int priv_id);
int notification_ipc_request_get_all_count(notification_type_e type, int *count, uid_t uid);

/* Functions related with socket */
int notification_ipc_socket_pair(int *fd);
int notification_ipc_socket_get_read_buf_size(int fd, unsigned int *size);
int notification_ipc_socket_get_write_buf_size(int fd, unsigned int *size);
int notification_ipc_socket_write(int fd, const char *buffer, unsigned int nbytes);
int notification_ipc_socket_write_string(int fd, const char *buffer, unsigned int string_len);
int notification_ipc_socket_read(int fd, char *buffer, unsigned int nbytes);
#ifdef __cplusplus
}
#endif
#endif /* __NOTIFICATION_IPC_H__ */

