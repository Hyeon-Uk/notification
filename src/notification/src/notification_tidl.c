/*
 * Copyright (c) 2025 Samsung Electronics Co., Ltd. All rights reserved.
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

#include <aul.h>
#include <unistd.h>
#include <sys/types.h>

#include <notification.h>
#include <notification_debug.h>
#include <notification_type.h>
#include <notification_private.h>
#include <notification_internal.h>
#include <notification_tidl_proxy.h>
#include <notification_setting.h>
#include <notification_setting_internal.h>

#include <notification_tidl.h>

/*
 * for TIDL
 */
#define NOTIFICATION_CTOR __attribute__((constructor))
#define NOTIFICATION_DTOR __attribute__((destructor))

static rpc_port_proxy_noti_service_h service_handle;
static rpc_port_proxy_noti_service_event_received_h event_handle;
static rpc_port_proxy_noti_service_changed_h changed_handle;

static bool _connected = false;
static GRecMutex __rec_mutex;

typedef struct _channel_foreach_cb_info {
	notification_channel_foreach_cb cb;
	void *user_data;
} channel_foreach_cb_info;

NOTIFICATION_CTOR static void __notification_init(void)
{
	g_rec_mutex_init(&__rec_mutex);
}

NOTIFICATION_DTOR static void __notification_fini(void)
{
	g_rec_mutex_clear(&__rec_mutex);
}

static void __notification_mutex_lock(void)
{
	g_rec_mutex_lock(&__rec_mutex);
}

static void __notification_mutex_unlock(void)
{
	g_rec_mutex_unlock(&__rec_mutex);
}

static void _rpc_port_proxy_noti_service_connected_cb (rpc_port_proxy_noti_service_h h, void *user_data)
{
	WARN("_rpc_port_proxy_noti_service_connected_cb");
	_connected = true;
}

static void _rpc_port_proxy_noti_service_disconnected_cb (rpc_port_proxy_noti_service_h h, void *user_data)
{
	WARN("_rpc_port_proxy_noti_service_disconnected_cb");
	_connected = false;
}

static void _rpc_port_proxy_noti_service_rejected_cb (rpc_port_proxy_noti_service_h h, void *user_data)
{
	WARN("_rpc_port_proxy_noti_service_rejected_cb");
	_connected = false;
}

static void _send_event(rpc_port_proxy_notification_h notihandle, int event_type)
{
	int ret;
	notification_h noti;

	WARN("send event : %d", event_type);

	ret = make_noti_from_notification(&noti, notihandle);
	if (ret != NOTIFICATION_ERROR_NONE) {
		ERR("failed to make noti : %d", ret);
		return;
	}
	notification_call_event_handler_cb(noti, event_type);

	notification_free(noti);
}

static void _delete_event(rpc_port_proxy_notification_h notihandle)
{
	int ret;
	int priv_id;

	ret = rpc_port_proxy_notification_get_priv_id(notihandle, &priv_id);
	if (ret != RPC_PORT_ERROR_NONE) {
		ERR("Failed to get priv id : %d", ret);
		return;
	}
	WARN("delete event id : %d", priv_id);

	notification_delete_event_handler_cb(priv_id);
}

/* LCOV_EXCL_START */
static void _disturb_app()
{
	notification_call_disturb_cb();
}
/* LCOV_EXCL_STOP */

// event callback
static void _rpc_port_proxy_noti_service_event_received_cb (void *user_data,
		rpc_port_proxy_notification_h notihandle, const char *command, int event_type)
{
	WARN("event received command : %s, type : %d", command, event_type);

	if (strncmp(command, "send_event", strlen("send_event")) == 0)
		_send_event(notihandle, event_type);
	else if (strncmp(command, "delete_noti", strlen("delete_noti")) == 0)
		_delete_event(notihandle);
	else if (strncmp(command, "disturb_app", strlen("disturb_app")) == 0)
		_disturb_app();
}

/*
 * functions to create operation list
 */
static notification_op *_tidl_create_op(notification_op_type_e type,
		int num_op, int *list_priv_id, notification_h *noti_list)
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

static void _add_noti_notify(rpc_port_proxy_notification_h notihandle,
		rpc_port_proxy_array_int_h extra)
{
	int ret;
	notification_op *noti_op = NULL;
	notification_h noti;
	int flag;
	int priv_id;
	int uid;

	WARN("_add_noti_notify");

	ret = rpc_port_proxy_notification_get_flags_for_property(notihandle, &flag);
	if (ret != RPC_PORT_ERROR_NONE) {
		ERR("failed to get flag changed callback[%d]", flag);
		return;
	}

	ret = rpc_port_proxy_notification_get_priv_id(notihandle, &priv_id);
	if (ret != RPC_PORT_ERROR_NONE) {
		ERR("failed to get privid : %d", ret);
		return;
	}

	ret = make_noti_from_notification(&noti, notihandle);
	if (ret != NOTIFICATION_ERROR_NONE) {
		ERR("failed to make noti : %d", ret);
		return;
	}

	if (flag & NOTIFICATION_PROP_DISABLE_UPDATE_ON_INSERT) {
		ERR("Disable changed callback[%d]", flag);
		/* Disable changed cb */
	} else {
		/* Enable changed cb */
		noti_op = _tidl_create_op(NOTIFICATION_OP_INSERT, 1, &priv_id, &noti);
		ret = rpc_port_proxy_notification_get_uid(notihandle, &uid);
		if (noti_op != NULL && ret == RPC_PORT_ERROR_NONE) {
			WARN("call changed cb, uid : %d", uid);
			notification_call_changed_cb_for_uid(noti_op, 1, (uid_t)uid);
		} else {
			ERR("failed call changed cb : %d", ret);
		}
	}
	WARN("_add_noti_notify done");

	notification_free(noti);

	if (noti_op)
		free(noti_op);
}

static void _update_noti_notify(rpc_port_proxy_notification_h notihandle,
		rpc_port_proxy_array_int_h extra)
{
	int ret;
	notification_op *noti_op = NULL;
	notification_h noti;
	int priv_id;
	int uid;

	WARN("_update_noti_notify");

	ret = rpc_port_proxy_notification_get_priv_id(notihandle, &priv_id);
	if (ret != RPC_PORT_ERROR_NONE) {
		ERR("failed to get privid : %d", ret);
		return;
	}

	ret = make_noti_from_notification(&noti, notihandle);
	if (ret != NOTIFICATION_ERROR_NONE) {
		ERR("failed to make noti : %d", ret);
		return;
	}

	noti_op = _tidl_create_op(NOTIFICATION_OP_UPDATE, 1, &priv_id, &noti);
	ret = rpc_port_proxy_notification_get_uid(notihandle, &uid);
	if (noti_op != NULL && ret == RPC_PORT_ERROR_NONE) {
		WARN("call changed cb, uid : %d", uid);
		notification_call_changed_cb_for_uid(noti_op, 1, (uid_t)uid);
	} else {
		ERR("failed call changed cb : %d", ret);
	}
	WARN("_update_noti_notify done");

	notification_free(noti);

	if (noti_op)
		free(noti_op);
}

static void _delete_single_notify(rpc_port_proxy_notification_h notihandle,
		rpc_port_proxy_array_int_h extra)
{
	int ret;
	int uid;
	int priv_id;
	char *app_id;
	notification_op *noti_op;

	WARN("_delete_single_notify");

	ret = rpc_port_proxy_notification_get_priv_id(notihandle, &priv_id);
	if (ret != RPC_PORT_ERROR_NONE) {
		ERR("failed to get privid : %d", ret);
		return;
	}

	ret = rpc_port_proxy_notification_get_uid(notihandle, &uid);
	if (ret != RPC_PORT_ERROR_NONE) {
		ERR("failed to get uid : %d", ret);
		return;
	}

	ret = rpc_port_proxy_notification_get_caller_app_id(notihandle, &app_id);
	if (ret != RPC_PORT_ERROR_NONE) {
		ERR("failed to get appid : %d", ret);
		return;
	}

	noti_op = _tidl_create_op(NOTIFICATION_OP_DELETE, 1, &priv_id, NULL);
	if (noti_op != NULL) {
		notification_call_changed_cb_for_uid(noti_op, 1, (uid_t)uid);
		free(noti_op);
	}
	WARN("delete single noti event done, appid [%s], priv id[%d], uid[%d]",
			app_id, priv_id, uid);

	free(app_id);
}

static void _delete_multiple_notify(rpc_port_proxy_notification_h notihandle,
		rpc_port_proxy_array_int_h extra)
{
	int ret;
	int uid;
	int *id_list;
	int cnt;
	notification_op *noti_op;

	WARN("_delete_multiple_notify");

	ret = rpc_port_proxy_notification_get_uid(notihandle, &uid);
	if (ret != RPC_PORT_ERROR_NONE) {
		ERR("failed to get uid : %d", ret);
		return;
	}

	ret = rpc_port_proxy_array_int_get(extra, &id_list, &cnt);
	if (ret != RPC_PORT_ERROR_NONE) {
		ERR("failed to get array int : %d", ret);
		return;
	}

	noti_op = _tidl_create_op(NOTIFICATION_OP_DELETE, cnt, id_list, NULL);
	if (noti_op != NULL) {
		WARN("delete multiple call changed");
		notification_call_changed_cb_for_uid(noti_op, cnt, (uid_t)uid);
		free(noti_op);
	} else {
		WARN("delete multiple op null");
	}
	free(id_list);
	WARN("delete multiple done : %d", cnt);
}

static void _refresh_noti_notify(rpc_port_proxy_notification_h notihandle,
		rpc_port_proxy_array_int_h extra)
{
	int ret;
	int uid;
	notification_op *noti_op;

	WARN("_refresh_noti_notify");

	ret = rpc_port_proxy_notification_get_uid(notihandle, &uid);
	if (ret != RPC_PORT_ERROR_NONE) {
		ERR("failed to get uid : %d", ret);
		return;
	}

	noti_op = _tidl_create_op(NOTIFICATION_OP_REFRESH, 1, NULL, NULL);
	if (noti_op != NULL) {
		WARN("call changed cb");
		notification_call_changed_cb_for_uid(noti_op, 1, (uid_t)uid);
		free(noti_op);
	}
	WARN("refresh noti done");
}

static void _change_dnd_notify(rpc_port_proxy_notification_h notihandle,
		rpc_port_proxy_array_int_h extra)
{
	int ret;
	int *do_not_disturb;
	int size;
	int uid;

	WARN("_change_dnd_notify");

	ret = rpc_port_proxy_notification_get_uid(notihandle, &uid);
	if (ret != RPC_PORT_ERROR_NONE) {
		ERR("failed to get privid : %d", ret);
		return;
	}

	ret = rpc_port_proxy_array_int_get(extra, &do_not_disturb, &size);
	if (ret != RPC_PORT_ERROR_NONE) {
		ERR("failed to get privid : %d", ret);
		return;
	}

	// get from extra
	notification_call_dnd_changed_cb_for_uid(*do_not_disturb, (uid_t)uid);

	free(do_not_disturb);
	WARN("_change_dnd_notify done");
}

static void _delete_by_display_applist_notify(rpc_port_proxy_notification_h notihandle,
		rpc_port_proxy_array_int_h extra)
{
	int ret;
	int uid;
	int *id_list;
	int cnt;
	notification_op *noti_op;

	WARN("_delete_by_display_applist_notify");

	ret = rpc_port_proxy_notification_get_uid(notihandle, &uid);
	if (ret != RPC_PORT_ERROR_NONE) {
		ERR("failed to get uid : %d", ret);
		return;
	}

	ret = rpc_port_proxy_array_int_get(extra, &id_list, &cnt);
	if (ret != RPC_PORT_ERROR_NONE) {
		ERR("failed to get array int : %d", ret);
		return;
	}
	WARN("delete by display applist uid %d, cnt : %d", uid, cnt);

	noti_op = _tidl_create_op(NOTIFICATION_OP_DELETE, cnt, id_list, NULL);
	if (noti_op != NULL) {
		WARN("delete by display applist call changed");
		notification_call_changed_cb_for_uid(noti_op, cnt, (uid_t)uid);
		free(noti_op);
	} else {
		WARN("delete by display applist op null");
	}

	free(id_list);
	WARN("delete by display applist done");
}

// changed callback
static void _rpc_port_proxy_noti_service_changed_cb (void *user_data,
		rpc_port_proxy_notification_h notihandle, const char *command,
		rpc_port_proxy_array_int_h extra)
{
	WARN("service changed command : %s", command);

	if (strncmp(command, "add_noti_notify", strlen("add_noti_notify")) == 0)
		_add_noti_notify(notihandle, extra);
	else if (strncmp(command, "update_noti_notify", strlen("update_noti_notify")) == 0)
		_update_noti_notify(notihandle, extra);
	else if (strncmp(command, "delete_single_notify", strlen("delete_single_notify")) == 0)
		_delete_single_notify(notihandle, extra);
	else if (strncmp(command, "delete_multiple_notify", strlen("delete_multiple_notify")) == 0)
		_delete_multiple_notify(notihandle, extra);
	else if (strncmp(command, "refresh_noti_notify", strlen("refresh_noti_notify")) == 0)
		_refresh_noti_notify(notihandle, extra);
	else if (strncmp(command, "change_dnd_notify", strlen("change_dnd_notify")) == 0)
		_change_dnd_notify(notihandle, extra);
	else if (strncmp(command, "delete_by_display_applist_notify", strlen("delete_by_display_applist_notify")) == 0)
		_delete_by_display_applist_notify(notihandle, extra);
}

static int _tidl_init()
{
	WARN("tidl init");
	int ret;
	rpc_port_proxy_noti_service_callback_s callbacks = {0,};

	__notification_mutex_lock();

	if (service_handle == NULL) {
		callbacks.connected = _rpc_port_proxy_noti_service_connected_cb;
		callbacks.disconnected = _rpc_port_proxy_noti_service_disconnected_cb;
		callbacks.rejected = _rpc_port_proxy_noti_service_rejected_cb;

		ret = rpc_port_proxy_noti_service_create("d::org.tizen.appfw.service.esd", &callbacks, NULL, &service_handle);
		if (ret != RPC_PORT_ERROR_NONE) {
			WARN("service create fail");
			__notification_mutex_unlock();
			return ret;
		}
		WARN("service create");
	} else {
		WARN("service already created");
	}

	if (!_connected) {
		ret = rpc_port_proxy_noti_service_connect_sync(service_handle);
		if (ret != RPC_PORT_ERROR_NONE) {
			WARN("service connect fail");
			__notification_mutex_unlock();
			return ret;
		}
		WARN("service connect");
	} else {
		WARN("service already connected");
	}

	__notification_mutex_unlock();

	return NOTIFICATION_ERROR_NONE;
}

int notification_tidl_is_master_ready(void)
{
	WARN("notification_tidl_is_master_ready");
	return _tidl_init();
}

// for event - send event
int notification_tidl_event_monitor_init(void)
{
	int ret;

	ret = _tidl_init();
	if (ret != NOTIFICATION_ERROR_NONE) {
		WARN("tidl init fail");
		return ret;
	}
	WARN("event tidl init done");

	if (event_handle == NULL) {
		ret = rpc_port_proxy_noti_service_event_received_create(&event_handle);
		if (ret != RPC_PORT_ERROR_NONE) {
			WARN("create event handle fail");
			return ret;
		}
		WARN("create event handle");

		ret = rpc_port_proxy_noti_service_event_received_set_callback(event_handle,
				_rpc_port_proxy_noti_service_event_received_cb, NULL);
		if (ret != RPC_PORT_ERROR_NONE) {
			WARN("set event callback fail");
			rpc_port_proxy_noti_service_event_received_destroy(event_handle);
			event_handle = NULL;
			return ret;
		}
		WARN("set event callback");

		ret = rpc_port_proxy_noti_service_invoke_register_event(service_handle, event_handle);
		if (ret != RPC_PORT_ERROR_NONE) {
			WARN("change callback register fail");
			rpc_port_proxy_noti_service_event_received_destroy(event_handle);
			event_handle = NULL;
			return ret;
		}
		WARN("invoke register event");
	} else {
		WARN("event handle already created");
	}
	WARN("change event register done");

	return NOTIFICATION_ERROR_NONE;
}

void notification_tidl_event_monitor_fini(void)
{
	int ret;

	ret = _tidl_init();
	if (ret != NOTIFICATION_ERROR_NONE) {
		WARN("tidl init fail");
		return;
	}
	WARN("event fini");

	ret = rpc_port_proxy_noti_service_invoke_unregister_event(service_handle);
	if (ret != RPC_PORT_ERROR_NONE) {
		ERR("Failed to unregister event");
		return;
	}

	if (event_handle) {
		rpc_port_proxy_noti_service_event_received_dispose(service_handle, event_handle);
		event_handle = NULL;
	}

	WARN("event fini done");
}

// for changed callback
int notification_tidl_monitor_init(void)
{
	int ret;

	ret = _tidl_init();
	if (ret != NOTIFICATION_ERROR_NONE) {
		WARN("tidl init fail");
		return ret;
	}
	WARN("monitor tidl init done");

	if (changed_handle == NULL) {
		ret = rpc_port_proxy_noti_service_changed_create(&changed_handle);
		if (ret != RPC_PORT_ERROR_NONE) {
			WARN("create change handle fail");
			return ret;
		}
		WARN("create changed handle");

		ret = rpc_port_proxy_noti_service_changed_set_callback(changed_handle,
				_rpc_port_proxy_noti_service_changed_cb, NULL);
		if (ret != RPC_PORT_ERROR_NONE) {
			WARN("set change callback fail");
			rpc_port_proxy_noti_service_changed_destroy(changed_handle);
			changed_handle = NULL;
			return ret;
		}
		WARN("set changed callback");

		ret = rpc_port_proxy_noti_service_invoke_register_changed(service_handle, changed_handle, aul_getuid());
		if (ret != RPC_PORT_ERROR_NONE) {
			WARN("change callback register fail");
			rpc_port_proxy_noti_service_changed_destroy(changed_handle);
			changed_handle = NULL;
			return ret;
		}

		ret = rpc_port_proxy_noti_service_invoke_register_service(service_handle, aul_getuid());
		if (ret != RPC_PORT_ERROR_NONE) {
			WARN("register register fail");
			return ret;
		}
		WARN("invoke register changed");
	} else {
		WARN("changed callback already created");
	}
	WARN("changed callback register done");

	return NOTIFICATION_ERROR_NONE;
}

int notification_tidl_monitor_fini(void)
{
	int ret;

	ret = _tidl_init();
	if (ret != NOTIFICATION_ERROR_NONE) {
		WARN("tidl init fail");
		return ret;
	}
	WARN("monitor fini");

	ret = rpc_port_proxy_noti_service_invoke_unregister_changed(service_handle);
	if (ret != RPC_PORT_ERROR_NONE) {
		ERR("Failed to unregister chnaged");
		return ret;
	}

	if (changed_handle) {
		rpc_port_proxy_noti_service_changed_dispose(service_handle, changed_handle);
		changed_handle = NULL;
	}

	WARN("monitor fini done");

	return NOTIFICATION_ERROR_NONE;
}

int notification_tidl_request_insert(notification_h noti, int *priv_id)
{
	int ret;
	int _priv_id;
	rpc_port_proxy_notification_h notihandle;

	WARN("request insert");

	ret = _tidl_init();
	if (ret != NOTIFICATION_ERROR_NONE)	{
		ERR("failed to init tidl : %d", ret);
		return ret;
	}

	/* Initialize private ID */
	noti->group_id = NOTIFICATION_GROUP_ID_NONE;
	noti->internal_group_id = NOTIFICATION_GROUP_ID_NONE;

	ret = rpc_port_proxy_notification_create(&notihandle);
	if (ret != RPC_PORT_ERROR_NONE) {
		WARN("create noti handle fail");
		return ret;
	}

	make_notification_from_noti(notihandle, noti, false);

	ret = rpc_port_proxy_noti_service_invoke_add_noti(service_handle, notihandle, &_priv_id);
	if (ret != RPC_PORT_ERROR_NONE) {
		WARN("request insert fail : %d", ret);
		rpc_port_proxy_notification_destroy(notihandle);
		return ret;
	}
	WARN("request insert done, id : %d", _priv_id);

	rpc_port_proxy_notification_destroy(notihandle);

	if (priv_id != NULL)
		*priv_id = _priv_id;

	return NOTIFICATION_ERROR_NONE;
}

int notification_tidl_request_refresh(uid_t uid)
{
	int ret;

	WARN("request refresh noti");

	ret = _tidl_init();
	if (ret != NOTIFICATION_ERROR_NONE) {
		WARN("tidl init fail");
		return ret;
	}
	WARN("tidl init done");

	ret = rpc_port_proxy_noti_service_invoke_refresh_noti(service_handle, (int)uid);
	if (ret != RPC_PORT_ERROR_NONE) {
		WARN("request refresh noti fail : %d", ret);
		return ret;
	}

	WARN("refresh noti done");

	return NOTIFICATION_ERROR_NONE;
}

int notification_tidl_request_update(notification_h noti)
{
	int ret;
	int priv_id;
	rpc_port_proxy_notification_h notihandle;

	WARN("request update noti");

	ret = _tidl_init();
	if (ret != NOTIFICATION_ERROR_NONE) {
		WARN("tidl init fail");
		return ret;
	}
	WARN("tidl init done");

	ret = rpc_port_proxy_notification_create(&notihandle);
	if (ret != RPC_PORT_ERROR_NONE) {
		WARN("create noti handle fail");
		return ret;
	}

	make_notification_from_noti(notihandle, noti, false);

	ret = rpc_port_proxy_noti_service_invoke_update_noti(service_handle, notihandle, &priv_id);
	if (ret != RPC_PORT_ERROR_NONE) {
		WARN("request update noti fail : %d", ret);
		rpc_port_proxy_notification_destroy(notihandle);
		return ret;
	}
	WARN("request update noti done, id : %d", priv_id);

	rpc_port_proxy_notification_destroy(notihandle);

	return NOTIFICATION_ERROR_NONE;
}

int notification_tidl_request_delete_single(char *app_id, int priv_id, uid_t uid)
{
	int ret;
	WARN("request del noti");

	ret = _tidl_init();
	if (ret != NOTIFICATION_ERROR_NONE) {
		WARN("tidl init fail");
		return ret;
	}
	WARN("tidl init done");

	ret = rpc_port_proxy_noti_service_invoke_delete_single_noti(service_handle,
			app_id, priv_id, (int)uid);
	if (ret != RPC_PORT_ERROR_NONE) {
		WARN("request delete noti fail : %d", ret);
		return ret;
	}
	WARN("service delete noti done, id : %d", priv_id);

	return NOTIFICATION_ERROR_NONE;
}

int notification_tidl_request_delete_multiple(notification_type_e type, char *app_id, uid_t uid)
{
	int ret;
	int deleted_num;

	WARN("request del multiple noti");

	ret = _tidl_init();
	if (ret != NOTIFICATION_ERROR_NONE) {
		WARN("tidl init fail");
		return ret;
	}
	WARN("tidl init done");

	if (app_id == NULL)
		app_id = "";

	ret = rpc_port_proxy_noti_service_invoke_delete_multiple_noti(service_handle,
			app_id, (int)type, (int)uid, &deleted_num);
	if (ret != RPC_PORT_ERROR_NONE) {
		WARN("request delete multi noti fail : %d", ret);
		return ret;
	}
	WARN("request delete multi noti done, id : %s, type : %d, num : %d",
			app_id, (int)type, deleted_num);

	WARN("request del multiple noti done");

	return NOTIFICATION_ERROR_NONE;
}

int notification_tidl_send_event(notification_h noti, int event_type, int priv_id)
{
	int ret;
	rpc_port_proxy_notification_h noti_handle = NULL;

	WARN("notification_tidl_send_event");

	ret = _tidl_init();
	if (ret != NOTIFICATION_ERROR_NONE) {
		WARN("tidl init fail");
		return ret;
	}
	WARN("tidl init done");

	if (priv_id > 0) {
		ret = rpc_port_proxy_noti_service_invoke_send_event_by_priv_id(service_handle, priv_id, event_type);
		if (ret != RPC_PORT_ERROR_NONE) {
			WARN("send event by priv id fail");
			return ret;
		}
	} else {
		ret = rpc_port_proxy_notification_create(&noti_handle);
		if (ret != RPC_PORT_ERROR_NONE) {
			WARN("create noti handle fail");
			return ret;
		}

		make_notification_from_noti(noti_handle, noti, false);

		ret = rpc_port_proxy_noti_service_invoke_send_event(service_handle, noti_handle, event_type);
		if (ret != RPC_PORT_ERROR_NONE) {
			WARN("send event fail");
			rpc_port_proxy_notification_destroy(noti_handle);
			return ret;
		}

		rpc_port_proxy_notification_destroy(noti_handle);
	}
	WARN("send event done : %d", ret);

	return NOTIFICATION_ERROR_NONE;
}

/* maybe not necessary app_id, uid */
int notification_tidl_request_load_noti_by_priv_id(notification_h *noti,
		const char *app_id, int priv_id, uid_t uid)
{
	int ret;
	notification_h _noti;
	rpc_port_proxy_notification_h notihandle;

	WARN("load noti by priv id : %d", priv_id);

	ret = _tidl_init();
	if (ret != NOTIFICATION_ERROR_NONE) {
		WARN("tidl init fail");
		return ret;
	}
	WARN("tidl init done");

	if (!app_id)
		app_id = "";

	ret = rpc_port_proxy_noti_service_invoke_load_noti_by_priv_id(service_handle,
			app_id, priv_id, uid, &notihandle);
	if (ret != RPC_PORT_ERROR_NONE) {
		WARN("request load by priv id fail : %d", ret);
		return ret;
	}

	ret = make_noti_from_notification(&_noti, notihandle);
	if (ret != NOTIFICATION_ERROR_NONE) {
		ERR("failed to make noti : %d", ret);
		rpc_port_proxy_notification_destroy(notihandle);
		return ret;
	}
	*noti = _noti;

	rpc_port_proxy_notification_destroy(notihandle);

	WARN("load by privid done : %d", priv_id);
	return NOTIFICATION_ERROR_NONE;
}

int notification_tidl_request_load_noti_by_tag(notification_h *noti,
		const char *app_id, const char *tag, uid_t uid)
{
	int ret;
	notification_h _noti;
	rpc_port_proxy_notification_h notihandle;

	WARN("load by tag : %s", tag);

	ret = _tidl_init();
	if (ret != NOTIFICATION_ERROR_NONE) {
		WARN("tidl init fail");
		return ret;
	}
	WARN("tidl init done");

	ret = rpc_port_proxy_noti_service_invoke_load_noti_by_tag(service_handle,
			app_id, tag, uid, &notihandle);
	if (ret != RPC_PORT_ERROR_NONE) {
		WARN("request load by tag fail : %d", ret);
		return ret;
	}

	ret = make_noti_from_notification(&_noti, notihandle);
	if (ret != NOTIFICATION_ERROR_NONE) {
		ERR("failed to make noti : %d", ret);
		rpc_port_proxy_notification_destroy(notihandle);
		return ret;
	}
	*noti = _noti;

	rpc_port_proxy_notification_destroy(notihandle);

	WARN("load by tag done : %s", tag);
	return NOTIFICATION_ERROR_NONE;
}

int notification_tidl_request_get_all_count(notification_type_e type, int *count, uid_t uid)
{
	int ret;
	int noti_count = 0;

	WARN("get all count");

	ret = _tidl_init();
	if (ret != NOTIFICATION_ERROR_NONE) {
		WARN("tidl init fail");
		return ret;
	}
	WARN("tidl init done");

	ret = rpc_port_proxy_noti_service_invoke_get_all_noti_count(service_handle,
			(int)type, uid, &noti_count);
	if (ret != RPC_PORT_ERROR_NONE) {
		WARN("get all count fail : %d", ret);
		return ret;
	}
	*count = noti_count;

	WARN("get all count done");

	return NOTIFICATION_ERROR_NONE;
}

// for setting
int notification_tidl_request_load_system_setting(notification_system_setting_h *setting, uid_t uid)
{
	int ret;
	notification_system_setting_h _setting;
	rpc_port_proxy_noti_system_setting_h system_setting;

	WARN("load system setting");

	ret = _tidl_init();
	if (ret != NOTIFICATION_ERROR_NONE) {
		WARN("tidl init fail");
		return ret;
	}
	WARN("tidl init done");

	// "load_system_setting" + "load_dnd_allow_exception"
	ret = rpc_port_proxy_noti_service_invoke_load_system_setting(service_handle, (int)uid, &system_setting);
	if (ret != RPC_PORT_ERROR_NONE) {
		WARN("load system setting fail : %d", ret);
		return ret;
	}

	make_setting_from_noti_system_setting(&_setting, system_setting);

	*setting = _setting;

	if (system_setting)
		rpc_port_proxy_noti_system_setting_destroy(system_setting);

	WARN("load system setting done");

	return NOTIFICATION_ERROR_NONE;
}

int notification_tidl_request_get_setting_array(
		notification_setting_h *setting_array, int *count, uid_t uid)
{
	int ret;
	int size;
	rpc_port_proxy_array_noti_setting_h handle_array;
	rpc_port_proxy_noti_setting_h *settings;
	notification_setting_h result_setting_array;
	notification_setting_h temp;

	WARN("load system setting array");

	ret = _tidl_init();
	if (ret != NOTIFICATION_ERROR_NONE) {
		WARN("tidl init fail");
		return ret;
	}
	WARN("tidl init done");

	ret = rpc_port_proxy_noti_service_invoke_get_setting_array(service_handle, (int)uid, &handle_array);
	if (ret != RPC_PORT_ERROR_NONE) {
		WARN("invoke setting array fail : %d", ret);
		return ret;
	}

	ret = rpc_port_proxy_array_noti_setting_get(handle_array, &settings, &size);
	if (ret != RPC_PORT_ERROR_NONE) {
		WARN("get setting fail : %d", ret);
		rpc_port_proxy_array_noti_setting_destroy(handle_array);
		return ret;
	}

	result_setting_array = (struct notification_setting *)malloc(sizeof(struct notification_setting) * size);
	if (result_setting_array == NULL) {
		ERR("malloc failed");
		rpc_port_proxy_array_noti_setting_destroy(handle_array);
		return NOTIFICATION_ERROR_OUT_OF_MEMORY;
	}

	for (int i = 0; i < size; i++) {
		temp = result_setting_array + i;
		make_setting_from_noti_setting(temp, settings[i]);
	}

	*count = size;
	*setting_array = result_setting_array;

	if (handle_array)
		rpc_port_proxy_array_noti_setting_destroy(handle_array);

	WARN("load system setting array done");

	return NOTIFICATION_ERROR_NONE;
}

int notification_tidl_request_get_setting_by_app_id(
		const char *app_id, notification_setting_h *setting, uid_t uid)
{
	int ret;
	rpc_port_proxy_noti_setting_h noti_setting;
	notification_setting_h _setting;

	WARN("get seting by appid : %s", app_id);

	ret = _tidl_init();
	if (ret != NOTIFICATION_ERROR_NONE) {
		WARN("tidl init fail");
		return ret;
	}
	WARN("tidl init done");

	ret = rpc_port_proxy_noti_service_invoke_get_setting_by_app_id(service_handle, app_id,
			(int)uid, &noti_setting);
	if (ret != RPC_PORT_ERROR_NONE) {
		WARN("invoke setting array by appid fail : %d", ret);
		return ret;
	}

	_setting = (struct notification_setting *)malloc(sizeof(struct notification_setting));
	if (_setting == NULL) {
		ERR("malloc failed");
		return NOTIFICATION_ERROR_OUT_OF_MEMORY;
	}

	make_setting_from_noti_setting(_setting, noti_setting);

	*setting = _setting;

	if (noti_setting)
		rpc_port_proxy_noti_setting_destroy(noti_setting);

	WARN("get seting by appid done");

	return NOTIFICATION_ERROR_NONE;
}

int notification_tidl_update_setting(notification_setting_h setting, uid_t uid)
{
	int ret;
	rpc_port_proxy_noti_setting_h noti_setting;

	WARN("update setting");

	ret = _tidl_init();
	if (ret != NOTIFICATION_ERROR_NONE) {
		WARN("tidl init fail");
		return ret;
	}
	WARN("tidl init done");

	ret = rpc_port_proxy_noti_setting_create(&noti_setting);
	if (ret != RPC_PORT_ERROR_NONE) {
		WARN("create noti_setting failed : %d", ret);
		return ret;
	}

	make_noti_setting_from_setting(noti_setting, setting);

	ret = rpc_port_proxy_noti_service_invoke_update_noti_setting(service_handle,
			noti_setting, (int)uid);
	if (ret != RPC_PORT_ERROR_NONE) {
		WARN("request invoke update noti setting failed : %d", ret);
		rpc_port_proxy_noti_setting_destroy(noti_setting);
		return ret;
	}
	rpc_port_proxy_noti_setting_destroy(noti_setting);

	WARN("update setting done");

	return NOTIFICATION_ERROR_NONE;
}

int notification_tidl_update_system_setting(
		notification_system_setting_h system_setting, uid_t uid)
{
	int ret;
	rpc_port_proxy_noti_system_setting_h sys_setting;

	WARN("update system setting");

	ret = _tidl_init();
	if (ret != NOTIFICATION_ERROR_NONE) {
		WARN("tidl init fail");
		return ret;
	}
	WARN("tidl init done");

	ret = rpc_port_proxy_noti_system_setting_create(&sys_setting);
	if (ret != RPC_PORT_ERROR_NONE) {
		WARN("create noti_system_setting failed : %d", ret);
		return ret;
	}

	make_noti_system_setting_from_setting(sys_setting, system_setting);

	ret = rpc_port_proxy_noti_service_invoke_update_noti_sys_setting(service_handle,
			sys_setting, (int)uid);
	if (ret != RPC_PORT_ERROR_NONE) {
		WARN("request invoke update noti system setting failed : %d", ret);
		rpc_port_proxy_noti_system_setting_destroy(sys_setting);
		return ret;
	}
	rpc_port_proxy_noti_system_setting_destroy(sys_setting);

	WARN("update system setting done");

	return NOTIFICATION_ERROR_NONE;
}


int notification_tidl_request_save_as_template(notification_h noti, const char *template_name)
{
	int ret;
	rpc_port_proxy_notification_h notihandle;

	WARN("save as template");

	ret = _tidl_init();
	if (ret != NOTIFICATION_ERROR_NONE) {
		WARN("tidl init fail");
		return ret;
	}
	WARN("tidl init done");

	ret = rpc_port_proxy_notification_create(&notihandle);
	if (ret != RPC_PORT_ERROR_NONE) {
		WARN("create noti handle fail");
		return ret;
	}

	make_notification_from_noti(notihandle, noti, false);

	ret = rpc_port_proxy_noti_service_invoke_save_as_template(service_handle, notihandle, template_name);
	if (ret != RPC_PORT_ERROR_NONE) {
		WARN("request invoke save as template failed : %d", ret);
		rpc_port_proxy_notification_destroy(notihandle);
		return ret;
	}
	rpc_port_proxy_notification_destroy(notihandle);

	WARN("save as template done ");

	return NOTIFICATION_ERROR_NONE;
}

int notification_tidl_request_create_from_template(notification_h *noti, const char *template_name)
{
	int ret;
	rpc_port_proxy_notification_h notihandle;
	notification_h _noti;

	WARN("create from template");

	ret = _tidl_init();
	if (ret != NOTIFICATION_ERROR_NONE) {
		WARN("tidl init fail");
		return ret;
	}
	WARN("tidl init done");

	ret = rpc_port_proxy_noti_service_invoke_create_from_template(service_handle, template_name, &notihandle);
	if (ret != RPC_PORT_ERROR_NONE) {
		WARN("request invoke save as template failed : %d", ret);
		return ret;
	}

	ret = make_noti_from_notification(&_noti, notihandle);
	if (ret != NOTIFICATION_ERROR_NONE) {
		ERR("failed to make noti : %d", ret);
		rpc_port_proxy_notification_destroy(notihandle);
		return ret;
	}
	*noti = _noti;

	rpc_port_proxy_notification_destroy(notihandle);

	WARN("create from template done : %d", ret);

	return NOTIFICATION_ERROR_NONE;
}

int notification_tidl_request_create_from_package_template(notification_h *noti,
		const char *app_id, const char *template_name)
{
	int ret;
	rpc_port_proxy_notification_h notihandle;
	notification_h _noti;

	WARN("create from package template");

	ret = _tidl_init();
	if (ret != NOTIFICATION_ERROR_NONE) {
		WARN("tidl init fail");
		return ret;
	}
	WARN("tidl init done");

	ret = rpc_port_proxy_noti_service_invoke_create_from_package_template(service_handle,
			app_id, template_name, &notihandle);
	if (ret != RPC_PORT_ERROR_NONE) {
		WARN("request invoke save as template failed : %d", ret);
		return ret;
	}

	ret = make_noti_from_notification(&_noti, notihandle);
	if (ret != NOTIFICATION_ERROR_NONE) {
		ERR("failed to make noti : %d", ret);
		rpc_port_proxy_notification_destroy(notihandle);
		return ret;
	}
	*noti = _noti;

	rpc_port_proxy_notification_destroy(notihandle);

	WARN("create from package template done");

	return NOTIFICATION_ERROR_NONE;
}

static bool _rpc_port_proxy_list_notification_cb(
		rpc_port_proxy_notification_h value, void *user_data)
{
	int ret;
	notification_h noti;
	notification_list_h *list = (notification_list_h*)user_data;

	WARN("notification list foreach");

	ret = make_noti_from_notification(&noti, value);
	if (ret != NOTIFICATION_ERROR_NONE) {
		ERR("failed to make noti : %d", ret);
		return ret;
	}
	*list = notification_list_append(*list, noti);

	return true;
}

int notification_tidl_request_load_noti_grouping_list(notification_type_e type,
		int count, int count_per_page, notification_list_h *list, uid_t uid)
{
	int ret;
	rpc_port_proxy_list_notification_h noti_list;

	WARN("load noti grouping list");

	ret = _tidl_init();
	if (ret != NOTIFICATION_ERROR_NONE) {
		WARN("tidl init fail");
		return ret;
	}
	WARN("tidl init done");

	ret = rpc_port_proxy_noti_service_invoke_load_noti_grouping_list(service_handle,
			(int)type, count, count_per_page, (int)uid, &noti_list);
	if (ret != RPC_PORT_ERROR_NONE) {
		WARN("request invoke load noti grouping list : %d", ret);
		return ret;
	}

	ret = rpc_port_proxy_list_notification_foreach(noti_list,
			_rpc_port_proxy_list_notification_cb, list);
	if (ret != RPC_PORT_ERROR_NONE) {
		WARN("failed to get length : %d", ret);
		rpc_port_proxy_list_notification_destroy(noti_list);
		return ret;
	}
	rpc_port_proxy_list_notification_destroy(noti_list);

	WARN("load noti grouping list done");

	return NOTIFICATION_ERROR_NONE;
}


int notification_tidl_request_load_noti_detail_list(const char *app_id,
		int group_id, int priv_id, int count, notification_list_h *list, uid_t uid)
{
	int ret;
	rpc_port_proxy_list_notification_h noti_list;

	WARN("load noti detail list : %d", priv_id);

	ret = _tidl_init();
	if (ret != NOTIFICATION_ERROR_NONE) {
		WARN("tidl init fail");
		return ret;
	}
	WARN("tidl init done");

	ret = rpc_port_proxy_noti_service_invoke_load_noti_detail_list(service_handle,
			app_id, group_id, priv_id, count, (int)uid, &noti_list);
	if (ret != RPC_PORT_ERROR_NONE) {
		WARN("request invoke load noti grouping list : %d", ret);
		return ret;
	}

	ret = rpc_port_proxy_list_notification_foreach(noti_list,
			_rpc_port_proxy_list_notification_cb, list);
	if (ret != RPC_PORT_ERROR_NONE) {
		WARN("failed to get length : %d", ret);
		rpc_port_proxy_list_notification_destroy(noti_list);
		return ret;
	}
	rpc_port_proxy_list_notification_destroy(noti_list);

	WARN("load noti detail list done");

	return NOTIFICATION_ERROR_NONE;
}

int notification_tidl_get_noti_block_state(const char *app_id, int *do_not_disturb,
		int *do_not_disturb_except, int *allow_to_notify, uid_t uid)
{
	int ret;

	WARN("get block state");

	ret = _tidl_init();
	if (ret != NOTIFICATION_ERROR_NONE) {
		WARN("tidl init fail");
		return ret;
	}
	WARN("tidl init done");

	ret = rpc_port_proxy_noti_service_invoke_get_noti_block_state(service_handle, app_id, (int)uid,
			do_not_disturb, do_not_disturb_except, allow_to_notify);
	if (ret != RPC_PORT_ERROR_NONE) {
		WARN("request invoke load noti grouping list : %d", ret);
		return ret;
	}
	WARN("get block state done");

	return NOTIFICATION_ERROR_NONE;
}

int notification_tidl_request_delete_by_display_applist(int display_applist, uid_t uid)
{
	int ret;

	WARN("delete by display app list");

	ret = _tidl_init();
	if (ret != NOTIFICATION_ERROR_NONE)	{
		ERR("failed to init tidl : %d", ret);
		return ret;
	}

	ret = rpc_port_proxy_noti_service_invoke_delete_noti_by_display_applist(
			service_handle, display_applist, uid);
	if (ret != RPC_PORT_ERROR_NONE) {
		ERR("request delete noti by display app list fail : %d", ret);
		return ret;
	}
	WARN("delete by display app list done");

	return NOTIFICATION_ERROR_NONE;
}

int notification_tidl_request_get_count(notification_type_e type,
		    const char *app_id, int group_id, int priv_id, int *count, uid_t uid)
{
	int ret;
	int re_count;

	WARN("get count : %s", app_id);

	ret = _tidl_init();
	if (ret != NOTIFICATION_ERROR_NONE)	{
		ERR("failed to init tidl : %d", ret);
		return ret;
	}

	ret = rpc_port_proxy_noti_service_invoke_get_noti_count(service_handle,
			(int)type, app_id, group_id, priv_id, uid, &re_count);
	if (ret != RPC_PORT_ERROR_NONE) {
		ERR("request delete noti by display app list fail : %d", ret);
		return ret;
	}
	*count = re_count;

	WARN("get count done : %d", re_count);

	return NOTIFICATION_ERROR_NONE;
}

int notification_tidl_check_event_receiver(int priv_id, bool *available)
{
	int ret;
	bool ret_available;

	WARN("check event receiver : %d", priv_id);

	ret = _tidl_init();
	if (ret != NOTIFICATION_ERROR_NONE)	{
		ERR("failed to init tidl : %d", ret);
		return ret;
	}

	ret = rpc_port_proxy_noti_service_invoke_check_event_receiver(service_handle, priv_id, &ret_available);
	if (ret != RPC_PORT_ERROR_NONE) {
		ERR("request delete noti by display app list fail : %d", ret);
		return ret;
	}
	*available = ret_available;

	WARN("check event receiver done");

	return NOTIFICATION_ERROR_NONE;
}

void notification_tidl_reset_event_handler(int priv_id)
{
	int ret;

	WARN("reset event handler : %d", priv_id);

	ret = _tidl_init();
	if (ret != NOTIFICATION_ERROR_NONE)	{
		ERR("failed to init tidl : %d", ret);
		return;
	}

	ret = rpc_port_proxy_noti_service_invoke_reset_event_handler(
			service_handle, priv_id);
	if (ret != RPC_PORT_ERROR_NONE) {
		ERR("request reset event handler fail : %d", ret);
		return;
	}
	WARN("reset event handler done");
}

int notification_tidl_request_register_dnd_app(uid_t uid, pid_t pid)
{
	int ret;

	WARN("request register dnd app");

	ret = _tidl_init();
	if (ret != NOTIFICATION_ERROR_NONE)	{
		ERR("failed to init tidl : %d", ret);
		return ret;
	}

	ret = rpc_port_proxy_noti_service_invoke_register_dnd_app(
			service_handle, uid, pid);
	if (ret != RPC_PORT_ERROR_NONE) {
		ERR("request register dnd app fail : %d", ret);
		return ret;
	}
	WARN("register dnd app done [%d]", ret);

	return NOTIFICATION_ERROR_NONE;
}

int notification_tidl_request_unregister_dnd_app(uid_t uid, pid_t pid)
{
	int ret;

	WARN("request unregister dnd app");

	ret = _tidl_init();
	if (ret != NOTIFICATION_ERROR_NONE)	{
		ERR("failed to init tidl : %d", ret);
		return ret;
	}

	ret = rpc_port_proxy_noti_service_invoke_unregister_dnd_app(service_handle, uid, pid);
	if (ret != RPC_PORT_ERROR_NONE) {
		ERR("request unregister dnd app fail : %d", ret);
		return ret;
	}
	WARN("request unregister dnd app done");

	return NOTIFICATION_ERROR_NONE;
}

/* for channel */
int notification_tidl_request_insert_channel(const char *app_id,
		const char *channel_name, bool blockable, bool is_blocked, uid_t uid)
{
	int ret;

	WARN("request insert channel");

	ret = _tidl_init();
	if (ret != NOTIFICATION_ERROR_NONE)	{
		ERR("failed to init tidl : %d", ret);
		return ret;
	}

	ret = rpc_port_proxy_noti_service_invoke_channel_insert(service_handle,
			app_id, channel_name, blockable, is_blocked, uid);
	if (ret != RPC_PORT_ERROR_NONE) {
		ERR("service insert channel fail : %d", ret);
		return ret;
	}
	WARN("request insert channel done [%d]", ret);

	return NOTIFICATION_ERROR_NONE;
}

int notification_tidl_request_delete_channel(const char *app_id,
		const char *channel_name, uid_t uid)
{
	int ret;

	WARN("request delete channel");

	ret = _tidl_init();
	if (ret != NOTIFICATION_ERROR_NONE)	{
		ERR("failed to init tidl : %d", ret);
		return ret;
	}

	ret = rpc_port_proxy_noti_service_invoke_channel_delete(service_handle,
			app_id, channel_name, uid);
	if (ret != RPC_PORT_ERROR_NONE) {
		ERR("request delete channel fail : %d", ret);
		return ret;
	}
	WARN("request delete channel done [%d]", ret);

	return NOTIFICATION_ERROR_NONE;
}

int notification_tidl_request_update_channel(const char *app_id,
		const char *channel_name, bool blockable, bool is_blocked, uid_t uid)
{
	int ret;

	WARN("request update channel");

	ret = _tidl_init();
	if (ret != NOTIFICATION_ERROR_NONE)	{
		ERR("failed to init tidl : %d", ret);
		return ret;
	}

	ret = rpc_port_proxy_noti_service_invoke_channel_update(service_handle,
			app_id, channel_name, blockable, is_blocked, uid);
	if (ret != RPC_PORT_ERROR_NONE) {
		ERR("request update channel fail : %d", ret);
		return ret;
	}
	WARN("request update channel done [%d]", ret);

	return NOTIFICATION_ERROR_NONE;
}

int notification_tidl_request_get_channel(const char *app_id,
		const char *channel_name, bool *blockable, bool *is_blocked, uid_t uid)
{
	int ret;
	bool _blockable;
	bool _is_blocked;

	WARN("request channel get");

	ret = _tidl_init();
	if (ret != NOTIFICATION_ERROR_NONE)	{
		ERR("failed to init tidl : %d", ret);
		return ret;
	}

	ret = rpc_port_proxy_noti_service_invoke_channel_get(service_handle,
			app_id, channel_name, &_blockable, &_is_blocked, uid);
	if (ret != RPC_PORT_ERROR_NONE) {
		ERR("request get channel fail : %d", ret);
		return ret;
	}

	*blockable = _blockable;
	*is_blocked = _is_blocked;

	WARN("request channel get done [%d]", ret);

	return NOTIFICATION_ERROR_NONE;
}

static bool _rpc_port_proxy_list_noti_channel_cb (
		rpc_port_proxy_noti_channel_h noti_channel, void *user_data)
{
	channel_foreach_cb_info *cb_info = (channel_foreach_cb_info *)user_data;
	char *app_id = NULL;
	char *channel_name = NULL;
	bool blockable;
	bool is_blocked;
	notification_channel_h channel = NULL;

	rpc_port_proxy_noti_channel_get_app_id(noti_channel, &app_id);
	rpc_port_proxy_noti_channel_get_channel_name(noti_channel, &channel_name);
	rpc_port_proxy_noti_channel_get_blockable(noti_channel, &blockable);
	rpc_port_proxy_noti_channel_get_is_blocked(noti_channel, &is_blocked);

	notification_channel_create_with_info(app_id, channel_name,
			blockable ? 1 : 0, is_blocked ? 1 : 0, &channel);

	WARN("channel list foreach [%s][%s][%d][%d]", app_id, channel_name, (int)blockable, (int)is_blocked);

	cb_info->cb(channel ,cb_info->user_data);

	if (channel)
		notification_channel_free(channel);

	if (app_id)
		free(app_id);

	if (channel_name)
		free(channel_name);

	return true;
}

int notification_tidl_request_channel_get_list(const char *app_id,
		notification_channel_foreach_cb cb, void *user_data, uid_t uid)
{
	int ret;
	rpc_port_proxy_list_noti_channel_h channel_list;
	channel_foreach_cb_info *cb_info = NULL;
	unsigned int length;

	WARN("request channel list");

	ret = _tidl_init();
	if (ret != NOTIFICATION_ERROR_NONE)	{
		ERR("failed to init tidl : %d", ret);
		return ret;
	}

	ret = rpc_port_proxy_noti_service_invoke_channel_get_list(service_handle,
			app_id, uid, &channel_list);
	if (ret != RPC_PORT_ERROR_NONE) {
		ERR("request update channel fail : %d", ret);
		return ret;
	}

	ret = rpc_port_proxy_list_noti_channel_length(channel_list, &length);
	if (ret != RPC_PORT_ERROR_NONE) {
		ERR("service update channel fail : %d", ret);
		rpc_port_proxy_list_noti_channel_destroy(channel_list);
		return ret;
	}
	WARN("request channel list length [%d]", (int)length);

	cb_info = (channel_foreach_cb_info *)calloc(1, sizeof(channel_foreach_cb_info));
	if (cb_info == NULL) {
		ERR("out of memory");
		return NOTIFICATION_ERROR_OUT_OF_MEMORY;
	}
	cb_info->cb = cb;
	cb_info->user_data = user_data;

	ret = rpc_port_proxy_list_noti_channel_foreach(channel_list,
			_rpc_port_proxy_list_noti_channel_cb, cb_info);
	if (ret != RPC_PORT_ERROR_NONE) {
		ERR("channel foreach fail : %d", ret);
		free(cb_info);
		rpc_port_proxy_list_noti_channel_destroy(channel_list);
		return ret;
	}
	free(cb_info);

	rpc_port_proxy_list_noti_channel_destroy(channel_list);

	WARN("request channel list done [%d]", ret);

	return NOTIFICATION_ERROR_NONE;
}

/* is it used*/
int notification_tidl_add_deffered_task(
		void (*deferred_task_cb)(void *data),
		void *user_data)
{
	int ret;

	WARN("add deffered task");

	ret = _tidl_init();
	if (ret != NOTIFICATION_ERROR_NONE)	{
		ERR("failed to init tidl : %d", ret);
		return ret;
	}
	deferred_task_cb(user_data);

	WARN("add deffered task done");

	return NOTIFICATION_ERROR_NONE;
}

int notification_tidl_del_deffered_task(
		void (*deferred_task_cb)(void *data))
{
	WARN("del deffered task");
	return NOTIFICATION_ERROR_NONE;
}
