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

using namespace std;
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
  LOGI("ButtonItem impl created");
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

  LOGE("%s : %s", cur_appid.c_str(), sender_appid.c_str());

  char* raw = nullptr;
  list<Bundle> ret_list;
  while (g_variant_iter_loop(iter, "(s)", &raw) && raw != nullptr) {
    Bundle ret(raw);
    ret_list.emplace_back(ret);
  }

  Bundle b(event_info_raw);
  EventInfo info(b);
  dl->parent_->NotifyObserver(info, ret_list);
}

void DBusEventListener::Impl::OnMethodCall(
    GDBusConnection *conn, const gchar *sender, const gchar *object_path,
    const gchar *iface_name, const gchar *method_name,
    GVariant *parameters, GDBusMethodInvocation *invocation,
    gpointer user_data) {
  LOGI("method_name[%s] sender[%s]", method_name, sender);
  DBusEventListener::Impl* dl = static_cast<DBusEventListener::Impl*>(user_data);
  GVariant *reply_body = NULL;
  if (g_strcmp0(method_name, "Get") == 0) {
    char* appid = NULL;
    char* serialized = NULL;
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
  }
  g_dbus_method_invocation_return_value(invocation, reply_body);
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

}  // namespace notification
