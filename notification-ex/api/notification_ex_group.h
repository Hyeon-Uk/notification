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

#ifndef CAPI_NOTIFICATION_EX_GROUP_H_
#define CAPI_NOTIFICATION_EX_GROUP_H_

#include <notification_ex_item.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Creates the group of notification_ex items.
 * @details The notification_ex is made with notification_ex items.
 *          If the user wants to make the group of notification_ex items,
 *          notification_ex handle have to be made as group item.
 * @since_tizen 5.5
 * @remarks @a handle must be released using noti_ex_item_destroy().
 * @param[out] handle The notification_ex item handle
 * @param[in] id The id of notification_ex item
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #NOTI_EX_ERROR_OUT_OF_MEMORY out of memory
 * @see #noti_ex_item_h
 * @see #noti_ex_item_destroy
 * @par Sample code:
 * @code
#include <notification_ex_group.h>

{
	int ret;
	noti_ex_item_h group_item = NULL;

	ret = noti_ex_item_group_create(&group_item, "group_id");
}
 * @endcode
 */
int noti_ex_item_group_create(noti_ex_item_h *handle, const char *id);

/**
 * @brief Sets the direction of children in the group item.
 * @details If vertical state is true, the children of group item are placed vertically.
 *          If vertical state is false, the children of group item are placed horizontally.
 * @since_tizen 5.5
 * @param[in] handle The notification_ex item handle
 * @param[in] vertical The vertical state of group item
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_item_h
 * @see #noti_ex_item_group_create
 * @par Sample code:
 * @code
#include <notification_ex_group.h>

{
	int ret;

	ret = noti_ex_item_group_set_direction(group_item, true);
}
 * @endcode
 */
int noti_ex_item_group_set_direction(noti_ex_item_h handle, bool vertical);

/**
 * @brief Gets the vertical state of group item.
 * @since_tizen 5.5
 * @param[in] handle The notification_ex item handle
 * @param[out] vertical The vertical state of group item
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_item_h
 * @see #noti_ex_item_group_create
 * @par Sample code:
 * @code
#include <notification_ex_group.h>

{
	int ret;
	bool vertical;

	ret = noti_ex_item_group_is_vertical(group_item, &vertical);
}
 * @endcode
 */
int noti_ex_item_group_is_vertical(noti_ex_item_h handle, bool *vertical);

/**
 * @brief Gets the label of application.
 * @since_tizen 5.5
 * @remarks @a label must be released using free().
 * @param[in] handle The notification_ex item handle
 * @param[out] label The label of application
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_item_h
 * @see #noti_ex_item_group_create
 * @par Sample code:
 * @code
#include <notification_ex_group.h>

{
	int ret;
	char *label = NULL;

	ret = noti_ex_item_group_get_app_label(group_item, &label);
}
 * @endcode
 */
int noti_ex_item_group_get_app_label(noti_ex_item_h handle, char **label);

/**
 * @brief Add the child to the group item.
 * @since_tizen 5.5
 * @remarks @a child must not be released after calling this function
 * @param[in] handle The notification_ex item handle
 * @param[in] child The child notification_ex item handle
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_item_h
 * @see #noti_ex_item_group_create
 * @par Sample code:
 * @code
#include <notification_ex_group.h>

{
	int ret;
	noti_ex_item_h group_item = NULL;
	noti_ex_item_h child_item = NULL;

	ret = noti_ex_item_group_create(&group_item, "group_id");
	ret = noti_ex_item_button_create(&child_item, "button_id", "button_title");
	ret = noti_ex_item_group_add_child(group_item, child_item);
}
 * @endcode
 */
int noti_ex_item_group_add_child(noti_ex_item_h handle, noti_ex_item_h child);

/**
 * @brief Removes the child from the group item by notification_ex item id.
 * @since_tizen 5.5
 * @param[in] handle The notification_ex item handle
 * @param[in] item_id The id of notification_ex item that is child of group item
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_item_h
 * @see #noti_ex_item_group_create
 * @par Sample code:
 * @code
#include <notification_ex_group.h>

{
	int ret;

	ret = noti_ex_item_group_remove_child(group_item, "child_id");
}
 * @endcode
 */
int noti_ex_item_group_remove_child(noti_ex_item_h handle, const char *item_id);

/**
 * @brief Called to get the data of child item for each child of the group item.
 * @since_tizen 5.5
 * @remarks @a handle must not be released
 * @param[in] handle The notification_ex item handle
 * @param[in] data The data of child passed from foreach function
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @see #noti_ex_item_h
 * @see #noti_ex_item_group_foreach
 */
typedef int (*noti_ex_item_group_foreach_cb)(noti_ex_item_h handle, void *data);

/**
 * @brief Gets the data of children from the group item.
 * @details The noti_ex_item_group_foreach_cb is called each child of group item.
 * @since_tizen 5.5
 * @param[in] handle The notification_ex item handle
 * @param[in] callback The callback function to get the children data
 * @param[in] data The data to be passed to the callback function
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_item_h
 * @see #noti_ex_item_group_foreach_cb
 * @par Sample code:
 * @code
#include <notification_ex_group.h>

int _ex_item_group_foreach_cb(noti_ex_item_h handle, void *data) {
	return 0;
}

{
	int ret;
	noti_ex_item_h group_item = NULL;
	noti_ex_item_h child_item = NULL;

	ret = noti_ex_item_group_create(&group_item, "group_id");
	ret = noti_ex_item_button_create(&child_item, "button_id", "button_title");
	ret = noti_ex_item_group_add_child(group_item, child_item);

	ret = noti_ex_item_group_foreach(_ex_item_group_foreach_cb, NULL);
}
 * @endcode
 */
int noti_ex_item_group_foreach(noti_ex_item_h handle,
		noti_ex_item_group_foreach_cb callback, void *data);

#ifdef __cplusplus
}
#endif
#endif  // CAPI_NOTIFICATION_EX_GROUP_H_
