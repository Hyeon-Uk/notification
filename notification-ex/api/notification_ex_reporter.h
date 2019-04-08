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

#ifndef CAPI_NOTIFICATION_EX_REPORTER_H_
#define CAPI_NOTIFICATION_EX_REPORTER_H_

#include <notification_ex_error.h>
#include <notification_ex_item.h>
#include <notification_ex_event_info.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief The notification_ex reporter handle.
 * @since_tizen 5.5
 */
typedef void* noti_ex_reporter_h;

/**
 * @brief Called when the error event is receved.
 * @since_tizen 5.5
 * @param[in] handle The reporter handle
 *            The @a handle can be used only in the callback.
 *            The @a handle will be freed right after callback finished.
 * @param[in] error The error type
 * @param[in] request_id The id of the request that occurred error
 * @param[in] data The user data which was registered with callback
 *            The @a data can be used only in the callback. To use outside, make a copy.
 * @see #noti_ex_reporter_h
 * @see #noti_ex_error_e
 */
typedef void (*noti_ex_reporter_events_error_cb)(noti_ex_reporter_h handle,
		noti_ex_error_e err, int request_id, void *data);

/**
 * @brief Called when the notification event is receved.
 * @since_tizen 5.5
 * @param[in] handle The reporter handle
 *            The @a handle can be used only in the callback.
 *            The @a handle will be freed right after callback finished.
 * @param[in] info The handle that contains event information
 *            The @a info can be used only in the callback. To use outside, make a copy.
 *            The @a info will be freed right after callback finished.
 * @param[in] items The target items of event
 *            The @a items can be used only in the callback.
 *            The @a items will be freed right after callback finished.
 * @param[in] count The count of @a items
 * @param[in] data The user data which was registered with callback
 *            The @a data can be used only in the callback. To use outside, make a copy.
 * @see #noti_ex_reporter_h
 * @see #noti_ex_event_info_h
 * @see #noti_ex_item_h
 * @see #noti_ex_event_info_clone
 */
typedef void (*noti_ex_reporter_events_event_cb)(noti_ex_reporter_h handle,
		noti_ex_event_info_h info, const noti_ex_item_h *items, int count,
		void *data);

/**
 * @brief Called when the notification request event is receved.
 * @since_tizen 5.5
 * @param[in] handle The reporter handle
 *            The @a handle can be used only in the callback.
 *            The @a handle will be freed right after callback finished.
 * @param[in] info The handle that contains event information
 *            The @a info can be used only in the callback. To use outside, make a copy.
 *            The @a info will be freed right after callback finished.
 * @param[out] items The item handle to be responded to the requester
 * @param[out] cnt The count of items which will be responded to the requester.
 * @param[in] data The user data which was registered with callback
 *            The @a data can be used only in the callback. To use outside, make a copy.
 * @see #noti_ex_reporter_h
 * @see #noti_ex_event_info_h
 * @see #noti_ex_item_h
 * @see #noti_ex_event_info_clone
 */
typedef void (*noti_ex_reporter_events_request_cb)(noti_ex_reporter_h handle,
		noti_ex_event_info_h info, noti_ex_item_h **items, int *cnt,
		void *data);

/**
 * @brief The structure type to contain the set of callback functions for handling the notification events.
 * @since_tizen 5.5
 *
 * @see noti_ex_reporter_events_request_cb()
 * @see noti_ex_reporter_events_event_cb()
 * @see noti_ex_reporter_events_error_cb()
 */
typedef struct {
	noti_ex_reporter_events_event_cb event;
	noti_ex_reporter_events_request_cb requested;
	noti_ex_reporter_events_error_cb error;
} noti_ex_reporter_events_s;

/**
 * @brief Creates the notification_ex reporter handle.
 * @details The reporter handle is created with a events callback and
 *          the user can listen notification events through the events callback.
 * @since_tizen 5.5
 * @privlevel public
 * @privilege %http://tizen.org/privilege/notification
 * @remarks @a handle must be released using noti_ex_reporter_destroy().
 * @param[out] handle The notification reporter handle
 * @param[in] event_callbacks The callbacks for notification_ex events
 * @param[in] data The user data for @a event_callbacks
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #NOTI_EX_ERROR_PERMISSION_DENIED Permission deny
 * @retval #NOTI_EX_ERROR_OUT_OF_MEMORY Out of memory
 * @see #noti_ex_item_h
 * @see #noti_ex_reporter_events_s
 * @par Sample code:
 * @code
#include <notification_ex_reporter.h>

void _reporter_events_event_cb(noti_ex_reporter_h handle, noti_ex_event_info_h info,
		noti_ex_item_h *items, int cnt, void *data) {
}

void _reporter_events_error_cb(noti_ex_reporter_h handle,
		noti_ex_error_e error, int req_id, void *data) {
}

void _reporter_events_request_cb(noti_ex_reporter_h handle,
		noti_ex_event_info_h info, noti_ex_item_h **items, int *cnt, void *data) {
}

{
	int ret;
	noti_ex_reporter_h handle;
	noti_ex_reporter_events_s ev;

	ev.requested = _reporter_events_request_cb;
	ev.event = _reporter_events_event_cb;
	ev.error = _reporter_events_error_cb;
	ret = noti_ex_reporter_create(&handle, ev, NULL);
}
 * @endcode
 */
int noti_ex_reporter_create(noti_ex_reporter_h *handle,
		noti_ex_reporter_events_s event_callbacks, void *data);

/**
 * @brief Destroys the notification_ex reporter handle.
 * @since_tizen 5.5
 * @privlevel public
 * @param[in] handle The notification reporter handle
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_reporter_create
 * @par Sample code:
 * @code
#include <notification_ex_reporter.h>

void _reporter_events_error_cb(noti_ex_reporter_h handle, noti_ex_error_e error,
		int req_id, void *data) {
}

{
	int ret;
	noti_ex_reporter_h handle;
	noti_ex_reporter_events_s ev;

	ev.error = _reporter_events_error_cb;
	ret = noti_ex_reporter_create(&handle, ev, NULL);
	ret = noti_ex_reporter_destroy(handle);
}
 * @endcode
 */
int noti_ex_reporter_destroy(noti_ex_reporter_h handle);

/**
 * @brief Sends error.
 * @since_tizen 5.5
 * @privlevel public
 * @privilege %http://tizen.org/privilege/notification
 * @param[out] handle The notification reporter handle
 * @param[in] receiver_group The notification group name of reporter
 * @param[in] info The handle that contains event information
 * @param[in] error The error type
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #NOTI_EX_ERROR_PERMISSION_DENIED Permission deny
 * @retval #NOTI_EX_ERROR_IO_ERROR IO error
 * @see #noti_ex_event_info_h
 * @see #noti_ex_manager_events_s
 * @see #noti_ex_error_e
 * @par Sample code:
 * @code
#include <notification_ex_reporter.h>

void _reporter_events_event_cb(noti_ex_reporter_h handle, noti_ex_event_info_h info,
		noti_ex_item_h *items, int cnt, void *data) {

	// if something wrong about process
	noti_ex_reporter_send_error(handle, info, NOTI_EX_ERROR_IO_ERROR);
}

void _reporter_events_error_cb(noti_ex_reporter_h handle,
		noti_ex_error_e error, int req_id, void *data) {
}

void _reporter_events_request_cb(noti_ex_reporter_h handle,
		noti_ex_event_info_h info, noti_ex_item_h **items, int *cnt, void *data) {
}

{
	int ret;
	noti_ex_reporter_h handle;
	noti_ex_reporter_events_s ev;
	int request_id;

	ev.requested = _reporter_events_request_cb;
	ev.event = _reporter_events_event_cb;
	ev.error = _reporter_events_error_cb;
	ret = noti_ex_reporter_create(&handle, ev, NULL, &request_id);
}
 * @endcode
 */
int noti_ex_reporter_send_error(noti_ex_reporter_h handle,
		noti_ex_event_info_h info, noti_ex_error_e error);

/**
 * @brief Posts notification.
 * @since_tizen 5.5
 * @privlevel public
 * @param[in] handle The notification reporter handle
 * @param[in] noti The notification handle to be posted
 * @param[out] request_id The id of request
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #NOTI_EX_ERROR_IO_ERROR IO error
 * @see #noti_ex_reporter_create
 * @par Sample code:
 * @code
#include <notification_ex_reporter.h>

void _reporter_events_error_cb(noti_ex_reporter_h handle, noti_ex_error_e error,
		int req_id, void *data) {
}

{
	int ret;
	noti_ex_reporter_h handle;
	noti_ex_reporter_events_s ev;
	noti_ex_item_h group_item = NULL;
	noti_ex_item_h child_item = NULL;
	int request_id;

	ev.error = _reporter_events_error_cb;
	ret = noti_ex_reporter_create(&handle, ev, NULL);
	ret = noti_ex_item_group_create(&group_item, "group_id");
	ret = noti_ex_item_button_create(&child_item, "button_id", "button_title");
	ret = noti_ex_item_group_add_child(group_item, child_item);
	ret = noti_ex_reporter_post(handle, group_item, &request_id);
}
 * @endcode
 */
int noti_ex_reporter_post(noti_ex_reporter_h handle, noti_ex_item_h noti,
		int *request_id);

/**
 * @brief Posts notification list.
 * @since_tizen 5.5
 * @privlevel public
 * @param[in] handle The notification reporter handle
 * @param[in] noti_list The notification handle list to be posted
 * @param[in] count The count of notifications in @a noti_list
 * @param[out] request_id The id of request
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #NOTI_EX_ERROR_IO_ERROR IO error
 * @see #noti_ex_reporter_create
 * @par Sample code:
 * @code
#include <notification_ex_reporter.h>

void _reporter_events_error_cb(noti_ex_reporter_h handle, noti_ex_error_e error,
		int req_id, void *data) {
}

{
	int ret;
	noti_ex_reporter_h handle;
	noti_ex_reporter_events_s ev;
	noti_ex_item_h group_item = NULL;
	noti_ex_item_h child_item = NULL;
	noti_ex_item_h *noti_list = (noti_ex_item_h *)calloc(2, sizeof(noti_ex_item_h));
	int request_id;

	ev.error = _reporter_events_error_cb;
	ret = noti_ex_reporter_create(&handle, ev, NULL);
	ret = noti_ex_item_group_create(&noti_list[0], "group_id1");
	ret = noti_ex_item_button_create(&child_item, "button_id1", "button_title");
	ret = noti_ex_item_group_add_child(noti_list[0], child_item);

	ret = noti_ex_item_group_create(&noti_list[1], "group_id2");
	ret = noti_ex_item_button_create(&child_item, "button_id2", "button_title");
	ret = noti_ex_item_group_add_child(noti_list[1], child_item);

	ret = noti_ex_reporter_post_list(handle, noti_list, 2, &request_id);
}
 * @endcode
 */
int noti_ex_reporter_post_list(noti_ex_reporter_h handle, noti_ex_item_h *noti_list, int count,
		int *request_id);

/**
 * @brief Updates notification.
 * @since_tizen 5.5
 * @privlevel public
 * @param[in] handle The notification reporter handle
 * @param[in] noti The notification handle to be updated
 * @param[out] request_id The id of request
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #NOTI_EX_ERROR_IO_ERROR IO error
 * @see #noti_ex_reporter_create
 * @par Sample code:
 * @code
#include <notification_ex_reporter.h>

void _reporter_events_error_cb(noti_ex_reporter_h handle, noti_ex_error_e error,
		int req_id, void *data) {
}

{
	int ret;
	noti_ex_reporter_h handle;
	noti_ex_reporter_events_s ev;
	noti_ex_item_h group_item = NULL;
	noti_ex_item_h child_item = NULL;
	int request_id;

	ev.error = _reporter_events_error_cb;
	ret = noti_ex_reporter_create(&handle, ev, NULL);
	ret = noti_ex_reporter_find_by_root_id(handle, "group_id", &group_item);
	ret = noti_ex_item_group_set_direction(group_item, false);
	ret = noti_ex_reporter_update(handle, group_item, &request_id);
}
 * @endcode
 */
int noti_ex_reporter_update(noti_ex_reporter_h handle, noti_ex_item_h noti,
		int *request_id);

/**
 * @brief Deletes notification.
 * @since_tizen 5.5
 * @privlevel public
 * @param[in] handle The notification reporter handle
 * @param[in] noti The notification handle to be updated
 * @param[out] request_id The id of request
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #NOTI_EX_ERROR_IO_ERROR IO error
 * @see #noti_ex_reporter_create
 * @par Sample code:
 * @code
#include <notification_ex_reporter.h>

void _reporter_events_error_cb(noti_ex_reporter_h handle, noti_ex_error_e error,
		int req_id, void *data) {
}

{
	int ret;
	noti_ex_reporter_h handle;
	noti_ex_reporter_events_s ev;
	noti_ex_item_h group_item = NULL;
	noti_ex_item_h child_item = NULL;
	int request_id;

	ev.error = _reporter_events_error_cb;
	ret = noti_ex_reporter_create(&handle, ev, NULL);
	ret = noti_ex_reporter_find_by_root_id(handle, "group_id", &group_item);
	ret = noti_ex_reporter_delete(handle, group_item, &request_id);
}
 * @endcode
 */
int noti_ex_reporter_delete(noti_ex_reporter_h handle, noti_ex_item_h noti,
		int *request_id);

/**
 * @brief Deletes all notifications.
 * @since_tizen 5.5
 * @privlevel public
 * @param[in] handle The notification reporter handle
 * @param[out] request_id The id of request
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #NOTI_EX_ERROR_IO_ERROR IO error
 * @see #noti_ex_reporter_create
 * @par Sample code:
 * @code
#include <notification_ex_reporter.h>

void _reporter_events_error_cb(noti_ex_reporter_h handle, noti_ex_error_e error,
		int req_id, void *data) {
}

{
	int ret;
	noti_ex_reporter_h handle;
	noti_ex_reporter_events_s ev;
	noti_ex_item_h group_item = NULL;
	noti_ex_item_h child_item = NULL;
	int request_id;

	ev.error = _reporter_events_error_cb;
	ret = noti_ex_reporter_create(&handle, ev, NULL);
	ret = noti_ex_reporter_delete_all(handle, &request_id);
}
 * @endcode
 */
int noti_ex_reporter_delete_all(noti_ex_reporter_h handle,
		int *request_id);

/**
 * @brief Finds notification by root id.
 * @since_tizen 5.5
 * @privlevel public
 * @param[in] handle The notification reporter handle
 * @param[in] root_id The id of the foremost notification item to be retrived
 * @param[out] item The result of the find operation
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #NOTI_EX_ERROR_IO_ERROR IO error
 * @see #noti_ex_reporter_create
 * @par Sample code:
 * @code
#include <notification_ex_reporter.h>

void _reporter_events_error_cb(noti_ex_reporter_h handle, noti_ex_error_e error,
		int req_id, void *data) {
}

{
	int ret;
	noti_ex_reporter_h handle;
	noti_ex_reporter_events_s ev;
	noti_ex_item_h group_item = NULL;
	noti_ex_item_h child_item = NULL;

	ev.error = _reporter_events_error_cb;
	ret = noti_ex_reporter_create(&handle, ev, NULL);
	ret = noti_ex_reporter_find_by_root_id(handle, "group_id", &group_item);
	ret = noti_ex_reporter_delete(handle, group_item);
}
 * @endcode
 */
int noti_ex_reporter_find_by_root_id(noti_ex_reporter_h handle, const char *root_id,
		noti_ex_item_h *item);

#ifdef __cplusplus
}
#endif
#endif  // CAPI_NOTIFICATION_EX_REPORTER_H_
