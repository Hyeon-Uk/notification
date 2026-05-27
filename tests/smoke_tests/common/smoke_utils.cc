/*
 * Copyright (c) 2026 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#include "common/smoke_utils.h"

#include <notification_internal.h>
#include <notification_list.h>

namespace smoke {

void ResetNotifications() {
  notification_delete_all_by_type(kSmokeAppId, NOTIFICATION_TYPE_NOTI);
  notification_delete_all_by_type(kSmokeAppId, NOTIFICATION_TYPE_ONGOING);
}

int CountByType(notification_type_e type) {
  notification_list_h list = nullptr;
  if (notification_get_list(type, -1, &list) != NOTIFICATION_ERROR_NONE)
    return 0;
  int count = 0;
  notification_list_h iter = notification_list_get_head(list);
  while (iter != nullptr) {
    notification_h n = notification_list_get_data(iter);
    char* caller = nullptr;
    if (n != nullptr &&
        notification_get_pkgname(n, &caller) == NOTIFICATION_ERROR_NONE &&
        caller != nullptr &&
        std::string(caller) == kSmokeAppId) {
      count++;
    }
    iter = notification_list_get_next(iter);
  }
  notification_free_list(list);
  return count;
}

notification_h MakeNoti(notification_type_e type,
                        const std::string& title,
                        const std::string& content) {
  notification_h n = notification_create(type);
  if (n == nullptr) return nullptr;
  notification_set_text(n, NOTIFICATION_TEXT_TYPE_TITLE,
      title.c_str(), nullptr, NOTIFICATION_VARIABLE_TYPE_NONE);
  notification_set_text(n, NOTIFICATION_TEXT_TYPE_CONTENT,
      content.c_str(), nullptr, NOTIFICATION_VARIABLE_TYPE_NONE);
  notification_set_app_id(n, kSmokeAppId);
  return n;
}

MainLoopRunner::MainLoopRunner() : loop_(g_main_loop_new(nullptr, FALSE)) {}

MainLoopRunner::~MainLoopRunner() { g_main_loop_unref(loop_); }

namespace {
gboolean OnTimeout(gpointer data) {
  auto* runner = static_cast<MainLoopRunner*>(data);
  runner->Quit();
  return G_SOURCE_REMOVE;
}
}  // namespace

bool MainLoopRunner::RunUntil(guint timeout_ms) {
  finished_ = false;
  guint tag = g_timeout_add(timeout_ms, OnTimeout, this);
  g_main_loop_run(loop_);
  if (tag > 0)
    g_source_remove(tag);
  return finished_.load();
}

void MainLoopRunner::Quit() {
  finished_ = true;
  if (g_main_loop_is_running(loop_))
    g_main_loop_quit(loop_);
}

}  // namespace smoke
