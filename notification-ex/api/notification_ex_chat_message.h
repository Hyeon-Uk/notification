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

#ifndef CAPI_NOTIFICATION_EX_CHAT_MESSAGE_H_
#define CAPI_NOTIFICATION_EX_CHAT_MESSAGE_H_

#include <notification_ex_item.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Enumeration for notification_ex chat message type.
 * @since_tizen 5.5
 */
typedef enum _noti_ex_item_chat_message_type {
	NOTI_EX_ITEM_CHAT_MESSAGE_TYPE_USER, /**< The user of chat message */
	NOTI_EX_ITEM_CHAT_MESSAGE_TYPE_SENDER, /**< The sender of chat messasge */
} noti_ex_item_chat_message_type_e;

/**
 * @brief Creates the notification_ex item handle for chat message.
 * @details The notification_ex is made with notification_ex items.
 *          If the user wants to make chat message notification,
 *          notification_ex handle have to be made as chat message item.
 * @since_tizen 5.5
 * @remarks @a handle must be released using noti_ex_item_destroy().
 *          @a name, @a text, @a image and @a time must not be released if it returns #NOTI_EX_ERROR_NONE.
 * @param[out] handle The noti ex item handle
 * @param[in] id The id of notification_ex item
 * @param[in] name The notification_ex item handle for name
 * @param[in] text The notification_ex item handle for text
 * @param[in] image The notification_ex item handle for image
 * @param[in] time The notification_ex item handle for time
 * @param[in] message_type The type of chat message
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #NOTI_EX_ERROR_OUT_OF_MEMORY out of memory
 * @see #noti_ex_item_h
 * @see #noti_ex_item_chat_message_type_e
 * @see #noti_ex_item_text_create
 * @see #noti_ex_item_image_create
 * @see #noti_ex_item_destroy
 * @par Sample code:
 * @code
#include <notification_ex_chat_message.h>

{
	int ret;
	noti_ex_item_h chatmessage_item = NULL;
	noti_ex_item_h name = NULL;
	noti_ex_item_h text = NULL;
	noti_ex_item_h image = NULL;
	noti_ex_item_h time = NULL;
	time_t current_time;

	noti_ex_item_text_create(&name, "name_id", "name", NULL);
	noti_ex_item_text_create(&text, "text_id", "text", NULL);
	noti_ex_item_image_create(&image, "image_id", "image_path");

	time(&current_time);
	noti_ex_item_time_create(&time, "time_id", current_time);

	ret = noti_ex_item_chat_message_create(&chatmessage_item, "message_id", name,
				text, image, time, NOTI_EX_ITEM_CHAT_MESSAGE_TYPE_USER);
}
 * @endcode
 */
int noti_ex_item_chat_message_create(noti_ex_item_h *handle, const char *id,
		noti_ex_item_h name, noti_ex_item_h text, noti_ex_item_h image,
		noti_ex_item_h time, noti_ex_item_chat_message_type_e message_type);

/**
 * @brief Gets the notification_ex item handle for name of chat message.
 * @details The name of chat message is notification_ex item handle with text that is name.
 * @since_tizen 5.5
 * @remarks @a name should not be released.
 * @param[in] handle The notification_ex item handle
 * @param[out] name The notification_ex item handle with name
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_item_h
 * @see #noti_ex_item_chat_message_create
 * @par Sample code:
 * @code
#include <notification_ex_chat_message.h>

{
	int ret;
	noti_ex_item_h name = NULL;

	ret = noti_ex_item_chat_message_get_name(chatmessage_item, &name);
}
 * @endcode
 */
int noti_ex_item_chat_message_get_name(noti_ex_item_h handle, noti_ex_item_h *name);

/**
 * @brief Gets the notification_ex item handle for text of chat message.
 * @details The text of chat message is notification_ex item handle with text.
 * @since_tizen 5.5
 * @remarks @a text should not be released.
 * @param[in] handle The notification_ex item handle
 * @param[out] text The notification_ex item handle with text
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_item_h
 * @see #noti_ex_item_chat_message_create
 * @par Sample code:
 * @code
#include <notification_ex_chat_message.h>

{
	int ret;
	noti_ex_item_h text = NULL;

	ret = noti_ex_item_chat_message_get_text(chatmessage_item, &text);
}
 * @endcode
 */
int noti_ex_item_chat_message_get_text(noti_ex_item_h handle, noti_ex_item_h *text);

/**
 * @brief Gets the notification_ex item handle for image of chat message.
 * @details The image of chat message is notification_ex item handle with image.
 * @since_tizen 5.5
 * @remarks @a image should not be released.
 * @param[in] handle The notification_ex item handle
 * @param[out] image The notification_ex item handle with image
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_item_h
 * @see #noti_ex_item_chat_message_create
 * @par Sample code:
 * @code
#include <notification_ex_chat_message.h>

{
	int ret;
	noti_ex_item_h image = NULL;

	ret = noti_ex_item_chat_message_get_image(chatmessage_item, &image);
}
 * @endcode
 */
int noti_ex_item_chat_message_get_image(noti_ex_item_h handle, noti_ex_item_h *image);

/**
 * @brief Gets the notification_ex item handle for time of chat message.
 * @details The time of chat message is notification_ex item handle with time.
 * @since_tizen 5.5
 * @remarks @a time should not be released.
 * @param[in] handle The notification_ex item handle
 * @param[out] time The notification_ex item handle with time
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_item_h
 * @see #noti_ex_item_chat_message_create
 * @par Sample code:
 * @code
#include <notification_ex_chat_message.h>

{
	int ret;
	noti_ex_item_h time = NULL;

	ret = noti_ex_item_chat_message_get_time(chatmessage_item, &time);
}
 * @endcode
 */
int noti_ex_item_chat_message_get_time(noti_ex_item_h handle, noti_ex_item_h *time);

/**
 * @brief Gets the type of chat message.
 * @since_tizen 5.5
 * @param[in] handle The notification_ex item handle
 * @param[out] message_type The type of chat message
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_item_h
 * @see #noti_ex_item_chat_message_create
 * @see #noti_ex_item_chat_message_type_e
 * @par Sample code:
 * @code
#include <notification_ex_chat_message.h>

{
	int ret;
	noti_ex_item_chat_message_type_e type;

	ret = noti_ex_item_chat_message_get_message_type(chatmessage_item, &type);
}
 * @endcode
 */
int noti_ex_item_chat_message_get_message_type(noti_ex_item_h handle, noti_ex_item_chat_message_type_e *message_type);

#ifdef __cplusplus
}
#endif
#endif  // CAPI_NOTIFICATION_EX_CHAT_MESSAGE_H_
