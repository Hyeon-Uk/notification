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

/*
 * N8-A.1 (Stage 8): migrated from C to C++.
 *
 * Public symbols are advertised with C linkage by
 * notification_status.h / notification_status_internal.h's `extern "C"`
 * guards, so the definitions below are wrapped in an explicit
 * `extern "C"` block to keep the linkage matched.  The DBus signal
 * callback `__notification_status_message_dbus_callback` is a function
 * pointer registered via g_dbus_connection_signal_subscribe; GLib
 * declares the callback typedef inside its own `extern "C"` block,
 * which means the pointer type expects C linkage.  Putting the
 * definition inside `extern "C"` here makes the implementation match
 * the registered prototype exactly.
 *
 * The file-static struct `_message_cb_data` and the storage `md` stay
 * inside the `extern "C"` block too — they are accessed only from
 * functions that are themselves C-linkage, so keeping them inside the
 * block avoids language-linkage churn between forward decls and
 * definitions.
 *
 * `<string.h>` swaps to `<cstring>`.  Everything else is unchanged
 * behaviour-wise; the dbus wiring and the no-op contract for the
 * `_post` / `_cb_set` / `_cb_unset` trio are preserved.
 */

#include <sys/types.h>
#include <unistd.h>
#include <cstdio>
#include <cstring>
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

#define PATH_NAME    "/Org/Tizen/System/Notification/Status_message"
#define INTERFACE_NAME "org.tizen.system.notification.status_message"
#define MEMBER_NAME	"status_message"

extern "C" {

struct _message_cb_data {
	notification_status_message_cb callback;
	void *data;
	GDBusConnection *conn;
	uint message_id;
};

static struct _message_cb_data md;

/*
 * Race audit pass 2: the file-static `md` struct is touched from two
 * thread contexts in the 3rd-party app process this lib is loaded into:
 *
 *   - notification_status_monitor_message_cb_set / _unset run on the
 *     app thread that calls the EXPORT_API entry points.
 *   - __notification_status_message_dbus_callback runs on the
 *     GMainContext that owns the system-bus GDBusConnection.
 *
 * Pre-fix: the callback path read `md.callback` then dereferenced it
 * unlocked.  A concurrent _unset() between the NULL check and the
 * indirect call freed the user's `data` and left a dangling cb
 * pointer; the dispatch path then called through it (use-after-free).
 * Symmetric race: _set() writing `md.callback` / `md.data` / `md.conn`
 * tore values that the dispatch path observed half-updated.
 *
 * Wrap every read AND every write of `md` with a file-local recursive
 * mutex.  The dispatch path snapshots `callback`/`data` under the lock
 * and then invokes the user callback OUTSIDE the lock — invoking the
 * user callback under our lock would invite deadlock if the user
 * re-enters _set / _unset from inside their own callback.
 */
#define NOTIFICATION_STATUS_CTOR __attribute__((constructor))
#define NOTIFICATION_STATUS_DTOR __attribute__((destructor))

static GRecMutex __md_rec_mutex;

NOTIFICATION_STATUS_CTOR static void __notification_status_init(void)
{
	g_rec_mutex_init(&__md_rec_mutex);
}

NOTIFICATION_STATUS_DTOR static void __notification_status_fini(void)
{
	g_rec_mutex_clear(&__md_rec_mutex);
}

static void __notification_status_lock(void)
{
	g_rec_mutex_lock(&__md_rec_mutex);
}

static void __notification_status_unlock(void)
{
	g_rec_mutex_unlock(&__md_rec_mutex);
}

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
	notification_status_message_cb cb;
	void *cb_data;

	g_variant_get(parameters, "(&s)", &message);
	if (strlen(message) <= 0) {
		ERR("message is NULL");
		return;
	}

	/* Snapshot under the lock so we don't tear callback+data in the
	 * face of a concurrent _set / _unset. */
	__notification_status_lock();
	cb = md.callback;
	cb_data = md.data;
	__notification_status_unlock();

	if (!cb) {
		ERR("No callback");
		return;
	}

	/* Invoke OUTSIDE the lock: the user is allowed to call _set /
	 * _unset from their own callback. */
	cb(message, cb_data);
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
EXPORT_API
int notification_status_monitor_message_cb_set(notification_status_message_cb callback, void *user_data)
{
	GError *error = NULL;
	int ret = NOTIFICATION_ERROR_NONE;

	if (!callback)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	__notification_status_lock();

	if (md.conn == NULL) {
		md.conn = g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, &error);
		if (md.conn == NULL) {
			ERR("Failed to connect to the D-BUS Daemon[%s]",
						error->message);
			g_error_free(error);
			ret = NOTIFICATION_ERROR_FROM_DBUS;
			goto out;
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
			md.conn = NULL;
			ret = NOTIFICATION_ERROR_FROM_DBUS;
			goto out;
		}
	}

	md.callback = callback;
	md.data = user_data;

out:
	__notification_status_unlock();
	return ret;
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
EXPORT_API
int notification_status_monitor_message_cb_unset(void)
{
	__notification_status_lock();

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

	__notification_status_unlock();

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
