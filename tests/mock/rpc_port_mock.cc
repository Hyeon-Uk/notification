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

RpcPortMock::RpcPortMock() = default;
RpcPortMock::~RpcPortMock() = default;

/* Raw proxy create/destroy */
extern "C" int rpc_port_proxy_create(rpc_port_proxy_h* h) {
  return MOCK_HOOK_P1(RpcPortMock, rpc_port_proxy_create, h);
}

extern "C" int rpc_port_proxy_destroy(rpc_port_proxy_h h) {
  return MOCK_HOOK_P1(RpcPortMock, rpc_port_proxy_destroy, h);
}

/* Proxy lifecycle / events */
extern "C" int rpc_port_proxy_connect_sync(rpc_port_proxy_h h,
    const char* appid, const char* port_name) {
  return MOCK_HOOK_P3(RpcPortMock, rpc_port_proxy_connect_sync,
      h, appid, port_name);
}

extern "C" int rpc_port_proxy_add_connected_event_cb(rpc_port_proxy_h h,
    rpc_port_proxy_connected_event_cb cb, void* user_data) {
  return MOCK_HOOK_P3(RpcPortMock, rpc_port_proxy_add_connected_event_cb,
      h, cb, user_data);
}

extern "C" int rpc_port_proxy_add_disconnected_event_cb(rpc_port_proxy_h h,
    rpc_port_proxy_disconnected_event_cb cb, void* user_data) {
  return MOCK_HOOK_P3(RpcPortMock,
      rpc_port_proxy_add_disconnected_event_cb, h, cb, user_data);
}

extern "C" int rpc_port_proxy_add_rejected_event_cb(rpc_port_proxy_h h,
    rpc_port_proxy_rejected_event_cb cb, void* user_data) {
  return MOCK_HOOK_P3(RpcPortMock,
      rpc_port_proxy_add_rejected_event_cb, h, cb, user_data);
}

extern "C" int rpc_port_proxy_add_received_event_cb(rpc_port_proxy_h h,
    rpc_port_proxy_received_event_cb cb, void* user_data) {
  return MOCK_HOOK_P3(RpcPortMock,
      rpc_port_proxy_add_received_event_cb, h, cb, user_data);
}

/* Port I/O */
extern "C" int rpc_port_disconnect(rpc_port_h h) {
  return MOCK_HOOK_P1(RpcPortMock, rpc_port_disconnect, h);
}

extern "C" int rpc_port_read(rpc_port_h h, void* buf, unsigned int size) {
  return MOCK_HOOK_P3(RpcPortMock, rpc_port_read, h, buf, size);
}

extern "C" int rpc_port_write(rpc_port_h h, const void* buf,
    unsigned int size) {
  return MOCK_HOOK_P3(RpcPortMock, rpc_port_write, h, buf, size);
}

/* Parcel lifecycle */
extern "C" int rpc_port_parcel_create(rpc_port_parcel_h* h) {
  return MOCK_HOOK_P1(RpcPortMock, rpc_port_parcel_create, h);
}

extern "C" int rpc_port_parcel_create_from_port(rpc_port_parcel_h* h,
    rpc_port_h port) {
  return MOCK_HOOK_P2(RpcPortMock, rpc_port_parcel_create_from_port,
      h, port);
}

extern "C" int rpc_port_parcel_destroy(rpc_port_parcel_h h) {
  return MOCK_HOOK_P1(RpcPortMock, rpc_port_parcel_destroy, h);
}

extern "C" int rpc_port_parcel_send(rpc_port_parcel_h h,
    rpc_port_h port) {
  return MOCK_HOOK_P2(RpcPortMock, rpc_port_parcel_send, h, port);
}

/* Parcel header */
extern "C" int rpc_port_parcel_get_header(rpc_port_parcel_h h,
    rpc_port_parcel_header_h* header) {
  return MOCK_HOOK_P2(RpcPortMock, rpc_port_parcel_get_header, h, header);
}

extern "C" int rpc_port_parcel_header_set_tag(rpc_port_parcel_header_h h,
    const char* tag) {
  return MOCK_HOOK_P2(RpcPortMock, rpc_port_parcel_header_set_tag,
      h, tag);
}

extern "C" int rpc_port_parcel_header_get_tag(rpc_port_parcel_header_h h,
    char** tag) {
  return MOCK_HOOK_P2(RpcPortMock, rpc_port_parcel_header_get_tag,
      h, tag);
}

extern "C" int rpc_port_parcel_header_get_seq_num(
    rpc_port_parcel_header_h h, int* seq) {
  return MOCK_HOOK_P2(RpcPortMock, rpc_port_parcel_header_get_seq_num,
      h, seq);
}

extern "C" int rpc_port_parcel_header_set_seq_num(
    rpc_port_parcel_header_h h, int seq) {
  return MOCK_HOOK_P2(RpcPortMock, rpc_port_parcel_header_set_seq_num,
      h, seq);
}

/* Parcel write primitives */
extern "C" int rpc_port_parcel_write_int32(rpc_port_parcel_h h, int i) {
  return MOCK_HOOK_P2(RpcPortMock, rpc_port_parcel_write_int32, h, i);
}

extern "C" int rpc_port_parcel_write_int64(rpc_port_parcel_h h,
    long long i) {
  return MOCK_HOOK_P2(RpcPortMock, rpc_port_parcel_write_int64, h, i);
}

extern "C" int rpc_port_parcel_write_string(rpc_port_parcel_h h,
    const char* str) {
  return MOCK_HOOK_P2(RpcPortMock, rpc_port_parcel_write_string, h, str);
}

extern "C" int rpc_port_parcel_write_bool(rpc_port_parcel_h h, bool b) {
  return MOCK_HOOK_P2(RpcPortMock, rpc_port_parcel_write_bool, h, b);
}

extern "C" int rpc_port_parcel_write_bundle(rpc_port_parcel_h h,
    bundle* b) {
  return MOCK_HOOK_P2(RpcPortMock, rpc_port_parcel_write_bundle, h, b);
}

/* Parcel read primitives */
extern "C" int rpc_port_parcel_read_int32(rpc_port_parcel_h h, int* i) {
  return MOCK_HOOK_P2(RpcPortMock, rpc_port_parcel_read_int32, h, i);
}

extern "C" int rpc_port_parcel_read_int64(rpc_port_parcel_h h,
    long long* i) {
  return MOCK_HOOK_P2(RpcPortMock, rpc_port_parcel_read_int64, h, i);
}

extern "C" int rpc_port_parcel_read_string(rpc_port_parcel_h h,
    char** str) {
  return MOCK_HOOK_P2(RpcPortMock, rpc_port_parcel_read_string, h, str);
}

extern "C" int rpc_port_parcel_read_bool(rpc_port_parcel_h h, bool* b) {
  return MOCK_HOOK_P2(RpcPortMock, rpc_port_parcel_read_bool, h, b);
}

extern "C" int rpc_port_parcel_read_bundle(rpc_port_parcel_h h,
    bundle** b) {
  return MOCK_HOOK_P2(RpcPortMock, rpc_port_parcel_read_bundle, h, b);
}
