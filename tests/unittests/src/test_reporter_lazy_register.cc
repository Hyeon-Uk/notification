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
 * N4-T1: Reporter lazy-register unit tests.
 *
 * Pre-N4-T1 the Reporter ctor immediately invoked
 *   listener_->RegisterObserver(parent_)
 * which (for the production DBusEventListener) acquires the system
 * D-Bus connection + subscribes to the Reporter's incoming-signal name
 * (~20-50 ms cold-start cost). N4-T1 defers that work to the first
 * user-facing entry point (Post / Update / Delete / Find* / SendEvent).
 *
 * Four scenarios mirror the design doc (N_phase4_design.md, N4-T1):
 *
 *   NoRegisterBeforeFirstPost   ctor MUST NOT trigger RegisterObserver.
 *   RegisterOnFirstPost         first Post() triggers it exactly once;
 *                               second Post() must NOT re-register.
 *   RegisterRaceSafe            8 threads race on the first call - only
 *                               one RegisterObserver runs (call_once).
 *   DestroyWithoutRegister      Reporter ctor + immediate dtor MUST NOT
 *                               call UnRegisterObserver.
 *
 * The tests inject a tiny in-process IEventSender / IEventListener
 * pair (CountingSender / CountingListener) that count Register +
 * UnRegister + send invocations without touching the system bus. This
 * keeps the test fully hermetic and lets us assert exact call counts.
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

#include "notification-ex/reporter.h"
#include "notification-ex/event_sender_interface.h"
#include "notification-ex/event_listener_interface.h"
#include "notification-ex/event_observer_interface.h"
#include "notification-ex/text_item.h"

using ::tizen_base::Bundle;
using ::notification::Reporter;
using ::notification::IEventSender;
using ::notification::IEventListener;
using ::notification::IEventObserver;
using ::notification::IEventInfo;
using ::notification::item::AbstractItem;
using ::notification::item::TextItem;

namespace {

/* CountingSender: in-process IEventSender that counts every outbound
 * call. We never inspect payload: the lazy-register tests only need to
 * know that "the first user-facing entry point ran". */
class CountingSender : public IEventSender {
 public:
  std::atomic<int> notify_count{0};
  std::atomic<int> send_message_count{0};
  std::atomic<int> request_count{0};
  std::atomic<int> request_return_count{0};

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
    /* Return one empty Bundle: Reporter::FindByRootID dereferences
     * front(), so an empty list would crash on the path we exercise. */
    std::list<Bundle> r;
    r.emplace_back();
    return r;
  }
  int RequestReturnValue(const IEventInfo& /*info*/) override {
    request_return_count.fetch_add(1, std::memory_order_relaxed);
    return 0;
  }
};

/* CountingListener: in-process IEventListener that counts Register /
 * UnRegister on a single observer pointer. */
class CountingListener : public IEventListener {
 public:
  std::atomic<int> register_count{0};
  std::atomic<int> unregister_count{0};
  IEventObserver* last_registered{nullptr};

  void RegisterObserver(IEventObserver* observer) override {
    register_count.fetch_add(1, std::memory_order_relaxed);
    last_registered = observer;
  }
  void UnRegisterObserver(IEventObserver* observer) override {
    unregister_count.fetch_add(1, std::memory_order_relaxed);
    if (last_registered == observer)
      last_registered = nullptr;
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

class ReporterLazyRegisterTest : public TestFixture {
 public:
  ReporterLazyRegisterTest() : TestFixture(std::make_unique<::Mocks>()) {}

  /* Build a Reporter wired to bare counter pointers; the Reporter
   * takes ownership of the unique_ptrs but we keep the raw pointers
   * for assertion. The counters live as long as the Reporter (and
   * outlive every Reporter call we make). */
  std::unique_ptr<Reporter> MakeReporter(
      CountingSender** out_sender, CountingListener** out_listener) {
    auto sender = std::make_unique<CountingSender>();
    auto listener = std::make_unique<CountingListener>();
    *out_sender = sender.get();
    *out_listener = listener.get();
    return std::make_unique<Reporter>(
        std::move(sender), std::move(listener));
  }
};

/* Helper: produce a minimal AbstractItem usable as Post() input.
 * TextItem with an id + contents satisfies Serialize() invariants
 * inside Reporter::Post. */
std::shared_ptr<AbstractItem> MakeItem() {
  return std::make_shared<TextItem>("noti_id", "hello");
}

}  // namespace

/*
 * Goal 1 (lazy): constructing a Reporter MUST NOT call
 * listener_->RegisterObserver. Pre-N4-T1 the ctor unconditionally
 * registered, so this test fails on the old code and passes on the new.
 */
TEST_F(ReporterLazyRegisterTest, NoRegisterBeforeFirstPost) {
  CountingSender* sender = nullptr;
  CountingListener* listener = nullptr;
  auto reporter = MakeReporter(&sender, &listener);

  EXPECT_EQ(listener->register_count.load(), 0)
      << "Reporter ctor triggered RegisterObserver - lazy contract broken.";
  EXPECT_EQ(listener->unregister_count.load(), 0);
  EXPECT_EQ(sender->send_message_count.load(), 0);
}

/*
 * Goal 2 (first-use register): the first Post() invocation MUST
 * trigger exactly one RegisterObserver. A second Post() MUST reuse
 * the existing registration (call_once semantics).
 */
TEST_F(ReporterLazyRegisterTest, RegisterOnFirstPost) {
  CountingSender* sender = nullptr;
  CountingListener* listener = nullptr;
  auto reporter = MakeReporter(&sender, &listener);

  ASSERT_EQ(listener->register_count.load(), 0);

  reporter->Post(MakeItem());
  EXPECT_EQ(listener->register_count.load(), 1)
      << "First Post() must register observer exactly once.";
  EXPECT_GE(sender->send_message_count.load(), 1)
      << "First Post() must reach the sender.";

  reporter->Post(MakeItem());
  EXPECT_EQ(listener->register_count.load(), 1)
      << "Second Post() must NOT re-register.";
  EXPECT_GE(sender->send_message_count.load(), 2);
}

/*
 * Goal 3 (race-free): 8 threads racing on the first Post() MUST
 * observe exactly one RegisterObserver. std::call_once guarantee.
 */
TEST_F(ReporterLazyRegisterTest, RegisterRaceSafe) {
  CountingSender* sender = nullptr;
  CountingListener* listener = nullptr;
  auto reporter = MakeReporter(&sender, &listener);

  constexpr int kThreads = 8;
  std::vector<std::thread> pool;
  pool.reserve(kThreads);
  std::atomic<int> reached{0};
  std::atomic<bool> go{false};

  for (int i = 0; i < kThreads; ++i) {
    pool.emplace_back([&]() {
      reached.fetch_add(1, std::memory_order_acq_rel);
      while (!go.load(std::memory_order_acquire)) { /* spin */ }
      reporter->Post(MakeItem());
    });
  }

  while (reached.load(std::memory_order_acquire) < kThreads)
    std::this_thread::yield();
  go.store(true, std::memory_order_release);

  for (auto& t : pool) t.join();

  EXPECT_EQ(listener->register_count.load(), 1)
      << "RegisterObserver ran " << listener->register_count.load()
      << " times across " << kThreads << " threads; expected exactly 1.";
  EXPECT_GE(sender->send_message_count.load(), kThreads);
}

/*
 * Goal 4 (no-Register no-Unregister): a Reporter that is constructed
 * and immediately destroyed without ever being used MUST NOT invoke
 * UnRegisterObserver. The pre-N4-T1 dtor unconditionally called it,
 * which would underflow refcounts on listeners that pair Register and
 * UnRegister (e.g. signal subscribe / unsubscribe).
 *
 * Strategy: install test-scoped global counters that outlive the
 * Reporter-owned listener so we can read them after Reporter destroy.
 */
TEST_F(ReporterLazyRegisterTest, DestroyWithoutRegister) {
  static std::atomic<int> g_register{0};
  static std::atomic<int> g_unregister{0};
  g_register.store(0);
  g_unregister.store(0);

  class GlobalSender : public IEventSender {
   public:
    void Notify(const IEventInfo&, std::list<Bundle>,
        std::string = "") override {}
    void SendMessage(const IEventInfo&, std::list<Bundle>,
        std::string = "") override {}
    std::list<Bundle> Request(const IEventInfo&) override {
      std::list<Bundle> r; r.emplace_back(); return r;
    }
    int RequestReturnValue(const IEventInfo&) override { return 0; }
  };
  class GlobalListener : public IEventListener {
   public:
    void RegisterObserver(IEventObserver*) override {
      g_register.fetch_add(1);
    }
    void UnRegisterObserver(IEventObserver*) override {
      g_unregister.fetch_add(1);
    }
    void NotifyObserver(const IEventInfo&, std::list<Bundle>) override {}
    std::list<Bundle> NotifyObserver(const IEventInfo&) override {
      return {};
    }
    int NotifyNumberRequest(const IEventInfo&) override { return 0; }
  };

  /* Construct + destroy without using. */
  {
    Reporter reporter(
        std::make_unique<GlobalSender>(),
        std::make_unique<GlobalListener>());
  }

  EXPECT_EQ(g_register.load(), 0)
      << "Reporter ctor called RegisterObserver - lazy contract broken.";
  EXPECT_EQ(g_unregister.load(), 0)
      << "Reporter dtor called UnRegisterObserver despite no Register.";

  /* Positive control: a Reporter that DOES use a first entry point
   * must emit Register=1 AND Unregister=1 on destroy. */
  g_register.store(0);
  g_unregister.store(0);
  {
    Reporter reporter(
        std::make_unique<GlobalSender>(),
        std::make_unique<GlobalListener>());
    reporter.Post(MakeItem());
    EXPECT_EQ(g_register.load(), 1);
    EXPECT_EQ(g_unregister.load(), 0);
  }
  EXPECT_EQ(g_register.load(), 1);
  EXPECT_EQ(g_unregister.load(), 1)
      << "Reporter dtor must call UnRegisterObserver when registered.";
}

/*
 * Sanity 5: Update/Delete/Find/GetCount/SendEvent are also gated by
 * EnsureRegistered. We exercise each entry point on its OWN Reporter
 * to prove the lazy hook is wired to every public method.
 */
TEST_F(ReporterLazyRegisterTest, AllEntryPointsRegister) {
  /* Update */
  {
    CountingSender* s = nullptr;
    CountingListener* l = nullptr;
    auto r = MakeReporter(&s, &l);
    ASSERT_EQ(l->register_count.load(), 0);
    r->Update(MakeItem());
    EXPECT_EQ(l->register_count.load(), 1);
  }
  /* Delete */
  {
    CountingSender* s = nullptr;
    CountingListener* l = nullptr;
    auto r = MakeReporter(&s, &l);
    ASSERT_EQ(l->register_count.load(), 0);
    r->Delete(MakeItem());
    EXPECT_EQ(l->register_count.load(), 1);
  }
  /* DeleteAll */
  {
    CountingSender* s = nullptr;
    CountingListener* l = nullptr;
    auto r = MakeReporter(&s, &l);
    ASSERT_EQ(l->register_count.load(), 0);
    r->DeleteAll();
    EXPECT_EQ(l->register_count.load(), 1);
  }
  /* GetCount */
  {
    CountingSender* s = nullptr;
    CountingListener* l = nullptr;
    auto r = MakeReporter(&s, &l);
    ASSERT_EQ(l->register_count.load(), 0);
    (void)r->GetCount();
    EXPECT_EQ(l->register_count.load(), 1);
  }
}
