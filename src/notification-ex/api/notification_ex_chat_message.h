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

#ifndef __TIZEN_APPFW_NOTIFICATION_EX_CHAT_MESSAGE_H__
#define __TIZEN_APPFW_NOTIFICATION_EX_CHAT_MESSAGE_H__

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
 * @brief Enumeration for notification_ex chat message type.
 * @since_tizen 5.5
 */
typedef enum _noti_ex_item_chat_message_type {
	NOTI_EX_ITEM_CHAT_MESSAGE_TYPE_USER, /**< The user of chat message */
	NOTI_EX_ITEM_CHAT_MESSAGE_TYPE_SENDER, /**< The sender of chat message */
	NOTI_EX_ITEM_CHAT_MESSAGE_TYPE_NONE, /**< Not user or sender */
} noti_ex_item_chat_message_type_e;

/**
 * @deprecated Deprecated since 11.0.
 * @brief Creates the notification_ex item handle for the chat message.
 * @details The notification_ex item is the predefined type of notifications.
 *          The notification can be created with one item or group of items.
 *          If the user wants to create a chat message notification,
 *          notification_ex handle has to be created as a chat message item.
 * @since_tizen 5.5
 * @remarks @a handle must be released using noti_ex_item_destroy().
 *          @a name, @a text, @a image and @a time must not be released if the function returns #NOTI_EX_ERROR_NONE.
 *          @a name, @a text, @a image and @a time will be released when @a handle is released.
 *          @a name and @a time are mandatory (cannot be null). Both @a text and @a image cannot be null.
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
 * @see noti_ex_item_text_create()
 * @see noti_ex_item_image_create()
 * @see noti_ex_item_destroy()
 * @par Sample code:
 * @code
#include <notification_ex.h>

{
	int ret;
	noti_ex_item_h chatmessage_item = NULL;
	noti_ex_item_h name_handle = NULL;
	noti_ex_item_h text_handle = NULL;
	noti_ex_item_h image_handle = NULL;
	noti_ex_item_h time_handle = NULL;
	time_t current_time;

	noti_ex_item_text_create(&name_handle, "name_id", "name", NULL);
	noti_ex_item_text_create(&text_handle, "text_id", "text", NULL);
	noti_ex_item_image_create(&image_handle, "image_id", APP_IMAGE_FULL_PATH);

	time(&current_time);
	noti_ex_item_time_create(&time_handle, "time_id", current_time);

	ret = noti_ex_item_chat_message_create(&chatmessage_item, "message_id", name_handle,
				text_handle, image_handle, time_handle, NOTI_EX_ITEM_CHAT_MESSAGE_TYPE_USER);
}
 * @endcode
 */
int noti_ex_item_chat_message_create(noti_ex_item_h *handle, const char *id,
		noti_ex_item_h name, noti_ex_item_h text, noti_ex_item_h image,
		noti_ex_item_h time, noti_ex_item_chat_message_type_e message_type) TIZEN_DEPRECATED_API;

/**
 * @deprecated Deprecated since 11.0.
 * @brief Gets the notification_ex item handle for the chat message name.
 * @details The name is a notification_ex item handle created with noti_ex_item_text_create().
 * @since_tizen 5.5
 * @remarks @a name must be released using noti_ex_item_destroy().
 * @remarks The @a name is available until @a handle is released.
 * @param[in] handle The notification_ex item handle
 * @param[out] name The notification_ex item handle with name
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter, if the item type is not correct, this error is returned
 * @see #noti_ex_item_h
 * @see noti_ex_item_chat_message_create()
 * @see noti_ex_item_text_get_contents()
 * @par Sample code:
 * @code
#include <notification_ex.h>

{
	int ret;
	noti_ex_item_h name_handle = NULL;
	char *name = NULL;

	ret = noti_ex_item_chat_message_get_name(chatmessage_item, &name_handle);
	ret = noti_ex_item_text_get_contents(name_handle, &name);
}
 * @endcode
 */
int noti_ex_item_chat_message_get_name(noti_ex_item_h handle, noti_ex_item_h *name) TIZEN_DEPRECATED_API;

/**
 * @deprecated Deprecated since 11.0.
 * @brief Gets the notification_ex item handle for the chat message text.
 * @details The text is a notification_ex item handle created with noti_ex_item_text_create().
 * @since_tizen 5.5
 * @remarks @a text must be released using noti_ex_item_destroy().
 * @remarks The @a text is available until @a handle is released.
 * @param[in] handle The notification_ex item handle
 * @param[out] text The notification_ex item handle with text
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter, if the item type is not correct, this error is returned
 * @see #noti_ex_item_h
 * @see noti_ex_item_chat_message_create()
 * @see noti_ex_item_text_get_contents()
 * @par Sample code:
 * @code
#include <notification_ex.h>

{
	int ret;
	noti_ex_item_h text_handle = NULL;
	char *text = NULL;

	ret = noti_ex_item_chat_message_get_text(chatmessage_item, &text_handle);
	ret = noti_ex_item_text_get_contents(text_handle, &text);
}
 * @endcode
 */
int noti_ex_item_chat_message_get_text(noti_ex_item_h handle, noti_ex_item_h *text) TIZEN_DEPRECATED_API;

/**
 * @deprecated Deprecated since 11.0.
 * @brief Gets the notification_ex item handle for the chat message image.
 * @details The image is a notification_ex item handle created with noti_ex_item_image_create().
 * @since_tizen 5.5
 * @remarks @a image must be released using noti_ex_item_destroy().
 * @remarks The @a image is available until @a handle is released.
 * @param[in] handle The notification_ex item handle
 * @param[out] image The notification_ex item handle with image
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter, if the item type is not correct, this error is returned
 * @see #noti_ex_item_h
 * @see noti_ex_item_chat_message_create()
 * @see noti_ex_item_image_get_image_path()
 * @par Sample code:
 * @code
#include <notification_ex.h>

{
	int ret;
	noti_ex_item_h image_handle = NULL;
	char *image_path = NULL;

	ret = noti_ex_item_chat_message_get_image(chatmessage_item, &image_handle);
	ret = noti_ex_item_image_get_image_path(image_handle, &image_path);
}
 * @endcode
 */
int noti_ex_item_chat_message_get_image(noti_ex_item_h handle, noti_ex_item_h *image) TIZEN_DEPRECATED_API;

/**
 * @deprecated Deprecated since 11.0.
 * @brief Gets the notification_ex item handle for the chat message time.
 * @details The time is notification_ex item handle created with noti_ex_item_time_create().
 * @since_tizen 5.5
 * @remarks @a time must be released using noti_ex_item_destroy().
 * @remarks The @a time is available until @a handle is released.
 * @param[in] handle The notification_ex item handle
 * @param[out] time The notification_ex item handle with time
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter, if the item type is not correct, this error is returned
 * @see #noti_ex_item_h
 * @see noti_ex_item_chat_message_create()
 * @see noti_ex_item_time_get_time()
 * @par Sample code:
 * @code
#include <notification_ex.h>

{
	int ret;
	noti_ex_item_h time_handle = NULL;
	time_t time_info;

	ret = noti_ex_item_chat_message_get_time(chatmessage_item, &time_handle);
	ret = noti_ex_item_time_get_time(time_handle, &time_info);
}
 * @endcode
 */
int noti_ex_item_chat_message_get_time(noti_ex_item_h handle, noti_ex_item_h *time) TIZEN_DEPRECATED_API;

/**
 * @deprecated Deprecated since 11.0.
 * @brief Gets the type of chat message.
 * @since_tizen 5.5
 * @param[in] handle The notification_ex item handle
 * @param[out] message_type The type of chat message
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter, if the item type is not correct, this error is returned
 * @see #noti_ex_item_h
 * @see #noti_ex_item_chat_message_type_e
 * @see noti_ex_item_chat_message_create()
 * @par Sample code:
 * @code
#include <notification_ex.h>

{
	int ret;
	noti_ex_item_chat_message_type_e type;

	ret = noti_ex_item_chat_message_get_message_type(chatmessage_item, &type);
}
 * @endcode
 */
int noti_ex_item_chat_message_get_message_type(noti_ex_item_h handle, noti_ex_item_chat_message_type_e *message_type) TIZEN_DEPRECATED_API;

/**
 * @}
 */

#ifdef __cplusplus
}
#endif
#endif  /* __TIZEN_APPFW_NOTIFICATION_EX_CHAT_MESSAGE_H__ */
