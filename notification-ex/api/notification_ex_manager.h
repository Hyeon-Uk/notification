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

#ifndef CAPI_NOTIFICATION_EX_MANAGER_H_
#define CAPI_NOTIFICATION_EX_MANAGER_H_

#include <notification_ex_error.h>
#include <notification_ex_item.h>
#include <notification_ex_event_info.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief The notification_ex manager handle.
 * @since_tizen 5.5
 */
typedef void* noti_ex_manager_h;

/**
 * @brief Called when the notification add event is receved.
 * @since_tizen 5.5
 * @param[in] handle The manager handle
 *            The @a handle can be used only in the callback.
 *            The @a handle will be freed right after callback finished.
 * @param[in] info The handle that contains event information
 *            The @a info can be used only in the callback. To use outside, make a copy.
 *            The @a info will be freed right after callback finished.
 * @param[in] added_item The item handle to be added
 *            The @a added_item can be used only in the callback.
 *            The @a added_item will be freed right after callback finished.
 * @param[in] count The count of the item to be added
 * @param[in] data The user data which was registered with callback
 *            The @a data can be used only in the callback. To use outside, make a copy.
 * @see #noti_ex_manager_h
 * @see #noti_ex_event_info_h
 * @see #noti_ex_item_h
 * @see #noti_ex_event_info_clone
 */
typedef void (*noti_ex_manager_events_add_cb)(noti_ex_manager_h handle,
		noti_ex_event_info_h info, noti_ex_item_h *added_item,
		int count, void *data);

/**
 * @brief Called when the notification update event is receved.
 * @since_tizen 5.5
 * @param[in] handle The manager handle
 *            The @a handle can be used only in the callback.
 *            The @a handle will be freed right after callback finished.
 * @param[in] info The handle that contains event information
 *            The @a info can be used only in the callback. To use outside, make a copy.
 *            The @a info will be freed right after callback finished.
 * @param[in] updated_item The item handle to be updated
 *            The @a updated_item can be used only in the callback.
 *            The @a updated_item will be freed right after callback finished.
 * @param[in] data The user data which was registered with callback
 *            The @a data can be used only in the callback. To use outside, make a copy.
 * @see #noti_ex_manager_h
 * @see #noti_ex_event_info_h
 * @see #noti_ex_item_h
 * @see #noti_ex_event_info_clone
 */
typedef void (*noti_ex_manager_events_update_cb)(noti_ex_manager_h handle,
		noti_ex_event_info_h info, noti_ex_item_h updated_item,
		void *data);

/**
 * @brief Called when the notification delete event is receved.
 * @since_tizen 5.5
 * @param[in] handle The manager handle
 *            The @a handle can be used only in the callback.
 *            The @a handle will be freed right after callback finished.
 * @param[in] info The handle that contains event information
 *            The @a info can be used only in the callback. To use outside, make a copy.
 *            The @a info will be freed right after callback finished.
 * @param[in] deleted_item The item handle to be deleted
 *            The @a deleted_item can be used only in the callback.
 *            The @a deleted_item will be freed right after callback finished.
 * @param[in] data The user data which was registered with callback
 *            The @a data can be used only in the callback. To use outside, make a copy.
 * @see #noti_ex_manager_h
 * @see #noti_ex_event_info_h
 * @see #noti_ex_item_h
 * @see #noti_ex_event_info_clone
 */
typedef void (*noti_ex_manager_events_delete_cb)(noti_ex_manager_h handle,
		noti_ex_event_info_h info, noti_ex_item_h deleted_item,
		void *data);

/**
 * @brief Called when the error event is receved.
 * @since_tizen 5.5
 * @param[in] handle The manager handle
 *            The @a handle can be used only in the callback.
 *            The @a handle will be freed right after callback finished.
 * @param[in] error The error type
 * @param[in] request_id The id of the request that occurred error
 * @param[in] data The user data which was registered with callback
 *            The @a data can be used only in the callback. To use outside, make a copy.
 * @see #noti_ex_manager_h
 * @see #noti_ex_error_e
 */
typedef void (*noti_ex_manager_events_error_cb)(noti_ex_manager_h handle,
		noti_ex_error_e error, int request_id, void *data);

/**
 * @brief Called when the notification request event is receved.
 * @since_tizen 5.5
 * @param[in] handle The manager handle
 *            The @a handle can be used only in the callback.
 *            The @a handle will be freed right after callback finished.
 * @param[in] info The handle that contains event information
 *            The @a info can be used only in the callback. To use outside, make a copy.
 *            The @a info will be freed right after callback finished.
 * @param[out] items The item handle to be responded to the requester
 * @param[out] cnt The count of items which will be responded to the requester.
 * @param[in] data The user data which was registered with callback
 *            The @a data can be used only in the callback. To use outside, make a copy.
 * @see #noti_ex_manager_h
 * @see #noti_ex_event_info_h
 * @see #noti_ex_item_h
 * @see #noti_ex_event_info_clone
 */
typedef void (*noti_ex_manager_events_request_cb)(noti_ex_manager_h handle,
		noti_ex_event_info_h info, noti_ex_item_h **items, int *cnt, void *data);

/**
 * @brief The structure type to contain the set of callback functions for handling the notification events.
 * @since_tizen 5.5
 *
 * @see noti_ex_manager_events_add_cb()
 * @see noti_ex_manager_events_update_cb()
 * @see noti_ex_manager_events_delete_cb()
 * @see noti_ex_manager_events_error_cb()
 * @see noti_ex_manager_events_request_cb()
 */
typedef struct {
	noti_ex_manager_events_add_cb added;
	noti_ex_manager_events_update_cb updated;
	noti_ex_manager_events_delete_cb deleted;
	noti_ex_manager_events_error_cb error;
	noti_ex_manager_events_request_cb requested;
} noti_ex_manager_events_s;

/**
 * @brief Creates the notification_ex manager handle.
 * @details The manager handle is created with a events callback and
 *          the user can listen notification events through the events callback.
 *          If the user wants to manage notifications for specific group
 *          the user should identify it with @a receiver_group
 * @since_tizen 5.5
 * @privlevel public
 * @privilege %http://tizen.org/privilege/notification
 * @remarks @a handle must be released using noti_ex_manager_destroy().
 * @param[out] handle The notification manager handle
 * @param[in] receiver_group The notification group name of manager
 * @param[in] event_callbacks The callbacks for notification_ex events
 * @param[in] data The user data for @a event_callbacks
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #NOTI_EX_ERROR_PERMISSION_DENIED Permission deny
 * @retval #NOTI_EX_ERROR_OUT_OF_MEMORY Out of memory
 * @see #noti_ex_manager_h
 * @see #noti_ex_manager_events_s
 * @see #RECEIVERGROUP_PANEL
 * @see #RECEIVERGROUP_TICKER
 * @see #RECEIVERGROUP_LOCKSCREEN
 * @see #RECEIVERGROUP_INDICATOR
 * @see #RECEIVERGROUP_POPUP
 * @par Sample code:
 * @code
#include <notification_ex_manager.h>

void _manager_events_add_cb(noti_ex_manager_h handle, noti_ex_event_info_h info,
		noti_ex_item_h *added_item, int cnt, void *data) {
}

void _manager_events_update_cb(noti_ex_manager_h handle,
		noti_ex_event_info_h info, noti_ex_item_h updated_item, void *data) {
}

void _manager_events_delete_cb(noti_ex_manager_h handle,
		noti_ex_event_info_h info, noti_ex_item_h deleted_item, void *data) {
}

void _manager_events_error_cb(noti_ex_manager_h handle,
		noti_ex_error_e error, int req_id, void *data) {
}

void _manager_events_request_cb(noti_ex_manager_h handle,
		noti_ex_event_info_h info, noti_ex_item_h **items, int *cnt, void *data) {
}

{
	int ret;
	noti_ex_manager_h handle;
	noti_ex_manager_events_s ev;

	ev.added = _manager_events_add_cb;
	ev.updated = _manager_events_update_cb;
	ev.deleted = _manager_events_delete_cb;
	ev.error = _manager_events_error_cb;
	ev.requested = _manager_events_request_cb;
	ret = noti_ex_manager_create(&handle, RECEIVERGROUP_PANEL, ev, NULL);
}
 * @endcode
 */
int noti_ex_manager_create(noti_ex_manager_h *handle,
		const char *receiver_group, noti_ex_manager_events_s event_callbacks,
		void *data);

/**
 * @brief Destroys the notification_ex manager handle.
 * @since_tizen 5.5
 * @privlevel public
 * @param[in] handle The notification manager handle
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_item_h
 * @par Sample code:
 * @code
#include <notification_ex_manager.h>

void _manager_events_error_cb(noti_ex_manager_h handle, noti_ex_error_e error,
		int req_id, void *data) {
}

{
	int ret;
	noti_ex_manager_h handle;
	noti_ex_manager_events_s ev;

	ev.error = _manager_events_error_cb;
	ret = noti_ex_manager_create(&handle, RECEIVERGROUP_PANEL, ev, NULL);
	ret = noti_ex_manager_destroy(handle);
}
 * @endcode
 */
int noti_ex_manager_destroy(noti_ex_manager_h handle);

/**
 * @brief Gets all notifications for the notification_ex manager.
 * @details Every notification that has been sent for the manager will be retrieved.
 * @since_tizen 5.5
 * @privlevel public
 * @privilege %http://tizen.org/privilege/notification
 * @param[in] handle The notification manager handle
 * @param[out] items The result of the get request
 * @param[out] count The count of retrieved notification items.
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #NOTI_EX_ERROR_PERMISSION_DENIED Permission deny
 * @see #noti_ex_item_h
 * @see #noti_ex_manager_h
 * @par Sample code:
 * @code
#include <notification_ex_manager.h>

void _manager_events_error_cb(noti_ex_manager_h handle, noti_ex_error_e error,
		int req_id, void *data) {
}

{
	int ret;
	noti_ex_manager_h handle;
	noti_ex_manager_events_s ev;
	noti_ex_item_h *items;
	noti_ex_item_h *item;
	int items_count;

	ev.error = _manager_events_error_cb;
	ret = noti_ex_manager_create(&handle, RECEIVERGROUP_PANEL, ev, NULL);
	ret = noti_ex_manager_get(handle, &items, &items_count);
	for (int i = 0; i < items_count; i++) {
		item = items[i];
		// do something with item
	}
}
 * @endcode
 */
int noti_ex_manager_get(noti_ex_manager_h handle,
		noti_ex_item_h **items, int *count);

/**
 * @brief Updates notification.
 * @since_tizen 5.5
 * @privlevel public
 * @privilege %http://tizen.org/privilege/notification
 * @param[in] handle The notification manager handle
 * @param[in] item The notification item to be updated
 * @param[out] request_id The id of request
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #NOTI_EX_ERROR_PERMISSION_DENIED Permission deny
 * @see #noti_ex_item_h
 * @see #noti_ex_manager_h
 * @par Sample code:
 * @code
#include <notification_ex_manager.h>

void _manager_events_error_cb(noti_ex_manager_h handle, noti_ex_error_e error,
		int req_id, void *data) {
}

{
	int ret;
	noti_ex_manager_h handle;
	noti_ex_manager_events_s ev;
	noti_ex_item_h *items;
	noti_ex_item_h *item;
	int items_count;
	int request_id;

	ev.error = _manager_events_error_cb;
	ret = noti_ex_manager_create(&handle, RECEIVERGROUP_PANEL, ev, NULL);
	ret = noti_ex_manager_get(handle, &items, &items_count);
	for (int i = 0; i < items_count; i++) {
		item = items[i];

		// modify item
		noti_ex_manager_update(handle, item, &request_id);
	}
}
 * @endcode
 */
int noti_ex_manager_update(noti_ex_manager_h handle, noti_ex_item_h item,
		int *request_id);

/**
 * @brief Deletes notification.
 * @since_tizen 5.5
 * @privlevel public
 * @privilege %http://tizen.org/privilege/notification
 * @param[in] handle The notification manager handle
 * @param[in] item The notification item to be updated
 * @param[out] request_id The id of request
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #NOTI_EX_ERROR_PERMISSION_DENIED Permission deny
 * @see #noti_ex_manager_h
 * @see #noti_ex_error_e
 * @par Sample code:
 * @code
#include <notification_ex_manager.h>

void _manager_events_error_cb(noti_ex_manager_h handle, noti_ex_error_e error,
		int req_id, void *data) {
}

{
	int ret;
	noti_ex_manager_h handle;
	noti_ex_manager_events_s ev;
	noti_ex_item_h *items;
	noti_ex_item_h *item;
	int items_count;
	int request_id;

	ev.error = _manager_events_error_cb;
	ret = noti_ex_manager_create(&handle, RECEIVERGROUP_PANEL, ev, NULL);
	ret = noti_ex_manager_get(handle, &items, &items_count);
	for (int i = 0; i < items_count; i++) {
		item = items[i];
		noti_ex_manager_delete(handle, item, &request_id);
	}
}
 * @endcode
 */
int noti_ex_manager_delete(noti_ex_manager_h handle, noti_ex_item_h noti,
		int *request_id);

/**
 * @brief Deletes all notification.
 * @since_tizen 5.5
 * @privlevel public
 * @privilege %http://tizen.org/privilege/notification
 * @param[in] handle The notification manager handle
 * @param[out] request_id The id of request
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #NOTI_EX_ERROR_PERMISSION_DENIED Permission deny
 * @see #noti_ex_manager_h
 * @see #noti_ex_error_e
 * @par Sample code:
 * @code
#include <notification_ex_manager.h>

void _manager_events_error_cb(noti_ex_manager_h handle, noti_ex_error_e error,
		int req_id, void *data) {
}

{
	int ret;
	noti_ex_manager_h handle;
	noti_ex_manager_events_s ev;
	noti_ex_item_h *items;
	noti_ex_item_h *item;
	int items_count;
	int request_id;

	ev.error = _manager_events_error_cb;
	ret = noti_ex_manager_create(&handle, RECEIVERGROUP_PANEL, ev, NULL);
	ret = noti_ex_manager_delete_all(handle, request_id);
}
 * @endcode
 */
int noti_ex_manager_delete_all(noti_ex_manager_h handle,
		int *request_id);

/**
 * @brief Hides notification.
 * @since_tizen 5.5
 * @privlevel public
 * @privilege %http://tizen.org/privilege/notification
 * @param[in] handle The notification manager handle
 * @param[in] item The notification item to be hidden
 * @param[out] request_id The id of request
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #NOTI_EX_ERROR_PERMISSION_DENIED Permission deny
 * @see #noti_ex_item_h
 * @see #noti_ex_manager_h
 * @par Sample code:
 * @code
#include <notification_ex_manager.h>

void _manager_events_error_cb(noti_ex_error_e error, int req_id, void *data) {
}

{
	int ret;
	noti_ex_manager_h handle;
	noti_ex_manager_events_s ev;
	noti_ex_item_h *items;
	noti_ex_item_h *item;
	int items_count;
	int request_id;

	ev.error = _manager_events_error_cb;
	ret = noti_ex_manager_create(&handle, RECEIVERGROUP_PANEL, ev, NULL);
	for (int i = 0; i < items_count; i++) {
		item = items[i];
		ret = noti_ex_manager_hide(handle, item, &request_id);
	}
}
 * @endcode
 */
int noti_ex_manager_hide(noti_ex_manager_h handle, noti_ex_item_h item,
		int *request_id);

/**
 * @brief Finds notification by root id.
 * @since_tizen 5.5
 * @privlevel public
 * @privilege %http://tizen.org/privilege/notification
 * @remarks @a root_id is the id of the foremost notification item.
 * @param[in] handle The notification manager handle
 * @param[in] root_id The id of the foremost notification item to be retrived
 * @param[out] item The result of the find operation
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #NOTI_EX_ERROR_PERMISSION_DENIED Permission deny
 * @see #noti_ex_manager_h
 * @see #noti_ex_item_h
 * @par Sample code:
 * @code
#include <notification_ex_manager.h>

void _manager_events_error_cb(noti_ex_error_e error, int req_id, void *data) {
}

{
	int ret;
	noti_ex_manager_h handle;
	noti_ex_manager_events_s ev;
	noti_ex_item_h item;

	ev.error = _manager_events_error_cb;
	ev.requested = _manager_events_request_cb;
	ret = noti_ex_manager_create(&handle, RECEIVERGROUP_PANEL, ev, NULL);
	ret = noti_ex_manager_find_by_root_id(handle, "group1", &item);
}
 * @endcode
 */
int noti_ex_manager_find_by_root_id(noti_ex_manager_h handle,
		const char *root_id, noti_ex_item_h *item);

/**
 * @brief Sends error.
 * @since_tizen 5.5
 * @privlevel public
 * @privilege %http://tizen.org/privilege/notification
 * @param[in] handle The notification manager handle
 * @param[in] info The handle that contains event information
 * @param[in] error The error type
 * @param[out] request_id The id of request
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #NOTI_EX_ERROR_PERMISSION_DENIED Permission deny
 * @see #noti_ex_event_info_h
 * @see #noti_ex_manager_h
 * @see #noti_ex_error_e
 * @par Sample code:
 * @code
#include <notification_ex_manager.h>

void _manager_events_add_cb(noti_ex_manager_h handle, noti_ex_event_info_h info,
		noti_ex_item_h *added_item, int cnt, void *data) {
}

void _manager_events_update_cb(noti_ex_manager_h handle, noti_ex_event_info_h info,
		noti_ex_item_h updated_item, void *data) {
	int request_id;
	// if something wrong about process
	noti_ex_manager_send_error(handle, info, NOTI_EX_ERROR_IO_ERROR, &request_id);
}

void _manager_events_delete_cb(noti_ex_manager_h handle, noti_ex_event_info_h info,
		noti_ex_item_h deleted_item, void *data) {
}

void _manager_events_error_cb(noti_ex_manager_h handle, noti_ex_error_e error,
		int req_id, void *data) {
}

void _manager_events_request_cb(noti_ex_manager_h handle, noti_ex_event_info_h info,
		noti_ex_item_h **items, int *cnt, void *data) {
}

{
	int ret;
	noti_ex_manager_h handle;
	noti_ex_manager_events_s ev;

	ev.added = _manager_events_add_cb;
	ev.updated = _manager_events_update_cb;
	ev.deleted = _manager_events_delete_cb;
	ev.error = _manager_events_error_cb;
	ev.requested = _manager_events_request_cb;
	ret = noti_ex_manager_create(&handle, RECEIVERGROUP_PANEL, ev, NULL);
}
 * @endcode
 */
int noti_ex_manager_send_error(noti_ex_manager_h handle,
		noti_ex_event_info_h info, noti_ex_error_e error, int *request_id);

/**
 * @brief Gets the count of notifications for the manager.
 * @since_tizen 5.5
 * @privlevel public
 * @privilege %http://tizen.org/privilege/notification
 * @param[out] handle The notification manager handle
 * @param[in] receiver_group The notification group name of manager
 * @param[in] event_callbacks The callbacks for notification_ex events
 * @param[in] data The user data for @a event_callbacks
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #NOTI_EX_ERROR_PERMISSION_DENIED Permission deny
 * @see #noti_ex_manager_h
 * @see #RECEIVERGROUP_PANEL
 * @see #RECEIVERGROUP_TICKER
 * @see #RECEIVERGROUP_LOCKSCREEN
 * @see #RECEIVERGROUP_INDICATOR
 * @see #RECEIVERGROUP_POPUP
 * @par Sample code:
 * @code
#include <notification_ex_manager.h>

void _manager_events_error_cb(noti_ex_manager_h handle, noti_ex_error_e error,
		int req_id, void *data) {
}

{
	int ret;
	noti_ex_manager_h handle;
	noti_ex_manager_events_s ev;
	int cnt;

	ev.error = _manager_events_error_cb;
	ret = noti_ex_manager_create(&handle, RECEIVERGROUP_PANEL, ev, NULL);
	ret = noti_ex_manager_get_count(handle, &cnt);
}
 * @endcode
 */
int noti_ex_manager_get_count(noti_ex_manager_h handle, int *cnt);

#ifdef __cplusplus
}
#endif
#endif  // CAPI_NOTIFICATION_EX_MANAGER_H_
