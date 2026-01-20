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

#ifndef __TIZEN_APPFW_NOTIFICATION_EX_TIME_H__
#define __TIZEN_APPFW_NOTIFICATION_EX_TIME_H__

#include <notification_ex_item.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup NOTIFICATION_EX_MODULE
 * @{
 */

/**
 * @deprecated Deprecated since 11.0.
 * @brief Creates the notification_ex item handle with time.
 * @details The notification_ex item is the predefined type of notifications.
 *          The notification can be created with one item or group of items.
 *          If the user wants to be display the notification with time,
 *          the notification_ex handle has to be created as a time item.
 * @since_tizen 5.5
 * @remarks @a handle must be released using noti_ex_item_destroy().
 * @param[out] handle The notification_ex item handle
 * @param[in] id The id of notification_ex item
 * @param[in] time The time information
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #NOTI_EX_ERROR_OUT_OF_MEMORY out of memory
 * @see #noti_ex_item_h
 * @see noti_ex_item_destroy()
 * @par Sample code:
 * @code
#include <notification_ex.h>

{
	int ret;
	noti_ex_item_h time_item = NULL;
	time_t time_info;
	time(&time_info);

	ret = noti_ex_item_time_create(&time_item, "time_id", time_info);
}
 * @endcode
 */
int noti_ex_item_time_create(noti_ex_item_h *handle, const char *id, time_t time) TIZEN_DEPRECATED_API;

/**
 * @deprecated Deprecated since 11.0.
 * @brief Gets the time information of time item.
 * @since_tizen 5.5
 * @remarks @a handle must be released using noti_ex_item_destroy().
 * @param[in] handle The notification_ex item handle
 * @param[out] time The time information of time item
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter, if the item type is not correct, this error is returned
 * @see #noti_ex_item_h
 * @see noti_ex_item_time_create()
 * @par Sample code:
 * @code
#include <notification_ex.h>

{
	int ret;
	time_t time_info;

	ret = noti_ex_item_time_get_time(time_item, &time_info);
}
 * @endcode
 */
int noti_ex_item_time_get_time(noti_ex_item_h handle, time_t *time) TIZEN_DEPRECATED_API;

/**
 * @}
 */

#ifdef __cplusplus
}
#endif
#endif  /* __TIZEN_APPFW_NOTIFICATION_EX_TIME_H__ */
