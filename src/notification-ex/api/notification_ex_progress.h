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

#ifndef __TIZEN_APPFW_NOTIFICATION_EX_PROGRESS_H__
#define __TIZEN_APPFW_NOTIFICATION_EX_PROGRESS_H__

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
 * @brief Enumeration for notification_ex progress item types.
 * @since_tizen 5.5
 */
typedef enum _noti_ex_item_progress_type {
	NOTI_EX_ITEM_PROGRESS_TYPE_DEFAULT, /**< Default */
	NOTI_EX_ITEM_PROGRESS_TYPE_TIME, /**< Time */
	NOTI_EX_ITEM_PROGRESS_TYPE_PERCENT, /**< Percent */
	NOTI_EX_ITEM_PROGRESS_TYPE_PENDING, /**< Pending */
} noti_ex_item_progress_type_e;

/**
 * @deprecated Deprecated since 11.0.
 * @brief Creates the notification_ex item handle with progress.
 * @details The notification_ex item is the predefined type of notifications.
 *          The notification can be created with one item or group of items.
 *          If the user wants to be display the notification with progress,
 *          the notification_ex handle has to be created as a progress item.
 * @since_tizen 5.5
 * @remarks @a handle must be released using noti_ex_item_destroy().
 * @param[out] handle The notification_ex item handle
 * @param[in] id The id of notification_ex item
 * @param[in] min The minimum value of progress
 * @param[in] current The current value of progress
 * @param[in] max The maximum value of progress
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
	noti_ex_item_h progress_item = NULL;

	ret = noti_ex_item_progress_create(&progress_item, "progress_id", 0, 50, 100);
}
 * @endcode
 */
int noti_ex_item_progress_create(noti_ex_item_h *handle, const char *id, float min, float current, float max) TIZEN_DEPRECATED_API;

/**
 * @deprecated Deprecated since 11.0.
 * @brief Gets the current value of progress.
 * @since_tizen 5.5
 * @param[in] handle The notification_ex item handle
 * @param[out] current The current value of progress
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter, if the item type is not correct, this error is returned
 * @see #noti_ex_item_h
 * @see noti_ex_item_progress_create()
 * @par Sample code:
 * @code
#include <notification_ex.h>

{
	int ret;
	float current;

	ret = noti_ex_item_progress_get_current(progress_item, &current);
}
 * @endcode
 */
int noti_ex_item_progress_get_current(noti_ex_item_h handle, float *current) TIZEN_DEPRECATED_API;

/**
 * @deprecated Deprecated since 11.0.
 * @brief Sets the current value of progress.
 * @since_tizen 5.5
 * @param[in] handle The notification_ex item handle
 * @param[in] current The current value of progress
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_item_h
 * @see noti_ex_item_progress_create()
 * @par Sample code:
 * @code
#include <notification_ex.h>

{
	int ret;

	ret = noti_ex_item_progress_set_current(progress_item, 70);
}
 * @endcode
 */
int noti_ex_item_progress_set_current(noti_ex_item_h handle, float current) TIZEN_DEPRECATED_API;

/**
 * @deprecated Deprecated since 11.0.
 * @brief Gets the minimum value of progress.
 * @since_tizen 5.5
 * @param[in] handle The notification_ex item handle
 * @param[out] min The minimum value of progress
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter, if the item type is not correct, this error is returned
 * @see #noti_ex_item_h
 * @see noti_ex_item_progress_create()
 * @par Sample code:
 * @code
#include <notification_ex.h>

{
	int ret;
	float min;

	ret = noti_ex_item_progress_get_min(progress_item, &min);
}
 * @endcode
 */
int noti_ex_item_progress_get_min(noti_ex_item_h handle, float *min) TIZEN_DEPRECATED_API;

/**
 * @deprecated Deprecated since 11.0.
 * @brief Gets the maximum value of progress.
 * @since_tizen 5.5
 * @param[in] handle The notification_ex item handle
 * @param[out] max The maximum value of progress
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter, if the item type is not correct, this error is returned
 * @see #noti_ex_item_h
 * @see noti_ex_item_progress_create()
 * @par Sample code:
 * @code
#include <notification_ex.h>

{
	int ret;
	float max;

	ret = noti_ex_item_progress_get_max(progress_item, &max);
}
 * @endcode
 */
int noti_ex_item_progress_get_max(noti_ex_item_h handle, float *max) TIZEN_DEPRECATED_API;

/**
 * @deprecated Deprecated since 11.0.
 * @brief Gets the type of progress.
 * The type is equal to one of the values of #noti_ex_item_progress_type_e.
 * @since_tizen 5.5
 * @param[in] handle The notification_ex item handle
 * @param[out] type The type of progress
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter,
 *    if the item type is not correct, this error is returned
 * @see #noti_ex_item_h
 * @see #noti_ex_item_progress_type_e
 * @see noti_ex_item_progress_create()
 * @par Sample code:
 * @code
#include <notification_ex.h>

{
	int ret;
	int type;

	ret = noti_ex_item_progress_get_type(progress_item, &type);
}
 * @endcode
 */
int noti_ex_item_progress_get_type(noti_ex_item_h handle, int *type) TIZEN_DEPRECATED_API;

/**
 * @deprecated Deprecated since 11.0.
 * @brief Sets the type of progress.
 * @details The type is equal to one of the values of #noti_ex_item_progress_type_e.
 * @since_tizen 5.5
 * @param[in] handle The notification_ex item handle
 * @param[in] type The type of progress
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter,
 *    if the item type is not correct, this error is returned
 * @see #noti_ex_item_h
 * @see #noti_ex_item_progress_type_e
 * @see noti_ex_item_progress_create()
 * @par Sample code:
 * @code
#include <notification_ex.h>

{
	int ret;
	noti_ex_item_h progress_item = NULL;
	int type = NOTI_EX_ITEM_PROGRESS_TYPE_PERCENT;

	ret = noti_ex_item_progress_create(&progress_item, "progress_id", 0, 50, 100);
	ret = noti_ex_item_progress_set_type(progress_item, type);
}
 * @endcode
 */
int noti_ex_item_progress_set_type(noti_ex_item_h handle, int type) TIZEN_DEPRECATED_API;

/**
 * @}
 */

#ifdef __cplusplus
}
#endif
#endif  /* __TIZEN_APPFW_NOTIFICATION_EX_PROGRESS_H__ */
