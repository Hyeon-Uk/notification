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

#ifndef __TIZEN_APPFW_NOTIFICATION_EX_ITEM_H__
#define __TIZEN_APPFW_NOTIFICATION_EX_ITEM_H__

#include <bundle.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup NOTIFICATION_EX_MODULE
 * @{
 */

/**
 * @deprecated Deprecated since 11.0.
 * @brief Predefined receiver group for the Panel.
 * @since_tizen 5.5
 */
#define NOTI_EX_RECEIVER_GROUP_PANEL "tizen.org/receiver/panel"

/**
 * @deprecated Deprecated since 11.0.
 * @brief Predefined receiver group for the Ticker.
 * @since_tizen 5.5
 */
#define NOTI_EX_RECEIVER_GROUP_TICKER "tizen.org/receiver/ticker"

/**
 * @deprecated Deprecated since 11.0.
 * @brief Predefined receiver group for the Lockscreen.
 * @since_tizen 5.5
 */
#define NOTI_EX_RECEIVER_GROUP_LOCKSCREEN "tizen.org/receiver/lockscreen"

/**
 * @deprecated Deprecated since 11.0.
 * @brief Predefined receiver group for the Indicator.
 * @since_tizen 5.5
 */
#define NOTI_EX_RECEIVER_GROUP_INDICATOR "tizen.org/receiver/indicator"

/**
 * @deprecated Deprecated since 11.0.
 * @brief Predefined receiver group for the Popup.
 * @since_tizen 5.5
 */
#define NOTI_EX_RECEIVER_GROUP_POPUP "tizen.org/receiver/popup"

/**
 * @deprecated Deprecated since 11.0.
 * @brief Default sound.
 * @since_tizen 5.5
 */
#define NOTI_EX_SOUND_TYPE_DEFAULT "__SOUND_TYPE_DEFAULT__"

/**
 * @deprecated Deprecated since 11.0.
 * @brief Default vibration.
 * @since_tizen 5.5
 */
#define NOTI_EX_VIBRATION_TYPE_DEFAULT "__VIBRATION_TYPE_DEFAULT__"

/**
 * @deprecated Deprecated since 11.0.
 * @brief Enumeration for notification_ex item types.
 * @since_tizen 5.5
 */
typedef enum _noti_ex_item_type {
	NOTI_EX_ITEM_TYPE_NULL, /**< Notification_ex item is null */
	NOTI_EX_ITEM_TYPE_TEXT, /**< Notification_ex item for text */
	NOTI_EX_ITEM_TYPE_IMAGE, /**< Notification_ex item for image */
	NOTI_EX_ITEM_TYPE_ICON, /**< Notification_ex item for icon */
	NOTI_EX_ITEM_TYPE_BUTTON, /**< Notification_ex item for button */
	NOTI_EX_ITEM_TYPE_CHAT_MESSAGE, /**< Notification_ex item for chat message */
	NOTI_EX_ITEM_TYPE_CHECKBOX, /**< Notification_ex item for check box */
	NOTI_EX_ITEM_TYPE_ICON_TEXT, /**< Notification_ex item for icon and text */
	NOTI_EX_ITEM_TYPE_INPUT_SELECTOR, /**< Notification_ex item for input selector */
	NOTI_EX_ITEM_TYPE_GROUP, /**< Notification_ex item for group */
	NOTI_EX_ITEM_TYPE_ENTRY, /**< Notification_ex item for entry */
	NOTI_EX_ITEM_TYPE_PROGRESS, /**< Notification_ex item for progress */
	NOTI_EX_ITEM_TYPE_TIME, /**< Notification_ex item for time */
	NOTI_EX_ITEM_TYPE_CUSTOM = 100 /**< Notification_ex item for custom item */
} noti_ex_item_type_e;

/**
 * @deprecated Deprecated since 11.0.
 * @brief Enumeration for notification_ex action type.
 * @since_tizen 5.5
 */
typedef enum _noti_ex_action_type {
	NOTI_EX_ACTION_TYPE_NULL, /**< Notification_ex action is null */
	NOTI_EX_ACTION_TYPE_APP_CONTROL, /**< Notification_ex action with app control */
	NOTI_EX_ACTION_TYPE_VISIBILITY, /**< Notification_ex action for visibility */
	NOTI_EX_ACTION_TYPE_CUSTOM = 100, /**< Notification_ex action for custom action */
} noti_ex_action_type_e;

/**
 * @deprecated Deprecated since 11.0.
 * @brief Enumeration for notification_ex item policy.
 * @since_tizen 5.5
 */
typedef enum _noti_ex_item_policy {
	NOTI_EX_ITEM_POLICY_NONE = 0, /**< No policy */
	NOTI_EX_ITEM_POLICY_ON_BOOT_CLEAR = 1 << 0, /**< On boot clear */
	NOTI_EX_ITEM_POLICY_SIM_MODE = 1 << 1, /**< SIM mode */
	NOTI_EX_ITEM_POLICY_DISABLE_AUTO_DELETE = 1 << 2, /**< Disable auto delete */
} noti_ex_item_policy_e;

/**
 * @deprecated Deprecated since 11.0.
 * @brief Enumeration for main types of notification_ex item.
 * @since_tizen 5.5
 */
typedef enum _noti_ex_item_main_type {
	/**
	* None
	*/
	NOTI_EX_ITEM_MAIN_TYPE_NONE = 0,
	/**
	* Main title.
	* This main type is valid only for items of type #NOTI_EX_ITEM_TYPE_TEXT.
	*/
	NOTI_EX_ITEM_MAIN_TYPE_TITLE,
	/**
	* Main contents.
	* This main type is valid only for items of type #NOTI_EX_ITEM_TYPE_TEXT.
	*/
	NOTI_EX_ITEM_MAIN_TYPE_CONTENTS,
	/**
	* Main icon.
	* This main type is valid only for items of type #NOTI_EX_ITEM_TYPE_IMAGE.
	*/
	NOTI_EX_ITEM_MAIN_TYPE_ICON,
	/**
	* Main button.
	* This main type is valid only for items of type #NOTI_EX_ITEM_TYPE_BUTTON.
	*/
	NOTI_EX_ITEM_MAIN_TYPE_BUTTON,
} noti_ex_item_main_type_e;

/**
 * @deprecated Deprecated since 11.0.
 * @brief The handle for the color information.
 * @since_tizen 5.5
 */
typedef void *noti_ex_color_h;

/**
 * @deprecated Deprecated since 11.0.
 * @brief Creates the notification_ex color handle.
 * @details Color information can be defined as a notification_ex color handle
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
 * @see noti_ex_color_destroy()
 * @par Sample code:
 * @code
#include <notification_ex.h>

{
	int ret;
	noti_ex_color_h color = NULL;

	ret = noti_ex_color_create(&color, 0, 0, 0, 0);
}
 * @endcode
 */
int noti_ex_color_create(noti_ex_color_h *handle, unsigned char a, unsigned char r, unsigned char g, unsigned char b) TIZEN_DEPRECATED_API;

/**
 * @deprecated Deprecated since 11.0.
 * @brief Releases a notification_ex color handle.
 * @since_tizen 5.5
 * @param[in] handle The notification_ex color handle
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_color_h
 * @par Sample code:
 * @code
#include <notification_ex.h>

{
	int ret;

	ret = noti_ex_color_destroy(color);
}
 * @endcode
 */
int noti_ex_color_destroy(noti_ex_color_h handle) TIZEN_DEPRECATED_API;

/**
 * @deprecated Deprecated since 11.0.
 * @brief Gets the alpha value of a color.
 * @since_tizen 5.5
 * @param[in] handle The notification_ex color handle
 * @param[out] val The alpha value
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_color_h
 * @see noti_ex_color_create()
 * @par Sample code:
 * @code
#include <notification_ex.h>

{
	int ret;
	unsigned char alpha;

	ret = noti_ex_color_get_alpha(color, &alpha);
}
 * @endcode
 */
int noti_ex_color_get_alpha(noti_ex_color_h handle, unsigned char *val) TIZEN_DEPRECATED_API;

/**
 * @deprecated Deprecated since 11.0.
 * @brief Gets the red value of a color.
 * @since_tizen 5.5
 * @param[in] handle The notification_ex color handle
 * @param[out] val The red value
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_color_h
 * @see noti_ex_color_create()
 * @par Sample code:
 * @code
#include <notification_ex.h>

{
	int ret;
	unsigned char red;

	ret = noti_ex_color_get_red(color, &red);
}
 * @endcode
 */
int noti_ex_color_get_red(noti_ex_color_h handle, unsigned char *val) TIZEN_DEPRECATED_API;

/**
 * @deprecated Deprecated since 11.0.
 * @brief Gets the green value of a color.
 * @since_tizen 5.5
 * @param[in] handle The notification_ex color handle
 * @param[out] val The green value
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_color_h
 * @see noti_ex_color_create()
 * @par Sample code:
 * @code
#include <notification_ex.h>

{
	int ret;
	unsigned char green;

	ret = noti_ex_color_get_green(color, &green);
}
 * @endcode
 */
int noti_ex_color_get_green(noti_ex_color_h handle, unsigned char *val) TIZEN_DEPRECATED_API;

/**
 * @deprecated Deprecated since 11.0.
 * @brief Gets the blue value of a color.
 * @since_tizen 5.5
 * @param[in] handle The notification_ex color handle
 * @param[out] val The blue value
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_color_h
 * @see noti_ex_color_create()
 * @par Sample code:
 * @code
#include <notification_ex.h>

{
	int ret;
	unsigned char blue;

	ret = noti_ex_color_get_blue(color, &blue);
}
 * @endcode
 */
int noti_ex_color_get_blue(noti_ex_color_h handle, unsigned char *val) TIZEN_DEPRECATED_API;

/**
 * @deprecated Deprecated since 11.0.
 * @brief The handle for the padding information.
 * @since_tizen 5.5
 */
typedef void *noti_ex_padding_h;

/**
 * @deprecated Deprecated since 11.0.
 * @brief Creates the notification_ex padding handle.
 * @details Padding information can be defined as a notification_ex padding handle
 *          with left, right, top and bottom values.
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
 * @see noti_ex_padding_destroy()
 * @par Sample code:
 * @code
#include <notification_ex.h>

{
	int ret;
	noti_ex_padding_h padding = NULL;

	ret = noti_ex_padding_create(&padding, 0, 0, 0, 0);
}
 * @endcode
 */
int noti_ex_padding_create(noti_ex_padding_h *handle, int left, int top, int right, int bottom) TIZEN_DEPRECATED_API;

/**
 * @deprecated Deprecated since 11.0.
 * @brief Releases a notification_ex padding handle.
 * @since_tizen 5.5
 * @param[in] handle The notification_ex padding handle
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_padding_h
 * @par Sample code:
 * @code
#include <notification_ex.h>

{
	int ret;

	ret = noti_ex_padding_destroy(padding);
}
 * @endcode
 */
int noti_ex_padding_destroy(noti_ex_padding_h handle) TIZEN_DEPRECATED_API;

/**
 * @deprecated Deprecated since 11.0.
 * @brief Gets the left value of padding.
 * @since_tizen 5.5
 * @param[in] handle The notification_ex padding handle
 * @param[out] val The left value
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_padding_h
 * @see noti_ex_padding_create()
 * @par Sample code:
 * @code
#include <notification_ex.h>

{
	int ret;
	int left;

	ret = noti_ex_padding_get_left(padding, &left);
}
 * @endcode
 */
int noti_ex_padding_get_left(noti_ex_padding_h handle, int *val) TIZEN_DEPRECATED_API;

/**
 * @deprecated Deprecated since 11.0.
 * @brief Gets the top value of padding.
 * @since_tizen 5.5
 * @param[in] handle The notification_ex padding handle
 * @param[out] val The top value
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_padding_h
 * @see noti_ex_padding_create()
 * @par Sample code:
 * @code
#include <notification_ex.h>

{
	int ret;
	int top;

	ret = noti_ex_padding_get_top(padding, &top);
}
 * @endcode
 */
int noti_ex_padding_get_top(noti_ex_padding_h handle, int *val) TIZEN_DEPRECATED_API;

/**
 * @deprecated Deprecated since 11.0.
 * @brief Gets the right value of padding.
 * @since_tizen 5.5
 * @param[in] handle The notification_ex padding handle
 * @param[out] val The right value
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_padding_h
 * @see noti_ex_padding_create()
 * @par Sample code:
 * @code
#include <notification_ex.h>

{
	int ret;
	int right;

	ret = noti_ex_padding_get_right(padding, &right);
}
 * @endcode
 */
int noti_ex_padding_get_right(noti_ex_padding_h handle, int *val) TIZEN_DEPRECATED_API;

/**
 * @deprecated Deprecated since 11.0.
 * @brief Gets the bottom value of padding.
 * @since_tizen 5.5
 * @param[in] handle The notification_ex padding handle
 * @param[out] val The bottom value
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_padding_h
 * @see noti_ex_padding_create()
 * @par Sample code:
 * @code
#include <notification_ex.h>

{
	int ret;
	int bottom;

	ret = noti_ex_padding_get_bottom(padding, &bottom);
}
 * @endcode
 */
int noti_ex_padding_get_bottom(noti_ex_padding_h handle, int *val) TIZEN_DEPRECATED_API;

/**
 * @deprecated Deprecated since 11.0.
 * @brief The handle for the geometry information.
 * @since_tizen 5.5
 */
typedef void *noti_ex_geometry_h;

/**
 * @deprecated Deprecated since 11.0.
 * @brief Creates the notification_ex geometry handle.
 * @details Geometry information can be defined as a notification_ex geometry handle
 *          with x, y, width and height value.
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
 * @see noti_ex_geometry_destroy()
 * @par Sample code:
 * @code
#include <notification_ex.h>

{
	int ret;
	noti_ex_geometry_h geometry = NULL;

	ret = noti_ex_geometry_create(&geometry, 0, 0, 0, 0);
}
 * @endcode
 */
int noti_ex_geometry_create(noti_ex_geometry_h *handle, int x, int y, int w, int h) TIZEN_DEPRECATED_API;

/**
 * @deprecated Deprecated since 11.0.
 * @brief Releases the notification_ex geometry handle.
 * @since_tizen 5.5
 * @param[in] handle The notification_ex geometry handle
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_geometry_h
 * @par Sample code:
 * @code
#include <notification_ex.h>

{
	int ret;

	ret = noti_ex_geometry_destroy(geometry);
}
 * @endcode
 */
int noti_ex_geometry_destroy(noti_ex_geometry_h handle) TIZEN_DEPRECATED_API;

/**
 * @deprecated Deprecated since 11.0.
 * @brief Gets the x position value.
 * @since_tizen 5.5
 * @param[in] handle The notification_ex geometry handle
 * @param[out] val The x value
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_geometry_h
 * @see noti_ex_geometry_create()
 * @par Sample code:
 * @code
#include <notification_ex.h>

{
	int ret;
	int x;

	ret = noti_ex_geometry_get_x(geometry, &x);
}
 * @endcode
 */
int noti_ex_geometry_get_x(noti_ex_geometry_h handle, int *val) TIZEN_DEPRECATED_API;

/**
 * @deprecated Deprecated since 11.0.
 * @brief Gets the y position value.
 * @since_tizen 5.5
 * @param[in] handle The notification_ex geometry handle
 * @param[out] val The y value
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_geometry_h
 * @see noti_ex_geometry_create()
 * @par Sample code:
 * @code
#include <notification_ex.h>

{
	int ret;
	int y;

	ret = noti_ex_geometry_get_y(geometry, &y);
}
 * @endcode
 */
int noti_ex_geometry_get_y(noti_ex_geometry_h handle, int *val) TIZEN_DEPRECATED_API;

/**
 * @deprecated Deprecated since 11.0.
 * @brief Gets the width value.
 * @since_tizen 5.5
 * @param[in] handle The notification_ex geometry handle
 * @param[out] val The width value
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_geometry_h
 * @see noti_ex_geometry_create()
 * @par Sample code:
 * @code
#include <notification_ex.h>

{
	int ret;
	int width;

	ret = noti_ex_geometry_get_width(geometry, &width);
}
 * @endcode
 */
int noti_ex_geometry_get_width(noti_ex_geometry_h handle, int *val) TIZEN_DEPRECATED_API;

/**
 * @deprecated Deprecated since 11.0.
 * @brief Gets the height value.
 * @since_tizen 5.5
 * @param[in] handle The notification_ex geometry handle
 * @param[out] val The height value
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_geometry_h
 * @see noti_ex_geometry_create()
 * @par Sample code:
 * @code
#include <notification_ex.h>

{
	int ret;
	int height;

	ret = noti_ex_geometry_get_height(geometry, &height);
}
 * @endcode
 */
int noti_ex_geometry_get_height(noti_ex_geometry_h handle, int *val) TIZEN_DEPRECATED_API;

/**
 * @deprecated Deprecated since 11.0.
 * @brief The handle for the style information.
 * @since_tizen 5.5
 */
typedef void *noti_ex_style_h;

/**
 * @deprecated Deprecated since 11.0.
 * @brief Creates the notification_ex style handle.
 * @details Style information can be defined as a notification_ex style handle
 *          with color, padding and geometry information.
 *          The color is the defined notification_ex color handle.
 *          The padding is the defined notification_ex padding handle.
 *          The geometry is the defined notification_ex geometry handle.
 * @since_tizen 5.5
 * @remarks @a handle must be released using noti_ex_style_destroy().
 *          @a color, @a padding and @a geometry must not be released if the function returns #NOTI_EX_ERROR_NONE.
 *          @a color, @a padding and @a geometry will be released when @a handle is released.
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
 * @see noti_ex_style_destroy()
 * @par Sample code:
 * @code
#include <notification_ex.h>

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
int noti_ex_style_create(noti_ex_style_h *handle, noti_ex_color_h color,
		noti_ex_padding_h padding, noti_ex_geometry_h geometry) TIZEN_DEPRECATED_API;

/**
 * @deprecated Deprecated since 11.0.
 * @brief Releases the notification_ex style handle.
 * @since_tizen 5.5
 * @param[in] handle The notification_ex style handle
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_style_h
 * @par Sample code:
 * @code
#include <notification_ex.h>

{
	int ret;

	ret = noti_ex_style_destroy(style);
}
 * @endcode
 */
int noti_ex_style_destroy(noti_ex_style_h handle) TIZEN_DEPRECATED_API;

/**
 * @deprecated Deprecated since 11.0.
 * @brief Gets the padding information from a style.
 * @since_tizen 5.5
 * @remarks @a padding must be released using noti_ex_padding_destroy().
 * @param[in] handle The notification_ex style handle
 * @param[out] padding The notification_ex padding handle
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_style_h
 * @see #noti_ex_padding_h
 * @see noti_ex_style_create()
 * @par Sample code:
 * @code
#include <notification_ex.h>

{
	int ret;
	noti_ex_padding_h padding = NULL;

	ret = noti_ex_style_get_padding(style, &padding);
}
 * @endcode
 */
int noti_ex_style_get_padding(noti_ex_style_h handle, noti_ex_padding_h *padding) TIZEN_DEPRECATED_API;

/**
 * @deprecated Deprecated since 11.0.
 * @brief Gets the color information from a style.
 * @since_tizen 5.5
 * @remarks @a color must be released using noti_ex_color_destroy().
 * @param[in] handle The notification_ex style handle
 * @param[out] color The notification_ex color handle
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_style_h
 * @see #noti_ex_color_h
 * @see noti_ex_style_create()
 * @par Sample code:
 * @code
#include <notification_ex.h>

{
	int ret;
	noti_ex_color_h color = NULL;

	ret = noti_ex_style_get_color(style, &color);
}
 * @endcode
 */
int noti_ex_style_get_color(noti_ex_style_h handle, noti_ex_color_h *color) TIZEN_DEPRECATED_API;

/**
 * @deprecated Deprecated since 11.0.
 * @brief Gets the geometry information from style.
 * @since_tizen 5.5
 * @remarks @a geometry must be released using noti_ex_geometry_destroy().
 * @param[in] handle The notification_ex style handle
 * @param[out] geometry The notification_ex geometry handle
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_style_h
 * @see #noti_ex_geometry_h
 * @see noti_ex_style_create()
 * @par Sample code:
 * @code
#include <notification_ex.h>

{
	int ret;
	noti_ex_geometry_h geometry = NULL;

	ret = noti_ex_style_get_geometry(style, &geometry);
}
 * @endcode
 */
int noti_ex_style_get_geometry(noti_ex_style_h handle, noti_ex_geometry_h *geometry) TIZEN_DEPRECATED_API;

/**
 * @deprecated Deprecated since 11.0.
 * @brief Gets the background image path from style.
 * @since_tizen 5.5
 * @remarks The @a image_path should be released using free().
 * @param[in] handle The notification_ex style handle
 * @param[out] image_path The background image path
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_style_h
 * @see noti_ex_style_create()
 * @par Sample code:
 * @code
#include <notification_ex.h>

{
	int ret;
	char *image_path;

	ret = noti_ex_style_get_background_image(style, &image_path);
}
 * @endcode
 */
int noti_ex_style_get_background_image(noti_ex_style_h handle,
		char **image_path) TIZEN_DEPRECATED_API;

/**
 * @deprecated Deprecated since 11.0.
 * @brief Sets the background image path of a style.
 * @since_tizen 5.5
 * @param[in] handle The notification_ex style handle
 * @param[in] image_path The background image path
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_style_h
 * @see noti_ex_style_create()
 * @par Sample code:
 * @code
#include <notification_ex.h>

{
	int ret;
	char *image_path[BUFLEN];

	char *shared_path = app_get_shared_resource_path();
	snprintf(image_path, BUFLEN, "%simage.png", shared_path);
	free(shared_path);

	ret = noti_ex_style_set_background_image(style, image_path);
}
 * @endcode
 */
int noti_ex_style_set_background_image(noti_ex_style_h handle,
		char *image_path) TIZEN_DEPRECATED_API;

/**
 * @deprecated Deprecated since 11.0.
 * @brief Gets the background color of a style.
 * @since_tizen 5.5
 * @remarks @a color must be released using noti_ex_color_destroy().
 * @param[in] handle The notification_ex style handle
 * @param[out] color The notification_ex color handle
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_style_h
 * @see noti_ex_style_create()
 * @par Sample code:
 * @code
#include <notification_ex.h>

{
	int ret;
	noti_ex_color_h color = NULL;

	ret = noti_ex_style_get_background_color(style, &color);
}
 * @endcode
 */
int noti_ex_style_get_background_color(noti_ex_style_h handle,
		noti_ex_color_h *color) TIZEN_DEPRECATED_API;

/**
 * @deprecated Deprecated since 11.0.
 * @brief Sets the background color of a style.
 * @since_tizen 5.5
 * @remarks @a color must be released using noti_ex_color_destroy().
 * @param[in] handle The notification_ex style handle
 * @param[in] color The notification_ex color handle
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_style_h
 * @see noti_ex_style_create()
 * @par Sample code:
 * @code
#include <notification_ex.h>

{
	int ret;
	noti_ex_color_h color;

	ret = noti_ex_color_create(&color, 0, 0, 0, 0);
	ret = noti_ex_style_set_background_color(style, color);
	ret = noti_ex_color_destroy(color);
}
 * @endcode
 */
int noti_ex_style_set_background_color(noti_ex_style_h handle,
		noti_ex_color_h color) TIZEN_DEPRECATED_API;

/**
 * @deprecated Deprecated since 11.0.
 * @brief The handle for the LED information.
 * @since_tizen 5.5
 */
typedef void *noti_ex_led_info_h;

/**
 * @deprecated Deprecated since 11.0.
 * @brief Creates the notification_ex LED info handle.
 * @details LED information can be defined as a notification_ex LED handle
 *          with color information.
 *          The color is the defined notification_ex color handle.
 * @since_tizen 5.5
 * @remarks @a handle must be released using noti_ex_led_info_destroy().
 *          @a color must not be released if the function returns #NOTI_EX_ERROR_NONE.
 *          @a color will be released when @a handle is released.
 * @param[out] handle The notification_ex LED info handle
 * @param[in] color The notification_ex color handle
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #NOTI_EX_ERROR_OUT_OF_MEMORY out of memory
 * @see #noti_ex_led_info_h
 * @see #noti_ex_color_h
 * @see noti_ex_led_info_destroy()
 * @par Sample code:
 * @code
#include <notification_ex.h>

{
	int ret;
	noti_ex_led_info_h led_info = NULL;
	noti_ex_color_h color = NULL;

	ret = noti_ex_color_create(&color, 0, 0, 0, 0);

	ret = noti_ex_led_info_create(&led_info, color);
}
 * @endcode
 */
int noti_ex_led_info_create(noti_ex_led_info_h *handle, noti_ex_color_h color) TIZEN_DEPRECATED_API;

/**
 * @deprecated Deprecated since 11.0.
 * @brief Releases the notification_ex LED info handle.
 * @since_tizen 5.5
 * @param[in] handle The notification_ex LED info handle
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_led_info_h
 * @par Sample code:
 * @code
#include <notification_ex.h>

{
	int ret;

	ret = noti_ex_led_info_destroy(led_info);
}
 * @endcode
 */
int noti_ex_led_info_destroy(noti_ex_led_info_h handle) TIZEN_DEPRECATED_API;

/**
 * @deprecated Deprecated since 11.0.
 * @brief Sets the time period for turning on the LED.
 * @since_tizen 5.5
 * @param[in] handle The notification_ex LED info handle
 * @param[in] ms The time for turning on the LED, in milliseconds
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_led_info_h
 * @see noti_ex_led_info_create()
 * @par Sample code:
 * @code
#include <notification_ex.h>

{
	int ret;

	ret = noti_ex_led_info_set_on_period(led_info, 5);
}
 * @endcode
 */
int noti_ex_led_info_set_on_period(noti_ex_led_info_h handle, int ms) TIZEN_DEPRECATED_API;

/**
 * @deprecated Deprecated since 11.0.
 * @brief Gets the time period for turning on the LED.
 * @since_tizen 5.5
 * @param[in] handle The notification_ex LED info handle
 * @param[out] ms The time for turning on the LED, in milliseconds
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_led_info_h
 * @see noti_ex_led_info_create()
 * @par Sample code:
 * @code
#include <notification_ex.h>

{
	int ret;
	int ms;

	ret = noti_ex_led_info_get_on_period(led_info, &ms);
}
 * @endcode
 */
int noti_ex_led_info_get_on_period(noti_ex_led_info_h handle, int *ms) TIZEN_DEPRECATED_API;

/**
 * @deprecated Deprecated since 11.0.
 * @brief Sets the time period for turning off the LED.
 * @since_tizen 5.5
 * @param[in] handle The notification_ex LED info handle
 * @param[in] ms The time for turning off the LED, in milliseconds
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_led_info_h
 * @see noti_ex_led_info_create()
 * @par Sample code:
 * @code
#include <notification_ex.h>

{
	int ret;

	ret = noti_ex_led_info_set_off_period(led_info, 5);
}
 * @endcode
 */
int noti_ex_led_info_set_off_period(noti_ex_led_info_h handle, int ms) TIZEN_DEPRECATED_API;

/**
 * @deprecated Deprecated since 11.0.
 * @brief Gets the time period for turning off the LED.
 * @since_tizen 5.5
 * @param[in] handle The notification_ex LED info handle
 * @param[out] ms The time for turning off the LED, in milliseconds
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_led_info_h
 * @see noti_ex_led_info_create()
 * @par Sample code:
 * @code
#include <notification_ex.h>

{
	int ret;
	int ms;

	ret = noti_ex_led_info_get_off_period(led_info, &ms);
}
 * @endcode
 */
int noti_ex_led_info_get_off_period(noti_ex_led_info_h handle, int *ms) TIZEN_DEPRECATED_API;

/**
 * @deprecated Deprecated since 11.0.
 * @brief Gets the color information from LED info.
 * @since_tizen 5.5
 * @remarks @a color must be released using noti_ex_color_destroy().
 * @param[in] handle The notification_ex LED info handle
 * @param[out] color The notification_ex color handle
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_led_info_h
 * @see #noti_ex_color_h
 * @see noti_ex_led_info_create()
 * @par Sample code:
 * @code
#include <notification_ex.h>

{
	int ret;
	noti_ex_color_h color = NULL;

	ret = noti_ex_led_info_get_color(led_info, &color);
}
 * @endcode
 */
int noti_ex_led_info_get_color(noti_ex_led_info_h handle, noti_ex_color_h *color) TIZEN_DEPRECATED_API;

/**
 * @deprecated Deprecated since 11.0.
 * @brief The notification_ex action handle.
 * @since_tizen 5.5
 */
typedef void *noti_ex_action_h;

/**
 * @deprecated Deprecated since 11.0.
 * @brief The notification_ex item handle.
 * @since_tizen 5.5
 */
typedef void *noti_ex_item_h;

/**
 * @deprecated Deprecated since 11.0.
 * @brief The notification_ex item_info handle.
 * @since_tizen 5.5
 */
typedef void *noti_ex_item_info_h;

/**
 * @deprecated Deprecated since 11.0.
 * @brief Releases the notification_ex action handle.
 * @since_tizen 5.5
 * @param[in] handle The notification_ex action handle
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_action_h
 * @par Sample code:
 * @code
#include <notification_ex.h>

{
	int ret;

	ret = noti_ex_action_destroy(action);
}
 * @endcode
 */
int noti_ex_action_destroy(noti_ex_action_h handle) TIZEN_DEPRECATED_API;

/**
 * @deprecated Deprecated since 11.0.
 * @brief Gets the type of an action.
 * @details The action type is equal to one of the values of #noti_ex_action_type_e
 * @since_tizen 5.5
 * @param[in] handle The notification_ex action handle
 * @param[out] type The type of action \n
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_action_h
 * @see #noti_ex_action_type_e
 * @par Sample code:
 * @code
#include <notification_ex.h>

{
	int ret;
	int type = NOTI_EX_ACTION_TYPE_NULL;

	ret = noti_ex_action_get_type(action, &type);

	if (type == NOTI_EX_ACTION_TYPE_VISIBILITY) {
		// Do something
	}
}
 * @endcode
 */
int noti_ex_action_get_type(noti_ex_action_h handle, int *type) TIZEN_DEPRECATED_API;

/**
 * @deprecated Deprecated since 11.0.
 * @brief Gets the local state of an action.
 * @since_tizen 5.5
 * @param[in] handle The notification_ex action handle
 * @param[out] local The local state of action
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_action_h
 * @par Sample code:
 * @code
#include <notification_ex.h>

{
	int ret;
	bool local;

	ret = noti_ex_action_is_local(action, &local);
}
 * @endcode
 */
int noti_ex_action_is_local(noti_ex_action_h handle, bool *local) TIZEN_DEPRECATED_API;

/**
 * @deprecated Deprecated since 11.0.
 * @brief Executes the action for a notification_ex item.
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
#include <notification_ex.h>

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
int noti_ex_action_execute(noti_ex_action_h handle, noti_ex_item_h item) TIZEN_DEPRECATED_API;

/**
 * @deprecated Deprecated since 11.0.
 * @brief Gets the extra data for an action.
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
#include <notification_ex.h>

{
	int ret;
	char *extra = NULL;

	ret = noti_ex_action_get_extra(action, &extra);
}
 * @endcode
 */
int noti_ex_action_get_extra(noti_ex_action_h handle, char **extra) TIZEN_DEPRECATED_API;

/**
 * @deprecated Deprecated since 11.0.
 * @brief Gets the time to hide the notification_ex item.
 * @since_tizen 5.5
 * @param[in] handle The notification_ex item info handle
 * @param[out] hide_time The time to hide notification, in seconds
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_item_info_h
 * @see noti_ex_item_get_info()
 * @par Sample code:
 * @code
#include <notification_ex.h>

{
	int ret;
	int hide_time;
	noti_ex_item_info_h item_info = NULL;

	ret = noti_ex_item_get_info(item_handle, &item_info);

	ret = noti_ex_item_info_get_hide_time(item_info, &hide_time);
}
 * @endcode
 */
int noti_ex_item_info_get_hide_time(noti_ex_item_info_h handle, int *hide_time) TIZEN_DEPRECATED_API;

/**
 * @deprecated Deprecated since 11.0.
 * @brief Sets the time to hide the notification_ex item.
 * @since_tizen 5.5
 * @param[in] handle The notification_ex item info handle
 * @param[in] hide_time The time to hide notification, in seconds
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_item_info_h
 * @see noti_ex_item_get_info()
 * @par Sample code:
 * @code
#include <notification_ex.h>

{
	int ret;
	noti_ex_item_info_h item_info = NULL;

	ret = noti_ex_item_get_info(item_handle, &item_info);

	ret = noti_ex_item_info_set_hide_time(action, 5);
}
 * @endcode
 */
int noti_ex_item_info_set_hide_time(noti_ex_item_info_h handle, int hide_time) TIZEN_DEPRECATED_API;

/**
 * @deprecated Deprecated since 11.0.
 * @brief Gets the time to delete the notification_ex item.
 * @since_tizen 5.5
 * @param[in] handle The notification_ex item info handle
 * @param[out] delete_time The time to delete notification, in seconds
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_item_info_h
 * @see noti_ex_item_get_info()
 * @par Sample code:
 * @code
#include <notification_ex.h>

{
	int ret;
	int delete_time;
	noti_ex_item_info_h item_info = NULL;

	ret = noti_ex_item_get_info(item_handle, &item_info);

	ret = noti_ex_item_info_get_delete_time(item_info, &delete_time);
}
 * @endcode
 */
int noti_ex_item_info_get_delete_time(noti_ex_item_info_h handle, int *delete_time) TIZEN_DEPRECATED_API;

/**
 * @deprecated Deprecated since 11.0.
 * @brief Sets the time to delete the notification_ex item.
 * @since_tizen 5.5
 * @param[in] handle The notification_ex item info handle
 * @param[in] delete_time The time to delete notification, in seconds
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_item_info_h
 * @see noti_ex_item_get_info()
 * @par Sample code:
 * @code
#include <notification_ex.h>

{
	int ret;
	noti_ex_item_info_h item_info = NULL;

	ret = noti_ex_item_get_info(item_handle, &item_info);

	ret = noti_ex_item_info_set_delete_time(item_info, 5);
}
 * @endcode
 */
int noti_ex_item_info_set_delete_time(noti_ex_item_info_h handle, int delete_time) TIZEN_DEPRECATED_API;

/**
 * @deprecated Deprecated since 11.0.
 * @brief Gets the time information.
 * @since_tizen 5.5
 * @param[in] handle The notification_ex item info handle
 * @param[out] time The time information
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_item_info_h
 * @see noti_ex_item_get_info()
 * @par Sample code:
 * @code
#include <notification_ex.h>

{
	int ret;
	time_t time;
	noti_ex_item_info_h item_info = NULL;

	ret = noti_ex_item_get_info(item_handle, &item_info);

	ret = noti_ex_item_info_get_time(item_info, &time);
}
 * @endcode
 */
int noti_ex_item_info_get_time(noti_ex_item_info_h handle, time_t *time) TIZEN_DEPRECATED_API;

/**
 * @deprecated Deprecated since 11.0.
 * @brief Releases the notification_ex item handle.
 * @since_tizen 5.5
 * @param[in] handle The notification_ex item handle
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_item_h
 * @par Sample code:
 * @code
#include <notification_ex.h>

{
	int ret;

	ret = noti_ex_item_destroy(item_handle);
}
 * @endcode
 */
int noti_ex_item_destroy(noti_ex_item_h handle) TIZEN_DEPRECATED_API;

/**
 * @deprecated Deprecated since 11.0.
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
#include <notification_ex.h>

{
	int ret;
	noti_ex_item_h item = NULL;

	ret = noti_ex_item_find_by_id(item_handle, "find_id", &item);
}
 * @endcode
 */
int noti_ex_item_find_by_id(noti_ex_item_h handle, const char *id, noti_ex_item_h *item) TIZEN_DEPRECATED_API;

/**
 * @deprecated Deprecated since 11.0.
 * @brief Gets the type of notification_ex item.
 * @details The item type is equal to one of the values of #noti_ex_item_type_e.
 * @since_tizen 5.5
 * @param[in] handle The notification_ex item handle
 * @param[out] type The type of notification_ex item
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_item_h
 * @see #noti_ex_item_type_e
 * @par Sample code:
 * @code
#include <notification_ex.h>

{
	int ret;
	int type = NOTI_EX_ITEM_TYPE_NULL;

	ret = noti_ex_item_get_type(item_handle, &type);

	if (type == NOTI_EX_ITEM_TYPE_BUTTON) {
		// Do something
	}
}
 * @endcode
 */
int noti_ex_item_get_type(noti_ex_item_h handle, int *type) TIZEN_DEPRECATED_API;

/**
 * @deprecated Deprecated since 11.0.
 * @brief Gets the id of a notification_ex item.
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
#include <notification_ex.h>

{
	int ret;
	char *id = NULL;

	ret = noti_ex_item_get_id(item_handle, &id);
}
 * @endcode
 */
int noti_ex_item_get_id(noti_ex_item_h handle, char **id) TIZEN_DEPRECATED_API;

/**
 * @deprecated Deprecated since 11.0.
 * @brief Sets the id of a notification_ex item.
 * @since_tizen 5.5
 * @param[in] handle The notification_ex item handle
 * @param[in] id The id of notification_ex item
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_item_h
 * @par Sample code:
 * @code
#include <notification_ex.h>

{
	int ret;

	ret = noti_ex_item_set_id(item_handle, "new_id");
}
 * @endcode
 */
int noti_ex_item_set_id(noti_ex_item_h handle, const char *id) TIZEN_DEPRECATED_API;

/**
 * @deprecated Deprecated since 11.0.
 * @brief Gets the action for a notification_ex item.
 * @details The action is defined as notification_ex action handle.
 * @since_tizen 5.5
 * @remarks @a action must be released using noti_ex_action_destroy().
 * @param[in] handle The notification_ex item handle
 * @param[out] action The notification_ex action handle
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_item_h
 * @see #noti_ex_action_h
 * @par Sample code:
 * @code
#include <notification_ex.h>

{
	int ret;
	noti_ex_action_h action = NULL;

	ret = noti_ex_item_get_action(item_handle, &action);
}
 * @endcode
 */
int noti_ex_item_get_action(noti_ex_item_h handle, noti_ex_action_h *action) TIZEN_DEPRECATED_API;

/**
 * @deprecated Deprecated since 11.0.
 * @brief Sets the action of a notification_ex item.
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
#include <notification_ex.h>

{
	int ret;
	noti_ex_item_h item_handle = NULL;
	noti_ex_action_h action = NULL;

	ret = app_control_create(&app_control);
	ret = app_control_set_app_id(app_control, "appid");
	ret = noti_ex_action_app_control_create(&action, app_control, NULL);

	ret = noti_ex_item_button_create(&item_handle, "button_id", "button_title");

	ret = noti_ex_item_set_action(item_handle, action);

	app_control_destroy(app_control);
}
 * @endcode
 */
int noti_ex_item_set_action(noti_ex_item_h handle, noti_ex_action_h action) TIZEN_DEPRECATED_API;

/**
 * @deprecated Deprecated since 11.0.
 * @brief Gets the style of a notification_ex item.
 * @details The style is defined as notification_ex style handle.
 * @since_tizen 5.5
 * @remarks @a style must be released using noti_ex_style_destroy().
 * @param[in] handle The notification_ex item handle
 * @param[out] style The notification_ex style handle
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_item_h
 * @see #noti_ex_style_h
 * @par Sample code:
 * @code
#include <notification_ex.h>

{
	int ret;
	noti_ex_style_h style = NULL;

	ret = noti_ex_item_get_style(item_handle, &style);
}
 * @endcode
 */
int noti_ex_item_get_style(noti_ex_item_h handle, noti_ex_style_h *style) TIZEN_DEPRECATED_API;

/**
 * @deprecated Deprecated since 11.0.
 * @brief Sets the style of a notification_ex item.
 * @details The style is defined as notification_ex style handle.
 * @since_tizen 5.5
 * @param[in] handle The notification_ex item handle
 * @param[in] style The notification_ex style handle
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_item_h
 * @see #noti_ex_style_h
 * @see noti_ex_style_create()
 * @par Sample code:
 * @code
#include <notification_ex.h>

{
	int ret;

	ret = noti_ex_item_set_style(item_handle, style);
}
 * @endcode
 */
int noti_ex_item_set_style(noti_ex_item_h handle, noti_ex_style_h style) TIZEN_DEPRECATED_API;

/**
 * @deprecated Deprecated since 11.0.
 * @brief Sets the visibility state of a notification_ex item.
 * @since_tizen 5.5
 * @param[in] handle The notification_ex item handle
 * @param[in] visible The visible state
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_item_h
 * @par Sample code:
 * @code
#include <notification_ex.h>

{
	int ret;

	ret = noti_ex_item_set_visible(item_handle, true);
}
 * @endcode
 */
int noti_ex_item_set_visible(noti_ex_item_h handle, bool visible) TIZEN_DEPRECATED_API;

/**
 * @deprecated Deprecated since 11.0.
 * @brief Gets the visibility state of a notification_ex item.
 * @since_tizen 5.5
 * @param[in] handle The notification_ex item handle
 * @param[out] visible The visible state
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_item_h
 * @par Sample code:
 * @code
#include <notification_ex.h>

{
	int ret;
	bool visible;

	ret = noti_ex_item_get_visible(item_handle, &visible);
}
 * @endcode
 */
int noti_ex_item_get_visible(noti_ex_item_h handle, bool *visible) TIZEN_DEPRECATED_API;

/**
 * @deprecated Deprecated since 11.0.
 * @brief Sets the enabled state of a notification_ex item.
 * @since_tizen 5.5
 * @param[in] handle The notification_ex item handle
 * @param[in] enable The enable state
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_item_h
 * @par Sample code:
 * @code
#include <notification_ex.h>

{
	int ret;

	ret = noti_ex_item_set_enable(item_handle, true);
}
 * @endcode
 */
int noti_ex_item_set_enable(noti_ex_item_h handle, bool enable) TIZEN_DEPRECATED_API;

/**
 * @deprecated Deprecated since 11.0.
 * @brief Gets the enabled state of a notification_ex item.
 * @since_tizen 5.5
 * @param[in] handle The notification_ex item handle
 * @param[in] enable The enable state
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_item_h
 * @par Sample code:
 * @code
#include <notification_ex.h>

{
	int ret;
	bool enable;

	ret = noti_ex_item_get_enable(item_handle, &enable);
}
 * @endcode
 */
int noti_ex_item_get_enable(noti_ex_item_h handle, bool *enable) TIZEN_DEPRECATED_API;

/**
 * @deprecated Deprecated since 11.0.
 * @brief Adds the receiver group of a notification_ex item.
 * @since_tizen 5.5
 * @param[in] handle The notification_ex item handle
 * @param[in] receiver_group The receiver group
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_item_h
 * @par Sample code:
 * @code
#include <notification_ex.h>

{
	int ret;

	ret = noti_ex_item_add_receiver(item_handle, NOTI_EX_RECEIVER_GROUP_INDICATOR);
}
 * @endcode
 */
int noti_ex_item_add_receiver(noti_ex_item_h handle, const char *receiver_group) TIZEN_DEPRECATED_API;

/**
 * @deprecated Deprecated since 11.0.
 * @brief Removes the receiver group from a notification_ex item.
 * @since_tizen 5.5
 * @param[in] handle The notification_ex item handle
 * @param[in] receiver_group The receiver group
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_item_h
 * @par Sample code:
 * @code
#include <notification_ex.h>

{
	int ret;

	ret = noti_ex_item_remove_receiver(item_handle, NOTI_EX_RECEIVER_GROUP_INDICATOR);
}
 * @endcode
 */
int noti_ex_item_remove_receiver(noti_ex_item_h handle, const char *receiver_group) TIZEN_DEPRECATED_API;

/**
 * @deprecated Deprecated since 11.0.
 * @brief Gets the list of receiver group of a notification_ex item.
 * @since_tizen 5.5
 * @remarks Each member of the list must be released using free(), then the @a list must be released using free().
 * @param[in] handle The notification_ex item handle
 * @param[out] list The list of receiver group
 * @param[out] count The count of list
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_item_h
 * @par Sample code:
 * @code
#include <notification_ex.h>

{
	int ret;
	char **list = NULL;
	int count;

	ret = noti_ex_item_get_receiver_list(item_handle, &list, &count);
}
 * @endcode
 */
int noti_ex_item_get_receiver_list(noti_ex_item_h handle, char ***list, int *count) TIZEN_DEPRECATED_API;

/**
 * @deprecated Deprecated since 11.0.
 * @brief Sets the policy of a notification_ex item.
 * @details @a policy should be set to values of #noti_ex_item_policy_e combined with bitwise 'or'.
 * @since_tizen 5.5
 * @param[in] handle The notification_ex item handle
 * @param[in] policy The policy of a notification_ex item
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_item_h
 * @see #noti_ex_item_policy_e
 * @par Sample code:
 * @code
#include <notification_ex.h>

{
	int ret;

	ret = noti_ex_item_set_policy(item_handle,
			NOTI_EX_ITEM_POLICY_ON_BOOT_CLEAR | NOTI_EX_ITEM_POLICY_SIM_MODE);
}
 * @endcode
 */
int noti_ex_item_set_policy(noti_ex_item_h handle, int policy) TIZEN_DEPRECATED_API;

/**
 * @deprecated Deprecated since 11.0.
 * @brief Gets the policy of a notification_ex item.
 * @details @a policy is equal to values of #noti_ex_item_policy_e combined with bitwise 'or'.
 * @since_tizen 5.5
 * @param[in] handle The notification_ex item handle
 * @param[out] policy The policy of a notification_ex item
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_item_h
 * @see #noti_ex_item_policy_e
 * @par Sample code:
 * @code
#include <notification_ex.h>

{
	int ret;
	int policy;

	ret = noti_ex_item_get_policy(item_handle, &policy);
}
 * @endcode
 */
int noti_ex_item_get_policy(noti_ex_item_h handle, int *policy) TIZEN_DEPRECATED_API;

/**
 * @deprecated Deprecated since 11.0.
 * @brief Gets the channel of a notification_ex item.
 * @since_tizen 5.5
 * @remarks @a channel must be released using free().
 * @param[in] handle The notification_ex item handle
 * @param[out] channel The channel of a notification_ex item
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_item_h
 * @par Sample code:
 * @code
#include <notification_ex.h>

{
	int ret;
	char *channel;

	ret = noti_ex_item_get_channel(item_handle, &channel);
}
 * @endcode
 */
int noti_ex_item_get_channel(noti_ex_item_h handle, char **channel) TIZEN_DEPRECATED_API;

/**
 * @deprecated Deprecated since 11.0.
 * @brief Sets the channel of a notification_ex item.
 * @since_tizen 5.5
 * @param[in] handle The notification_ex item handle
 * @param[in] channel The channel of a notification_ex item
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_item_h
 * @par Sample code:
 * @code
#include <notification_ex.h>

{
	int ret;

	ret = noti_ex_item_set_channel(item_handle, "channel");
}
 * @endcode
 */
int noti_ex_item_set_channel(noti_ex_item_h handle, const char *channel) TIZEN_DEPRECATED_API;

/**
 * @deprecated Deprecated since 11.0.
 * @brief Sets the LED information of a notification_ex item.
 * @details The LED information is defined as notification_ex LED info handle
 * @since_tizen 5.5
 * @param[in] handle The notification_ex item handle
 * @param[in] led The notification_ex LED info item
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_item_h
 * @see #noti_ex_led_info_h
 * @see noti_ex_led_info_create()
 * @par Sample code:
 * @code
#include <notification_ex.h>

{
	int ret;

	ret = noti_ex_item_set_led_info(item_handle, led_info);
}
 * @endcode
 */
int noti_ex_item_set_led_info(noti_ex_item_h handle, noti_ex_led_info_h led) TIZEN_DEPRECATED_API;

/**
 * @deprecated Deprecated since 11.0.
 * @brief Gets the LED information of a notification_ex item.
 * @details The LED information is defined as notification_ex LED info handle
 * @since_tizen 5.5
 * @remarks @a led must be released using noti_ex_led_info_destroy().
 * @param[in] handle The notification_ex item handle
 * @param[out] led The notification_ex LED info item
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_item_h
 * @see #noti_ex_led_info_h
 * @par Sample code:
 * @code
#include <notification_ex.h>

{
	int ret;
	noti_ex_led_info_h led_info = NULL;

	ret = noti_ex_item_get_led_info(item_handle, &led_info);
}
 * @endcode
 */
int noti_ex_item_get_led_info(noti_ex_item_h handle, noti_ex_led_info_h *led) TIZEN_DEPRECATED_API;

/**
 * @deprecated Deprecated since 11.0.
 * @brief Sets the sound path of a notification_ex item.
 * @since_tizen 5.5
 * @param[in] handle The notification_ex item handle
 * @param[in] path The absolute path of sound for notification_ex item
 *                 If you want to set a default sound, enter #NOTI_EX_SOUND_TYPE_DEFAULT.
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_item_h
 * @par Sample code:
 * @code
#include <notification_ex.h>

{
	int ret;

	ret = noti_ex_item_set_sound_path(item_handle, "sound_path");
}
 * @endcode
 */
int noti_ex_item_set_sound_path(noti_ex_item_h handle, const char *path) TIZEN_DEPRECATED_API;

/**
 * @deprecated Deprecated since 11.0.
 * @brief Gets the sound path of a notification_ex item.
 * @since_tizen 5.5
 * @remarks @a path must be released using free().
 * @param[in] handle The notification_ex item handle
 * @param[out] path The absolute path of sound for notification_ex item
 *                  If not set, you will get NULL.
 *                  If set default, you will get #NOTI_EX_SOUND_TYPE_DEFAULT.
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_item_h
 * @par Sample code:
 * @code
#include <notification_ex.h>

{
	int ret;
	char *path = NULL;

	ret = noti_ex_item_get_sound_path(item_handle, &path);
}
 * @endcode
 */
int noti_ex_item_get_sound_path(noti_ex_item_h handle, char **path) TIZEN_DEPRECATED_API;

/**
 * @deprecated Deprecated since 11.0.
 * @brief Sets the vibration path of a notification_ex item.
 * @since_tizen 5.5
 * @param[in] handle The notification_ex item handle
 * @param[in] path The absolute path of vibration for notification_ex item
 *                 If you want to set a default sound, enter #NOTI_EX_VIBRATION_TYPE_DEFAULT.
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_item_h
 * @par Sample code:
 * @code
#include <notification_ex.h>

{
	int ret;

	ret = noti_ex_item_set_vibration_path(item_handle, "vibration_path");
}
 * @endcode
 */
int noti_ex_item_set_vibration_path(noti_ex_item_h handle, const char *path) TIZEN_DEPRECATED_API;

/**
 * @deprecated Deprecated since 11.0.
 * @brief Gets the vibration path of a notification_ex item.
 * @since_tizen 5.5
 * @remarks @a path must be released using free().
 * @param[in] handle The notification_ex item handle
 * @param[out] path The absolute path of vibration for a notification_ex item
 *                  If not set, you will get NULL.
 *                  If set default, you will get #NOTI_EX_VIBRATION_TYPE_DEFAULT.
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_item_h
 * @par Sample code:
 * @code
#include <notification_ex.h>

{
	int ret;
	char *path = NULL;

	ret = noti_ex_item_get_vibration_path(item_handle, &path);
}
 * @endcode
 */
int noti_ex_item_get_vibration_path(noti_ex_item_h handle, char **path) TIZEN_DEPRECATED_API;

/**
 * @deprecated Deprecated since 11.0.
 * @brief Gets the information of a notification_ex item.
 * @details The information of notification_ex item is defined as
 *          notification_ex item info handle.
 * @since_tizen 5.5
 * @remarks @a info should not be released.
 * @remarks @a info is available until @a handle is released.
 * @param[in] handle The notification_ex item handle
 * @param[out] info The notification_ex item info handle
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_item_h
 * @see #noti_ex_item_info_h
 * @par Sample code:
 * @code
#include <notification_ex.h>

{
	int ret;
	noti_ex_item_info_h item_info = NULL;

	ret = noti_ex_item_get_info(item_handle, &item_info);
}
 * @endcode
 */
int noti_ex_item_get_info(noti_ex_item_h handle, noti_ex_item_info_h *info) TIZEN_DEPRECATED_API;

/**
 * @deprecated Deprecated since 11.0.
 * @brief Gets the sender app id of a notification_ex item.
 * @since_tizen 5.5
 * @remarks @a id must be released using free().
 * @param[in] handle The notification_ex item handle
 * @param[out] id The sender app id of a notification_ex item
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_item_h
 * @par Sample code:
 * @code
#include <notification_ex.h>

{
	int ret;
	char *app_id = NULL;

	ret = noti_ex_item_get_sender_app_id(item_handle, &app_id);
}
 * @endcode
 */
int noti_ex_item_get_sender_app_id(noti_ex_item_h handle, char **id) TIZEN_DEPRECATED_API;

/**
 * @deprecated Deprecated since 11.0.
 * @brief Gets the tag of a notification_ex item.
 * @since_tizen 5.5
 * @remarks @a tag must be released using free().
 * @param[in] handle The notification_ex item handle
 * @param[out] tag The tag of a notification_ex item
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_item_h
 * @par Sample code:
 * @code
#include <notification_ex.h>

{
	int ret;
	char *tag = NULL;

	ret = noti_ex_item_get_tag(item_handle, &tag);
}
 * @endcode
 */
int noti_ex_item_get_tag(noti_ex_item_h handle, char **tag) TIZEN_DEPRECATED_API;

/**
 * @deprecated Deprecated since 11.0.
 * @brief Sets the tag of a notification_ex item.
 * @since_tizen 5.5
 * @param[in] handle The notification_ex item handle
 * @param[in] tag The tag of a notification_ex item
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_item_h
 * @par Sample code:
 * @code
#include <notification_ex.h>

{
	int ret;

	ret = noti_ex_item_set_tag(item_handle, "item_tag");
}
 * @endcode
 */
int noti_ex_item_set_tag(noti_ex_item_h handle, const char *tag) TIZEN_DEPRECATED_API;

/**
 * @deprecated Deprecated since 11.0.
 * @brief Gets the value indicating whether a notification is currently ongoing.
 * @since_tizen 5.5
 * @param[in] handle The notification_ex item handle
 * @param[out] ongoing The state of ongoing
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_item_h
 * @par Sample code:
 * @code
#include <notification_ex.h>

{
	int ret;
	bool ongoing;

	ret = noti_ex_item_get_ongoing_state(item_handle, &ongoing);
}
 * @endcode
 */
int noti_ex_item_get_ongoing_state(noti_ex_item_h handle, bool *ongoing) TIZEN_DEPRECATED_API;

/**
 * @deprecated Deprecated since 11.0.
 * @brief Sets the value indicating whether a notification is currently ongoing.
 * @since_tizen 5.5
 * @param[in] handle The notification_ex item handle
 * @param[in] ongoing The state of ongoing
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_item_h
 * @par Sample code:
 * @code
#include <notification_ex.h>

{
	int ret;

	ret = noti_ex_item_set_ongoing_state(item_handle, true);
}
 * @endcode
 */
int noti_ex_item_set_ongoing_state(noti_ex_item_h handle, bool ongoing) TIZEN_DEPRECATED_API;

/**
 * @deprecated Deprecated since 11.0.
 * @brief Checks if an item of a specified type is included in a notification_ex item.
 * @since_tizen 5.5
 * @param[in] handle The notification_ex item handle
 * @param[in] type The type of a notification_ex item
 * @param[out] exist The state of if a type is included
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_item_h
 * @see #noti_ex_item_type_e
 * @par Sample code:
 * @code
#include <notification_ex.h>

{
	int ret;
	int type = NOTI_EX_ITEM_TYPE_ENTRY;
	bool exist;

	ret = noti_ex_item_check_type_exist(item_handle, type, &exist);
}
 * @endcode
 */
int noti_ex_item_check_type_exist(noti_ex_item_h handle, int type, bool *exist) TIZEN_DEPRECATED_API;

/**
 * @deprecated Deprecated since 11.0.
 * @brief Gets the main type of a notification item.
 * @details The type is equal to one of the values of #noti_ex_item_main_type_e.
 * @since_tizen 5.5
 * @param[in] handle The notification_ex item handle
 * @param[out] type The main type
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_item_h
 * @see #noti_ex_item_main_type_e
 * @par Sample code:
 * @code
#include <notification_ex.h>

{
	int ret;
	int type = NOTI_EX_ITEM_MAIN_TYPE_NONE;

	ret = noti_ex_item_get_main_type(item_handle, &type);

	if (type == NOTI_EX_ITEM_MAIN_TYPE_TITLE) {
		// Do something
	}
}
 * @endcode
 */
int noti_ex_item_get_main_type(noti_ex_item_h handle, int *type) TIZEN_DEPRECATED_API;

/**
 * @deprecated Deprecated since 11.0.
 * @brief Sets the main type of a notification item.
 * @details Use this function to configure main type among multiple items.
 *          In other words, if there is only one item, it should not be used.
 *          The type is equal to one of the values of #noti_ex_item_main_type_e.
 * @since_tizen 5.5
 * @remarks When set to a specific type, the previous main item of the same type is canceled.
 * @param[in] handle The notification_ex item handle
 * @param[in] id The ID of notification_ex item
 * @param[in] type The main type
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_item_h
 * @see #noti_ex_item_main_type_e
 * @par Sample code:
 * @code
#include <notification_ex.h>

{
	noti_ex_item_h text_item = NULL;
	noti_ex_item_h group_item = NULL;
	int type = NOTI_EX_ITEM_MAIN_TYPE_TITLE;
	int ret;

	ret = noti_ex_item_group_create(&group_item, "group_id");
	ret = noti_ex_item_text_create(&text_item, "text_id", "text", "hyperlink");
	ret = noti_ex_item_group_add_child(group_item, text_item);
	noti_ex_item_destroy(text_item);

	ret = noti_ex_item_set_main_type(group_item, "text_id", type);
}
 * @endcode
 */
int noti_ex_item_set_main_type(noti_ex_item_h handle, const char *id, int type) TIZEN_DEPRECATED_API;

/**
 * @deprecated Deprecated since 11.0.
 * @brief Finds the notification item of the specified main type.
 * @details Finds the item that matches the main type among the group items.
 * @since_tizen 5.5
 * @remarks @a handle must be released using noti_ex_item_destroy().
 * @param[in] handle The notification_ex item handle
 * @param[in] type The main type
 * @param[out] item The notification_ex item handle
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_item_h
 * @see #noti_ex_item_main_type_e
 * @par Sample code:
 * @code
#include <notification_ex.h>

{
	noti_ex_item_h text_item = NULL;
	int type = NOTI_EX_ITEM_MAIN_TYPE_TITLE;
	int ret;

	ret = noti_ex_item_find_by_main_type(item_handle, type, &text_item);

	if (text_item != NULL) {
		// Do something
	}
}
 * @endcode
 */
int noti_ex_item_find_by_main_type(noti_ex_item_h handle, int type,
		noti_ex_item_h *item) TIZEN_DEPRECATED_API;

/**
 * @deprecated Deprecated since 11.0.
 * @brief Gets the extension data.
 * @since_tizen 5.5
 * @remarks The @a value should be released using bundle_free().
 * @param[in] handle The notification_ex item handle
 * @param[in] key The key
 * @param[out] value The bundle data
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_item_h
 * @par Sample code:
 * @code
#include <notification_ex.h>

{
	int ret;
	char *key = "ADDITIONAL_DATA_KEY";
	bundle *b;

	ret = noti_ex_item_get_extension_data(item_handle, key, &b);
	...
	bundle_free(b);
}
 * @endcode
 */
int noti_ex_item_get_extension_data(noti_ex_item_h handle, const char *key,
		bundle **value) TIZEN_DEPRECATED_API;

/**
 * @deprecated Deprecated since 11.0.
 * @brief Sets the extension data.
 * @since_tizen 5.5
 * @param[in] handle The notification_ex item handle
 * @param[in] key The key
 * @param[in] value The bundle data
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_item_h
 * @par Sample code:
 * @code
#include <notification_ex.h>

{
	int ret;
	char *key = "ADDITIONAL_DATA_KEY";
	bundle *b = bundle_create();
	bundle_add_str(b, "key", "value");

	ret = noti_ex_item_set_extension_data(item_handle, key, b);
	bundle_free(b);
}
 * @endcode
 */
int noti_ex_item_set_extension_data(noti_ex_item_h handle, const char *key,
		bundle *value) TIZEN_DEPRECATED_API;

/**
 * @}
 */

#ifdef __cplusplus
}
#endif
#endif  /* __TIZEN_APPFW_NOTIFICATION_EX_ITEM_H__ */
