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

#ifndef __TIZEN_APPFW_NOTIFICATION_EX_BUTTON_H__
#define __TIZEN_APPFW_NOTIFICATION_EX_BUTTON_H__

#include <notification_ex_item.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup NOTIFICATION_EX_MODULE
 * @{
 */

/**
 * @brief Creates the notification_ex item handle with a button.
 * @details The notification_ex item is the predefined type of notifications.
 *          The notification can be created with one item or group of items.
 *          If the user wants to display the notification with a button,
 *          the notification_ex handle has to be created as a button item.
 * @since_tizen 5.5
 * @remarks @a handle must be released using noti_ex_item_destroy().
 * @param[out] handle The notification_ex item handle
 * @param[in] id The id of notification_ex item
 * @param[in] title The title of notification_ex item
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #NOTI_EX_ERROR_OUT_OF_MEMORY out of memory
 * @see #noti_ex_item_h
 * @see #noti_ex_item_destroy()
 * @par Sample code:
 * @code
#include <notification_ex_button.h>

{
	int ret;
	noti_ex_item_h button_item = NULL;

	ret = noti_ex_item_button_create(&button_item, "button_id", "button_title");
}
 * @endcode
 */
int noti_ex_item_button_create(noti_ex_item_h *handle, const char *id, const char *title);

/**
 * @brief Gets the title of a button item.
 * @since_tizen 5.5
 * @remarks @a title must be released using free().
 * @param[in] handle The notification_ex item handle
 * @param[out] title The title of notification_ex item
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter, if the item type is not correct, this error is returned
 * @see #noti_ex_item_h
 * @see #noti_ex_item_button_create()
 * @par Sample code:
 * @code
#include <notification_ex_button.h>

{
	int ret;
	char *title = NULL;

	ret = noti_ex_item_button_get_title(button_item, &title);
}
 * @endcode
 */
int noti_ex_item_button_get_title(noti_ex_item_h handle, char **title);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif
#endif  /* __TIZEN_APPFW_NOTIFICATION_EX_BUTTON_H__ */
