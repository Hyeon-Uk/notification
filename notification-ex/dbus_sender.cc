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
#include "notification-ex/exception.h"

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

/* LCOV_EXCL_START */
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

void DBusSender::Impl::EmitSignal(string bus_name, string signal_name,
    GVariant* data) {
  GError* err = nullptr;
  GDBusConnection* conn = DBusConnectionManager::GetInst().GetConnection();
  gboolean result = g_dbus_connection_emit_signal(conn,
      bus_name.empty() ? NULL : bus_name.c_str(),
      path_.c_str(),
      DBusConnectionManager::GetInst().GetInterfaceName().c_str(),
      signal_name.c_str(), data, &err);
  if (result == FALSE) {
    int err_code = ERROR_IO_ERROR;
    LOGE("g_dbus_connection_emit_signal() is failed. error(%s)",
        err ? err->message : "Unknown");
    if (err) {
      err_code = util::ConvertDBusError(err->code);
      g_clear_error(&err);
    }
    THROW(err_code);
  }

  result = g_dbus_connection_flush_sync(conn, NULL, &err);
  if (result == FALSE) {
    int err_code = ERROR_IO_ERROR;
    LOGE("g_dbus_connection_flush_sync() is failed. error(%s)",
        err ? err->message : "Unknown");
    if (err) {
      err_code = util::ConvertDBusError(err->code);
      g_clear_error(&err);
    }
    THROW(err_code);
  }
  LOGI("Successfully emit signal to %s, %s, %s",
      bus_name.c_str(), path_.c_str(), signal_name.c_str());
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
}

void DBusSender::Notify(const IEventInfo& info, list<Bundle> serialized,
    string dest_appid) {

  std::unique_ptr<GVariantBuilder,
      decltype(g_variant_builder_unref)*> builder_ptr(
          g_variant_builder_new(G_VARIANT_TYPE("a(s)")),
              g_variant_builder_unref);
  int idx = 0;
  for (auto& i : serialized) {
    if (idx != 0 && idx % 10 == 0) {
      Notify(builder_ptr.get(), info, dest_appid);
      builder_ptr.reset(g_variant_builder_new(G_VARIANT_TYPE("a(s)")));
    }
    g_variant_builder_add(builder_ptr.get(), "(s)",
        reinterpret_cast<char*>(i.ToRaw().first.get()));
    idx++;
  }
  Notify(builder_ptr.get(), info, dest_appid);
}

int DBusSender::Impl::MethodCall(string appid, string method_name,
    Bundle serialized) {

  std::string dpm_name = DBusConnectionManager::GetInst()
      .GetDataProviderMasterName();
  std::string interface_name = DBusConnectionManager::GetInst()
      .GetInterfaceName();

  std::unique_ptr<GDBusMessage, decltype(g_object_unref)*> msg_ptr(
            g_dbus_message_new_method_call(dpm_name.c_str(), path_.c_str(),
                interface_name.c_str(), method_name.c_str()),
            g_object_unref);
  if (!msg_ptr || !msg_ptr.get()) {
    LOGE("Can't allocate new method call");
    THROW(ERROR_OUT_OF_MEMORY);
  }

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
    int err_code = util::ConvertDBusError(err->code);
    g_error_free(err);
    THROW(err_code);
  }

  if (g_dbus_message_to_gerror(reply_ptr.get(), &err)) {
    LOGE("RequestReturnValue got error %s", err->message);
    int err_code = util::ConvertDBusError(err->code);
    g_error_free(err);
    THROW(err_code);
  }

  int num;
  GVariant *reply_body = g_dbus_message_get_body(reply_ptr.get());
  g_variant_get(reply_body, "(i)", &num);
  return num;
}

std::list<Bundle> DBusSender::Impl::GetListMethodCall(
    string appid, string method_name, Bundle serialized) {
  list<Bundle> ret_list;
  int fds[2];
  int ret = socketpair(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK, 0, fds);
  if (ret < 0) {
    LOGE("socketpair fail [%d]", ret);
    THROW(ERROR_IO_ERROR);
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
    THROW(ERROR_OUT_OF_MEMORY);
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
    int err_code = util::ConvertDBusError(err->code);
    g_error_free(err);
    THROW(err_code);
  } else {
    GError* err = nullptr;
    if (g_dbus_message_to_gerror(reply_ptr.get(), &err)) {
      LOGE("Request got error %s", err->message);
      int err_code = util::ConvertDBusError(err->code);
      g_error_free(err);
      THROW(err_code);
    }
    GVariant *reply_body = g_dbus_message_get_body(reply_ptr.get());
    int cnt;
    g_variant_get(reply_body, "(i)", &cnt);
    ret_list = util::ReadBundleList(recv_sock.Get(), cnt);
    if (ret_list.size() == 0)
      THROW(ERROR_NOT_EXIST_ID);

    if (static_cast<int>(ret_list.size()) != cnt)
      THROW(ERROR_IO_ERROR);
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

void DBusSender::Impl::MethodCall(string appid, const IEventInfo& info,
    list<Bundle> serialized) {
  int fds[2];
  int ret = socketpair(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK, 0, fds);
  if (ret < 0) {
    LOGE("socketpair fail [%d]", ret);
    THROW(ERROR_IO_ERROR);
  }

  SocketHandler send_sock(fds[SEND_SOCK_IDX]);
  SocketHandler recv_sock(fds[RECV_SOCK_IDX]);
  GUnixFDList* fd_list = GetFdList(recv_sock.Get());
  auto fd_list_ptr = std::unique_ptr<GUnixFDList, decltype(g_object_unref)*>(
          fd_list, g_object_unref);

  std::string dpm_name = DBusConnectionManager::GetInst()
      .GetDataProviderMasterName();
  std::string interface_name = DBusConnectionManager::GetInst()
      .GetInterfaceName();
  std::unique_ptr<GDBusMessage, decltype(g_object_unref)*> msg_ptr(
      g_dbus_message_new_method_call(
          dpm_name.c_str(),
          path_.c_str(),
          interface_name.c_str(),
          EventInfo::GetString(info.GetEventType()).c_str()),
      g_object_unref);
  if (!msg_ptr || !msg_ptr.get()) {
    LOGE("Can't allocate new method call");
    THROW(ERROR_OUT_OF_MEMORY);
  }

  g_dbus_message_set_unix_fd_list(msg_ptr.get(), fd_list);
  g_dbus_message_set_body(msg_ptr.get(),
      g_variant_new("(ssi)",
          appid.c_str(),
          reinterpret_cast<char*>(info.Serialize().ToRaw().first.get()),
          serialized.size()));

  LOGI("send message !! (%s) (%s) (%s)",
      path_.c_str(),
      EventInfo::GetString(info.GetEventType()).c_str(), appid.c_str());

  GError* err = nullptr;
  std::unique_ptr<GDBusMessage, decltype(g_object_unref)*> reply_ptr(
      g_dbus_connection_send_message_with_reply_sync(
          DBusConnectionManager::GetInst().GetConnection(), msg_ptr.get(),
          G_DBUS_SEND_MESSAGE_FLAGS_NONE, -1, NULL, NULL, &err),
      g_object_unref);
  if (err != nullptr) {
    LOGE("MethodCall, err[%s]", err->message);
    int err_code = util::ConvertDBusError(err->code);
    g_error_free(err);
    THROW(err_code);
  }

  if (g_dbus_message_to_gerror(reply_ptr.get(), &err)) {
    LOGE("SendMessage got error %s", err->message);
    int err_code = util::ConvertDBusError(err->code);
    g_error_free(err);
    THROW(err_code);
  }

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

void DBusSender::SendMessage(const IEventInfo& info, list<Bundle> serialized,
    string dest_appid) {
  string appid = info.GetOwner();
  if (appid.length() == 0)
    appid = util::GetAppId();

  if (appid == "d::org.tizen.appfw.service.esd")
    appid = "data-provider-master";

  impl_->MethodCall(appid, info, serialized);
}

std::list<Bundle> DBusSender::Request(const IEventInfo& info) {
  string appid = info.GetOwner();
  if (appid.length() == 0)
    appid = util::GetAppId();

  string method_name = EventInfo::GetString(info.GetEventType());
  Bundle serialized = info.Serialize();
  return impl_->GetListMethodCall(appid, method_name, serialized);
}

int DBusSender::RequestReturnValue(const IEventInfo& info) {
  string appid = info.GetOwner();
  if (appid.length() == 0)
    appid = util::GetAppId();

  string method_name = EventInfo::GetString(info.GetEventType());
  Bundle serialized = info.Serialize();
  return impl_->MethodCall(appid, method_name, serialized);
}
/* LCOV_EXCL_STOP */

}  // namespace notification
