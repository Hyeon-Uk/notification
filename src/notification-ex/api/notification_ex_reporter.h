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

#ifndef __TIZEN_APPFW_NOTIFICATION_EX_REPORTER_H__
#define __TIZEN_APPFW_NOTIFICATION_EX_REPORTER_H__

#include <notification_ex_error.h>
#include <notification_ex_item.h>
#include <notification_ex_event_info.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup NOTIFICATION_EX_MODULE
 * @{
 */

/**
 * @deprecated Deprecated since 11.0.
 * @brief The notification_ex reporter handle.
 * @since_tizen 5.5
 */
typedef void *noti_ex_reporter_h;

/**
 * @deprecated Deprecated since 11.0.
 * @brief Called when the error event is received.
 * @details The errors are passed from noti_ex_manager_send_error().
 *          If this callback is set to NULL in @a event_callbacks, error events cannot be received.
 * @since_tizen 5.5
 * @param[in] handle The reporter handle \n
 *            The @a handle is the same handle for which the callback was set
 * @param[in] error The error type
 * @param[in] request_id The id of the request that occurred error
 * @param[in] user_data The user data which was registered with callback
 * @see #noti_ex_reporter_h
 * @see #noti_ex_error_e
 */
typedef void (*noti_ex_reporter_events_error_cb)(noti_ex_reporter_h handle,
		noti_ex_error_e error, int request_id, void *user_data);

/**
 * @deprecated Deprecated since 11.0.
 * @brief Called when the notification event is received.
 * @details If this callback is set to NULL in @a event_callbacks, notification events cannot be received.
 * @since_tizen 5.5
 * @param[in] handle The reporter handle \n
 *            The @a handle is the same handle for which the callback was set
 * @param[in] info The handle that contains event information \n
 *            The @a info can be used only in the callback. To use outside, make a copy. \n
 *            The @a info will be freed after the callback exits.
 * @param[in] items The target items of event \n
 *            Each item in @a items must be released using noti_ex_item_destroy(), then @a items must be released using free().
 * @param[in] count The count of @a items
 * @param[in] user_data The user data which was registered with callback
 * @see #noti_ex_reporter_h
 * @see #noti_ex_event_info_h
 * @see #noti_ex_item_h
 * @see noti_ex_event_info_clone()
 * @see noti_ex_item_destroy()
 */
typedef void (*noti_ex_reporter_events_event_cb)(noti_ex_reporter_h handle,
		noti_ex_event_info_h info, noti_ex_item_h *items, int count,
		void *user_data);

/**
 * @deprecated Deprecated since 11.0.
 * @brief The structure type to contain the set of callback functions for handling the notification events.
 * @since_tizen 5.5
 *
 * @see noti_ex_reporter_events_event_cb()
 * @see noti_ex_reporter_events_error_cb()
 */
typedef struct {
	noti_ex_reporter_events_event_cb event; /**< Called when a notification event is received */
	noti_ex_reporter_events_error_cb error; /**< Called when an error event is received */
} noti_ex_reporter_events_s;

/**
 * @deprecated Deprecated since 11.0.
 * @brief Creates the notification_ex reporter handle.
 * @details The reporter handle is created with event callbacks and
 *          the user can listen notification events through the event callbacks.
 *          Callbacks in @a event_callbacks can be set to NULL, which will result in the corresponding events not being received.
 * @since_tizen 5.5
 * @privlevel public
 * @privilege %http://tizen.org/privilege/notification
 * @remarks @a handle must be released using noti_ex_reporter_destroy().
 * @param[out] handle The notification reporter handle
 * @param[in] event_callbacks The callbacks for notification events
 * @param[in] user_data The user data for @a event_callbacks
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_PERMISSION_DENIED Permission deny
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #NOTI_EX_ERROR_OUT_OF_MEMORY Out of memory
 * @retval #NOTI_EX_ERROR_IO_ERROR IO error
 * @see #noti_ex_item_h
 * @see #noti_ex_reporter_events_s
 * @see noti_ex_reporter_destroy()
 * @par Sample code:
 * @code
#include <notification_ex.h>

void _reporter_events_event_cb(noti_ex_reporter_h handle, noti_ex_event_info_h info,
		noti_ex_item_h *items, int cnt, void *user_data) {
}

void _reporter_events_error_cb(noti_ex_reporter_h handle,
		noti_ex_error_e error, int req_id, void *user_data) {
}

{
	int ret;
	noti_ex_reporter_h handle;
	noti_ex_reporter_events_s ev;

	ev.event = _reporter_events_event_cb;
	ev.error = _reporter_events_error_cb;
	ret = noti_ex_reporter_create(&handle, ev, NULL);
}
 * @endcode
 */
int noti_ex_reporter_create(noti_ex_reporter_h *handle,
		noti_ex_reporter_events_s event_callbacks, void *user_data) TIZEN_DEPRECATED_API;

/**
 * @deprecated Deprecated since 11.0.
 * @brief Destroys the notification_ex reporter handle.
 * @since_tizen 5.5
 * @param[in] handle The notification reporter handle
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see noti_ex_reporter_create()
 * @par Sample code:
 * @code
#include <notification_ex.h>

void _reporter_events_error_cb(noti_ex_reporter_h handle, noti_ex_error_e error,
		int req_id, void *user_data) {
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
int noti_ex_reporter_destroy(noti_ex_reporter_h handle) TIZEN_DEPRECATED_API;

/**
 * @deprecated Deprecated since 11.0.
 * @brief Sends error to the manager.
 * @since_tizen 5.5
 * @privlevel public
 * @privilege %http://tizen.org/privilege/notification
 * @param[out] handle The notification reporter handle
 * @param[in] info The handle that contains event information
 * @param[in] error The error type
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_PERMISSION_DENIED Permission deny
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #NOTI_EX_ERROR_IO_ERROR IO error
 * @see #noti_ex_event_info_h
 * @see #noti_ex_manager_events_s
 * @see #noti_ex_error_e
 * @par Sample code:
 * @code
#include <notification_ex.h>

void _reporter_events_event_cb(noti_ex_reporter_h handle, noti_ex_event_info_h info,
		noti_ex_item_h *items, int cnt, void *user_data) {

	// if something wrong about process
	noti_ex_reporter_send_error(handle, info, NOTI_EX_ERROR_IO_ERROR);
}

void _reporter_events_error_cb(noti_ex_reporter_h handle,
		noti_ex_error_e error, int req_id, void *user_data) {
}

{
	int ret;
	noti_ex_reporter_h handle;
	noti_ex_reporter_events_s ev;

	ev.event = _reporter_events_event_cb;
	ev.error = _reporter_events_error_cb;
	ret = noti_ex_reporter_create(&handle, ev, NULL);
}
 * @endcode
 */
int noti_ex_reporter_send_error(noti_ex_reporter_h handle,
		noti_ex_event_info_h info, noti_ex_error_e error) TIZEN_DEPRECATED_API;

/**
 * @deprecated Deprecated since 11.0.
 * @brief Posts a notification.
 * @since_tizen 5.5
 * @privlevel public
 * @privilege %http://tizen.org/privilege/notification
 * @param[in] handle The notification reporter handle
 * @param[in] noti The notification handle to be posted
 * @param[out] request_id The id of request
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_PERMISSION_DENIED Permission deny
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #NOTI_EX_ERROR_IO_ERROR IO error
 * @see noti_ex_reporter_create()
 * @par Sample code:
 * @code
#include <notification_ex.h>

void _reporter_events_error_cb(noti_ex_reporter_h handle, noti_ex_error_e error,
		int req_id, void *user_data) {
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
	noti_ex_item_destroy(child_item);

	ret = noti_ex_reporter_post(handle, group_item, &request_id);
	noti_ex_item_destroy(group_item);
}
 * @endcode
 */
int noti_ex_reporter_post(noti_ex_reporter_h handle, noti_ex_item_h noti,
		int *request_id) TIZEN_DEPRECATED_API;

/**
 * @deprecated Deprecated since 11.0.
 * @brief Posts notifications contained in a list.
 * @since_tizen 5.5
 * @privlevel public
 * @privilege %http://tizen.org/privilege/notification
 * @param[in] handle The notification reporter handle
 * @param[in] noti_list The notification handle list to be posted
 * @param[in] count The count of notifications in @a noti_list
 * @param[out] request_id The id of request
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_PERMISSION_DENIED Permission deny
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #NOTI_EX_ERROR_IO_ERROR IO error
 * @see noti_ex_reporter_create()
 * @par Sample code:
 * @code
#include <notification_ex.h>

void _reporter_events_error_cb(noti_ex_reporter_h handle, noti_ex_error_e error,
		int req_id, void *user_data) {
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
	noti_ex_item_destroy(child_item);

	ret = noti_ex_item_group_create(&noti_list[1], "group_id2");
	ret = noti_ex_item_button_create(&child_item, "button_id2", "button_title");
	ret = noti_ex_item_group_add_child(noti_list[1], child_item);
	noti_ex_item_destroy(child_item);

	ret = noti_ex_reporter_post_list(handle, noti_list, 2, &request_id);
	noti_ex_item_destroy(noti_list[0]);
	noti_ex_item_destroy(noti_list[1]);

	free(noti_list);
}
 * @endcode
 */
int noti_ex_reporter_post_list(noti_ex_reporter_h handle, noti_ex_item_h *noti_list, int count,
		int *request_id) TIZEN_DEPRECATED_API;

/**
 * @deprecated Deprecated since 11.0.
 * @brief Updates a notification.
 * @since_tizen 5.5
 * @privlevel public
 * @privilege %http://tizen.org/privilege/notification
 * @param[in] handle The notification reporter handle
 * @param[in] noti The notification handle to be updated
 * @param[out] request_id The id of request
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_PERMISSION_DENIED Permission deny
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #NOTI_EX_ERROR_IO_ERROR IO error
 * @see noti_ex_reporter_create()
 * @par Sample code:
 * @code
#include <notification_ex.h>

void _reporter_events_error_cb(noti_ex_reporter_h handle, noti_ex_error_e error,
		int req_id, void *user_data) {
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
		int *request_id) TIZEN_DEPRECATED_API;

/**
 * @deprecated Deprecated since 11.0.
 * @brief Deletes a notification.
 * @since_tizen 5.5
 * @privlevel public
 * @privilege %http://tizen.org/privilege/notification
 * @param[in] handle The notification reporter handle
 * @param[in] noti The notification handle to be updated
 * @param[out] request_id The id of request
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_PERMISSION_DENIED Permission deny
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #NOTI_EX_ERROR_IO_ERROR IO error
 * @see noti_ex_reporter_create()
 * @par Sample code:
 * @code
#include <notification_ex.h>

void _reporter_events_error_cb(noti_ex_reporter_h handle, noti_ex_error_e error,
		int req_id, void *user_data) {
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
		int *request_id) TIZEN_DEPRECATED_API;

/**
 * @deprecated Deprecated since 11.0.
 * @brief Deletes all notifications.
 * @since_tizen 5.5
 * @privlevel public
 * @privilege %http://tizen.org/privilege/notification
 * @param[in] handle The notification reporter handle
 * @param[out] request_id The id of request
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_PERMISSION_DENIED Permission deny
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #NOTI_EX_ERROR_IO_ERROR IO error
 * @see noti_ex_reporter_create()
 * @par Sample code:
 * @code
#include <notification_ex.h>

void _reporter_events_error_cb(noti_ex_reporter_h handle, noti_ex_error_e error,
		int req_id, void *user_data) {
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
		int *request_id) TIZEN_DEPRECATED_API;

/**
 * @deprecated Deprecated since 11.0.
 * @brief Finds notification by root id.
 * @since_tizen 5.5
 * @privlevel public
 * @privilege %http://tizen.org/privilege/notification
 * @param[in] handle The notification reporter handle
 * @param[in] root_id The id of the foremost notification item to be retrieved
 * @param[out] item The result of the find operation \n
 *             The @a item must be released using noti_ex_item_destroy().
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_PERMISSION_DENIED Permission deny
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #NOTI_EX_ERROR_IO_ERROR IO error
 * @see noti_ex_reporter_create()
 * @see noti_ex_item_destroy()
 * @par Sample code:
 * @code
#include <notification_ex.h>

void _reporter_events_error_cb(noti_ex_reporter_h handle, noti_ex_error_e error,
		int req_id, void *user_data) {
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
int noti_ex_reporter_find_by_root_id(noti_ex_reporter_h handle, const char *root_id,
		noti_ex_item_h *item) TIZEN_DEPRECATED_API;

/**
 * @}
 */

#ifdef __cplusplus
}
#endif
#endif  /* __TIZEN_APPFW_NOTIFICATION_EX_REPORTER_H__ */
