/*
 * Copyright (c) 2019 Samsung Electronics Co., Ltd.
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

#ifndef NOTIFICATION_EX_REPORTER_IMPLEMENTATION_H_
#define NOTIFICATION_EX_REPORTER_IMPLEMENTATION_H_

#include <atomic>
#include <list>
#include <memory>
#include <mutex>
#include <string>

#include "notification-ex/reporter.h"

namespace notification {

class Reporter::Impl {
 public:
  virtual ~Impl();

 private:
  Impl(Reporter* parent, std::unique_ptr<IEventSender> sender,
      std::unique_ptr<IEventListener> listener);

  /*
   * N4-T1: lazy observer registration. The original ctor invoked
   * listener_->RegisterObserver(parent_) which, for the production
   * DBusEventListener, triggers g_bus_get_sync + SubscribeSignal on the
   * cold-start path (~20-50 ms). EnsureRegistered() defers that work to
   * the first user-facing entry point (Post / Update / Delete / Find* /
   * SendEvent / SetEventCb), guarded by std::call_once so concurrent
   * callers race-free converge on a single RegisterObserver call.
   *
   * registered_ is a separate atomic flag used by ~Impl to decide
   * whether UnRegisterObserver must run. We cannot consult once_flag
   * directly (no public observer in std::call_once), and we must NOT
   * invoke UnRegisterObserver when no Register ever happened (would be
   * a NOP for DBusEventListener but a contract violation for arbitrary
   * IEventListener implementations).
   */
  void EnsureRegistered();

 private:
  friend class Reporter;
  std::unique_ptr<IEventSender> sender_;
  std::unique_ptr<IEventListener> listener_;
  Reporter* parent_;
  std::once_flag register_once_;
  std::atomic<bool> registered_{false};
};

}  // namespace notification

#endif  // NOTIFICATION_EX_REPORTER_IMPLEMENTATION_H_
