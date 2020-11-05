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
#include <gio/gunixfdlist.h>

#include <string>
#include <list>

#include "notification-ex/dbus_connection_manager.h"
#include "notification-ex/dbus_sender.h"
#include "notification-ex/dbus_sender_implementation.h"
#include "notification-ex/event_info_internal.h"
#include "notification-ex/ex_util.h"
#include "notification-ex/socket_handler.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "NOTIFICATION_EX"
#define MAX_PACKAGE_STR_SIZE 512

#define RECV_SOCK_IDX 0
#define SEND_SOCK_IDX 1

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
  GError* err = nullptr;
  GDBusConnection* conn = DBusConnectionManager::GetInst().GetConnection();
  gboolean result = g_dbus_connection_emit_signal(conn,
      bus_name.empty() ? NULL : bus_name.c_str(),
      path_.c_str(),
      DBusConnectionManager::GetInst().GetInterfaceName().c_str(),
      signal_name.c_str(), data, &err);
  if (result == FALSE) {
    LOGE("g_dbus_connection_emit_signal() is failed. error(%s)",
        err ? err->message : "Unknown");
    g_clear_error(&err);
    return false;
  }

  result = g_dbus_connection_flush_sync(conn, NULL, &err);
  if (result == FALSE) {
    LOGE("g_dbus_connection_flush_sync() is failed. error(%s)",
        err ? err->message : "Unknown");
    g_clear_error(&err);
    return false;
  }

  LOGI("Successfully emit signal to %s, %s, %s",
      bus_name.c_str(), path_.c_str(), signal_name.c_str());
  return true;
}

string DBusSender::Impl::GetBusName(string appid, string dest_appid) const {
  if (!DBusConnectionManager::GetInst().IsDataProviderMaster(appid))
    return DBusConnectionManager::GetInst().GetDataProviderMasterName();

  return DBusConnectionManager::GetInst().GetBusName(dest_appid);
}

void DBusSender::Notify(GVariantBuilder* builder,
    const IEventInfo& info, string dest_appid) {
  string signal_name = EventInfo::GetString(info.GetEventType());
  string appid = util::GetAppId();
  GVariant* data = g_variant_new("(ssa(s))",
        appid.c_str(),
        reinterpret_cast<char*>(info.Serialize().ToRaw().first.get()), builder);
  string bus_name = impl_->GetBusName(appid, dest_appid);
  LOGI("bus name %s, %s", dest_appid.c_str(), bus_name.c_str());
  impl_->EmitSignal(bus_name, signal_name, data);
  g_variant_builder_unref(builder);
}

void DBusSender::Notify(const IEventInfo& info, list<Bundle> serialized,
    string dest_appid) {
  GVariantBuilder* builder = g_variant_builder_new(G_VARIANT_TYPE("a(s)"));
  int idx = 0;
  for (auto& i : serialized) {
    if (idx != 0 && idx % 10 == 0) {
      Notify(builder, info, dest_appid);
      g_variant_builder_unref(builder);
      builder = g_variant_builder_new(G_VARIANT_TYPE("a(s)"));
    }
    g_variant_builder_add(builder, "(s)",
        reinterpret_cast<char*>(i.ToRaw().first.get()));
    idx++;
  }
  Notify(builder, info, dest_appid);
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
        reinterpret_cast<char*>(serialized.ToRaw().first.get())));

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

  g_object_unref(msg);
  return reply;
}

std::list<Bundle> DBusSender::Impl::MethodCallWithSocket(
    string appid, string method_name, Bundle serialized) {
  list<Bundle> ret_list;
  int fds[2];
  int ret = socketpair(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK, 0, fds);
  if (ret < 0) {
    LOGE("socketpair [%d]", ret);
    return ret_list;
  }
  SocketHandler send_sock(fds[SEND_SOCK_IDX]);
  SocketHandler recv_sock(fds[RECV_SOCK_IDX]);
  std::unique_ptr<GUnixFDList, decltype(g_object_unref)*> fd_list_ptr(
          GetFdList(send_sock.Get()), g_object_unref);


  std::unique_ptr<GDBusMessage, decltype(g_object_unref)*> msg_ptr(
      g_dbus_message_new_method_call(
          DBusConnectionManager::GetInst().GetDataProviderMasterName().c_str(),
          path_.c_str(),
          DBusConnectionManager::GetInst().GetInterfaceName().c_str(),
          method_name.c_str()),
      g_object_unref);
  if (!msg_ptr || !msg_ptr.get()) {
    LOGE("Can't allocate new method call");
    return ret_list;
  }

  g_dbus_message_set_unix_fd_list(msg_ptr.get(), fd_list_ptr.get());
  g_dbus_message_set_body(msg_ptr.get(),
      g_variant_new("(ss)",
        appid.c_str(),
        reinterpret_cast<char*>(serialized.ToRaw().first.get())));

  LOGI("send message !! (%s) (%s) (%s)",
      path_.c_str(), method_name.c_str(), appid.c_str());

  GError* err = nullptr;
  std::unique_ptr<GDBusMessage, decltype(g_object_unref)*> reply_ptr(
      g_dbus_connection_send_message_with_reply_sync(
          DBusConnectionManager::GetInst().GetConnection(), msg_ptr.get(),
          G_DBUS_SEND_MESSAGE_FLAGS_NONE, -1, NULL, NULL, &err),
      g_object_unref);
  if (err != NULL) {
    LOGE("MethodCall, err[%s]", err->message);
    if (err->code == G_DBUS_ERROR_ACCESS_DENIED)
      set_last_result(ERROR_PERMISSION_DENIED);
    else
      set_last_result(ERROR_IO_ERROR);
    g_error_free(err);
  } else if (CheckReplyMessage(reply_ptr.get()) == 0) {
    GVariant *reply_body = g_dbus_message_get_body(reply_ptr.get());
    int cnt;
    g_variant_get(reply_body, "(i)", &cnt);
    ret_list = util::ReadBundleList(recv_sock.Get(), cnt);
  }
  return ret_list;
}

GUnixFDList* DBusSender::Impl::GetFdList(int fd) {
  GError* g_err = nullptr;
  GUnixFDList* fd_list = g_unix_fd_list_new();
  g_unix_fd_list_append(fd_list, fd, &g_err);
  if (g_err != nullptr) {
    LOGE("g_unix_fd_list_append [%s]", g_err->message);
    g_object_unref(fd_list);
    g_error_free(g_err);
    return nullptr;
  }
  return fd_list;
}

GDBusMessage* DBusSender::Impl::MethodCall(string appid, const IEventInfo& info,
    list<Bundle> serialized) {
  int fds[2];
  int ret = socketpair(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK, 0, fds);
  if (ret < 0) {
    LOGE("socketpair [%d]", ret);
    return nullptr;
  }

  SocketHandler send_sock(fds[SEND_SOCK_IDX]);
  SocketHandler recv_sock(fds[RECV_SOCK_IDX]);
  GUnixFDList* fd_list = GetFdList(recv_sock.Get());
  auto fd_list_ptr = std::unique_ptr<GUnixFDList, decltype(g_object_unref)*>(
          fd_list, g_object_unref);

  GDBusMessage* msg = g_dbus_message_new_method_call(
      DBusConnectionManager::GetInst().GetDataProviderMasterName().c_str(),
      path_.c_str(),
      DBusConnectionManager::GetInst().GetInterfaceName().c_str(),
      EventInfo::GetString(info.GetEventType()).c_str());
  if (!msg) {
    LOGE("Can't allocate new method call");
    return nullptr;
  }

  g_dbus_message_set_unix_fd_list(msg, fd_list);
  g_dbus_message_set_body(msg,
      g_variant_new("(ssi)",
          appid.c_str(),
          reinterpret_cast<char*>(info.Serialize().ToRaw().first.get()),
          serialized.size()));

  LOGI("send message !! (%s) (%s) (%s)",
      path_.c_str(),
      EventInfo::GetString(info.GetEventType()).c_str(), appid.c_str());

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
  } else {
    for (auto& i : serialized) {
      unsigned int data_size = static_cast<unsigned int>(
          strlen(reinterpret_cast<char*>(i.ToRaw().first.get())));
      int ret = util::WriteSocket(send_sock.Get(),
          reinterpret_cast<char*>(&data_size), sizeof(data_size));
      if (ret != 0)
        break;
      ret = util::WriteSocket(send_sock.Get(),
          reinterpret_cast<char*>(i.ToRaw().first.get()), data_size);
      if (ret != 0)
        break;
    }
  }

  g_object_unref(msg);
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

  return 0;
}

int DBusSender::Impl::CheckReplyMessage(GDBusMessage* reply) {
  GError* err = nullptr;
  if (g_dbus_message_to_gerror(reply, &err)) {
    LOGE("Request got error %s", err->message);
    if (err->code == G_DBUS_ERROR_ACCESS_DENIED)
      set_last_result(ERROR_PERMISSION_DENIED);
    else
      set_last_result(ERROR_IO_ERROR);

    g_error_free(err);
    return -1;
  } else {
    set_last_result(ERROR_NONE);
  }
  return 0;
}

std::list<Bundle> DBusSender::Request(const IEventInfo& info) {
  string appid = info.GetOwner();
  if (appid.length() == 0)
    appid = util::GetAppId();

  string method_name = EventInfo::GetString(info.GetEventType());
  Bundle serialized = info.Serialize();
  return impl_->MethodCallWithSocket(appid, method_name, serialized);
}

int DBusSender::RequestReturnValue(const IEventInfo& info) {
  string appid = info.GetOwner();
  if (appid.length() == 0)
    appid = util::GetAppId();

  string method_name = EventInfo::GetString(info.GetEventType());
  Bundle serialized = info.Serialize();

  GDBusMessage* reply = impl_->MethodCall(appid, method_name, serialized);
  if (!reply)
    return 0;

  auto reply_ptr = std::unique_ptr<GDBusMessage, decltype(g_object_unref)*>(
      reply, g_object_unref);

  GError* err = nullptr;
  if (g_dbus_message_to_gerror(reply, &err)) {
    LOGE("RequestReturnValue got error %s", err->message);
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
