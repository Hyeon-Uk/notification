/*
 * Copyright (c) 2000 - 2016 Samsung Electronics Co., Ltd. All rights reserved.
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
#include <vconf.h>
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

	void (*task_cb) (void *data);
	void *data;
};

static task_list *g_task_list;

static int _ipc_monitor_register(uid_t uid);
static int _ipc_monitor_deregister(void);
static void _do_deffered_task(void);

static void _print_noti(notification_h noti)
{
	char *pkgname = NULL;
	char *text = NULL;
	char *content = NULL;
	const char *tag = NULL;
	notification_vibration_type_e vib_type = NOTIFICATION_VIBRATION_TYPE_NONE;

	notification_get_pkgname(noti, &pkgname);
	notification_get_text(noti, NOTIFICATION_TEXT_TYPE_TITLE, &text);
	notification_get_text(noti, NOTIFICATION_TEXT_TYPE_CONTENT, &content);
	notification_get_tag(noti, &tag);
	notification_get_vibration(noti, &vib_type, NULL);

	NOTIFICATION_DBG("client print_noti  pkgname  = %s ", pkgname);
	NOTIFICATION_DBG("client print_noti  title  = %s ", text);
	NOTIFICATION_DBG("client print_noti  content  = %s ", content);
	NOTIFICATION_DBG("client print_noti  tag  = %s ", tag);
	NOTIFICATION_DBG("client print_noti  priv_id  = %d ", noti->priv_id);
	NOTIFICATION_DBG("client print_noti  vibration_path  = %s ", noti->vibration_path);
	NOTIFICATION_DBG("client print_noti  vibration_type  = %d ", vib_type);
}

static inline char *_string_get(char *string)
{
	if (string == NULL)
		return NULL;

	if (string[0] == '\0')
		return NULL;

	return string;
}

static int _dbus_init()
{
	GError *error = NULL;

	if (_gdbus_conn == NULL) {
		_gdbus_conn = g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, &error);

		if (_gdbus_conn == NULL) {
			if (error != NULL) {
				NOTIFICATION_ERR("Failed to get dbus [%s]", error->message);
				g_error_free(error);
			}
			return NOTIFICATION_ERROR_IO_ERROR;
		}
		_bus_name = g_dbus_connection_get_unique_name(_gdbus_conn);
		NOTIFICATION_DBG("bus name : %s", _bus_name);

		notification_error_quark();

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
		NOTIFICATION_ERR("Can't init dbus %d", ret);
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
			NOTIFICATION_ERR("No reply. error = %s", err->message);
			g_error_free(err);
		}
		NOTIFICATION_ERR("is master ready fail");
		is_master_started = 0;
	} else {
		g_variant_get(result, "(b)", &name_exist);

		if (!name_exist) {
			NOTIFICATION_ERR("Name not exist %s", PROVIDER_BUS_NAME);
			NOTIFICATION_ERR("the master has been stopped");
			is_master_started = 0;
		} else {
			NOTIFICATION_DBG("the master has been started");
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
			NOTIFICATION_DBG("called:%p", list_do->task_cb);
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
		NOTIFICATION_ERR("malloc failed");
		return NULL;
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
		NOTIFICATION_ERR("Error: %s\n", strerror_r(errno, err_buf, sizeof(err_buf)));

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
	notification_op *noti_op;
	GVariant *body = NULL;
	uid_t uid;

	NOTIFICATION_DBG("add noti notify");
	noti = notification_create(NOTIFICATION_TYPE_NOTI);
	if (!noti) {
		NOTIFICATION_ERR("failed to create a notification");
		return;
	}

	g_variant_get(parameters, "(v)", &body);
	notification_ipc_make_noti_from_gvariant(noti, body);
	_print_noti(noti);
	if (noti->flags_for_property & NOTIFICATION_PROP_DISABLE_UPDATE_ON_INSERT) {
		NOTIFICATION_ERR("disable changed callback %d", noti->flags_for_property);
		/* Disable changed cb */
	} else {
		/* Enable changed cb */
		noti_op = _ipc_create_op(NOTIFICATION_OP_INSERT, 1, &(noti->priv_id), 1, &noti);
		ret = notification_get_uid(noti, &uid);
		if (noti_op != NULL && ret == NOTIFICATION_ERROR_NONE) {
			notification_call_changed_cb_for_uid(noti_op, 1, uid);
			free(noti_op);
		}
	}
	g_variant_unref(body);
	notification_free(noti);
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
static void _update_noti_notify(GVariant *parameters)
{
	int ret;
	notification_h noti;
	notification_op *noti_op;
	GVariant *body = NULL;
	uid_t uid;

	noti = notification_create(NOTIFICATION_TYPE_NOTI);
	if (!noti) {
		NOTIFICATION_ERR("failed to create a notification");
		return;
	}

	g_variant_get(parameters, "(v)", &body);
	notification_ipc_make_noti_from_gvariant(noti, body);
	_print_noti(noti);

	noti_op = _ipc_create_op(NOTIFICATION_OP_UPDATE, 1, &(noti->priv_id), 1, &noti);
	ret = notification_get_uid(noti, &uid);
	if (noti_op != NULL && ret == NOTIFICATION_ERROR_NONE) {
		notification_call_changed_cb_for_uid(noti_op, 1, uid);
		free(noti_op);
	}

	g_variant_unref(body);
	notification_free(noti);
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
	int buf[100] = {0,};
	int idx = 0;
	notification_op *noti_op;
	GVariantIter *iter;
	uid_t uid;

	g_variant_get(parameters, "(a(i)i)", &iter, &uid);
	while (g_variant_iter_loop(iter, "(i)", &buf[idx])) {
		NOTIFICATION_DBG("delete_noti_multiple priv_id : %d", buf[idx]);
		idx++;
	}
	g_variant_iter_free(iter);

	NOTIFICATION_DBG("data num deleted:%d", idx);
	noti_op = _ipc_create_op(NOTIFICATION_OP_DELETE, idx, buf, idx, NULL);
	if (noti_op == NULL) {
		NOTIFICATION_ERR("_ipc_create_op failed");
		return;
	}

	notification_call_changed_cb_for_uid(noti_op, idx, uid);
	free(noti_op);
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
static void _change_dnd_notify(GVariant *parameters)
{
	int do_not_disturb;
	uid_t uid;

	g_variant_get(parameters, "(ii)", &do_not_disturb, &uid);
	NOTIFICATION_DBG("do_not_disturb[%d], uid[%d]", do_not_disturb, uid);

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
	NOTIFICATION_DBG("own_name : %s signal_name: %s",
			 g_dbus_connection_get_unique_name(connection), signal_name);

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
}
/* LCOV_EXCL_STOP */

static void _send_event(GVariant *parameters)
{
	int ret;
	int event_type;
	notification_h noti;
	GVariant *coupled_body;
	GVariant *body;

	noti = notification_create(NOTIFICATION_TYPE_NOTI);
	if (noti == NULL) {
		NOTIFICATION_ERR("failed to create a notification");
		return;
	}

	g_variant_get(parameters, "(vi)", &coupled_body, &event_type);
	g_variant_get(coupled_body, "(v)", &body);

	ret = notification_ipc_make_noti_from_gvariant(noti, body);
	g_variant_unref(coupled_body);
	g_variant_unref(body);
	if (ret != NOTIFICATION_ERROR_NONE) {
		NOTIFICATION_ERR("failed to make notification handle from gvariant");
		return;
	}

	notification_call_event_handler_cb(noti, event_type);
}

static void _delete_event(GVariant *parameters)
{
	int priv_id;

	g_variant_get(parameters, "(i)", &priv_id);
	notification_delete_event_handler_cb(priv_id);
}

static void _handle_noti_event_handler_notify(GDBusConnection *connection,
		const gchar     *sender_name,
		const gchar     *object_path,
		const gchar     *interface_name,
		const gchar     *signal_name,
		GVariant        *parameters,
		gpointer         user_data)
{
	NOTIFICATION_DBG("own_name : %s signal_name: %s",
			 g_dbus_connection_get_unique_name(connection), signal_name);

	if (g_strcmp0(signal_name, "send_event") == 0)
		_send_event(parameters);
	else if (g_strcmp0(signal_name, "delete_noti") == 0)
		_delete_event(parameters);
}

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

		NOTIFICATION_DBG("subscribe id : %d", id);
		if (id == 0) {
			ret = NOTIFICATION_ERROR_IO_ERROR;
			NOTIFICATION_ERR("Failed to subscribe _dbus_event_handler_signal");
		} else {
			event_monitor_id = id;
		}
	}

	return ret;
}

static int _dbus_signal_init()
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

		NOTIFICATION_DBG("subscribe id : %d", id);
		if (id == 0) {
			/* LCOV_EXCL_START */
			ret = NOTIFICATION_ERROR_IO_ERROR;
			NOTIFICATION_ERR("Failed to _register_noti_dbus_interface");
			/* LCOV_EXCL_STOP */
		} else {
			monitor_id = id;
			ret = NOTIFICATION_ERROR_NONE;
		}
	}

	return ret;
}

static int _send_sync_noti(GVariant *body, GDBusMessage **reply, char *cmd)
{
	int ret = NOTIFICATION_ERROR_NONE;
	GError *err = NULL;
	GDBusMessage *msg;

	msg = g_dbus_message_new_method_call(
			PROVIDER_BUS_NAME,
			PROVIDER_OBJECT_PATH,
			PROVIDER_NOTI_INTERFACE_NAME,
			cmd);
	if (!msg) {
		/* LCOV_EXCL_START */
		NOTIFICATION_ERR("Can't allocate new method call");
		if (body)
			g_variant_unref(body);
		return NOTIFICATION_ERROR_OUT_OF_MEMORY;
		/* LCOV_EXCL_STOP */
	}

	if (body != NULL)
		g_dbus_message_set_body(msg, body);

	*reply = g_dbus_connection_send_message_with_reply_sync(
			_gdbus_conn,
			msg,
			G_DBUS_SEND_MESSAGE_FLAGS_NONE,
			-1,
			NULL,
			NULL,
			&err);

	g_object_unref(msg);

	if (!*reply) {
		/* LCOV_EXCL_START */
		ret = NOTIFICATION_ERROR_SERVICE_NOT_READY;
		if (err != NULL) {
			NOTIFICATION_ERR("No reply. cmd = %s,  error = %s", cmd, err->message);
			if (err->code == G_DBUS_ERROR_ACCESS_DENIED)
				ret = NOTIFICATION_ERROR_PERMISSION_DENIED;
			g_error_free(err);
		}
		return ret;
		/* LCOV_EXCL_STOP */
	}

	if (g_dbus_message_to_gerror(*reply, &err)) {
		if (err->code == G_DBUS_ERROR_ACCESS_DENIED)
			ret = NOTIFICATION_ERROR_PERMISSION_DENIED;
		else
			ret = err->code;

		NOTIFICATION_ERR("_send_sync_noti cmd = %s, error %s, err code %d", cmd, err->message, ret);
		g_error_free(err);
		return ret;
	}

	NOTIFICATION_DBG("_send_sync_noti done !!");
	return NOTIFICATION_ERROR_NONE;
}

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
		NOTIFICATION_ERR("Failed to get a callback item");
		return;
	}

	reply = g_dbus_connection_send_message_with_reply_finish(
			connection,
			res,
			&err);

	if (!reply) {
		/* LCOV_EXCL_START */
		if (err != NULL) {
			NOTIFICATION_ERR("No reply. error = %s", err->message);
			g_error_free(err);
		}
		result = NOTIFICATION_ERROR_SERVICE_NOT_READY;
		/* LCOV_EXCL_STOP */

	} else if (g_dbus_message_to_gerror(reply, &err)) {
		/* LCOV_EXCL_START */
		if (err->code == G_DBUS_ERROR_ACCESS_DENIED)
			result = NOTIFICATION_ERROR_PERMISSION_DENIED;
		else
			result = err->code;

		NOTIFICATION_ERR("_send_async_noti error %s", err->message);
		g_error_free(err);
		/* LCOV_EXCL_STOP */
	}

	NOTIFICATION_DBG("_send_async_noti done !![%d]", result);

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

static int _send_async_noti(GVariant *body, result_cb_item *cb_item, char *cmd)
{
	GDBusMessage *msg;

	msg = g_dbus_message_new_method_call(
			PROVIDER_BUS_NAME,
			PROVIDER_OBJECT_PATH,
			PROVIDER_NOTI_INTERFACE_NAME,
			cmd);
	if (!msg) {
		/* LCOV_EXCL_START */
		NOTIFICATION_ERR("Can't allocate new method call");
		return NOTIFICATION_ERROR_OUT_OF_MEMORY;
		/* LCOV_EXCL_STOP */
	}

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

	NOTIFICATION_DBG("_send_async_noti done !!");
	return NOTIFICATION_ERROR_NONE;
}

int notification_ipc_request_insert(notification_h noti, int *priv_id)
{
	int result;
	int id = NOTIFICATION_PRIV_ID_NONE;
	bool event_flag;
	GDBusMessage *reply = NULL;
	GVariant *body;
	GVariant *reply_body;

	result = _dbus_init();
	if (result != NOTIFICATION_ERROR_NONE) {
		NOTIFICATION_ERR("Can't init dbus %d", result);
		return result;
	}

	result = notification_get_event_flag(noti, &event_flag);
	if (result != NOTIFICATION_ERROR_NONE)
		return result;

	if (event_flag == true && event_monitor_id == 0) {
		result = _dbus_event_handler_signal_init();
		if (result != NOTIFICATION_ERROR_NONE)
			return result;
	}

	/* Initialize private ID */
	noti->group_id = NOTIFICATION_GROUP_ID_NONE;
	noti->internal_group_id = NOTIFICATION_GROUP_ID_NONE;

	_print_noti(noti);
	body = notification_ipc_make_gvariant_from_noti(noti, false);
	if (body == NULL) {
		/* LCOV_EXCL_START */
		NOTIFICATION_ERR("cannot make gvariant");
		return NOTIFICATION_ERROR_OUT_OF_MEMORY;
		/* LCOV_EXCL_STOP */
	}

	result = _send_sync_noti(body, &reply, "add_noti");
	NOTIFICATION_DBG("_send_sync_noti %d", result);

	if (result == NOTIFICATION_ERROR_NONE) {
		reply_body = g_dbus_message_get_body(reply);
		g_variant_get(reply_body, "(i)", &id);

		if (priv_id != NULL)
			*priv_id = id;
	}

	if (reply)
		g_object_unref(reply);

	NOTIFICATION_DBG("notification_ipc_request_insert done [priv_id : %d, result: %d]", id, result);
	return result;
}

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
		NOTIFICATION_ERR("Can't init dbus %d", result);
		return result;
		/* LCOV_EXCL_STOP */
	}

	body = notification_ipc_make_gvariant_from_noti(noti, false);
	if (body == NULL) {
		/* LCOV_EXCL_START */
		NOTIFICATION_ERR("cannot make gvariant");
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

	NOTIFICATION_DBG("notification_ipc_request_update done [result: %d, priv_id :%d]", result, priv_id);
	return result;
}

int notification_ipc_request_update_async(notification_h noti,
		void (*result_cb)(int priv_id, int result, void *data), void *user_data)
{
	int result;
	result_cb_item *cb_item;
	GVariant *body;

	result = _dbus_init();
	if (result != NOTIFICATION_ERROR_NONE) {
		/* LCOV_EXCL_START */
		NOTIFICATION_ERR("Can't init dbus %d", result);
		return result;
		/* LCOV_EXCL_STOP */
	}

	cb_item = calloc(1, sizeof(result_cb_item));
	if (cb_item == NULL)
		return NOTIFICATION_ERROR_OUT_OF_MEMORY;

	cb_item->result_cb = result_cb;
	cb_item->data = user_data;

	body = notification_ipc_make_gvariant_from_noti(noti, false);
	if (body == NULL) {
		/* LCOV_EXCL_START */
		NOTIFICATION_ERR("cannot make gvariant");
		free(cb_item);
		return NOTIFICATION_ERROR_OUT_OF_MEMORY;
		/* LCOV_EXCL_STOP */
	}

	result = _send_async_noti(body, cb_item, "update_noti");
	NOTIFICATION_DBG("notification_ipc_request_update_async done [result: %d]", result);

	if (result != NOTIFICATION_ERROR_NONE) {
		/* LCOV_EXCL_START */
		free(cb_item);
		cb_item = NULL;
		/* LCOV_EXCL_STOP */
	}

	g_variant_unref(body);

	return result;
}

int notification_ipc_request_refresh(uid_t uid)
{
	int result;
	GDBusMessage *reply = NULL;
	GVariant *body;

	result = _dbus_init();
	if (result != NOTIFICATION_ERROR_NONE) {
		NOTIFICATION_ERR("Can't init dbus %d", result);
		return result;
	}

	body = g_variant_new("(i)", uid);
	result = _send_sync_noti(body, &reply, "refresh_noti");

	if (reply)
		g_object_unref(reply);

	NOTIFICATION_ERR("notification_ipc_request_refresh done [result: %d]", result);
	return result;
}

int notification_ipc_request_delete_single(notification_type_e type, char *pkgname, int priv_id, uid_t uid)
{
	int result;
	int id;
	GDBusMessage *reply = NULL;
	GVariant *body;
	GVariant *reply_body;

	result = _dbus_init();
	if (result != NOTIFICATION_ERROR_NONE) {
		NOTIFICATION_ERR("Can't init dbus %d", result);
		return result;
	}

	if (!pkgname)
		pkgname = "";

	body = g_variant_new("(sii)", pkgname, priv_id, uid);
	result = _send_sync_noti(body, &reply, "del_noti_single");

	if (result == NOTIFICATION_ERROR_NONE) {
		reply_body = g_dbus_message_get_body(reply);
		g_variant_get(reply_body, "(i)", &id);
	}

	if (reply)
		g_object_unref(reply);

	NOTIFICATION_DBG("notification_ipc_request_delete_single done [result: %d]", result);
	return result;
}

int notification_ipc_request_delete_multiple(notification_type_e type, char *pkgname, uid_t uid)
{
	int result;
	int num_deleted;
	GVariant *body;
	GVariant *reply_body;
	GDBusMessage *reply = NULL;

	result = _dbus_init();
	if (result != NOTIFICATION_ERROR_NONE) {
		NOTIFICATION_ERR("Can't init dbus %d", result);
		return result;
	}

	if (!pkgname)
		pkgname = "";

	body = g_variant_new("(sii)", pkgname, type, uid);
	result = _send_sync_noti(body, &reply, "del_noti_multiple");

	if (result == NOTIFICATION_ERROR_NONE) {
		reply_body = g_dbus_message_get_body(reply);
		g_variant_get(reply_body, "(i)", &num_deleted);
		NOTIFICATION_ERR("num deleted:%d", num_deleted);
	}

	if (reply)
		g_object_unref(reply);

	return result;
}

int notification_ipc_request_load_noti_by_tag(notification_h noti, const char *pkgname, const char *tag, uid_t uid)
{
	int result;
	GDBusMessage *reply = NULL;
	GVariant *body;
	GVariant *reply_body;
	GVariant *noti_body;

	result = _dbus_init();
	if (result != NOTIFICATION_ERROR_NONE) {
		NOTIFICATION_ERR("Can't init dbus %d", result);
		return result;
	}

	if (!pkgname)
		pkgname = "";

	body = g_variant_new("(ssi)", pkgname, tag, uid);
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

	NOTIFICATION_DBG("notification_ipc_request_load_noti_by_tag done [result: %d]", result);
	return result;
}

/* LCOV_EXCL_START */
int notification_ipc_request_load_noti_by_priv_id(notification_h noti, const char *pkgname, int priv_id, uid_t uid)
{
	int result;
	GDBusMessage *reply = NULL;
	GVariant *body;
	GVariant *reply_body;
	GVariant *noti_body;

	result = _dbus_init();
	if (result != NOTIFICATION_ERROR_NONE) {
		NOTIFICATION_ERR("Can't init dbus %d", result);
		return result;
	}

	if (!pkgname)
		pkgname = "";

	body = g_variant_new("(sii)", pkgname, priv_id, uid);
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

	NOTIFICATION_DBG("notification_ipc_request_load_noti_by_priv_id done [result: %d]", result);
	return result;
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
int notification_ipc_request_get_count(notification_type_e type,
		    const char *pkgname, int group_id, int priv_id, int *count, uid_t uid)
{
	int result;
	GDBusMessage *reply = NULL;
	GVariant *body;
	GVariant *reply_body;
	int re_count;

	result = _dbus_init();
	if (result != NOTIFICATION_ERROR_NONE) {
		NOTIFICATION_ERR("Can't init dbus %d", result);
		return result;
	}

	if (!pkgname)
		pkgname = "";

	body = g_variant_new("(isiii)", type, pkgname, group_id, priv_id, uid);
	result = _send_sync_noti(body, &reply, "get_noti_count");

	if (result == NOTIFICATION_ERROR_NONE) {
		reply_body = g_dbus_message_get_body(reply);
		g_variant_get(reply_body, "(i)", &re_count);

		*count = re_count;
		NOTIFICATION_DBG("noti count [%d]", re_count);
	}

	if (reply)
		g_object_unref(reply);

	NOTIFICATION_DBG("notification_ipc_request_get_count done [result: %d]", result);
	return result;
}
/* LCOV_EXCL_STOP */

int notification_ipc_request_load_noti_grouping_list(notification_type_e type, int count,
		notification_list_h *list, uid_t uid)
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
		NOTIFICATION_ERR("Can't init dbus %d", result);
		return result;
	}

	body = g_variant_new("(iii)", type, count, uid);
	result = _send_sync_noti(body, &reply, "load_noti_grouping_list");

	if (result == NOTIFICATION_ERROR_NONE) {
		reply_body = g_dbus_message_get_body(reply);
		g_variant_get(reply_body, "(a(v))", &iter);

		while (g_variant_iter_loop(iter, "(v)", &iter_body)) {
			noti = notification_create(NOTIFICATION_TYPE_NOTI);
			if (!noti) {
				result = NOTIFICATION_ERROR_OUT_OF_MEMORY;
				NOTIFICATION_ERR("failed to create a notification");
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

	NOTIFICATION_DBG("notification_ipc_request_load_noti_grouping_list done [result: %d]", result);
	return result;
}

int notification_ipc_request_load_noti_detail_list(const char *pkgname,
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
		NOTIFICATION_ERR("Can't init dbus %d", result);
		return result;
	}

	body = g_variant_new("(siiii)", pkgname, group_id, priv_id, count, uid);
	result = _send_sync_noti(body, &reply, "load_noti_detail_list");

	if (result == NOTIFICATION_ERROR_NONE) {
		reply_body = g_dbus_message_get_body(reply);
		g_variant_get(reply_body, "(a(v))", &iter);

		while (g_variant_iter_loop(iter, "(v)", &iter_body)) {
			noti = notification_create(NOTIFICATION_TYPE_NOTI);
			if (!noti) {
				result = NOTIFICATION_ERROR_OUT_OF_MEMORY;
				NOTIFICATION_ERR("failed to create a notification");
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

	NOTIFICATION_DBG("notification_ipc_request_load_noti_detail_list done [result: %d]", result);
	return result;
}

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
		NOTIFICATION_ERR("Can't init dbus %d", result);
		return result;
	}

	result = _send_sync_noti(g_variant_new("(i)", uid), &reply, "get_setting_array");

	if (result == NOTIFICATION_ERROR_NONE) {
		reply_body = g_dbus_message_get_body(reply);
		g_variant_get(reply_body, "(ia(v))", &setting_cnt, &iter);

		NOTIFICATION_DBG("get setting arr cnt: %d", setting_cnt);
		result_setting_array = (struct notification_setting *)malloc(sizeof(struct notification_setting) * setting_cnt);
		if (result_setting_array == NULL) {
			NOTIFICATION_ERR("malloc failed");
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

	NOTIFICATION_DBG("notification_ipc_request_get_setting_array done [result: %d]", result);
	return result;
}

int notification_ipc_request_get_setting_by_appid(
		const char *appid, notification_setting_h *setting, uid_t uid)
{
	int result;
	GDBusMessage *reply = NULL;
	GVariant *body;
	GVariant *reply_body;
	GVariant *setting_body;
	notification_setting_h result_setting;

	result = _dbus_init();
	if (result != NOTIFICATION_ERROR_NONE) {
		NOTIFICATION_ERR("Can't init dbus %d", result);
		return result;
	}

	body = g_variant_new("(si)", appid, uid);
	result = _send_sync_noti(body, &reply, "get_setting_by_appid");

	if (result == NOTIFICATION_ERROR_NONE) {
		reply_body = g_dbus_message_get_body(reply);
		g_variant_get(reply_body, "(v)", &setting_body);

		result_setting = (struct notification_setting *)malloc(sizeof(struct notification_setting));
		if (result_setting == NULL) {
			NOTIFICATION_ERR("malloc failed");
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

	NOTIFICATION_DBG("notification_ipc_request_get_setting_by_package_name done [result: %d]", result);
	return result;
}

int notification_ipc_request_load_system_setting(notification_system_setting_h *setting, uid_t uid)
{
	int result;
	int count;
	int index = 0;
	GDBusMessage *reply = NULL;
	GVariant *setting_body;
	GVariant *reply_body;
	GVariant *iter_body;
	GVariantIter *iter;
	notification_system_setting_h result_setting;
	dnd_allow_exception_h dnd_allow_exception;
	dnd_allow_exception_h temp;

	result = _dbus_init();
	if (result != NOTIFICATION_ERROR_NONE) {
		NOTIFICATION_ERR("Can't init dbus %d", result);
		return result;
	}

	result = _send_sync_noti(g_variant_new("(i)", uid), &reply, "load_system_setting");
	if (result == NOTIFICATION_ERROR_NONE) {
		reply_body = g_dbus_message_get_body(reply);
		g_variant_get(reply_body, "(v)", &setting_body);

		result_setting = (struct notification_system_setting *)malloc(sizeof(struct notification_system_setting));
		if (result_setting == NULL) {
			NOTIFICATION_ERR("malloc failed");
			g_object_unref(reply);
			return NOTIFICATION_ERROR_OUT_OF_MEMORY;
		}

		notification_ipc_make_system_setting_from_gvariant(result_setting, setting_body);
		result_setting->dnd_allow_exceptions = NULL;

		result = _send_sync_noti(g_variant_new("(i)", uid), &reply, "load_dnd_allow_exception");
		if (result == NOTIFICATION_ERROR_NONE) {
			reply_body = g_dbus_message_get_body(reply);
			g_variant_get(reply_body, "(ia(v))", &count, &iter);

			dnd_allow_exception = (dnd_allow_exception_h)malloc(sizeof(struct notification_system_setting_dnd_allow_exception) * count);
			if (dnd_allow_exception == NULL) {
				g_object_unref(reply);
				g_variant_iter_free(iter);
				return NOTIFICATION_ERROR_OUT_OF_MEMORY;
			}

			while (g_variant_iter_loop(iter, "(v)", &iter_body)) {
				temp = dnd_allow_exception + index;

				notification_ipc_make_dnd_allow_exception_from_gvariant(temp, iter_body);
				result_setting->dnd_allow_exceptions = g_list_append(result_setting->dnd_allow_exceptions, temp);
				index++;
			}
		}

		*setting = result_setting;
		g_variant_unref(setting_body);
		g_variant_iter_free(iter);
	}

	if (reply)
		g_object_unref(reply);

	NOTIFICATION_DBG("notification_ipc_request_load_system_setting done [result: %d]", result);
	return result;
}

int notification_ipc_update_setting(notification_setting_h setting, uid_t uid)
{
	int result;
	GDBusMessage *reply = NULL;
	GVariant *body;

	result = _dbus_init();
	if (result != NOTIFICATION_ERROR_NONE) {
		NOTIFICATION_ERR("Can't init dbus %d", result);
		return result;
	}

	body = g_variant_new("(ssiiiiii)",
			setting->package_name,
			setting->appid,
			(int)(setting->allow_to_notify),
			(int)(setting->do_not_disturb_except),
			(int)(setting->visibility_class),
			(int)(setting->pop_up_notification),
			(int)(setting->lock_screen_content_level),
			uid);

	result = _send_sync_noti(body, &reply, "update_noti_setting");

	if (reply)
		g_object_unref(reply);

	NOTIFICATION_DBG("notification_ipc_update_setting done [result: %d]", result);
	return result;
}

int notification_ipc_update_system_setting(notification_system_setting_h system_setting, uid_t uid)
{
	int result;
	GDBusMessage *reply = NULL;
	GVariant *body;
	GList *list;
	dnd_allow_exception_h dnd_allow_exception;

	result = _dbus_init();
	if (result != NOTIFICATION_ERROR_NONE) {
		NOTIFICATION_ERR("Can't init dbus %d", result);
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

	NOTIFICATION_DBG("notification_ipc_update_system_setting done [result: %d]", result);
	return result;
}

int notification_ipc_request_save_as_template(notification_h noti, const char *template_name)
{
	int result;
	GDBusMessage *reply = NULL;
	GVariant *body;

	result = _dbus_init();
	if (result != NOTIFICATION_ERROR_NONE) {
		NOTIFICATION_ERR("Can't init dbus %d", result);
		return result;
	}

	body = notification_ipc_make_gvariant_from_noti(noti, false);
	if (body == NULL) {
		NOTIFICATION_ERR("cannot make gvariant");
		return NOTIFICATION_ERROR_OUT_OF_MEMORY;
	}

	result = _send_sync_noti(g_variant_new("(vs)", body, template_name), &reply, "save_as_template");

	if (reply)
		g_object_unref(reply);

	NOTIFICATION_DBG("notification_ipc_request_save_as_template [result: %d]", result);
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
		NOTIFICATION_ERR("Can't init dbus %d", result);
		return result;
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

	NOTIFICATION_DBG("notification_ipc_request_create_from_template done [result: %d]", result);

	return result;
}

int notification_ipc_request_create_from_package_template(notification_h noti, const char *pkgname, const char *template_name)
{
	int result;
	GDBusMessage *reply = NULL;
	GVariant *body;
	GVariant *reply_body;
	GVariant *noti_body;

	result = _dbus_init();
	if (result != NOTIFICATION_ERROR_NONE) {
		NOTIFICATION_ERR("Can't init dbus %d", result);
		return result;
	}

	body = g_variant_new("(ss)", pkgname, template_name);

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

	NOTIFICATION_DBG("notification_ipc_request_create_from_package_template [result: %d]", result);

	return result;
}

int notification_ipc_get_noti_block_state(const char *pkgname, int *do_not_disturb,
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

	if (pkgname == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	ret = _dbus_init();
	if (ret != NOTIFICATION_ERROR_NONE) {
		NOTIFICATION_ERR("Can't init dbus %d", ret);
		return ret;
	}

	body = g_variant_new("(si)", pkgname, uid);

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

	NOTIFICATION_DBG("notification_ipc_get_noti_block_state done");

	return ret;
}

int notification_ipc_send_event(notification_h noti, int event_type)
{
	int ret;
	GVariant *body = NULL;
	GDBusMessage *reply = NULL;

	ret = _dbus_init();
	if (ret != NOTIFICATION_ERROR_NONE) {
		NOTIFICATION_ERR("Can't init dbus %d", ret);
		return ret;
	}

	body = notification_ipc_make_gvariant_from_noti(noti, false);
	if (body == NULL) {
		NOTIFICATION_ERR("Can't make gvariant to noti");
		return NOTIFICATION_ERROR_OUT_OF_MEMORY;
	}

	ret = _send_sync_noti(g_variant_new("(vi)", body, event_type), &reply, "send_noti_event");

	if (reply)
		g_object_unref(reply);

	return ret;
}

EXPORT_API GVariant *notification_ipc_make_gvariant_from_noti(notification_h noti, bool translate)
{
	NOTIFICATION_DBG("make gvariant from noti");
	int i = 0;
	int b_encode_len = 0;
	bundle_raw *args = NULL;
	bundle_raw *group_args = NULL;
	bundle_raw *b_image_path = NULL;
	bundle_raw *b_execute_option = NULL;
	bundle_raw *b_service_responding = NULL;
	bundle_raw *b_service_single_launch = NULL;
	bundle_raw *b_service_multi_launch = NULL;
	bundle_raw *b_event_handler[NOTIFICATION_EVENT_TYPE_MAX] = {NULL, };
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
	g_variant_builder_add(&builder, "{iv}", NOTIFICATION_DATA_TYPE_CALLER_PKGNAME, g_variant_new_string((const gchar *)noti->caller_pkgname));
	g_variant_builder_add(&builder, "{iv}", NOTIFICATION_DATA_TYPE_DISPLAY_APPLIST, g_variant_new_int32(noti->display_applist));

	if (noti->args) {
		bundle_encode(noti->args, (bundle_raw **)&args, NULL);
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

	for (i = 0; i < NOTIFICATION_EVENT_TYPE_MAX; i++) {
		if (noti->b_event_handler[i]) {
			bundle_encode(noti->b_event_handler[i],
					(bundle_raw **)&b_event_handler[i], &b_encode_len);
			g_variant_builder_add(&builder, "{iv}", NOTIFICATION_DATA_TYPE_BUTTON1_EVENT + i, g_variant_new_string((const gchar *)b_event_handler[i]));

			if (b_event_handler[i])
				bundle_free_encoded_rawdata(&b_event_handler[i]);
		}
	}

	if (noti->launch_pkgname)
		g_variant_builder_add(&builder, "{iv}", NOTIFICATION_DATA_TYPE_LAUNCH_PKGNAME, g_variant_new_string((const gchar *)noti->launch_pkgname));

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

	if (noti->sound_type != NOTIFICATION_SOUND_TYPE_NONE)
		g_variant_builder_add(&builder, "{iv}", NOTIFICATION_DATA_TYPE_SOUND_TYPE, g_variant_new_int32(noti->sound_type));

	if (noti->sound_path)
		g_variant_builder_add(&builder, "{iv}", NOTIFICATION_DATA_TYPE_SOUND_PATH, g_variant_new_string((const gchar *)noti->sound_path));

	g_variant_builder_add(&builder, "{iv}", NOTIFICATION_DATA_TYPE_VIBRATION_TYPE, g_variant_new_int32(noti->vibration_type));

	if (noti->vibration_path)
		g_variant_builder_add(&builder, "{iv}", NOTIFICATION_DATA_TYPE_VIBRATION_PATH, g_variant_new_string((const gchar *)noti->vibration_path));

	if (noti->led_operation != NOTIFICATION_LED_OP_OFF)
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
	if (noti->app_name != NULL)
		g_variant_builder_add(&builder, "{iv}", NOTIFICATION_DATA_TYPE_APP_NAME, g_variant_new_string((const gchar *)noti->app_name));
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
	g_variant_builder_add(&builder, "{iv}", NOTIFICATION_DATA_TYPE_TIMEOUT, g_variant_new_int32(noti->timeout));
	g_variant_builder_add(&builder, "{iv}", NOTIFICATION_DATA_TYPE_TEXT_INPUT_MAX_LENGTH, g_variant_new_int32(noti->text_input_max_length));
	g_variant_builder_add(&builder, "{iv}", NOTIFICATION_DATA_TYPE_EVENT_FLAG, g_variant_new_int32(noti->event_flag));
	g_variant_builder_add(&builder, "{iv}", NOTIFICATION_DATA_TYPE_TRANSLATION, g_variant_new_int32(noti->is_translation));
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
			g_hash_table_unref(*dict);
			return FALSE;
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

	NOTIFICATION_DBG("make noti from GVariant");
	GHashTable *dict;

	int i;
	char *caller_pkgname = NULL;
	char *launch_pkgname = NULL;
	bundle_raw *args = NULL;
	bundle_raw *group_args = NULL;
	bundle_raw *b_execute_option = NULL;
	bundle_raw *b_service_responding = NULL;
	bundle_raw *b_service_single_launch = NULL;
	bundle_raw *b_service_multi_launch = NULL;
	bundle_raw *b_event_handler[NOTIFICATION_EVENT_TYPE_MAX] = { NULL, };
	char *domain = NULL;
	char *dir = NULL;
	bundle_raw *b_text = NULL;
	bundle_raw *b_key = NULL;
	bundle_raw *b_format_args = NULL;
	bundle_raw *b_image_path = NULL;
	char *sound_path = NULL;
	char *vibration_path = NULL;
	char *app_icon_path = NULL;
	char *app_name = NULL;
	char *temp_title = NULL;
	char *temp_content = NULL;
	char *tag = NULL;

	if (noti == NULL) {
		NOTIFICATION_ERR("invalid data noti NULL");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	if (variant == NULL) {
		NOTIFICATION_ERR("invalid data variant NULL");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	if (!_variant_to_int_dict(&dict, variant))
		return NOTIFICATION_ERROR_OUT_OF_MEMORY;

	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_NOTI_TYPE, "i", &noti->type);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_LAYOUT, "i", &noti->layout);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_GROUP_ID, "i", &noti->group_id);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_INTERNAL_GROUP_ID, "i", &noti->internal_group_id);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_PRIV_ID, "i", &noti->priv_id);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_CALLER_PKGNAME, "&s", &caller_pkgname);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_LAUNCH_PKGNAME, "&s", &launch_pkgname);
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
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_DOMAIN, "&s", &domain);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_DIR, "&s", &dir);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_TEXT, "&s", &b_text);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_KEY, "&s", &b_key);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_FORMAT_ARGS, "&s", &b_format_args);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_NUM_FORMAT_ARGS, "i", &noti->num_format_args);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_IMAGE_PATH, "&s", &b_image_path);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_SOUND_TYPE, "i", &noti->sound_type);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_SOUND_PATH, "&s", &sound_path);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_VIBRATION_TYPE, "i", &noti->vibration_type);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_VIBRATION_PATH, "&s", &vibration_path);
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
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_APP_NAME, "&s", &app_name);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_TEMP_TITLE, "&s", &temp_title);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_TEMP_CONTENT, "&s", &temp_content);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_TAG, "&s", &tag);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_ONGOING_FLAG, "i", &noti->ongoing_flag);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_ONGOING_VALUE_TYPE, "i", &noti->ongoing_value_type);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_ONGOING_CURRENT, "i", &noti->ongoing_current);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_ONGOING_DURATION, "i", &noti->ongoing_duration);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_AUTO_REMOVE, "i", &noti->auto_remove);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_DEFAULT_BUTTON, "i", &noti->default_button_index);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_TIMEOUT, "i", &noti->timeout);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_TEXT_INPUT_MAX_LENGTH, "i", &noti->text_input_max_length);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_EVENT_FLAG, "i", &noti->event_flag);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_TRANSLATION, "i", &noti->is_translation);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_UID, "i", &noti->uid);

	noti->caller_pkgname = _dup_string(caller_pkgname);
	noti->launch_pkgname = _dup_string(launch_pkgname);
	noti->args = _create_bundle_from_bundle_raw(args);
	noti->group_args = _create_bundle_from_bundle_raw(group_args);
	noti->b_execute_option = _create_bundle_from_bundle_raw(b_execute_option);
	noti->b_service_responding = _create_bundle_from_bundle_raw(
			b_service_responding);
	noti->b_service_single_launch = _create_bundle_from_bundle_raw(
			b_service_single_launch);
	noti->b_service_multi_launch = _create_bundle_from_bundle_raw(
			b_service_multi_launch);
	for (i = 0; i < NOTIFICATION_EVENT_TYPE_MAX; i++) {
		noti->b_event_handler[i] = _create_bundle_from_bundle_raw(
				b_event_handler[i]);
	}
	noti->domain = _dup_string(domain);
	noti->dir = _dup_string(dir);
	noti->b_text = _create_bundle_from_bundle_raw(b_text);
	noti->b_key = _create_bundle_from_bundle_raw(b_key);
	noti->b_format_args = _create_bundle_from_bundle_raw(b_format_args);
	noti->b_image_path = _create_bundle_from_bundle_raw(b_image_path);
	noti->sound_path = _dup_string(sound_path);
	noti->vibration_path = _dup_string(vibration_path);
	noti->app_icon_path = _dup_string(app_icon_path);
	noti->app_name = _dup_string(app_name);
	noti->temp_title = _dup_string(temp_title);
	noti->temp_content = _dup_string(temp_content);
	noti->tag = _dup_string(tag);

	g_hash_table_unref(dict);

	return NOTIFICATION_ERROR_NONE;
}

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
		NOTIFICATION_ERR("invalid data");
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

	NOTIFICATION_DBG("system setting  #### %d, %d, %d, %d, [%d:%d] [%d:%d], %d",
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

	body = g_variant_new("(ssiiiii)",
			noti_setting->package_name,
			noti_setting->appid,
			noti_setting->allow_to_notify,
			noti_setting->do_not_disturb_except,
			noti_setting->visibility_class,
			noti_setting->pop_up_notification,
			noti_setting->lock_screen_content_level);

	return body;
}

EXPORT_API int notification_ipc_make_setting_from_gvariant(struct notification_setting *noti_setting,
		GVariant *variant)
{
	NOTIFICATION_DBG("notification_ipc_make_setting_from_gvariant !!!!");
	char *pkgname;
	char *appid;
	int allow_to_notify;
	int do_not_disturb_except;
	int visibility_class;
	int pop_up_notification;
	int lock_screen_content_level;

	if (noti_setting == NULL || variant == NULL) {
		NOTIFICATION_ERR("invalid data");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}
	g_variant_get(variant,
			"(&s&siiiii)",
			&pkgname,
			&appid,
			&allow_to_notify,
			&do_not_disturb_except,
			&visibility_class,
			&pop_up_notification,
			&lock_screen_content_level);

	NOTIFICATION_DBG("setting from variant %s !!", pkgname);

	noti_setting->package_name = _dup_string(pkgname);
	noti_setting->appid = _dup_string(appid);
	noti_setting->allow_to_notify = allow_to_notify;
	noti_setting->do_not_disturb_except = do_not_disturb_except;
	noti_setting->visibility_class = visibility_class;
	noti_setting->pop_up_notification = pop_up_notification;
	noti_setting->lock_screen_content_level = lock_screen_content_level;

	NOTIFICATION_DBG("setting from variant %s, %s",
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
		NOTIFICATION_ERR("invalid data");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	g_variant_get(variant, "(ii)", &type, &value);

	dnd_allow_exception->type = type;
	dnd_allow_exception->value = value;

	return NOTIFICATION_ERROR_NONE;
}

static int _send_service_register(uid_t uid)
{
	GDBusMessage *reply = NULL;
	int result;
	notification_op *noti_op = NULL;

	result = _send_sync_noti(g_variant_new("(i)", uid), &reply, "noti_service_register");

	if (reply)
		g_object_unref(reply);

	NOTIFICATION_DBG("_send_service_register done = %s, result = %d", _bus_name, result);
	noti_op = _ipc_create_op(NOTIFICATION_OP_SERVICE_READY, 1, NULL, 1, NULL);
	if (noti_op != NULL) {
		notification_call_changed_cb_for_uid(noti_op, 1, uid);
		free(noti_op);
	}

	return result;
}

static int _ipc_monitor_register(uid_t uid)
{
	return  _send_service_register(uid);
}

/* LCOV_EXCL_START */
static void _on_name_appeared(GDBusConnection *connection,
		const gchar     *name,
		const gchar     *name_owner,
		gpointer         user_data)
{
	int uid = GPOINTER_TO_INT(user_data);

	NOTIFICATION_DBG("name appeared [%d] : %s", uid, name);
	is_master_started = 1;
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

	NOTIFICATION_DBG("name vanished [%d] : %s", uid, name);
	is_master_started = 0;
}
/* LCOV_EXCL_STOP */

int notification_ipc_monitor_init(uid_t uid)
{
	int ret;

	ret = _dbus_init();
	if (ret != NOTIFICATION_ERROR_NONE) {
		/* LCOV_EXCL_START */
		NOTIFICATION_ERR("Can't init dbus %d", ret);
		return ret;
		/* LCOV_EXCL_STOP */
	}

	ret = _dbus_signal_init();
	if (ret != NOTIFICATION_ERROR_NONE) {
		/* LCOV_EXCL_START */
		NOTIFICATION_ERR("Can't signal_init %d", ret);
		return ret;
		/* LCOV_EXCL_STOP */
	}

	ret = _ipc_monitor_register(uid);
	if (ret != NOTIFICATION_ERROR_NONE) {
		/* LCOV_EXCL_START */
		NOTIFICATION_ERR("Can't init ipc_monitor_register %d", ret);
		return ret;
		/* LCOV_EXCL_STOP */
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
			/* LCOV_EXCL_START */
			g_dbus_connection_signal_unsubscribe(_gdbus_conn, monitor_id);
			monitor_id = 0;
			NOTIFICATION_ERR("watch on name fail");
			return NOTIFICATION_ERROR_IO_ERROR;
			/* LCOV_EXCL_STOP */
		}
	}

	return NOTIFICATION_ERROR_NONE;
}

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

void notification_ipc_event_monitor_fini(void)
{
	if (event_monitor_id == 0)
		return;

	g_dbus_connection_signal_unsubscribe(_gdbus_conn, event_monitor_id);
	event_monitor_id = 0;
}
