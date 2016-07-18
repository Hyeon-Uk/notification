/*
 * Copyright (c) 2015 - 2016 Samsung Electronics Co., Ltd. All rights reserved.
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
#include "notification.h"
#include "notification_setting.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * The prototype of handler that 'Do not disturb' mode set schdule changed alarm.
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
} dnd_schedule_week_flag_e;

/**
* @brief Enumeration for lock screen content.
* @since_tizen 3.0
*/
typedef enum lock_screen_content_level {
	SHOW_ALL_CONTENT = 0,
	HIDE_SENSITIVE_CONTENT,
	DO_NOT_SHOW_NOTIFICATIONS,
} lock_screen_content_level_e;

/* Application setting */
struct notification_setting {
	char *package_name;
	bool  allow_to_notify;
	bool  do_not_disturb_except;
	int   visibility_class;
};

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
};

int notification_setting_get_setting_array(notification_setting_h *setting_array, int *count);
int notification_setting_get_setting_array_for_uid(notification_setting_h *setting_array, int *count, uid_t uid);

int notification_setting_get_setting_by_package_name(const char *package_name, notification_setting_h *setting);
int notification_setting_get_setting_by_package_name_for_uid(const char *package_name, notification_setting_h *setting, uid_t uid);

int notification_setting_get_package_name(notification_setting_h setting, char **value);
int notification_setting_set_package_name(notification_setting_h setting, char *value);

int notification_setting_get_allow_to_notify(notification_setting_h setting, bool *value);
int notification_setting_set_allow_to_notify(notification_setting_h setting, bool value);

int notification_setting_get_do_not_disturb_except(notification_setting_h setting, bool *value);
int notification_setting_set_do_not_disturb_except(notification_setting_h setting, bool value);

int notification_setting_insert_package_for_uid(const char *package_id, uid_t uid);
int notification_setting_delete_package_for_uid(const char *package_id, uid_t uid);

int notification_setting_update_setting_for_uid(notification_setting_h setting, uid_t uid);

/* System setting */
typedef struct notification_system_setting *notification_system_setting_h;

int notification_system_setting_load_system_setting(notification_system_setting_h *system_setting);
int notification_system_setting_load_system_setting_for_uid(notification_system_setting_h *system_setting, uid_t uid);

int notification_system_setting_update_system_setting(notification_system_setting_h system_setting);
int notification_system_setting_update_system_setting_for_uid(notification_system_setting_h system_setting, uid_t uid);

int notification_system_setting_free_system_setting(notification_system_setting_h system_setting);

int notification_system_setting_get_do_not_disturb(notification_system_setting_h system_setting, bool *value);
int notification_system_setting_set_do_not_disturb(notification_system_setting_h system_setting, bool value);

int notification_system_setting_get_visibility_class(notification_system_setting_h system_setting, int *value);
int notification_system_setting_set_visibility_class(notification_system_setting_h system_setting, int value);

int notification_system_setting_dnd_schedule_get_enabled(notification_system_setting_h system_setting, bool *enabled);
int notification_system_setting_dnd_schedule_set_enabled(notification_system_setting_h system_setting, bool enabled);

int notification_system_setting_dnd_schedule_get_day(notification_system_setting_h system_setting, int *day);
int notification_system_setting_dnd_schedule_set_day(notification_system_setting_h system_setting, int day);

int notification_system_setting_dnd_schedule_get_start_time(notification_system_setting_h system_setting, int *hour, int *min);
int notification_system_setting_dnd_schedule_set_start_time(notification_system_setting_h system_setting, int hour, int min);

int notification_system_setting_dnd_schedule_get_end_time(notification_system_setting_h system_setting, int *hour, int *min);
int notification_system_setting_dnd_schedule_set_end_time(notification_system_setting_h system_setting, int hour, int min);

int notification_system_setting_set_lock_screen_content(notification_system_setting_h system_setting, lock_screen_content_level_e level);
int notification_system_setting_get_lock_screen_content(notification_system_setting_h system_setting, lock_screen_content_level_e *level);



/*
 * @internal
 * @brief Registers a callback for 'Do not disturb' mode setting schedule is start or end.
 * @since_tizen 3.0
 * @param[in] callback 	The callback function
 * @param[in] user_data	The user data
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
	if(noti_err != NOTIFICATION_ERROR_NONE) {
		return;
	}

	return 0;

}
 * @endcode
 */
int notification_register_system_setting_dnd_changed_cb(dnd_changed_cb callback, void *user_data);
int notification_register_system_setting_dnd_changed_cb_for_uid(dnd_changed_cb callback, void *user_data, uid_t uid);

/*
 * @internal
 * @brief Unregisters a callback for 'Do not disturb' mode setting schedule is start or end.
 * @since_tizen 3.0
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
	if(noti_err != NOTIFICATION_ERROR_NONE) {
		return;
	}

	return 0;

}
 * @endcode
 */
int notification_unregister_system_setting_dnd_changed_cb(dnd_changed_cb callback);
int notification_unregister_system_setting_dnd_changed_cb_for_uid(dnd_changed_cb callback, uid_t uid);


/* OLD IMPLEMENTATION */
int notification_setting_property_set(const char *pkgname, const char *property, const char *value) NOTIFICATION_DEPRECATED_API;
int notification_setting_property_get(const char *pkgname, const char *property, char **value) NOTIFICATION_DEPRECATED_API;

#ifdef __cplusplus
}
#endif
#endif /* __NOTIFICATION_SETTING_INTERNAL_H__ */

