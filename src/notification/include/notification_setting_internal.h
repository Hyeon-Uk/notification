/*
 * Copyright (c) 2015 - 2017 Samsung Electronics Co., Ltd. All rights reserved.
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

#ifndef __NOTIFICATION_SETTING_INTERNAL_H__
#define __NOTIFICATION_SETTING_INTERNAL_H__

#include <sys/types.h>
#include <stdbool.h>
#include <glib.h>
#include "notification.h"
#include "notification_setting.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct notification_system_setting *notification_system_setting_h;
typedef struct notification_system_setting_dnd_allow_exception *dnd_allow_exception_h;

/**
 * @brief The prototype of handler that 'Do not disturb' mode set schdule changed alarm.
 * @since_tizen 3.0
 */
typedef void (*dnd_changed_cb)(void *user_data, int do_not_disturb);

/**
 * @brief Enumeration for Week Flag, the days of the week.
 * @since_tizen 3.0
 */
typedef enum {
	DND_SCHEDULE_WEEK_FLAG_SUNDAY = 0x01,		/**< Sunday */
	DND_SCHEDULE_WEEK_FLAG_MONDAY = 0x02,		/**< Monday */
	DND_SCHEDULE_WEEK_FLAG_TUESDAY = 0x04,		/**< Tuesday */
	DND_SCHEDULE_WEEK_FLAG_WEDNESDAY = 0x08,	/**< Wednesday */
	DND_SCHEDULE_WEEK_FLAG_THURSDAY = 0x10,		/**< Thursday */
	DND_SCHEDULE_WEEK_FLAG_FRIDAY = 0x20,		/**< Friday */
	DND_SCHEDULE_WEEK_FLAG_SATURDAY = 0x40,		/**< Saturday */
	DND_SCHEDULE_WEEK_FLAG_ALL = DND_SCHEDULE_WEEK_FLAG_SUNDAY|
					DND_SCHEDULE_WEEK_FLAG_MONDAY |
					DND_SCHEDULE_WEEK_FLAG_TUESDAY |
					DND_SCHEDULE_WEEK_FLAG_WEDNESDAY |
					DND_SCHEDULE_WEEK_FLAG_THURSDAY |
					DND_SCHEDULE_WEEK_FLAG_FRIDAY |
					DND_SCHEDULE_WEEK_FLAG_SATURDAY
} dnd_schedule_week_flag_e;

/**
 * @brief Enumeration for lock screen content.
 * @since_tizen 3.0
 */
typedef enum lock_screen_content_level {
	SHOW_ALL_CONTENT = 0, /**< Show all*/
	HIDE_SENSITIVE_CONTENT, /**< Hide sensitive */
	DO_NOT_SHOW_NOTIFICATIONS, /**< Do not Showw */
} lock_screen_content_level_e;

/**
 * @brief Enumeration for do_not_disturb allow exception type.
 * @since_tizen 3.0
 */
typedef enum dnd_allow_exception_type {
	NOTIFICATION_DND_ALLOWED_CALLS = 0, /**< Call */
	/* possible to add */
} dnd_allow_exception_type_e;

/**
 * @brief Enumeration for allowed_calls.
 * @since_tizen 3.0
 */
typedef enum notification_dnd_allowed_calls {
	NOTIFICATION_DND_ALLOWED_CALLS_EVERYONE = 0, /**< Everyone */
	NOTIFICATION_DND_ALLOWED_CALLS_CONTACT, /**< Contact */
	NOTIFICATION_DND_ALLOWED_CALLS_FAVORITE, /**< Favorite */
	NOTIFICATION_DND_ALLOWED_CALLS_NOBODY, /**< Nobody */
} notification_dnd_allowed_calls_e;

/* Application setting */
struct notification_setting {
	char *package_name;
	char *app_id;
	bool allow_to_notify;
	bool do_not_disturb_except;
	bool pop_up_notification;
	int visibility_class;
	lock_screen_content_level_e lock_screen_content_level;
	bool app_disabled;
};

/* System setting */
struct notification_system_setting {
	bool do_not_disturb;
	int visibility_class;
	bool dnd_schedule_enabled;
	int dnd_schedule_day;
	int dnd_start_hour;
	int dnd_start_min;
	int dnd_end_hour;
	int dnd_end_min;
	lock_screen_content_level_e lock_screen_content_level;
	GList *dnd_allow_exceptions;
};

/* dnd_allow_exception */
struct notification_system_setting_dnd_allow_exception {
	int type;
	int value;
};

/**
 * @internal
 * @brief Gets the array of notification setting.
 * @since_tizen 2.3
 * @privlevel public
 * @privilege %http://tizen.org/privilege/notification
 * @param[out] setting_array The array of notification setting
 * @param[out] count The count of array
 * @return #NOTIFICATION_ERROR_NONE on success,
 *         otherwise any other value on failure
 * @retval #NOTIFICATION_ERROR_NONE         Success
 * @retval #NOTIFICATION_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #NOTIFICATION_ERROR_IO_ERROR I/O error
 * @retval #NOTIFICATION_ERROR_OUT_OF_MEMORY out of memory
 * @retval #NOTIFICATION_ERROR_SERVICE_NOT_READY No reponse from notification service
 * @retval #NOTIFICATION_ERROR_PERMISSION_DENIED Permission denied
 * @par Sample code:
 * @code
#include <notification_setting_internal.h>
...
{
	int noti_err = 0;
	int count = 0;
	notification_setting_h setting;

	noti_err = notification_setting_get_setting_array(&setting, &count);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		return;
	}

	...

	notification_setting_free_notification(setting);
}
 * @endcode
 */

int notification_setting_get_setting_array(notification_setting_h *setting_array, int *count);
int notification_setting_get_setting_array_for_uid(notification_setting_h *setting_array, int *count, uid_t uid);

/**
 * @internal
 * @brief Gets notification setting by package name.
 * @since_tizen @if wearable 2.3.1 @elseif mobile 2.3 @endif
 * @privlevel public
 * @privilege %http://tizen.org/privilege/notification
 * @param[in] package_name The package_name
 * @param[out] setting The notification setting
 * @return #NOTIFICATION_ERROR_NONE on success,
 *         otherwise any other value on failure
 * @retval #NOTIFICATION_ERROR_NONE			Success
 * @retval #NOTIFICATION_ERROR_INVALID_PARAMETER	Invalid parameter
 * @retval #NOTIFICATION_ERROR_IO_ERROR I/O error	I/O error
 * @retval #NOTIFICATION_ERROR_OUT_OF_MEMORY		out of memory
 * @retval #NOTIFICATION_ERROR_SERVICE_NOT_READY	No reponse from notification service
 * @retval #NOTIFICATION_ERROR_PERMISSION_DENIED	Permission denied
 * @par sample code:
 * @code
#include <notification_setting_internal.h>
...
{
	int noti_err = 0;
	notification_setting_h setting = NULL;

	noti_err = notification_setting_get_setting_by_package_name(PACKAGE_NAME, &setting);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		return;
	}

	notification_setting_free_notification(setting);

	return 0;

}
 * @endcode
 */
int notification_setting_get_setting_by_package_name(const char *package_name, notification_setting_h *setting);
int notification_setting_get_setting_by_appid_for_uid(const char *app_id, notification_setting_h *setting, uid_t uid);

/**
 * @internal
 * @brief Gets package name from notification setting handle.
 * @since_tizen @if wearable 2.3.1 @elseif mobile 2.3 @endif
 * @param[in] setting The notification setting handle
 * @param[out] value The package name
 * @return #NOTIFICATION_ERROR_NONE on success,
 *         otherwise any other value on failure
 * @retval #NOTIFICATION_ERROR_NONE         Success
 * @retval #NOTIFICATION_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #NOTIFICATION_ERROR_NOT_EXIST_ID Already exist private ID
 * @par sample code:
 * @code
#include <notification_setting_internal.h>
...
{
	int noti_err = 0;
	char *package_name = NULL;
	notification_setting_h setting = NULL;

	...

	noti_err = notification_setting_get_package_name(setting, &package_name);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		return;
	}

	notification_setting_free_notification(setting);

	return 0;

}
 * @endcode
 */
int notification_setting_get_package_name(notification_setting_h setting, char **value);

/**
 * @internal
 * @brief Gets application id from notification setting handle.
 * @since_tizen 3.0
 * @param[in] setting The notification setting handle
 * @param[out] app_id The application id
 * @return #NOTIFICATION_ERROR_NONE on success,
 *         otherwise any other value on failure
 * @retval #NOTIFICATION_ERROR_NONE         Success
 * @retval #NOTIFICATION_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #NOTIFICATION_ERROR_NOT_EXIST_ID Already exist private ID
 * @par sample code:
 * @code
#include <notification_setting_internal.h>
...
{
	int noti_err = 0;
	char *app_id = NULL;
	notification_setting_h setting = NULL;

	...

	noti_err = notification_setting_get_appid(setting, &app_id);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		return;
	}

	notification_setting_free_notification(setting);

	return 0;

}
 * @endcode
 */
int notification_setting_get_appid(notification_setting_h setting, char **app_id);

/**
 * @internal
 * @brief Gets value which whether allow notification from individual applications.
 * @since_tizen @if wearable 2.3.1 @elseif mobile 2.3 @endif
 * @param[in] setting The notification setting handle
 * @param[out] value The value which whether allow to notification
 * @return #NOTIFICATION_ERROR_NONE on success,
 *         otherwise any other value on failure
 * @retval #NOTIFICATION_ERROR_NONE			Success
 * @retval #NOTIFICATION_ERROR_INVALID_PARAMETER	Invalid parameter
 * @par sample code:
 * @code
#include <notification_setting_internal.h>
...
{
	int noti_err = 0;
	bool value;
	notification_setting_h setting = NULL;

	...

	noti_err = notification_setting_get_allow_to_notify(setting, &value);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		return;
	}

	notification_setting_free_notification(setting);

	return 0;

}
 * @endcode
 */
int notification_setting_get_allow_to_notify(notification_setting_h setting, bool *value);

/**
 * @internal
 * @brief Sets value which whether allow notification from individual applications.
 * @details After notification_setting_update_setting() call, the allow_to_notify is updated.
 * @since_tizen @if wearable 2.3.1 @elseif mobile 2.3 @endif
 * @param[in] setting The notification setting handle
 * @param[in] value The value whether allow to notification
 * @return #NOTIFICATION_ERROR_NONE on success,
 *         otherwise any other value on failure
 * @retval #NOTIFICATION_ERROR_NONE         Success
 * @retval #NOTIFICATION_ERROR_INVALID_PARAMETER Invalid parameter
 * @par sample code:
 * @code
#include <notification_setting_internal.h>
...
{
	int noti_err = 0;
	bool value;
	notification_setting_h setting = NULL;

	...

	value = true;	// or false

	noti_err = notification_setting_set_allow_to_notify(setting, value);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		return;
	}

	notification_setting_free_notification(setting);

	return 0;

}
 * @endcode
 */
int notification_setting_set_allow_to_notify(notification_setting_h setting, bool value);

/**
 * @internal
 * @brief Gets value which whether do not disturb notification from notification setting handle.
 * @since_tizen @if wearable 2.3.1 @elseif mobile 2.3 @endif
 * @param[in] setting The notification setting handle
 * @param[out] value The value which whether do not disturb notification or not
 * @return #NOTIFICATION_ERROR_NONE on success,
 *         otherwise any other value on failure
 * @retval #NOTIFICATION_ERROR_NONE         Success
 * @retval #NOTIFICATION_ERROR_INVALID_PARAMETER Invalid parameter
 * @par sample code:
 * @code
#include <notification_setting_internal.h>
...
{
	int noti_err = 0;
	bool value;
	notification_setting_h setting = NULL;

	...

	noti_err = notification_setting_get_do_not_disturb_except(setting, &value);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		return;
	}

	notification_setting_free_notification(setting);

	return 0;

}
 * @endcode
 */
int notification_setting_get_do_not_disturb_except(notification_setting_h setting, bool *value);

/**
 * @internal
 * @brief Sets value which whether do not disturb notification or not.
 * @details After notification_setting_update_setting() call, the do_not_disturb value is updated.
 * @since_tizen @if wearable 2.3.1 @elseif mobile 2.3 @endif
 * @param[in] setting The notification setting handle
 * @param[in] value The value which do not disturb notification or not
 * @return #NOTIFICATION_ERROR_NONE on success,
 *         otherwise any other value on failure
 * @retval #NOTIFICATION_ERROR_NONE         Success
 * @retval #NOTIFICATION_ERROR_INVALID_PARAMETER Invalid parameter
 * @par sample code:
 * @code
#include <notification_setting_internal.h>
...
{
	int noti_err = 0;
	bool value;
	notification_setting_h setting = NULL;

	...

	value = true;	// or false

	noti_err = notification_setting_set_do_not_disturb_except(setting, value);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		return;
	}

	notification_setting_free_notification(setting);

	return 0;

}
 * @endcode
 */
int notification_setting_set_do_not_disturb_except(notification_setting_h setting, bool value);

/**
 * @internal
 * @brief Gets value whether Pop up notification is allowed or not.
 * @since_tizen 3.0
 * @param[in] setting The notification setting handle
 * @param[in] value The value
 * @return #NOTIFICATION_ERROR_NONE on success,
 *         otherwise any other value on failure
 * @retval #NOTIFICATION_ERROR_NONE         Success
 * @retval #NOTIFICATION_ERROR_INVALID_PARAMETER Invalid parameter
 * @par sample code:
 * @code
#include <notification_setting_internal.h>
...
{
	int noti_err = 0;
	bool value;
	notification_setting_h setting = NULL;

	...

	value = true;	// or false

	noti_err = notification_setting_get_pop_up_notification(setting, value);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		return;
	}

	notification_setting_free_notification(setting);

	return 0;

}
 * @endcode
 */
int notification_setting_get_pop_up_notification(notification_setting_h setting, bool *value);

/**
 * @internal
 * @brief Sets value which Pop up notification allow or block.
 * @details After notification_setting_update_setting() call, the pop_up_notification value is updated.
 * @since_tizen 3.0
 * @param[in] setting The notification setting handle
 * @param[in] value The value
 * @return #NOTIFICATION_ERROR_NONE on success,
 *         otherwise any other value on failure
 * @retval #NOTIFICATION_ERROR_NONE         Success
 * @retval #NOTIFICATION_ERROR_INVALID_PARAMETER Invalid parameter
 * @par sample code:
 * @code
#include <notification_setting_internal.h>
...
{
	int noti_err = 0;
	bool value;
	notification_setting_h setting = NULL;

	...

	value = true;	// or false

	noti_err = notification_setting_get_pop_up_notification(setting, value);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		return;
	}

	notification_setting_free_notification(setting);

	return 0;

}
 * @endcode
 */
int notification_setting_set_pop_up_notification(notification_setting_h setting, bool value);

/**
 * @internal
 * @brief Gets displaying level that notification's information on lock screen from individual application.
 * @since_tizen 3.0
 * @param[in] setting The notification setting handle
 * @param[out] level The displaying level of notification's information on lock screen
 * @return #NOTIFICATION_ERROR_NONE on success,
 *         otherwise any other value on failure
 * @retval #NOTIFICATION_ERROR_NONE         Success
 * @retval #NOTIFICATION_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #lock_screen_content_level_e
 * @par sample code:
 * @code
#include <notification_setting_internal.h>
...
{
	int noti_err = 0;
	notification_setting_h setting = NULL;
	lock_screen_content_level_e level;

	...

	noti_err = notification_setting_get_lock_screen_content(setting, &level);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		return;
	}

	notification_setting_free_notification(setting);

	return 0;

}
 * @endcode
 */
int notification_setting_get_lock_screen_content(notification_setting_h setting, lock_screen_content_level_e *level);

/**
 * @internal
 * @brief Sets displaying level that notification's information on lock screen from individual application.
 * @details After notification_setting_update_setting() call, the lock_screen_content_level value is updated.
 * @since_tizen 3.0
 * @param[in] setting The notification setting handle
 * @param[out] level The displaying level of notification's information on lock screen
 * @return #NOTIFICATION_ERROR_NONE on success,
 *         otherwise any other value on failure
 * @retval #NOTIFICATION_ERROR_NONE         Success
 * @retval #NOTIFICATION_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #lock_screen_content_level_e
 * @par sample code:
 * @code
#include <notification_setting_internal.h>
...
{
	int noti_err = 0;
	notification_setting_h setting = NULL;
	lock_screen_content_level_e level;

	...

	level = SHOW_ALL_CONTENT;	// or HIDE_SENSITIVE_CONTENT or DO_NOT_SHOW_NOTIFICATIONS;

	noti_err = notification_setting_get_lock_screen_content(setting, level);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		return;
	}

	notification_setting_free_notification(setting);

	return 0;

}
 * @endcode
 */
int notification_setting_set_lock_screen_content(notification_setting_h setting, lock_screen_content_level_e level);

/**
 * @internal
 * @brief Gets The value that determines whether the app is disabled.
 * @since_tizen 3.0
 * @privlevel public
 * @privilege %http://tizen.org/privilege/notification
 * @param[in] setting The notification system setting handle
 * @param[out] value The value that determines whether the app is disabled
 * @return #NOTIFICATION_ERROR_NONE on success,
 *         otherwise any other value on failure
 * @retval #NOTIFICATION_ERROR_NONE         Success
 * @retval #NOTIFICATION_ERROR_INVALID_PARAMETER Invalid parameter
 * @par sample code:
 * @code
#include <notification_setting_internal.h>
{
	int err = 0;
	bool value = true;
	notification_system_setting_h setting = NULL;

	err = notification_setting_get_setting(&setting);
	if (err != NOTIFICATION_ERROR_NONE)
		return;

	err = notification_setting_get_app_disabled(setting, &value);
	if (err != NOTIFICATION_ERROR_NONE)
		return;

	return 0;
}
 * @endcode
 */
int notification_setting_get_app_disabled(notification_setting_h setting, bool *value);

/**
 * @internal
 * @brief Gets the notification system setting handle.
 * @since_tizen @if wearable 2.3.1 @elseif mobile 2.3 @endif
 * @privlevel public
 * @privilege %http://tizen.org/privilege/notification
 * @param[in] system_setting The notification system setting handle
 * @return #NOTIFICATION_ERROR_NONE on success,
 *         otherwise any other value on failure
 * @retval #NOTIFICATION_ERROR_NONE         Success
 * @retval #NOTIFICATION_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #NOTIFICATION_ERROR_OUT_OF_MEMORY Out of memory
 * @retval #NOTIFICATION_ERROR_IO_ERROR I/O error
 * @retval #NOTIFICATION_ERROR_SERVICE_NOT_READY No response from notification service
 * @retval #NOTIFICATION_ERROR_PERMISSION_DENIED Permission denied
 * @par sample code:
 * @code
#include <notification_setting_internal.h>
...
{
	int noti_err = 0;
	notification_system_setting_h setting = NULL;

	...

	noti_err = notification_system_setting_load_system_setting(&setting);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		return;
	}

	notification_system_setting_free_system_setting(setting);

	return 0;

}
 * @endcode
 */
int notification_system_setting_load_system_setting(notification_system_setting_h *system_setting);
int notification_system_setting_load_system_setting_for_uid(notification_system_setting_h *system_setting, uid_t uid);

/**
 * @internal
 * @brief Updates the notification system handle.
 * @since_tizen @if wearable 2.3.1 @elseif mobile 2.3 @endif
 * @privlevel public
 * @privilege %http://tizen.org/privilege/notification
 * @param[in] system_setting The notification system setting handle
 * @return #NOTIFICATION_ERROR_NONE on success,
 *         otherwise any other value on failure
 * @retval #NOTIFICATION_ERROR_NONE         Success
 * @retval #NOTIFICATION_ERROR_INVALID_PARAMETER Invalid parameter
 * @par sample code:
 * @code
#include <notification_setting_internal.h>
...
{
	int noti_err = 0;
	notification_system_setting_h setting = NULL;

	...

	noti_err = notification_system_setting_update_system_setting(setting);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		return;
	}

	notification_system_setting_free_system_setting(setting);

	return 0;

}
 * @endcode
 */
int notification_system_setting_update_system_setting(notification_system_setting_h system_setting);
int notification_system_setting_update_system_setting_for_uid(notification_system_setting_h system_setting, uid_t uid);

/**
 * @internal
 * @brief Frees the internal structure data of a notification system setting handle.
 * @since_tizen @if wearable 2.3.1 @elseif mobile 2.3 @endif
 * @param[in] system_setting The notification system setting handle
 * @return #NOTIFICATION_ERROR_NONE on success,
 *         otherwise any other value on failure
 * @retval #NOTIFICATION_ERROR_NONE         Success
 * @retval #NOTIFICATION_ERROR_INVALID_PARAMETER Invalid parameter
 * @par sample code:
 * @code
#include <notification_setting_internal.h>
...
{
	int noti_err = 0;
	notification_system_setting_h setting = NULL;

	...

	noti_err = notification_system_setting_free_system_setting(setting);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		return;
	}

	return 0;

}
 * @endcode
 */
int notification_system_setting_free_system_setting(notification_system_setting_h system_setting);

/**
 * @internal
 * @brief Gets value which whether do not disturb notification from notification system setting handle.
 * @since_tizen @if wearable 2.3.1 @elseif mobile 2.3 @endif
 * @param[in] system_setting The notification system setting handle
 * @param[out] value The value which whether do not disturb notification or not
 * @return #NOTIFICATION_ERROR_NONE on success,
 *         otherwise any other value on failure
 * @retval #NOTIFICATION_ERROR_NONE         Success
 * @retval #NOTIFICATION_ERROR_INVALID_PARAMETER Invalid parameter
 * @par sample code:
 * @code
#include <notification_setting_internal.h>
...
{
	int noti_err = 0;
	bool value;
	notification_system_setting_h setting = NULL;

	...

	noti_err = notification_system_setting_get_do_not_disturb_except(setting, &value);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		return;
	}

	notification_system_setting_free_system_setting(setting);

	return 0;

}
 * @endcode
 */
int notification_system_setting_get_do_not_disturb(notification_system_setting_h system_setting, bool *value);

/**
 * @internal
 * @brief Sets value which whether do not disturb notification or not.
 * @details After notification_system_setting_update_system_setting() call, the do_not_disturb value is not updated.
 * @since_tizen @if wearable 2.3.1 @elseif mobile 2.3 @endif
 * @param[in] system_setting The notification system setting handle
 * @param[in] value The value which do not disturb notification or not
 * @return #NOTIFICATION_ERROR_NONE on success,
 *         otherwise any other value on failure
 * @retval #NOTIFICATION_ERROR_NONE         Success
 * @retval #NOTIFICATION_ERROR_INVALID_PARAMETER Invalid parameter
 * @par sample code:
 * @code
#include <notification_setting_internal.h>
...
{
	int noti_err = 0;
	bool value;
	notification_system_setting_h setting = NULL;

	...

	value = true;	// or false

	noti_err = notification_setting_set_do_not_disturb_except(setting, value);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		return;
	}

	notification_system_setting_free_system_setting(setting);

	return 0;

}
 * @endcode
 */
int notification_system_setting_set_do_not_disturb(notification_system_setting_h system_setting, bool value);

/**
 * @internal
 * @brief Gets value which whether information on the notification view is visible.
 * @since_tizen @if wearable 2.3.1 @elseif mobile 2.3 @endif
 * @param[in] system_setting The notification system setting handle
 * @param[out] value The visibility_class value
 * @return #NOTIFICATION_ERROR_NONE on success,
 *         otherwise any other value on failure
 * @retval #NOTIFICATION_ERROR_NONE         Success
 * @retval #NOTIFICATION_ERROR_INVALID_PARAMETER Invalid parameter
 * @par sample code:
 * @code
#include <notification_setting_internal.h>
...
{
	int noti_err = 0;
	bool value;
	notification_system_setting_h setting = NULL;

	...

	noti_err = notification_system_setting_get_visibility_class(setting, &value);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		return;
	}

	notification_system_setting_free_system_setting(setting);

	return 0;

}
 * @endcode
 */
int notification_system_setting_get_visibility_class(notification_system_setting_h system_setting, int *value);

/**
 * @internal
 * @brief Sets value which whether information on the notification view is visible.
 * @details After notification_system_setting_update_system_setting() call, the visibility_class value is not updated.
 * @since_tizen @if wearable 2.3.1 @elseif mobile 2.3 @endif
 * @param[in] system_setting The notification setting handle
 * @param[in] value The visibility_class value
 * @return #NOTIFICATION_ERROR_NONE on success,
 *         otherwise any other value on failure
 * @retval #NOTIFICATION_ERROR_NONE         Success
 * @retval #NOTIFICATION_ERROR_INVALID_PARAMETER Invalid parameter
 * @par sample code:
 * @code
#include <notification_setting_internal.h>
...
{
	int noti_err = 0;
	bool value;
	notification_system_setting_h setting = NULL;

	...

	value = true;	// or false

	noti_err = notification_setting_set_visibility_class(setting, value);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		return;
	}

	notification_system_setting_free_system_setting(setting);

	return 0;

}
 * @endcode
 */
int notification_system_setting_set_visibility_class(notification_system_setting_h system_setting, int value);

/**
 * @internal
 * @brief Gets value which whether 'Do not disturb' mode is enable or not.
 * @since_tizen 3.0
 * @param[in] system_setting The notification system setting handle
 * @param[out] enabled The dnd_schedule_enabled value
 * @return #NOTIFICATION_ERROR_NONE on success,
 *         otherwise any other value on failure
 * @retval #NOTIFICATION_ERROR_NONE         Success
 * @retval #NOTIFICATION_ERROR_INVALID_PARAMETER Invalid parameter
 * @par sample code:
 * @code
#include <notification_setting_internal.h>
...
{
	int noti_err = 0;
	bool value;
	notification_system_setting_h setting = NULL;

	...

	noti_err = notification_system_setting_dnd_schedule_get_enabled(setting, &value);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		return;
	}

	notification_system_setting_free_system_setting(setting);

	return 0;

}
 * @endcode
 */
int notification_system_setting_dnd_schedule_get_enabled(notification_system_setting_h system_setting, bool *enabled);

/**
 * @internal
 * @brief Sets value which whether 'Do not disturb' mode is enable or not.
 * @details After notification_system_setting_update_system_setting() call, the 'Do not disturb' mode is not updated.
 * @since_tizen 3.0
 * @param[in] system_setting The notification system setting handle
 * @param[in] enabled The dnd_schedule_enabled value
 * @return #NOTIFICATION_ERROR_NONE on success,
 *         otherwise any other value on failure
 * @retval #NOTIFICATION_ERROR_NONE         Success
 * @retval #NOTIFICATION_ERROR_INVALID_PARAMETER Invalid parameter
 * @par sample code:
 * @code
#include <notification_setting_internal.h>
...
{
	int noti_err = 0;
	bool value;
	notification_system_setting_h setting = NULL;

	...

	value = true;	// or false

	noti_err = notification_system_setting_dnd_schedule_set_enabled(setting, &value);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		return;
	}

	notification_system_setting_free_system_setting(setting);

	return 0;

}
 * @endcode
 */
int notification_system_setting_dnd_schedule_set_enabled(notification_system_setting_h system_setting, bool enabled);

/**
 * @internal
 * @brief Gets days of the week that 'Do not disturb' mode is enable.
 * @since_tizen 3.0
 * @param[in] system_setting The notification system setting handle
 * @param[out] day The days of the week that enable 'Do not disturb' mode
 * @return #NOTIFICATION_ERROR_NONE on success,
 *         otherwise any other value on failure
 * @retval #NOTIFICATION_ERROR_NONE         Success
 * @retval #NOTIFICATION_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #dnd_schedule_week_flag_e
 * @par sample code:
 * @code
#include <notification_setting_internal.h>
...
{
	int noti_err = 0;
	int day;
	notification_system_setting_h setting = NULL;

	...

	noti_err = notification_system_setting_dnd_schedule_get_day(setting, &day);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		return;
	}

	notification_system_setting_free_system_setting(setting);

	return 0;

}
 * @endcode
 */
int notification_system_setting_dnd_schedule_get_day(notification_system_setting_h system_setting, int *day);

/**
 * @internal
 * @brief Sets days of the week that 'Do not disturb' mode is enable.
 * @details After notification_system_setting_update_system_setting() call, the days not updated.
 * @since_tizen 3.0
 * @param[in] system_setting The notification system setting handle
 * @param[in] day The days of the week that enable 'Do not disturb' mode
 * @return #NOTIFICATION_ERROR_NONE on success,
 *         otherwise any other value on failure
 * @retval #NOTIFICATION_ERROR_NONE         Success
 * @retval #NOTIFICATION_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #dnd_schedule_week_flag_e
 * @par sample code:
 * @code
#include <notification_setting_internal.h>
...
{
	int noti_err = 0;
	int day;
	notification_system_setting_h setting = NULL;

	...

	day = DND_SCHEDULE_WEEK_FLAG_SUNDAY;

	noti_err = notification_system_setting_dnd_schedule_set_day(setting, day);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		return;
	}

	notification_system_setting_free_system_setting(setting);

	return 0;

}
 * @endcode
 */
int notification_system_setting_dnd_schedule_set_day(notification_system_setting_h system_setting, int day);

/**
 * @internal
 * @brief Gets time that 'Do not disturb' mode is started.
 * @since_tizen 3.0
 * @param[in] system_setting The notification system setting handle
 * @param[out] hour The hour that 'Do not disturb' mode is started
 * @param[out] min The min that 'Do not disturb' mode is started
 * @return #NOTIFICATION_ERROR_NONE on success,
 *         otherwise any other value on failure
 * @retval #NOTIFICATION_ERROR_NONE         Success
 * @retval #NOTIFICATION_ERROR_INVALID_PARAMETER Invalid parameter
 * @par sample code:
 * @code
#include <notification_setting_internal.h>
...
{
	int noti_err = 0;
	int hour;
	int min;
	notification_system_setting_h setting = NULL;

	...

	noti_err = notification_system_setting_dnd_schedule_get_start_time(setting, &hour, &min);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		return;
	}

	notification_system_setting_free_system_setting(setting);

	return 0;

}
 * @endcode
 */
int notification_system_setting_dnd_schedule_get_start_time(notification_system_setting_h system_setting, int *hour, int *min);

/**
 * @internal
 * @brief Sets time that 'Do not disturb' mode is started.
 * @since_tizen 3.0
 * @param[in] system_setting The notification system setting handle
 * @param[in] hour The hour that 'Do not disturb' mode is startd
 * @param[in] min The min that 'Do not disturb' mode is started
 * @return #NOTIFICATION_ERROR_NONE on success,
 *         otherwise any other value on failure
 * @retval #NOTIFICATION_ERROR_NONE         Success
 * @retval #NOTIFICATION_ERROR_INVALID_PARAMETER Invalid parameter
 * @par sample code:
 * @code
#include <notification_setting_internal.h>
...
{
	int noti_err = 0;
	int hour;
	int min;
	notification_setting_h setting = NULL;

	...

	hour = START_HOUR;	// 0 ~ 23
	min = START_MIN		// 0 ~ 59

	noti_err = notification_system_setting_dnd_schedule_set_start_time(setting, hour, min);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		return;
	}

	notification_system_setting_free_system_setting(setting);

	return 0;

}
 * @endcode
 */
int notification_system_setting_dnd_schedule_set_start_time(notification_system_setting_h system_setting, int hour, int min);

/**
 * @internal
 * @brief Gets time that 'Do not disturb' mode is ended.
 * @since_tizen 3.0
 * @param[in] system_setting The notification system setting handle
 * @param[out] hour The hour that 'Do not disturb' mode is ended
 * @param[out] min The min that 'Do not disturb' mode is ended
 * @return #NOTIFICATION_ERROR_NONE on success,
 *         otherwise any other value on failure
 * @retval #NOTIFICATION_ERROR_NONE         Success
 * @retval #NOTIFICATION_ERROR_INVALID_PARAMETER Invalid parameter
 * @par sample code:
 * @code
#include <notification_setting_internal.h>
...
{
	int noti_err = 0;
	int hour;
	int min;
	notification_system_setting_h setting = NULL;

	...

	noti_err = notification_system_setting_dnd_schedule_get_end_time(setting, &hour, &min);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		return;
	}

	notification_system_setting_free_system_setting(setting);

	return 0;

}
 * @endcode
 */
int notification_system_setting_dnd_schedule_get_end_time(notification_system_setting_h system_setting, int *hour, int *min);

/**
 * @internal
 * @brief Sets time that 'Do not disturb' mode is ended.
 * @since_tizen 3.0
 * @param[in] system_setting The notification system setting handle
 * @param[in] hour The hour that 'Do not disturb' mode is ended
 * @param[in] min The min that 'Do not disturb' mode is ended
 * @return #NOTIFICATION_ERROR_NONE on success,
 *         otherwise any other value on failure
 * @retval #NOTIFICATION_ERROR_NONE         Success
 * @retval #NOTIFICATION_ERROR_INVALID_PARAMETER Invalid parameter
 * @par sample code:
 * @code
#include <notification_setting_internal.h>
...
{
	int noti_err = 0;
	int hour;
	int min;
	notification_system_setting_h setting = NULL;

	...

	hour = START_HOUR;	// 0 ~ 23
	min = START_MIN		// 0 ~ 59

	noti_err = notification_system_setting_dnd_schedule_set_end_time(setting, hour, min);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		return;
	}

	notification_system_setting_free_system_setting(setting);

	return 0;

}
 * @endcode
 */
int notification_system_setting_dnd_schedule_set_end_time(notification_system_setting_h system_setting, int hour, int min);

/**
 * @internal
 * @brief Gets displaying level that notification's information on lock screen.
 * @since_tizen 3.0
 * @param[in] system_setting The notification system setting handle
 * @param[out] level The displaying level of notification's information on lock screen
 * @return #NOTIFICATION_ERROR_NONE on success,
 *         otherwise any other value on failure
 * @retval #NOTIFICATION_ERROR_NONE         Success
 * @retval #NOTIFICATION_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #lock_screen_content_level_e
 * @par sample code:
 * @code
#include <notification_setting_internal.h>
...
{
	int noti_err = 0;
	notification_setting_h setting = NULL;
	lock_screen_content_level_e level;

	...

	noti_err = notification_system_setting_get_lock_screen_content(setting, &level);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		return;
	}

	notification_system_setting_free_system_setting(setting);

	return 0;

}
 * @endcode
 */
int notification_system_setting_get_lock_screen_content(notification_system_setting_h system_setting, lock_screen_content_level_e *level);

/*
 * @internal
 * @brief Sets displaying level that notification's information on lock screen.
 * @since_tizen 3.0
 * @param[in] system_setting The notification system setting handle
 * @param[in] level The displaying level of notification's information on lock screen
 * @return #NOTIFICATION_ERROR_NONE on success,
 *         otherwise any other value on failure
 * @retval #NOTIFICATION_ERROR_NONE         Success
 * @retval #NOTIFICATION_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #lock_screen_content_level_e
 * @par sample code:
 * @code
#include <notification_setting_internal.h>
...
{
	int noti_err = 0;
	notification_system_setting_h setting = NULL;
	lock_screen_content_level_e level;

	...

	level = SHOW_ALL_CONTENT;	// or HIDE_SENSITIVE_CONTENT or DO_NOT_SHOW_NOTIFICATIONS;

	noti_err = notification_system_setting_set_lock_screen_content(setting, &level);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		return;
	}

	notification_system_setting_free_system_setting(setting);

	return 0;

}
 *
 */
int notification_system_setting_set_lock_screen_content(notification_system_setting_h system_setting, lock_screen_content_level_e level);

/**
 * @internal
 * @brief Gets a value of the do_not_disturb allow exceptions.
 * @since_tizen 3.0
 * @privlevel public
 * @privilege %http://tizen.org/privilege/notification
 * @param[in] system_setting The notification system setting handle
 * @param[in] type The exceptional item of do_not_distrub
 * @param[out] value The value of the exceptional item
 * @return #NOTIFICATION_ERROR_NONE on success,
 *         otherwise any other value on failure
 * @retval #NOTIFICATION_ERROR_NONE Success
 * @retval #NOTIFICATION_ERROR_INVALID_PARAMETER Invalid parameter
 * @par sample code:
 * @code
#include <notification_setting_internal.h>
...
{
	int noti_err = 0;
	int value;
	notification_system_setting_h setting = NULL;
	...
	noti_err = notification_system_setting_load_system_setting(&setting);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		return;
	}

	noti_err = notification_system_setting_get_dnd_allow_exceptions(setting, ALLOWED_CALLS, &value);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		return;
	}
	...
	notification_system_setting_free_system_setting(setting);

	return 0;

}
 * @endcode
 */
int notification_system_setting_get_dnd_allow_exceptions(notification_system_setting_h system_setting, dnd_allow_exception_type_e type, int *value);

/**
 * @internal
 * @brief Sets a value of the do_not_disturb allow exceptions.
 * @since_tizen 3.0
 * @privlevel public
 * @privilege %http://tizen.org/privilege/notification
 * @param[in] system_setting The notification system setting handle
 * @param[in] type The exceptional item of do_not_distrub
 * @param[in] value The value of the exceptional item
 * @return #NOTIFICATION_ERROR_NONE on success,
 *         otherwise any other value on failure
 * @retval #NOTIFICATION_ERROR_NONE Success
 * @retval #NOTIFICATION_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #NOTIFICATION_ERROR_OUT_OF_MEMORY out of memory
 * @par sample code:
 * @code
#include <notification_setting_internal.h>
...
{
	int noti_err = 0;
	int value = 0;
	notification_system_setting_h setting = NULL;
	...
	noti_err = notification_system_setting_load_system_setting(&setting);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		return;
	}

	noti_err = notification_system_setting_set_dnd_allow_exceptions(setting, ALLOWED_CALLS, value);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		return;
	}
	...
	notification_system_setting_free_system_setting(setting);

	return 0;

}
 * @endcode
 */
int notification_system_setting_set_dnd_allow_exceptions(notification_system_setting_h system_setting, dnd_allow_exception_type_e type, int value);

/**
 * @internal
 * @brief Registers a callback for turn on/off 'Do not disturb' mode by user_data
 *        or 'Do not disturb' mode setting schedule is start or end.
 * @since_tizen 3.0
 * @privlevel public
 * @privilege %http://tizen.org/privilege/notification
 * @param[in] callback The callback function
 * @param[in] user_data The user data
 * @return #NOTIFICATION_ERROR_NONE on success,
 *         otherwise any other value on failure
 * @retval #NOTIFICATION_ERROR_NONE Success
 * @retval #NOTIFICATION_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #NOTIFICATION_ERROR_IO_ERROR I/O Error
 * @retval #NOTIFICATION_ERROR_OUT_OF_MEMORY out of memory
 * @par sample code:
 * @code
#include <notification_setting_internal.h>
...

static void changed_cb(void *user_data, int do_not_disturb)
{
	...
}

...
{
	int noti_err = 0;

	...

	noti_err = notification_register_system_setting_dnd_changed_cb(changed_cb, NULL);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		return;
	}

	return 0;

}
 * @endcode
 */
int notification_register_system_setting_dnd_changed_cb(dnd_changed_cb callback, void *user_data);
int notification_register_system_setting_dnd_changed_cb_for_uid(dnd_changed_cb callback, void *user_data, uid_t uid);

/**
 * @internal
 * @brief Unregisters a callback for turn on/off 'Do not disturb' mode by user_data
 *        or 'Do not disturb' mode setting schedule is start or end.
 * @since_tizen 3.0
 * @privlevel public
 * @privilege %http://tizen.org/privilege/notification
 * @param[in] callback The callback function
 * @return #NOTIFICATION_ERROR_NONE on success,
 *         otherwise any other value on failure
 * @retval #NOTIFICATION_ERROR_NONE Success
 * @retval #NOTIFICATION_ERROR_INVALID_PARAMETER Invalid parameter
 * @par sample code:
 * @code
#include <notification_setting_internal.h>
...

static void changed_cb(void *user_data, int do_not_disturb)
{
	...
}

...
{
	int noti_err = 0;

	...

	noti_err = notification_unregister_system_setting_dnd_changed_cb(changed_cb);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		return;
	}

	return 0;
}
 * @endcode
 */
int notification_unregister_system_setting_dnd_changed_cb(dnd_changed_cb callback);
int notification_unregister_system_setting_dnd_changed_cb_for_uid(dnd_changed_cb callback, uid_t uid);

/**
 * @internal
 * @brief Updates the notification setting if the pacakge is installed or updated.
 * @since_tizen 3.0
 * @param[in] package_name
 * @param[in] uid User id
 * @return #NOTIFICATION_ERROR_NONE on success,
 *         otherwise any other value on failure
 * @retval #NOTIFICATION_ERROR_NONE Success
 * @retval #NOTIFICATION_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #NOTIFICATION_ERROR_FROM_DB Error from DB query
 * @par sample code:
 * @code
#include <notification_setting.h>
...
{
	int ret;

	ret = notification_setting_insert_package_for_uid(pkg_name, uid);
	if (ret = NOTIFICATION_ERROR_NONE)
		return;
}
 * @endcode
 */
int notification_setting_insert_package_for_uid(const char *package_name, uid_t uid);

/**
 * @internal
 * @brief Deletes the notification setting if the pacakge is uninstalled.
 * @since_tizen 3.0
 * @param[in] package_name
 * @param[in] uid User id
 * @return #NOTIFICATION_ERROR_NONE on success,
 *         otherwise any other value on failure
 * @retval #NOTIFICATION_ERROR_NONE Success
 * @retval #NOTIFICATION_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #NOTIFICATION_ERROR_FROM_DB Error from DB query
 * @par sample code:
 * @code
#include <notification_setting.h>
...
{
	int ret;

	ret = notification_setting_delete_package_for_uid(pkg_name, uid);
	if (ret = NOTIFICATION_ERROR_NONE)
		return;
}
 * @endcode
 */
int notification_setting_delete_package_for_uid(const char *package_name, uid_t uid);

/**
 * @internal
 * @brief Updates the notification setting.
 * @since_tizen 3.0
 * @param[in] setting Notification setting handle
 * @param[in] uid User id
 * @return #NOTIFICATION_ERROR_NONE on success,
 *         otherwise any other value on failure
 * @retval #NOTIFICATION_ERROR_NONE Success
 * @retval #NOTIFICATION_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #NOTIFICATION_ERROR_OUT_OF_MEMORY Out of memory
 * @retval #NOTIFICATION_ERROR_IO_ERROR I/O error
 * @retval #NOTIFICATION_ERROR_SERVICE_NOT_READY No response from notification service
 * @retval #NOTIFICATION_ERROR_PERMISSION_DENIED Permission denied
 * @par sample code:
 * @code
#include <notification_setting.h>
...
{
	int ret;
	notification_setting_h setting;

	// Get setting data

	// Update setting data

	ret = notification_setting_update_setting_for_uid(setting, uid);
	if (ret = NOTIFICATION_ERROR_NONE)
		return;
}
 * @endcode
 */
int notification_setting_update_setting_for_uid(notification_setting_h setting, uid_t uid);

/* OLD IMPLEMENTATION */
int notification_setting_property_set(const char *pkgname, const char *property, const char *value) NOTIFICATION_DEPRECATED_API;
int notification_setting_property_get(const char *pkgname, const char *property, char **value) NOTIFICATION_DEPRECATED_API;

#ifdef __cplusplus
}
#endif
#endif /* __NOTIFICATION_SETTING_INTERNAL_H__ */

