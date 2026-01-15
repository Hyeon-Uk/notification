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

#ifndef __TIZEN_APPFW_NOTIFICATION_EX_CHECKBOX_H__
#define __TIZEN_APPFW_NOTIFICATION_EX_CHECKBOX_H__

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
 * @brief Creates the notification_ex item handle with checkbox.
 * @details The notification_ex item is the predefined type of notifications.
 *          The notification can be created with one item or group of items.
 *          If the user wants to display the notification with a checkbox,
 *          the notification_ex handle has to be created as a checkbox item.
 * @since_tizen 5.5
 * @remarks @a handle must be released using noti_ex_item_destroy().
 * @param[out] handle The notification_ex item handle
 * @param[in] id The id of notification_ex item
 * @param[in] title The title of notification_ex item
 * @param[in] checked The check state of checkbox
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
	noti_ex_item_h checkbox_item = NULL;

	ret = noti_ex_item_checkbox_create(&checkbox_item, "checkbox_id", "checkbox_title", false);
}
 * @endcode
 */
int noti_ex_item_checkbox_create(noti_ex_item_h *handle, const char *id, const char *title, bool checked) TIZEN_DEPRECATED_API;

/**
 * @deprecated Deprecated since 11.0.
 * @brief Gets the title of a checkbox.
 * @since_tizen 5.5
 * @remarks @a title must be released using free().
 * @remarks If the multi-language handle is set by noti_ex_item_checkbox_set_multi_language_title(), \n
 *          it will return a multi-language title.
 * @param[in] handle The notification_ex item handle
 * @param[out] title The title of notification_ex item
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter, if the item type is not correct, this error is returned
 * @see #noti_ex_item_h
 * @see noti_ex_item_checkbox_create()
 * @see noti_ex_item_checkbox_set_multi_language_title()
 * @par Sample code:
 * @code
#include <notification_ex.h>

{
	int ret;
	char *title = NULL;

	ret = noti_ex_item_checkbox_get_title(checkbox_item, &title);
}
 * @endcode
 */
int noti_ex_item_checkbox_get_title(noti_ex_item_h handle, char **title) TIZEN_DEPRECATED_API;

/**
 * @deprecated Deprecated since 11.0.
 * @brief Sets the multi-language handle.
 * @since_tizen 5.5
 * @remarks After setting the multi-language handle successfully, \n
 *          noti_ex_item_checkbox_get_title() will return the multi-language text.
 * @remarks The #noti_ex_item_h handle keeps its own copy of #noti_ex_multi_lang_h. \n
 *          Therefore, you can destroy @a multi after it was set for @ handle.
 * @remarks You can pass NULL to @a multi, if you want to remove the multi-language text.
 * @param[in] handle The notification_ex item handle
 * @param[in] multi The multi-language handle
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter, if the item type is not correct, this error is returned
 * @see #noti_ex_item_h
 * @see noti_ex_item_checkbox_create()
 * @see noti_ex_item_checkbox_get_title()
 * @see noti_ex_multi_lang_create()
 * @par Sample code:
 * @code
#include <notification_ex.h>

{
	int ret;
	noti_ex_item_h checkbox_item = NULL;
	noti_ex_multi_lang_h multi;

	ret = noti_ex_item_checkbox_create(&checkbox_item, "checkbox_id", "checkbox_title", false);
	ret = noti_ex_multi_lang_create(&multi, "_IDS_NOTI_TEST_", "Hi %d I'm %s", 33, "test");
	ret = noti_ex_item_checkbox_set_multi_language_title(checkbox_item, multi);
	noti_ex_multi_lang_destroy(multi);
}
 * @endcode
 */
int noti_ex_item_checkbox_set_multi_language_title(noti_ex_item_h handle,
		noti_ex_multi_lang_h multi) TIZEN_DEPRECATED_API;

/**
 * @deprecated Deprecated since 11.0.
 * @brief Gets the check state of a checkbox.
 * @since_tizen 5.5
 * @param[in] handle The notification_ex item handle
 * @param[out] checked The check state of a checkbox
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter, if the item type is not correct, this error is returned
 * @see #noti_ex_item_h
 * @see noti_ex_item_checkbox_create()
 * @par Sample code:
 * @code
#include <notification_ex.h>

{
	int ret;
	bool checked;

	ret = noti_ex_item_checkbox_get_check_state(checkbox_item, &checked);
}
 * @endcode
 */
int noti_ex_item_checkbox_get_check_state(noti_ex_item_h handle, bool *checked) TIZEN_DEPRECATED_API;

/**
 * @deprecated Deprecated since 11.0.
 * @brief Sets the check state of a checkbox.
 * @since_tizen 5.5
 * @param[in] handle The notification_ex item handle
 * @param[in] checked The check state of a checkbox
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter, if the item type is not correct, this error is returned
 * @see #noti_ex_item_h
 * @see noti_ex_item_checkbox_create()
 * @par Sample code:
 * @code
#include <notification_ex.h>

{
	int ret;

	ret = noti_ex_item_checkbox_set_check_state(checkbox_item, true);
}
 * @endcode
 */
int noti_ex_item_checkbox_set_check_state(noti_ex_item_h handle, bool checked) TIZEN_DEPRECATED_API;

/**
 * @}
 */

#ifdef __cplusplus
}
#endif
#endif  /* __TIZEN_APPFW_NOTIFICATION_EX_CHECKBOX_H__ */
