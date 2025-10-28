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

/* LCOV_EXCL_START */
class DBusEventListener::Impl {
 public:
  virtual ~Impl();

 private:
  friend class DBusEventListener;
  Impl(DBusEventListener* parent, std::string path);

 private:
  void UnRegisterGDBusInterface();
  int RegisterGDBusInterface();
  std::list<tizen_base::Bundle> ReadNotiList(
      GDBusMethodInvocation* invocation, int noti_cnt);
  static int GetRecvSocket(GDBusMethodInvocation* invocation);
  static void OnMethodCall(
    GDBusConnection *conn, const gchar *sender, const gchar *object_path,
    const gchar *iface_name, const gchar *method_name,
    GVariant *parameters, GDBusMethodInvocation *invocation,
    gpointer user_data);
  static int MethodCallHandler(std::string sender_appid,
      std::string event_info_raw, int noti_cnt,
      GDBusMethodInvocation* invocation, Impl* dl, pid_t pid, uid_t uid);
  static void MultiNotiUpdateProcess(
      GVariant* parameters, GDBusMethodInvocation* invocation,
      Impl* dl, pid_t pid, uid_t uid);
  static void GlobalUpdateProcess(
      GVariant* parameters, GDBusMethodInvocation* invocation,
      Impl* dl, pid_t pid, uid_t uid);
  static void GetProcess(
      GVariant* parameters, GDBusMethodInvocation* invocation, Impl* dl);

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
/* LCOV_EXCL_STOP */

}  // namespace notification
#endif  // NOTIFICATION_EX_DBUS_EVENT_LISTENER_IMPLEMENTATION_H_
