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

#ifndef NOTIFICATION_EX_DBUS_EVENT_LISTENER_IMPLEMENTATION_H_
#define NOTIFICATION_EX_DBUS_EVENT_LISTENER_IMPLEMENTATION_H_

#include <string>
#include <memory>
#include <list>

#include "notification-ex/dbus_event_listener.h"

namespace notification {

class DBusEventListener::Impl {
 public:
  virtual ~Impl();

 private:
  friend class DBusEventListener;
  Impl(DBusEventListener* parent, std::string path);

 private:
  void UnRegisterGDBusInterface();
  int RegisterGDBusInterface();
  static void OnMethodCall(
    GDBusConnection *conn, const gchar *sender, const gchar *object_path,
    const gchar *iface_name, const gchar *method_name,
    GVariant *parameters, GDBusMethodInvocation *invocation,
    gpointer user_data);
  static void MethodCallHandler(GVariant *parameters, Impl* dl, pid_t pid, uid_t uid);

  static GDBusInterfaceVTable InterfaceVtable;
  static void SignalCb(GDBusConnection* connection,
                      const gchar* sender_name,
                      const gchar* object_path,
                      const gchar* interface_name,
                      const gchar* signal_name,
                      GVariant* parameters,
                      void* user_data);
  static pid_t GetSenderPid(GDBusConnection* connection,
                     const char* sender_name);
  static uid_t GetSenderUid(GDBusConnection* connection,
                     const char* sender_name);

  bool SubscribeSignal();
  void UnSubscribeSignal();
  IEventObserver* observer_ = nullptr;
  int subscribe_id_;
  int registration_id_;
  std::string path_;
  DBusEventListener* parent_;
};

}  // namespace notification
#endif  // NOTIFICATION_EX_DBUS_EVENT_LISTENER_IMPLEMENTATION_H_
