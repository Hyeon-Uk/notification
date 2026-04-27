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
 * N4-T1: Manager lazy-register unit tests.
 *
 * Pre-N4-T1 the Manager ctor performed two cold-start operations:
 *   1. listener_->RegisterObserver(parent_)  (~20-50 ms via D-Bus)
 *   2. sender_->RequestReturnValue(EventInfo::Register, ...)  (~5-15 ms RPC)
 * Both are now deferred to the first user-facing entry point via
 * EnsureRegistered() under std::call_once.
 *
 * Four scenarios:
 *   NoRegisterBeforeFirstCall   ctor MUST NOT call RegisterObserver
 *                               nor RequestReturnValue.
 *   RegisterOnFirstCall         first Update() (or any entry point)
 *                               triggers exactly one Register +
 *                               one EventInfo::Register RPC.
 *   RegisterRaceSafe            8 threads race on the first call -
 *                               exactly one Register + one Register
 *                               RPC observed.
 *   DestroyWithoutRegister      Manager ctor + immediate dtor MUST
 *                               NOT call UnRegisterObserver nor send
 *                               an EventInfo::Unregister RPC.
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <atomic>
#include <list>
#include <memory>
#include <thread>
#include <vector>

#include "mock/aul_mock.hh"
#include "mock/app_common_mock.hh"
#include "mock/glib_mock.hh"
#include "mock/test_fixture.hh"

#include "notification-ex/manager.h"
#include "notification-ex/event_info_internal.h"
#include "notification-ex/event_sender_interface.h"
#include "notification-ex/event_listener_interface.h"
#include "notification-ex/event_observer_interface.h"
#include "notification-ex/text_item.h"

using ::tizen_base::Bundle;
using ::notification::Manager;
using ::notification::EventInfo;
using ::notification::IEventInfo;
using ::notification::IEventSender;
using ::notification::IEventListener;
using ::notification::IEventObserver;
using ::notification::item::AbstractItem;
using ::notification::item::TextItem;

namespace {

class CountingSender : public IEventSender {
 public:
  std::atomic<int> notify_count{0};
  std::atomic<int> send_message_count{0};
  std::atomic<int> request_count{0};
  std::atomic<int> request_return_count{0};
  /* Tracks Register / Unregister RPCs separately so we can assert the
   * Manager-specific contract (Register once, Unregister once when
   * registered). */
  std::atomic<int> register_rpc_count{0};
  std::atomic<int> unregister_rpc_count{0};

  void Notify(const IEventInfo& /*info*/,
      std::list<Bundle> /*serialized*/,
      std::string /*dest_appid*/ = "") override {
    notify_count.fetch_add(1, std::memory_order_relaxed);
  }
  void SendMessage(const IEventInfo& /*info*/,
      std::list<Bundle> /*serialized*/,
      std::string /*dest_appid*/ = "") override {
    send_message_count.fetch_add(1, std::memory_order_relaxed);
  }
  std::list<Bundle> Request(const IEventInfo& /*info*/) override {
    request_count.fetch_add(1, std::memory_order_relaxed);
    std::list<Bundle> r;
    r.emplace_back();
    return r;
  }
  int RequestReturnValue(const IEventInfo& info) override {
    request_return_count.fetch_add(1, std::memory_order_relaxed);
    if (info.GetEventType() == EventInfo::Register)
      register_rpc_count.fetch_add(1, std::memory_order_relaxed);
    else if (info.GetEventType() == EventInfo::Unregister)
      unregister_rpc_count.fetch_add(1, std::memory_order_relaxed);
    return 0;
  }
};

class CountingListener : public IEventListener {
 public:
  std::atomic<int> register_count{0};
  std::atomic<int> unregister_count{0};

  void RegisterObserver(IEventObserver* /*observer*/) override {
    register_count.fetch_add(1, std::memory_order_relaxed);
  }
  void UnRegisterObserver(IEventObserver* /*observer*/) override {
    unregister_count.fetch_add(1, std::memory_order_relaxed);
  }
  void NotifyObserver(const IEventInfo& /*info*/,
      std::list<Bundle> /*serialized*/) override {}
  std::list<Bundle> NotifyObserver(const IEventInfo& /*info*/) override {
    return {};
  }
  int NotifyNumberRequest(const IEventInfo& /*info*/) override { return 0; }
};

class Mocks
    : virtual public ::testing::NiceMock<GlibMock>,
      virtual public ::testing::NiceMock<AulMock>,
      virtual public ::testing::NiceMock<AppCommonMock> {};

class ManagerLazyRegisterTest : public TestFixture {
 public:
  ManagerLazyRegisterTest() : TestFixture(std::make_unique<::Mocks>()) {}

  std::unique_ptr<Manager> MakeManager(
      CountingSender** out_sender, CountingListener** out_listener,
      const std::string& receiver_group = "") {
    auto sender = std::make_unique<CountingSender>();
    auto listener = std::make_unique<CountingListener>();
    *out_sender = sender.get();
    *out_listener = listener.get();
    return std::make_unique<Manager>(
        std::move(sender), std::move(listener), receiver_group);
  }
};

std::shared_ptr<AbstractItem> MakeItem() {
  return std::make_shared<TextItem>("noti_id", "hello");
}

}  // namespace

/*
 * Goal 1 (lazy): ctor must NOT trigger Register / RegisterRPC. */
TEST_F(ManagerLazyRegisterTest, NoRegisterBeforeFirstCall) {
  CountingSender* sender = nullptr;
  CountingListener* listener = nullptr;
  auto manager = MakeManager(&sender, &listener);

  EXPECT_EQ(listener->register_count.load(), 0)
      << "Manager ctor invoked RegisterObserver - lazy contract broken.";
  EXPECT_EQ(sender->register_rpc_count.load(), 0)
      << "Manager ctor sent EventInfo::Register RPC - lazy contract broken.";
  EXPECT_EQ(sender->request_return_count.load(), 0);
}

/*
 * Goal 2 (first-use register): the first Update() (or any entry
 * point) MUST trigger exactly one RegisterObserver AND exactly one
 * EventInfo::Register RPC. A second call MUST reuse them.
 */
TEST_F(ManagerLazyRegisterTest, RegisterOnFirstCall) {
  CountingSender* sender = nullptr;
  CountingListener* listener = nullptr;
  auto manager = MakeManager(&sender, &listener);

  ASSERT_EQ(listener->register_count.load(), 0);
  ASSERT_EQ(sender->register_rpc_count.load(), 0);

  manager->Update(MakeItem());
  EXPECT_EQ(listener->register_count.load(), 1)
      << "First Update() must register observer exactly once.";
  EXPECT_EQ(sender->register_rpc_count.load(), 1)
      << "First Update() must emit one Register RPC.";

  manager->Update(MakeItem());
  EXPECT_EQ(listener->register_count.load(), 1);
  EXPECT_EQ(sender->register_rpc_count.load(), 1);
}

/*
 * Goal 3 (race-free): 8 threads racing on first call MUST observe
 * exactly one Register + one Register RPC. */
TEST_F(ManagerLazyRegisterTest, RegisterRaceSafe) {
  CountingSender* sender = nullptr;
  CountingListener* listener = nullptr;
  auto manager = MakeManager(&sender, &listener);

  constexpr int kThreads = 8;
  std::vector<std::thread> pool;
  pool.reserve(kThreads);
  std::atomic<int> reached{0};
  std::atomic<bool> go{false};

  for (int i = 0; i < kThreads; ++i) {
    pool.emplace_back([&]() {
      reached.fetch_add(1, std::memory_order_acq_rel);
      while (!go.load(std::memory_order_acquire)) { /* spin */ }
      manager->Update(MakeItem());
    });
  }

  while (reached.load(std::memory_order_acquire) < kThreads)
    std::this_thread::yield();
  go.store(true, std::memory_order_release);

  for (auto& t : pool) t.join();

  EXPECT_EQ(listener->register_count.load(), 1)
      << "RegisterObserver ran " << listener->register_count.load()
      << " times; expected exactly 1.";
  EXPECT_EQ(sender->register_rpc_count.load(), 1)
      << "Register RPC fired " << sender->register_rpc_count.load()
      << " times; expected exactly 1.";
}

/*
 * Goal 4 (no-Register no-Unregister): Manager ctor + immediate
 * destroy MUST NOT call UnRegisterObserver nor send Unregister RPC.
 *
 * Strategy: install thread_local counters that survive past the
 * Manager's owned-pointer destruction. The CountingSender /
 * CountingListener defined at the top of this file already aggregate
 * into instance counters; here we use a NoOp variant that bumps a
 * test-scoped global so we can read it after the unique_ptr destroys
 * the wrapped object. */
TEST_F(ManagerLazyRegisterTest, DestroyWithoutRegister) {
  /* Test-scoped counters we can read AFTER Manager dtor releases
   * the owned listener / sender. */
  static std::atomic<int> g_register_count{0};
  static std::atomic<int> g_unregister_count{0};
  static std::atomic<int> g_register_rpc_count{0};
  static std::atomic<int> g_unregister_rpc_count{0};
  g_register_count.store(0);
  g_unregister_count.store(0);
  g_register_rpc_count.store(0);
  g_unregister_rpc_count.store(0);

  class GlobalSender : public IEventSender {
   public:
    void Notify(const IEventInfo&, std::list<Bundle>,
        std::string = "") override {}
    void SendMessage(const IEventInfo&, std::list<Bundle>,
        std::string = "") override {}
    std::list<Bundle> Request(const IEventInfo&) override {
      std::list<Bundle> r; r.emplace_back(); return r;
    }
    int RequestReturnValue(const IEventInfo& info) override {
      if (info.GetEventType() == EventInfo::Register)
        g_register_rpc_count.fetch_add(1);
      else if (info.GetEventType() == EventInfo::Unregister)
        g_unregister_rpc_count.fetch_add(1);
      return 0;
    }
  };
  class GlobalListener : public IEventListener {
   public:
    void RegisterObserver(IEventObserver*) override {
      g_register_count.fetch_add(1);
    }
    void UnRegisterObserver(IEventObserver*) override {
      g_unregister_count.fetch_add(1);
    }
    void NotifyObserver(const IEventInfo&, std::list<Bundle>) override {}
    std::list<Bundle> NotifyObserver(const IEventInfo&) override {
      return {};
    }
    int NotifyNumberRequest(const IEventInfo&) override { return 0; }
  };

  {
    Manager manager(
        std::make_unique<GlobalSender>(),
        std::make_unique<GlobalListener>(), "");
    /* Destroy without ever calling an entry point. */
  }

  EXPECT_EQ(g_register_count.load(), 0)
      << "Manager ctor called RegisterObserver - lazy contract broken.";
  EXPECT_EQ(g_unregister_count.load(), 0)
      << "Manager dtor called UnRegisterObserver despite no Register.";
  EXPECT_EQ(g_register_rpc_count.load(), 0)
      << "Manager ctor sent Register RPC - lazy contract broken.";
  EXPECT_EQ(g_unregister_rpc_count.load(), 0)
      << "Manager dtor sent Unregister RPC despite no Register.";
}

/*
 * Sanity 5: every Manager external entry point must trigger
 * EnsureRegistered. This guards against a future contributor adding
 * a new public method without the Ensure call.
 */
TEST_F(ManagerLazyRegisterTest, AllEntryPointsRegister) {
  /* Delete */
  {
    CountingSender* s = nullptr;
    CountingListener* l = nullptr;
    auto m = MakeManager(&s, &l);
    ASSERT_EQ(l->register_count.load(), 0);
    m->Delete(MakeItem());
    EXPECT_EQ(l->register_count.load(), 1);
  }
  /* DeleteAll */
  {
    CountingSender* s = nullptr;
    CountingListener* l = nullptr;
    auto m = MakeManager(&s, &l);
    ASSERT_EQ(l->register_count.load(), 0);
    m->DeleteAll();
    EXPECT_EQ(l->register_count.load(), 1);
  }
  /* GetCount */
  {
    CountingSender* s = nullptr;
    CountingListener* l = nullptr;
    auto m = MakeManager(&s, &l);
    ASSERT_EQ(l->register_count.load(), 0);
    (void)m->GetCount();
    EXPECT_EQ(l->register_count.load(), 1);
  }
  /* Hide */
  {
    CountingSender* s = nullptr;
    CountingListener* l = nullptr;
    auto m = MakeManager(&s, &l);
    ASSERT_EQ(l->register_count.load(), 0);
    m->Hide(MakeItem());
    EXPECT_EQ(l->register_count.load(), 1);
  }
}
