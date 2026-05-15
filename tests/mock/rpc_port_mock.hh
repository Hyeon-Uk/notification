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

#ifndef UNIT_TESTS_MOCK_RPC_PORT_MOCK_HH_
#define UNIT_TESTS_MOCK_RPC_PORT_MOCK_HH_

#include <gmock/gmock.h>
#include <rpc-port.h>
#include <rpc-port-parcel.h>

#include "module_mock.hh"

class RpcPortMock : public virtual ModuleMock {
 public:
  RpcPortMock();
  virtual ~RpcPortMock();

  /* Raw proxy create/destroy */
  MOCK_METHOD(int, rpc_port_proxy_create, (rpc_port_proxy_h*));
  MOCK_METHOD(int, rpc_port_proxy_destroy, (rpc_port_proxy_h));

  /* Proxy lifecycle / events */
  MOCK_METHOD(int, rpc_port_proxy_connect_sync,
      (rpc_port_proxy_h, const char*, const char*));
  MOCK_METHOD(int, rpc_port_proxy_add_connected_event_cb,
      (rpc_port_proxy_h, rpc_port_proxy_connected_event_cb, void*));
  MOCK_METHOD(int, rpc_port_proxy_add_disconnected_event_cb,
      (rpc_port_proxy_h, rpc_port_proxy_disconnected_event_cb, void*));
  MOCK_METHOD(int, rpc_port_proxy_add_rejected_event_cb,
      (rpc_port_proxy_h, rpc_port_proxy_rejected_event_cb, void*));
  MOCK_METHOD(int, rpc_port_proxy_add_received_event_cb,
      (rpc_port_proxy_h, rpc_port_proxy_received_event_cb, void*));

  /* Port I/O */
  MOCK_METHOD(int, rpc_port_disconnect, (rpc_port_h));
  MOCK_METHOD(int, rpc_port_read, (rpc_port_h, void*, unsigned int));
  MOCK_METHOD(int, rpc_port_write,
      (rpc_port_h, const void*, unsigned int));

  /* Parcel lifecycle */
  MOCK_METHOD(int, rpc_port_parcel_create, (rpc_port_parcel_h*));
  MOCK_METHOD(int, rpc_port_parcel_create_from_port,
      (rpc_port_parcel_h*, rpc_port_h));
  MOCK_METHOD(int, rpc_port_parcel_destroy, (rpc_port_parcel_h));
  MOCK_METHOD(int, rpc_port_parcel_send,
      (rpc_port_parcel_h, rpc_port_h));

  /* Parcel header */
  MOCK_METHOD(int, rpc_port_parcel_get_header,
      (rpc_port_parcel_h, rpc_port_parcel_header_h*));
  MOCK_METHOD(int, rpc_port_parcel_header_set_tag,
      (rpc_port_parcel_header_h, const char*));
  MOCK_METHOD(int, rpc_port_parcel_header_get_tag,
      (rpc_port_parcel_header_h, char**));
  MOCK_METHOD(int, rpc_port_parcel_header_get_seq_num,
      (rpc_port_parcel_header_h, int*));
  MOCK_METHOD(int, rpc_port_parcel_header_set_seq_num,
      (rpc_port_parcel_header_h, int));

  /* Parcel write primitives */
  MOCK_METHOD(int, rpc_port_parcel_write_int32,
      (rpc_port_parcel_h, int));
  MOCK_METHOD(int, rpc_port_parcel_write_int64,
      (rpc_port_parcel_h, long long));
  MOCK_METHOD(int, rpc_port_parcel_write_string,
      (rpc_port_parcel_h, const char*));
  MOCK_METHOD(int, rpc_port_parcel_write_bool,
      (rpc_port_parcel_h, bool));
  MOCK_METHOD(int, rpc_port_parcel_write_bundle,
      (rpc_port_parcel_h, bundle*));

  /* Parcel read primitives */
  MOCK_METHOD(int, rpc_port_parcel_read_int32,
      (rpc_port_parcel_h, int*));
  MOCK_METHOD(int, rpc_port_parcel_read_int64,
      (rpc_port_parcel_h, long long*));
  MOCK_METHOD(int, rpc_port_parcel_read_string,
      (rpc_port_parcel_h, char**));
  MOCK_METHOD(int, rpc_port_parcel_read_bool,
      (rpc_port_parcel_h, bool*));
  MOCK_METHOD(int, rpc_port_parcel_read_bundle,
      (rpc_port_parcel_h, bundle**));
};

#endif  // UNIT_TESTS_MOCK_RPC_PORT_MOCK_HH_
