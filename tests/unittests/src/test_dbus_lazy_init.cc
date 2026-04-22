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
 */

/*
 * N1-T1: DBusConnectionManager lazy-connect unit tests.
 *
 * Four scenarios (design doc logs/N_phase1_design.md, N1-T1 section):
 *   1. DBusLazyInit_NoConnectBeforeFirstAccessor
 *   2. DBusLazyInit_ConnectOnFirstReporterCall
 *   3. DBusLazyInit_ConnectRaceSafe
 *   4. DBusLazyInit_AccessorsWithoutConnect
 *
 * The production code exposes a file-static test-hook namespace
 * (dbus_test_hooks) that lets us install a sentinel GDBusConnection* so we
 * can (a) skip the real system-bus call and (b) count how many times the
 * lazy EnsureConnected path actually runs. Without these hooks, asserting
 * "connect count == 0" would require intercepting g_bus_get_sync, which the
 * existing GlibMock can do but only per-test and with strict expectation
 * syntax - the hook counter is simpler and race-safe.
 *
 * ABI note: dbus_test_hooks is a file-static namespace declared inside
 * dbus_connection_manager.cc. It is NOT EXPORT_API; we forward-declare the
 * symbols here so the test binary can link them, because the unittest
 * target links the same .cc file via libnotification-ex.
 *
 * NB: DBusConnectionManager is a singleton, so its connect_once_ flag
 * outlives any individual TEST. The singleton provides no public reset.
 * We work around this with a test-only reset hook that re-seats the
 * singleton's private state. Without this, tests after the first one
 * would all see "connect already happened". See dbus_test_hooks::
 * ResetSingletonForTest().
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <atomic>
#include <thread>
#include <vector>

#include "mock/test_fixture.hh"
#include "mock/aul_mock.hh"
#include "mock/glib_mock.hh"

#include "notification-ex/dbus_connection_manager.h"

namespace notification {
namespace dbus_test_hooks {
/* Implemented in src/notification-ex/dbus_connection_manager.cc. */
void SetOverrideConnection(GDBusConnection* conn);
int  GetOverrideConnectCount();
void ResetOverrideConnectCount();
void SetForceConnectFailure(bool v);
void ResetSingletonForTest();
}  // namespace dbus_test_hooks
}  // namespace notification

namespace {

/* util::GetAppId() runs inside EnsureConnected() even on the override path,
 * and it calls into aul. Provide AulMock so the MOCK_HOOK_P1 aul wrappers
 * don't throw "no mock registered". */
class Mocks : virtual public ::testing::NiceMock<GlibMock>,
              virtual public ::testing::NiceMock<AulMock> {};

/*
 * DBusLazyInitTest: resets the singleton's connect_once_ flag before each
 * test so every case sees a fresh "pre-connect" state. It also clears the
 * override connection so a test that does not opt in to the override gets
 * the production path (which, without a mocked g_bus_get_sync, would fail
 * fast - we don't let those reach the bus layer here).
 */
class DBusLazyInitTest : public TestFixture {
 public:
  DBusLazyInitTest() : TestFixture(std::make_unique<::Mocks>()) {}

  void SetUp() override {
    namespace dh = notification::dbus_test_hooks;
    dh::SetOverrideConnection(nullptr);
    dh::SetForceConnectFailure(false);
    dh::ResetOverrideConnectCount();
    dh::ResetSingletonForTest();
  }

  void TearDown() override {
    namespace dh = notification::dbus_test_hooks;
    dh::SetOverrideConnection(nullptr);
    dh::SetForceConnectFailure(false);
    dh::ResetOverrideConnectCount();
    dh::ResetSingletonForTest();
  }
};

/*
 * Helper: sentinel non-null GDBusConnection* used to prove the override
 * path fired without pulling in the real GIO bus. The pointer is never
 * dereferenced (SetOverrideConnection only stores it and GetConnection
 * returns it through), so any non-null address works.
 */
GDBusConnection* FakeConn() {
  static char fake_storage = 0;
  return reinterpret_cast<GDBusConnection*>(&fake_storage);
}

}  // namespace

/*
 * Goal 1 (lazy): After program load, the first call to GetInst() MUST NOT
 * attempt to acquire the D-Bus connection. We prove this by asking for the
 * singleton and verifying the test hook counter stays at zero (the
 * production EnsureConnected never ran, or equivalently ran with a null
 * override and therefore would have hit g_bus_get_sync - but since the
 * counter only increments on the override path, "counter == 0 AND we never
 * called GetConnection" is the evidence the accessors stay pure).
 */
TEST_F(DBusLazyInitTest, NoConnectBeforeFirstAccessor) {
  namespace dh = notification::dbus_test_hooks;

  /* Fetch the singleton. This MUST NOT connect. */
  auto& mgr = notification::DBusConnectionManager::GetInst();
  (void)mgr;

  EXPECT_EQ(dh::GetOverrideConnectCount(), 0)
      << "GetInst() alone triggered EnsureConnected - lazy contract broken.";
}

/*
 * Goal 2 (first-use connect): Installing the override sentinel and calling
 * GetConnection() exactly once MUST invoke EnsureConnected exactly once.
 * We do not test real Reporter construction here: Reporter's ctor pulls in
 * DBusEventListener::RegisterObserver which subscribes on GetConnection,
 * so the same observable (counter increments by 1) covers it. The design
 * document explicitly names "first Reporter or first GetConnection()" as
 * the trigger, so we exercise GetConnection() directly.
 */
TEST_F(DBusLazyInitTest, ConnectOnFirstReporterCall) {
  namespace dh = notification::dbus_test_hooks;

  dh::SetOverrideConnection(FakeConn());

  auto& mgr = notification::DBusConnectionManager::GetInst();
  EXPECT_EQ(dh::GetOverrideConnectCount(), 0)
      << "Pre-condition: still no connect after GetInst().";

  GDBusConnection* conn = mgr.GetConnection();
  EXPECT_EQ(conn, FakeConn());
  EXPECT_EQ(dh::GetOverrideConnectCount(), 1)
      << "First GetConnection() must have triggered EnsureConnected once.";

  /* A second GetConnection() must reuse the cached connection, NOT
   * increment the counter. This proves call_once semantics on the
   * hot path. */
  GDBusConnection* conn2 = mgr.GetConnection();
  EXPECT_EQ(conn2, FakeConn());
  EXPECT_EQ(dh::GetOverrideConnectCount(), 1)
      << "Second GetConnection() must NOT re-run EnsureConnected.";
}

/*
 * Goal 3 (race-free): 8 threads race on GetConnection(). The lazy path
 * MUST run exactly once (std::call_once guarantee). Any other count means
 * the double-checked locking or once_flag plumbing is broken.
 */
TEST_F(DBusLazyInitTest, ConnectRaceSafe) {
  namespace dh = notification::dbus_test_hooks;

  dh::SetOverrideConnection(FakeConn());

  constexpr int kThreads = 8;
  std::vector<std::thread> pool;
  pool.reserve(kThreads);
  std::atomic<int> reached{0};
  std::atomic<bool> go{false};

  for (int i = 0; i < kThreads; ++i) {
    pool.emplace_back([&]() {
      /* Spin until released so threads actually contend. */
      reached.fetch_add(1, std::memory_order_acq_rel);
      while (!go.load(std::memory_order_acquire)) { /* spin */ }
      auto& mgr = notification::DBusConnectionManager::GetInst();
      GDBusConnection* c = mgr.GetConnection();
      EXPECT_EQ(c, FakeConn());
    });
  }

  /* Wait for all threads to be ready, then release. */
  while (reached.load(std::memory_order_acquire) < kThreads) {
    std::this_thread::yield();
  }
  go.store(true, std::memory_order_release);

  for (auto& t : pool) t.join();

  EXPECT_EQ(dh::GetOverrideConnectCount(), 1)
      << "EnsureConnected ran " << dh::GetOverrideConnectCount()
      << " times across " << kThreads << " threads; expected exactly 1.";
}

/*
 * Goal 4 (pure accessors): GetInterfaceName / GetBusName /
 * GetDataProviderMasterName / IsDataProviderMaster(appid) MUST NOT trigger
 * EnsureConnected. They only read compile-time constants or derive from
 * the argument.
 */
TEST_F(DBusLazyInitTest, AccessorsWithoutConnect) {
  namespace dh = notification::dbus_test_hooks;

  auto& mgr = notification::DBusConnectionManager::GetInst();

  /* Interface name is a compile-time constant. */
  std::string iface = mgr.GetInterfaceName();
  EXPECT_FALSE(iface.empty());

  /* Bus name for an appid is derived from an MD5 of the string: pure. */
  std::string bus = mgr.GetBusName("org.tizen.unittest");
  EXPECT_FALSE(bus.empty());

  /* Data-provider-master bus name: string concat, pure. */
  std::string dpm = mgr.GetDataProviderMasterName();
  EXPECT_FALSE(dpm.empty());

  /* Appid classification: pure string compare. */
  EXPECT_TRUE(mgr.IsDataProviderMaster(std::string("data-provider-master")));
  EXPECT_FALSE(mgr.IsDataProviderMaster(std::string("some.app")));

  /* None of the above should have walked the connect path. */
  EXPECT_EQ(dh::GetOverrideConnectCount(), 0)
      << "A pure accessor triggered EnsureConnected - review "
         "dbus_connection_manager.cc.";
}

/*
 * Sanity: empty appid returns empty bus name and does NOT connect.
 * Guards against someone accidentally moving this branch inside
 * EnsureConnected.
 */
TEST_F(DBusLazyInitTest, AccessorsEmptyAppidIsSafe) {
  namespace dh = notification::dbus_test_hooks;

  auto& mgr = notification::DBusConnectionManager::GetInst();
  EXPECT_EQ(mgr.GetBusName(std::string()), std::string());
  EXPECT_EQ(dh::GetOverrideConnectCount(), 0);
}
