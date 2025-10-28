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

#ifndef __NOTIFICATION_SETTING_H__
#define __NOTIFICATION_SETTING_H__

#include <stdbool.h>
#include <sys/types.h>

#include "notification.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct notification_setting *notification_setting_h;

/**
 * @internal
 * @brief Gets setting handle of current process.
 * @since_tizen 2.4
 * @param[out] setting The setting handle
 * @return #NOTIFICATION_ERROR_NONE on success,
 *         otherwise any other value on failure
 * @retval #NOTIFICATION_ERROR_NONE Success
 * @retval #NOTIFICATION_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #NOTIFICATION_ERROR_OUT_OF_MEMORY Out of memory
 * @retval #NOTIFICATION_ERROR_NOT_EXIST_ID Already exist private ID
 * @retval #NOTIFICATION_ERROR_IO_ERROR I/O error
 * @retval #NOTIFICATION_ERROR_PERMISSION_DENIED Permission denied
 * @par sample code:
 * @code
#include <notification_setting.h>
...
{
	int noti_err = 0;
	notification_setting_h setting = NULL;

	noti_err = notification_setting_get_setting(&setting);
	if (noti_err != NOTIFICATION_ERROR_NONE)
		return;
}
 * @endcode
 */
int notification_setting_get_setting(notification_setting_h *setting);

/**
 * @internal
 * @brief Gets value which whether information on the notification view is visible.
 * @since_tizen @if wearable 2.3.1 @elseif mobile 2.3 @endif
 * @param[in] setting	The notification setting handle
 * @param[out] value	The visibility_class value
 * @return #NOTIFICATION_ERROR_NONE on success,
 *         otherwise any other value on failure
 * @retval #NOTIFICATION_ERROR_NONE         Success
 * @retval #NOTIFICATION_ERROR_INVALID_PARAMETER Invalid parameter
 * @par sample code:
 * @code
#include <notification_setting.h>
...
{
	int noti_err = 0;
	bool value;
	notification_setting_h setting = NULL;

	...

	noti_err = notification_setting_get_visibility_class(setting, &value);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		return;
	}

	notification_setting_free_notification(setting);

	return 0;

}
 * @endcode
 */
int notification_setting_get_visibility_class(notification_setting_h setting, int *value);

/**
 * @internal
 * @brief Sets value which whether information on the notification view is visible.
 * @details After notification_setting_update_setting() call, the visibility_class value is not updated.
 * @since_tizen @if wearable 2.3.1 @elseif mobile 2.3 @endif
 * @param[in] setting	The notification setting handle
 * @param[in] value	The visibility_class value
 * @return #NOTIFICATION_ERROR_NONE on success,
 *         otherwise any other value on failure
 * @retval #NOTIFICATION_ERROR_NONE         Success
 * @retval #NOTIFICATION_ERROR_INVALID_PARAMETER Invalid parameter
 * @par sample code:
 * @code
#include <notification_setting.h>
...
{
	int noti_err = 0;
	bool value;
	notification_setting_h setting = NULL;

	...

	value = true;	// or false

	noti_err = notification_setting_set_visibility_class(setting, value);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		return;
	}

	notification_setting_free_notification(setting);

	return 0;

}
 * @endcode
 */
int notification_setting_set_visibility_class(notification_setting_h setting, int value);

/**
 * @internal
 * @brief Updates the notification setting handle.
 * @since_tizen @if wearable 2.3.1 @elseif mobile 2.3 @endif
 * @privlevel public
 * @privilege %http://tizen.org/privilege/notification
 * @param[in] setting	The notification setting handle
 * @return #NOTIFICATION_ERROR_NONE on success,
 *         otherwise any other value on failure
 * @retval #NOTIFICATION_ERROR_NONE         Success
 * @retval #NOTIFICATION_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #NOTIFICATION_ERROR_IO_ERROR I/O error
 * @retval #NOTIFICATION_ERROR_SERVICE_NOT_READY No response from notification service
 * @retval #NOTIFICATION_ERROR_PERMISSION_DENIED Permission denied
 * @par sample code:
 * @code
#include <notification_setting.h>
...
{
	int noti_err = 0;
	bool value;
	notification_setting_h setting = NULL;

	...

	noti_err = notification_setting_update_setting(setting);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		return;
	}

	notification_setting_free_notification(setting);

	return 0;

}
 * @endcode
 */
int notification_setting_update_setting(notification_setting_h setting);

/**
 * @internal
 * @brief Frees the internal structure data of a notification setting handle.
 * @since_tizen @if wearable 2.3.1 @elseif mobile 2.3 @endif
 * @param[in] setting	The notification setting handle
 * @return #NOTIFICATION_ERROR_NONE on success,
 *         otherwise any other value on failure
 * @retval #NOTIFICATION_ERROR_NONE         Success
 * @retval #NOTIFICATION_ERROR_INVALID_PARAMETER Invalid parameter
 * @par sample code:
 * @code
#include <notification_setting.h>
...
{
	int noti_err = 0;
	notification_setting_h setting = NULL;

	...

	noti_err = notification_setting_free_notification(setting);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		return;
	}

	return 0;

}
 * @endcode
 */
int notification_setting_free_notification(notification_setting_h setting);

/**
 * @internal
 * @brief Refreshs the setting table of current user.
 * @since_tizen 2.4
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

	ret = notification_setting_refresh_setting_table(uid);
	if (ret = NOTIFICATION_ERROR_NONE)
		return;
}
 * @endcode
 */
int notification_setting_refresh_setting_table(uid_t uid);

/**
 * @internal
 * @brief Initializes the system setting table of the current user.
 * @since_tizen 2.4
 * @param[in] uid User id
 * @return #NOTIFICATION_ERROR_NONE on success,
 *         otherwise any other value on failure
 * @retval #NOTIFICATION_ERROR_NONE Success
 * @retval #NOTIFICATION_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #NOTIFICATION_ERROR_OUT_OF_MEMORY Out of memory
 * @retval #NOTIFICATION_ERROR_NOT_EXIST_ID Already exist private ID
 * @retval #NOTIFICATION_ERROR_IO_ERROR I/O error
 * @par sample code:
 * @code
#include <notification_setting.h>
...
{
	int ret;

	ret = notification_system_setting_init_system_setting_table(uid);
	if (ret != NOTIFICATION_ERROR_NONE)
		return;
}
 * @endcode
 */
int notification_system_setting_init_system_setting_table(uid_t uid);

#ifdef __cplusplus
}
#endif
#endif /* __NOTIFICATION_SETTING_H__ */

