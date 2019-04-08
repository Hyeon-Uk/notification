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

#ifndef CAPI_NOTIFICATION_EX_ITEM_H_
#define CAPI_NOTIFICATION_EX_ITEM_H_

#include <bundle.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RECEIVERGROUP_PANEL "tizen.org/receiver/panel" /**< Panel */
#define RECEIVERGROUP_TICKER "tizen.org/receiver/ticker" /**< Ticker */
#define RECEIVERGROUP_LOCKSCREEN "tizen.org/receiver/lockscreen" /**< Lockscreen */
#define RECEIVERGROUP_INDICATOR "tizen.org/receiver/indicator" /**< Indicator */
#define RECEIVERGROUP_POPUP "tizen.org/receiver/popup" /**< Popup */

/**
 * @brief Enumeration for notification_ex item types.
 * @since_tizen 5.5
 */
typedef enum _noti_ex_item_type {
	NOTI_EX_ITEM_TYPE_NULL, /**< notification_ex item is null */
	NOTI_EX_ITEM_TYPE_TEXT, /**< notificatin_ex item for text */
	NOTI_EX_ITEM_TYPE_IMAGE, /**< notificatin_ex item for image */
	NOTI_EX_ITEM_TYPE_ICON, /**< notificatin_ex item for icon */
	NOTI_EX_ITEM_TYPE_BUTTON, /**< notificatin_ex item for button */
	NOTI_EX_ITEM_TYPE_CHAT_MESSAGE, /**< notificatin_ex item for chat message */
	NOTI_EX_ITEM_TYPE_CHECKBOX, /**< notificatin_ex item for check box */
	NOTI_EX_ITEM_TYPE_ICON_TEXT, /**< notificatin_ex item for icon and text */
	NOTI_EX_ITEM_TYPE_INPUT_SELECTOR, /**< notificatin_ex item for input selector */
	NOTI_EX_ITEM_TYPE_GROUP, /**< notificatin_ex item for group */
	NOTI_EX_ITEM_TYPE_ENTRY, /**< notificatin_ex item for entry */
	NOTI_EX_ITEM_TYPE_PROGRESS, /**< notificatin_ex item for progress */
	NOTI_EX_ITEM_TYPE_TIME, /**< notificatin_ex item for time */
	NOTI_EX_ITEM_TYPE_CUSTOM = 100 /**< notificatin_ex item for custom item */
} noti_ex_item_type_e;

/**
 * @brief Enumeration for notification_ex action type.
 * @since_tizen 5.5
 */
typedef enum _noti_ex_action_type {
	NOTI_EX_ACTION_TYPE_NULL, /**< notification_ex action is null */
	NOTI_EX_ACTION_TYPE_APP_CONTROL, /**< notification_ex action with app control */
	NOTI_EX_ACTION_TYPE_VISIBILITY, /**< notification_ex action for visibility */
	NOTI_EX_ACTION_TYPE_CUSTOM = 100, /**< notification_ex action for custom action */
} noti_ex_action_type;

/**
 * @brief Enumeration for notification_ex item policy.
 * @since_tizen 5.5
 */
typedef enum _noti_ex_item_policy {
	NOTI_EX_ITEM_POLICY_NONE = 0, /**< no policy */
	NOTI_EX_ITEM_POLICY_ON_BOOT_CLEAR = 1 << 0, /**< on boot clear */
	NOTI_EX_ITEM_POLICY_SIM_MODE = 1 << 1, /**< sim mode */
} noti_ex_item_policy_e;

/**
 * @brief The handle for the color information.
 * @since_tizen 5.5
 */
typedef void* noti_ex_color_h;

/**
 * @brief Creates the notification_ex color handle.
 * @details Color information can be defined as notification_ex color handle
 *          with alpha, red, green, blue value.
 * @since_tizen 5.5
 * @remarks @a handle must be released using noti_ex_color_destroy().
 * @param[out] handle The notification_ex color handle
 * @param[in] a The alpha value
 * @param[in] r The red value
 * @param[in] g The green value
 * @param[in] b The blue value
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #NOTI_EX_ERROR_OUT_OF_MEMORY out of memory
 * @see #noti_ex_color_h
 * @see #noti_ex_color_destroy
 * @par Sample code:
 * @code
#include <notification_ex_item.h>

{
	int ret;
	noti_ex_color_h color = NULL;

	ret = noti_ex_color_create(&color, 0, 0, 0, 0);
}
 * @endcode
 */
int noti_ex_color_create(noti_ex_color_h *handle, unsigned char a, unsigned char r, unsigned char g, unsigned char b);

/**
 * @brief Releases notification_ex color handle.
 * @since_tizen 5.5
 * @param[in] handle The notification_ex color handle
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_color_h
 * @par Sample code:
 * @code
#include <notification_ex_item.h>

{
	int ret;

	ret = noti_ex_color_destroy(color);
}
 * @endcode
 */
int noti_ex_color_destroy(noti_ex_color_h handle);

/**
 * @brief Gets the alpha value of color.
 * @since_tizen 5.5
 * @param[in] handle The notification_ex color handle
 * @param[out] val The alpha value
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_color_h
 * @see #noti_ex_color_create
 * @par Sample code:
 * @code
#include <notification_ex_item.h>

{
	int ret;
	unsigned char alpha;

	ret = noti_ex_color_get_alpha(color, &alpha);
}
 * @endcode
 */
int noti_ex_color_get_alpha(noti_ex_color_h handle, unsigned char *val);

/**
 * @brief Gets the red value of color.
 * @since_tizen 5.5
 * @param[in] handle The notification_ex color handle
 * @param[out] val The red value
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_color_h
 * @see #noti_ex_color_create
 * @par Sample code:
 * @code
#include <notification_ex_item.h>

{
	int ret;
	unsigned char red;

	ret = noti_ex_color_get_red(color, &red);
}
 * @endcode
 */
int noti_ex_color_get_red(noti_ex_color_h handle, unsigned char *val);

/**
 * @brief Gets the green value of color.
 * @since_tizen 5.5
 * @param[in] handle The notification_ex color handle
 * @param[out] val The green value
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_color_h
 * @see #noti_ex_color_create
 * @par Sample code:
 * @code
#include <notification_ex_item.h>

{
	int ret;
	unsigned char green;

	ret = noti_ex_color_get_green(color, &green);
}
 * @endcode
 */
int noti_ex_color_get_green(noti_ex_color_h handle, unsigned char *val);

/**
 * @brief Gets the blue value of color.
 * @since_tizen 5.5
 * @param[in] handle The notification_ex color handle
 * @param[out] val The blue value
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_color_h
 * @see #noti_ex_color_create
 * @par Sample code:
 * @code
#include <notification_ex_item.h>

{
	int ret;
	unsigned char blue;

	ret = noti_ex_color_get_blue(color, &blue);
}
 * @endcode
 */
int noti_ex_color_get_blue(noti_ex_color_h handle, unsigned char *val);

/**
 * @brief The handle for the padding information.
 * @since_tizen 5.5
 */
typedef void* noti_ex_padding_h;

/**
 * @brief Creates the notification_ex padding handle.
 * @details Padding information can be defined as notification_ex padding handle
 *          with left, top, right, bottom value.
 * @since_tizen 5.5
 * @remarks @a handle must be released using noti_ex_padding_destroy().
 * @param[out] handle The notification_ex padding handle
 * @param[in] left The left value
 * @param[in] top The top value
 * @param[in] right The right value
 * @param[in] bottom The bottom value
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #NOTI_EX_ERROR_OUT_OF_MEMORY out of memory
 * @see #noti_ex_padding_h
 * @see #noti_ex_padding_destroy
 * @par Sample code:
 * @code
#include <notification_ex_item.h>

{
	int ret;
	noti_ex_padding_h padding = NULL;

	ret = noti_ex_padding_create(&padding, 0, 0, 0, 0);
}
 * @endcode
 */
int noti_ex_padding_create(noti_ex_padding_h *handle, int left, int top, int right, int bottom);

/**
 * @brief Releases notification_ex padding handle.
 * @since_tizen 5.5
 * @param[in] handle The notification_ex padding handle
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_padding_h
 * @par Sample code:
 * @code
#include <notification_ex_item.h>

{
	int ret;

	ret = noti_ex_padding_destroy(padding);
}
 * @endcode
 */
int noti_ex_padding_destroy(noti_ex_padding_h handle);

/**
 * @brief Gets the left value of padding.
 * @since_tizen 5.5
 * @param[in] handle The notification_ex padding handle
 * @param[out] val The left value
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_padding_h
 * @see #@see #noti_ex_padding_create
 * @par Sample code:
 * @code
#include <notification_ex_item.h>

{
	int ret;
	int left;

	ret = noti_ex_padding_get_left(padding, &left);
}
 * @endcode
 */
int noti_ex_padding_get_left(noti_ex_padding_h handle, int *val);

/**
 * @brief Gets the top value of padding.
 * @since_tizen 5.5
 * @param[in] handle The notification_ex padding handle
 * @param[out] val The top value
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_padding_h
 * @see #@see #noti_ex_padding_create
 * @par Sample code:
 * @code
#include <notification_ex_item.h>

{
	int ret;
	int top;

	ret = noti_ex_padding_get_top(padding, &top);
}
 * @endcode
 */
int noti_ex_padding_get_top(noti_ex_padding_h handle, int *val);

/**
 * @brief Gets the right value of padding.
 * @since_tizen 5.5
 * @param[in] handle The notification_ex padding handle
 * @param[out] val The right value
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_padding_h
 * @see #@see #noti_ex_padding_create
 * @par Sample code:
 * @code
#include <notification_ex_item.h>

{
	int ret;
	int right;

	ret = noti_ex_padding_get_right(padding, &right);
}
 * @endcode
 */
int noti_ex_padding_get_right(noti_ex_padding_h handle, int *val);

/**
 * @brief Gets the bottom value of padding.
 * @since_tizen 5.5
 * @param[in] handle The notification_ex padding handle
 * @param[out] val The bottom value
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_padding_h
 * @see #@see #noti_ex_padding_create
 * @par Sample code:
 * @code
#include <notification_ex_item.h>

{
	int ret;
	int bottom;

	ret = noti_ex_padding_get_bottom(padding, &bottom);
}
 * @endcode
 */
int noti_ex_padding_get_bottom(noti_ex_padding_h handle, int *val);

/**
 * @brief The handle for the geometry information.
 * @since_tizen 5.5
 */
typedef void* noti_ex_geometry_h;

/**
 * @brief Creates the notification_ex geometry handle.
 * @details Geometry information can be defined as notification_ex geometry handle
 *          with x, y, width, height value.
 * @since_tizen 5.5
 * @remarks @a handle must be released using noti_ex_geometry_destroy().
 * @param[out] handle The notification_ex geometry handle
 * @param[in] x The x value
 * @param[in] y The y value
 * @param[in] w The width value
 * @param[in] h The height value
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #NOTI_EX_ERROR_OUT_OF_MEMORY out of memory
 * @see #noti_ex_geometry_h
 * @see #noti_ex_geometry_destroy
 * @par Sample code:
 * @code
#include <notification_ex_item.h>

{
	int ret;
	noti_ex_geometry_h geometry = NULL;

	ret = noti_ex_geometry_create(&geometry, 0, 0, 0, 0);
}
 * @endcode
 */
int noti_ex_geometry_create(noti_ex_geometry_h *handle, int x, int y, int w, int h);

/**
 * @brief Releases the notification_ex geometry handle.
 * @since_tizen 5.5
 * @param[in] handle The notification_ex geometry handle
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_geometry_h
 * @par Sample code:
 * @code
#include <notification_ex_item.h>

{
	int ret;

	ret = noti_ex_geometry_destroy(geometry);
}
 * @endcode
 */
int noti_ex_geometry_destroy(noti_ex_geometry_h handle);

/**
 * @brief Gets the x position value.
 * @since_tizen 5.5
 * @param[in] handle The notification_ex geometry handle
 * @param[out] val The x value
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_geometry_h
 * @see #noti_ex_geometry_create
 * @par Sample code:
 * @code
#include <notification_ex_item.h>

{
	int ret;
	int x;

	ret = noti_ex_geometry_get_x(geometry, &x);
}
 * @endcode
 */
int noti_ex_geometry_get_x(noti_ex_geometry_h handle, int *val);

/**
 * @brief Gets the y position value.
 * @since_tizen 5.5
 * @param[in] handle The notification_ex geometry handle
 * @param[out] val The y value
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_geometry_h
 * @see #noti_ex_geometry_create
 * @par Sample code:
 * @code
#include <notification_ex_item.h>

{
	int ret;
	int y;

	ret = noti_ex_geometry_get_y(geometry, &y);
}
 * @endcode
 */
int noti_ex_geometry_get_y(noti_ex_geometry_h handle, int *val);

/**
 * @brief Gets the width value.
 * @since_tizen 5.5
 * @param[in] handle The notification_ex geometry handle
 * @param[out] val The width value
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_geometry_h
 * @see #noti_ex_geometry_create
 * @par Sample code:
 * @code
#include <notification_ex_item.h>

{
	int ret;
	int width;

	ret = noti_ex_geometry_get_width(geometry, &width);
}
 * @endcode
 */
int noti_ex_geometry_get_width(noti_ex_geometry_h handle, int *val);

/**
 * @brief Gets the height value.
 * @since_tizen 5.5
 * @param[in] handle The notification_ex geometry handle
 * @param[out] val The height value
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_geometry_h
 * @see #noti_ex_geometry_create
 * @par Sample code:
 * @code
#include <notification_ex_item.h>

{
	int ret;
	int height;

	ret = noti_ex_geometry_get_height(geometry, &height);
}
 * @endcode
 */
int noti_ex_geometry_get_height(noti_ex_geometry_h handle, int *val);

/**
 * @brief The handle for the style information.
 * @since_tizen 5.5
 */
typedef void* noti_ex_style_h;

/**
 * @brief Creates the notification_ex style handle.
 * @details Style information can be defined as notification_ex style handle
 *          with color, padding, geometry information.
 *          The color is defined notification_ex color handle.
 *          The padding is defined notification_ex padding handle.
 *          The geometry is defined notification_ex geometry handle.
 * @since_tizen 5.5
 * @remarks @a handle must be released using noti_ex_style_destroy().
 * @param[out] handle The notification_ex padding handle
 * @param[in] color The notification_ex color handle
 * @param[in] padding The notification_ex padding handle
 * @param[in] geometry The notification_ex geometry handle
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #NOTI_EX_ERROR_OUT_OF_MEMORY out of memory
 * @see #noti_ex_style_h
 * @see #noti_ex_color_h
 * @see #noti_ex_padding_h
 * @see #noti_ex_geometry_h
 * @see #noti_ex_style_destroy
 * @par Sample code:
 * @code
#include <notification_ex_item.h>

{
	int ret;
	noti_ex_style_h style = NULL;
	noti_ex_color_h color = NULL;
	noti_ex_padding_h padding = NULL;
	noti_ex_geometry_h geometry = NULL;

	ret = noti_ex_color_create(&color, 0, 0, 0, 0);
	ret = noti_ex_padding_create(&padding, 0, 0, 0, 0);
	ret = noti_ex_geometry_create(&geometry, 0, 0, 0, 0);

	ret = noti_ex_style_create(&style, color, padding, geometry);
}
 * @endcode
 */
int noti_ex_style_create(noti_ex_style_h *handle, noti_ex_color_h color, noti_ex_padding_h padding, noti_ex_geometry_h geometry);

/**
 * @brief Releases the notification_ex style handle.
 * @since_tizen 5.5
 * @param[in] handle The notification_ex style handle
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_style_h
 * @par Sample code:
 * @code
#include <notification_ex_item.h>

{
	int ret;

	ret = noti_ex_style_destroy(style);
}
 * @endcode
 */
int noti_ex_style_destroy(noti_ex_style_h handle);

/**
 * @brief Gets the padding information.
 * @since_tizen 5.5
 * @remarks @a padding must be released using noti_ex_padding_destroy().
 * @param[in] handle The notification_ex style handle
 * @param[out] padding The notification_ex padding handle
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_style_h
 * @see #noti_ex_padding_h
 * @see #noti_ex_style_create
 * @par Sample code:
 * @code
#include <notification_ex_item.h>

{
	int ret;
	noti_ex_padding_h padding = NULL;

	ret = noti_ex_style_get_padding(style, &padding);
}
 * @endcode
 */
int noti_ex_style_get_padding(noti_ex_style_h handle, noti_ex_padding_h *padding);

/**
 * @brief Gets the color information.
 * @since_tizen 5.5
 * @remarks @a color must be released using noti_ex_color_destroy().
 * @param[in] handle The notification_ex style handle
 * @param[out] color The notification_ex color handle
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_style_h
 * @see #noti_ex_color_h
 * @see #noti_ex_style_create
 * @par Sample code:
 * @code
#include <notification_ex_item.h>

{
	int ret;
	noti_ex_color_h color = NULL;

	ret = noti_ex_style_get_color(style, &color);
}
 * @endcode
 */
int noti_ex_style_get_color(noti_ex_style_h handle, noti_ex_color_h *color);

/**
 * @brief Gets the geometry information.
 * @since_tizen 5.5
 * @remarks @a geometry must be released using noti_ex_geometry_destroy().
 * @param[in] handle The notification_ex style handle
 * @param[out] geometry The notification_ex geometry handle
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_style_h
 * @see #noti_ex_geometry_h
 * @see #noti_ex_style_create
 * @par Sample code:
 * @code
#include <notification_ex_item.h>

{
	int ret;
	noti_ex_geometry_h geometry = NULL;

	ret = noti_ex_style_get_geometry(style, &geometry);
}
 * @endcode
 */
int noti_ex_style_get_geometry(noti_ex_style_h handle, noti_ex_geometry_h *geometry);

/**
 * @brief The handle for the led information.
 * @since_tizen 5.5
 */
typedef void* noti_ex_led_info_h;

/**
 * @brief Creates the notification_ex led info handle.
 * @details LED information can be defined as notification_ex led handle
 *          with color information.
 *          The color is defined notification_ex color handle.
 * @since_tizen 5.5
 * @remarks @a handle must be released using noti_ex_led_info_destroy().
 * @param[out] handle The notification_ex led info handle
 * @param[in] color The notification_ex color handle
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #NOTI_EX_ERROR_OUT_OF_MEMORY out of memory
 * @see #noti_ex_led_info_h
 * @see #noti_ex_color_h
 * @see #noti_ex_led_info_destroy
 * @par Sample code:
 * @code
#include <notification_ex_item.h>

{
	int ret;
	noti_ex_led_info_h led_info = NULL;
	noti_ex_color_h color = NULL;

	ret = noti_ex_color_create(&color, 0, 0, 0, 0);

	ret = noti_ex_led_info_create(&led_info, color);
}
 * @endcode
 */
int noti_ex_led_info_create(noti_ex_led_info_h *handle, noti_ex_color_h color);

/**
 * @brief Releases the notification_ex led info handle.
 * @since_tizen 5.5
 * @param[in] handle The notification_ex led info handle
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_led_info_h
 * @par Sample code:
 * @code
#include <notification_ex_item.h>

{
	int ret;

	ret = noti_ex_led_info_destroy(led_info);
}
 * @endcode
 */
int noti_ex_led_info_destroy(noti_ex_led_info_h handle);

/**
 * @brief Sets the time period for turning on the led.
 * @since_tizen 5.5
 * @param[in] handle The notification_ex led info handle
 * @param[in] ms The time for turning on the led
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_led_info_h
 * @see #noti_ex_led_info_create
 * @par Sample code:
 * @code
#include <notification_ex_item.h>

{
	int ret;

	ret = noti_ex_led_info_set_on_period(led_info, 5);
}
 * @endcode
 */
int noti_ex_led_info_set_on_period(noti_ex_led_info_h handle, int ms);

/**
 * @brief Gets the time period for turning on the led.
 * @since_tizen 5.5
 * @param[in] handle The notification_ex led info handle
 * @param[out] ms The time for turning on the led
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_led_info_h
 * @see #noti_ex_led_info_create
 * @par Sample code:
 * @code
#include <notification_ex_item.h>

{
	int ret;
	int ms;

	ret = noti_ex_led_info_get_on_period(led_info, &ms);
}
 * @endcode
 */
int noti_ex_led_info_get_on_period(noti_ex_led_info_h handle, int *ms);

/**
 * @brief Sets the time period for turning off the led.
 * @since_tizen 5.5
 * @param[in] handle The notification_ex led info handle
 * @param[in] ms The time for turning off the led
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_led_info_h
 * @see #noti_ex_led_info_create
 * @par Sample code:
 * @code
#include <notification_ex_item.h>

{
	int ret;

	ret = noti_ex_led_info_set_off_period(led_info, 5);
}
 * @endcode
 */
int noti_ex_led_info_set_off_period(noti_ex_led_info_h handle, int ms);

/**
 * @brief Gets the time period for turning off the led.
 * @since_tizen 5.5
 * @param[in] handle The notification_ex led info handle
 * @param[out] ms The time for turning off the led
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_led_info_h
 * @see #noti_ex_led_info_create
 * @par Sample code:
 * @code
#include <notification_ex_item.h>

{
	int ret;
	int ms;

	ret = noti_ex_led_info_get_off_period(led_info, &ms);
}
 * @endcode
 */
int noti_ex_led_info_get_off_period(noti_ex_led_info_h handle, int *ms);

/**
 * @brief Gets the color information.
 * @since_tizen 5.5
 * @param[in] handle The notification_ex led info handle
 * @param[out] color The notification_ex color handle
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_led_info_h
 * @see #noti_ex_color_h
 * @see #noti_ex_led_info_create
 * @par Sample code:
 * @code
#include <notification_ex_item.h>

{
	int ret;
	noti_ex_color_h color = NULL;

	ret = noti_ex_led_info_get_color(led_info, &color);
}
 * @endcode
 */
int noti_ex_led_info_get_color(noti_ex_led_info_h handle, noti_ex_color_h *color);

/**
 * @brief The notification_ex action handle
 * @since_tizen 5.5
 */
typedef void* noti_ex_action_h;

/**
 * @brief The notification_ex item handle
 * @since_tizen 5.5
 */
typedef void* noti_ex_item_h;

/**
 * @brief The notification_ex item_info handle
 * @since_tizen 5.5
 */
typedef void* noti_ex_item_info_h;

/**
 * @brief Releases the notification_ex action handle.
 * @since_tizen 5.5
 * @param[in] handle The notification_ex action handle
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_action_h
 * @par Sample code:
 * @code
#include <notification_ex_item.h>

{
	int ret;

	ret = noti_ex_action_destroy(action);
}
 * @endcode
 */
int noti_ex_action_destroy(noti_ex_action_h handle);

/**
 * @brief Gets the type of action.
 * @since_tizen 5.5
 * @param[in] handle The notification_ex action handle
 * @param[out] type The type of action
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_action_h
 * @par Sample code:
 * @code
#include <notification_ex_item.h>

{
	int ret;
	int type;

	ret = noti_ex_action_get_type(action,&type);
}
 * @endcode
 */
int noti_ex_action_get_type(noti_ex_action_h handle, int *type);

/**
 * @brief Gets the local state of action.
 * @since_tizen 5.5
 * @param[in] handle The notification_ex action handle
 * @param[out] local The local state of action
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_action_h
 * @par Sample code:
 * @code
#include <notification_ex_item.h>

{
	int ret;
	bool local;

	ret = noti_ex_action_is_local(action, &local);
}
 * @endcode
 */
int noti_ex_action_is_local(noti_ex_action_h handle, bool *local);

/**
 * @brief Executes the action for notification_ex item.
 * @since_tizen 5.5
 * @param[in] handle The notification_ex action handle
 * @param[in] item The notification_ex item handle
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_action_h
 * @see #noti_ex_item_h
 * @par Sample code:
 * @code
#include <notification_ex_item.h>

{
	int ret;
	noti_ex_item_h button_item = NULL;
	noti_ex_action_h action = NULL;

	ret = noti_ex_action_visibility_create(&action, NULL);
	ret = noti_ex_item_button_create(&button_item, "button_id", "button_title");
	ret = noti_ex_item_set_action(button_item, action);

	ret = noti_ex_action_execute(action, button_item);
}
 * @endcode
 */
int noti_ex_action_execute(noti_ex_action_h handle, noti_ex_item_h item);

/**
 * @brief Gets the extra data for action.
 * @since_tizen 5.5
 * @remarks @a extra must be released using free().
 * @param[in] handle The notification_ex action handle
 * @param[out] extra The extra data
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_action_h
 * @par Sample code:
 * @code
#include <notification_ex_item.h>

{
	int ret;
	char *extra = NULL;

	ret = noti_ex_action_get_extra(action, &extra);
}
 * @endcode
 */
int noti_ex_action_get_extra(noti_ex_action_h handle, char **extra);

/**
 * @brief Gets the time to hide the notification_ex item.
 * @since_tizen 5.5
 * @param[in] handle The notification_ex item info handle
 * @param[out] hide_time The time to hide notification
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_item_info_h
 * @see #noti_ex_item_get_info
 * @par Sample code:
 * @code
#include <notification_ex_item.h>

{
	int ret;
	int hide_time;
	noti_ex_item_info_h item_info = NULL;

	ret = noti_ex_item_get_info(item_handle, &item_info);

	ret = noti_ex_item_info_get_hide_time(item_info, &hide_time);
}
 * @endcode
 */
int noti_ex_item_info_get_hide_time(noti_ex_item_info_h handle, int *hide_time);

/**
 * @brief Sets the time to hide the notification_ex item.
 * @since_tizen 5.5
 * @param[in] handle The notification_ex item info handle
 * @param[in] hide_time The time to hide notification
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_item_info_h
 * @see #noti_ex_item_get_info
 * @par Sample code:
 * @code
#include <notification_ex_item.h>

{
	int ret;
	noti_ex_item_info_h item_info = NULL;

	ret = noti_ex_item_get_info(item_handle, &item_info);

	ret = noti_ex_item_info_set_hide_time(action, 5);
}
 * @endcode
 */
int noti_ex_item_info_set_hide_time(noti_ex_item_info_h handle, int hide_time);

/**
 * @brief Gets the time to delete the notification_ex item.
 * @since_tizen 5.5
 * @param[in] handle The notification_ex item info handle
 * @param[out] delete_time The time to delete notification
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_item_info_h
 * @see #noti_ex_item_get_info
 * @par Sample code:
 * @code
#include <notification_ex_item.h>

{
	int ret;
	int delete_time;
	noti_ex_item_info_h item_info = NULL;

	ret = noti_ex_item_get_info(item_handle, &item_info);

	ret = noti_ex_item_info_get_delete_time(item_info, &delete_time);
}
 * @endcode
 */
int noti_ex_item_info_get_delete_time(noti_ex_item_info_h handle, int *delete_time);

/**
 * @brief Sets the time to delete the notification_ex item.
 * @since_tizen 5.5
 * @param[in] handle The notification_ex item info handle
 * @param[out] delete_time The time to delete notification
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_item_info_h
 * @see #noti_ex_item_get_info
 * @par Sample code:
 * @code
#include <notification_ex_item.h>

{
	int ret;
	noti_ex_item_info_h item_info = NULL;

	ret = noti_ex_item_get_info(item_handle, &item_info);

	ret = noti_ex_item_info_set_delete_time(item_info, 5);
}
 * @endcode
 */
int noti_ex_item_info_set_delete_time(noti_ex_item_info_h handle, int delete_time);

/**
 * @brief Gets the time information.
 * @since_tizen 5.5
 * @param[in] handle The notification_ex item info handle
 * @param[out] time The time information
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_item_info_h
 * @see #noti_ex_item_get_info
 * @par Sample code:
 * @code
#include <notification_ex_item.h>

{
	int ret;
	time_t time;
	noti_ex_item_info_h item_info = NULL;

	ret = noti_ex_item_get_info(item_handle, &item_info);

	ret = noti_ex_item_info_get_time(item_info, &time);
}
 * @endcode
 */
int noti_ex_item_info_get_time(noti_ex_item_info_h handle, time_t *time);

/**
 * @brief Releases the notification_ex item handle.
 * @since_tizen 5.5
 * @param[in] handle The notification_ex item handle
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_item_h
 * @par Sample code:
 * @code
#include <notification_ex_item.h>

{
	int ret;

	ret = noti_ex_item_destroy(item_handle);
}
 * @endcode
 */
int noti_ex_item_destroy(noti_ex_item_h handle);

/**
 * @brief Finds the notification_ex item handle by notification_ex item id.
 * @since_tizen 5.5
 * @param[in] handle The notification_ex item handle
 * @param[in] id The id of notification_ex item
 * @param[in] item The notification_ex item handle
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_item_h
 * @par Sample code:
 * @code
#include <notification_ex_item.h>

{
	int ret;
	noti_ex_item_h item = NULL;

	ret = noti_ex_item_find_by_id(item_handle, "find_id", &item);
}
 * @endcode
 */
int noti_ex_item_find_by_id(noti_ex_item_h handle, const char *id, noti_ex_item_h *item);

/**
 * @brief Gets the type of notification_ex item.
 * @since_tizen 5.5
 * @param[in] handle The notification_ex item handle
 * @param[out] type The type of notification_ex item
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_item_h
 * @par Sample code:
 * @code
#include <notification_ex_item.h>

{
	int ret;
	int type;

	ret = noti_ex_item_get_type(item_handle, &type);
}
 * @endcode
 */
int noti_ex_item_get_type(noti_ex_item_h handle, int *type);

/**
 * @brief Gets the path of shared file.
 * @since_tizen 5.5
 * @remarks @a path must be released after releasing all paths using free()
 * @param[in] handle The notification_ex item handle
 * @param[out] path The path of shared file
 * @param[out] count The count of path
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_item_h
 * @par Sample code:
 * @code
#include <notification_ex_item.h>

{
	int ret;
	char **path = NULL;
	int count;

	ret = noti_ex_item_get_shared_path(item_handle, &path, &count);
}
 * @endcode
 */
int noti_ex_item_get_shared_path(noti_ex_item_h handle, char ***path, int *count);

/**
 * @brief Gets the id of notification_ex item.
 * @since_tizen 5.5
 * @remarks @a id must be released using free().
 * @param[in] handle The notification_ex item handle
 * @param[out] id The id of notification_ex item
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_item_h
 * @par Sample code:
 * @code
#include <notification_ex_item.h>

{
	int ret;
	char *id = NULL;

	ret = noti_ex_item_get_id(item_handle, &id);
}
 * @endcode
 */
int noti_ex_item_get_id(noti_ex_item_h handle, char **id);

/**
 * @brief Sets the id of notification_ex item.
 * @since_tizen 5.5
 * @param[in] handle The notification_ex item handle
 * @param[in] id The id of notification_ex item
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_item_h
 * @par Sample code:
 * @code
#include <notification_ex_item.h>

{
	int ret;

	ret = noti_ex_item_set_id(item_handle, "new_id");
}
 * @endcode
 */
int noti_ex_item_set_id(noti_ex_item_h handle, const char *id);

/**
 * @brief Gets the action for notification_ex item.
 * @details The action is defined as notification_ex action handle.
 * @since_tizen 5.5
 * @param[in] handle The notification_ex item handle
 * @param[out] action The notification_ex action handle
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_item_h
 * @see #noti_ex_action_h
 * @par Sample code:
 * @code
#include <notification_ex_item.h>

{
	int ret;
	noti_ex_action_h action = NULL;

	ret = noti_ex_item_get_action(item_handle, &action);
}
 * @endcode
 */
int noti_ex_item_get_action(noti_ex_item_h handle, noti_ex_action_h *action);

/**
 * @brief Sets the action for notification_ex item.
 * @details The action is defined as notification_ex action handle.
 * @since_tizen 5.5
 * @param[in] handle The notification_ex item handle
 * @param[in] action The notification_ex action handle
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_item_h
 * @see #noti_ex_action_h
 * @par Sample code:
 * @code
#include <notification_ex_item.h>

{
	int ret;
	noti_ex_item_h item_handle = NULL;
	noti_ex_action_h action = NULL;

	ret = noti_ex_action_visibility_create(&action, NULL);
	ret = noti_ex_item_button_create(&item_handle, "button_id", "button_title");

	ret = noti_ex_item_set_action(item_handle, action);
}
 * @endcode
 */
int noti_ex_item_set_action(noti_ex_item_h handle, noti_ex_action_h action);

/**
 * @brief Gets the style for notification_ex item.
 * @details The style is defined as notification_ex style handle.
 * @since_tizen 5.5
 * @param[in] handle The notification_ex item handle
 * @param[out] style The notification_ex style handle
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_item_h
 * @see #noti_ex_style_h
 * @par Sample code:
 * @code
#include <notification_ex_item.h>

{
	int ret;
	noti_ex_style_h style = NULL;

	ret = noti_ex_item_get_style(item_handle, &style);
}
 * @endcode
 */
int noti_ex_item_get_style(noti_ex_item_h handle, noti_ex_style_h *style);

/**
 * @brief Sets the style for notification_ex item.
 * @details The style is defined as notification_ex style handle.
 * @since_tizen 5.5
 * @param[in] handle The notification_ex item handle
 * @param[in] style The notification_ex style handle
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_item_h
 * @see #noti_ex_style_h
 * @see #noti_ex_style_create
 * @par Sample code:
 * @code
#include <notification_ex_item.h>

{
	int ret;

	ret = noti_ex_item_set_style(item_handle, style);
}
 * @endcode
 */
int noti_ex_item_set_style(noti_ex_item_h handle, noti_ex_style_h style);

/**
 * @brief Sets the visible state for notification_ex item.
 * @since_tizen 5.5
 * @param[in] handle The notification_ex item handle
 * @param[in] visible The visible state
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_item_h
 * @par Sample code:
 * @code
#include <notification_ex_item.h>

{
	int ret;

	ret = noti_ex_item_set_visible(item_handle, true);
}
 * @endcode
 */
int noti_ex_item_set_visible(noti_ex_item_h handle, bool visible);

/**
 * @brief Gets the visible state for notification_ex item.
 * @since_tizen 5.5
 * @param[in] handle The notification_ex item handle
 * @param[out] visible The visible state
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_item_h
 * @par Sample code:
 * @code
#include <notification_ex_item.h>

{
	int ret;
	bool visible;

	ret = noti_ex_item_get_visible(item_handle, &visible);
}
 * @endcode
 */
int noti_ex_item_get_visible(noti_ex_item_h handle, bool *visible);

/**
 * @brief Sets the enable state for notification_ex item.
 * @since_tizen 5.5
 * @param[in] handle The notification_ex item handle
 * @param[in] enable The enable state
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_item_h
 * @par Sample code:
 * @code
#include <notification_ex_item.h>

{
	int ret;

	ret = noti_ex_item_set_enable(item_handle, true);
}
 * @endcode
 */
int noti_ex_item_set_enable(noti_ex_item_h handle, bool enable);

/**
 * @brief Gets the enable state for notification_ex item.
 * @since_tizen 5.5
 * @param[in] handle The notification_ex item handle
 * @param[in] enable The enable state
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_item_h
 * @par Sample code:
 * @code
#include <notification_ex_item.h>

{
	int ret;
	bool enable;

	ret = noti_ex_item_get_enable(item_handle, &enable);
}
 * @endcode
 */
int noti_ex_item_get_enable(noti_ex_item_h handle, bool *enable);

/**
 * @brief Adds the receiver group for notification_ex item.
 * @since_tizen 5.5
 * @param[in] handle The notification_ex item handle
 * @param[in] receiver_group The receiver group
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_item_h
 * @par Sample code:
 * @code
#include <notification_ex_item.h>

{
	int ret;

	ret = noti_ex_item_add_receiver(item_handle, RECEIVERGROUP_PANEL);
}
 * @endcode
 */
int noti_ex_item_add_receiver(noti_ex_item_h handle, const char *receiver_group);

/**
 * @brief Removes the receiver group of notification_ex item.
 * @since_tizen 5.5
 * @param[in] handle The notification_ex item handle
 * @param[in] receiver_group The receiver group
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_item_h
 * @par Sample code:
 * @code
#include <notification_ex_item.h>

{
	int ret;

	ret = noti_ex_item_remove_receiver(item_handle, RECEIVERGROUP_PANEL);
}
 * @endcode
 */
int noti_ex_item_remove_receiver(noti_ex_item_h handle, const char *receiver_group);

/**
 * @brief Gets the list of receiver group for notification_ex item.
 * @since_tizen 5.5
 * @remarks @a list must be released after releasing all members of list using free()
 * @param[in] handle The notification_ex item handle
 * @param[out] list The list of receiver group
 * @param[out] count The count of list
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_item_h
 * @par Sample code:
 * @code
#include <notification_ex_item.h>

{
	int ret;
	char **list = NULL;
	int count;

	ret = noti_ex_item_get_receiver_list(item_handle, &list, &count);
}
 * @endcode
 */
int noti_ex_item_get_receiver_list(noti_ex_item_h handle, char ***list, int *count);

/**
 * @brief Sets the policy for notification_ex item.
 * @since_tizen 5.5
 * @param[in] handle The notification_ex item handle
 * @param[in] policy The policy for notification_ex item
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_item_h
 * @par Sample code:
 * @code
#include <notification_ex_item.h>

{
	int ret;

	ret = noti_ex_item_set_policy(item_handle, NOTI_EX_ITEM_POLICY_ON_BOOT_CLEAR);
}
 * @endcode
 */
int noti_ex_item_set_policy(noti_ex_item_h handle, int policy);

/**
 * @brief Gets the policy for notification_ex item.
 * @since_tizen 5.5
 * @param[in] handle The notification_ex item handle
 * @param[out] policy The policy for notification_ex item
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_item_h
 * @par Sample code:
 * @code
#include <notification_ex_item.h>

{
	int ret;
	int policy;

	ret = noti_ex_item_get_policy(item_handle, &policy);
}
 * @endcode
 */
int noti_ex_item_get_policy(noti_ex_item_h handle, int *policy);

/**
 * @brief Gets the channel for notification_ex item.
 * @since_tizen 5.5
 * @remarks @a channel must be released using free().
 * @param[in] handle The notification_ex item handle
 * @param[out] channel The channel for notification_ex item
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_item_h
 * @par Sample code:
 * @code
#include <notification_ex_item.h>

{
	int ret;
	char *channel;

	ret = noti_ex_item_get_channel(item_handle, &channel);
}
 * @endcode
 */
int noti_ex_item_get_channel(noti_ex_item_h handle, char **channel);

/**
 * @brief Sets the channel for notification_ex item.
 * @since_tizen 5.5
 * @param[in] handle The notification_ex item handle
 * @param[in] channel The channel for notification_ex item
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_item_h
 * @par Sample code:
 * @code
#include <notification_ex_item.h>

{
	int ret;

	ret = noti_ex_item_set_channel(item_handle, "channel");
}
 * @endcode
 */
int noti_ex_item_set_channel(noti_ex_item_h handle, const char *channel);

/**
 * @brief Sets the led for notification_ex item.
 * @details The LED information is defined as notification_ex led info handle
 * @since_tizen 5.5
 * @param[in] handle The notification_ex item handle
 * @param[in] led The notification_ex led info item
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_item_h
 * @see #noti_ex_led_info_h
 * @see #noti_ex_led_info_create
 * @par Sample code:
 * @code
#include <notification_ex_item.h>

{
	int ret;

	ret = noti_ex_item_set_led_info(item_handle, led_info);
}
 * @endcode
 */
int noti_ex_item_set_led_info(noti_ex_item_h handle, noti_ex_led_info_h led);

/**
 * @brief Gets the led option for notification_ex item.
 * @details The LED information is defined as notification_ex led info handle
 * @since_tizen 5.5
 * @param[in] handle The notification_ex item handle
 * @param[out] led The notification_ex led info item
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_item_h
 * @see #noti_ex_led_info_h
 * @par Sample code:
 * @code
#include <notification_ex_item.h>

{
	int ret;
	noti_ex_led_info_h led_info = NULL;

	ret = noti_ex_item_get_led_info(item_handle, &led_info);
}
 * @endcode
 */
int noti_ex_item_get_led_info(noti_ex_item_h handle, noti_ex_led_info_h *led);

/**
 * @brief Sets the sound path for notification_ex item.
 * @since_tizen 5.5
 * @param[in] handle The notification_ex item handle
 * @param[in] path The sound path for notification_ex item
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_item_h
 * @par Sample code:
 * @code
#include <notification_ex_item.h>

{
	int ret;

	ret = noti_ex_item_set_sound_path(item_handle, "sound_path");
}
 * @endcode
 */
int noti_ex_item_set_sound_path(noti_ex_item_h handle, const char *path);

/**
 * @brief Sets the vibration path for notification_ex item.
 * @since_tizen 5.5
 * @param[in] handle The notification_ex item handle
 * @param[in] path The vibration path for notification_ex item
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_item_h
 * @par Sample code:
 * @code
#include <notification_ex_item.h>

{
	int ret;

	ret = noti_ex_item_set_vibration_path(item_handle, "vibration_path");
}
 * @endcode
 */
int noti_ex_item_set_vibration_path(noti_ex_item_h handle, const char *path);

/**
 * @brief Gets the sound path for notification_ex item.
 * @since_tizen 5.5
 * @remarks @a path must be released using free().
 * @param[in] handle The notification_ex item handle
 * @param[out] path The sound path for notification_ex item
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_item_h
 * @par Sample code:
 * @code
#include <notification_ex_item.h>

{
	int ret;
	char *path = NULL

	ret = noti_ex_item_get_sound_path(item_handle, &path);
}
 * @endcode
 */
int noti_ex_item_get_sound_path(noti_ex_item_h handle, char **path);

/**
 * @brief Gets the vibration path for notification_ex item.
 * @since_tizen 5.5
 * @remarks @a path must be released using free().
 * @param[in] handle The notification_ex item handle
 * @param[out] path The vibration path for notification_ex item
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_item_h
 * @par Sample code:
 * @code
#include <notification_ex_item.h>

{
	int ret;
	char *path = NULL

	ret = noti_ex_item_get_vibration_path(item_handle, &path);
}
 * @endcode
 */
int noti_ex_item_get_vibration_path(noti_ex_item_h handle, char **path);

/**
 * @brief Gets the information for notification_ex item.
 * @details The information of notificaction_ex item is defined as
 *          notification_ex item info handle.
 * @since_tizen 5.5
 * @param[in] handle The notification_ex item handle
 * @param[out] info The notification_ex item info handle
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_item_h
 * @see #noti_ex_item_info_h
 * @par Sample code:
 * @code
#include <notification_ex_item.h>

{
	int ret;
	noti_ex_item_info_h item_info = NULL;

	ret = noti_ex_item_get_info(item_handle, &item_info);
}
 * @endcode
 */
int noti_ex_item_get_info(noti_ex_item_h handle, noti_ex_item_info_h *info);

/**
 * @brief Gets the sender app id of notification_ex item.
 * @since_tizen 5.5
 * @remarks @a id must be released using free().
 * @param[in] handle The notification_ex item handle
 * @param[out] id The sender app id of notification_ex item
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_item_h
 * @par Sample code:
 * @code
#include <notification_ex_item.h>

{
	int ret;
	char *app_id = NULL;

	ret = noti_ex_item_get_sender_app_id(item_handle, &app_id);
}
 * @endcode
 */
int noti_ex_item_get_sender_app_id(noti_ex_item_h handle, char **id);

/**
 * @brief Sets the sender app id of notification_ex item.
 * @since_tizen 5.5
 * @param[in] handle The notification_ex item handle
 * @param[in] id The sender app id of notification_ex item
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_item_h
 * @par Sample code:
 * @code
#include <notification_ex_item.h>

{
	int ret;

	ret = noti_ex_item_set_sender_app_id(item_handle, "sender_appid");
}
 * @endcode
 */
int noti_ex_item_set_sender_app_id(noti_ex_item_h handle, const char *id);

/**
 * @brief Gets the tag of notification_ex item.
 * @since_tizen 5.5
 * @remarks @a tag must be released using free().
 * @param[in] handle The notification_ex item handle
 * @param[out] tag The tag of notification_ex item
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_item_h
 * @par Sample code:
 * @code
#include <notification_ex_item.h>

{
	int ret;
	char *tag = NULL;

	ret = noti_ex_item_get_tag(item_handle, &tag);
}
 * @endcode
 */
int noti_ex_item_get_tag(noti_ex_item_h handle, char **tag);

/**
 * @brief Sets the tag of notification_ex item.
 * @since_tizen 5.5
 * @param[in] handle The notification_ex item handle
 * @param[int] tag The tag of notification_ex item
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_item_h
 * @par Sample code:
 * @code
#include <notification_ex_item.h>

{
	int ret;

	ret = noti_ex_item_set_tag(item_handle, "item_tag");
}
 * @endcode
 */
int noti_ex_item_set_tag(noti_ex_item_h handle, const char *tag);

#ifdef __cplusplus
}
#endif
#endif  // CAPI_NOTIFICATION_EX_ITEM_H_
