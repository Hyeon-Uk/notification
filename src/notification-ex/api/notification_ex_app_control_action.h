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

#ifndef __TIZEN_APPFW_NOTIFICATION_EX_APP_CONTROL_ACTION_H__
#define __TIZEN_APPFW_NOTIFICATION_EX_APP_CONTROL_ACTION_H__

#include <app_control.h>
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
 * @brief Creates the notification_ex action handle with app control handle.
 * @details The notification_ex can define the action with #noti_ex_action_h.
 *          If it is necessary to use app control handle for an action,
 *          the app control handle must be set in notification_ex action handle.
 *          @a extra can be used to pass user defined data
 *          and the extra data can be obtained from an action handle using #noti_ex_action_get_extra().
 * @since_tizen 5.5
 * @remarks @a handle must be released using noti_ex_action_destroy().
 *          @a app_control must be released after noti_ex_action_app_control_create() function is called.
 * @param[out] handle The notification_ex action handle
 * @param[in] app_control The app control handle
 * @param[in] extra The extra data
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #NOTI_EX_ERROR_OUT_OF_MEMORY out of memory
 * @see #noti_ex_action_h
 * @see noti_ex_action_destroy()
 * @see noti_ex_action_get_extra()
 * @par Sample code:
 * @code
#include <notification_ex.h>

{
	int ret;
	app_control_h app_control = NULL;
	noti_ex_action_h appcontrol_action = NULL;

	app_control_create(&app_control);
	app_control_set_app_id(app_control, "temp_appid");

	ret = noti_ex_action_app_control_create(&appcontrol_action, app_control, NULL);

	app_control_destroy(app_control);
}
 * @endcode
 */
int noti_ex_action_app_control_create(noti_ex_action_h *handle, app_control_h app_control, const char *extra) TIZEN_DEPRECATED_API;

/**
 * @deprecated Deprecated since 11.0.
 * @brief Sets the app control handle for notification_ex action.
 * @since_tizen 5.5
 * @param[in] handle The notification_ex action handle
 * @param[in] app_control The app control handle
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_action_h
 * @see noti_ex_action_app_control_create()
 * @par Sample code:
 * @code
#include <notification_ex.h>

{
	int ret;
	app_control_h app_control = NULL;

	app_control_create(&app_control);
	app_control_set_app_id(app_control, "new_appid");

	ret = noti_ex_action_app_control_set(appcontrol_action, app_control);
}
 * @endcode
 */
int noti_ex_action_app_control_set(noti_ex_action_h handle, app_control_h app_control) TIZEN_DEPRECATED_API;

/**
 * @deprecated Deprecated since 11.0.
 * @brief Gets the app control handle from notification_ex action.
 * @since_tizen 5.5
 * @remarks The @a app_control should be released using app_control_destroy().
 * @param[in] handle The notification_ex action handle
 * @param[out] app_control The app control handle
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_action_h
 * @see noti_ex_action_app_control_create()
 * @par Sample code:
 * @code
#include <notification_ex.h>

{
	int ret;
	app_control_h app_control = NULL;

	ret = noti_ex_action_app_control_get(appcontrol_action, &app_control);
}
 * @endcode
 */
int noti_ex_action_app_control_get(noti_ex_action_h handle, app_control_h *app_control) TIZEN_DEPRECATED_API;

/**
 * @}
 */

#ifdef __cplusplus
}
#endif
#endif  /* __TIZEN_APPFW_NOTIFICATION_EX_APP_CONTROL_ACTION_H__ */
