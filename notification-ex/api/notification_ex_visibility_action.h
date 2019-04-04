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

#ifndef CAPI_NOTIFICATION_EX_VISIBILITY_ACTION_H_
#define CAPI_NOTIFICATION_EX_VISIBILITY_ACTION_H_

#include <stdbool.h>
#include <notification_ex_item.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Creates the notification_ex action handle to control visibility.
 * @details The notification_ex can define the action with noti_ex_action_h.
 *          If it is necessary to control visibility of notification_ex item,
 *          notification_ex action handle have to be made as visibility action. .
 * @since_tizen 5.5
 * @remarks @a handle must be released using noti_ex_action_destroy().
 * @param[out] handle The notification_ex action handle
 * @param[in] extra The extra data
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #NOTI_EX_ERROR_OUT_OF_MEMORY out of memory
 * @see #noti_ex_action_h
 * @see #noti_ex_action_destroy
 * @par Sample code:
 * @code
#include <notification_ex_visibility_action.h>

{
	int ret;
	noti_ex_action_h visibility_action = NULL;

	ret = noti_ex_action_visibility_create(&visibility_action, NULL);
}
 * @endcode
 */
int noti_ex_action_visibility_create(noti_ex_action_h *handle, const char *extra);

/**
 * @brief Sets the visible state for notification_ex item.
 * @since_tizen 5.5
 * @param[in] handle The notification_ex action handle
 * @param[in] id The id of notification_ex item
 * @param[in] visible The visible state
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_action_h
 * @see #noti_ex_action_visibility_create
 * @par Sample code:
 * @code
#include <notification_ex_visibility_action.h>

{
	int ret;

	ret = noti_ex_action_visibility_set(&visibility_action, "item_id", true);
}
 * @endcode
 */
int noti_ex_action_visibility_set(noti_ex_action_h handle, const char *id, bool visible);

#ifdef __cplusplus
}
#endif
#endif  // CAPI_NOTIFICATION_EX_VISIBILITY_ACTION_H_
