/*
 * Copyright (c) 2026 Samsung Electronics Co., Ltd.
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

#include "noti_service_proxy_mock.hh"

#include "mock_hook.hh"
#include "test_fixture.hh"

NotiServiceProxyMock::NotiServiceProxyMock() = default;
NotiServiceProxyMock::~NotiServiceProxyMock() = default;

/* Proxy lifecycle */
extern "C" int rpc_port_proxy_noti_service_create(const char* stub_appid,
    rpc_port_proxy_noti_service_callback_s* callback, void* user_data,
    rpc_port_proxy_noti_service_h* h) {
  return MOCK_HOOK_P4(NotiServiceProxyMock,
      rpc_port_proxy_noti_service_create,
      stub_appid, callback, user_data, h);
}

extern "C" int rpc_port_proxy_noti_service_destroy(
    rpc_port_proxy_noti_service_h h) {
  return MOCK_HOOK_P1(NotiServiceProxyMock,
      rpc_port_proxy_noti_service_destroy, h);
}

extern "C" int rpc_port_proxy_noti_service_connect_sync(
    rpc_port_proxy_noti_service_h h) {
  return MOCK_HOOK_P1(NotiServiceProxyMock,
      rpc_port_proxy_noti_service_connect_sync, h);
}

/* Changed callback handle */
extern "C" int rpc_port_proxy_noti_service_changed_create(
    rpc_port_proxy_noti_service_changed_h* h) {
  return MOCK_HOOK_P1(NotiServiceProxyMock,
      rpc_port_proxy_noti_service_changed_create, h);
}

extern "C" int rpc_port_proxy_noti_service_changed_destroy(
    rpc_port_proxy_noti_service_changed_h h) {
  return MOCK_HOOK_P1(NotiServiceProxyMock,
      rpc_port_proxy_noti_service_changed_destroy, h);
}

extern "C" int rpc_port_proxy_noti_service_changed_dispose(
    rpc_port_proxy_noti_service_h proxy,
    rpc_port_proxy_noti_service_changed_h h) {
  return MOCK_HOOK_P2(NotiServiceProxyMock,
      rpc_port_proxy_noti_service_changed_dispose, proxy, h);
}

extern "C" int rpc_port_proxy_noti_service_changed_set_callback(
    rpc_port_proxy_noti_service_changed_h h,
    rpc_port_proxy_noti_service_changed_cb callback, void* user_data) {
  return MOCK_HOOK_P3(NotiServiceProxyMock,
      rpc_port_proxy_noti_service_changed_set_callback,
      h, callback, user_data);
}

/* Event-received callback handle */
extern "C" int rpc_port_proxy_noti_service_event_received_create(
    rpc_port_proxy_noti_service_event_received_h* h) {
  return MOCK_HOOK_P1(NotiServiceProxyMock,
      rpc_port_proxy_noti_service_event_received_create, h);
}

extern "C" int rpc_port_proxy_noti_service_event_received_destroy(
    rpc_port_proxy_noti_service_event_received_h h) {
  return MOCK_HOOK_P1(NotiServiceProxyMock,
      rpc_port_proxy_noti_service_event_received_destroy, h);
}

extern "C" int rpc_port_proxy_noti_service_event_received_dispose(
    rpc_port_proxy_noti_service_h proxy,
    rpc_port_proxy_noti_service_event_received_h h) {
  return MOCK_HOOK_P2(NotiServiceProxyMock,
      rpc_port_proxy_noti_service_event_received_dispose, proxy, h);
}

extern "C" int rpc_port_proxy_noti_service_event_received_set_callback(
    rpc_port_proxy_noti_service_event_received_h h,
    rpc_port_proxy_noti_service_event_received_cb callback,
    void* user_data) {
  return MOCK_HOOK_P3(NotiServiceProxyMock,
      rpc_port_proxy_noti_service_event_received_set_callback,
      h, callback, user_data);
}

/* Invoke: register/unregister */
extern "C" int rpc_port_proxy_noti_service_invoke_register_changed(
    rpc_port_proxy_noti_service_h h,
    rpc_port_proxy_noti_service_changed_h callback, int uid) {
  return MOCK_HOOK_P3(NotiServiceProxyMock,
      rpc_port_proxy_noti_service_invoke_register_changed,
      h, callback, uid);
}

extern "C" int rpc_port_proxy_noti_service_invoke_unregister_changed(
    rpc_port_proxy_noti_service_h h) {
  return MOCK_HOOK_P1(NotiServiceProxyMock,
      rpc_port_proxy_noti_service_invoke_unregister_changed, h);
}

extern "C" int rpc_port_proxy_noti_service_invoke_register_event(
    rpc_port_proxy_noti_service_h h,
    rpc_port_proxy_noti_service_event_received_h callback) {
  return MOCK_HOOK_P2(NotiServiceProxyMock,
      rpc_port_proxy_noti_service_invoke_register_event, h, callback);
}

extern "C" int rpc_port_proxy_noti_service_invoke_unregister_event(
    rpc_port_proxy_noti_service_h h) {
  return MOCK_HOOK_P1(NotiServiceProxyMock,
      rpc_port_proxy_noti_service_invoke_unregister_event, h);
}

extern "C" int rpc_port_proxy_noti_service_invoke_register_service(
    rpc_port_proxy_noti_service_h h, int uid) {
  return MOCK_HOOK_P2(NotiServiceProxyMock,
      rpc_port_proxy_noti_service_invoke_register_service, h, uid);
}

/* Invoke: notification CRUD */
extern "C" int rpc_port_proxy_noti_service_invoke_add_noti(
    rpc_port_proxy_noti_service_h h,
    rpc_port_proxy_notification_h notihandle, int* priv_id) {
  return MOCK_HOOK_P3(NotiServiceProxyMock,
      rpc_port_proxy_noti_service_invoke_add_noti,
      h, notihandle, priv_id);
}

extern "C" int rpc_port_proxy_noti_service_invoke_update_noti(
    rpc_port_proxy_noti_service_h h,
    rpc_port_proxy_notification_h notihandle, int* priv_id) {
  return MOCK_HOOK_P3(NotiServiceProxyMock,
      rpc_port_proxy_noti_service_invoke_update_noti,
      h, notihandle, priv_id);
}

extern "C" int rpc_port_proxy_noti_service_invoke_refresh_noti(
    rpc_port_proxy_noti_service_h h, int uid) {
  return MOCK_HOOK_P2(NotiServiceProxyMock,
      rpc_port_proxy_noti_service_invoke_refresh_noti, h, uid);
}

extern "C" int rpc_port_proxy_noti_service_invoke_delete_single_noti(
    rpc_port_proxy_noti_service_h h, const char* app_id, int priv_id,
    int uid) {
  return MOCK_HOOK_P4(NotiServiceProxyMock,
      rpc_port_proxy_noti_service_invoke_delete_single_noti,
      h, app_id, priv_id, uid);
}

extern "C" int rpc_port_proxy_noti_service_invoke_delete_multiple_noti(
    rpc_port_proxy_noti_service_h h, const char* app_id, int noti_type,
    int uid, int* deleted_num) {
  return MOCK_HOOK_P5(NotiServiceProxyMock,
      rpc_port_proxy_noti_service_invoke_delete_multiple_noti,
      h, app_id, noti_type, uid, deleted_num);
}

extern "C" int
rpc_port_proxy_noti_service_invoke_delete_noti_by_display_applist(
    rpc_port_proxy_noti_service_h h, int display_applist, int uid) {
  return MOCK_HOOK_P3(NotiServiceProxyMock,
      rpc_port_proxy_noti_service_invoke_delete_noti_by_display_applist,
      h, display_applist, uid);
}

/* Invoke: notification queries */
extern "C" int rpc_port_proxy_noti_service_invoke_get_noti_count(
    rpc_port_proxy_noti_service_h h, int type, const char* app_id,
    int group_id, int priv_id, int uid, int* ret_count) {
  return MOCK_HOOK_P7(NotiServiceProxyMock,
      rpc_port_proxy_noti_service_invoke_get_noti_count,
      h, type, app_id, group_id, priv_id, uid, ret_count);
}

extern "C" int rpc_port_proxy_noti_service_invoke_get_all_noti_count(
    rpc_port_proxy_noti_service_h h, int type, int uid, int* ret_count) {
  return MOCK_HOOK_P4(NotiServiceProxyMock,
      rpc_port_proxy_noti_service_invoke_get_all_noti_count,
      h, type, uid, ret_count);
}

extern "C" int rpc_port_proxy_noti_service_invoke_load_noti_by_tag(
    rpc_port_proxy_noti_service_h h, const char* app_id, const char* tag,
    int uid, rpc_port_proxy_notification_h* notihandle) {
  return MOCK_HOOK_P5(NotiServiceProxyMock,
      rpc_port_proxy_noti_service_invoke_load_noti_by_tag,
      h, app_id, tag, uid, notihandle);
}

extern "C" int rpc_port_proxy_noti_service_invoke_load_noti_by_priv_id(
    rpc_port_proxy_noti_service_h h, const char* app_id, int priv_id,
    int uid, rpc_port_proxy_notification_h* notihandle) {
  return MOCK_HOOK_P5(NotiServiceProxyMock,
      rpc_port_proxy_noti_service_invoke_load_noti_by_priv_id,
      h, app_id, priv_id, uid, notihandle);
}

extern "C" int rpc_port_proxy_noti_service_invoke_load_noti_grouping_list(
    rpc_port_proxy_noti_service_h h, int type, int count,
    int count_per_page, int uid,
    rpc_port_proxy_list_notification_h* noti_list) {
  return MOCK_HOOK_P6(NotiServiceProxyMock,
      rpc_port_proxy_noti_service_invoke_load_noti_grouping_list,
      h, type, count, count_per_page, uid, noti_list);
}

extern "C" int rpc_port_proxy_noti_service_invoke_load_noti_detail_list(
    rpc_port_proxy_noti_service_h h, const char* app_id, int group_id,
    int priv_id, int count, int uid,
    rpc_port_proxy_list_notification_h* noti_list) {
  return MOCK_HOOK_P7(NotiServiceProxyMock,
      rpc_port_proxy_noti_service_invoke_load_noti_detail_list,
      h, app_id, group_id, priv_id, count, uid, noti_list);
}

/* Invoke: settings */
extern "C" int rpc_port_proxy_noti_service_invoke_update_noti_setting(
    rpc_port_proxy_noti_service_h h, rpc_port_proxy_noti_setting_h setting,
    int uid) {
  return MOCK_HOOK_P3(NotiServiceProxyMock,
      rpc_port_proxy_noti_service_invoke_update_noti_setting,
      h, setting, uid);
}

extern "C" int rpc_port_proxy_noti_service_invoke_update_noti_sys_setting(
    rpc_port_proxy_noti_service_h h,
    rpc_port_proxy_noti_system_setting_h setting, int uid) {
  return MOCK_HOOK_P3(NotiServiceProxyMock,
      rpc_port_proxy_noti_service_invoke_update_noti_sys_setting,
      h, setting, uid);
}

extern "C" int rpc_port_proxy_noti_service_invoke_get_setting_array(
    rpc_port_proxy_noti_service_h h, int uid,
    rpc_port_proxy_array_noti_setting_h* setting_arr) {
  return MOCK_HOOK_P3(NotiServiceProxyMock,
      rpc_port_proxy_noti_service_invoke_get_setting_array,
      h, uid, setting_arr);
}

extern "C" int rpc_port_proxy_noti_service_invoke_get_setting_by_app_id(
    rpc_port_proxy_noti_service_h h, const char* app_id, int uid,
    rpc_port_proxy_noti_setting_h* setting) {
  return MOCK_HOOK_P4(NotiServiceProxyMock,
      rpc_port_proxy_noti_service_invoke_get_setting_by_app_id,
      h, app_id, uid, setting);
}

extern "C" int rpc_port_proxy_noti_service_invoke_load_system_setting(
    rpc_port_proxy_noti_service_h h, int uid,
    rpc_port_proxy_noti_system_setting_h* setting) {
  return MOCK_HOOK_P3(NotiServiceProxyMock,
      rpc_port_proxy_noti_service_invoke_load_system_setting,
      h, uid, setting);
}

/* Invoke: templates */
extern "C" int rpc_port_proxy_noti_service_invoke_save_as_template(
    rpc_port_proxy_noti_service_h h,
    rpc_port_proxy_notification_h notihandle, const char* name) {
  return MOCK_HOOK_P3(NotiServiceProxyMock,
      rpc_port_proxy_noti_service_invoke_save_as_template,
      h, notihandle, name);
}

extern "C" int rpc_port_proxy_noti_service_invoke_create_from_template(
    rpc_port_proxy_noti_service_h h, const char* name,
    rpc_port_proxy_notification_h* notihandle) {
  return MOCK_HOOK_P3(NotiServiceProxyMock,
      rpc_port_proxy_noti_service_invoke_create_from_template,
      h, name, notihandle);
}

extern "C" int
rpc_port_proxy_noti_service_invoke_create_from_package_template(
    rpc_port_proxy_noti_service_h h, const char* app_id, const char* name,
    rpc_port_proxy_notification_h* notihandle) {
  return MOCK_HOOK_P4(NotiServiceProxyMock,
      rpc_port_proxy_noti_service_invoke_create_from_package_template,
      h, app_id, name, notihandle);
}

/* Invoke: DND / block state */
extern "C" int rpc_port_proxy_noti_service_invoke_get_noti_block_state(
    rpc_port_proxy_noti_service_h h, const char* app_id, int uid, int* dnd,
    int* dnd_except, int* allow_to_notify) {
  return MOCK_HOOK_P6(NotiServiceProxyMock,
      rpc_port_proxy_noti_service_invoke_get_noti_block_state,
      h, app_id, uid, dnd, dnd_except, allow_to_notify);
}

extern "C" int
rpc_port_proxy_noti_service_invoke_update_dnd_allow_exception(
    rpc_port_proxy_noti_service_h h, int type, int value, int uid) {
  return MOCK_HOOK_P4(NotiServiceProxyMock,
      rpc_port_proxy_noti_service_invoke_update_dnd_allow_exception,
      h, type, value, uid);
}

extern "C" int rpc_port_proxy_noti_service_invoke_register_dnd_app(
    rpc_port_proxy_noti_service_h h, int uid, int pid) {
  return MOCK_HOOK_P3(NotiServiceProxyMock,
      rpc_port_proxy_noti_service_invoke_register_dnd_app, h, uid, pid);
}

extern "C" int rpc_port_proxy_noti_service_invoke_unregister_dnd_app(
    rpc_port_proxy_noti_service_h h, int uid, int pid) {
  return MOCK_HOOK_P3(NotiServiceProxyMock,
      rpc_port_proxy_noti_service_invoke_unregister_dnd_app, h, uid, pid);
}

/* Invoke: events */
extern "C" int rpc_port_proxy_noti_service_invoke_send_event(
    rpc_port_proxy_noti_service_h h,
    rpc_port_proxy_notification_h notihandle, int type) {
  return MOCK_HOOK_P3(NotiServiceProxyMock,
      rpc_port_proxy_noti_service_invoke_send_event,
      h, notihandle, type);
}

extern "C" int rpc_port_proxy_noti_service_invoke_send_event_by_priv_id(
    rpc_port_proxy_noti_service_h h, int priv_id, int type) {
  return MOCK_HOOK_P3(NotiServiceProxyMock,
      rpc_port_proxy_noti_service_invoke_send_event_by_priv_id,
      h, priv_id, type);
}

extern "C" int rpc_port_proxy_noti_service_invoke_check_event_receiver(
    rpc_port_proxy_noti_service_h h, int priv_id, bool* available) {
  return MOCK_HOOK_P3(NotiServiceProxyMock,
      rpc_port_proxy_noti_service_invoke_check_event_receiver,
      h, priv_id, available);
}

extern "C" int rpc_port_proxy_noti_service_invoke_reset_event_handler(
    rpc_port_proxy_noti_service_h h, int priv_id) {
  return MOCK_HOOK_P2(NotiServiceProxyMock,
      rpc_port_proxy_noti_service_invoke_reset_event_handler, h, priv_id);
}

/* Invoke: channels */
extern "C" int rpc_port_proxy_noti_service_invoke_channel_insert(
    rpc_port_proxy_noti_service_h h, const char* app_id,
    const char* channel_name, bool blockable, bool is_blocked, int uid) {
  return MOCK_HOOK_P6(NotiServiceProxyMock,
      rpc_port_proxy_noti_service_invoke_channel_insert,
      h, app_id, channel_name, blockable, is_blocked, uid);
}

extern "C" int rpc_port_proxy_noti_service_invoke_channel_delete(
    rpc_port_proxy_noti_service_h h, const char* app_id,
    const char* channel_name, int uid) {
  return MOCK_HOOK_P4(NotiServiceProxyMock,
      rpc_port_proxy_noti_service_invoke_channel_delete,
      h, app_id, channel_name, uid);
}

extern "C" int rpc_port_proxy_noti_service_invoke_channel_update(
    rpc_port_proxy_noti_service_h h, const char* app_id,
    const char* channel_name, bool blockable, bool is_blocked, int uid) {
  return MOCK_HOOK_P6(NotiServiceProxyMock,
      rpc_port_proxy_noti_service_invoke_channel_update,
      h, app_id, channel_name, blockable, is_blocked, uid);
}

extern "C" int rpc_port_proxy_noti_service_invoke_channel_get(
    rpc_port_proxy_noti_service_h h, const char* app_id,
    const char* channel_name, bool* blockable, bool* is_blocked, int uid) {
  return MOCK_HOOK_P6(NotiServiceProxyMock,
      rpc_port_proxy_noti_service_invoke_channel_get,
      h, app_id, channel_name, blockable, is_blocked, uid);
}

extern "C" int rpc_port_proxy_noti_service_invoke_channel_get_list(
    rpc_port_proxy_noti_service_h h, const char* app_id, int uid,
    rpc_port_proxy_list_noti_channel_h* channel_list) {
  return MOCK_HOOK_P4(NotiServiceProxyMock,
      rpc_port_proxy_noti_service_invoke_channel_get_list,
      h, app_id, uid, channel_list);
}
