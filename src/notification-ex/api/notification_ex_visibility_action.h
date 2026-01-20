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

#ifndef __TIZEN_APPFW_NOTIFICATION_EX_VISIBILITY_ACTION_H__
#define __TIZEN_APPFW_NOTIFICATION_EX_VISIBILITY_ACTION_H__

#include <stdbool.h>
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
 * @brief Creates the notification_ex action handle to control visibility.
 * @details The notification_ex can define the action with #noti_ex_action_h.
 *          If it is necessary to control visibility of the notification_ex item,
 *          the notification_ex action handle has to be created as a visibility action.
 *          @a extra can be used to pass user defined data
 *          and the extra data can be obtained from an action handle using #noti_ex_action_get_extra().
 * @since_tizen 5.5
 * @remarks @a handle must be released using noti_ex_action_destroy().
 * @param[out] handle The notification_ex action handle
 * @param[in] extra The extra data
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #NOTI_EX_ERROR_OUT_OF_MEMORY out of memory
 * @see #noti_ex_action_h
 * @see noti_ex_action_destroy()
 * @see noti_ex_action_get_extra()
 * @par Sample code:
 * @code
#include <notification_ex.h>

{
	int ret;
	noti_ex_action_h visibility_action = NULL;

	ret = noti_ex_action_visibility_create(&visibility_action, NULL);
}
 * @endcode
 */
int noti_ex_action_visibility_create(noti_ex_action_h *handle, const char *extra) TIZEN_DEPRECATED_API;

/**
 * @deprecated Deprecated since 11.0.
 * @brief Sets the visibility state for notification_ex item.
 * @since_tizen 5.5
 * @param[in] handle The notification_ex action handle
 * @param[in] id The id of notification_ex item
 * @param[in] visible The visibility state
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_action_h
 * @see noti_ex_action_visibility_create()
 * @par Sample code:
 * @code
#include <notification_ex.h>

{
	int ret;
	noti_ex_item_h button_item = NULL;
	noti_ex_action_h action = NULL;

	ret = noti_ex_item_button_create(&button_item, "button_id1", "button_title");

	ret = noti_ex_action_visibility_create(&action, NULL);
	ret = noti_ex_action_visibility_set(action, "button_id1", true);
	ret = noti_ex_action_visibility_set(action, "button_id2", false);

	ret = noti_ex_item_set_action(button_item, action);
}
 * @endcode
 */
int noti_ex_action_visibility_set(noti_ex_action_h handle, const char *id, bool visible) TIZEN_DEPRECATED_API;

/**
 * @}
 */

#ifdef __cplusplus
}
#endif
#endif  /* __TIZEN_APPFW_NOTIFICATION_EX_VISIBILITY_ACTION_H__ */
