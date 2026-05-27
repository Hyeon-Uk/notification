/*
 * Copyright (c) 2026 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef TESTS_SMOKE_TESTS_COMMON_SMOKE_UTILS_H_
#define TESTS_SMOKE_TESTS_COMMON_SMOKE_UTILS_H_

#include <glib.h>
#include <notification.h>

#include <atomic>
#include <string>

namespace smoke {

constexpr const char* kSmokeAppId = "org.tizen.notification.smoketest";
constexpr guint kCbTimeoutMs = 3000;

/* Delete every notification this app owns for both types so each test
 * starts from a known state. */
void ResetNotifications();

/* Count notifications belonging to this app for the given type. */
int CountByType(notification_type_e type);

/* Build a populated notification with sane defaults; tests can mutate
 * before posting. */
notification_h MakeNoti(notification_type_e type,
                        const std::string& title,
                        const std::string& content);

/* Run a glib mainloop with a timeout. The cb argument is captured by the
 * caller; this helper just runs the loop. Returns true if the loop was
 * quit before the timeout. */
class MainLoopRunner {
 public:
  MainLoopRunner();
  ~MainLoopRunner();
  MainLoopRunner(const MainLoopRunner&) = delete;
  MainLoopRunner& operator=(const MainLoopRunner&) = delete;

  /* Run until Quit() is called or timeout_ms elapses. Returns true on
   * normal quit, false on timeout. */
  bool RunUntil(guint timeout_ms);
  void Quit();

  GMainLoop* loop() const { return loop_; }

 private:
  GMainLoop* loop_;
  std::atomic<bool> finished_{false};
};

}  // namespace smoke

#endif  // TESTS_SMOKE_TESTS_COMMON_SMOKE_UTILS_H_
