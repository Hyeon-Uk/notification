/*
 * Copyright (c) 2023 Samsung Electronics Co., Ltd. All rights reserved.
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

#include <dlfcn.h>

#include <notification.h>
#include <notification_setting.h>
#include <notification_setting_internal.h>
#include <notification_list.h>
#include <notification_internal.h>

extern "C" {

int tidl_stub_insert_called;
int tidl_stub_update_called;
int tidl_stub_delete_called;
int tidl_stub_delete_all_called;
int tidl_stub_save_template_called;
int tidl_stub_create_template_called;
int tidl_stub_refresh_called;

int tidl_stub_next_insert_ret;
int tidl_stub_next_update_ret;
int tidl_stub_next_delete_ret;
int tidl_stub_next_delete_all_ret;
int tidl_stub_next_save_template_ret;
int tidl_stub_next_create_template_ret;

bool tidl_stub_plt_active(void) {
  void* sym = dlsym(RTLD_NEXT, "notification_tidl_request_insert");
  return sym != nullptr;
}

void tidl_stub_reset(void) {
  tidl_stub_insert_called = 0;
  tidl_stub_update_called = 0;
  tidl_stub_delete_called = 0;
  tidl_stub_delete_all_called = 0;
  tidl_stub_save_template_called = 0;
  tidl_stub_create_template_called = 0;
  tidl_stub_refresh_called = 0;

  tidl_stub_next_insert_ret = NOTIFICATION_ERROR_NONE;
  tidl_stub_next_update_ret = NOTIFICATION_ERROR_NONE;
  tidl_stub_next_delete_ret = NOTIFICATION_ERROR_NONE;
  tidl_stub_next_delete_all_ret = NOTIFICATION_ERROR_NONE;
  tidl_stub_next_save_template_ret = NOTIFICATION_ERROR_NONE;
  tidl_stub_next_create_template_ret = NOTIFICATION_ERROR_NONE;
}

int notification_tidl_is_master_ready(void) {
  return 1;
}

int notification_tidl_event_monitor_init(void) {
  return NOTIFICATION_ERROR_NONE;
}

void notification_tidl_event_monitor_fini(void) {
}

int notification_tidl_monitor_init(void) {
  return NOTIFICATION_ERROR_NONE;
}

int notification_tidl_monitor_fini(void) {
  return NOTIFICATION_ERROR_NONE;
}

int notification_tidl_request_insert(notification_h noti, int *priv_id) {
  tidl_stub_insert_called++;
  if (priv_id)
    *priv_id = 9001;
  return tidl_stub_next_insert_ret;
}

int notification_tidl_request_refresh(uid_t uid) {
  tidl_stub_refresh_called++;
  return NOTIFICATION_ERROR_NONE;
}

int notification_tidl_request_update(notification_h noti) {
  tidl_stub_update_called++;
  return tidl_stub_next_update_ret;
}

int notification_tidl_request_delete_single(char *app_id, int priv_id,
    uid_t uid) {
  tidl_stub_delete_called++;
  return tidl_stub_next_delete_ret;
}

int notification_tidl_request_delete_multiple(notification_type_e type,
    char *app_id, uid_t uid) {
  tidl_stub_delete_all_called++;
  return tidl_stub_next_delete_all_ret;
}

int notification_tidl_send_event(notification_h noti, int event_type,
    int priv_id) {
  return NOTIFICATION_ERROR_NONE;
}

int notification_tidl_request_load_noti_by_priv_id(notification_h *noti,
    const char *app_id, int priv_id, uid_t uid) {
  return NOTIFICATION_ERROR_IO_ERROR;
}

int notification_tidl_request_load_noti_by_tag(notification_h *noti,
    const char *app_id, const char *tag, uid_t uid) {
  return NOTIFICATION_ERROR_IO_ERROR;
}

int notification_tidl_request_get_all_count(notification_type_e type,
    int *count, uid_t uid) {
  return NOTIFICATION_ERROR_IO_ERROR;
}

int notification_tidl_request_load_system_setting(
    notification_system_setting_h *setting, uid_t uid) {
  return NOTIFICATION_ERROR_IO_ERROR;
}

int notification_tidl_request_get_setting_array(
    notification_setting_h *setting_array, int *count, uid_t uid) {
  return NOTIFICATION_ERROR_IO_ERROR;
}

int notification_tidl_request_get_setting_by_app_id(const char *app_id,
    notification_setting_h *setting, uid_t uid) {
  return NOTIFICATION_ERROR_IO_ERROR;
}

int notification_tidl_update_setting(notification_setting_h setting,
    uid_t uid) {
  return NOTIFICATION_ERROR_IO_ERROR;
}

int notification_tidl_update_system_setting(
    notification_system_setting_h system_setting, uid_t uid) {
  return NOTIFICATION_ERROR_IO_ERROR;
}

int notification_tidl_request_save_as_template(notification_h noti,
    const char *template_name) {
  tidl_stub_save_template_called++;
  return tidl_stub_next_save_template_ret;
}

int notification_tidl_request_create_from_template(notification_h *noti,
    const char *template_name) {
  tidl_stub_create_template_called++;
  if (tidl_stub_next_create_template_ret != NOTIFICATION_ERROR_NONE)
    return tidl_stub_next_create_template_ret;
  if (noti)
    *noti = notification_create(NOTIFICATION_TYPE_NOTI);
  return NOTIFICATION_ERROR_NONE;
}

int notification_tidl_request_create_from_package_template(
    notification_h *noti, const char *app_id,
    const char *template_name) {
  return NOTIFICATION_ERROR_IO_ERROR;
}

int notification_tidl_request_load_noti_grouping_list(
    notification_type_e type, int count, int count_per_page,
    notification_list_h *list, uid_t uid) {
  return NOTIFICATION_ERROR_IO_ERROR;
}

int notification_tidl_request_load_noti_detail_list(const char *app_id,
    int group_id, int priv_id, int count, notification_list_h *list,
    uid_t uid) {
  return NOTIFICATION_ERROR_IO_ERROR;
}

int notification_tidl_get_noti_block_state(const char *app_id,
    int *do_not_disturb, int *do_not_disturb_except,
    int *allow_to_notify, uid_t uid) {
  return NOTIFICATION_ERROR_IO_ERROR;
}

int notification_tidl_request_delete_by_display_applist(
    int display_applist, uid_t uid) {
  return NOTIFICATION_ERROR_IO_ERROR;
}

int notification_tidl_request_get_count(notification_type_e type,
    const char *app_id, int group_id, int priv_id, int *count,
    uid_t uid) {
  return NOTIFICATION_ERROR_IO_ERROR;
}

int notification_tidl_check_event_receiver(int priv_id, bool *available) {
  return NOTIFICATION_ERROR_IO_ERROR;
}

void notification_tidl_reset_event_handler(int priv_id) {
}

int notification_tidl_request_register_dnd_app(uid_t uid, pid_t pid) {
  return NOTIFICATION_ERROR_IO_ERROR;
}

int notification_tidl_request_unregister_dnd_app(uid_t uid, pid_t pid) {
  return NOTIFICATION_ERROR_IO_ERROR;
}

int notification_tidl_request_insert_channel(const char *app_id,
    const char *channel_name, bool blockable, bool is_blocked,
    uid_t uid) {
  return NOTIFICATION_ERROR_IO_ERROR;
}

int notification_tidl_request_delete_channel(const char *app_id,
    const char *channel_name, uid_t uid) {
  return NOTIFICATION_ERROR_IO_ERROR;
}

int notification_tidl_request_update_channel(const char *app_id,
    const char *channel_name, bool blockable, bool is_blocked,
    uid_t uid) {
  return NOTIFICATION_ERROR_IO_ERROR;
}

int notification_tidl_request_get_channel(const char *app_id,
    const char *channel_name, bool *blockable, bool *is_blocked,
    uid_t uid) {
  return NOTIFICATION_ERROR_IO_ERROR;
}

int notification_tidl_request_channel_get_list(const char *app_id,
    notification_channel_foreach_cb cb, void *user_data, uid_t uid) {
  return NOTIFICATION_ERROR_IO_ERROR;
}

int notification_tidl_add_deffered_task(
    void (*deferred_task_cb)(void *data), void *user_data) {
  return NOTIFICATION_ERROR_IO_ERROR;
}

int notification_tidl_del_deffered_task(
    void (*deferred_task_cb)(void *data)) {
  return NOTIFICATION_ERROR_IO_ERROR;
}

}  // extern "C"
