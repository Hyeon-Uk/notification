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

#ifndef CAPI_NOTIFICATION_EX_ENTRY_H_
#define CAPI_NOTIFICATION_EX_ENTRY_H_

#include <notification_ex_item.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Creates the notification_ex item handle for entry item.
 * @details The notification_ex is made with notification_ex items.
 *          If the user wants to get text input directly from the notification,
 *          notification_ex handle have to be made as entry item.
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
#include <notification_ex_entry.h>

{
	int ret;
	noti_ex_item_h entry_item = NULL;

	ret = noti_ex_item_entry_create(&entry_item, "entry_id");
}
 * @endcode
 */
int noti_ex_item_entry_create(noti_ex_item_h *handle, const char *id);

/**
 * @brief Gets the text of entry item.
 * @since_tizen 5.5
 * @remarks @a text must be released using free().
 * @param[in] handle The notification_ex item handle
 * @param[out] text The text of entry item
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_item_h
 * @see #noti_ex_item_entry_create
 * @par Sample code:
 * @code
#include <notification_ex_entry.h>

{
	int ret;
	char *text = NULL;

	ret = noti_ex_item_entry_get_text(entry_item, &text);
}
 * @endcode
 */
int noti_ex_item_entry_get_text(noti_ex_item_h handle, char **text);

/**
 * @brief Sets the text of entry item.
 * @since_tizen 5.5
 * @param[in] handle The notification_ex item handle
 * @param[in] text The text of entry item
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_item_h
 * @see #noti_ex_item_entry_create
 * @par Sample code:
 * @code
#include <notification_ex_entry.h>

{
	int ret;

	ret = noti_ex_item_entry_set_text(entry_item, "entry_text");
}
 * @endcode
 */
int noti_ex_item_entry_set_text(noti_ex_item_h handle, const char *text);

#ifdef __cplusplus
}
#endif
#endif  // CAPI_NOTIFICATION_EX_ENTRY_H_
