/*
 * Copyright (c) 2000 - 2017 Samsung Electronics Co., Ltd. All rights reserved.
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

#ifndef __NOTIFICATION_TYPE_INTERNAL_H__
#define __NOTIFICATION_TYPE_INTERNAL_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup NOTIFICATION_INTERNAL
 * @{
 */

#define NOTIFICATION_GLOBAL_UID -1
#define NOTIFICATION_DISPLAY_APP_HEADS_UP NOTIFICATION_DISPLAY_APP_ACTIVE /* To avoid build error */

#define NOTIFICATION_LY_MAX NOTIFICATION_LY_EXTENSION
#define NOTIFICATION_EVENT_TYPE_MAX NOTIFICATION_EVENT_TYPE_CLICK_ON_BUTTON_10
#define NOTIFICATION_SOUND_TYPE_MAX NOTIFICATION_SOUND_TYPE_USER_DATA
#define NOTIFICATION_VIBRATION_TYPE_MAX NOTIFICATION_VIBRATION_TYPE_USER_DATA
#define NOTIFICATION_LED_OP_MAX NOTIFICATION_LED_OP_ON_CUSTOM_COLOR
#define NOTIFICATION_COUNT_DISPLAY_TYPE_MAX NOTIFICATION_COUNT_DISPLAY_TYPE_RIGHT
#define NOTIFICATION_TEXT_TYPE_MAX NOTIFICATION_TEXT_TYPE_BUTTON_10
#define NOTIFICATION_IMAGE_TYPE_MAX NOTIFICATION_IMAGE_TYPE_BUTTON_10
#define NOTIFICATION_EXECUTE_TYPE_MAX NOTIFICATION_EXECUTE_TYPE_MULTI_LAUNCH
#define NOTIFICATION_TYPE_MAX NOTIFICATION_TYPE_ONGOING
#define NOTIFICATION_OP_DATA_MAX NOTIFICATION_OP_DATA_EXTRA_INFO_2
#define NOTIFICATION_COUNT_POS_MAX NOTIFICATION_COUNT_POS_RIGHT
#define NOTIFICATION_VARIABLE_TYPE_MAX NOTIFICATION_VARIABLE_TYPE_COUNT

/**
 * @brief Enumeration for notification resource path type.
 * @since_tizen 3.0
 */
typedef enum _notification_res_path_type {
	NOTIFICATION_RES_PATH_TYPE_SOUND = NOTIFICATION_IMAGE_TYPE_MAX + 1, /**< Sound */
	NOTIFICATION_RES_PATH_TYPE_VIBRATION, /**< Vibration */
} notification_res_path_type_e;

/**
 * @brief Enumeration for notification ongoing value type.
 * @since_tizen 3.0
 */
typedef enum _notification_ongoing_value_type {
	NOTIFICATION_ONGOING_VALUE_TYPE_PERCENT = 0, /**< Percent */
	NOTIFICATION_ONGOING_VALUE_TYPE_TIME, /**< Time */
} notification_ongoing_value_type_e;

/**
 * @brief Enumeration for extension event type on notification.
 * @since_tizen 3.0
 * @see #notification_event_type_e
 */
typedef enum _notification_event_type_extension {
	NOTIFICATION_EVENT_TYPE_HIDDEN_BY_USER = 100, /**< Hidden by user */
	NOTIFICATION_EVENT_TYPE_HIDDEN_BY_TIMEOUT = 101, /**< Hidden by timeout */
	NOTIFICATION_EVENT_TYPE_HIDDEN_BY_EXTERNAL = 102, /**< Hidden by external */
	NOTIFICATION_EVENT_TYPE_PRESSED = 200, /**< Pressed by user */
	NOTIFICATION_EVENT_TYPE_DELETED = 201, /**< Deleted by user */
	NOTIFICATION_EVENT_TYPE_CHECK_BOX = 300, /**< Check Box */
} notification_event_type_extension_e;

/**
 * @}
 */

#ifdef __cplusplus
}
#endif
#endif /* __NOTIFICATION_TYPE_INTERNAL_H__ */
