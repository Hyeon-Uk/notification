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

#include "rpc_port_mock.hh"

#include "mock_hook.hh"
#include "test_fixture.hh"

/*
 * extern "C" wrappers route the tidlc-generated proxy ABI to the
 * RpcPortMock instance held by the active TestFixture.  Wrapper bodies
 * mirror sqlite_mock.cc verbatim in style.
 */

extern "C" int rpc_port_proxy_noti_service_create(
    const char* arg0, rpc_port_proxy_noti_service_callback_s* arg1,
    void* arg2, rpc_port_proxy_noti_service_h* arg3) {
  return MOCK_HOOK_P4(RpcPortMock,
      rpc_port_proxy_noti_service_create, arg0, arg1, arg2, arg3);
}

extern "C" int rpc_port_proxy_noti_service_destroy(
    rpc_port_proxy_noti_service_h arg0) {
  return MOCK_HOOK_P1(RpcPortMock,
      rpc_port_proxy_noti_service_destroy, arg0);
}

extern "C" int rpc_port_proxy_noti_service_connect(
    rpc_port_proxy_noti_service_h arg0) {
  return MOCK_HOOK_P1(RpcPortMock,
      rpc_port_proxy_noti_service_connect, arg0);
}

extern "C" int rpc_port_proxy_noti_service_connect_sync(
    rpc_port_proxy_noti_service_h arg0) {
  return MOCK_HOOK_P1(RpcPortMock,
      rpc_port_proxy_noti_service_connect_sync, arg0);
}

extern "C" int rpc_port_proxy_noti_service_disconnect(
    rpc_port_proxy_noti_service_h arg0) {
  return MOCK_HOOK_P1(RpcPortMock,
      rpc_port_proxy_noti_service_disconnect, arg0);
}

/* changed delegate */

extern "C" int rpc_port_proxy_noti_service_changed_create(
    rpc_port_proxy_noti_service_changed_h* arg0) {
  return MOCK_HOOK_P1(RpcPortMock,
      rpc_port_proxy_noti_service_changed_create, arg0);
}

extern "C" int rpc_port_proxy_noti_service_changed_destroy(
    rpc_port_proxy_noti_service_changed_h arg0) {
  return MOCK_HOOK_P1(RpcPortMock,
      rpc_port_proxy_noti_service_changed_destroy, arg0);
}

extern "C" int rpc_port_proxy_noti_service_changed_set_callback(
    rpc_port_proxy_noti_service_changed_h arg0,
    rpc_port_proxy_noti_service_changed_cb arg1, void* arg2) {
  return MOCK_HOOK_P3(RpcPortMock,
      rpc_port_proxy_noti_service_changed_set_callback,
      arg0, arg1, arg2);
}

extern "C" int rpc_port_proxy_noti_service_changed_dispose(
    rpc_port_proxy_noti_service_h arg0,
    rpc_port_proxy_noti_service_changed_h arg1) {
  return MOCK_HOOK_P2(RpcPortMock,
      rpc_port_proxy_noti_service_changed_dispose, arg0, arg1);
}

/* event_received delegate */

extern "C" int rpc_port_proxy_noti_service_event_received_create(
    rpc_port_proxy_noti_service_event_received_h* arg0) {
  return MOCK_HOOK_P1(RpcPortMock,
      rpc_port_proxy_noti_service_event_received_create, arg0);
}

extern "C" int rpc_port_proxy_noti_service_event_received_destroy(
    rpc_port_proxy_noti_service_event_received_h arg0) {
  return MOCK_HOOK_P1(RpcPortMock,
      rpc_port_proxy_noti_service_event_received_destroy, arg0);
}

extern "C" int rpc_port_proxy_noti_service_event_received_set_callback(
    rpc_port_proxy_noti_service_event_received_h arg0,
    rpc_port_proxy_noti_service_event_received_cb arg1, void* arg2) {
  return MOCK_HOOK_P3(RpcPortMock,
      rpc_port_proxy_noti_service_event_received_set_callback,
      arg0, arg1, arg2);
}

extern "C" int rpc_port_proxy_noti_service_event_received_dispose(
    rpc_port_proxy_noti_service_h arg0,
    rpc_port_proxy_noti_service_event_received_h arg1) {
  return MOCK_HOOK_P2(RpcPortMock,
      rpc_port_proxy_noti_service_event_received_dispose, arg0, arg1);
}

/* representative invoke_* */

extern "C" int rpc_port_proxy_noti_service_invoke_register_service(
    rpc_port_proxy_noti_service_h arg0, int arg1) {
  return MOCK_HOOK_P2(RpcPortMock,
      rpc_port_proxy_noti_service_invoke_register_service, arg0, arg1);
}
