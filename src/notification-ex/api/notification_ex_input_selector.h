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

#ifndef __TIZEN_APPFW_NOTIFICATION_EX_INPUT_SELECTOR_H__
#define __TIZEN_APPFW_NOTIFICATION_EX_INPUT_SELECTOR_H__

#include <notification_ex_item.h>
#include <notification_ex_multi_language.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup NOTIFICATION_EX_MODULE
 * @{
 */

/**
 * @deprecated Deprecated since 11.0.
 * @brief Creates the notification_ex item handle with input selector.
 * @details The notification_ex item is the predefined type of notifications.
 *          The notification can be created with one item or group of items.
 *          If the user wants to be display the notification with selector to select predefined input,
 *          the notification_ex handle has to be created as an input selector item.
 * @since_tizen 5.5
 * @remarks @a handle must be released using noti_ex_item_destroy().
 * @param[out] handle The notification_ex item handle
 * @param[in] id The id of notification_ex item
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
	noti_ex_item_h selector_item = NULL;

	ret = noti_ex_item_input_selector_create(&selector_item, "selector_id");
}
 * @endcode
 */
int noti_ex_item_input_selector_create(noti_ex_item_h *handle, const char *id) TIZEN_DEPRECATED_API;

/**
 * @deprecated Deprecated since 11.0.
 * @brief Gets the contents that can be selected by input selector.
 * @since_tizen 5.5
 * @remarks Each item in the @a list must be released using free(), then the @a list must be released using free().
 * @remarks If the multi-language handle is set by noti_ex_item_input_selector_set_multi_language_contents(), \n
 *          it will return a multi-language contents.
 * @param[in] handle The notification_ex item handle
 * @param[out] list The list of contents
 * @param[out] count The count of contents
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter, if the item type is not correct, this error is returned
 * @see #noti_ex_item_h
 * @see noti_ex_item_input_selector_create()
 * @see noti_ex_item_input_selector_set_contents()
 * @see noti_ex_item_input_selector_set_multi_language_contents()
 * @par Sample code:
 * @code
#include <notification_ex.h>

{
	int ret;
	char **list = NULL;
	int count = 0;

	ret = noti_ex_item_input_selector_get_contents(selector_item, &list, &count);
}
 * @endcode
 */
int noti_ex_item_input_selector_get_contents(noti_ex_item_h handle, char ***list, int *count) TIZEN_DEPRECATED_API;

/**
 * @deprecated Deprecated since 11.0.
 * @brief Sets the contents for input selector item.
 * @since_tizen 5.5
 * @param[in] handle The notification_ex item handle
 * @param[in] contents The list of contents
 * @param[in] count The count of contents
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_item_h
 * @see noti_ex_item_input_selector_create()
 * @par Sample code:
 * @code
#include <notification_ex.h>

{
	int ret;
	const char *contents[] = {"What's up?", "On my way", "Nice"};
	int count = 3;

	ret = noti_ex_item_input_selector_set_contents(selector_item, contents, count);
}
 * @endcode
 */
int noti_ex_item_input_selector_set_contents(noti_ex_item_h handle, const char **contents, int count) TIZEN_DEPRECATED_API;

/**
 * @deprecated Deprecated since 11.0.
 * @brief Sets the multi-language contents for input selector item.
 * @since_tizen 5.5
 * @remarks After setting the multi-language handle successfully, \n
 *          noti_ex_item_input_selector_get_contents() will return the multi-language text. \n
 * @remarks The #noti_ex_item_h handle keeps its own copy of #noti_ex_multi_lang_h list. \n
 *          Therefore, you can destroy @a multi_language_list after it was set for @a handle.
 * @remarks You can pass NULL to @a multi_language_list, if you want to remove the multi-language text.
 * @param[in] handle The notification_ex item handle
 * @param[in] multi_language_list The list of noti_ex_multi_lang_h
 * @param[in] count The count of contents
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_item_h
 * @see noti_ex_item_input_selector_create()
 * @see noti_ex_item_input_selector_get_contents()
 * @par Sample code:
 * @code
#include <notification_ex.h>

{
	int ret;
	noti_ex_multi_lang_h *contents[] = {multi1, multi2, multi3};
	int count = 3;

	ret = noti_ex_item_input_selector_set_multi_language_contents(selector_item, contents, count);
}
 * @endcode
 */
int noti_ex_item_input_selector_set_multi_language_contents(
		noti_ex_item_h handle, noti_ex_multi_lang_h *multi_language_list, int count) TIZEN_DEPRECATED_API;

/**
 * @}
 */

#ifdef __cplusplus
}
#endif
#endif  /* __TIZEN_APPFW_NOTIFICATION_EX_INPUT_SELECTOR_H__ */
