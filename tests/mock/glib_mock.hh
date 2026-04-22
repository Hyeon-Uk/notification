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

#ifndef UNIT_TESTS_MOCK_GLIB_MOCK_H_
#define UNIT_TESTS_MOCK_GLIB_MOCK_H_

#include <gio/gio.h>
#include <glib.h>
#include <gmock/gmock.h>

#include "module_mock.hh"

class GlibMock : public virtual ModuleMock {
 public:
  GlibMock() {
    using ::testing::_;
    using ::testing::Return;
    using ::testing::Invoke;
  }

  MOCK_METHOD1(g_file_new_for_path, GFile* (const char*));
  MOCK_METHOD2(g_file_query_exists, gboolean (GFile*, GCancellable*));
  MOCK_METHOD3(g_file_make_directory, gboolean (GFile*, GCancellable*, GError**));

  MOCK_METHOD2(access, int (const char*, int));
  MOCK_METHOD3(read, size_t(int , void*, size_t));

  MOCK_METHOD3(g_bus_get_sync, GDBusConnection*(GBusType, GCancellable*, GError**));

  MOCK_METHOD7(g_dbus_connection_send_message_with_reply_sync,
      GDBusMessage*(GDBusConnection*, GDBusMessage*, GDBusSendMessageFlags,
          gint, volatile guint32*, GCancellable*, GError**));

  MOCK_METHOD1(g_dbus_message_get_body, GVariant*(GDBusMessage*));
  MOCK_METHOD2(g_list_append, GList*(GList*, gpointer));
  MOCK_METHOD10(g_dbus_connection_signal_subscribe,
      guint(GDBusConnection*, const gchar*, const gchar*, const gchar*,
      const gchar*, const gchar*, GDBusSignalFlags, GDBusSignalCallback,
      gpointer, GDestroyNotify));
  MOCK_METHOD7(g_dbus_connection_emit_signal,
      gboolean(GDBusConnection*, const gchar*, const gchar*, const gchar*,
          const gchar*, GVariant*, GError**));
  MOCK_METHOD3(g_dbus_connection_flush_sync,
      gboolean(GDBusConnection*, GCancellable*, GError**));
  MOCK_METHOD1(g_dbus_connection_get_unique_name, const gchar*(GDBusConnection*));
  MOCK_METHOD7(g_bus_watch_name, guint(GBusType, const gchar*, GBusNameWatcherFlags,
      GBusNameAppearedCallback, GBusNameVanishedCallback, gpointer, GDestroyNotify));
  MOCK_METHOD4(g_dbus_message_new_method_call,
      GDBusMessage*(const gchar*, const gchar*, const gchar*, const gchar*));
};

/*
 * N1-T7: autoptr + builder-reuse counters.
 *
 * These are free-function counters (not gmock MOCK_METHODs) intentionally:
 * we need to count every call to g_object_unref / g_variant_builder_unref /
 * g_variant_builder_new coming out of the production code WITHOUT replacing
 * their real behavior (otherwise every other unit test that relies on real
 * glib object lifetimes would break).
 *
 * The extern "C" wrappers in glib_mock.cc bump these counters before
 * delegating to RTLD_NEXT. Unit tests call ResetN1T7Counters() in SetUp
 * and read Get*Count() afterwards to prove the scope-exit cleanup fired.
 */
namespace glib_mock_n1t7 {
  void ResetCounters();
  int GetObjectUnrefCount();
  int GetErrorFreeCount();
  int GetVariantBuilderNewCount();
  int GetVariantBuilderUnrefCount();
}  // namespace glib_mock_n1t7

#endif  // UNIT_TESTS_MOCK_GLIB_MOCK_H_
