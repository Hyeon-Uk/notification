/*
 * Copyright (c) 2026 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 */

/*
 * bench_dbus_rtt
 * --------------
 * Measures a GDBus round-trip to org.freedesktop.DBus.Peer.Ping. This is a
 * stand-in for the notification broker RTT described in plan item P9
 * (D-Bus RTT p50/p99). On sandboxed hosts with no session bus we fall
 * back to skipping so the bench binary still exits 0.
 */

#include <benchmark/benchmark.h>
#include <gio/gio.h>

namespace {

GDBusConnection* g_conn = nullptr;

void EnsureConn(benchmark::State& state) {
  if (g_conn) return;
  GError* err = nullptr;
  g_conn = g_bus_get_sync(G_BUS_TYPE_SESSION, nullptr, &err);
  if (!g_conn) {
    state.SkipWithError("No session D-Bus available");
    if (err) g_error_free(err);
  }
}

void BM_DbusPingRTT(benchmark::State& state) {
  EnsureConn(state);
  if (!g_conn) return;

  for (auto _ : state) {
    GError* err = nullptr;
    GVariant* reply = g_dbus_connection_call_sync(
        g_conn,
        "org.freedesktop.DBus",
        "/org/freedesktop/DBus",
        "org.freedesktop.DBus.Peer",
        "Ping",
        nullptr,
        nullptr,
        G_DBUS_CALL_FLAGS_NONE,
        1000,
        nullptr,
        &err);
    if (reply) g_variant_unref(reply);
    if (err) {
      g_error_free(err);
      break;
    }
  }
  state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_DbusPingRTT)->Unit(benchmark::kMicrosecond);

}  // namespace

BENCHMARK_MAIN();
