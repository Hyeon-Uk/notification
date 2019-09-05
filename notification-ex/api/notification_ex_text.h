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

#ifndef __TIZEN_APPFW_NOTIFICATION_EX_TEXT_H__
#define __TIZEN_APPFW_NOTIFICATION_EX_TEXT_H__

#include <notification_ex_item.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup NOTIFICATION_EX_ITEM
 * @{
 */

/**
 * @brief Creates the notification_ex item handle with text.
 * @details The notification_ex item is the predefined type of notifications.
 *          The notification can be created with one item or group of items.
 *          If the user wants to be display the notification with text,
 *          the notification_ex handle has to be created as a text item.
 * @since_tizen 5.5
 * @remarks @a handle must be released using noti_ex_item_destroy().
 * @param[out] handle The notification_ex item handle
 * @param[in] id The id of notification_ex item
 * @param[in] text The text of text item
 * @param[in] hyperlink The hyperlink of text item
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
	noti_ex_item_h text_item = NULL;

	ret = noti_ex_item_text_create(&text_item, "text_id", "text", "hyperlink");
}
 * @endcode
 */
int noti_ex_item_text_create(noti_ex_item_h *handle, const char *id, const char *text, const char *hyperlink);

/**
 * @brief Sets the text contents of the text item.
 * @since_tizen 5.5
 * @param[in] handle The notification_ex item handle
 * @param[in] contents The text contents
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_item_h
 * @see noti_ex_item_text_create()
 * @par Sample code:
 * @code
#include <notification_ex.h>

{
	int ret;

	ret = noti_ex_item_text_set_contents(text_item, "text_contents");
}
 * @endcode
 */
int noti_ex_item_text_set_contents(noti_ex_item_h handle, const char *contents);

/**
 * @brief Gets the text contents of the text item.
 * @since_tizen 5.5
 * @remarks @a contents must be released using free().
 * @param[in] handle The notification_ex item handle
 * @param[out] contents The text contents
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter, if the item type is not correct, this error is returned
 * @see #noti_ex_item_h
 * @see noti_ex_item_text_create()
 * @par Sample code:
 * @code
#include <notification_ex.h>

{
	int ret;
	char *contents = NULL;

	ret = noti_ex_item_text_get_contents(text_item, &contents);
}
 * @endcode
 */
int noti_ex_item_text_get_contents(noti_ex_item_h handle, char **contents);

/**
 * @brief Gets the hyperlink of the text item.
 * @since_tizen 5.5
 * @remarks @a hyperlink must be released using free().
 * @param[in] handle The notification_ex item handle
 * @param[out] hyperlink The hyperlink
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter, if the item type is not correct, this error is returned
 * @see #noti_ex_item_h
 * @see noti_ex_item_text_create()
 * @par Sample code:
 * @code
#include <notification_ex.h>

{
	int ret;
	char *hyperlink = NULL;

	ret = noti_ex_item_text_get_hyperlink(text_item, &hyperlink);
}
 * @endcode
 */
int noti_ex_item_text_get_hyperlink(noti_ex_item_h handle, char **hyperlink);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif
#endif  /* __TIZEN_APPFW_NOTIFICATION_EX_TEXT_H__ */
