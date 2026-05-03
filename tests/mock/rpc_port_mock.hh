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

#ifndef UNIT_TESTS_MOCK_RPC_PORT_MOCK_H_
#define UNIT_TESTS_MOCK_RPC_PORT_MOCK_H_

#include <gmock/gmock.h>

#include "module_mock.hh"
#include "rpc_port_stub_types.hh"

/*
 * Seed mock for the tidlc-generated noti_service proxy ABI.  Surface is
 * intentionally minimal: lifecycle (create/destroy/connect[_sync]/
 * disconnect), the two delegate registration paths
 * (changed_*, event_received_*), and one representative invoke_*.
 *
 * Extension to the remaining 38 invoke_* and the data-struct
 * setters/getters is deferred to Phase 1.5-E and is added on demand
 * (matching the precedent set by sqlite_mock.cc, which grew
 * sqlite3_table_column_metadata in commit e03305e once a consumer
 * test demanded it).
 */
class RpcPortMock : public virtual ModuleMock {
 public:
  virtual ~RpcPortMock() {}

  /* lifecycle */
  MOCK_METHOD4(rpc_port_proxy_noti_service_create,
      int(const char*, rpc_port_proxy_noti_service_callback_s*,
          void*, rpc_port_proxy_noti_service_h*));
  MOCK_METHOD1(rpc_port_proxy_noti_service_destroy,
      int(rpc_port_proxy_noti_service_h));
  MOCK_METHOD1(rpc_port_proxy_noti_service_connect,
      int(rpc_port_proxy_noti_service_h));
  MOCK_METHOD1(rpc_port_proxy_noti_service_connect_sync,
      int(rpc_port_proxy_noti_service_h));
  MOCK_METHOD1(rpc_port_proxy_noti_service_disconnect,
      int(rpc_port_proxy_noti_service_h));

  /* "changed" delegate */
  MOCK_METHOD1(rpc_port_proxy_noti_service_changed_create,
      int(rpc_port_proxy_noti_service_changed_h*));
  MOCK_METHOD1(rpc_port_proxy_noti_service_changed_destroy,
      int(rpc_port_proxy_noti_service_changed_h));
  MOCK_METHOD3(rpc_port_proxy_noti_service_changed_set_callback,
      int(rpc_port_proxy_noti_service_changed_h,
          rpc_port_proxy_noti_service_changed_cb, void*));
  MOCK_METHOD2(rpc_port_proxy_noti_service_changed_dispose,
      int(rpc_port_proxy_noti_service_h,
          rpc_port_proxy_noti_service_changed_h));

  /* "event_received" delegate */
  MOCK_METHOD1(rpc_port_proxy_noti_service_event_received_create,
      int(rpc_port_proxy_noti_service_event_received_h*));
  MOCK_METHOD1(rpc_port_proxy_noti_service_event_received_destroy,
      int(rpc_port_proxy_noti_service_event_received_h));
  MOCK_METHOD3(rpc_port_proxy_noti_service_event_received_set_callback,
      int(rpc_port_proxy_noti_service_event_received_h,
          rpc_port_proxy_noti_service_event_received_cb, void*));
  MOCK_METHOD2(rpc_port_proxy_noti_service_event_received_dispose,
      int(rpc_port_proxy_noti_service_h,
          rpc_port_proxy_noti_service_event_received_h));

  /* representative invoke_* (proves the wrapper pattern) */
  MOCK_METHOD2(rpc_port_proxy_noti_service_invoke_register_service,
      int(rpc_port_proxy_noti_service_h, int));
};

#endif  /* UNIT_TESTS_MOCK_RPC_PORT_MOCK_H_ */
