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

#ifndef UNIT_TESTS_MOCK_RPC_PORT_STUB_TYPES_H_
#define UNIT_TESTS_MOCK_RPC_PORT_STUB_TYPES_H_

/*
 * Compile-time shim for opaque rpc-port proxy types.
 *
 * The real definitions live in notification_tidl_proxy.h, which is
 * generated at build time by tidlc and is only available inside the
 * production source tree.  Test-only translation units that need to
 * provide extern "C" wrappers for the proxy ABI do NOT see that
 * generated header.  This shim provides enough type information for
 * those wrappers to compile -- empty struct bodies, opaque from the
 * caller's perspective.
 *
 * Production builds never include this header.
 */

#ifdef __cplusplus
extern "C" {
#endif

/* lifecycle handle */
struct rpc_port_proxy_noti_service_s { int _stub; };
typedef struct rpc_port_proxy_noti_service_s *rpc_port_proxy_noti_service_h;

/* delegate handles */
struct rpc_port_proxy_noti_service_changed_s { int _stub; };
typedef struct rpc_port_proxy_noti_service_changed_s
    *rpc_port_proxy_noti_service_changed_h;

struct rpc_port_proxy_noti_service_event_received_s { int _stub; };
typedef struct rpc_port_proxy_noti_service_event_received_s
    *rpc_port_proxy_noti_service_event_received_h;

/* connected/disconnected and per-delegate callback signatures.  Only
 * forward-typedef-d here; the test code that supplies a value casts a
 * function pointer of the matching production signature into these. */
typedef void (*rpc_port_proxy_noti_service_connected_cb)(
    rpc_port_proxy_noti_service_h h, void *user_data);
typedef void (*rpc_port_proxy_noti_service_disconnected_cb)(
    rpc_port_proxy_noti_service_h h, void *user_data);
typedef void (*rpc_port_proxy_noti_service_rejected_cb)(
    rpc_port_proxy_noti_service_h h, void *user_data);

typedef struct rpc_port_proxy_noti_service_callback_s {
  rpc_port_proxy_noti_service_connected_cb connected;
  rpc_port_proxy_noti_service_disconnected_cb disconnected;
  rpc_port_proxy_noti_service_rejected_cb rejected;
} rpc_port_proxy_noti_service_callback_s;

typedef void (*rpc_port_proxy_noti_service_changed_cb)(void *user_data);
typedef void (*rpc_port_proxy_noti_service_event_received_cb)(
    void *user_data);

#ifdef __cplusplus
}
#endif

#endif  /* UNIT_TESTS_MOCK_RPC_PORT_STUB_TYPES_H_ */
