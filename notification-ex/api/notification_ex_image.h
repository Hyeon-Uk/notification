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

#ifndef CAPI_NOTIFICATION_EX_IMAGE_H_
#define CAPI_NOTIFICATION_EX_IMAGE_H_

#include <notification_ex_item.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Creates the notification_ex item handle with image.
 * @details The notification_ex is made with notification_ex items.
 *          If the user wants to be displayed with image,
 *          notification_ex handle have to be made as image item.
 * @since_tizen 5.5
 * @remarks @a handle must be released using noti_ex_item_destroy().
 * @param[out] handle The notification_ex item handle
 * @param[in] id The id of notification_ex item
 * @param[in] image_path The path of image
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #NOTI_EX_ERROR_OUT_OF_MEMORY out of memory
 * @see #noti_ex_item_h
 * @see #noti_ex_item_destroy
 * @par Sample code:
 * @code
#include <notification_ex_image.h>

{
	int ret;
	noti_ex_item_h image_item = NULL;

	ret = noti_ex_item_image_create(&image_item, "image_id", "image_path");
}
 * @endcode
 */
int noti_ex_item_image_create(noti_ex_item_h *handle, const char *id, const char *image_path);

/**
 * @brief Gets the path of image.
 * @since_tizen 5.5
 * @remarks @a image_path must be released using free().
 * @param[in] handle The notification_ex item handle
 * @param[out] image_path The path of image
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_item_h
 * @see #noti_ex_item_image_create
 * @par Sample code:
 * @code
#include <notification_ex_image.h>

{
	int ret;
	char *path = NULL;

	ret = noti_ex_item_image_get_image_path(button_item, &path);
}
 * @endcode
 */
int noti_ex_item_image_get_image_path(noti_ex_item_h handle, char **image_path);

#ifdef __cplusplus
}
#endif
#endif  // CAPI_NOTIFICATION_EX_IMAGE_H_
