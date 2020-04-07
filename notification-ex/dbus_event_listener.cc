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
#include "notification-ex/dbus_event_listener.h"
#include "notification-ex/dbus_event_listener_implementation.h"
#include "notification-ex/exception.h"
#include "notification-ex/ex_util.h"
#include "notification-ex/event_info_internal.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "NOTIFICATION_EX"
#define MAX_PACKAGE_STR_SIZE 512
#define NORMAL_UID_BASE 5000

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
  GDBusMessage* msg = nullptr;
  GDBusMessage* reply = nullptr;
  GError* err = nullptr;
  GVariant* body;
  uid_t uid = 0;

  msg = g_dbus_message_new_method_call(DBUS_NAME, DBUS_OBJECT_PATH,
      DBUS_INTERFACE_NAME, "GetConnectionUnixUser");
  if (!msg) {
    LOGE("Failed to alloc new method call");
    goto out;
  }

  g_dbus_message_set_body(msg, g_variant_new("(s)", sender_name));
  reply = g_dbus_connection_send_message_with_reply_sync(connection, msg,
      G_DBUS_SEND_MESSAGE_FLAGS_NONE, -1, nullptr, nullptr, &err);

  if (!reply) {
    if (err != nullptr) {
      LOGE("Failed to get uid [%s]", err->message);
      g_error_free(err);
    }
    goto out;
  }

  body = g_dbus_message_get_body(reply);
  g_variant_get(body, "(u)", &uid);

out:
  if (msg)
    g_object_unref(msg);
  if (reply)
    g_object_unref(reply);

  return uid;
}

pid_t DBusEventListener::Impl::GetSenderPid(GDBusConnection* connection,
               const char* sender_name) {
  GDBusMessage* msg = nullptr;
  GDBusMessage* reply = nullptr;
  GError* err = nullptr;
  GVariant* body;
  pid_t pid = 0;

  msg = g_dbus_message_new_method_call(DBUS_NAME, DBUS_OBJECT_PATH,
      DBUS_INTERFACE_NAME, "GetConnectionUnixProcessID");
  if (!msg) {
    LOGE("Failed to alloc new method call");
    goto out;
  }

  g_dbus_message_set_body(msg, g_variant_new("(s)", sender_name));
  reply = g_dbus_connection_send_message_with_reply_sync(connection, msg,
      G_DBUS_SEND_MESSAGE_FLAGS_NONE, -1, nullptr, nullptr, &err);

  if (!reply) {
    if (err != nullptr) {
      LOGE("Failed to get uid [%s]", err->message);
      g_error_free(err);
    }
    goto out;
  }

  body = g_dbus_message_get_body(reply);
  g_variant_get(body, "(u)", &pid);

out:
  if (msg)
    g_object_unref(msg);
  if (reply)
    g_object_unref(reply);

  return pid;
}

void DBusEventListener::Impl::SignalCb(GDBusConnection* connection,
                      const gchar* sender_name,
                      const gchar* object_path,
                      const gchar* interface_name,
                      const gchar* signal_name,
                      GVariant* parameters,
                      void* user_data) {
  DBusEventListener::Impl* dl = static_cast<DBusEventListener::Impl*>(user_data);
  char* appid = nullptr;
  GVariantIter *iter = nullptr;
  char* event_info_raw = nullptr;
  g_variant_get(parameters, "(ssa(s))", &appid, &event_info_raw, &iter);

  LOGI("signal callback!! (%s)", appid);
  string sender_appid = string(appid);
  string cur_appid = util::GetAppId();
  if (sender_appid == cur_appid)
    return;
  if ((!DBusConnectionManager::GetInst().IsDataProviderMaster(cur_appid)
    && !DBusConnectionManager::GetInst().IsDataProviderMaster(sender_appid))
    || (cur_appid == sender_appid))
    return;

  LOGD("%s : %s", cur_appid.c_str(), sender_appid.c_str());

  try {
    char* raw = nullptr;
    list<Bundle> ret_list;
    while (g_variant_iter_loop(iter, "(s)", &raw) && raw != nullptr) {
      Bundle ret(raw);
      ret_list.emplace_back(ret);
    }

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
  } catch(Exception &ex) {
    LOGE("%s %d", ex.what(), ex.GetErrorCode());
  }
}

void DBusEventListener::Impl::OnMethodCall(
    GDBusConnection *conn, const gchar *sender, const gchar *object_path,
    const gchar *iface_name, const gchar *method_name,
    GVariant *parameters, GDBusMethodInvocation *invocation,
    gpointer user_data) {
  LOGI("method_name[%s] sender[%s]", method_name, sender);
  DBusEventListener::Impl* dl = static_cast<DBusEventListener::Impl*>(user_data);
  GVariant *reply_body = NULL;
  try {
    char* appid = NULL;
    char* serialized = NULL;
    if (g_strcmp0(method_name, "Get") == 0) {
      g_variant_get(parameters, "(&s&s)", &appid, &serialized);

      Bundle b(serialized);
      EventInfo info(b);
      list<Bundle> result = dl->parent_->NotifyObserver(info);
      GVariantBuilder* builder = g_variant_builder_new(G_VARIANT_TYPE("a(s)"));
      for (auto& i : result) {
        g_variant_builder_add(builder, "(s)",
          reinterpret_cast<char*>(i.ToRaw().first.get()));
      }
      reply_body = g_variant_new("(a(s))", builder);
      g_variant_builder_unref(builder);
    } else if (g_strcmp0(method_name, "Count") == 0) {
      g_variant_get(parameters, "(&s&s)", &appid, &serialized);

      Bundle b(serialized);
      EventInfo info(b);
      int num = dl->parent_->NotifyNumberRequest(info);
      reply_body = g_variant_new("(i)", num);
    }
    g_dbus_method_invocation_return_value(invocation, reply_body);
  } catch (Exception &ex) {
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
    "        <method name='Get'>"
    "          <arg type='s' name='appid' direction='in'/>"
    "          <arg type='s' name='serialized' direction='in'/>"
    "          <arg type='a(s)' name='noti_arr' direction='out'/>"
    "        </method>"
    "        <method name='Count'>"
    "          <arg type='s' name='appid' direction='in'/>"
    "          <arg type='s' name='serialized' direction='in'/>"
    "          <arg type='i' name='num' direction='out'/>"
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
