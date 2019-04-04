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

#ifndef CAPI_NOTIFICATION_EX_INPUT_SELECTOR_H_
#define CAPI_NOTIFICATION_EX_INPUT_SELECTOR_H_

#include <notification_ex_item.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Creates the notification_ex item handle with input selector.
 * @details The notification_ex is made with notification_ex items.
 *          If the user wants to be displayed with selector to select predefined input,
 *          notification_ex handle have to be made as input selector item.
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
#include <notification_ex_input_selector.h>

{
	int ret;
	noti_ex_item_h selector_item = NULL;

	ret = noti_ex_item_input_selector_create(&selector_item, "selector_id");
}
 * @endcode
 */
int noti_ex_item_input_selector_create(noti_ex_item_h *handle, const char *id);

/**
 * @brief Gets the contents that can be selected by input selector.
 * @since_tizen 5.5
 * @remarks @a list must be released after releasing all contents using free().
 * @param[in] handle The notification_ex item handle
 * @param[out] list The list of contents
 * @param[out] count The count of contents
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_item_h
 * @see #noti_ex_item_input_selector_create
 * @par Sample code:
 * @code
#include <notification_ex_input_selector.h>

{
	int ret;
    char **list = NULL;
    int count = 0;

	ret = noti_ex_item_input_selector_get_contents(&selector_item, &list, &count);
}
 * @endcode
 */
int noti_ex_item_input_selector_get_contents(noti_ex_item_h handle, char ***list, int *count);

/**
 * @brief Sets the contents for input selector item.
 * @since_tizen 5.5
 * @param[in] handle The notification_ex item handle
 * @param[out] contents The list of contents
 * @param[out] count The count of contents
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_item_h
 * @see #noti_ex_item_input_selector_create
 * @par Sample code:
 * @code
#include <notification_ex_input_selector.h>

{
	int ret;

	ret = noti_ex_item_input_selector_set_contents(&selector_item, list, count);
}
 * @endcode
 */
int noti_ex_item_input_selector_set_contents(noti_ex_item_h handle, const char **contents, int count);

#ifdef __cplusplus
}
#endif
#endif  // CAPI_NOTIFICATION_EX_INPUT_SELECTOR_H_
