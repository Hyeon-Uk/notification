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

#ifndef __TIZEN_APPFW_NOTIFICATION_EX_EVENT_INFO_H__
#define __TIZEN_APPFW_NOTIFICATION_EX_EVENT_INFO_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup NOTIFICATION_EX_MODULE
 * @{
 */

/**
 * @deprecated Deprecated since 11.0.
 * @brief Enumeration for the type of notification event.
 * @since_tizen 5.5
 */
typedef enum _noti_ex_event_info_type {
	NOTI_EX_EVENT_POST, /**< Post the notification */
	NOTI_EX_EVENT_UPDATE, /**< Update the notification */
	NOTI_EX_EVENT_DELETE, /**< Delete the notification */
	NOTI_EX_EVENT_GET, /**< Get the notification */
	NOTI_EX_EVENT_ERROR, /**< Error occurs */
	NOTI_EX_EVENT_DELETE_ALL, /**< Delete all notifications */
} noti_ex_event_info_type_e;

/**
 * @deprecated Deprecated since 11.0.
 * @brief The handle for the notification event information.
 * @since_tizen 5.5
 */
typedef void *noti_ex_event_info_h;

/**
 * @deprecated Deprecated since 11.0.
 * @brief Clones the notification event info handle.
 * @details The event for notification item is defined as
 *          notification event info handle.
 * @since_tizen 5.5
 * @remarks @a cloned_handle must be released using noti_ex_event_info_destroy().
 * @param[in] handle The notification event info handle to be cloned
 * @param[out] cloned_handle The notification event info handle
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #NOTI_EX_ERROR_OUT_OF_MEMORY out of memory
 * @see #noti_ex_event_info_h
 * @see noti_ex_event_info_destroy()
 * @par Sample code:
 * @code
#include <notification_ex.h>

void _manager_events_update_cb(noti_ex_manager_h handle,
		noti_ex_event_info_h info, noti_ex_item_h updated_item, void *user_data) {

	int ret;
	noti_ex_event_info_h cloned = NULL;

	ret = noti_ex_event_info_clone(info, &cloned);
}
 * @endcode
 */
int noti_ex_event_info_clone(noti_ex_event_info_h handle,
		noti_ex_event_info_h *cloned_handle) TIZEN_DEPRECATED_API;

/**
 * @deprecated Deprecated since 11.0.
 * @brief Releases the notification event info handle.
 * @since_tizen 5.5
 * @param[in] handle The notification event info handle
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_event_info_h
 * @par Sample code:
 * @code
#include <notification_ex.h>

void _manager_events_update_cb(noti_ex_manager_h handle,
		noti_ex_event_info_h info, noti_ex_item_h updated_item, void *user_data) {

	int ret;
	noti_ex_event_info_h cloned = NULL;

	ret = noti_ex_event_info_clone(info, &cloned);
	ret = noti_ex_event_info_destroy(cloned);
}
 * @endcode
 */
int noti_ex_event_info_destroy(noti_ex_event_info_h handle) TIZEN_DEPRECATED_API;

/**
 * @deprecated Deprecated since 11.0.
 * @brief Gets the type of an event.
 * @since_tizen 5.5
 * @param[in] handle The notification event info handle
 * @param[out] event_type The type of event
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_event_info_h
 * @see noti_ex_event_info_clone()
 * @par Sample code:
 * @code
#include <notification_ex.h>

{
	int ret;
	noti_ex_event_info_type_e type;

	ret = noti_ex_event_info_get_event_type(event_handle, &type);
}
 * @endcode
 */
int noti_ex_event_info_get_event_type(noti_ex_event_info_h handle, noti_ex_event_info_type_e *event_type) TIZEN_DEPRECATED_API;

/**
 * @deprecated Deprecated since 11.0.
 * @brief Gets the app id of event owner.
 * @since_tizen 5.5
 * @remarks @a owner must be released using free().
 * @param[in] handle The notification event info handle
 * @param[out] owner The app id of event owner
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_event_info_h
 * @see noti_ex_event_info_clone()
 * @par Sample code:
 * @code
#include <notification_ex.h>

{
	int ret;
	char *owner = NULL;

	ret = noti_ex_event_info_get_owner(event_handle, &owner);
}
 * @endcode
 */
int noti_ex_event_info_get_owner(noti_ex_event_info_h handle, char **owner) TIZEN_DEPRECATED_API;

/**
 * @deprecated Deprecated since 11.0.
 * @brief Gets the channel of an event.
 * @since_tizen 5.5
 * @remarks @a channel must be released using free().
 * @param[in] handle The notification event info handle
 * @param[out] channel The channel of event
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_event_info_h
 * @see noti_ex_event_info_clone()
 * @par Sample code:
 * @code
#include <notification_ex.h>

{
	int ret;
	char *channel = NULL;

	ret = noti_ex_event_info_get_channel(event_handle, &channel);
}
 * @endcode
 */
int noti_ex_event_info_get_channel(noti_ex_event_info_h handle, char **channel) TIZEN_DEPRECATED_API;

/**
 * @deprecated Deprecated since 11.0.
 * @brief Gets the id of a notification_ex item.
 * @since_tizen 5.5
 * @remarks @a item_id must be released using free().
 * @param[in] handle The notification event info handle
 * @param[out] item_id The id of notification_ex item
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_event_info_h
 * @see noti_ex_event_info_clone()
 * @par Sample code:
 * @code
#include <notification_ex.h>

{
	int ret;
	char *item_id = NULL;

	ret = noti_ex_event_info_get_item_id(event_handle, &item_id);
}
 * @endcode
 */
int noti_ex_event_info_get_item_id(noti_ex_event_info_h handle, char **item_id) TIZEN_DEPRECATED_API;

/**
 * @deprecated Deprecated since 11.0.
 * @brief Gets the id of request for an event.
 * @since_tizen 5.5
 * @param[in] handle The notification event info handle
 * @param[out] req_id The id of request for event
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_event_info_h
 * @see noti_ex_event_info_clone()
 * @par Sample code:
 * @code
#include <notification_ex.h>

{
	int ret;
	int request_id;

	ret = noti_ex_event_info_get_request_id(event_handle, &request_id);
}
 * @endcode
 */
int noti_ex_event_info_get_request_id(noti_ex_event_info_h handle, int *req_id) TIZEN_DEPRECATED_API;

/**
 * @}
 */

#ifdef __cplusplus
}
#endif
#endif  /* __TIZEN_APPFW_NOTIFICATION_EX_EVENT_INFO_H__ */
