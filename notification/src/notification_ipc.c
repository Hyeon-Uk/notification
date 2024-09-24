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
#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <bundle_internal.h>

#include <notification_ipc.h>
#include <notification_db.h>
#include <notification_type.h>
#include <notification_private.h>
#include <notification_debug.h>
#include <notification_setting.h>
#include <notification_setting_internal.h>
#include <notification_internal.h>

#include <gio/gio.h>
#include <gio/gunixfdlist.h>

#define PROVIDER_BUS_NAME "org.tizen.data_provider_service"
#define PROVIDER_OBJECT_PATH "/org/tizen/data_provider_service"
#define PROVIDER_NOTI_INTERFACE_NAME "org.tizen.data_provider_noti_service"
#define PROVIDER_NOTI_EVENT_INTERFACE_NAME "org.tizen.data_provider_noti_event_service"

#define DBUS_SERVICE_DBUS "org.freedesktop.DBus"
#define DBUS_PATH_DBUS "/org/freedesktop/DBus"
#define DBUS_INTERFACE_DBUS "org.freedesktop.DBus"
#define ERR_BUFFER_SIZE		1024

static const gchar *_bus_name = NULL;
static GDBusConnection *_gdbus_conn = NULL;
static guint provider_watcher_id = 0;
static int monitor_id = 0;
static int event_monitor_id = 0;
static int provider_monitor_id = 0;
static int is_master_started = 0;

typedef struct _result_cb_item {
	void (*result_cb)(int priv_id, int result, void *data);
	void *data;
} result_cb_item;

typedef struct _task_list task_list;
struct _task_list {
	task_list *prev;
	task_list *next;

	void (*task_cb)(void *data);
	void *data;
};

static task_list *g_task_list;

static int _ipc_monitor_register(uid_t uid);
static int _ipc_monitor_deregister(void);
static void _do_deffered_task(void);

static void _print_noti(notification_h noti)
{
	char *app_id = NULL;
	char *text = NULL;
	char *content = NULL;
	const char *tag = NULL;

	notification_get_pkgname(noti, &app_id);
	notification_get_text(noti, NOTIFICATION_TEXT_TYPE_TITLE, &text);
	notification_get_text(noti, NOTIFICATION_TEXT_TYPE_CONTENT, &content);
	notification_get_tag(noti, &tag);

	DBG("Noti-info : app_id[%s] title[%s] content[%s] tag[%s] priv_id[%d]",
			 app_id, text, content, tag, noti->priv_id);
}

static void __provider_appeared_cb(GDBusConnection *connection,
				   const gchar *name,
				   const gchar *name_owner,
				   gpointer user_data)
{
	INFO("name [%s] name_owner[%s]", name, name_owner);
	notification_reset_event_handler_list();
}

/* LCOV_EXCL_START */
static void __provider_vanished_cb(GDBusConnection *connection,
				   const gchar *name,
				   gpointer user_data)
{
	INFO("name [%s]", name);
}
/* LCOV_EXCL_STOP */

static int _dbus_init(void)
{
	GError *error = NULL;

	if (_gdbus_conn == NULL) {
		_gdbus_conn = g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, &error);

		if (_gdbus_conn == NULL) {
			/* LCOV_EXCL_START */
			if (error != NULL) {
				ERR("Failed to get dbus[%s]",
						error->message);
				g_error_free(error);
			}
			return NOTIFICATION_ERROR_IO_ERROR;
			/* LCOV_EXCL_STOP */
		}
		_bus_name = g_dbus_connection_get_unique_name(_gdbus_conn);
		INFO("Connected bus name[%s]", _bus_name);

		notification_error_quark();
	}

	if (provider_watcher_id == 0) {
		provider_watcher_id = g_bus_watch_name(G_BUS_TYPE_SYSTEM,
					      PROVIDER_BUS_NAME,
					      G_BUS_NAME_WATCHER_FLAGS_NONE,
					      __provider_appeared_cb,
					      __provider_vanished_cb,
					      NULL,
					      NULL);
		DBG("Watching data-provider-master is [%s] watcher_id [%d]",
				 provider_watcher_id ? "success" : "fail", provider_watcher_id);
	}

	return NOTIFICATION_ERROR_NONE;
}

/* LCOV_EXCL_START */
int notification_ipc_is_master_ready(void)
{
	GVariant *result;
	GError *err = NULL;
	gboolean name_exist;
	int ret = NOTIFICATION_ERROR_NONE;

	ret = _dbus_init();
	if (ret != NOTIFICATION_ERROR_NONE) {
		ERR("Failed to init dbus connection[%d]", ret);
		is_master_started = 0;
		return is_master_started;
	}

	result = g_dbus_connection_call_sync(
			_gdbus_conn,
			DBUS_SERVICE_DBUS,
			DBUS_PATH_DBUS,
			DBUS_INTERFACE_DBUS,
			"NameHasOwner",
			g_variant_new("(s)", PROVIDER_BUS_NAME),
			G_VARIANT_TYPE("(b)"),
			G_DBUS_CALL_FLAGS_NONE,
			-1,
			NULL,
			&err);

	if (err || (result == NULL)) {
		if (err) {
			ERR("No reply[%s]", err->message);
			g_error_free(err);
		}
		ERR("Failed to ready master");
		is_master_started = 0;
	} else {
		g_variant_get(result, "(b)", &name_exist);

		if (!name_exist) {
			ERR("The master has been stopped, Not exsited name[%s]", PROVIDER_BUS_NAME);
			is_master_started = 0;
		} else {
			DBG("The master has been started");
			is_master_started = 1;
		}
	}

	if (result)
		g_variant_unref(result);

	return is_master_started;
}
/* LCOV_EXCL_STOP */

/* TODO: dbus activation isn't enough ? */
/*
 * store tasks when daemon stopped
 */
 /* LCOV_EXCL_START */
int notification_ipc_add_deffered_task(
		void (*deferred_task_cb)(void *data),
		void *user_data)
{
	task_list *list;
	task_list *list_new;

	list_new = (task_list *) malloc(sizeof(task_list));
	if (list_new == NULL)
		return NOTIFICATION_ERROR_OUT_OF_MEMORY;

	list_new->next = NULL;
	list_new->prev = NULL;

	list_new->task_cb = deferred_task_cb;
	list_new->data = user_data;

	if (g_task_list == NULL) {
		g_task_list = list_new;
	} else {
		list = g_task_list;

		while (list->next != NULL)
			list = list->next;

		list->next = list_new;
		list_new->prev = list;
	}

	return NOTIFICATION_ERROR_NONE;
}

int notification_ipc_del_deffered_task(
		void (*deferred_task_cb)(void *data))
{
	task_list *list_del;
	task_list *list_prev;
	task_list *list_next;

	list_del = g_task_list;

	if (list_del == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	while (list_del->prev != NULL)
		list_del = list_del->prev;

	do {
		if (list_del->task_cb == deferred_task_cb) {
			list_prev = list_del->prev;
			list_next = list_del->next;

			if (list_prev == NULL)
				g_task_list = list_next;
			else
				list_prev->next = list_next;

			if (list_next == NULL) {
				if (list_prev != NULL)
					list_prev->next = NULL;
			} else {
				list_next->prev = list_prev;
			}

			free(list_del);
			return NOTIFICATION_ERROR_NONE;
		}
		list_del = list_del->next;
	} while (list_del != NULL);

	return NOTIFICATION_ERROR_INVALID_PARAMETER;
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
static void _do_deffered_task(void)
{
	task_list *list_do;
	task_list *list_temp;

	if (g_task_list == NULL)
		return;

	list_do = g_task_list;
	g_task_list = NULL;

	while (list_do->prev != NULL)
		list_do = list_do->prev;

	while (list_do != NULL) {
		if (list_do->task_cb != NULL) {
			list_do->task_cb(list_do->data);
			DBG("called:%p", list_do->task_cb);
		}
		list_temp = list_do->next;
		free(list_do);
		list_do = list_temp;
	}
}
/* LCOV_EXCL_STOP */

/*!
 * functions to create operation list
 */
static notification_op *_ipc_create_op(notification_op_type_e type,
		int num_op, int *list_priv_id, int num_priv_id, notification_h *noti_list)
{
	int i;
	notification_op *op_list;

	if (num_op <= 0)
		return NULL;

	op_list = (notification_op *)malloc(sizeof(notification_op) * num_op);
	if (op_list == NULL) {
		/* LCOV_EXCL_START */
		ERR("Failed to alloc memory");
		return NULL;
		/* LCOV_EXCL_STOP */
	}

	memset(op_list, 0x0, sizeof(notification_op) * num_op);

	for (i = 0; i < num_op; i++) {
		(op_list + i)->type = type;
		if (list_priv_id != NULL)
			(op_list + i)->priv_id = *(list_priv_id + i);
		if (noti_list != NULL)
			(op_list + i)->noti = *(noti_list + i);
	}

	return op_list;
}

/*!
 * utility functions creating notification packet
 */
static inline char *_dup_string(const char *string)
{
	char *ret;
	char err_buf[ERR_BUFFER_SIZE];

	if (string == NULL || string[0] == '\0')
		return NULL;

	ret = strdup(string);
	if (!ret)
		ERR("Failed to strdup[%s]",
				strerror_r(errno, err_buf, sizeof(err_buf)));

	return ret;
}

static inline bundle *_create_bundle_from_bundle_raw(bundle_raw *string)
{
	if (string == NULL || string[0] == '\0')
		return NULL;

	return bundle_decode(string, strlen((char *)string));
}

/* LCOV_EXCL_START */
static void _add_noti_notify(GVariant *parameters)
{
	int ret;
	notification_h noti;
	notification_op *noti_op = NULL;
	GVariant *body = NULL;
	uid_t uid;

	WARN("add noti notify");
	noti = notification_create(NOTIFICATION_TYPE_NOTI);
	if (!noti) {
		ERR("Failed to create notification handle");
		return;
	}

	g_variant_get(parameters, "(v)", &body);
	notification_ipc_make_noti_from_gvariant(noti, body);
	_print_noti(noti);
	if (noti->flags_for_property & NOTIFICATION_PROP_DISABLE_UPDATE_ON_INSERT) {
		ERR("Disable changed callback[%d]", noti->flags_for_property);
		/* Disable changed cb */
	} else {
		/* Enable changed cb */
		noti_op = _ipc_create_op(NOTIFICATION_OP_INSERT, 1, &(noti->priv_id), 1, &noti);
		ret = notification_get_uid(noti, &uid);
		if (noti_op != NULL && ret == NOTIFICATION_ERROR_NONE)
			notification_call_changed_cb_for_uid(noti_op, 1, uid);
	}
	g_variant_unref(body);
	notification_free(noti);
	if (noti_op)
		free(noti_op);
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
static void _update_noti_notify(GVariant *parameters)
{
	int ret;
	notification_h noti;
	notification_op *noti_op = NULL;
	GVariant *body = NULL;
	uid_t uid;

	noti = notification_create(NOTIFICATION_TYPE_NOTI);
	if (!noti) {
		ERR("Failed to create notification handle");
		return;
	}

	g_variant_get(parameters, "(v)", &body);
	notification_ipc_make_noti_from_gvariant(noti, body);
	_print_noti(noti);

	noti_op = _ipc_create_op(NOTIFICATION_OP_UPDATE, 1, &(noti->priv_id), 1, &noti);
	ret = notification_get_uid(noti, &uid);
	if (noti_op != NULL && ret == NOTIFICATION_ERROR_NONE)
		notification_call_changed_cb_for_uid(noti_op, 1, uid);

	g_variant_unref(body);
	notification_free(noti);
	if (noti_op)
		free(noti_op);
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
static void _refresh_noti_notify(GVariant *parameters)
{
	uid_t uid;
	notification_op *noti_op = _ipc_create_op(NOTIFICATION_OP_REFRESH, 1, NULL, 0, NULL);

	g_variant_get(parameters, "(i)", &uid);

	if (noti_op != NULL) {
		notification_call_changed_cb_for_uid(noti_op, 1, uid);
		free(noti_op);
	}
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
static void _delete_single_notify(GVariant *parameters)
{
	int num_deleted;
	int priv_id;
	notification_op *noti_op;
	uid_t uid;

	/* num_deleted ?? */
	g_variant_get(parameters, "(iii)", &num_deleted, &priv_id, &uid);

	noti_op = _ipc_create_op(NOTIFICATION_OP_DELETE, 1, &priv_id, 1, NULL);
	if (noti_op != NULL) {
		notification_call_changed_cb_for_uid(noti_op, 1, uid);
		free(noti_op);
	}
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
static void _delete_multiple_notify(GVariant *parameters)
{
	int *buf;
	int idx = 0;
	int num;
	notification_op *noti_op;
	GVariantIter *iter;
	uid_t uid;

	g_variant_get(parameters, "(a(i)ii)", &iter, &num, &uid);
	if (num <= 0) {
		ERR("Invalid number to delete");
		return;
	}
	DBG("Deleted count[%d]", num);

	buf = (int *)malloc(sizeof(int) * num);
	if (buf == NULL) {
		ERR("Failed to alloc");
		g_variant_iter_free(iter);
		return;
	}

	while (idx < num && g_variant_iter_loop(iter, "(i)", &buf[idx])) {
		DBG("priv id[%d]", buf[idx]);
		idx++;
	}
	g_variant_iter_free(iter);

	noti_op = _ipc_create_op(NOTIFICATION_OP_DELETE, idx, buf, idx, NULL);
	if (noti_op == NULL) {
		ERR("Failed to create op");
		free(buf);
		return;
	}

	notification_call_changed_cb_for_uid(noti_op, idx, uid);
	free(noti_op);
	free(buf);
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
static void _delete_by_display_applist_notify(GVariant *parameters)
{
	int *buf;
	int idx = 0;
	int num;
	notification_op *noti_op;
	GVariantIter *iter;
	uid_t uid;

	g_variant_get(parameters, "(a(i)ii)", &iter, &num, &uid);
	if (num <= 0) {
		ERR("Invalid number to delete");
		return;
	}

	DBG("Deleted count[%d]", num);

	buf = (int *)malloc(sizeof(int) * num);
	if (buf == NULL) {
		ERR("Failed to alloc");
		g_variant_iter_free(iter);
		return;
	}

	while (idx < num && g_variant_iter_loop(iter, "(i)", &buf[idx])) {
		DBG("priv id[%d]", buf[idx]);
		idx++;
	}
	g_variant_iter_free(iter);

	noti_op = _ipc_create_op(NOTIFICATION_OP_DELETE, idx, buf, idx, NULL);
	if (noti_op == NULL) {
		ERR("Failed to create op");
		free(buf);
		return;
	}

	notification_call_changed_cb_for_uid(noti_op, idx, uid);
	free(noti_op);
	free(buf);
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
static void _change_dnd_notify(GVariant *parameters)
{
	int do_not_disturb;
	uid_t uid;

	g_variant_get(parameters, "(ii)", &do_not_disturb, &uid);
	DBG("do_not_disturb[%d], uid[%d]", do_not_disturb, uid);

	notification_call_dnd_changed_cb_for_uid(do_not_disturb, uid);
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
static void _handle_noti_notify(GDBusConnection *connection,
		const gchar     *sender_name,
		const gchar     *object_path,
		const gchar     *interface_name,
		const gchar     *signal_name,
		GVariant        *parameters,
		gpointer         user_data)
{
	WARN("own_name : %s signal_name: %s",
			g_dbus_connection_get_unique_name(connection),
			signal_name);

	if (g_strcmp0(signal_name, "add_noti_notify") == 0)
		_add_noti_notify(parameters);
	else if (g_strcmp0(signal_name, "update_noti_notify") == 0)
		_update_noti_notify(parameters);
	else if (g_strcmp0(signal_name, "delete_single_notify") == 0)
		_delete_single_notify(parameters);
	else if (g_strcmp0(signal_name, "delete_multiple_notify") == 0)
		_delete_multiple_notify(parameters);
	else if (g_strcmp0(signal_name, "refresh_noti_notify") == 0)
		_refresh_noti_notify(parameters);
	else if (g_strcmp0(signal_name, "change_dnd_notify") == 0)
		_change_dnd_notify(parameters);
	else if (g_strcmp0(signal_name, "delete_by_display_applist_notify") == 0)
		_delete_by_display_applist_notify(parameters);
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
static void _send_event(GVariant *parameters)
{
	int ret;
	int event_type;
	notification_h noti;
	GVariant *coupled_body;
	GVariant *body;

	noti = notification_create(NOTIFICATION_TYPE_NOTI);
	if (noti == NULL) {
		ERR("Failed to create notification handle");
		return;
	}

	g_variant_get(parameters, "(vi)", &coupled_body, &event_type);
	g_variant_get(coupled_body, "(v)", &body);

	ret = notification_ipc_make_noti_from_gvariant(noti, body);
	g_variant_unref(coupled_body);
	g_variant_unref(body);
	if (ret != NOTIFICATION_ERROR_NONE) {
		ERR("Failed to make notification handle from gvariant");
		notification_free(noti);
		return;
	}

	notification_call_event_handler_cb(noti, event_type);
	notification_free(noti);
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
static void _delete_event(GVariant *parameters)
{
	int priv_id;

	g_variant_get(parameters, "(i)", &priv_id);
	notification_delete_event_handler_cb(priv_id);
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
static void _disturb_app(GVariant *parameters)
{
	notification_call_disturb_cb();
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
static void _handle_noti_event_handler_notify(GDBusConnection *connection,
		const gchar     *sender_name,
		const gchar     *object_path,
		const gchar     *interface_name,
		const gchar     *signal_name,
		GVariant        *parameters,
		gpointer         user_data)
{
	WARN("own_name : %s signal_name: %s",
			 g_dbus_connection_get_unique_name(connection), signal_name);

	if (g_strcmp0(signal_name, "send_event") == 0)
		_send_event(parameters);
	else if (g_strcmp0(signal_name, "delete_noti") == 0)
		_delete_event(parameters);
	else if (g_strcmp0(signal_name, "disturb_app") == 0)
		_disturb_app(parameters);
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
static int _dbus_event_handler_signal_init(void)
{
	int id;
	int ret = NOTIFICATION_ERROR_NONE;

	if (event_monitor_id == 0) {
		id = g_dbus_connection_signal_subscribe(_gdbus_conn,
				PROVIDER_BUS_NAME,
				PROVIDER_NOTI_EVENT_INTERFACE_NAME,	/*	interface */
				NULL,					/*	member */
				PROVIDER_OBJECT_PATH,			/*	path */
				NULL,					/*	arg0 */
				G_DBUS_SIGNAL_FLAGS_NONE,
				_handle_noti_event_handler_notify,
				NULL,
				NULL);

		WARN("subscribe id[%d]", id);
		if (id == 0) {
			ret = NOTIFICATION_ERROR_IO_ERROR;
			ERR("Failed to subscribe connection signal");
		} else {
			event_monitor_id = id;
		}
	}

	return ret;
}

static int _dbus_signal_init(void)
{
	int id;
	int ret = NOTIFICATION_ERROR_NONE;

	if (monitor_id == 0) {
		id = g_dbus_connection_signal_subscribe(_gdbus_conn,
				PROVIDER_BUS_NAME,
				PROVIDER_NOTI_INTERFACE_NAME,	/*	interface */
				NULL,				/*	member */
				PROVIDER_OBJECT_PATH,		/*	path */
				NULL,				/*	arg0 */
				G_DBUS_SIGNAL_FLAGS_NONE,
				_handle_noti_notify,
				NULL,
				NULL);

		WARN("subscribe id : %d", id);
		if (id == 0) {
			ret = NOTIFICATION_ERROR_IO_ERROR;
			ERR("Failed to register dbus_interface");
		} else {
			monitor_id = id;
			ret = NOTIFICATION_ERROR_NONE;
		}
	}

	return ret;
}
/* LCOV_EXCL_STOP */

static GDBusMessage *__get_new_msg(GVariant *body, const char *cmd)
{
	GDBusMessage *msg = NULL;

	msg = g_dbus_message_new_method_call(
			PROVIDER_BUS_NAME,
			PROVIDER_OBJECT_PATH,
			PROVIDER_NOTI_INTERFACE_NAME,
			cmd);
	if (!msg) {
		/* LCOV_EXCL_START */
		ERR("Failed to alloc new method call");
		if (body)
			g_variant_unref(body);
		return NULL;
		/* LCOV_EXCL_STOP */
	}

	if (body != NULL)
		g_dbus_message_set_body(msg, body);

	return msg;
}

static int __send_message(GDBusMessage *msg, GDBusMessage **reply, const char *cmd)
{
	int ret = NOTIFICATION_ERROR_NONE;
	GError *g_err = NULL;

	*reply = g_dbus_connection_send_message_with_reply_sync(
			_gdbus_conn,
			msg,
			G_DBUS_SEND_MESSAGE_FLAGS_NONE,
			-1,
			NULL,
			NULL,
			&g_err);

	if (!*reply) {
		/* LCOV_EXCL_START */
		ret = NOTIFICATION_ERROR_SERVICE_NOT_READY;
		if (g_err != NULL) {
			ERR("No reply. cmd[%s] err[%s]", cmd, g_err->message);
			if (g_err->code == G_DBUS_ERROR_ACCESS_DENIED)
				ret = NOTIFICATION_ERROR_PERMISSION_DENIED;
			g_error_free(g_err);
		}
		return ret;
		/* LCOV_EXCL_STOP */
	}

	if (g_dbus_message_to_gerror(*reply, &g_err)) {
		if (g_err->code == G_DBUS_ERROR_ACCESS_DENIED)
			ret = NOTIFICATION_ERROR_PERMISSION_DENIED;
		else
			ret = g_err->code;
		g_error_free(g_err);
		return ret;
	}

	INFO("Success to send message[%s]", cmd);
	return NOTIFICATION_ERROR_NONE;
}

static int _send_sync_noti_with_fd(int fd, GVariant *body, GDBusMessage **reply, char *cmd)
{
	int ret = NOTIFICATION_ERROR_NONE;
	GDBusMessage *msg = NULL;
	GUnixFDList *fd_list = NULL;
	GError *g_err = NULL;

	fd_list = g_unix_fd_list_new();
	g_unix_fd_list_append(fd_list, fd, &g_err);
	if (g_err != NULL) {
		/* LCOV_EXCL_START */
		ERR("g_unix_fd_list_append [%s]", g_err->message);
		g_object_unref(fd_list);
		g_error_free(g_err);
		return NOTIFICATION_ERROR_IO_ERROR;
		/* LCOV_EXCL_STOP */
	}

	msg = __get_new_msg(body, cmd);
	if (msg == NULL) {
		g_object_unref(fd_list);
		return NOTIFICATION_ERROR_IO_ERROR;
	}

	g_dbus_message_set_unix_fd_list(msg, fd_list);

	ret = __send_message(msg, reply, cmd);

	g_object_unref(msg);
	g_object_unref(fd_list);

	INFO("Done - send sync message with fd list [%d]", ret);
	return ret;
}

static int _send_sync_noti(GVariant *body, GDBusMessage **reply, char *cmd)
{
	int ret = NOTIFICATION_ERROR_NONE;
	GDBusMessage *msg = NULL;

	msg = __get_new_msg(body, cmd);
	if (msg == NULL)
		return NOTIFICATION_ERROR_IO_ERROR;

	ret = __send_message(msg, reply, cmd);

	if (msg)
		g_object_unref(msg);

	INFO("Done - send sync message [%d]", ret);
	return ret;
}

/* LCOV_EXCL_START */
static void _send_message_with_reply_async_cb(GDBusConnection *connection,
		GAsyncResult *res,
		gpointer user_data)
{
	GVariant *body;
	int result = NOTIFICATION_ERROR_NONE;
	int priv_id;
	GDBusMessage *reply = NULL;
	GError *err = NULL;
	result_cb_item *cb_item = (result_cb_item *)user_data;

	if (cb_item == NULL) {
		ERR("Failed to get a callback item");
		return;
	}

	reply = g_dbus_connection_send_message_with_reply_finish(
			connection,
			res,
			&err);

	if (!reply) {
		if (err != NULL) {
			ERR("No reply[%s]", err->message);
			g_error_free(err);
		}
		result = NOTIFICATION_ERROR_SERVICE_NOT_READY;

	} else if (g_dbus_message_to_gerror(reply, &err)) {
		if (err->code == G_DBUS_ERROR_ACCESS_DENIED)
			result = NOTIFICATION_ERROR_PERMISSION_DENIED;
		else
			result = err->code;

		ERR("Failed to send message[%s]", err->message);
		g_error_free(err);
	}

	INFO("Async message callback result[%d]", result);
	if (result == NOTIFICATION_ERROR_NONE) {
		body = g_dbus_message_get_body(reply);
		g_variant_get(body, "(i)", &priv_id);

		if (cb_item->result_cb)
			cb_item->result_cb(priv_id, result, cb_item->data);
	} else {
		if (cb_item->result_cb)
			cb_item->result_cb(NOTIFICATION_PRIV_ID_NONE, result, cb_item->data);
	}

	if (reply)
		g_object_unref(reply);
	free(cb_item);
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
static int _send_async_noti(GVariant *body, result_cb_item *cb_item, char *cmd)
{
	GDBusMessage *msg;

	msg = g_dbus_message_new_method_call(
			PROVIDER_BUS_NAME,
			PROVIDER_OBJECT_PATH,
			PROVIDER_NOTI_INTERFACE_NAME,
			cmd);
	if (!msg) {
		ERR("Failed to alloc new method call");
		return NOTIFICATION_ERROR_OUT_OF_MEMORY;
	}

	if (g_variant_is_floating(body))
		g_variant_ref(body);

	if (body != NULL)
		g_dbus_message_set_body(msg, body);

	g_dbus_connection_send_message_with_reply(
			_gdbus_conn,
			msg,
			G_DBUS_SEND_MESSAGE_FLAGS_NONE,
			-1,
			NULL,
			NULL,
			(GAsyncReadyCallback)_send_message_with_reply_async_cb,
			cb_item);

	if (msg)
		g_object_unref(msg);

	DBG("Success to send async message");
	return NOTIFICATION_ERROR_NONE;
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
int notification_ipc_request_insert(notification_h noti, int *priv_id)
{
	int result;
	int id = NOTIFICATION_PRIV_ID_NONE;
	GDBusMessage *reply = NULL;
	GVariant *body;
	GVariant *reply_body;

	WARN("request insert noti");
	result = _dbus_init();
	if (result != NOTIFICATION_ERROR_NONE) {
		/* LCOV_EXCL_START */
		ERR("Failed to init dbus connection[%d]", result);
		return result;
		/* LCOV_EXCL_STOP */
	}

	/* Initialize private ID */
	noti->group_id = NOTIFICATION_GROUP_ID_NONE;
	noti->internal_group_id = NOTIFICATION_GROUP_ID_NONE;

	_print_noti(noti);
	body = notification_ipc_make_gvariant_from_noti(noti, false);
	if (body == NULL) {
		/* LCOV_EXCL_START */
		ERR("Failed to make gvariant from notification handle");
		return NOTIFICATION_ERROR_OUT_OF_MEMORY;
		/* LCOV_EXCL_STOP */
	}

	result = _send_sync_noti(body, &reply, "add_noti");
	DBG("_send_sync_noti %d", result);

	if (result == NOTIFICATION_ERROR_NONE) {
		reply_body = g_dbus_message_get_body(reply);
		g_variant_get(reply_body, "(i)", &id);

		if (priv_id != NULL)
			*priv_id = id;
	}

	if (reply)
		g_object_unref(reply);

	WARN("priv_id[%d] result[%d]", id, result);
	return result;
}
/* LCOV_EXCL_STOP */

int notification_ipc_request_update(notification_h noti)
{
	int result;
	int priv_id = NOTIFICATION_PRIV_ID_NONE;

	GDBusMessage *reply = NULL;
	GVariant *body;
	GVariant *reply_body;

	result = _dbus_init();
	if (result != NOTIFICATION_ERROR_NONE) {
		/* LCOV_EXCL_START */
		ERR("Failed to init dbus connection[%d]", result);
		return result;
		/* LCOV_EXCL_STOP */
	}

	body = notification_ipc_make_gvariant_from_noti(noti, false);
	if (body == NULL) {
		/* LCOV_EXCL_START */
		ERR("cannot make gvariant");
		return NOTIFICATION_ERROR_OUT_OF_MEMORY;
		/* LCOV_EXCL_STOP */
	}

	result = _send_sync_noti(body, &reply, "update_noti");
	if (result == NOTIFICATION_ERROR_NONE) {
		reply_body = g_dbus_message_get_body(reply);
		g_variant_get(reply_body, "(i)", &priv_id);
	}

	if (reply)
		g_object_unref(reply);

	DBG("priv_id[%d] result[%d]", priv_id, result);
	return result;
}

/* LCOV_EXCL_START */
int notification_ipc_request_update_async(notification_h noti,
		void (*result_cb)(int priv_id, int result, void *data), void *user_data)
{
	int result;
	result_cb_item *cb_item;
	GVariant *body;

	result = _dbus_init();
	if (result != NOTIFICATION_ERROR_NONE) {
		ERR("Failed to init dbus connection[%d]", result);
		return result;
	}

	cb_item = calloc(1, sizeof(result_cb_item));
	if (cb_item == NULL)
		return NOTIFICATION_ERROR_OUT_OF_MEMORY;

	cb_item->result_cb = result_cb;
	cb_item->data = user_data;

	body = notification_ipc_make_gvariant_from_noti(noti, false);
	if (body == NULL) {
		ERR("Failed to make gvariant from notification handle");
		free(cb_item);
		return NOTIFICATION_ERROR_OUT_OF_MEMORY;
	}

	result = _send_async_noti(body, cb_item, "update_noti");
	DBG("Update async result[%d]", result);

	if (result != NOTIFICATION_ERROR_NONE) {
		free(cb_item);
		cb_item = NULL;
	}

	g_variant_unref(body);

	return result;
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
int notification_ipc_request_refresh(uid_t uid)
{
	int result;
	GDBusMessage *reply = NULL;
	GVariant *body;

	result = _dbus_init();
	if (result != NOTIFICATION_ERROR_NONE) {
		ERR("Failed to init dbus connection[%d]", result);
		return result;
	}

	body = g_variant_new("(i)", uid);
	result = _send_sync_noti(body, &reply, "refresh_noti");

	if (reply)
		g_object_unref(reply);

	DBG("result[%d]", result);
	return result;
}
/* LCOV_EXCL_STOP */

int notification_ipc_request_delete_single(notification_type_e type, char *app_id, int priv_id, uid_t uid)
{
	int result;
	int id;
	GDBusMessage *reply = NULL;
	GVariant *body;
	GVariant *reply_body;

	WARN("request del noti [%d]", priv_id);
	result = _dbus_init();
	if (result != NOTIFICATION_ERROR_NONE) {
		/* LCOV_EXCL_START */
		ERR("Failed to init dbus connection[%d]", result);
		return result;
		/* LCOV_EXCL_STOP */
	}

	body = g_variant_new("(sii)", app_id, priv_id, uid);
	result = _send_sync_noti(body, &reply, "del_noti_single");

	if (result == NOTIFICATION_ERROR_NONE) {
		reply_body = g_dbus_message_get_body(reply);
		g_variant_get(reply_body, "(i)", &id);
	}

	if (reply)
		g_object_unref(reply);

	WARN("result[%d]", result);
	return result;
}

int notification_ipc_request_delete_multiple(notification_type_e type, char *app_id, uid_t uid)
{
	int result;
	int num_deleted;
	GVariant *body;
	GVariant *reply_body;
	GDBusMessage *reply = NULL;

	result = _dbus_init();
	if (result != NOTIFICATION_ERROR_NONE) {
		/* LCOV_EXCL_START */
		ERR("Failed to init dbus connection[%d]", result);
		return result;
		/* LCOV_EXCL_STOP */
	}

	if (!app_id)
		app_id = "";

	body = g_variant_new("(sii)", app_id, type, uid);
	result = _send_sync_noti(body, &reply, "del_noti_multiple");

	if (result == NOTIFICATION_ERROR_NONE) {
		reply_body = g_dbus_message_get_body(reply);
		g_variant_get(reply_body, "(i)", &num_deleted);
		DBG("Deleted count[%d]", num_deleted);
	}

	if (reply)
		g_object_unref(reply);

	DBG("result[%d]", result);
	return result;
}

/* LCOV_EXCL_START */
int notification_ipc_request_delete_by_display_applist(int display_applist, uid_t uid)
{
	int result;
	int num_deleted;
	GVariant *body;
	GVariant *reply_body;
	GDBusMessage *reply = NULL;

	result = _dbus_init();
	if (result != NOTIFICATION_ERROR_NONE) {
		ERR("Failed to init dbus connection[%d]", result);
		return result;
	}

	body = g_variant_new("(ii)", display_applist, uid);

	result = _send_sync_noti(body, &reply, "del_noti_by_display_applist");
	if (result == NOTIFICATION_ERROR_NONE) {
		reply_body = g_dbus_message_get_body(reply);
		g_variant_get(reply_body, "(i)", &num_deleted);
		DBG("Deleted count[%d]", num_deleted);
	}

	if (reply)
		g_object_unref(reply);

	DBG("result[%d]", result);
	return result;
}
/* LCOV_EXCL_STOP */

int notification_ipc_request_load_noti_by_tag(notification_h noti, const char *app_id, const char *tag, uid_t uid)
{
	int result;
	GDBusMessage *reply = NULL;
	GVariant *body;
	GVariant *reply_body;
	GVariant *noti_body;

	result = _dbus_init();
	if (result != NOTIFICATION_ERROR_NONE) {
		/* LCOV_EXCL_START */
		ERR("Failed to init dbus connection[%d]", result);
		return result;
		/* LCOV_EXCL_STOP */
	}

	body = g_variant_new("(ssi)", app_id, tag, uid);
	result = _send_sync_noti(body, &reply, "load_noti_by_tag");

	if (result == NOTIFICATION_ERROR_NONE) {
		reply_body = g_dbus_message_get_body(reply);
		g_variant_get(reply_body, "(v)", &noti_body);

		notification_ipc_make_noti_from_gvariant(noti, noti_body);
		g_variant_unref(noti_body);
		_print_noti(noti);

	}

	if (reply)
		g_object_unref(reply);

	DBG("tag[%s] result[%d]", tag, result);
	return result;
}

/* LCOV_EXCL_START */
int notification_ipc_request_load_noti_by_priv_id(notification_h noti, const char *app_id, int priv_id, uid_t uid)
{
	int result;
	GDBusMessage *reply = NULL;
	GVariant *body;
	GVariant *reply_body;
	GVariant *noti_body;

	result = _dbus_init();
	if (result != NOTIFICATION_ERROR_NONE) {
		ERR("Failed to init dbus connection[%d]", result);
		return result;
	}

	if (!app_id)
		app_id = "";

	body = g_variant_new("(sii)", app_id, priv_id, uid);
	result = _send_sync_noti(body, &reply, "load_noti_by_priv_id");

	if (result == NOTIFICATION_ERROR_NONE) {
		reply_body = g_dbus_message_get_body(reply);
		g_variant_get(reply_body, "(v)", &noti_body);

		notification_ipc_make_noti_from_gvariant(noti, noti_body);
		g_variant_unref(noti_body);
		_print_noti(noti);
	}

	if (reply)
		g_object_unref(reply);

	DBG("priv id[%d], result[%d]", priv_id, result);
	return result;
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
int notification_ipc_request_get_count(notification_type_e type,
		    const char *app_id, int group_id, int priv_id, int *count, uid_t uid)
{
	int result;
	GDBusMessage *reply = NULL;
	GVariant *body;
	GVariant *reply_body;
	int re_count;

	result = _dbus_init();
	if (result != NOTIFICATION_ERROR_NONE) {
		ERR("Failed to init dbus connection[%d]", result);
		return result;
	}

	body = g_variant_new("(isiii)", type, app_id, group_id, priv_id, uid);
	result = _send_sync_noti(body, &reply, "get_noti_count");

	if (result == NOTIFICATION_ERROR_NONE) {
		reply_body = g_dbus_message_get_body(reply);
		g_variant_get(reply_body, "(i)", &re_count);

		*count = re_count;
		DBG("notification count[%d]", re_count);
	}

	if (reply)
		g_object_unref(reply);

	DBG("Count notification result[%d]", result);
	return result;
}
/* LCOV_EXCL_STOP */

static int __receive_list_from_socket(int fd, notification_list_h *list, int list_count)
{
	int ret = NOTIFICATION_ERROR_NONE;
	char *data = NULL;
	unsigned int data_size = 0;
	unsigned int buf_size = 0;
	notification_h noti;
	GVariant *noti_body;
	GVariant *reply_body;

	ret = notification_ipc_socket_get_read_buf_size(fd, &buf_size);
	if (ret != NOTIFICATION_ERROR_NONE) {
		ERR("get read buf size");
		return NOTIFICATION_ERROR_IO_ERROR;
	}

	data = (char *)calloc(buf_size, sizeof(char));
	if (data == NULL) {
		ERR("OOM - socket buffer");
		return NOTIFICATION_ERROR_OUT_OF_MEMORY;
	}

	while (list_count > 0) {
		ret = notification_ipc_socket_read(fd, (char *)&data_size, sizeof(data_size));
		if (ret != NOTIFICATION_ERROR_NONE) {
			ERR("socket read buf [%d]", ret);
			goto out;
		}

		if (data_size > buf_size) {
			buf_size = data_size;
			if (data)
				free(data);
			data = (char *)calloc(data_size, sizeof(char));
			if (data == NULL) {
				ret = NOTIFICATION_ERROR_OUT_OF_MEMORY;
				ERR("OOM - socket bulk buffer");
				goto out;
			}
		}

		ret = notification_ipc_socket_read(fd, data, data_size);
		if (ret != NOTIFICATION_ERROR_NONE) {
			ERR("socket read buf [%d]", ret);
			goto out;
		}

		noti = (notification_h)calloc(1, sizeof(struct _notification));
		if (noti == NULL) {
			ret = NOTIFICATION_ERROR_OUT_OF_MEMORY;
			ERR("OOM - notification handle");
			goto out;
		}

		reply_body = g_variant_new_from_data(G_VARIANT_TYPE("(v)"),
				data, data_size, TRUE, NULL, NULL);
		g_variant_get(reply_body, "(v)", &noti_body);
		notification_ipc_make_noti_from_gvariant(noti, noti_body);
		*list = notification_list_append(*list, noti);
		g_variant_unref(noti_body);
		g_variant_unref(reply_body);
		list_count--;
	}

out:
	if (ret != NOTIFICATION_ERROR_NONE)
		notification_free_list(*list);
	if (data)
		free(data);

	return ret;
}

int notification_ipc_request_load_noti_grouping_list(notification_type_e type,
		int count, int count_per_page, notification_list_h *list, uid_t uid)
{
#define RCV_SOCK 0
#define SND_SOCK 1
	int result;
	int sockfd[2] = { 0, };
	int list_count;
	GDBusMessage *reply = NULL;
	GVariant *body;
	GVariant *reply_body;

	result = _dbus_init();
	if (result != NOTIFICATION_ERROR_NONE) {
		/* LCOV_EXCL_START */
		ERR("Failed to init dbus connection[%d]", result);
		return result;
		/* LCOV_EXCL_STOP */
	}

	result = notification_ipc_socket_pair(sockfd);
	if (result != NOTIFICATION_ERROR_NONE)
		return result;
	INFO("socket receive[%d] send[%d]", sockfd[RCV_SOCK], sockfd[SND_SOCK]);

	body = g_variant_new("(iiii)", type, count, count_per_page, uid);
	result = _send_sync_noti_with_fd(sockfd[SND_SOCK], body, &reply, "load_noti_grouping_list");
	if (result == NOTIFICATION_ERROR_NONE) {
		reply_body = g_dbus_message_get_body(reply);
		g_variant_get(reply_body, "(i)", &list_count);
		result = __receive_list_from_socket(sockfd[RCV_SOCK], list, list_count);
	}

	if (reply)
		g_object_unref(reply);
	if (sockfd[RCV_SOCK])
		close(sockfd[RCV_SOCK]);
	if (sockfd[SND_SOCK])
		close(sockfd[SND_SOCK]);

	INFO("result [%d]", result);
	return result;
}

int notification_ipc_request_load_noti_detail_list(const char *app_id,
		int group_id,
		int priv_id,
		int count,
		notification_list_h *list,
		uid_t uid)
{
	int result;
	GDBusMessage *reply = NULL;
	GVariant *body;
	GVariant *reply_body;
	GVariant *iter_body;
	GVariantIter *iter;
	notification_h noti;
	GVariant *noti_body;

	result = _dbus_init();
	if (result != NOTIFICATION_ERROR_NONE) {
		ERR("Failed to init dbus connection[%d]", result);
		return result;
	}

	body = g_variant_new("(siiii)", app_id, group_id, priv_id, count, uid);
	result = _send_sync_noti(body, &reply, "load_noti_detail_list");

	if (result == NOTIFICATION_ERROR_NONE) {
		reply_body = g_dbus_message_get_body(reply);
		g_variant_get(reply_body, "(a(v))", &iter);

		while (g_variant_iter_loop(iter, "(v)", &iter_body)) {
			noti = notification_create(NOTIFICATION_TYPE_NOTI);
			if (!noti) {
				result = NOTIFICATION_ERROR_OUT_OF_MEMORY;
				ERR("failed to create a notification");
				notification_free_list(*list);
				break;
			}
			g_variant_get(iter_body, "(v)", &noti_body);
			notification_ipc_make_noti_from_gvariant(noti, noti_body);
			*list = notification_list_append(*list, noti);
			g_variant_unref(noti_body);
		}
		g_variant_iter_free(iter);
	}

	if (reply)
		g_object_unref(reply);

	DBG("result[%d]", result);
	return result;
}

/* LCOV_EXCL_START */
int notification_ipc_request_get_setting_array(
		notification_setting_h *setting_array,
		int *count,
		uid_t uid)
{
	int result;
	GDBusMessage *reply = NULL;
	GVariant *reply_body;
	GVariant *iter_body;
	GVariantIter *iter;
	int setting_cnt;
	notification_setting_h result_setting_array;
	notification_setting_h temp;
	int setting_idx;

	result = _dbus_init();
	if (result != NOTIFICATION_ERROR_NONE) {
		ERR("Failed to init dbus connection[%d]", result);
		return result;
	}

	result = _send_sync_noti(g_variant_new("(i)", uid), &reply, "get_setting_array");

	if (result == NOTIFICATION_ERROR_NONE) {
		reply_body = g_dbus_message_get_body(reply);
		g_variant_get(reply_body, "(ia(v))", &setting_cnt, &iter);

		DBG("get setting arr cnt: %d", setting_cnt);
		result_setting_array = (struct notification_setting *)malloc(sizeof(struct notification_setting) * setting_cnt);
		if (result_setting_array == NULL) {
			ERR("malloc failed");
			g_object_unref(reply);
			g_variant_iter_free(iter);
			return NOTIFICATION_ERROR_OUT_OF_MEMORY;
		}

		setting_idx = 0;
		while (g_variant_iter_loop(iter, "(v)", &iter_body)) {
			temp = result_setting_array + setting_idx;
			notification_ipc_make_setting_from_gvariant(temp, iter_body);
			setting_idx++;
		}

		*count = setting_cnt;
		*setting_array = result_setting_array;
		g_variant_iter_free(iter);
	}

	if (reply)
		g_object_unref(reply);

	DBG("result[%d]", result);
	return result;
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
int notification_ipc_request_get_setting_by_app_id(
		const char *app_id, notification_setting_h *setting, uid_t uid)
{
	int result;
	GDBusMessage *reply = NULL;
	GVariant *body;
	GVariant *reply_body;
	GVariant *setting_body;
	notification_setting_h result_setting;

	result = _dbus_init();
	if (result != NOTIFICATION_ERROR_NONE) {
		ERR("Failed to init dbus connection[%d]", result);
		return result;
	}

	body = g_variant_new("(si)", app_id, uid);
	result = _send_sync_noti(body, &reply, "get_setting_by_app_id");

	if (result == NOTIFICATION_ERROR_NONE) {
		reply_body = g_dbus_message_get_body(reply);
		g_variant_get(reply_body, "(v)", &setting_body);

		result_setting = (struct notification_setting *)malloc(sizeof(struct notification_setting));
		if (result_setting == NULL) {
			ERR("malloc failed");
			g_object_unref(reply);
			g_variant_unref(body);
			return NOTIFICATION_ERROR_OUT_OF_MEMORY;
		}
		notification_ipc_make_setting_from_gvariant(result_setting, setting_body);

		*setting = result_setting;
		g_variant_unref(setting_body);
	}

	if (reply)
		g_object_unref(reply);

	DBG("result[%d]", result);
	return result;
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
int notification_ipc_request_load_system_setting(notification_system_setting_h *setting, uid_t uid)
{
	int result;
	int count;
	GDBusMessage *reply = NULL;
	GVariant *setting_body = NULL;
	GVariant *reply_body = NULL;
	GVariant *iter_body = NULL;
	GVariantIter *iter = NULL;
	notification_system_setting_h result_setting = NULL;
	dnd_allow_exception_h dnd_allow_exception;

	result = _dbus_init();
	if (result != NOTIFICATION_ERROR_NONE) {
		ERR("Failed to init dbus connection[%d]", result);
		return result;
	}

	result = _send_sync_noti(g_variant_new("(i)", uid), &reply, "load_system_setting");
	if (result == NOTIFICATION_ERROR_NONE) {
		reply_body = g_dbus_message_get_body(reply);
		g_variant_get(reply_body, "(v)", &setting_body);

		result_setting = (struct notification_system_setting *)calloc(1, sizeof(struct notification_system_setting));
		if (result_setting == NULL) {
			ERR("malloc failed");
			result = NOTIFICATION_ERROR_OUT_OF_MEMORY;
			goto out;
		}

		notification_ipc_make_system_setting_from_gvariant(result_setting, setting_body);

		if (reply) {
			g_object_unref(reply);
			reply = NULL;
		}

		result = _send_sync_noti(g_variant_new("(i)", uid), &reply, "load_dnd_allow_exception");
		if (result == NOTIFICATION_ERROR_NONE) {
			reply_body = g_dbus_message_get_body(reply);
			g_variant_get(reply_body, "(ia(v))", &count, &iter);

			while (g_variant_iter_loop(iter, "(v)", &iter_body)) {
				dnd_allow_exception = (dnd_allow_exception_h)calloc(1, sizeof(struct notification_system_setting_dnd_allow_exception));
				if (dnd_allow_exception == NULL) {
					ERR("Out of memory");
					result = NOTIFICATION_ERROR_OUT_OF_MEMORY;
					g_variant_unref(iter_body);
					goto out;
				}

				notification_ipc_make_dnd_allow_exception_from_gvariant(dnd_allow_exception, iter_body);
				result_setting->dnd_allow_exceptions = g_list_append(result_setting->dnd_allow_exceptions, dnd_allow_exception);
				g_variant_unref(iter_body);
			}
			*setting = result_setting;
		}
	}

out:
	if (result != NOTIFICATION_ERROR_NONE && result_setting)
		notification_system_setting_free_system_setting(result_setting);
	if (iter)
		g_variant_iter_free(iter);
	if (setting_body)
		g_variant_unref(setting_body);
	if (reply)
		g_object_unref(reply);

	DBG("result[%d]", result);
	return result;
}
/* LCOV_EXCL_START */

int notification_ipc_update_setting(notification_setting_h setting, uid_t uid)
{
	int result;
	GDBusMessage *reply = NULL;
	GVariant *body;

	result = _dbus_init();
	if (result != NOTIFICATION_ERROR_NONE) {
		/* LCOV_EXCL_START */
		ERR("Failed to init dbus connection[%d]", result);
		return result;
		/* LCOV_EXCL_STOP */
	}

	body = g_variant_new("(ssiiiiii)",
			setting->package_name,
			setting->app_id,
			(int)(setting->allow_to_notify),
			(int)(setting->do_not_disturb_except),
			(int)(setting->visibility_class),
			(int)(setting->pop_up_notification),
			(int)(setting->lock_screen_content_level),
			uid);

	result = _send_sync_noti(body, &reply, "update_noti_setting");

	if (reply)
		g_object_unref(reply);

	DBG("result[%d]", result);
	return result;
}

/* LCOV_EXCL_START */
int notification_ipc_update_system_setting(notification_system_setting_h system_setting, uid_t uid)
{
	int result;
	GDBusMessage *reply = NULL;
	GVariant *body;
	GList *list;
	dnd_allow_exception_h dnd_allow_exception;

	result = _dbus_init();
	if (result != NOTIFICATION_ERROR_NONE) {
		ERR("Failed to init dbus connection[%d]", result);
		return result;
	}

	body = g_variant_new("(iiiiiiiiii)",
			(int)(system_setting->do_not_disturb),
			(int)(system_setting->visibility_class),
			(int)(system_setting->dnd_schedule_enabled),
			(int)(system_setting->dnd_schedule_day),
			(int)(system_setting->dnd_start_hour),
			(int)(system_setting->dnd_start_min),
			(int)(system_setting->dnd_end_hour),
			(int)(system_setting->dnd_end_min),
			(int)(system_setting->lock_screen_content_level),
			uid);

	result = _send_sync_noti(body, &reply, "update_noti_sys_setting");

	/* update dnd_allow_exceptions */
	list = g_list_first(system_setting->dnd_allow_exceptions);

	for (; list != NULL; list = list->next) {
		dnd_allow_exception = list->data;

		body = g_variant_new("(iii)",
			(int)(dnd_allow_exception->type),
			(int)(dnd_allow_exception->value),
			uid);
		result = _send_sync_noti(body, &reply, "update_dnd_allow_exception");
	}

	if (reply)
		g_object_unref(reply);

	DBG("result[%d]", result);
	return result;
}
/* LCOV_EXCL_STOP */

int notification_ipc_request_save_as_template(notification_h noti, const char *template_name)
{
	int result;
	GDBusMessage *reply = NULL;
	GVariant *body;

	result = _dbus_init();
	if (result != NOTIFICATION_ERROR_NONE) {
		/* LCOV_EXCL_START */
		ERR("Failed to init dbus connection[%d]", result);
		return result;
		/* LCOV_EXCL_STOP */
	}

	body = notification_ipc_make_gvariant_from_noti(noti, false);
	if (body == NULL) {
		ERR("Failed to make gvariant from notification handle");
		return NOTIFICATION_ERROR_OUT_OF_MEMORY;
	}

	result = _send_sync_noti(g_variant_new("(vs)", body, template_name), &reply, "save_as_template");

	if (reply)
		g_object_unref(reply);

	DBG("result[%d]", result);
	return result;
}

int notification_ipc_request_create_from_template(notification_h noti, const char *template_name)
{
	int result;
	GDBusMessage *reply = NULL;
	GVariant *body;
	GVariant *reply_body;
	GVariant *noti_body;

	result = _dbus_init();
	if (result != NOTIFICATION_ERROR_NONE) {
		/* LCOV_EXCL_START */
		ERR("Failed to init dbus connection[%d]", result);
		return result;
		/* LCOV_EXCL_STOP */
	}

	body = g_variant_new("(s)", template_name);

	result = _send_sync_noti(body, &reply, "create_from_template");
	if (result == NOTIFICATION_ERROR_NONE) {
		reply_body = g_dbus_message_get_body(reply);
		g_variant_get(reply_body, "(v)", &noti_body);

		notification_ipc_make_noti_from_gvariant(noti, noti_body);
		g_variant_unref(noti_body);
		_print_noti(noti);
	}

	if (reply)
		g_object_unref(reply);

	DBG("result[%d]", result);

	return result;
}

/* LCOV_EXCL_START */
int notification_ipc_request_create_from_package_template(notification_h noti, const char *app_id, const char *template_name)
{
	int result;
	GDBusMessage *reply = NULL;
	GVariant *body;
	GVariant *reply_body;
	GVariant *noti_body;

	result = _dbus_init();
	if (result != NOTIFICATION_ERROR_NONE) {
		ERR("Failed to init dbus connection[%d]", result);
		return result;
	}

	body = g_variant_new("(ss)", app_id, template_name);

	result = _send_sync_noti(body, &reply, "create_from_package_template");
	if (result == NOTIFICATION_ERROR_NONE) {
		reply_body = g_dbus_message_get_body(reply);
		g_variant_get(reply_body, "(v)", &noti_body);

		notification_ipc_make_noti_from_gvariant(noti, noti_body);
		g_variant_unref(noti_body);
		_print_noti(noti);
	}

	if (reply)
		g_object_unref(reply);

	DBG("result[%d]", result);

	return result;
}
/* LCOV_EXCL_STOP */

int notification_ipc_get_noti_block_state(const char *app_id, int *do_not_disturb,
					  int *do_not_disturb_except, int *allow_to_notify,
					  uid_t uid)
{
	int ret;
	int dnd;
	int dnd_except;
	int allow;
	GDBusMessage *reply = NULL;
	GVariant *body;
	GVariant *reply_body;
	GVariant *result_body;

	if (app_id == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	ret = _dbus_init();
	if (ret != NOTIFICATION_ERROR_NONE) {
		/* LCOV_EXCL_START */
		ERR("Failed to init dbus connection[%d]", ret);
		return ret;
		/* LCOV_EXCL_STOP */
	}

	body = g_variant_new("(si)", app_id, uid);

	ret = _send_sync_noti(body, &reply, "get_noti_block_state");

	if (ret == NOTIFICATION_ERROR_NONE) {
		reply_body = g_dbus_message_get_body(reply);
		g_variant_get(reply_body, "(v)", &result_body);
		g_variant_get(result_body, "(iii)", &dnd, &dnd_except, &allow);
		*do_not_disturb = dnd;
		*do_not_disturb_except = dnd_except;
		*allow_to_notify = allow;
		g_variant_unref(result_body);
	}

	if (reply)
		g_object_unref(reply);

	DBG("result[%d]", ret);

	return ret;
}

/* LCOV_EXCL_START */
int notification_ipc_request_get_all_count(notification_type_e type, int *count, uid_t uid)
{
	int ret;
	int ret_count = -1;
	GDBusMessage *reply = NULL;
	GVariant *reply_body = NULL;

	ret = _dbus_init();
	if (ret != NOTIFICATION_ERROR_NONE) {
		ERR("Failed to init dbus connection[%d]", ret);
		return ret;
	}

	ret = _send_sync_noti(g_variant_new("(ii)", type, uid),
			&reply, "get_noti_all_count");
	if (ret == NOTIFICATION_ERROR_NONE) {
		reply_body = g_dbus_message_get_body(reply);
		g_variant_get(reply_body, "(i)", &ret_count);
		*count = ret_count;
	}

	if (reply)
		g_object_unref(reply);

	return ret;
}

int notification_ipc_send_event(notification_h noti, int event_type, int priv_id)
{
	int ret;
	GVariant *body = NULL;
	GDBusMessage *reply = NULL;

	ret = _dbus_init();
	if (ret != NOTIFICATION_ERROR_NONE) {
		/* LCOV_EXCL_START */
		ERR("Failed to init dbus connection[%d]", ret);
		return ret;
		/* LCOV_EXCL_STOP */
	}

	if (priv_id > 0) {
		ret = _send_sync_noti(g_variant_new("(ii)", priv_id, event_type), &reply, "send_noti_event_by_priv_id");
	} else {
		body = notification_ipc_make_gvariant_from_noti(noti, false);
		if (body == NULL) {
			ERR("Can't make gvariant to noti");
			return NOTIFICATION_ERROR_OUT_OF_MEMORY;
		}

		ret = _send_sync_noti(g_variant_new("(vi)", body, event_type), &reply, "send_noti_event");
	}

	if (reply)
		g_object_unref(reply);

	DBG("result[%d]", ret);

	return ret;
}

int notification_ipc_check_event_receiver(int priv_id, bool *available)
{
	int ret;
	int ret_available;
	GVariant *reply_body = NULL;
	GDBusMessage *reply = NULL;

	ret = _dbus_init();
	if (ret != NOTIFICATION_ERROR_NONE) {
		/* LCOV_EXCL_START */
		ERR("Failed to init dbus connection[%d]", ret);
		return ret;
		/* LCOV_EXCL_STOP */
	}

	ret = _send_sync_noti(g_variant_new("(i)", priv_id), &reply, "check_event_receiver");
	if (ret == NOTIFICATION_ERROR_NONE) {
		reply_body = g_dbus_message_get_body(reply);
		g_variant_get(reply_body, "(i)", &ret_available);
		*available = (bool)ret_available;
	}

	if (reply)
		g_object_unref(reply);

	DBG("result[%d]", ret);
	return ret;
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
void notification_ipc_reset_event_handler(int priv_id)
{
	int ret;
	GDBusMessage *reply = NULL;

	ret = _dbus_init();
	if (ret != NOTIFICATION_ERROR_NONE) {
		ERR("Failed to init dbus connection[%d]", ret);
		return;
	}

	_send_sync_noti(g_variant_new("(i)", priv_id), &reply, "reset_event_handler");

	if (reply)
		g_object_unref(reply);
}
/* LCOV_EXCL_STOP */

EXPORT_API GVariant *notification_ipc_make_gvariant_from_noti(notification_h noti, bool translate)
{
	DBG("make gvariant from noti");
	int i = 0;
	int b_encode_len = 0;
	bundle_raw *args = NULL;
	bundle_raw *group_args = NULL;
	bundle_raw *b_image_path = NULL;
	bundle_raw *b_priv_image_path = NULL;
	bundle_raw *b_execute_option = NULL;
	bundle_raw *b_service_responding = NULL;
	bundle_raw *b_service_single_launch = NULL;
	bundle_raw *b_service_multi_launch = NULL;
	bundle_raw *b_event_handler[NOTIFICATION_EVENT_TYPE_MAX+1] = {NULL, };
	bundle_raw *b_text = NULL;
	bundle_raw *b_key = NULL;
	bundle_raw *b_format_args = NULL;
	GVariant *body = NULL;
	GVariant *result_body = NULL;
	GVariantBuilder builder;

	if (translate)
		notification_translate_localized_text(noti);

	g_variant_builder_init(&builder, G_VARIANT_TYPE("a{iv}"));
	g_variant_builder_add(&builder, "{iv}", NOTIFICATION_DATA_TYPE_NOTI_TYPE, g_variant_new_int32(noti->type));
	g_variant_builder_add(&builder, "{iv}", NOTIFICATION_DATA_TYPE_LAYOUT, g_variant_new_int32(noti->layout));
	g_variant_builder_add(&builder, "{iv}", NOTIFICATION_DATA_TYPE_GROUP_ID, g_variant_new_int32(noti->group_id));
	g_variant_builder_add(&builder, "{iv}", NOTIFICATION_DATA_TYPE_INTERNAL_GROUP_ID, g_variant_new_int32(noti->internal_group_id));
	g_variant_builder_add(&builder, "{iv}", NOTIFICATION_DATA_TYPE_PRIV_ID, g_variant_new_int32(noti->priv_id));
	g_variant_builder_add(&builder, "{iv}", NOTIFICATION_DATA_TYPE_PKG_ID, g_variant_new_string((const gchar *)noti->pkg_id));
	g_variant_builder_add(&builder, "{iv}", NOTIFICATION_DATA_TYPE_CALLER_APP_ID, g_variant_new_string((const gchar *)noti->caller_app_id));
	g_variant_builder_add(&builder, "{iv}", NOTIFICATION_DATA_TYPE_DISPLAY_APPLIST, g_variant_new_int32(noti->display_applist));

	if (noti->args) {
		bundle_encode(noti->args, (bundle_raw **)&args, &b_encode_len);
		g_variant_builder_add(&builder, "{iv}", NOTIFICATION_DATA_TYPE_ARGS, g_variant_new_string((const gchar *)args));

		if (args)
			bundle_free_encoded_rawdata(&args);
	}

	if (noti->group_args) {
		bundle_encode(noti->group_args, (bundle_raw **)&group_args,
			      &b_encode_len);
		g_variant_builder_add(&builder, "{iv}", NOTIFICATION_DATA_TYPE_GROUP_ARGS, g_variant_new_string((const gchar *)group_args));

		if (group_args)
			bundle_free_encoded_rawdata(&group_args);
	}

	if (noti->b_execute_option) {
		bundle_encode(noti->b_execute_option,
			      (bundle_raw **)&b_execute_option, &b_encode_len);
		g_variant_builder_add(&builder, "{iv}", NOTIFICATION_DATA_TYPE_EXECUTE_OPTION, g_variant_new_string((const gchar *)b_execute_option));

		if (b_execute_option)
			bundle_free_encoded_rawdata(&b_execute_option);
	}

	if (noti->b_service_responding) {
		bundle_encode(noti->b_service_responding,
			      (bundle_raw **)&b_service_responding, &b_encode_len);
		g_variant_builder_add(&builder, "{iv}", NOTIFICATION_DATA_TYPE_SERVICE_RESPONDING, g_variant_new_string((const gchar *)b_service_responding));

		if (b_service_responding)
			bundle_free_encoded_rawdata(&b_service_responding);
	}

	if (noti->b_service_single_launch) {
		bundle_encode(noti->b_service_single_launch,
			      (bundle_raw **)&b_service_single_launch, &b_encode_len);
		g_variant_builder_add(&builder, "{iv}", NOTIFICATION_DATA_TYPE_SERVICE_SINGLE_LAUNCH, g_variant_new_string((const gchar *)b_service_single_launch));

		if (b_service_single_launch)
			bundle_free_encoded_rawdata(&b_service_single_launch);
	}

	if (noti->b_service_multi_launch) {
		bundle_encode(noti->b_service_multi_launch,
			      (bundle_raw **)&b_service_multi_launch, &b_encode_len);
		g_variant_builder_add(&builder, "{iv}", NOTIFICATION_DATA_TYPE_SERVICE_MULTI_LAUNCH, g_variant_new_string((const gchar *)b_service_multi_launch));

		if (b_service_multi_launch)
			bundle_free_encoded_rawdata(&b_service_multi_launch);
	}

	for (i = 0; i <= NOTIFICATION_EVENT_TYPE_MAX; i++) {
		if (noti->b_event_handler[i]) {
			bundle_encode(noti->b_event_handler[i],
					(bundle_raw **)&b_event_handler[i], &b_encode_len);
			g_variant_builder_add(&builder, "{iv}", NOTIFICATION_DATA_TYPE_BUTTON1_EVENT + i, g_variant_new_string((const gchar *)b_event_handler[i]));

			if (b_event_handler[i])
				bundle_free_encoded_rawdata(&b_event_handler[i]);
		}
	}

	if (noti->launch_app_id)
		g_variant_builder_add(&builder, "{iv}", NOTIFICATION_DATA_TYPE_LAUNCH_APP_ID, g_variant_new_string((const gchar *)noti->launch_app_id));

	if (noti->domain != NULL)
		g_variant_builder_add(&builder, "{iv}", NOTIFICATION_DATA_TYPE_DOMAIN, g_variant_new_string((const gchar *)noti->domain));

	if (noti->dir != NULL)
		g_variant_builder_add(&builder, "{iv}", NOTIFICATION_DATA_TYPE_DIR, g_variant_new_string((const gchar *)noti->dir));

	if (noti->b_text) {
		bundle_encode(noti->b_text, (bundle_raw **)&b_text, &b_encode_len);
		g_variant_builder_add(&builder, "{iv}", NOTIFICATION_DATA_TYPE_TEXT, g_variant_new_string((const gchar *)b_text));

		if (b_text)
			bundle_free_encoded_rawdata(&b_text);
	}

	if (noti->b_key) {
		bundle_encode(noti->b_key, (bundle_raw **)&b_key, &b_encode_len);
		g_variant_builder_add(&builder, "{iv}", NOTIFICATION_DATA_TYPE_KEY, g_variant_new_string((const gchar *)b_key));

		if (b_key)
			bundle_free_encoded_rawdata(&b_key);
	}

	if (noti->b_format_args) {
		bundle_encode(noti->b_format_args,
			      (bundle_raw **)&b_format_args, &b_encode_len);
		g_variant_builder_add(&builder, "{iv}", NOTIFICATION_DATA_TYPE_FORMAT_ARGS, g_variant_new_string((const gchar *)b_format_args));

		if (b_format_args)
			bundle_free_encoded_rawdata(&b_format_args);
	}

	if (noti->num_format_args != 0)
		g_variant_builder_add(&builder, "{iv}", NOTIFICATION_DATA_TYPE_NUM_FORMAT_ARGS, g_variant_new_int32(noti->num_format_args));

	if (noti->b_image_path) {
		bundle_encode(noti->b_image_path,
			      (bundle_raw **)&b_image_path, &b_encode_len);
		g_variant_builder_add(&builder, "{iv}", NOTIFICATION_DATA_TYPE_IMAGE_PATH, g_variant_new_string((const gchar *)b_image_path));

		if (b_image_path)
			bundle_free_encoded_rawdata(&b_image_path);
	}

	if (noti->b_priv_image_path) {
		bundle_encode(noti->b_priv_image_path,
			      (bundle_raw **)&b_priv_image_path, &b_encode_len);
		g_variant_builder_add(&builder, "{iv}", NOTIFICATION_DATA_TYPE_PRIV_IMAGE_PATH, g_variant_new_string((const gchar *)b_priv_image_path));

		if (b_priv_image_path)
			bundle_free_encoded_rawdata(&b_priv_image_path);
	}

	g_variant_builder_add(&builder, "{iv}", NOTIFICATION_DATA_TYPE_SOUND_TYPE, g_variant_new_int32(noti->sound_type));

	if (noti->sound_path)
		g_variant_builder_add(&builder, "{iv}", NOTIFICATION_DATA_TYPE_SOUND_PATH, g_variant_new_string((const gchar *)noti->sound_path));

	if (noti->priv_sound_path)
		g_variant_builder_add(&builder, "{iv}", NOTIFICATION_DATA_TYPE_PRIV_SOUND_PATH, g_variant_new_string((const gchar *)noti->priv_sound_path));

	g_variant_builder_add(&builder, "{iv}", NOTIFICATION_DATA_TYPE_VIBRATION_TYPE, g_variant_new_int32(noti->vibration_type));

	if (noti->vibration_path)
		g_variant_builder_add(&builder, "{iv}", NOTIFICATION_DATA_TYPE_VIBRATION_PATH, g_variant_new_string((const gchar *)noti->vibration_path));

	if (noti->priv_vibration_path)
		g_variant_builder_add(&builder, "{iv}", NOTIFICATION_DATA_TYPE_PRIV_VIBRATION_PATH, g_variant_new_string((const gchar *)noti->priv_vibration_path));

	g_variant_builder_add(&builder, "{iv}", NOTIFICATION_DATA_TYPE_LED_OPERATION, g_variant_new_int32(noti->led_operation));

	if (noti->led_argb != 0)
		g_variant_builder_add(&builder, "{iv}", NOTIFICATION_DATA_TYPE_LED_ARGB, g_variant_new_int32(noti->led_argb));

	if (noti->led_on_ms != 0)
		g_variant_builder_add(&builder, "{iv}", NOTIFICATION_DATA_TYPE_LED_ON_MS, g_variant_new_int32(noti->led_on_ms));

	if (noti->led_off_ms != 0)
		g_variant_builder_add(&builder, "{iv}", NOTIFICATION_DATA_TYPE_LED_OFF_MS, g_variant_new_int32(noti->led_off_ms));

	if (noti->time != 0)
		g_variant_builder_add(&builder, "{iv}", NOTIFICATION_DATA_TYPE_TIME, g_variant_new_int32(noti->time));

	if (noti->insert_time != 0)
		g_variant_builder_add(&builder, "{iv}", NOTIFICATION_DATA_TYPE_INSERT_TIME, g_variant_new_int32(noti->insert_time));

	if (noti->flags_for_property != 0)
		g_variant_builder_add(&builder, "{iv}", NOTIFICATION_DATA_TYPE_FLAGS_FOR_PROPERTY, g_variant_new_int32(noti->flags_for_property));

	if (noti->progress_size != 0.0)
		g_variant_builder_add(&builder, "{iv}", NOTIFICATION_DATA_TYPE_PROGRESS_SIZE, g_variant_new_double(noti->progress_size));

	if (noti->progress_percentage != 0.0)
		g_variant_builder_add(&builder, "{iv}", NOTIFICATION_DATA_TYPE_PROGRESS_PERCENTAGE, g_variant_new_double(noti->progress_percentage));

	if (noti->app_icon_path != NULL)
		g_variant_builder_add(&builder, "{iv}", NOTIFICATION_DATA_TYPE_APP_ICON_PATH, g_variant_new_string((const gchar *)noti->app_icon_path));
	if (noti->app_label != NULL)
		g_variant_builder_add(&builder, "{iv}", NOTIFICATION_DATA_TYPE_APP_LABEL, g_variant_new_string((const gchar *)noti->app_label));
	if (noti->temp_title != NULL)
		g_variant_builder_add(&builder, "{iv}", NOTIFICATION_DATA_TYPE_TEMP_TITLE, g_variant_new_string((const gchar *)noti->temp_title));
	if (noti->temp_content != NULL)
		g_variant_builder_add(&builder, "{iv}", NOTIFICATION_DATA_TYPE_TEMP_CONTENT, g_variant_new_string((const gchar *)noti->temp_content));
	if (noti->tag != NULL)
		g_variant_builder_add(&builder, "{iv}", NOTIFICATION_DATA_TYPE_TAG, g_variant_new_string((const gchar *)noti->tag));

	g_variant_builder_add(&builder, "{iv}", NOTIFICATION_DATA_TYPE_ONGOING_FLAG, g_variant_new_int32(noti->ongoing_flag));
	g_variant_builder_add(&builder, "{iv}", NOTIFICATION_DATA_TYPE_ONGOING_VALUE_TYPE, g_variant_new_int32(noti->ongoing_value_type));
	g_variant_builder_add(&builder, "{iv}", NOTIFICATION_DATA_TYPE_ONGOING_CURRENT, g_variant_new_int32(noti->ongoing_current));
	g_variant_builder_add(&builder, "{iv}", NOTIFICATION_DATA_TYPE_ONGOING_DURATION, g_variant_new_int32(noti->ongoing_duration));
	g_variant_builder_add(&builder, "{iv}", NOTIFICATION_DATA_TYPE_AUTO_REMOVE, g_variant_new_int32(noti->auto_remove));
	g_variant_builder_add(&builder, "{iv}", NOTIFICATION_DATA_TYPE_DEFAULT_BUTTON, g_variant_new_int32(noti->default_button_index));
	g_variant_builder_add(&builder, "{iv}", NOTIFICATION_DATA_TYPE_HIDE_TIMEOUT, g_variant_new_int32(noti->hide_timeout));
	g_variant_builder_add(&builder, "{iv}", NOTIFICATION_DATA_TYPE_DELETE_TIMEOUT, g_variant_new_int32(noti->delete_timeout));
	g_variant_builder_add(&builder, "{iv}", NOTIFICATION_DATA_TYPE_TEXT_INPUT_MAX_LENGTH, g_variant_new_int32(noti->text_input_max_length));
	g_variant_builder_add(&builder, "{iv}", NOTIFICATION_DATA_TYPE_EVENT_FLAG, g_variant_new_int32(noti->event_flag));
	g_variant_builder_add(&builder, "{iv}", NOTIFICATION_DATA_TYPE_TRANSLATION, g_variant_new_int32(noti->is_translation));
	g_variant_builder_add(&builder, "{iv}", NOTIFICATION_DATA_TYPE_EXTENSION_IMAGE_SIZE, g_variant_new_int32(noti->extension_image_size));
	g_variant_builder_add(&builder, "{iv}", NOTIFICATION_DATA_TYPE_CHECK_BOX, g_variant_new_int32(noti->check_box));
	g_variant_builder_add(&builder, "{iv}", NOTIFICATION_DATA_TYPE_CHECK_BOX_VALUE, g_variant_new_int32(noti->check_box_value));
	g_variant_builder_add(&builder, "{iv}", NOTIFICATION_DATA_TYPE_UID, g_variant_new_int32(noti->uid));

	result_body = g_variant_builder_end(&builder);
	body = g_variant_new("(v)", result_body);

	return body;
}

static gboolean _variant_to_int_dict(GHashTable **dict, GVariant *variant)
{
	GVariantIter iter;
	int key;
	int *hash_key;
	GVariant *value;

	*dict = g_hash_table_new_full(g_int_hash, g_int_equal, g_free, (GDestroyNotify)g_variant_unref);
	if (*dict == NULL)
		return FALSE;

	g_variant_iter_init(&iter, variant);
	while (g_variant_iter_next(&iter, "{iv}", &key, &value)) {
		hash_key = (int *)calloc(sizeof(int), 1);
		if (hash_key == NULL) {
			/* LCOV_EXCL_START */
			g_hash_table_unref(*dict);
			return FALSE;
			/* LCOV_EXCL_STOP */
		}
		*hash_key = key;
		g_hash_table_insert(*dict, (gpointer)hash_key, value);
	}
	return TRUE;
}

static gboolean _variant_dict_lookup(GHashTable *dict,
		int key,
		const gchar  *format_string,
		...)
{
	GVariant *value;
	va_list ap;

	value = g_hash_table_lookup(dict, (gpointer)&key);

	if (value == NULL || !g_variant_check_format_string(value, format_string, FALSE))
		return FALSE;

	va_start(ap, format_string);
	g_variant_get_va(value, format_string, NULL, &ap);
	va_end(ap);

	return TRUE;
}

/*!
 * functions creating notification packet
 */
EXPORT_API int notification_ipc_make_noti_from_gvariant(notification_h noti,
		GVariant *variant) {

	DBG("make noti from GVariant");
	GHashTable *dict;

	int i;
	char *pkg_id = NULL;
	char *caller_app_id = NULL;
	char *launch_app_id = NULL;
	bundle_raw *args = NULL;
	bundle_raw *group_args = NULL;
	bundle_raw *b_execute_option = NULL;
	bundle_raw *b_service_responding = NULL;
	bundle_raw *b_service_single_launch = NULL;
	bundle_raw *b_service_multi_launch = NULL;
	bundle_raw *b_event_handler[NOTIFICATION_EVENT_TYPE_MAX+1] = { NULL, };
	char *domain = NULL;
	char *dir = NULL;
	bundle_raw *b_text = NULL;
	bundle_raw *b_key = NULL;
	bundle_raw *b_format_args = NULL;
	bundle_raw *b_image_path = NULL;
	bundle_raw *b_priv_image_path = NULL;
	char *sound_path = NULL;
	char *priv_sound_path = NULL;
	char *vibration_path = NULL;
	char *priv_vibration_path = NULL;
	char *app_icon_path = NULL;
	char *app_label = NULL;
	char *temp_title = NULL;
	char *temp_content = NULL;
	char *tag = NULL;

	if (noti == NULL) {
		ERR("Invalid noti NULL");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	if (variant == NULL) {
		ERR("Invalid variant NULL");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	if (!_variant_to_int_dict(&dict, variant))
		return NOTIFICATION_ERROR_OUT_OF_MEMORY;

	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_NOTI_TYPE, "i", &noti->type);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_LAYOUT, "i", &noti->layout);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_GROUP_ID, "i", &noti->group_id);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_INTERNAL_GROUP_ID, "i", &noti->internal_group_id);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_PRIV_ID, "i", &noti->priv_id);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_PKG_ID, "&s", &pkg_id);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_CALLER_APP_ID, "&s", &caller_app_id);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_LAUNCH_APP_ID, "&s", &launch_app_id);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_ARGS, "&s", &args);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_GROUP_ARGS, "&s", &group_args);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_EXECUTE_OPTION, "&s", &b_execute_option);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_SERVICE_RESPONDING, "&s", &b_service_responding);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_SERVICE_SINGLE_LAUNCH, "&s", &b_service_single_launch);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_SERVICE_MULTI_LAUNCH, "&s", &b_service_multi_launch);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_BUTTON1_EVENT, "&s", &b_event_handler[NOTIFICATION_EVENT_TYPE_CLICK_ON_BUTTON_1]);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_BUTTON2_EVENT, "&s", &b_event_handler[NOTIFICATION_EVENT_TYPE_CLICK_ON_BUTTON_2]);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_BUTTON3_EVENT, "&s", &b_event_handler[NOTIFICATION_EVENT_TYPE_CLICK_ON_BUTTON_3]);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_BUTTON4_EVENT, "&s", &b_event_handler[NOTIFICATION_EVENT_TYPE_CLICK_ON_BUTTON_4]);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_BUTTON5_EVENT, "&s", &b_event_handler[NOTIFICATION_EVENT_TYPE_CLICK_ON_BUTTON_5]);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_BUTTON6_EVENT, "&s", &b_event_handler[NOTIFICATION_EVENT_TYPE_CLICK_ON_BUTTON_6]);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_ICON_EVENT, "&s", &b_event_handler[NOTIFICATION_EVENT_TYPE_CLICK_ON_ICON]);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_THUMBNAIL_EVENT, "&s", &b_event_handler[NOTIFICATION_EVENT_TYPE_CLICK_ON_THUMBNAIL]);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_TEXT_INPUT_BUTTON_EVENT, "&s", &b_event_handler[NOTIFICATION_EVENT_TYPE_CLICK_ON_TEXT_INPUT_BUTTON]);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_BUTTON7_EVENT, "&s", &b_event_handler[NOTIFICATION_EVENT_TYPE_CLICK_ON_BUTTON_7]);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_BUTTON8_EVENT, "&s", &b_event_handler[NOTIFICATION_EVENT_TYPE_CLICK_ON_BUTTON_8]);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_BUTTON9_EVENT, "&s", &b_event_handler[NOTIFICATION_EVENT_TYPE_CLICK_ON_BUTTON_9]);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_BUTTON10_EVENT, "&s", &b_event_handler[NOTIFICATION_EVENT_TYPE_CLICK_ON_BUTTON_10]);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_DOMAIN, "&s", &domain);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_DIR, "&s", &dir);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_TEXT, "&s", &b_text);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_KEY, "&s", &b_key);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_FORMAT_ARGS, "&s", &b_format_args);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_NUM_FORMAT_ARGS, "i", &noti->num_format_args);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_IMAGE_PATH, "&s", &b_image_path);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_PRIV_IMAGE_PATH, "&s", &b_priv_image_path);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_SOUND_TYPE, "i", &noti->sound_type);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_SOUND_PATH, "&s", &sound_path);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_PRIV_SOUND_PATH, "&s", &priv_sound_path);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_VIBRATION_TYPE, "i", &noti->vibration_type);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_VIBRATION_PATH, "&s", &vibration_path);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_PRIV_VIBRATION_PATH, "&s", &priv_vibration_path);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_LED_OPERATION, "i", &noti->led_operation);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_LED_ARGB, "i", &noti->led_argb);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_LED_ON_MS, "i", &noti->led_on_ms);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_LED_OFF_MS, "i", &noti->led_off_ms);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_TIME, "i", &noti->time);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_INSERT_TIME, "i", &noti->insert_time);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_FLAGS_FOR_PROPERTY, "i", &noti->flags_for_property);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_DISPLAY_APPLIST, "i", &noti->display_applist);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_PROGRESS_SIZE, "d", &noti->progress_size);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_PROGRESS_PERCENTAGE, "d", &noti->progress_percentage);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_APP_ICON_PATH, "&s", &app_icon_path);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_APP_LABEL, "&s", &app_label);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_TEMP_TITLE, "&s", &temp_title);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_TEMP_CONTENT, "&s", &temp_content);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_TAG, "&s", &tag);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_ONGOING_FLAG, "i", &noti->ongoing_flag);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_ONGOING_VALUE_TYPE, "i", &noti->ongoing_value_type);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_ONGOING_CURRENT, "i", &noti->ongoing_current);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_ONGOING_DURATION, "i", &noti->ongoing_duration);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_AUTO_REMOVE, "i", &noti->auto_remove);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_DEFAULT_BUTTON, "i", &noti->default_button_index);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_HIDE_TIMEOUT, "i", &noti->hide_timeout);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_DELETE_TIMEOUT, "i", &noti->delete_timeout);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_TEXT_INPUT_MAX_LENGTH, "i", &noti->text_input_max_length);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_EVENT_FLAG, "i", &noti->event_flag);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_TRANSLATION, "i", &noti->is_translation);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_EXTENSION_IMAGE_SIZE, "i", &noti->extension_image_size);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_CHECK_BOX, "i", &noti->check_box);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_CHECK_BOX_VALUE, "i", &noti->check_box_value);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_UID, "i", &noti->uid);

	if (noti->pkg_id)
		free(noti->pkg_id);
	noti->pkg_id = _dup_string(pkg_id);

	if (noti->caller_app_id)
		free(noti->caller_app_id);
	noti->caller_app_id = _dup_string(caller_app_id);

	noti->launch_app_id = _dup_string(launch_app_id);

	noti->args = _create_bundle_from_bundle_raw(args);
	noti->group_args = _create_bundle_from_bundle_raw(group_args);
	noti->b_execute_option = _create_bundle_from_bundle_raw(b_execute_option);
	noti->b_service_responding = _create_bundle_from_bundle_raw(
			b_service_responding);
	noti->b_service_single_launch = _create_bundle_from_bundle_raw(
			b_service_single_launch);
	noti->b_service_multi_launch = _create_bundle_from_bundle_raw(
			b_service_multi_launch);
	for (i = 0; i <= NOTIFICATION_EVENT_TYPE_MAX; i++) {
		noti->b_event_handler[i] = _create_bundle_from_bundle_raw(
				b_event_handler[i]);
	}

	if (noti->domain)
		free(noti->domain);
	noti->domain = _dup_string(domain);

	if (noti->dir)
		free(noti->dir);
	noti->dir = _dup_string(dir);

	noti->b_text = _create_bundle_from_bundle_raw(b_text);
	noti->b_key = _create_bundle_from_bundle_raw(b_key);
	noti->b_format_args = _create_bundle_from_bundle_raw(b_format_args);
	noti->b_image_path = _create_bundle_from_bundle_raw(b_image_path);
	noti->b_priv_image_path = _create_bundle_from_bundle_raw(b_priv_image_path);
	noti->sound_path = _dup_string(sound_path);
	noti->priv_sound_path = _dup_string(priv_sound_path);
	noti->vibration_path = _dup_string(vibration_path);
	noti->priv_vibration_path = _dup_string(priv_vibration_path);
	noti->app_icon_path = _dup_string(app_icon_path);
	noti->app_label = _dup_string(app_label);
	noti->temp_title = _dup_string(temp_title);
	noti->temp_content = _dup_string(temp_content);
	noti->tag = _dup_string(tag);

	g_hash_table_unref(dict);

	return NOTIFICATION_ERROR_NONE;
}

/* LCOV_EXCL_START */
EXPORT_API GVariant *notification_ipc_make_gvariant_from_system_setting(struct notification_system_setting *noti_setting)
{
	GVariant *body = NULL;

	body = g_variant_new("(iiiiiiiii)",
			noti_setting->do_not_disturb,
			noti_setting->visibility_class,
			noti_setting->dnd_schedule_enabled,
			noti_setting->dnd_schedule_day,
			noti_setting->dnd_start_hour,
			noti_setting->dnd_start_min,
			noti_setting->dnd_end_hour,
			noti_setting->dnd_end_min,
			noti_setting->lock_screen_content_level);
	return body;
}

EXPORT_API int notification_ipc_make_system_setting_from_gvariant(struct notification_system_setting *noti_setting,
		GVariant *variant)
{
	int do_not_disturb;
	int visibility_class;
	int dnd_schedule_enabled;
	int dnd_schedule_day;
	int dnd_start_hour;
	int dnd_start_min;
	int dnd_end_hour;
	int dnd_end_min;
	int lock_screen_content_level;

	if (noti_setting == NULL) {
		ERR("Invalid setting handle");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	g_variant_get(variant,
			"(iiiiiiiii)",
			&do_not_disturb,
			&visibility_class,
			&dnd_schedule_enabled,
			&dnd_schedule_day,
			&dnd_start_hour,
			&dnd_start_min,
			&dnd_end_hour,
			&dnd_end_min,
			&lock_screen_content_level);

	DBG("system setting %d, %d, %d, %d, [%d:%d] [%d:%d], %d",
			do_not_disturb, visibility_class, dnd_schedule_enabled,
			dnd_schedule_day, dnd_start_hour, dnd_start_min,
			dnd_end_hour, dnd_end_min, lock_screen_content_level);

	noti_setting->do_not_disturb = do_not_disturb;
	noti_setting->visibility_class = visibility_class;
	noti_setting->dnd_schedule_enabled = dnd_schedule_enabled;
	noti_setting->dnd_schedule_day = dnd_schedule_day;
	noti_setting->dnd_start_hour = dnd_start_hour;
	noti_setting->dnd_start_min = dnd_start_min;
	noti_setting->dnd_end_hour = dnd_end_hour;
	noti_setting->dnd_end_min = dnd_end_min;
	noti_setting->lock_screen_content_level = lock_screen_content_level;

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API GVariant *notification_ipc_make_gvariant_from_setting(struct notification_setting *noti_setting)
{
	GVariant *body = NULL;

	body = g_variant_new("(ssiiiiii)",
			noti_setting->package_name,
			noti_setting->app_id,
			noti_setting->allow_to_notify,
			noti_setting->do_not_disturb_except,
			noti_setting->visibility_class,
			noti_setting->pop_up_notification,
			noti_setting->lock_screen_content_level,
			noti_setting->app_disabled);

	return body;
}

EXPORT_API int notification_ipc_make_setting_from_gvariant(struct notification_setting *noti_setting,
		GVariant *variant)
{
	char *pkgname;
	char *app_id;
	int allow_to_notify;
	int do_not_disturb_except;
	int visibility_class;
	int pop_up_notification;
	int lock_screen_content_level;
	int app_disabled;

	if (noti_setting == NULL || variant == NULL) {
		ERR("invalid data");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}
	g_variant_get(variant,
			"(&s&siiiiii)",
			&pkgname,
			&app_id,
			&allow_to_notify,
			&do_not_disturb_except,
			&visibility_class,
			&pop_up_notification,
			&lock_screen_content_level,
			&app_disabled);

	DBG("setting from variant %s !!", pkgname);

	noti_setting->package_name = _dup_string(pkgname);
	noti_setting->app_id = _dup_string(app_id);
	noti_setting->allow_to_notify = allow_to_notify;
	noti_setting->do_not_disturb_except = do_not_disturb_except;
	noti_setting->visibility_class = visibility_class;
	noti_setting->pop_up_notification = pop_up_notification;
	noti_setting->lock_screen_content_level = lock_screen_content_level;
	noti_setting->app_disabled = app_disabled;

	DBG("setting->pkgname[%s] pkgname[%s]",
			noti_setting->package_name, pkgname);

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API GVariant *notification_ipc_make_gvariant_from_dnd_allow_exception(struct notification_system_setting_dnd_allow_exception *dnd_allow_exception)
{
	GVariant *body = NULL;

	body = g_variant_new("(ii)",
		dnd_allow_exception->type,
		dnd_allow_exception->value);

	return body;
}

int notification_ipc_make_dnd_allow_exception_from_gvariant(struct notification_system_setting_dnd_allow_exception *dnd_allow_exception, GVariant *variant)
{
	int type;
	int value;

	if (dnd_allow_exception == NULL) {
		ERR("Invalid data");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	g_variant_get(variant, "(ii)", &type, &value);

	dnd_allow_exception->type = type;
	dnd_allow_exception->value = value;

	return NOTIFICATION_ERROR_NONE;
}

static int _send_service_register(uid_t uid)
{
	int result;
	notification_op *noti_op = NULL;
	GDBusMessage *reply = NULL;

	result = _send_sync_noti(g_variant_new("(i)", uid), &reply, "noti_service_register");
	if (reply)
		g_object_unref(reply);
	if (result != NOTIFICATION_ERROR_NONE) {
		ERR("send sync noti [%d]", result);
		return result;
	}

	is_master_started = 1;

	noti_op = _ipc_create_op(NOTIFICATION_OP_SERVICE_READY, 1, NULL, 1, NULL);
	if (noti_op != NULL) {
		notification_call_changed_cb_for_uid(noti_op, 1, uid);
		free(noti_op);
	} else {
		ERR("Failed to create op");
	}

	DBG("bus name[%s] result[%d]", _bus_name, result);
	return result;
}

static int _ipc_monitor_register(uid_t uid)
{
	return  _send_service_register(uid);
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
static void _on_name_appeared(GDBusConnection *connection,
		const gchar     *name,
		const gchar     *name_owner,
		gpointer         user_data)
{
	int uid = GPOINTER_TO_INT(user_data);

	WARN("uid[%d] name[%s]", uid, name);

	if (is_master_started == 0)
		_ipc_monitor_register(uid);

	/* TODO: dbus activation isn't enough ? */
	_do_deffered_task();
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
static void _on_name_vanished(GDBusConnection *connection,
		const gchar     *name,
		gpointer         user_data)
{
	int uid = GPOINTER_TO_INT(user_data);

	DBG("uid[%d] name[%s]", uid, name);
	is_master_started = 0;
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
int notification_ipc_monitor_init(uid_t uid)
{
	int ret;

	ret = _dbus_init();
	if (ret != NOTIFICATION_ERROR_NONE) {
		ERR("Failed to init dbus connection[%d]", ret);
		return ret;
	}

	ret = _dbus_signal_init();
	if (ret != NOTIFICATION_ERROR_NONE) {
		ERR("Failed to init signal[%d]", ret);
		return ret;
	}

	ret = _ipc_monitor_register(uid);
	if (ret != NOTIFICATION_ERROR_NONE) {
		ERR("Failed to register service[%d]", ret);
		return ret;
	}

	if (provider_monitor_id == 0) {
		provider_monitor_id = g_bus_watch_name_on_connection(
				_gdbus_conn,
				PROVIDER_BUS_NAME,
				G_BUS_NAME_WATCHER_FLAGS_NONE,
				_on_name_appeared,
				_on_name_vanished,
				GINT_TO_POINTER((int)uid),
				NULL);
		if (provider_monitor_id == 0) {
			g_dbus_connection_signal_unsubscribe(_gdbus_conn, monitor_id);
			monitor_id = 0;
			ERR("Failed to watch name");
			return NOTIFICATION_ERROR_IO_ERROR;
		}
	}

	return NOTIFICATION_ERROR_NONE;
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
static int _ipc_monitor_deregister(void)
{
	if (provider_monitor_id) {
		g_bus_unwatch_name(provider_monitor_id);
		provider_monitor_id = 0;
	}

	if (monitor_id) {
		g_dbus_connection_signal_unsubscribe(_gdbus_conn, monitor_id);
		monitor_id = 0;
	}

	return NOTIFICATION_ERROR_NONE;
}

int notification_ipc_monitor_fini(void)
{
	return  _ipc_monitor_deregister();
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
void notification_ipc_event_monitor_fini(void)
{
	if (event_monitor_id == 0)
		return;

	g_dbus_connection_signal_unsubscribe(_gdbus_conn, event_monitor_id);
	event_monitor_id = 0;
	WARN("event monitor fini");
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
int notification_ipc_event_monitor_init(void)
{
	WARN("event monitor init");
	return _dbus_event_handler_signal_init();
}
/* LCOV_EXCL_STOP */

int notification_ipc_request_register_dnd_app(uid_t uid, pid_t pid)
{
	int ret;
	GVariant *body;
	GDBusMessage *reply = NULL;

	WARN("request register dnd app");
	ret = _dbus_init();
	if (ret != NOTIFICATION_ERROR_NONE) {
		ERR("Failed to init dbus connection[%d]", ret);
		return ret;
	}

	body = g_variant_new("(ii)", uid, pid);

	ret = _send_sync_noti(body, &reply, "register_dnd_app");
	if (reply)
		g_object_unref(reply);

	WARN("send sync noti [%d]", ret);

	return ret;
}
