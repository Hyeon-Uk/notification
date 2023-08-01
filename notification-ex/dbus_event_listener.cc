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
#include <map>
#include <list>

#include "notification-ex/dbus_connection_manager.h"
#include "notification-ex/dbus_event_listener.h"
#include "notification-ex/dbus_event_listener_implementation.h"
#include "notification-ex/exception.h"
#include "notification-ex/ex_util.h"
#include "notification-ex/event_info_internal.h"
#include "notification-ex/socket_handler.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "NOTIFICATION_EX"
#define MAX_PACKAGE_STR_SIZE 512
#define NORMAL_UID_BASE 5000
#define MAX_CACHE_COUNT 20

#define DBUS_NAME "org.freedesktop.DBus"
#define DBUS_OBJECT_PATH "/org/freedesktop/DBus"
#define DBUS_INTERFACE_NAME "org.freedesktop.DBus"

using namespace std;
using namespace tizen_base;

namespace notification {

DBusEventListener::DBusEventListener(string path)
    : impl_(new Impl(this, path)) {
  LOGW("Created (%s)", path.c_str());
}

DBusEventListener::~DBusEventListener() {
  LOGW("Destroyed");
}

DBusEventListener::Impl::~Impl() = default;

DBusEventListener::Impl::Impl(DBusEventListener* parent, string path)
    : subscribe_id_(0), registration_id_(0), path_(path), parent_(parent) {
  LOGI("Dbus_event_listener impl created");
}

uid_t DBusEventListener::Impl::GetSenderUid(GDBusConnection* connection,
                const char* sender_name) {
  static std::map<std::string, uid_t> cache;
  auto cp = cache.find(sender_name);
  if (cp != cache.end())
    return cp->second;
  std::unique_ptr<GDBusMessage, decltype(g_object_unref)*> msg_ptr(
            g_dbus_message_new_method_call(DBUS_NAME, DBUS_OBJECT_PATH,
                DBUS_INTERFACE_NAME, "GetConnectionUnixUser"),
            g_object_unref);

  g_dbus_message_set_body(msg_ptr.get(), g_variant_new("(s)", sender_name));
  GError* err = nullptr;
  GDBusMessage* reply = g_dbus_connection_send_message_with_reply_sync(
      connection, msg_ptr.get(), G_DBUS_SEND_MESSAGE_FLAGS_NONE, -1, nullptr,
      nullptr, &err);

  if (!reply) {
    if (err != nullptr) {
      LOGE("Failed to get uid [%s]", err->message);
      g_error_free(err);
    }
    THROW(ERROR_IO_ERROR);
  }

  GVariant* body = g_dbus_message_get_body(reply);
  uid_t uid = 0;
  g_variant_get(body, "(u)", &uid);
  g_object_unref(reply);

  if (cache.size() >= MAX_CACHE_COUNT)
    cache.clear();

  cache[sender_name] = uid;
  return uid;
}

pid_t DBusEventListener::Impl::GetSenderPid(GDBusConnection* connection,
                const char* sender_name) {
  static std::map<std::string, pid_t> cache;
  auto cp = cache.find(sender_name);
  if (cp != cache.end())
    return cp->second;

  std::unique_ptr<GDBusMessage, decltype(g_object_unref)*> msg_ptr(
            g_dbus_message_new_method_call(DBUS_NAME, DBUS_OBJECT_PATH,
                DBUS_INTERFACE_NAME, "GetConnectionUnixProcessID"),
            g_object_unref);

  if (!msg_ptr || !msg_ptr.get()) {
    LOGE("Failed to alloc new method call");
    THROW(ERROR_OUT_OF_MEMORY);
  }

  g_dbus_message_set_body(msg_ptr.get(), g_variant_new("(s)", sender_name));
  GError* err = nullptr;
  GDBusMessage* reply = g_dbus_connection_send_message_with_reply_sync(
      connection, msg_ptr.get(), G_DBUS_SEND_MESSAGE_FLAGS_NONE, -1,
      nullptr, nullptr, &err);

  if (!reply) {
    if (err != nullptr) {
      LOGE("Failed to get uid [%s]", err->message);
      g_error_free(err);
    }
    THROW(ERROR_IO_ERROR);
  }

  GVariant* body = g_dbus_message_get_body(reply);
  pid_t pid = 0;
  g_variant_get(body, "(u)", &pid);
  g_object_unref(reply);

  if (cache.size() >= MAX_CACHE_COUNT)
    cache.clear();

  cache[sender_name] = pid;
  return pid;
}

void DBusEventListener::Impl::SignalCb(GDBusConnection* connection,
                      const gchar* sender_name,
                      const gchar* object_path,
                      const gchar* interface_name,
                      const gchar* signal_name,
                      GVariant* parameters,
                      void* user_data) {
  DBusEventListener::Impl* dl =
    static_cast<DBusEventListener::Impl*>(user_data);
  char* appid = nullptr;
  GVariantIter *iter = nullptr;
  char* event_info_raw = nullptr;
  g_variant_get(parameters, "(&s&sa(s))", &appid, &event_info_raw, &iter);

  LOGI("signal callback!! (%s)", appid);
  string sender_appid = string(appid);
  string cur_appid = util::GetAppId();
  if (sender_appid == cur_appid) {
    g_variant_iter_free(iter);
    return;
  }
  if ((!DBusConnectionManager::GetInst().IsDataProviderMaster(cur_appid)
    && !DBusConnectionManager::GetInst().IsDataProviderMaster(sender_appid))
    || (cur_appid == sender_appid)) {
    g_variant_iter_free(iter);
    return;
  }

  LOGD("%s : %s", cur_appid.c_str(), sender_appid.c_str());

  try {
    char* raw = nullptr;
    list<Bundle> ret_list;
    while (g_variant_iter_loop(iter, "(&s)", &raw) && raw != nullptr) {
      Bundle ret(raw);
      ret_list.emplace_back(ret);
    }

    g_variant_iter_free(iter);
    Bundle b(event_info_raw);
    EventInfo info(b);
    if (info.GetEventType() == EventInfo::Post
           || info.GetEventType() == EventInfo::Update) {
      uid_t uid = GetSenderUid(connection, sender_name);
      if (uid >= NORMAL_UID_BASE) {
        pid_t pid = GetSenderPid(connection, sender_name);
        info.SetValidatedOwner(util::GetAppId(pid));
        info.SetValidatedUid(uid);
      }
    }
    dl->parent_->NotifyObserver(info, ret_list);
  } catch(Exception& ex) {
    LOGE("%s %d", ex.what(), ex.GetErrorCode());
  }
}

int DBusEventListener::Impl::GetRecvSocket(GDBusMethodInvocation* invocation) {
  GDBusMessage* msg = g_dbus_method_invocation_get_message(invocation);
  GUnixFDList* fd_list = g_dbus_message_get_unix_fd_list(msg);
  if (fd_list == NULL) {
    LOGE("fd list is null");
    return -1;
  }

  GError* g_err = nullptr;
  int ret_fd = g_unix_fd_list_get(fd_list, 0, &g_err);
  if (ret_fd == -1 || g_err) {
    LOGE("g_unix_fd_list_get [%s]", g_err->message);
    g_error_free(g_err);
    return -1;
  }

  LOGI("sender fd : %d", ret_fd);
  return ret_fd;
}

list<Bundle> DBusEventListener::Impl::ReadNotiList(
      GDBusMethodInvocation* invocation, int noti_cnt) {
  list<Bundle> ret_list;
  list<Bundle> empty_list;
  int recvfd = GetRecvSocket(invocation);
  if (recvfd < 1) {
    LOGE("Fail to get socket fd");
    return ret_list;
  }
  SocketHandler recv_sock(recvfd);

  ret_list = util::ReadBundleList(recv_sock.Get(), noti_cnt);
  return ret_list;
}

int DBusEventListener::Impl::MethodCallHandler(
      string sender_appid, string event_info_raw, int noti_cnt,
      GDBusMethodInvocation* invocation, DBusEventListener::Impl* dl,
      pid_t pid, uid_t uid) {
  string cur_appid = util::GetAppId();
  LOGI("MethodCallHandler!! sender(%s), cur(%s)",
      sender_appid.c_str(), cur_appid.c_str());

  if (sender_appid == cur_appid)
    return 0;

  if ((!DBusConnectionManager::GetInst().IsDataProviderMaster(cur_appid)
    && !DBusConnectionManager::GetInst().IsDataProviderMaster(sender_appid))
    || (cur_appid == sender_appid))
    return 0;

  list<Bundle> ret_list;
  if (noti_cnt > 0) {
    ret_list = dl->ReadNotiList(invocation, noti_cnt);
    if (ret_list.size() != static_cast<unsigned int>(noti_cnt)) {
      LOGE("Read Noti List fail : Expect(%d), Actual(%zu)",
          noti_cnt, ret_list.size());
      return -1;
    }
  }

  Bundle b(event_info_raw);
  EventInfo info(b);
  if ((info.GetEventType() == EventInfo::Post
        || info.GetEventType() == EventInfo::Update)
        && (uid >= NORMAL_UID_BASE)) {
    info.SetValidatedOwner(util::GetAppId(pid));
    info.SetValidatedUid(uid);
  }
  dl->parent_->NotifyObserver(info, ret_list);
  return 0;
}

void DBusEventListener::Impl::MultiNotiUpdateProcess(
      GVariant* parameters, GDBusMethodInvocation* invocation,
      DBusEventListener::Impl* dl, pid_t pid, uid_t uid) {
  int noti_cnt;
  char* appid = nullptr;
  char* event_info_raw = nullptr;
  g_variant_get(parameters, "(&s&si)", &appid, &event_info_raw, &noti_cnt);

  g_dbus_method_invocation_return_value(invocation, nullptr);
  MethodCallHandler(
      appid, event_info_raw, noti_cnt, invocation, dl, pid, uid);
}

void DBusEventListener::Impl::GlobalUpdateProcess(
      GVariant* parameters, GDBusMethodInvocation* invocation,
      DBusEventListener::Impl* dl, pid_t pid, uid_t uid) {
  char* appid = nullptr;
  char* event_info_raw = nullptr;
  g_variant_get(parameters, "(&s&s)", &appid, &event_info_raw);
  g_dbus_method_invocation_return_value(invocation, nullptr);
  MethodCallHandler(
      appid, event_info_raw, 0, invocation, dl, pid, uid);
}

void DBusEventListener::Impl::GetProcess(
      GVariant* parameters, GDBusMethodInvocation* invocation,
      DBusEventListener::Impl* dl) {
  char* appid = NULL;
  char* serialized = NULL;
  g_variant_get(parameters, "(&s&s)", &appid, &serialized);
  int recvfd = GetRecvSocket(invocation);
  if (recvfd < 1) {
    g_dbus_method_invocation_return_error(invocation,
          noti_ex_error_quark(), ERROR_IO_ERROR, "Fail to get socket");
    return;
  }

  SocketHandler recv_sock(recvfd);
  Bundle b(serialized);
  EventInfo info(b);
  list<Bundle> result = dl->parent_->NotifyObserver(info);
  g_dbus_method_invocation_return_value(invocation,
      g_variant_new("(i)", result.size()));
  for (auto& i : result) {
    unsigned int data_size = static_cast<unsigned int>(
        strlen(reinterpret_cast<char*>(i.ToRaw().first.get())));
    int ret = util::WriteSocket(recv_sock.Get(),
        reinterpret_cast<char*>(&data_size), sizeof(data_size));
    if (ret != 0) {
      LOGE("Fail to write (%d)", ret);
      break;
    }

    ret = util::WriteSocket(recv_sock.Get(),
        reinterpret_cast<char*>(i.ToRaw().first.get()), data_size);
    if (ret != 0) {
      LOGE("Fail to write (%d)", ret);
      break;
    }
  }
}

void DBusEventListener::Impl::OnMethodCall(
    GDBusConnection* conn, const gchar* sender, const gchar* object_path,
    const gchar* iface_name, const gchar* method_name,
    GVariant* parameters, GDBusMethodInvocation* invocation,
    gpointer user_data) {
  LOGI("method_name[%s] sender[%s]", method_name, sender);
  DBusEventListener::Impl* dl =
      static_cast<DBusEventListener::Impl*>(user_data);
  try {
    uid_t uid = GetSenderUid(conn, sender);
    pid_t pid = GetSenderPid(conn, sender);

    if (g_strcmp0(method_name, "Post") == 0
           || g_strcmp0(method_name, "Update") == 0
           || g_strcmp0(method_name, "Delete") == 0) {
      MultiNotiUpdateProcess(parameters, invocation, dl, pid, uid);
    } else if (g_strcmp0(method_name, "Error") == 0
           || g_strcmp0(method_name, "DeleteAll") == 0
           || g_strcmp0(method_name, "Register") == 0
           || g_strcmp0(method_name, "Unregister") == 0) {

      GlobalUpdateProcess(parameters, invocation, dl, pid, uid);
    } else if (g_strcmp0(method_name, "Count") == 0) {
      char* appid = NULL;
      char* serialized = NULL;
      g_variant_get(parameters, "(&s&s)", &appid, &serialized);

      Bundle b(serialized);
      EventInfo info(b);
      int num = dl->parent_->NotifyNumberRequest(info);
      g_dbus_method_invocation_return_value(invocation,
          g_variant_new("(i)", num));
    } else if (g_strcmp0(method_name, "Get") == 0) {
      GetProcess(parameters, invocation, dl);
    } else {
      g_dbus_method_invocation_return_error(invocation, noti_ex_error_quark(),
            ERROR_IO_ERROR, "invalid operation");
      return;
    }
  } catch (Exception& ex) {
    LOGE("%s %d", ex.what(), ex.GetErrorCode());
    g_dbus_method_invocation_return_error(invocation, noti_ex_error_quark(),
            ERROR_IO_ERROR, "OnMethodCall fail");
  }
}

int DBusEventListener::Impl::RegisterGDBusInterface() {
  static const GDBusInterfaceVTable interface_vtable = {
    OnMethodCall,
    nullptr,
    nullptr
  };
  static gchar introspection_xml[] =
    "  <node>"
    "  <interface name='org.tizen.notification_ex'>"
    "        <method name='Post'>"
    "          <arg type='s' name='appid' direction='in'/>"
    "          <arg type='s' name='info' direction='in'/>"
    "          <arg type='i' name='noti_cnt' direction='in'/>"
    "        </method>"
    "        <method name='Update'>"
    "          <arg type='s' name='appid' direction='in'/>"
    "          <arg type='s' name='info' direction='in'/>"
    "          <arg type='i' name='noti_cnt' direction='in'/>"
    "        </method>"
    "        <method name='Delete'>"
    "          <arg type='s' name='appid' direction='in'/>"
    "          <arg type='s' name='info' direction='in'/>"
    "          <arg type='i' name='noti_cnt' direction='in'/>"
    "        </method>"
    "        <method name='Get'>"
    "          <arg type='s' name='appid' direction='in'/>"
    "          <arg type='s' name='serialized' direction='in'/>"
    "          <arg type='i' name='ret' direction='out'/>"
    "        </method>"
    "        <method name='Error'>"
    "          <arg type='s' name='appid' direction='in'/>"
    "          <arg type='s' name='info' direction='in'/>"
    "        </method>"
    "        <method name='Count'>"
    "          <arg type='s' name='appid' direction='in'/>"
    "          <arg type='s' name='serialized' direction='in'/>"
    "          <arg type='i' name='num' direction='out'/>"
    "        </method>"
    "        <method name='DeleteAll'>"
    "          <arg type='s' name='appid' direction='in'/>"
    "          <arg type='s' name='info' direction='in'/>"
    "        </method>"
    "        <method name='Register'>"
    "          <arg type='s' name='appid' direction='in'/>"
    "          <arg type='s' name='info' direction='in'/>"
    "        </method>"
    "        <method name='Unregister'>"
    "          <arg type='s' name='appid' direction='in'/>"
    "          <arg type='s' name='info' direction='in'/>"
    "        </method>"
    "  </interface>"
    "  </node>";
  GError *error = NULL;
  GDBusNodeInfo *introspection_data =
      g_dbus_node_info_new_for_xml(introspection_xml, &error);
  if (!introspection_data) {
    LOGE("g_dbus_node_info_new_for_xml is failed.");
    if (error != NULL) {
      LOGE("g_dbus_node_info_new_for_xml err [%s]", error->message);
      g_error_free(error);
    }
    return ERROR_IO_ERROR;
  }

  registration_id_ = g_dbus_connection_register_object(
      DBusConnectionManager::GetInst().GetConnection(),
      path_.c_str(), introspection_data->interfaces[0],
      &interface_vtable, this, NULL, NULL);
  g_dbus_node_info_unref(introspection_data);
  if (registration_id_ == 0) {
    LOGE("register object fail");
    return ERROR_IO_ERROR;
  }

  LOGI("RegisterGDBusInterface success");
  return ERROR_NONE;
}

void DBusEventListener::Impl::UnRegisterGDBusInterface() {
  g_dbus_connection_unregister_object(
        DBusConnectionManager::GetInst().GetConnection(),
        registration_id_);
}

bool DBusEventListener::Impl::SubscribeSignal() {
  subscribe_id_ = g_dbus_connection_signal_subscribe(
                    DBusConnectionManager::GetInst().GetConnection(),
                    NULL,
                    DBusConnectionManager::GetInst().GetInterfaceName().c_str(),
                    NULL,
                    path_.c_str(),
                    NULL,
                    G_DBUS_SIGNAL_FLAGS_NONE,
                    SignalCb,
                    this,
                    NULL);
  return subscribe_id_ > 0;
}

void DBusEventListener::Impl::UnSubscribeSignal() {
  g_dbus_connection_signal_unsubscribe(
      DBusConnectionManager::GetInst().GetConnection(), subscribe_id_);
  subscribe_id_ = 0;
}

void DBusEventListener::RegisterObserver(IEventObserver* observer) {
  if (impl_->subscribe_id_ == 0) {
    impl_->SubscribeSignal();
  } else {
    impl_->UnSubscribeSignal();
    impl_->SubscribeSignal();
  }

  if (impl_->registration_id_ == 0) {
    impl_->RegisterGDBusInterface();
  } else {
    impl_->UnRegisterGDBusInterface();
    impl_->RegisterGDBusInterface();
  }
  impl_->observer_ = observer;
}

void DBusEventListener::UnRegisterObserver(IEventObserver* observer) {
  impl_->UnSubscribeSignal();
  impl_->observer_ = NULL;
}

void DBusEventListener::NotifyObserver(
    const IEventInfo& info, list<Bundle> serialized) {
  impl_->observer_->OnEvent(info, serialized);
}

list<Bundle> DBusEventListener::NotifyObserver(const IEventInfo& info) {
  return impl_->observer_->OnRequest(info);
}

int DBusEventListener::NotifyNumberRequest(const IEventInfo& info) {
  return impl_->observer_->OnRequestNumber(info);
}

}  // namespace notification
