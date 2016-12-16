/*
 * Copyright (c) 2000 - 2016 Samsung Electronics Co., Ltd. All rights reserved.
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


#ifndef __NOTIFICATION_H__
#define __NOTIFICATION_H__


#include <time.h>
#include <bundle.h>
#include <app.h>


#include <notification_error.h>
#include <notification_type.h>
#include <notification_status.h>


#ifdef __cplusplus
extern "C" {
#endif


/**
 * @file notification.h
 * @brief This file contains the notification API.
 */


/**
 * @addtogroup NOTIFICATION_MODULE
 * @{
 */


/**
 * @brief Sets an absolute path for an image file to display on the notification view.
 * @since_tizen @if WEARABLE 2.3.1 @elseif MOBILE 2.3 @endif
 * @param[in] noti The notification handle
 * @param[in] type The notification image type
 * @param[in] image_path The image file full path
 * @return #NOTIFICATION_ERROR_NONE on success,
 *         otherwise any other value on failure
 * @retval #NOTIFICATION_ERROR_NONE Success
 * @retval #NOTIFICATION_ERROR_INVALID_PARAMETER Invalid parameter
 * @pre Notification handle should be created by notification_create().
 * @see #notification_image_type_e
 * @see notification_create()
 * @par Sample code:
 * @code
#include <notification.h>

{
	notification_h noti = NULL;
	int noti_err = NOTIFICATION_ERROR_NONE;

	noti = notification_create(NOTIFICATION_TYPE_NOTI);
	if (noti == NULL)
		return;

	noti_err = notification_set_image(noti, NOTIFICATION_IMAGE_TYPE_ICON, APP_IMAGE_FULL_PATH);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		notification_free(noti);
		return;
	}
}
 * @endcode
 */
int notification_set_image(notification_h noti, notification_image_type_e type, const char *image_path);


/**
 * @brief Gets the absolute path of an image file.
 * @since_tizen @if WEARABLE 2.3.1 @elseif MOBILE 2.3 @endif
 * @remarks Do not free @a image_path. It will be freed when notification_free() is called.
 * @param[in] noti Notification handle
 * @param[in] type Notification image type
 * @param[out] image_path Image file full path
 * @return NOTIFICATION_ERROR_NONE on success,
 *         other value on failure
 * @retval NOTIFICATION_ERROR_NONE Success
 * @retval NOTIFICATION_ERROR_INVALID_PARAMETER Invalid parameter
 * @pre Notification handle should be created by notification_create().
 * @see #notification_image_type_e
 * @see notification_create()
 * @par Sample code:
 * @code
#include <notification.h>

{
	char *image_path = NULL;
	int noti_err = NOTIFICATION_ERROR_NONE;

	noti_err = notification_get_image(noti, NOTIFICATION_IMAGE_TYPE_ICON, &image_path);
	if (noti_err != NOTIFICATION_ERROR_NONE)
		return;
}
 * @endcode
 */
int notification_get_image(notification_h noti, notification_image_type_e type, char **image_path);


/**
 * @brief Sets a timestamp.
 * @details If input_time is @c 0, time information is taken from the current time.
 * @since_tizen @if WEARABLE 2.3.1 @elseif MOBILE 2.3 @endif
 * @param[in] noti The notification handle
 * @param[in] input_time The input time. If you want the time stamp to not be shown, set this as NOTIFICATION_DO_NOT_SHOW_TIME_STAMP
 * @return #NOTIFICATION_ERROR_NONE on success,
 *         otherwise any other value on failure
 * @retval #NOTIFICATION_ERROR_NONE Success
 * @retval #NOTIFICATION_ERROR_INVALID_PARAMETER Invalid parameter
 * @pre Notification handle should be created by notification_create().
 * @see notification_create()
 * @see NOTIFICATION_DO_NOT_SHOW_TIME_STAMP
 * @par Sample code:
 * @code
#include <notification.h>

{
	notification_h noti = NULL;
	int noti_err = NOTIFICATION_ERROR_NONE;

	noti = notification_create(NOTIFICATION_TYPE_NOTI);
	if (noti == NULL)
		return;

	noti_err = notification_set_time(noti, time(NULL));
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		notification_free(noti);
		return;
	}
}
 * @endcode
 */
int notification_set_time(notification_h noti, time_t input_time);


/**
 * @brief Gets a timestamp.
 * @details If ret_time is @c 0, time information is not set before.
 * @since_tizen @if WEARABLE 2.3.1 @elseif MOBILE 2.3 @endif
 * @param[in] noti The notification handle
 * @param[out] ret_time The return time value
 * @return #NOTIFICATION_ERROR_NONE on success,
 *         otherwise any other value on failure
 * @retval #NOTIFICATION_ERROR_NONE Success
 * @retval #NOTIFICATION_ERROR_INVALID_PARAMETER Invalid parameter
 * @pre Notification handle should be created by notification_create().
 * @see notification_create()
 * @par Sample code:
 * @code
#include <notification.h>

{
	time_t ret_time;
	int noti_err = NOTIFICATION_ERROR_NONE;

	noti_err = notification_get_time(noti, &ret_time);
	if (noti_err != NOTIFICATION_ERROR_NONE)
		return;
}
 * @endcode
 */
int notification_get_time(notification_h noti, time_t *ret_time);


/**
 * @brief Gets an insertion timestamp of the notification.
 * @details If ret_time is @c 0, this notification data is not inserted before.
 * @since_tizen @if WEARABLE 2.3.1 @elseif MOBILE 2.3 @endif
 * @param[in] noti The notification handle
 * @param[out] ret_time The return time value
 * @return #NOTIFICATION_ERROR_NONE on success,
 *         otherwise any other value on failure
 * @retval #NOTIFICATION_ERROR_NONE Success
 * @retval #NOTIFICATION_ERROR_INVALID_PARAMETER Invalid parameter
 * @par Sample code:
 * @code
#include <notification.h>

{
	time_t ret_time;
	int noti_err = NOTIFICATION_ERROR_NONE;

	noti_err = notification_get_insert_time(noti, &ret_time);
	if (noti_err != NOTIFICATION_ERROR_NONE)
		return;
}
 * @endcode
 */
int notification_get_insert_time(notification_h noti, time_t *ret_time);


/**
 * @brief Sets the text to display on the notification view.
 * @details Sets title, content string. If the text is formatted data (only %d, %f, %s are supported), type - value pair should be set.
 *          If %d, the type #NOTIFICATION_VARIABLE_TYPE_INT and the value is an integer.
 *          If %f, the type #NOTIFICATION_VARIABLE_TYPE_DOUBLE and the value is a double.
 *          If %s, the type #NOTIFICATION_VARIABLE_TYPE_STRING and the value is a string.
 *          If the type is #NOTIFICATION_VARIABLE_TYPE_COUNT, notification count is displaying with text.
 *          If the value is #NOTIFICATION_COUNT_POS_LEFT, count is displayed at the left of the text.
 *          If the value is #NOTIFICATION_COUNT_POS_IN, count is displayed in the text when text has %d format.
 *          If the value is #NOTIFICATION_COUNT_POS_RIGHT, count is displayed at the right of the text.
 *          Variable parameters should be terminated #NOTIFICATION_VARIABLE_TYPE_NONE.
 * @since_tizen @if WEARABLE 2.3.1 @elseif MOBILE 2.3 @endif
 * @param[in] noti The notification handle
 * @param[in] type The notification text type
 * @param[in] text The basic text
 * @param[in] key The text key for localization
 * @param[in] args_type The variable parameter that type - value pair
 * @return #NOTIFICATION_ERROR_NONE on success,
 *         otherwise any other value on failure
 * @retval #NOTIFICATION_ERROR_NONE Success
 * @retval #NOTIFICATION_ERROR_INVALID_PARAMETER Invalid parameter
 * @pre notification handle should be created by notification_create().
 * @par Sample code:
 * @code
#include <notification.h>

{
	notification_h noti = NULL;
	int noti_err = NOTIFICATION_ERROR_NONE;

	noti = notification_create(NOTIFICATION_TYPE_NOTI);
	if (noti == NULL)
		return;

	noti_err = notification_set_text(noti, NOTIFICATION_TEXT_TYPE_TITLE, "I'm Title",
					"IDS_APP_BODY_IM_TITLE", NOTIFICATION_VARIABLE_TYPE_NONE);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		notification_free(noti);
		return;
	}
}
 * @endcode
 */
int notification_set_text(notification_h noti, notification_text_type_e type,
		const char *text, const char *key, int args_type, ...);


/**
 * @brief Gets the text from the notification handle.
 * @since_tizen @if WEARABLE 2.3.1 @elseif MOBILE 2.3 @endif
 * @param[in] noti The notification handle
 * @param[in] type The notification text type
 * @param[out] text The notification text
 * @return #NOTIFICATION_ERROR_NONE on success,
 *         otherwise any other value on failure
 * @retval #NOTIFICATION_ERROR_NONE Success
 * @retval #NOTIFICATION_ERROR_INVALID_PARAMETER Invalid parameter
 * @par Sample code:
 * @code
#include <notification.h>

{
	notification_h noti = NULL;
	int noti_err = NOTIFICATION_ERROR_NONE;
	char *text = NULL;

	noti_err = notification_get_text(noti, NOTIFICATION_TEXT_TYPE_TITLE, &text);
	if (noti_err != NOTIFICATION_ERROR_NONE)
		return;

}
 * @endcode
 */
int notification_get_text(notification_h noti, notification_text_type_e type, char **text);


/**
 * @brief Sets the timestamp to display on the notification view.
 * @details The timestamp will be converted to a formatted string and it will be displayed on the set text area.
 * @since_tizen @if WEARABLE 2.3.1 @elseif MOBILE 2.3 @endif
 * @param[in] noti The notification handle
 * @param[in] type The notification text type
 * @param[in] time The timestamp
 * @return #NOTIFICATION_ERROR_NONE on success,
 *         otherwise any other value on failure
 * @retval #NOTIFICATION_ERROR_NONE Success
 * @retval #NOTIFICATION_ERROR_INVALID_PARAMETER Invalid parameter
 * @pre Notification handle should be created by notification_create().
 */
int notification_set_time_to_text(notification_h noti, notification_text_type_e type, time_t time);


/**
 * @brief Gets the timestamp from the notification handle.
 * @since_tizen @if WEARABLE 2.3.1 @elseif MOBILE 2.3 @endif
 * @param[in] noti The notification handle
 * @param[in] type The notification text type
 * @param[in] time The pointer of time stamp
 * @return #NOTIFICATION_ERROR_NONE on success,
 *         otherwise any other value on failure
 * @retval #NOTIFICATION_ERROR_NONE Success
 * @retval #NOTIFICATION_ERROR_INVALID_PARAMETER Invalid parameter
 * @pre Notification handle should be created by notification_create().
 */
int notification_get_time_from_text(notification_h noti, notification_text_type_e type, time_t *time);


/**
 * @brief Sets the sound type for the notification.
 * @since_tizen @if WEARABLE 2.3.1 @elseif MOBILE 2.3 @endif
 * @param[in] noti The notification handle
 * @param[in] type The notification sound type
 * @param[in] path The user sound file path
 * @return #NOTIFICATION_ERROR_NONE on success,
 *         otherwise any other value on failure
 * @retval #NOTIFICATION_ERROR_NONE Success
 * @retval #NOTIFICATION_ERROR_INVALID_PARAMETER Invalid parameter
 * @par Sample code:
 * @code
#include <notification.h>

{
	notification_h noti = NULL;
	int noti_err = NOTIFICATION_ERROR_NONE;

	noti_err = notification_set_sound(noti, NOTIFICATION_SOUND_TYPE_DEFAULT, NULL);
	if (noti_err != NOTIFICATION_ERROR_NONE)
		return;

}
 * @endcode
 */
int notification_set_sound(notification_h noti, notification_sound_type_e type, const char *path);


/**
 * @brief Gets the sound type from the notification handle.
 * @since_tizen @if WEARABLE 2.3.1 @elseif MOBILE 2.3 @endif
 * @param[in] noti The notification handle
 * @param[out] type The notification sound type
 * @param[out] path The user sound file path
 * @return #NOTIFICATION_ERROR_NONE on success,
 *         otherwise any other value on failure
 * @retval #NOTIFICATION_ERROR_NONE Success
 * @retval #NOTIFICATION_ERROR_INVALID_PARAMETER Invalid parameter
 * @par Sample code:
 * @code
#include <notification.h>

{
	notification_h noti = NULL;
	int noti_err = NOTIFICATION_ERROR_NONE;
	notification_sound_type_e type = NOTIFICATION_SOUND_TYPE_NONE;

	noti_err = notification_get_sound(noti, &type, NULL);
	if (noti_err != NOTIFICATION_ERROR_NONE)
		return;

}
 * @endcode
 */
int notification_get_sound(notification_h noti, notification_sound_type_e *type, const char **path);


/**
 * @brief Sets the vibration type for the notification.
 * @since_tizen @if WEARABLE 2.3.1 @elseif MOBILE 2.3 @endif
 * @param[in] noti The notification handle
 * @param[in] type The notification vibration type
 * @param[in] path The user vibration file path
 * @return #NOTIFICATION_ERROR_NONE on success,
 *         otherwise any other value on failure
 * @retval #NOTIFICATION_ERROR_NONE Success
 * @retval #NOTIFICATION_ERROR_INVALID_PARAMETER Invalid parameter
 * @par Sample code:
 * @code
#include <notification.h>

{
	notification_h noti = NULL;
	int noti_err = NOTIFICATION_ERROR_NONE;

	noti_err = notification_set_vibration(noti, NOTIFICATION_VIBRATION_TYPE_DEFAULT, NULL);
	if (noti_err != NOTIFICATION_ERROR_NONE)
		return;

}
 * @endcode
 */
int notification_set_vibration(notification_h noti, notification_vibration_type_e type, const char *path);


/**
 * @brief Gets the vibrate type from the notification handle.
 * @since_tizen @if WEARABLE 2.3.1 @elseif MOBILE 2.3 @endif
 * @param[in] noti The notification handle
 * @param[out] type The notification sound type
 * @param[out] path The user vibration file path
 * @return #NOTIFICATION_ERROR_NONE on success,
 *         otherwise other value on failure
 * @retval #NOTIFICATION_ERROR_NONE Success
 * @retval #NOTIFICATION_ERROR_INVALID_PARAMETER Invalid parameter
 * @par Sample code:
 * @code
#include <notification.h>

{
	notification_h noti = NULL;
	int noti_err = NOTIFICATION_ERROR_NONE;
	notification_vibration_type_e type = NOTIFICATION_VIBRATION_TYPE_NONE;

	noti_err = notification_get_vibration(noti, &type, NULL);
	if (noti_err != NOTIFICATION_ERROR_NONE)
		return;

}
  * @endcode
  */
int notification_get_vibration(notification_h noti, notification_vibration_type_e *type, const char **path);


/**
 * @brief Sets the LED displaying option.
 * @since_tizen @if WEARABLE 2.3.1 @elseif MOBILE 2.3 @endif
 * @param[in] noti The notification handle
 * @param[in] operation The LED notification operation
 * @param[in] led_argb The notification LED color
 * @return #NOTIFICATION_ERROR_NONE on success,
 *         otherwise other value on failure
 * @retval #NOTIFICATION_ERROR_NONE Success
 * @retval #NOTIFICATION_ERROR_INVALID_PARAMETER Invalid parameter
 * @par Sample code:
 * @code
#include <notification.h>

{
	notification_h noti = NULL;
	int noti_err = NOTIFICATION_ERROR_NONE;

	noti_err = notification_set_led(noti, NOTIFICATION_LED_OP_ON, NULL);
	if (noti_err != NOTIFICATION_ERROR_NONE)
		return;

}
 * @endcode
 */
int notification_set_led(notification_h noti, notification_led_op_e operation, int led_argb);


/**
 * @brief Gets the LED displaying option from the notification handle.
 * @since_tizen @if WEARABLE 2.3.1 @elseif MOBILE 2.3 @endif
 * @param[in] noti The notification handle
 * @param[out] operation The LED notification operation
 * @param[out] led_argb The notification LED color
 * @return #NOTIFICATION_ERROR_NONE on success,
 *         otherwise any other value on failure
 * @retval #NOTIFICATION_ERROR_NONE Success
 * @retval #NOTIFICATION_ERROR_INVALID_PARAMETER Invalid parameter
 * @par Sample code:
 * @code
#include <notification.h>

{
	notification_h noti = NULL;
	int noti_err = NOTIFICATION_ERROR_NONE;
	notification_led_type_e type = NOTIFICATION_LED_OP_OFF;

	noti_err = notification_get_led(noti, &type, NULL);
	if (noti_err != NOTIFICATION_ERROR_NONE)
		return;

}
  * @endcode
  */
int notification_get_led(notification_h noti, notification_led_op_e *operation, int *led_argb);


/**
 * @brief Sets the time period of flashing the LED.
 * @since_tizen @if WEARABLE 2.3.1 @elseif MOBILE 2.3 @endif
 * @param[in] noti The notification handle
 * @param[in] on_ms The time for turning on the LED
 * @param[in] off_ms The time for turning off the LED
 * @return #NOTIFICATION_ERROR_NONE on success,
 *         otherwise any other value on failure
 * @retval #NOTIFICATION_ERROR_NONE Success
 * @retval #NOTIFICATION_ERROR_INVALID_PARAMETER Invalid parameter
 * @par Sample code:
 * @code
#include <notification.h>

{
	notification_h noti = NULL;
	int noti_err = NOTIFICATION_ERROR_NONE;

	noti_err = notification_set_led_time_period(noti, 100, 100);
	if (noti_err != NOTIFICATION_ERROR_NONE)
		return;

}
 * @endcode
 */
int notification_set_led_time_period(notification_h noti, int on_ms, int off_ms);


/**
 * @brief Gets the time period of flashing the LED from the notification handle.
 * @since_tizen @if WEARABLE 2.3.1 @elseif MOBILE 2.3 @endif
 * @param[in] noti The notification handle
 * @param[out] on_ms The time for turning on the LED
 * @param[out] off_ms The time for turning on the LED
 * @return #NOTIFICATION_ERROR_NONE on success,
 *         otherwise any other value on failure
 * @retval #NOTIFICATION_ERROR_NONE Success
 * @retval #NOTIFICATION_ERROR_INVALID_PARAMETER Invalid parameter
 * @par Sample code:
 * @code
#include <notification.h>

{
	notification_h noti = NULL;
	int noti_err = NOTIFICATION_ERROR_NONE;
	int led_on_ms = 0;
	int led_off_ms = 0;

	noti_err = notification_get_led_time_period(noti, &led_on_ms, &led_off_ms);
	if (noti_err != NOTIFICATION_ERROR_NONE)
		return;

}
  * @endcode
  */
int notification_get_led_time_period(notification_h noti, int *on_ms, int *off_ms);


/**
 * @brief Sets the launch option for a notification.
 * @details When notification data selected in display application, application launched by app_control_send_launch_request with app_control handle.
 * @since_tizen @if WEARABLE 2.3.1 @elseif MOBILE 2.3 @endif
 * @param[in] noti The notification handle
 * @param[in] type Launching option type
 * @param[in] option App Control handler
 * @return #NOTIFICATION_ERROR_NONE on success,
 *         otherwise any other value on failure
 * @retval #NOTIFICATION_ERROR_NONE Success
 * @retval #NOTIFICATION_ERROR_INVALID_PARAMETER Invalid parameter
 * @par Sample code:
 * @code
#include <notification.h>

{
	notification_h noti = NULL;
	app_control_h app_control = NULL;
	int noti_err = NOTIFICATION_ERROR_NONE;

	// Do something

	app_control_create(&app_control);
	app_control_set_app_id(app_control, "org.tizen.app");

	// Do something

	noti_err = notification_set_launch_option(noti, NOTIFICATION_LAUNCH_OPTION_APP_CONTROL, (void *)app_control);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		app_control_destroy(app_control);
		notification_free(noti);
		return;
	}

	app_control_destroy(app_control);
}
 * @endcode
 */
int notification_set_launch_option(notification_h noti, notification_launch_option_type type, void *option);


/**
 * @brief Gets the launch option from the notification handle.
 * @since_tizen @if WEARABLE 2.3.1 @elseif MOBILE 2.3 @endif
 * @remarks You must release @a app_control using app_control_destroy().
 * @param[in] noti The notification handle
 * @param[in] type Launching option type
 * @param[out] option The pointer of App Control handler
 * @return #NOTIFICATION_ERROR_NONE on success,
 *         otherwise any other value on failure
 * @retval #NOTIFICATION_ERROR_NONE Success
 * @retval #NOTIFICATION_ERROR_INVALID_PARAMETER Invalid parameter
 * @par Sample code:
 * @code
#include <notification.h>

{
	app_control_h app_control = NULL;
	app_control_create(&app_control);

	// Do something

	noti_err = notification_get_launch_option(noti, NOTIFICATION_LAUNCH_OPTION_APP_CONTROL, (void *)&app_control);
	if (noti_err != NOTIFICATION_ERROR_NONE)
		return;
}
 * @endcode
 */
int notification_get_launch_option(notification_h noti, notification_launch_option_type type, void *option);


/**
 * @brief Sets the handler for a specific event.
 * @details When some event occurs on notification, application launched by app_control_send_launch_request with app_control handle. \n
 *          Setting event handler of a button means that the notification will show the button.
 * @since_tizen @if MOBILE 2.4 @elseif WEARABLE 3.0 @endif
 * @param[in] noti The notification handle
 * @param[in] event_type Event type
 * @param[in] event_handler App control handle
 * @return #NOTIFICATION_ERROR_NONE on success,
 *         otherwise any other value on failure
 * @retval #NOTIFICATION_ERROR_NONE Success
 * @retval #NOTIFICATION_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #notification_event_type_e
 * @par Sample code:
 * @code
#include <notification.h>

{
	notification_h noti = NULL;
	app_control_h app_control = NULL;
	int noti_err = NOTIFICATION_ERROR_NONE;

	// Do something

	app_control_create(&app_control);
	app_control_set_app_id(app_control, "org.tizen.app");

	// Do something

	noti_err = notification_set_event_handler(noti, NOTIFICATION_EVENT_TYPE_CLICK_ON_BUTTON_1, app_control);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		app_control_destroy(app_control);
		notification_free(noti);
		return;
	}

	app_control_destroy(app_control);
}
 * @endcode
 */
int notification_set_event_handler(notification_h noti, notification_event_type_e event_type, app_control_h event_handler);


/**
 * @brief Gets the event handler of a specific event.
 * @remarks You must release @a app_control using app_control_destroy().
 * @since_tizen @if MOBILE 2.4 @elseif WEARABLE 3.0 @endif
 * @param[in] noti The notification handle
 * @param[in] event_type Launching option type
 * @param[out] event_handler The handler of App Control
 * @return #NOTIFICATION_ERROR_NONE on success,
 *         otherwise any other value on failure
 * @retval #NOTIFICATION_ERROR_NONE Success
 * @retval #NOTIFICATION_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #notification_event_type_e
 * @par Sample code:
 * @code
#include <notification.h>

{
	app_control_h app_control = NULL;
	app_control_create(&app_control);

	// Do something

	noti_err = notification_get_event_handler(noti, NOTIFICATION_EVENT_TYPE_CLICK_ON_BUTTON_1, &app_control);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		notification_free(noti);
		return;
	}
}
 * @endcode
 */
int notification_get_event_handler(notification_h noti, notification_event_type_e event_type, app_control_h *event_handler);


/**
 * @brief Sets the property of the notification.
 * @since_tizen @if WEARABLE 2.3.1 @elseif MOBILE 2.3 @endif
 * @param[in] noti The notification handle
 * @param[in] flags The property with | operation
 * @return #NOTIFICATION_ERROR_NONE on success,
 *         otherwise any other value on failure
 * @retval #NOTIFICATION_ERROR_NONE Success
 * @retval #NOTIFICATION_ERROR_INVALID_PARAMETER Invalid input value
 * @par Sample code:
 * @code
#include <notification.h>

{
	notification_h noti = NULL;
	int noti_err = NOTIFICATION_ERROR_NONE;
	bundle *b = NULL;

	noti = notification_create(NOTIFICATION_TYPE_NOTI);
	if (noti == NULL)
		return;

	noti_err = notification_set_property(noti, NOTIFICATION_PROP_DISPLAY_ONLY_SIMMODE | NOTIFICATION_PROP_DISABLE_APP_LAUNCH);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		notification_free(noti);
		return;
	}
}
 * @endcode
 */
int notification_set_property(notification_h noti, int flags);


/**
 * @brief Gets the property of the notification from the notification handle.
 * @since_tizen @if WEARABLE 2.3.1 @elseif MOBILE 2.3 @endif
 * @param[in] noti The notification handle
 * @param[out] flags The notification property
 * @return #NOTIFICATION_ERROR_NONE on success,
 *         otherwise any other value on failure
 * @retval #NOTIFICATION_ERROR_NONE Success
 * @retval #NOTIFICATION_ERROR_INVALID_PARAMETER Invalid input value
 * @par Sample code:
 * @code
#include <notification.h>

{
	notification_h noti = NULL;
	int noti_err = NOTIFICATION_ERROR_NONE;
	int flags = 0;

	noti_err = notification_get_property(noti, &flags);
	if (noti_err != NOTIFICATION_ERROR_NONE)
		return;

}
 * @endcode
 */
int notification_get_property(notification_h noti, int *flags);


/**
 * @brief Sets applications to display the notification.
 * @details All display application is enabled(NOTIFICATION_DISPLAY_APP_ALL) if you do not call this API.
 * @since_tizen @if WEARABLE 2.3.1 @elseif MOBILE 2.3 @endif
 * @param[in] noti The notification handle
 * @param[in] applist The with | operation
 * @return #NOTIFICATION_ERROR_NONE on success,
 *         otherwise any other value on failure
 * @retval #NOTIFICATION_ERROR_NONE Success
 * @retval #NOTIFICATION_ERROR_INVALID_PARAMETER Invalid input value
 * @par Sample code:
 * @code
#include <notification.h>

{
	notification_h noti = NULL;
	int noti_err = NOTIFICATION_ERROR_NONE;
	bundle *b = NULL;

	noti = notification_create(NOTIFICATION_TYPE_NOTI);
	if (noti == NULL)
		return;

	noti_err = notification_set_display_applist(noti, NOTIFICATION_DISPLAY_APP_NOTIFICATION_TRAY |
							NOTIFICATION_DISPLAY_APP_TICKER | NOTIFICATION_DISPLAY_APP_INDICATOR);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		notification_free(noti);
		return;
	}
}
 * @endcode
 */
int notification_set_display_applist(notification_h noti, int applist);


/**
 * @brief Gets the application list displaying the notification from the notification handle.
 * @since_tizen @if WEARABLE 2.3.1 @elseif MOBILE 2.3 @endif
 * @param[in] noti The notification handle
 * @param[out] applist The display application list
 * @return #NOTIFICATION_ERROR_NONE on success,
 *         otherwise any other value on failure
 * @retval #NOTIFICATION_ERROR_NONE Success
 * @retval #NOTIFICATION_ERROR_INVALID_PARAMETER Invalid input value
 * @par Sample code:
 * @code
#include <notification.h>

{
	notification_h noti = NULL;
	int noti_err = NOTIFICATION_ERROR_NONE;
	int applist = 0;

	noti_err = notification_get_display_applist(noti, &applist);
	if (noti_err != NOTIFICATION_ERROR_NONE)
		return;

}
 * @endcode
 */
int notification_get_display_applist(notification_h noti, int *applist);


/**
 * @brief Sets the initial size for the ongoing type.
 * @details After notification_post() call, the size is not updated.
 * @since_tizen @if WEARABLE 2.3.1 @elseif MOBILE 2.3 @endif
 * @param[in] noti The notification handle
 * @param[in] size The double type size
 * @return #NOTIFICATION_ERROR_NONE on success,
 *         otherwise any other value on failure
 * @retval #NOTIFICATION_ERROR_NONE Success
 * @retval #NOTIFICATION_ERROR_INVALID_PARAMETER Invalid input value
 * @par Sample code:
 * @code
#include <notification.h>

{
	notification_h noti = NULL;
	int noti_err = NOTIFICATION_ERROR_NONE;

	noti = notification_create(NOTIFICATION_TYPE_NOTI);
	if (noti == NULL)
		return;

	noti_err = notification_set_size(noti, 0.0);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		notification_free(noti);
		return;
	}
}
 * @endcode
 */
int notification_set_size(notification_h noti, double size);


/**
 * @brief Gets the progress size.
 * @since_tizen @if WEARABLE 2.3.1 @elseif MOBILE 2.3 @endif
 * @param[in] noti The notification handle
 * @param[out] size The progress size
 * @return #NOTIFICATION_ERROR_NONE on success,
 *         otherwise any other value on failure
 * @retval #NOTIFICATION_ERROR_NONE Success
 * @retval #NOTIFICATION_ERROR_INVALID_PARAMETER Invalid input value
 * @par Sample code:
 * @code
#include <notification.h>

{
	notification_h noti = NULL;
	int noti_err = NOTIFICATION_ERROR_NONE;
	double size = 0.0;

	noti_err = notification_get_size(noti, &size);
	if (noti_err != NOTIFICATION_ERROR_NONE)
		return;
}
 * @endcode
 */
int notification_get_size(notification_h noti, double *size);


/**
 * @brief Sets the initial progress for the ongoing type.
 * @details After the notification_post() call, the progress is not updated.
 * @since_tizen @if WEARABLE 2.3.1 @elseif MOBILE 2.3 @endif
 * @param[in] noti The notification handle
 * @param[in] percentage The progress percentage
 * @return #NOTIFICATION_ERROR_NONE on success,
 *         otherwise any other value on failure
 * @retval #NOTIFICATION_ERROR_NONE Success
 * @retval #NOTIFICATION_ERROR_INVALID_PARAMETER Invalid input value
 * @par Sample code:
 * @code
#include <notification.h>

{
	notification_h noti = NULL;
	int noti_err = NOTIFICATION_ERROR_NONE;

	noti = notification_create(NOTIFICATION_TYPE_NOTI);
	if (noti == NULL)
		return;

	noti_err = notification_set_progress(noti, 0.0);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		notification_free(noti);
		return;
	}
}
 * @endcode
 */
int notification_set_progress(notification_h noti, double percentage);


/**
 * @brief Gets the progress from the notification handle.
 * @since_tizen @if WEARABLE 2.3.1 @elseif MOBILE 2.3 @endif
 * @remarks At the end of the operation, the progress should be @c 1.0.
 * @param[in] noti The notification handle
 * @param[out] percentage The progress percentage
 * @return #NOTIFICATION_ERROR_NONE on success,
 *         otherwise any other value on failure
 * @retval #NOTIFICATION_ERROR_NONE Success
 * @retval #NOTIFICATION_ERROR_INVALID_PARAMETER Invalid input value
 * @par Sample code:
 * @code
#include <notification.h>

{
	notification_h noti = NULL;
	int noti_err = NOTIFICATION_ERROR_NONE;
	double percentage = 0.0;

	noti_err = notification_get_progress(noti, &percentage);
	if (noti_err != NOTIFICATION_ERROR_NONE)
		return;

}
 * @endcode
 */
int notification_get_progress(notification_h noti, double *percentage);


/**
 * @brief Sets the layout of the notification view.
 * @details Caller can set displaying layout of notification.
 * @since_tizen @if WEARABLE 2.3.1 @elseif MOBILE 2.3 @endif
 * @param[in] noti The notification handle
 * @param[in] layout The type of layout
 * @return #NOTIFICATION_ERROR_NONE on success,
 *         otherwise any other value on failure
 * @retval #NOTIFICATION_ERROR_NONE Success
 * @retval #NOTIFICATION_ERROR_INVALID_PARAMETER Invalid input value
 * @see #notification_ly_type_e
 */
int notification_set_layout(notification_h noti, notification_ly_type_e layout);


/**
 * @brief Gets the layout of the notification view from the notification handle.
 * @since_tizen @if WEARABLE 2.3.1 @elseif MOBILE 2.3 @endif
 * @param[in] noti The notification handle
 * @param[out] layout The type of layout
 * @return #NOTIFICATION_ERROR_NONE on success,
 *         otherwise any other value on failure
 * @retval #NOTIFICATION_ERROR_NONE Success
 * @retval #NOTIFICATION_ERROR_INVALID_PARAMETER Invalid input value
 * @see #notification_ly_type_e
 */
int notification_get_layout(notification_h noti, notification_ly_type_e *layout);


/**
 * @brief Gets the type of a notification.
 * @since_tizen @if WEARABLE 2.3.1 @elseif MOBILE 2.3 @endif
 * @param[in] noti The notification handle
 * @param[out] type The notification type
 * @return #NOTIFICATION_ERROR_NONE on success,
 *         otherwise any other value on failure
 * @retval #NOTIFICATION_ERROR_NONE Success
 * @retval #NOTIFICATION_ERROR_INVALID_PARAMETER Invalid parameter
 * @par Sample code:
 * @code
#include <notification.h>

{
	int noti_err = NOTIFICATION_ERROR_NONE;
	notification_type_e type;

	noti_err = notification_get_type(noti, &type);
	if (noti_err != NOTIFICATION_ERROR_NONE)
		return;
}
 * @endcode
 */
int notification_get_type(notification_h noti, notification_type_e *type);


/**
 * @brief Updates notification data.
 * @details The updated notification will appear in the notification area.
 * @since_tizen @if WEARABLE 2.3.1 @elseif MOBILE 2.3 @endif
 * @privlevel public
 * @privilege %http://tizen.org/privilege/notification
 * @param[in] noti The notification handle that is created by notification_create()
 * @return #NOTIFICATION_ERROR_NONE on success,
 *         otherwise any other value on failure
 * @retval #NOTIFICATION_ERROR_NONE Success
 * @retval #NOTIFICATION_ERROR_INVALID_PARAMETER Invalid input value
 * @retval #NOTIFICATION_ERROR_NOT_EXIST_ID Priv ID does not exist
 * @retval #NOTIFICATION_ERROR_PERMISSION_DENIED The application does not have the privilege to call this method
 * @par Sample code:
 * @code
#include <notification.h>

{
	int noti_err = NOTIFICATION_ERROR_NONE;

	noti_err = notification_update(NULL);
	if (noti_err != NOTIFICATION_ERROR_NONE)
		return;
}
 * @endcode
 */
int notification_update(notification_h noti);


/**
 * @brief Deletes a notification with the given handle.
 * @details notification_delete() removes notification data from database and notification_free() releases memory of notification data.
 * @since_tizen @if WEARABLE 2.3.1 @elseif MOBILE 2.3 @endif
 * @privlevel public
 * @privilege %http://tizen.org/privilege/notification
 * @param[in] noti The notification handle
 * @return #NOTIFICATION_ERROR_NONE on success,
 *         otherwise any other value on failure
 * @retval #NOTIFICATION_ERROR_NONE Success
 * @retval #NOTIFICATION_ERROR_INVALID_PARAMETER Invalid input value
 * @retval NOTIFICATION_ERROR_PERMISSION_DENIED The application does not have the privilege to call this method
 * @par Sample code:
 * @code
#include <notification.h>

{
	notificaton_h noti = NULL;
	int noti_err = NOTIFICATION_ERROR_NONE;

	// Do something

	noti_err = notification_delete(noti);
	if (noti_err != NOTIFICATION_ERROR_NONE)
		return;

}
 * @endcode
 */
int notification_delete(notification_h noti);


/**
 * @brief Creates internal structure data and returns a notification handle.
 * @details Available type is #NOTIFICATION_TYPE_NOTI and #NOTIFICATION_TYPE_ONGOING.
 *          #NOTIFICATION_TYPE_NOTI is remaining notification data even if device is restarted.
 *          #NOTIFICATION_TYPE_ONGOING can display progress on a notification with #NOTIFICATION_LY_ONGOING_PROGRESS layout.
 * @since_tizen @if WEARABLE 2.3.1 @elseif MOBILE 2.3 @endif
 * @remarks The specific error code can be obtained using the get_last_result() method. Error codes are described in Exception section.
 * @param[in] type The notification type
 * @return Notification handle(notification_h) on success,
 *         otherwise @c NULL on failure
 * @retval notification_h Success
 * @retval NULL Failure
 * @exception #NOTIFICATION_ERROR_NONE Success
 * @exception #NOTIFICATION_ERROR_INVALID_PARAMETER Invalid input value
 * @exception #NOTIFICATION_ERROR_OUT_OF_MEMORY Out of memory
 * @see #notification_type_e
 * @par Sample code:
 * @code
#include <notification.h>

{
	notification_h noti = NULL;

	noti = notification_create(NOTIFICATION_TYPE_NOTI);
	if (noti == NULL)
		return;

	// Do something

}
 * @endcode
 */
notification_h notification_create(notification_type_e type);


/**
 * @brief Creates a notification clone.
 * @details Newly created notification handle is returned.
 * @since_tizen @if WEARABLE 2.3.1 @elseif MOBILE 2.3 @endif
 * @remarks This cloned notification handle should be freed using notification_free().
 * @param[in] noti The notification handle
 * @param[out] clone The newly created notification handle that has same with input @a noti
 * @return #NOTIFICATION_ERROR_NONE if success,
 *         otherwise any other value if failure
 * @retval #NOTIFICATION_ERROR_NONE Success
 * @retval #NOTIFICATION_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #notification_type_e
 * @par Sample code:
 * @code
#include <notification.h>

{
	notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);
	notification_h clone = NULL;

	notification_clone(noti, &clone);

	// Do something

}
 * @endcode
 */
int notification_clone(notification_h noti, notification_h *clone);


/**
 * @brief Frees the internal structure data of a notification handle.
 * @details Internal data of a notification handle is released. Data of the inserted notification is not deleted.
 * @since_tizen @if WEARABLE 2.3.1 @elseif MOBILE 2.3 @endif
 * @param[in] noti The notification handle
 * @return #NOTIFICATION_ERROR_NONE on success,
 *         otherwise any other value on failure
 * @retval #NOTIFICATION_ERROR_NONE Success
 * @retval #NOTIFICATION_ERROR_INVALID_PARAMETER Invalid parameter
 * @pre Notification handle should be created by notification_create().
 * @par Sample code:
 * @code
#include <notification.h>

{
	notification_h noti = NULL;
	int noti_err = NOTIFICATION_ERROR_NONE;

	noti = notification_create(NOTIFICATION_TYPE_NOTI);
	if (noti == NULL)
		return;

	// Do something

	noti_err = notification_free(noti);
	if (noti_err != NOTIFICATION_ERROR_NONE)
		return;

}
 * @endcode
 */
int notification_free(notification_h noti);


/**
 * @}
 */


/**
 * @addtogroup NOTIFICATION_MODULE
 * @{
 */


/**
 * @brief Sets the tag of the notification handle.
 * @since_tizen @if WEARABLE 2.3.1 @elseif MOBILE 2.3 @endif
 * @param[in] noti Notification handle
 * @param[in] tag Tag for loading notification handle
 * @return #NOTIFICATION_ERROR_NONE on success,
 *         other value on failure
 * @retval #NOTIFICATION_ERROR_NONE Success
 * @retval #NOTIFICATION_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #NOTIFICATION_ERROR_PERMISSION_DENIED The application does not have the privilege to call this method
 * @see notification_get_tag()
 * @par Sample code:
 * @code
#include <notification.h>

{
	notification_h noti = NULL;
	int noti_err = NOTIFICATION_ERROR_NONE;

	noti = notification_create(NOTIFICATION_TYPE_NOTI);
	if (noti == NULL)
		return;

	// Do something

	noti_err = notification_set_tag(noti, tag);
	if (noti_err != NOTIFICATION_ERROR_NONE)
		return;

}
 * @endcode
 */
int notification_set_tag(notification_h noti, const char *tag);


/**
 * @brief Gets the tag of the notification handle.
 * @since_tizen @if WEARABLE 2.3.1 @elseif MOBILE 2.3 @endif
 * @param[in] noti Notification handle
 * @param[out] tag Tag for loading notification handle
 * @return #NOTIFICATION_ERROR_NONE on success,
 *         other value on failure
 * @retval #NOTIFICATION_ERROR_NONE Success
 * @retval #NOTIFICATION_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #NOTIFICATION_ERROR_PERMISSION_DENIED The application does not have the privilege to call this method
 * @see notification_set_tag()
 * @par Sample code:
 * @code
#include <notification.h>

{
	int noti_err = NOTIFICATION_ERROR_NONE;
	const char *tag = NULL;

	// Do something

	noti_err = notification_get_tag(noti, &tag);
	if (noti_err != NOTIFICATION_ERROR_NONE)
		return;

}
 * @endcode
 */
int notification_get_tag(notification_h noti, const char **tag);


/**
 * @brief Loads a notification from the notification's database with the tag.
 * @since_tizen @if WEARABLE 2.3.1 @elseif MOBILE 2.3 @endif
 * @privlevel public
 * @privilege %http://tizen.org/privilege/notification
 * @remarks The specific error code can be obtained using the get_last_result() method. Error codes are described in Exception section.
 * @param[in] tag Tag for loading notification handle
 * @return Notification handle(notification_h) on success,
 *         NULL on failure
 * @retval notification_h Success
 * @retval NULL Failure
 * @exception #NOTIFICATION_ERROR_NONE Success
 * @exception #NOTIFICATION_ERROR_INVALID_PARAMETER Invalid input value
 * @exception #NOTIFICATION_ERROR_OUT_OF_MEMORY Out of memory
 * @exception #NOTIFICATION_ERROR_PERMISSION_DENIED The application does not have the privilege to call this method
 * @see #notification_type_e
 * @par Sample code:
 * @code
#include <notification.h>

{
	notification_h noti = NULL;

	noti = notification_load_by_tag(tag);
	if (noti == NULL)
		return;

	// Do something

}
 * @endcode
 */
notification_h notification_load_by_tag(const char *tag);


/**
 * @brief Deletes all notifications of the given type.
 * @since_tizen @if WEARABLE 2.3.1 @elseif MOBILE 2.3 @endif
 * @privlevel public
 * @privilege %http://tizen.org/privilege/notification
 * @param[in] type Notification type
 * @return #NOTIFICATION_ERROR_NONE if success,
 *         other value if failure
 * @retval #NOTIFICATION_ERROR_NONE Success
 * @retval #NOTIFICATION_ERROR_INVALID_PARAMETER Invalid input value
 * @retval #NOTIFICATION_ERROR_PERMISSION_DENIED The application does not have the privilege to call this method
 * @par Sample code:
 * @code
#include <notification.h>

{
	int noti_err = NOTIFICATION_ERROR_NONE;

	noti_err = notification_delete_all(NOTIFICATION_TYPE_NOTI);
	if (noti_err != NOTIFICATION_ERROR_NONE)
		return;

}
 * @endcode
 */
int notification_delete_all(notification_type_e type);


/**
 * @brief Posts a notification.
 * @since_tizen @if WEARABLE 2.3.1 @elseif MOBILE 2.3 @endif
 * @privlevel public
 * @privilege %http://tizen.org/privilege/notification
 * @param[in] noti Notification handle
 * @return #NOTIFICATION_ERROR_NONE if success,
 *         other value if failure
 * @retval #NOTIFICATION_ERROR_NONE Success
 * @retval #NOTIFICATION_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #NOTIFICATION_ERROR_PERMISSION_DENIED The application does not have the privilege to call this method
 * @pre Notification handle should be created by notification_create().
 * @post notification_free().
 * @par Sample code:
 * @code
#include <notification.h>

{
	int noti_err = NOTIFICATION_ERROR_NONE;

	noti_err = notification_post(noti);
	if (noti_err != NOTIFICATION_ERROR_NONE)
		return;

}
 * @endcode
 */
int notification_post(notification_h noti);


/**
 * @brief Gets the package name of the notification.
 * @since_tizen @if MOBILE 2.4 @elseif WEARABLE 3.0 @endif
 * @param[in] noti Notification handle
 * @param[out] pkgname The package name of the notification
 * @return #NOTIFICATION_ERROR_NONE on success,
 *         otherwise a negative error value
 * @retval NOTIFICATION_ERROR_NONE Success
 * @retval NOTIFICATION_ERROR_INVALID_PARAMETER Invalid input value
 * @par Sample code:
 * @code
#include <notification.h>

{
	notification_h noti = NULL;
	int noti_err = NOTIFICATION_ERROR_NONE;
	char *pkgname = NULL;

	// Do something

	noti_err = notification_get_pkgname(noti, &pkgname);

	if (noti_err != NOTIFICATION_ERROR_NONE) {
		notification_free(noti);
		return;
	}
}
 * @endcode
 */
int notification_get_pkgname(notification_h noti, char **pkgname);


/**
 * @brief Adds a button on the notification.
 * @since_tizen @if MOBILE 2.4 @elseif WEARABLE 3.0 @endif
 * @param[in] noti Notification handle
 * @param[in] button_index Button index
 * @return #NOTIFICATION_ERROR_NONE on success,
 *         otherwise a negative error value
 * @retval NOTIFICATION_ERROR_NONE Success
 * @retval NOTIFICATION_ERROR_INVALID_PARAMETER Invalid input value
 * @par Sample code:
 * @code
#include <notification.h>

{
	notification_h noti = NULL;
	int noti_err = NOTIFICATION_ERROR_NONE;
	char *pkgname = NULL;

	// Do something

	noti_err = notification_add_button(noti, NOTIFICATION_BUTTON_1);

	if (noti_err != NOTIFICATION_ERROR_NONE) {
		notification_free(noti);
		return;
	}
}
 * @endcode
 */
int notification_add_button(notification_h noti, notification_button_index_e button_index);

/**
 * @brief Removes a button on the notification.
 * @since_tizen @if MOBILE 2.4 @elseif WEARABLE 3.0 @endif
 * @param[in] noti Notification handle
 * @param[in] button_index Button index
 * @return #NOTIFICATION_ERROR_NONE on success,
 *         otherwise a negative error value
 * @retval NOTIFICATION_ERROR_NONE Success
 * @retval NOTIFICATION_ERROR_INVALID_PARAMETER Invalid input value
 * @par Sample code:
 * @code
#include <notification.h>

{
	notification_h noti = NULL;
	int noti_err = NOTIFICATION_ERROR_NONE;
	char *pkgname = NULL;

	// Do something

	noti_err = notification_remove_button(noti, NOTIFICATION_BUTTON_1);

	if (noti_err != NOTIFICATION_ERROR_NONE) {
		notification_free(noti);
		return;
	}
}
 * @endcode
 */
int notification_remove_button(notification_h noti, notification_button_index_e button_index);


/**
 * @brief Sets the 'auto remove' option of the active notification.
 * @details The 'auto remove' option lets the active notification be removed several seconds after it shows. Default value is true.
 * @remarks When 'auto_remove' is set as false, the active notification will not be removed
 *          as long as the user removes the active notification or the app which posted the active notification removes the active notification.
 * @since_tizen @if MOBILE 2.4 @elseif WEARABLE 3.0 @endif
 * @param[in] noti Notification handle
 * @param[in] auto_remove Auto remove option
 * @return #NOTIFICATION_ERROR_NONE On success,
 *         other value if failure
 * @retval #NOTIFICATION_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #notification_h
 * @see #notification_get_auto_remove
 * @par Sample code:
 * @code
#include <notification.h>

{
	notification_h noti = NULL;
	int noti_err = NOTIFICATION_ERROR_NONE;

	// Do something

	noti_err = notification_set_auto_remove(noti, false);
	if (noti_err != NOTIFICATION_ERROR_NONE)
		return;

}
 * @endcode
 */
int notification_set_auto_remove(notification_h noti, bool auto_remove);


/**
 * @brief Gets the 'auto remove' option of the active notification.
 * @details The 'auto remove' option lets the active notification be removed several seconds after it shows. Default value is true.
 * @since_tizen @if MOBILE 2.4 @elseif WEARABLE 3.0 @endif
 * @param[in] noti Notification handle
 * @param[out] auto_remove Auto remove option
 * @return #NOTIFICATION_ERROR_NONE On success, other value on failure
 * @retval #NOTIFICATION_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #notification_h
 * @see #notification_get_auto_remove
 * @par Sample code:
 * @code
#include <notification.h>

{
	int noti_err = NOTIFICATION_ERROR_NONE;
	bool auto_remove;

	// Do something

	noti_err = notification_get_auto_remove(noti, &auto_remove);
	if (noti_err != NOTIFICATION_ERROR_NONE)
		return;

}
 * @endcode
 */
int notification_get_auto_remove(notification_h noti, bool *auto_remove);


/**
 * @brief Saves a notification template to the notification database.
 * @details An application can save the created notification as a template for later reuse.
 *          If the template has the same name as a saved one, the saved template will be overwritten.
 *          A saved template can be loaded only by the application which saved it.
 *          All templates are removed when the application package is uninstalled.
 * @since_tizen 3.0
 * @privlevel public
 * @privilege %http://tizen.org/privilege/notification
 * @remarks The number of templates is limited to 10.
 *          When you try to add more than 10 templates, #NOTIFICATION_ERROR_MAX_EXCEEDED will be returned.
 * @param[in] noti Notification handle
 * @param[in] template_name Template name
 * @return #NOTIFICATION_ERROR_NONE On success, other value on failure
 * @retval #NOTIFICATION_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #NOTIFICATION_ERROR_IO_ERROR I/O error
 * @retval #NOTIFICATION_ERROR_OUT_OF_MEMORY Out of memory
 * @retval #NOTIFICATION_ERROR_PERMISSION_DENIED The Permission denied
 * @retval #NOTIFICATION_ERROR_MAX_EXCEEDED Max notification count exceeded
 * @see #notification_h
 * @see notification_create_from_template()
 * @par Sample code:
 * @code
#include <notification.h>

{
	int noti_err = NOTIFICATION_ERROR_NONE;

	// Do something

	noti_err = notification_save_as_template(noti, "CALL_ACCEPT");
	if (noti_err != NOTIFICATION_ERROR_NONE)
		return;

}
 * @endcode
 */
int notification_save_as_template(notification_h noti, const char *template_name);


/**
 * @brief Loads a notification template from the notification database.
 * @details An application can load a saved template and post it.
 *          An application can load only templates that it has saved.
 * @since_tizen 3.0
 * @privlevel public
 * @privilege %http://tizen.org/privilege/notification
 * @remarks The returned handle should be destroyed using notification_free().
 *          The specific error code can be obtained using get_last_result().
 *          Error codes are described in the Exception section.
 *          If an invalid template name is given, the result will be set to #NOTIFICATION_ERROR_FROM_DB.
 * @param[in] template_name Template name
 * @return Notification handle on success, NULL on failure
 * @exception #NOTIFICATION_ERROR_NONE Success
 * @exception #NOTIFICATION_ERROR_INVALID_PARAMETER Invalid input value
 * @exception #NOTIFICATION_ERROR_OUT_OF_MEMORY Out of memory
 * @exception #NOTIFICATION_ERROR_PERMISSION_DENIED The application does not have the privilege to call this method
 * @exception #NOTIFICATION_ERROR_FROM_DB Error from DB query
 * @see #notification_h
 * @see notification_save_as_template()
 * @par Sample code:
 * @code
#include <notification.h>

{
	notification_h noti = NULL;

	noti = notification_create_from_template("CALL_ACCEPT");
	if (noti == NULL)
		return;

}
 * @endcode
 */
notification_h notification_create_from_template(const char *template_name);


/**
 * @brief Gets notification block state.
 * @details	The user can set the notification block state in settings.
 *		The block state indicates whether or not notifications can be posted.
 *		Additionally only notifications to the notification panel are
 *		allowed in "Do not disturb mode". Sound, Vibrate and
 *		Active/Instant notifications are blocked.
 * @since_tizen 3.0
 * @privlevel public
 * @privilege %http://tizen.org/privilege/notification
 * @param[out] state Notification block state
 * @return #NOTIFICATION_ERROR_NONE On success, other value on failure
 * @retval #NOTIFICATION_ERROR_NONE Success
 * @retval #NOTIFICATION_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #NOTIFICATION_ERROR_OUT_OF_MEMORY out of memory
 * @retval #NOTIFICATION_ERROR_IO_ERROR I/O Error
 * @retval #NOTIFICATION_ERROR_PERMISSION_DENIED Permission denied
 * @retval #NOTIFICATION_ERROR_SERVICE_NOT_READY No response from notification service
 * @see #notification_block_state_e
 * @par Sample code:
 * @code
#include <notification.h>

{
	int noti_err = NOTIFICATION_ERROR_NONE;
	notification_block_state_e state;

	// Do something

	noti_err = notification_get_noti_block_state(&state);
	if (noti_err != NOTIFICATION_ERROR_NONE)
		return;

}
 * @endcode
 */
int notification_get_noti_block_state(notification_block_state_e *state);


/**
 * @brief Sets a text input box to reply directly on the notification.
 * @details When you add a text input to the active notification, the notification UI will show a text input with a button.
 *          So, the user can enter any text and press the button to confirm the text as a input.
 *          You can edit some UI component that is related to the text input.
 *          First, you can add placeholder text to guide the user using notification_set_text() with #NOTIFICATION_TEXT_TYPE_TEXT_INPUT_PLACEHOLDER type.
 *          You also can edit button for the text input.
 *          For setting just a text to the button, you can set the text using notification_set_text() with #NOTIFICATION_TEXT_TYPE_TEXT_INPUT_BUTTON type.
 *          If you want to show image button, you can set an image for the button using notification_set_image() with #NOTIFICATION_IMAGE_TYPE_TEXT_INPUT_BUTTON type.
 *
 *          Note that you should set an app_control for handling the event for user input using notification_set_event_handler().
 *          #NOTIFICATION_EVENT_TYPE_CLICK_ON_TEXT_INPUT_BUTTON is the event type for the text input.
 *          You can get the text the user enters in the app_control handle that is passed as a result of the event.
 *          The app_control will contain APP_CONTROL_DATA_TEXT key, so you can get the text using app_control_get_extra_data() using APP_CONTROL_DATA_TEXT key.
 *          The value will contain the text user enters.
 *
 *          Note that you are able to make the switching button to the text input box.
 *          You have to set the app_control which you will set in a text input box to the switching button.
 *          Refer to the second sample code.
 * @since_tizen 3.0
 * @param[in] noti Notification handle
 * @param[in] text_input_max_length The maximum value which can be inputted
 * @return #NOTIFICATION_ERROR_NONE on success,
 *         otherwise any other value on failure
 * @retval #NOTIFICATION_ERROR_NONE         Success
 * @retval #NOTIFICATION_ERROR_INVALID_PARAMETER Invalid parameter
 * @par Sample code:
 * @code
#include <notification.h>

{
	int noti_err = NOTIFICATION_ERROR_NONE;
	notification_h noti = NULL;
	app_control = NULL;

	noti = notification_create(NOTIFICATION_TYPE_NOTI);
	if (noti == NULL)
		return;

	noti_err = notification_set_text_input(noti, 160);
	if (noti_err != NOTIFICATION_ERROR_NONE)
		return;

	noti_err = notification_set_text(noti,
			NOTIFICATION_TEXT_TYPE_TEXT_INPUT_PLACEHOLDER,
			"Text message",
			NULL,
			NOTIFICATION_VARIABLE_TYPE_NONE);
	if (noti_err != NOTIFICATION_ERROR_NONE)
		return;

	noti_err = notification_set_text(noti,
			NOTIFICATION_TEXT_TYPE_TEXT_INPUT_BUTTON,
			"SEND",
			NULL,
			NOTIFICATION_VARIABLE_TYPE_NONE);
	if (noti_err != NOTIFICATION_ERROR_NONE)
		return;

	noti_err = notification_set_display_applist(noti,
			NOTIFICATION_DISPLAY_APP_NOTIFICATION_TRAY | NOTIFICATION_DISPLAY_APP_ACTIVE);
	if (noti_err != NOTIFICATION_ERROR_NONE)
		return;

	// Do something

	noti_err = app_control_create(&app_control);
	if (noti_err != APP_CONTROL_ERROR_NONE)
		return;

	noti_err = app_control_set_app_id(app_control, appid);
	if (noti_err != APP_CONTROL_ERROR_NONE)
		return;

	noti_err = app_control_set_operation(app_control, APP_CONTROL_OPERATION_DEFAULT);
	if (noti_err != APP_CONTROL_ERROR_NONE)
		return;

	noti_err = notification_set_event_handler(noti,
			NOTIFICATION_EVENT_TYPE_CLICK_ON_TEXT_INPUT_BUTTON,
			app_control);
	if (noti_err != NOTIFICATION_ERROR_NONE)
		return;

	noti_err = app_control_destroy(app_control);
	if (noti_err != APP_CONTROL_ERROR_NONE)
		return;

	noti_err  = notification_post(noti);
	if(noti_err != NOTIFICATION_ERROR_NONE)
		return;

}


{
	int noti_err = NOTIFICATION_ERROR_NONE;
	notification_h noti = NULL;
	app_control = NULL;

	noti = notification_create(NOTIFICATION_TYPE_NOTI);
	if (noti == NULL) {
		return;
	}

	noti_err = notification_set_text_input(noti, 160);
	if (noti_err != NOTIFICATION_ERROR_NONE)
		return;

	noti_err = notification_set_text(noti,
			NOTIFICATION_TEXT_TYPE_TEXT_INPUT_PLACEHOLDER,
			"Text message",
			NULL,
			NOTIFICATION_VARIABLE_TYPE_NONE);
	if (noti_err != NOTIFICATION_ERROR_NONE)
		return;

	noti_err = notification_set_text(noti,
			NOTIFICATION_TEXT_TYPE_TEXT_INPUT_BUTTON,
			"SEND",
			NULL,
			NOTIFICATION_VARIABLE_TYPE_NONE);
	if (noti_err != NOTIFICATION_ERROR_NONE)
		return;

	noti_err = notification_add_button(notification, NOTIFICATION_BUTTON_1);
	if (noti_err != NOTIFICATION_ERROR_NONE)
		return;

	noti_err = notification_set_text(notification,
			NOTIFICATION_TEXT_TYPE_BUTTON_1,
			"reply",
			NULL,
			NOTIFICATION_VARIABLE_TYPE_NONE);
	if (noti_err != NOTIFICATION_ERROR_NONE)
		return;

	noti_err = notification_set_display_applist(noti,
			NOTIFICATION_DISPLAY_APP_NOTIFICATION_TRAY | NOTIFICATION_DISPLAY_APP_ACTIVE);
	if (noti_err != NOTIFICATION_ERROR_NONE)
		return;

	// Do something

	noti_err = app_control_create(&app_control);
	if (noti_err != APP_CONTROL_ERROR_NONE)
		return;

	noti_err = app_control_set_app_id(app_control, appid);
	if (noti_err != APP_CONTROL_ERROR_NONE)
		return;

	noti_err = app_control_set_operation(app_control, APP_CONTROL_OPERATION_DEFAULT);
	if (noti_err != APP_CONTROL_ERROR_NONE)
		return;

	noti_err = notification_set_event_handler(notification,
			NOTIFICATION_EVENT_TYPE_CLICK_ON_BUTTON_1,
			app_control);
	if (noti_err != NOTIFICATION_ERROR_NONE)
		return;

	noti_err = notification_set_event_handler(noti,
			NOTIFICATION_EVENT_TYPE_CLICK_ON_TEXT_INPUT_BUTTON,
			app_control);
	if (noti_err != NOTIFICATION_ERROR_NONE)
		return;

	noti_err = app_control_destroy(app_control);
	if (noti_err != APP_CONTROL_ERROR_NONE)
		return;

	noti_err  = notification_post(noti);
	if(noti_err != NOTIFICATION_ERROR_NONE)
		return;
}
 * @endcode
 */
int notification_set_text_input(notification_h noti, int text_input_max_length);


/**
 * @}
 */

#ifdef __cplusplus
}
#endif
#endif /* __NOTIFICATION_H__ */

