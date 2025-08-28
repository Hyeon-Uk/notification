/*
 * Copyright (c) 2022 Samsung Electronics Co., Ltd.
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

#include "glib_mock.hh"

#include <dlfcn.h>

#include "mock_hook.hh"
#include "test_fixture.hh"

extern "C" GFile* g_file_new_for_path(const char* arg1) {
  return MOCK_HOOK_P1(GlibMock, g_file_new_for_path, arg1);
}

extern "C" gboolean g_file_query_exists(GFile* arg1, GCancellable* arg2) {
  return MOCK_HOOK_P2(GlibMock, g_file_query_exists, arg1, arg2);
}

extern "C" gboolean g_file_make_directory(GFile* arg1, GCancellable* arg2,
    GError** arg3) {
  return MOCK_HOOK_P3(GlibMock, g_file_make_directory, arg1, arg2, arg3);
}

extern "C" int access(const char* arg1, int arg2) {
  if (strstr(arg1, ".notification.db") ||
      strstr(arg1, "notification.ini"))
    return 0;

  int (*access_func)(const char*, int) =
      reinterpret_cast<int (*)(const char*, int)>(dlsym(RTLD_NEXT, "access"));
  if (access_func) return access_func(arg1, arg2);

  return 0;
}

extern "C" ssize_t read(int fd, void *buf, size_t count) {
  return MOCK_HOOK_P3(GlibMock, read, fd, buf, count);
}

extern "C" GDBusConnection* g_bus_get_sync(GBusType type,
    GCancellable* cancellable, GError** error) {
  return MOCK_HOOK_P3(GlibMock, g_bus_get_sync, type, cancellable, error);
}

extern "C" GDBusMessage* g_dbus_connection_send_message_with_reply_sync(
    GDBusConnection* conn, GDBusMessage* msg, GDBusSendMessageFlags flags,
    gint timeout, volatile guint32* out_serial, GCancellable* cancellable,
    GError** error) {
  return MOCK_HOOK_P7(GlibMock, g_dbus_connection_send_message_with_reply_sync,
      conn, msg, flags, timeout, out_serial, cancellable, error);
}

extern "C" GVariant* g_dbus_message_get_body(GDBusMessage* arg0) {
  return MOCK_HOOK_P1(GlibMock, g_dbus_message_get_body, arg0);
}

extern "C" GList* g_list_append (GList* list, gpointer data) {
  return MOCK_HOOK_P2(GlibMock, g_list_append, list, data);
}

extern "C" guint g_dbus_connection_signal_subscribe(GDBusConnection* arg0,
    const gchar* arg1, const gchar* arg2, const gchar* arg3, const gchar* arg4,
    const gchar* arg5, GDBusSignalFlags arg6, GDBusSignalCallback arg7,
    gpointer arg8, GDestroyNotify arg9) {
  return MOCK_HOOK_P10(GlibMock, g_dbus_connection_signal_subscribe,
      arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9);
}

extern "C" gboolean g_dbus_connection_emit_signal(GDBusConnection* arg0,
    const gchar* arg1, const gchar* arg2, const gchar* arg3, const gchar* arg4,
    GVariant* arg5, GError** arg6) {
  return MOCK_HOOK_P7(GlibMock, g_dbus_connection_emit_signal,
      arg0, arg1, arg2, arg3, arg4, arg5, arg6);
}

extern "C" gboolean g_dbus_connection_flush_sync(GDBusConnection* arg0,
    GCancellable* arg1, GError** arg2) {
  return MOCK_HOOK_P3(GlibMock, g_dbus_connection_flush_sync, arg0, arg1, arg2);
}

extern "C" const gchar* g_dbus_connection_get_unique_name(GDBusConnection* connection) {
  return MOCK_HOOK_P1(GlibMock, g_dbus_connection_get_unique_name, connection);
}

extern "C" guint g_bus_watch_name (GBusType bus_type, const gchar* name, GBusNameWatcherFlags flags,
    GBusNameAppearedCallback name_appeared_handler, GBusNameVanishedCallback name_vanished_handler,
    gpointer user_data, GDestroyNotify user_data_free_func) {
  return MOCK_HOOK_P7(GlibMock, g_bus_watch_name, bus_type, name, flags, name_appeared_handler,
      name_vanished_handler, user_data, user_data_free_func);
}

extern "C" GDBusMessage* g_dbus_message_new_method_call(const gchar* arg0,
    const gchar* arg1, const gchar* arg2, const gchar* arg3) {
  return MOCK_HOOK_P4(GlibMock, g_dbus_message_new_method_call, arg0, arg1, arg2,
      arg3);
}
