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

#ifndef __TIZEN_APPFW_NOTIFICATION_EX_MANAGER_H__
#define __TIZEN_APPFW_NOTIFICATION_EX_MANAGER_H__

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
 * @brief The notification_ex manager handle.
 * @since_tizen 5.5
 */
typedef void *noti_ex_manager_h;

/**
 * @deprecated Deprecated since 11.0.
 * @brief Called when a notification addition event is received.
 * @details If this callback is set to NULL in @a event_callbacks, addition events cannot be received.
 * @since_tizen 5.5
 * @param[in] handle The manager handle \n
 *            The @a handle is the same handle for which the callback was set
 * @param[in] info The handle that contains event information \n
 *            The @a info can be used only in the callback. To use outside, make a copy. \n
 *            The @a info will be freed after the callback exits.
 * @param[in] added_items The item handles to be added \n
 *            Each item in @a added_items should be destroyed with using noti_ex_item_destroy(), then @a added_items should be released with free().
 * @param[in] count The count of the item to be added
 * @param[in] user_data The user data which was registered with callback
 * @see #noti_ex_manager_h
 * @see #noti_ex_event_info_h
 * @see #noti_ex_item_h
 * @see noti_ex_event_info_clone()
 * @see noti_ex_item_destroy()
 */
typedef void (*noti_ex_manager_events_add_cb)(noti_ex_manager_h handle,
		noti_ex_event_info_h info, noti_ex_item_h *added_items,
		int count, void *user_data);

/**
 * @deprecated Deprecated since 11.0.
 * @brief Called when a notification update event is received.
 * @details If this callback is set to NULL in @a event_callbacks, update events cannot be received.
 * @since_tizen 5.5
 * @param[in] handle The manager handle \n
 *            The @a handle is the same handle for which the callback was set
 * @param[in] info The handle that contains event information \n
 *            The @a info can be used only in the callback. To use outside, make a copy. \n
 *            The @a info will be freed after the callback exits.
 * @param[in] updated_item The item handle to be updated \n
 *            The @a updated_item must be released using noti_ex_item_destroy().
 * @param[in] user_data The user data which was registered with callback
 * @see #noti_ex_manager_h
 * @see #noti_ex_event_info_h
 * @see #noti_ex_item_h
 * @see noti_ex_event_info_clone()
 * @see noti_ex_item_destroy()
 */
typedef void (*noti_ex_manager_events_update_cb)(noti_ex_manager_h handle,
		noti_ex_event_info_h info, noti_ex_item_h updated_item,
		void *user_data);

/**
 * @deprecated Deprecated since 11.0.
 * @brief Called when a notification deletion event is received.
 * @details If this callback is set to NULL in @a event_callbacks, deletion events cannot be received.
 * @since_tizen 5.5
 * @param[in] handle The manager handle \n
 *            The @a handle is the same handle for which the callback was set
 * @param[in] info The handle that contains event information \n
 *            The @a info can be used only in the callback. To use outside, make a copy. \n
 *            The @a info will be freed after the callback exits.
 * @param[in] deleted_item The item handle to be deleted \n
 *            The @a deleted_item must be released using noti_ex_item_destroy().
 * @param[in] user_data The user data which was registered with callback
 * @see #noti_ex_manager_h
 * @see #noti_ex_event_info_h
 * @see #noti_ex_item_h
 * @see noti_ex_event_info_clone()
 * @see noti_ex_item_destroy()
 */
typedef void (*noti_ex_manager_events_delete_cb)(noti_ex_manager_h handle,
		noti_ex_event_info_h info, noti_ex_item_h deleted_item,
		void *user_data);

/**
 * @deprecated Deprecated since 11.0.
 * @brief Called when an error event is received.
 * @details The errors are passed from noti_ex_reporter_send_error().
 *          If this callback is set to NULL in @a event_callbacks, error events cannot be received.
 * @since_tizen 5.5
 * @param[in] handle The manager handle \n
 *            The @a handle is the same handle for which the callback was set
 * @param[in] error The error type
 * @param[in] request_id The id of the request that occurred error
 * @param[in] user_data The user data which was registered with callback
 * @see #noti_ex_manager_h
 * @see #noti_ex_error_e
 */
typedef void (*noti_ex_manager_events_error_cb)(noti_ex_manager_h handle,
		noti_ex_error_e error, int request_id, void *user_data);

/**
 * @deprecated Deprecated since 11.0.
 * @brief The structure type to contain the set of callback functions for handling the notification events.
 * @since_tizen 5.5
 *
 * @see noti_ex_manager_events_add_cb()
 * @see noti_ex_manager_events_update_cb()
 * @see noti_ex_manager_events_delete_cb()
 * @see noti_ex_manager_events_error_cb()
 */
typedef struct {
	noti_ex_manager_events_add_cb added; /**< Called when a notification addition event is received */
	noti_ex_manager_events_update_cb updated; /**< Called when a notification update event is received */
	noti_ex_manager_events_delete_cb deleted; /**< Called when a notification deletion event is received */
	noti_ex_manager_events_error_cb error; /**< Called when an error event is received */
} noti_ex_manager_events_s;

/**
 * @deprecated Deprecated since 11.0.
 * @partner
 * @brief Creates the notification_ex manager handle.
 * @details The manager handle is created with event callbacks and
 *          the user can listen to notification events through the event callbacks.
 *          If the user wants to manage notifications for specific group
 *          the user should identify it with @a receiver_group
 *          Callbacks in @a event_callbacks can be set to NULL, which will result in the corresponding events not being received.
 * @since_tizen 5.5
 * @privlevel partner
 * @privilege %http://tizen.org/privilege/notification.admin
 * @remarks @a handle must be released using noti_ex_manager_destroy().
 * @param[out] handle The notification manager handle
 * @param[in] receiver_group The notification group name of manager
 * @param[in] event_callbacks The callbacks for notification events
 * @param[in] user_data The user data for @a event_callbacks
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_PERMISSION_DENIED Permission deny
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #NOTI_EX_ERROR_OUT_OF_MEMORY Out of memory
 * @retval #NOTI_EX_ERROR_IO_ERROR IO error
 * @see #noti_ex_manager_h
 * @see #noti_ex_manager_events_s
 * @see #NOTI_EX_RECEIVER_GROUP_TICKER
 * @see #NOTI_EX_RECEIVER_GROUP_LOCKSCREEN
 * @see #NOTI_EX_RECEIVER_GROUP_INDICATOR
 * @see #NOTI_EX_RECEIVER_GROUP_POPUP
 * @see noti_ex_manager_destroy()
 * @par Sample code:
 * @code
#include <notification_ex.h>

void _manager_events_add_cb(noti_ex_manager_h handle, noti_ex_event_info_h info,
		noti_ex_item_h *added_item, int cnt, void *user_data) {
}

void _manager_events_update_cb(noti_ex_manager_h handle,
		noti_ex_event_info_h info, noti_ex_item_h updated_item, void *user_data) {
}

void _manager_events_delete_cb(noti_ex_manager_h handle,
		noti_ex_event_info_h info, noti_ex_item_h deleted_item, void *user_data) {
}

void _manager_events_error_cb(noti_ex_manager_h handle,
		noti_ex_error_e error, int req_id, void *user_data) {
}

{
	int ret;
	noti_ex_manager_h handle;
	noti_ex_manager_events_s ev;

	ev.added = _manager_events_add_cb;
	ev.updated = _manager_events_update_cb;
	ev.deleted = _manager_events_delete_cb;
	ev.error = _manager_events_error_cb;
	ret = noti_ex_manager_create(&handle, NOTI_EX_RECEIVER_GROUP_INDICATOR, ev, NULL);
}
 * @endcode
 */
int noti_ex_manager_create(noti_ex_manager_h *handle,
		const char *receiver_group, noti_ex_manager_events_s event_callbacks,
		void *user_data) TIZEN_DEPRECATED_API;

/**
 * @deprecated Deprecated since 11.0.
 * @brief Destroys the notification_ex manager handle.
 * @since_tizen 5.5
 * @param[in] handle The notification manager handle
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_item_h
 * @par Sample code:
 * @code
#include <notification_ex.h>

void _manager_events_error_cb(noti_ex_manager_h handle, noti_ex_error_e error,
		int req_id, void *user_data) {
}

{
	int ret;
	noti_ex_manager_h handle;
	noti_ex_manager_events_s ev;

	ev.error = _manager_events_error_cb;
	ret = noti_ex_manager_create(&handle, NOTI_EX_RECEIVER_GROUP_INDICATOR, ev, NULL);
	ret = noti_ex_manager_destroy(handle);
}
 * @endcode
 */
int noti_ex_manager_destroy(noti_ex_manager_h handle) TIZEN_DEPRECATED_API;

/**
 * @deprecated Deprecated since 11.0.
 * @partner
 * @brief Gets all notifications for the notification_ex manager.
 * @details Every notification that has been sent for the manager will be retrieved.
 * @since_tizen 5.5
 * @privlevel partner
 * @privilege %http://tizen.org/privilege/notification.admin
 * @param[in] handle The notification manager handle
 * @param[out] items The result of the get request \n
 *             Each item in @a items must be released using noti_ex_item_destroy(), then @a items must be released using free().
 * @param[out] count The count of retrieved notification items
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_PERMISSION_DENIED Permission deny
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #NOTI_EX_ERROR_IO_ERROR IO error
 * @see #noti_ex_item_h
 * @see #noti_ex_manager_h
 * @see noti_ex_item_destroy()
 * @par Sample code:
 * @code
#include <notification_ex.h>

void _manager_events_error_cb(noti_ex_manager_h handle, noti_ex_error_e error,
		int req_id, void *user_data) {
}

{
	int ret;
	noti_ex_manager_h handle;
	noti_ex_manager_events_s ev;
	noti_ex_item_h *items;
	noti_ex_item_h *item;
	int items_count;

	ev.error = _manager_events_error_cb;
	ret = noti_ex_manager_create(&handle, NOTI_EX_RECEIVER_GROUP_INDICATOR, ev, NULL);
	ret = noti_ex_manager_get(handle, &items, &items_count);
	for (int i = 0; i < items_count; i++) {
		item = items[i];
		// do something with item
		noti_ex_item_destroy(item);
	}

	free(items);
}
 * @endcode
 */
int noti_ex_manager_get(noti_ex_manager_h handle, noti_ex_item_h **items,
		int *count) TIZEN_DEPRECATED_API;

/**
 * @deprecated Deprecated since 11.0.
 * @partner
 * @brief Gets notifications with @a channel for the notification_ex manager.
 * @details Every notification that has been sent for the manager will be retrieved.
 * @since_tizen 5.5
 * @privlevel partner
 * @privilege %http://tizen.org/privilege/notification.admin
 * @param[in] handle The notification manager handle
 * @param[in] channel The channel of a notification_ex item
 * @param[out] items The result of the get request \n
 *             Each item in @a items must be released using noti_ex_item_destroy(), then @a items must be released using free().
 * @param[out] count The count of retrieved notification items
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_PERMISSION_DENIED Permission deny
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #NOTI_EX_ERROR_IO_ERROR IO error
 * @see #noti_ex_item_h
 * @see #noti_ex_manager_h
 * @see noti_ex_item_destroy()
 * @par Sample code:
 * @code
#include <notification_ex.h>

void _manager_events_error_cb(noti_ex_manager_h handle, noti_ex_error_e error,
		int req_id, void *user_data) {
}

{
	int ret;
	noti_ex_manager_h handle;
	noti_ex_manager_events_s ev;
	noti_ex_item_h *items;
	noti_ex_item_h *item;
	int items_count;
	char *channel = "channel";

	ev.error = _manager_events_error_cb;
	ret = noti_ex_manager_create(&handle, NOTI_EX_RECEIVER_GROUP_INDICATOR, ev, NULL);
	...
	ret = noti_ex_manager_get_by_channel(handle, channel, &items, &items_count);
	for (int i = 0; i < items_count; i++) {
		item = items[i];
		// do something with item
		noti_ex_item_destroy(item);
	}

	free(items);
}
 * @endcode
 */
int noti_ex_manager_get_by_channel(noti_ex_manager_h handle, char *channel,
		noti_ex_item_h **items, int *count) TIZEN_DEPRECATED_API;

/**
 * @deprecated Deprecated since 11.0.
 * @partner
 * @brief Updates notification.
 * @since_tizen 5.5
 * @privlevel partner
 * @privilege %http://tizen.org/privilege/notification.admin
 * @param[in] handle The notification manager handle
 * @param[in] item The notification item to be updated
 * @param[out] request_id The id of request
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_PERMISSION_DENIED Permission deny
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #NOTI_EX_ERROR_IO_ERROR IO error
 * @see #noti_ex_item_h
 * @see #noti_ex_manager_h
 * @par Sample code:
 * @code
#include <notification_ex.h>

void _manager_events_error_cb(noti_ex_manager_h handle, noti_ex_error_e error,
		int req_id, void *user_data) {
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
	ret = noti_ex_manager_create(&handle, NOTI_EX_RECEIVER_GROUP_INDICATOR, ev, NULL);
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
		int *request_id) TIZEN_DEPRECATED_API;

/**
 * @deprecated Deprecated since 11.0.
 * @partner
 * @brief Deletes notification.
 * @since_tizen 5.5
 * @privlevel partner
 * @privilege %http://tizen.org/privilege/notification.admin
 * @param[in] handle The notification manager handle
 * @param[in] noti The notification item to be deleted
 * @param[out] request_id The id of request
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_PERMISSION_DENIED Permission deny
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #NOTI_EX_ERROR_IO_ERROR IO error
 * @see #noti_ex_manager_h
 * @see #noti_ex_error_e
 * @par Sample code:
 * @code
#include <notification_ex.h>

void _manager_events_error_cb(noti_ex_manager_h handle, noti_ex_error_e error,
		int req_id, void *user_data) {
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
	ret = noti_ex_manager_create(&handle, NOTI_EX_RECEIVER_GROUP_INDICATOR, ev, NULL);
	ret = noti_ex_manager_get(handle, &items, &items_count);
	for (int i = 0; i < items_count; i++) {
		item = items[i];
		noti_ex_manager_delete(handle, item, &request_id);
	}
}
 * @endcode
 */
int noti_ex_manager_delete(noti_ex_manager_h handle, noti_ex_item_h noti,
		int *request_id) TIZEN_DEPRECATED_API;

/**
 * @deprecated Deprecated since 11.0.
 * @partner
 * @brief Deletes all notification.
 * @since_tizen 5.5
 * @privlevel partner
 * @privilege %http://tizen.org/privilege/notification.admin
 * @param[in] handle The notification manager handle
 * @param[out] request_id The id of request
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_PERMISSION_DENIED Permission deny
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #NOTI_EX_ERROR_IO_ERROR IO error
 * @see #noti_ex_manager_h
 * @see #noti_ex_error_e
 * @par Sample code:
 * @code
#include <notification_ex.h>

void _manager_events_error_cb(noti_ex_manager_h handle, noti_ex_error_e error,
		int req_id, void *user_data) {
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
	ret = noti_ex_manager_create(&handle, NOTI_EX_RECEIVER_GROUP_INDICATOR, ev, NULL);
	ret = noti_ex_manager_delete_all(handle, &request_id);
}
 * @endcode
 */
int noti_ex_manager_delete_all(noti_ex_manager_h handle,
		int *request_id) TIZEN_DEPRECATED_API;

/**
 * @deprecated Deprecated since 11.0.
 * @partner
 * @brief Hides notification.
 * @since_tizen 5.5
 * @privlevel partner
 * @privilege %http://tizen.org/privilege/notification.admin
 * @param[in] handle The notification manager handle
 * @param[in] item The notification item to be hidden
 * @param[out] request_id The id of request
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_PERMISSION_DENIED Permission deny
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #NOTI_EX_ERROR_IO_ERROR IO error
 * @see #noti_ex_item_h
 * @see #noti_ex_manager_h
 * @par Sample code:
 * @code
#include <notification_ex.h>

void _manager_events_error_cb(noti_ex_manager_h handle, noti_ex_error_e error,
		int req_id, void *user_data) {
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
	ret = noti_ex_manager_create(&handle, NOTI_EX_RECEIVER_GROUP_INDICATOR, ev, NULL);
	for (int i = 0; i < items_count; i++) {
		item = items[i];
		ret = noti_ex_manager_hide(handle, item, &request_id);
	}
}
 * @endcode
 */
int noti_ex_manager_hide(noti_ex_manager_h handle, noti_ex_item_h item,
		int *request_id) TIZEN_DEPRECATED_API;

/**
 * @deprecated Deprecated since 11.0.
 * @partner
 * @brief Finds notification by root id.
 * @since_tizen 5.5
 * @privlevel partner
 * @privilege %http://tizen.org/privilege/notification.admin
 * @remarks @a root_id is the id of the foremost notification item.
 * @param[in] handle The notification manager handle
 * @param[in] root_id The id of the foremost notification item to be retrieved
 * @param[out] item The result of the find operation \n
 *             The @a item must be released using noti_ex_item_destroy().
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_PERMISSION_DENIED Permission deny
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #NOTI_EX_ERROR_IO_ERROR IO error
 * @see #noti_ex_manager_h
 * @see #noti_ex_item_h
 * @see noti_ex_item_destroy()
 * @par Sample code:
 * @code
#include <notification_ex.h>

void _manager_events_error_cb(noti_ex_manager_h handle, noti_ex_error_e error,
		int req_id, void *user_data) {
}

{
	int ret;
	noti_ex_manager_h handle;
	noti_ex_manager_events_s ev;
	noti_ex_item_h item;

	ev.error = _manager_events_error_cb;
	ret = noti_ex_manager_create(&handle, NOTI_EX_RECEIVER_GROUP_INDICATOR, ev, NULL);
	ret = noti_ex_manager_find_by_root_id(handle, "group1", &item);
}
 * @endcode
 */
int noti_ex_manager_find_by_root_id(noti_ex_manager_h handle,
		const char *root_id, noti_ex_item_h *item) TIZEN_DEPRECATED_API;

/**
 * @deprecated Deprecated since 11.0.
 * @partner
 * @brief Sends error to the reporter.
 * @since_tizen 5.5
 * @privlevel partner
 * @privilege %http://tizen.org/privilege/notification.admin
 * @param[in] handle The notification manager handle
 * @param[in] info The handle that contains event information
 * @param[in] error The error type
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_PERMISSION_DENIED Permission deny
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #NOTI_EX_ERROR_IO_ERROR IO error
 * @see #noti_ex_event_info_h
 * @see #noti_ex_manager_h
 * @see #noti_ex_error_e
 * @par Sample code:
 * @code
#include <notification_ex.h>

void _manager_events_add_cb(noti_ex_manager_h handle, noti_ex_event_info_h info,
		noti_ex_item_h *added_item, int cnt, void *user_data) {
}

void _manager_events_update_cb(noti_ex_manager_h handle, noti_ex_event_info_h info,
		noti_ex_item_h updated_item, void *user_data) {
	// if something wrong about process
	noti_ex_manager_send_error(handle, info, NOTI_EX_ERROR_IO_ERROR);
}

void _manager_events_delete_cb(noti_ex_manager_h handle, noti_ex_event_info_h info,
		noti_ex_item_h deleted_item, void *user_data) {
}

void _manager_events_error_cb(noti_ex_manager_h handle, noti_ex_error_e error,
		int req_id, void *user_data) {
}

{
	int ret;
	noti_ex_manager_h handle;
	noti_ex_manager_events_s ev;

	ev.added = _manager_events_add_cb;
	ev.updated = _manager_events_update_cb;
	ev.deleted = _manager_events_delete_cb;
	ev.error = _manager_events_error_cb;
	ret = noti_ex_manager_create(&handle, NOTI_EX_RECEIVER_GROUP_INDICATOR, ev, NULL);
}
 * @endcode
 */
int noti_ex_manager_send_error(noti_ex_manager_h handle,
		noti_ex_event_info_h info, noti_ex_error_e error) TIZEN_DEPRECATED_API;

/**
 * @deprecated Deprecated since 11.0.
 * @partner
 * @brief Gets the count of notifications for the manager.
 * @since_tizen 5.5
 * @privlevel partner
 * @privilege %http://tizen.org/privilege/notification.admin
 * @param[in] handle The notification manager handle
 * @param[out] count The count of notifications
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_PERMISSION_DENIED Permission deny
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #NOTI_EX_ERROR_IO_ERROR IO error
 * @see #noti_ex_manager_h
 * @see #NOTI_EX_RECEIVER_GROUP_TICKER
 * @see #NOTI_EX_RECEIVER_GROUP_LOCKSCREEN
 * @see #NOTI_EX_RECEIVER_GROUP_INDICATOR
 * @see #NOTI_EX_RECEIVER_GROUP_POPUP
 * @par Sample code:
 * @code
#include <notification_ex.h>

void _manager_events_error_cb(noti_ex_manager_h handle, noti_ex_error_e error,
		int req_id, void *user_data) {
}

{
	int ret;
	noti_ex_manager_h handle;
	noti_ex_manager_events_s ev;
	int cnt;

	ev.error = _manager_events_error_cb;
	ret = noti_ex_manager_create(&handle, NOTI_EX_RECEIVER_GROUP_INDICATOR, ev, NULL);
	ret = noti_ex_manager_get_notification_count(handle, &cnt);
}
 * @endcode
 */
int noti_ex_manager_get_notification_count(noti_ex_manager_h handle, int *count) TIZEN_DEPRECATED_API;

/**
 * @}
 */

#ifdef __cplusplus
}
#endif
#endif  /* __TIZEN_APPFW_NOTIFICATION_EX_MANAGER_H__ */
