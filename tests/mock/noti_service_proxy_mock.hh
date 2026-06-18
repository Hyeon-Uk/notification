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

#ifndef UNIT_TESTS_MOCK_NOTI_SERVICE_PROXY_MOCK_H_
#define UNIT_TESTS_MOCK_NOTI_SERVICE_PROXY_MOCK_H_

#include <gmock/gmock.h>
#include <notification_tidl_proxy.h>

#include "module_mock.hh"

class NotiServiceProxyMock : public virtual ModuleMock {
 public:
  NotiServiceProxyMock();
  virtual ~NotiServiceProxyMock();

  /* Proxy lifecycle */
  MOCK_METHOD(int, rpc_port_proxy_noti_service_create,
      (const char*, rpc_port_proxy_noti_service_callback_s*, void*,
       rpc_port_proxy_noti_service_h*));
  MOCK_METHOD(int, rpc_port_proxy_noti_service_destroy,
      (rpc_port_proxy_noti_service_h));
  MOCK_METHOD(int, rpc_port_proxy_noti_service_connect_sync,
      (rpc_port_proxy_noti_service_h));

  /* Changed callback handle */
  MOCK_METHOD(int, rpc_port_proxy_noti_service_changed_create,
      (rpc_port_proxy_noti_service_changed_h*));
  MOCK_METHOD(int, rpc_port_proxy_noti_service_changed_destroy,
      (rpc_port_proxy_noti_service_changed_h));
  MOCK_METHOD(int, rpc_port_proxy_noti_service_changed_dispose,
      (rpc_port_proxy_noti_service_h,
       rpc_port_proxy_noti_service_changed_h));
  MOCK_METHOD(int, rpc_port_proxy_noti_service_changed_set_callback,
      (rpc_port_proxy_noti_service_changed_h,
       rpc_port_proxy_noti_service_changed_cb, void*));

  /* Event-received callback handle */
  MOCK_METHOD(int, rpc_port_proxy_noti_service_event_received_create,
      (rpc_port_proxy_noti_service_event_received_h*));
  MOCK_METHOD(int, rpc_port_proxy_noti_service_event_received_destroy,
      (rpc_port_proxy_noti_service_event_received_h));
  MOCK_METHOD(int, rpc_port_proxy_noti_service_event_received_dispose,
      (rpc_port_proxy_noti_service_h,
       rpc_port_proxy_noti_service_event_received_h));
  MOCK_METHOD(int, rpc_port_proxy_noti_service_event_received_set_callback,
      (rpc_port_proxy_noti_service_event_received_h,
       rpc_port_proxy_noti_service_event_received_cb, void*));

  /* Invoke: register/unregister */
  MOCK_METHOD(int, rpc_port_proxy_noti_service_invoke_register_changed,
      (rpc_port_proxy_noti_service_h,
       rpc_port_proxy_noti_service_changed_h, int));
  MOCK_METHOD(int, rpc_port_proxy_noti_service_invoke_unregister_changed,
      (rpc_port_proxy_noti_service_h));
  MOCK_METHOD(int, rpc_port_proxy_noti_service_invoke_register_event,
      (rpc_port_proxy_noti_service_h,
       rpc_port_proxy_noti_service_event_received_h));
  MOCK_METHOD(int, rpc_port_proxy_noti_service_invoke_unregister_event,
      (rpc_port_proxy_noti_service_h));
  MOCK_METHOD(int, rpc_port_proxy_noti_service_invoke_register_service,
      (rpc_port_proxy_noti_service_h, int));

  /* Invoke: notification CRUD */
  MOCK_METHOD(int, rpc_port_proxy_noti_service_invoke_add_noti,
      (rpc_port_proxy_noti_service_h, rpc_port_proxy_notification_h, int*));
  MOCK_METHOD(int, rpc_port_proxy_noti_service_invoke_update_noti,
      (rpc_port_proxy_noti_service_h, rpc_port_proxy_notification_h, int*));
  MOCK_METHOD(int, rpc_port_proxy_noti_service_invoke_refresh_noti,
      (rpc_port_proxy_noti_service_h, int));
  MOCK_METHOD(int, rpc_port_proxy_noti_service_invoke_delete_single_noti,
      (rpc_port_proxy_noti_service_h, const char*, int, int));
  MOCK_METHOD(int, rpc_port_proxy_noti_service_invoke_delete_multiple_noti,
      (rpc_port_proxy_noti_service_h, const char*, int, int, int*));
  MOCK_METHOD(int,
      rpc_port_proxy_noti_service_invoke_delete_noti_by_display_applist,
      (rpc_port_proxy_noti_service_h, int, int));

  /* Invoke: notification queries */
  MOCK_METHOD(int, rpc_port_proxy_noti_service_invoke_get_noti_count,
      (rpc_port_proxy_noti_service_h, int, const char*, int, int, int,
       int*));
  MOCK_METHOD(int, rpc_port_proxy_noti_service_invoke_get_all_noti_count,
      (rpc_port_proxy_noti_service_h, int, int, int*));
  MOCK_METHOD(int, rpc_port_proxy_noti_service_invoke_load_noti_by_tag,
      (rpc_port_proxy_noti_service_h, const char*, const char*, int,
       rpc_port_proxy_notification_h*));
  MOCK_METHOD(int, rpc_port_proxy_noti_service_invoke_load_noti_by_priv_id,
      (rpc_port_proxy_noti_service_h, const char*, int, int,
       rpc_port_proxy_notification_h*));
  MOCK_METHOD(int,
      rpc_port_proxy_noti_service_invoke_load_noti_grouping_list,
      (rpc_port_proxy_noti_service_h, int, int, int, int,
       rpc_port_proxy_list_notification_h*));
  MOCK_METHOD(int, rpc_port_proxy_noti_service_invoke_load_noti_detail_list,
      (rpc_port_proxy_noti_service_h, const char*, int, int, int, int,
       rpc_port_proxy_list_notification_h*));

  /* Invoke: settings */
  MOCK_METHOD(int, rpc_port_proxy_noti_service_invoke_update_noti_setting,
      (rpc_port_proxy_noti_service_h, rpc_port_proxy_noti_setting_h, int));
  MOCK_METHOD(int,
      rpc_port_proxy_noti_service_invoke_update_noti_sys_setting,
      (rpc_port_proxy_noti_service_h,
       rpc_port_proxy_noti_system_setting_h, int));
  MOCK_METHOD(int, rpc_port_proxy_noti_service_invoke_get_setting_array,
      (rpc_port_proxy_noti_service_h, int,
       rpc_port_proxy_array_noti_setting_h*));
  MOCK_METHOD(int, rpc_port_proxy_noti_service_invoke_get_setting_by_app_id,
      (rpc_port_proxy_noti_service_h, const char*, int,
       rpc_port_proxy_noti_setting_h*));
  MOCK_METHOD(int, rpc_port_proxy_noti_service_invoke_load_system_setting,
      (rpc_port_proxy_noti_service_h, int,
       rpc_port_proxy_noti_system_setting_h*));

  /* Invoke: templates */
  MOCK_METHOD(int, rpc_port_proxy_noti_service_invoke_save_as_template,
      (rpc_port_proxy_noti_service_h, rpc_port_proxy_notification_h,
       const char*));
  MOCK_METHOD(int, rpc_port_proxy_noti_service_invoke_create_from_template,
      (rpc_port_proxy_noti_service_h, const char*,
       rpc_port_proxy_notification_h*));
  MOCK_METHOD(int,
      rpc_port_proxy_noti_service_invoke_create_from_package_template,
      (rpc_port_proxy_noti_service_h, const char*, const char*,
       rpc_port_proxy_notification_h*));

  /* Invoke: DND / block state */
  MOCK_METHOD(int, rpc_port_proxy_noti_service_invoke_get_noti_block_state,
      (rpc_port_proxy_noti_service_h, const char*, int, int*, int*, int*));
  MOCK_METHOD(int,
      rpc_port_proxy_noti_service_invoke_update_dnd_allow_exception,
      (rpc_port_proxy_noti_service_h, int, int, int));
  MOCK_METHOD(int, rpc_port_proxy_noti_service_invoke_register_dnd_app,
      (rpc_port_proxy_noti_service_h, int, int));
  MOCK_METHOD(int, rpc_port_proxy_noti_service_invoke_unregister_dnd_app,
      (rpc_port_proxy_noti_service_h, int, int));

  /* Invoke: events */
  MOCK_METHOD(int, rpc_port_proxy_noti_service_invoke_send_event,
      (rpc_port_proxy_noti_service_h, rpc_port_proxy_notification_h, int));
  MOCK_METHOD(int,
      rpc_port_proxy_noti_service_invoke_send_event_by_priv_id,
      (rpc_port_proxy_noti_service_h, int, int));
  MOCK_METHOD(int, rpc_port_proxy_noti_service_invoke_check_event_receiver,
      (rpc_port_proxy_noti_service_h, int, bool*));
  MOCK_METHOD(int, rpc_port_proxy_noti_service_invoke_reset_event_handler,
      (rpc_port_proxy_noti_service_h, int));

  /* Invoke: channels */
  MOCK_METHOD(int, rpc_port_proxy_noti_service_invoke_channel_insert,
      (rpc_port_proxy_noti_service_h, const char*, const char*, bool, bool,
       int));
  MOCK_METHOD(int, rpc_port_proxy_noti_service_invoke_channel_delete,
      (rpc_port_proxy_noti_service_h, const char*, const char*, int));
  MOCK_METHOD(int, rpc_port_proxy_noti_service_invoke_channel_update,
      (rpc_port_proxy_noti_service_h, const char*, const char*, bool, bool,
       int));
  MOCK_METHOD(int, rpc_port_proxy_noti_service_invoke_channel_get,
      (rpc_port_proxy_noti_service_h, const char*, const char*, bool*,
       bool*, int));
  MOCK_METHOD(int, rpc_port_proxy_noti_service_invoke_channel_get_list,
      (rpc_port_proxy_noti_service_h, const char*, int,
       rpc_port_proxy_list_noti_channel_h*));

  /* List notification iteration / lifecycle */
  MOCK_METHOD(int, rpc_port_proxy_list_notification_foreach,
      (rpc_port_proxy_list_notification_h,
       rpc_port_proxy_list_notification_cb, void*));
  MOCK_METHOD(int, rpc_port_proxy_list_notification_destroy,
      (rpc_port_proxy_list_notification_h));
};

#endif  // UNIT_TESTS_MOCK_NOTI_SERVICE_PROXY_MOCK_H_
