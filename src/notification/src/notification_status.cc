/*
 * Copyright (c) 2000 - 2017 Samsung Electronics Co., Ltd. All rights reserved.
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

#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <glib.h>
#include <gio/gio.h>

#include <notification.h>
#include <notification_db.h>
#include <notification_list.h>
#include <notification_noti.h>
#include <notification_debug.h>
#include <notification_private.h>
#include <notification_status.h>
#include <notification_status_internal.h>

extern "C" {

#define PATH_NAME    "/Org/Tizen/System/Notification/Status_message"
#define INTERFACE_NAME "org.tizen.system.notification.status_message"
#define MEMBER_NAME	"status_message"

struct _message_cb_data {
	notification_status_message_cb callback;
	void *data;
	GDBusConnection *conn;
	uint message_id;
};

static struct _message_cb_data md;

/* LCOV_EXCL_START */
static void __notification_status_message_dbus_callback(GDBusConnection *connection,
					const gchar *sender_name,
					const gchar *object_path,
					const gchar *interface_name,
					const gchar *signal_name,
					GVariant *parameters,
					gpointer user_data)
{
	char *message = NULL;

	g_variant_get(parameters, "(&s)", &message);
	if (strlen(message) <= 0) {
		ERR("message is NULL");
		return;
	}

	if (!md.callback) {
		ERR("No callback");
		return;
	}

	md.callback(message, md.data);
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
EXPORT_API
int notification_status_monitor_message_cb_set(notification_status_message_cb callback, void *user_data)
{
	GError *error = NULL;

	if (!callback)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	if (md.conn == NULL) {
		md.conn = g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, &error);
		if (md.conn == NULL) {
			ERR("Failed to connect to the D-BUS Daemon[%s]",
						error->message);
			g_error_free(error);
			return NOTIFICATION_ERROR_FROM_DBUS;
		}
	}

	if (!md.message_id) {
		md.message_id = g_dbus_connection_signal_subscribe(md.conn,
					NULL,
					INTERFACE_NAME,
					MEMBER_NAME,
					PATH_NAME,
					NULL,
					G_DBUS_SIGNAL_FLAGS_NONE,
					__notification_status_message_dbus_callback,
					NULL,
					NULL);
		if (md.message_id == 0) {
			ERR("Failed to subscribe signal");
			g_object_unref(md.conn);
			return NOTIFICATION_ERROR_FROM_DBUS;
		}
	}

	md.callback = callback;
	md.data = user_data;

	return NOTIFICATION_ERROR_NONE;
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
EXPORT_API
int notification_status_monitor_message_cb_unset(void)
{
	if (md.message_id) {
		g_dbus_connection_signal_unsubscribe(md.conn, md.message_id);
		md.message_id = 0;
	}

	if (md.conn) {
		g_object_unref(md.conn);
		md.conn = NULL;
	}

	md.callback = NULL;
	md.data = NULL;

	return NOTIFICATION_ERROR_NONE;
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
EXPORT_API
int notification_status_message_post(const char *message)
{
	GError *err = NULL;
	GDBusConnection *conn;
	GVariant *param;
	int ret = NOTIFICATION_ERROR_NONE;

	if (!message) {
		ERR("message is NULL");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	conn = g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, &err);
	if (conn == NULL) {
		ERR("Failed to connect to the D-BUS Daemon[%s]",
					err->message);
		ret = NOTIFICATION_ERROR_FROM_DBUS;
		goto end;
	}

	param = g_variant_new("(s)", message);

	if (g_dbus_connection_emit_signal(conn,
					NULL,
					PATH_NAME,
					INTERFACE_NAME,
					MEMBER_NAME,
					param,
					&err) == FALSE) {
		ERR("Failed to emit signal[%s]",
					err->message);
		ret = NOTIFICATION_ERROR_FROM_DBUS;
		goto end;
	}

	if (g_dbus_connection_flush_sync(conn, NULL, &err) == FALSE) {
		ERR("Failed to flush connection sync[%s]",
					err->message);
		ret = NOTIFICATION_ERROR_FROM_DBUS;
		goto end;
	}

end:
	if (err)
		g_error_free(err); /* LCOV_EXCL_LINE */

	if (conn)
		g_object_unref(conn);

	return ret;
}
/* LCOV_EXCL_STOP */

}  // extern "C"
