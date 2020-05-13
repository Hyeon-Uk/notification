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

#include <dlog.h>
#include <glib.h>
#include <unistd.h>

#include <string>
#include <list>

#include "notification-ex/dbus_connection_manager.h"
#include "notification-ex/dbus_sender.h"
#include "notification-ex/dbus_sender_implementation.h"
#include "notification-ex/event_info_internal.h"
#include "notification-ex/ex_util.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "NOTIFICATION_EX"
#define MAX_PACKAGE_STR_SIZE 512

using namespace std;
using namespace tizen_base;
namespace notification {

DBusSender::DBusSender(string path) : impl_(new Impl(this, path)) {
  LOGW("Created (%s)", path.c_str());
}

DBusSender::Impl::~Impl() = default;

DBusSender::Impl::Impl(DBusSender* parent, string path)
    : path_(path), parent_(parent) {
  LOGI("Dbus_sender impl created");
}

DBusSender::~DBusSender() {
  LOGW("Destroyed");
}

bool DBusSender::Impl::EmitSignal(string bus_name, string signal_name,
    GVariant* data) {
  GError* err = NULL;
  gboolean result = TRUE;

  result = g_dbus_connection_emit_signal(
              DBusConnectionManager::GetInst().GetConnection(),
              bus_name.empty() ? NULL : bus_name.c_str(),
              path_.c_str(),
              DBusConnectionManager::GetInst().GetInterfaceName().c_str(),
              signal_name.c_str(), data, &err);
  if (result == FALSE) {
    LOGE("g_dbus_connection_emit_signal() is failed");
    if (err != NULL) {
      LOGE("g_dbus_connection_emit_signal() err : %s",
        err->message);
      g_error_free(err);
    }
  } else {
    LOGI("Successfully emit signal to %s, %s, %s",
      bus_name.c_str(), path_.c_str(), signal_name.c_str());
  }
  return result;
}

string DBusSender::Impl::GetBusName(string appid, string dest_appid) const {
  if (!DBusConnectionManager::GetInst().IsDataProviderMaster(appid))
    return DBusConnectionManager::GetInst().GetDataProviderMasterName();

  return DBusConnectionManager::GetInst().GetBusName(dest_appid);
}

void DBusSender::Notify(const IEventInfo& info, list<Bundle> serialized,
    string dest_appid) {
  string signal_name = EventInfo::GetString(info.GetEventType());
  string appid = util::GetAppId();

  GVariantBuilder* builder = g_variant_builder_new(G_VARIANT_TYPE("a(s)"));
  for (auto& i : serialized) {
    g_variant_builder_add(builder, "(s)",
        reinterpret_cast<char*>(i.ToRaw().first.get()));
  }

  GVariant* data = g_variant_new("(ssa(s))",
        appid.c_str(),
        reinterpret_cast<char*>(info.Serialize().ToRaw().first.get()), builder);
  string bus_name = impl_->GetBusName(appid, dest_appid);
  LOGI("bus name %s, %s", dest_appid.c_str(), bus_name.c_str());
  impl_->EmitSignal(bus_name, signal_name, data);
  g_variant_builder_unref(builder);
}

GDBusMessage* DBusSender::Impl::MethodCall(string appid, string method_name,
    Bundle serialized) {
  GDBusMessage* msg = g_dbus_message_new_method_call(
      DBusConnectionManager::GetInst().GetDataProviderMasterName().c_str(),
      path_.c_str(),
      DBusConnectionManager::GetInst().GetInterfaceName().c_str(),
      method_name.c_str());
  if (!msg) {
    LOGE("Can't allocate new method call");
    return nullptr;
  }

  g_dbus_message_set_body(msg,
      g_variant_new("(ss)",
        appid.c_str(),
        reinterpret_cast<char*>(serialized.ToRaw().first.get())
      )
  );

  LOGI("send message !! (%s) (%s) (%s)",
      path_.c_str(), method_name.c_str(), appid.c_str());

  GError* err = nullptr;
  GDBusMessage* reply = g_dbus_connection_send_message_with_reply_sync(
      DBusConnectionManager::GetInst().GetConnection(), msg,
      G_DBUS_SEND_MESSAGE_FLAGS_NONE, -1, NULL, NULL, &err);
  if (err != NULL) {
    LOGE("MethodCall, err[%s]", err->message);
    if (err->code == G_DBUS_ERROR_ACCESS_DENIED)
      set_last_result(ERROR_PERMISSION_DENIED);
    else
      set_last_result(ERROR_IO_ERROR);
    g_error_free(err);
  }

  return reply;
}

GDBusMessage* DBusSender::Impl::MethodCall(string appid, const IEventInfo& info,
    list<Bundle> serialized) {
  GDBusMessage* msg = g_dbus_message_new_method_call(
      DBusConnectionManager::GetInst().GetDataProviderMasterName().c_str(),
      path_.c_str(),
      DBusConnectionManager::GetInst().GetInterfaceName().c_str(),
      EventInfo::GetString(info.GetEventType()).c_str());
  if (!msg) {
    LOGE("Can't allocate new method call");
    return nullptr;
  }

  GVariantBuilder* builder = g_variant_builder_new(G_VARIANT_TYPE("a(s)"));
  for (auto& i : serialized) {
    g_variant_builder_add(builder, "(s)",
        reinterpret_cast<char*>(i.ToRaw().first.get()));
  }

  g_dbus_message_set_body(msg,
      g_variant_new("(ssa(s))",
          appid.c_str(),
          reinterpret_cast<char*>(info.Serialize().ToRaw().first.get()),
          builder
      )
  );

  LOGI("send message !! (%s) (%s) (%s)",
      path_.c_str(), EventInfo::GetString(info.GetEventType()).c_str(), appid.c_str());

  GError* err = nullptr;
  GDBusMessage* reply = g_dbus_connection_send_message_with_reply_sync(
      DBusConnectionManager::GetInst().GetConnection(), msg,
      G_DBUS_SEND_MESSAGE_FLAGS_NONE, -1, NULL, NULL, &err);
  if (err != NULL) {
    LOGE("MethodCall, err[%s]", err->message);
    if (err->code == G_DBUS_ERROR_ACCESS_DENIED)
      set_last_result(ERROR_PERMISSION_DENIED);
    else
      set_last_result(ERROR_IO_ERROR);
    g_error_free(err);
  }

  g_variant_builder_unref(builder);
  return reply;
}

int DBusSender::SendMessage(const IEventInfo& info, list<Bundle> serialized,
    string dest_appid) {
  string appid = info.GetOwner();
  if (appid.length() == 0)
    appid = util::GetAppId();

  GDBusMessage* reply = impl_->MethodCall(appid, info, serialized);
  if (!reply)
    return ERROR_IO_ERROR;

  int ret = ERROR_NONE;
  GError* err = nullptr;
  if (g_dbus_message_to_gerror(reply, &err)) {
    LOGE("SendMessage got error %s", err->message);
    if (err->code == G_DBUS_ERROR_ACCESS_DENIED) {
      set_last_result(ERROR_PERMISSION_DENIED);
      ret = ERROR_PERMISSION_DENIED;
    } else {
      set_last_result(ERROR_IO_ERROR);
      ret = ERROR_IO_ERROR;
    }
    g_error_free(err);
    return ret;
  } else {
    set_last_result(ERROR_NONE);
  }

  GVariant *reply_body = g_dbus_message_get_body(reply);
  g_variant_get(reply_body, "(i)", &ret);
  return ret;
}

std::list<Bundle> DBusSender::Request(const IEventInfo& info) {
  string appid = info.GetOwner();
  if (appid.length() == 0)
    appid = util::GetAppId();

  string method_name = EventInfo::GetString(info.GetEventType());
  Bundle serialized = info.Serialize();

  list<Bundle> ret_list;
  GDBusMessage* reply = impl_->MethodCall(appid, method_name, serialized);
  if (!reply)
    return ret_list;

  GError* err = nullptr;
  if (g_dbus_message_to_gerror(reply, &err)) {
    LOGE("Request got error %s", err->message);
    if (err->code == G_DBUS_ERROR_ACCESS_DENIED)
      set_last_result(ERROR_PERMISSION_DENIED);
    else
      set_last_result(ERROR_IO_ERROR);

    g_error_free(err);
    return ret_list;
  } else {
    set_last_result(ERROR_NONE);
  }

  GVariant *reply_body = g_dbus_message_get_body(reply);
  GVariantIter *iter = nullptr;
  g_variant_get(reply_body, "(a(s))", &iter);
  char* raw = nullptr;
  while (g_variant_iter_loop(iter, "(s)", &raw) && raw != nullptr) {
    Bundle ret(raw);
    ret_list.emplace_back(ret);
  }

  return ret_list;
}

int DBusSender::RequestNumber(const IEventInfo& info) {
  string appid = info.GetOwner();
  if (appid.length() == 0)
    appid = util::GetAppId();

  string method_name = EventInfo::GetString(info.GetEventType());
  Bundle serialized = info.Serialize();

  GDBusMessage* reply = impl_->MethodCall(appid, method_name, serialized);
  if (!reply)
    return 0;

  GError* err = nullptr;
  if (g_dbus_message_to_gerror(reply, &err)) {
    LOGE("RequestNumber got error %s", err->message);
    if (err->code == G_DBUS_ERROR_ACCESS_DENIED)
      set_last_result(ERROR_PERMISSION_DENIED);
    else
      set_last_result(ERROR_IO_ERROR);

    g_error_free(err);
    return 0;
  } else {
    set_last_result(ERROR_NONE);
  }

  int num;
  GVariant *reply_body = g_dbus_message_get_body(reply);
  g_variant_get(reply_body, "(i)", &num);
  return num;
}

}  // namespace notification
