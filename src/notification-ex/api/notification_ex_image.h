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

#ifndef __TIZEN_APPFW_NOTIFICATION_EX_IMAGE_H__
#define __TIZEN_APPFW_NOTIFICATION_EX_IMAGE_H__

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
 * @brief Creates the notification_ex item handle with an image.
 * @details The notification_ex item is the predefined type of notifications.
 *          The notification can be created with one item or group of items.
 *          If the user wants to display the notification with an image,
 *          the notification_ex handle has to be created as an image item.
 * @since_tizen 5.5
 * @remarks @a handle must be released using noti_ex_item_destroy().
 * @param[out] handle The notification_ex item handle
 * @param[in] id The id of notification_ex item
 * @param[in] image_path The absolute path of image
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
	noti_ex_item_h image_item = NULL;
	char *image_path[BUFLEN];

	char *shared_path = app_get_shared_resource_path();
	snprintf(image_path, BUFLEN, "%simage.png", shared_path);
	free(shared_path);

	ret = noti_ex_item_image_create(&image_item, "image_id", image_path);
}
 * @endcode
 */
int noti_ex_item_image_create(noti_ex_item_h *handle, const char *id, const char *image_path) TIZEN_DEPRECATED_API;

/**
 * @deprecated Deprecated since 11.0.
 * @brief Gets the path of image.
 * @since_tizen 5.5
 * @remarks @a image_path must be released using free().
 * @param[in] handle The notification_ex item handle
 * @param[out] image_path The absolute path of image
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter, if the item type is not correct, this error is returned
 * @see #noti_ex_item_h
 * @see noti_ex_item_image_create()
 * @par Sample code:
 * @code
#include <notification_ex.h>

{
	int ret;
	char *path = NULL;

	ret = noti_ex_item_image_get_image_path(image_item, &path);
}
 * @endcode
 */
int noti_ex_item_image_get_image_path(noti_ex_item_h handle, char **image_path) TIZEN_DEPRECATED_API;

/**
 * @}
 */

#ifdef __cplusplus
}
#endif
#endif  /* __TIZEN_APPFW_NOTIFICATION_EX_IMAGE_H__ */
