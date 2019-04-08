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

#ifndef CAPI_NOTIFICATION_EX_TEXT_H_
#define CAPI_NOTIFICATION_EX_TEXT_H_

#include <notification_ex_item.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Creates the notification_ex item handle with text.
 * @details The notification_ex is made with notification_ex items.
 *          If the user wants to be displayed with text,
 *          notification_ex handle have to be made as text item.
 * @since_tizen 5.5
 * @remarks @a handle must be released using noti_ex_item_destroy().
 * @param[out] handle The notification_ex item handle
 * @param[in] id The id of notification_ex item
 * @param[in] text The text of text item
 * @param[in] hyper_link The hyperlink of text item
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #NOTI_EX_ERROR_OUT_OF_MEMORY out of memory
 * @see #noti_ex_item_h
 * @see #noti_ex_item_destroy
 * @par Sample code:
 * @code
#include <notification_ex_text.h>

{
	int ret;
	noti_ex_item_h text_item = NULL;

	ret = noti_ex_item_text_create(&text_item, "text_id", "text", "hyperlink");
}
 * @endcode
 */
int noti_ex_item_text_create(noti_ex_item_h *handle, const char *id, const char *text, const char *hyper_link);

/**
 * @brief Sets the text contents of text item
 * @since_tizen 5.5
 * @param[in] handle The notification_ex item handle
 * @param[in] contents The text contents of text item
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_item_h
 * @see #noti_ex_item_text_create
 * @par Sample code:
 * @code
#include <notification_ex_text.h>

{
	int ret;

	ret = noti_ex_item_text_set_contents(text_item, "text_contents");
}
 * @endcode
 */
int noti_ex_item_text_set_contents(noti_ex_item_h handle, const char *contents);

/**
 * @brief Gets the text contents of text item
 * @since_tizen 5.5
 * @remarks @a contents must be released using free().
 * @param[in] handle The notification_ex item handle
 * @param[out] contents The text contents of text item
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_item_h
 * @see #noti_ex_item_text_create
 * @par Sample code:
 * @code
#include <notification_ex_text.h>

{
	int ret;
    char *contents = NULL;

	ret = noti_ex_item_text_get_contents(text_item, &contents);
}
 * @endcode
 */
int noti_ex_item_text_get_contents(noti_ex_item_h handle, char **contents);

/**
 * @brief Gets the hyperlink of text item
 * @since_tizen 5.5
 * @remarks @a hyper_link must be released using free().
 * @param[in] handle The notification_ex item handle
 * @param[out] hyper_link The hyperlink of text item
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_item_h
 * @see #noti_ex_item_text_create
 * @par Sample code:
 * @code
#include <notification_ex_text.h>

{
	int ret;
    char *hyper_link = NULL;

	ret = noti_ex_item_text_get_hyperlink(text_item, &hyper_link);
}
 * @endcode
 */
int noti_ex_item_text_get_hyperlink(noti_ex_item_h handle, char **hyper_link);

#ifdef __cplusplus
}
#endif
#endif  // CAPI_NOTIFICATION_EX_TEXT_H_
