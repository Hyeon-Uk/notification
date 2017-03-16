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

#ifndef __NOTIFICATION_INTERNAL_H__
#define __NOTIFICATION_INTERNAL_H__
#include <glib.h>
#include <sys/types.h>

#include <notification_error.h>
#include <notification_type.h>
#include <notification_list.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup NOTIFICATION_INTERNAL
 * @{
 */
#define NOTIFICATION_GLOBAL_UID -1
#define NOTIFICATION_DISPLAY_APP_HEADS_UP NOTIFICATION_DISPLAY_APP_ACTIVE /* To avoid build error */
#define NOTIFICATION_ERROR (notification_error_quark())

#define NOTIFICATION_LY_MAX NOTIFICATION_LY_EXTENSION
#define NOTIFICATION_EVENT_TYPE_MAX NOTIFICATION_EVENT_TYPE_CLICK_ON_TEXT_INPUT_BUTTON
#define NOTIFICATION_SOUND_TYPE_MAX NOTIFICATION_SOUND_TYPE_USER_DATA
#define NOTIFICATION_VIBRATION_TYPE_MAX NOTIFICATION_VIBRATION_TYPE_USER_DATA
#define NOTIFICATION_LED_OP_MAX NOTIFICATION_LED_OP_ON_CUSTOM_COLOR
#define NOTIFICATION_COUNT_DISPLAY_TYPE_MAX NOTIFICATION_COUNT_DISPLAY_TYPE_RIGHT
#define NOTIFICATION_TEXT_TYPE_MAX NOTIFICATION_TEXT_TYPE_CONTENT_EXTENSION
#define NOTIFICATION_IMAGE_TYPE_MAX NOTIFICATION_IMAGE_TYPE_EXTENSION
#define NOTIFICATION_EXECUTE_TYPE_MAX NOTIFICATION_EXECUTE_TYPE_MULTI_LAUNCH
#define NOTIFICATION_TYPE_MAX NOTIFICATION_TYPE_ONGOING
#define NOTIFICATION_OP_DATA_MAX NOTIFICATION_OP_DATA_EXTRA_INFO_2
#define NOTIFICATION_COUNT_POS_MAX NOTIFICATION_COUNT_POS_RIGHT
#define NOTIFICATION_VARIABLE_TYPE_MAX NOTIFICATION_VARIABLE_TYPE_COUNT

/**
 * @brief Enumeration for notification ongoing value type.
 * @since_tizen 3.0
 */
typedef enum _notification_ongoing_value_type {
	NOTIFICATION_ONGOING_VALUE_TYPE_PERCENT = 0,
	NOTIFICATION_ONGOING_VALUE_TYPE_TIME,
} notification_ongoing_value_type_e;

/**
 * @brief Enumeration for extension event type on notification.
 * @since_tizen 3.0
 * @see #notification_event_type_e
 */
typedef enum _notification_event_type_extension {
	NOTIFICATION_EVENT_TYPE_HIDDEN_BY_USER = 100,
	NOTIFICATION_EVENT_TYPE_HIDDEN_BY_TIMEOUT = 101,
	NOTIFICATION_EVENT_TYPE_PRESSED = 200,
	NOTIFICATION_EVENT_TYPE_DELETED = 201,
} notification_event_type_extension_e;

GQuark notification_error_quark(void);

/**
 * @brief This function add deferred task. the registered task will be executed when notification service become ready
 * @param[in] deferred_task_cb The callback function
 * @param[in] user_data The user data to be passed to the callback function
 * @return #NOTIFICATION_ERROR_NONE if success, other value if failure
 * @see #notification_error_e
 * @see notification_is_service_ready()
 */
int notification_add_deferred_task(
		void (*deferred_task_cb)(void *data), void *user_data);

/**
 * @brief This function remove deferred task.
 * @param[in] deferred_task_cb The callback function
 * @return #NOTIFICATION_ERROR_NONE if success, other value if failure
 * @see #notification_error_e
 * @see notification_is_service_ready()
 */
int notification_del_deferred_task(
		void (*deferred_task_cb)(void *data));


/**
 * @brief This function will be removed.
 * @see notification_unresister_changed_cb()
 */
int
notification_resister_changed_cb(
	void (*changed_cb)(void *data, notification_type_e type),
	void *user_data);

int
notification_resister_changed_cb_for_uid(
	void (*changed_cb)(void *data, notification_type_e type),
	void *user_data, uid_t uid);
/**
 * @brief This function will be removed.
 * @see notification_resister_changed_cb()
 */
int
notification_unresister_changed_cb(
	void (*changed_cb)(void *data, notification_type_e type));
int
notification_unresister_changed_cb_for_uid(
	void (*changed_cb)(void *data, notification_type_e type), uid_t uid);

/**
 * @brief Updates the progress of the inserted notification. This only works for the ongoing notification (NOTIFICATION_TYPE_ONGOING).
 * @details The Notification view on the notification area could be updated.
 * @param[in] noti Notification handle or NULL if priv_id is valid
 * @param[in] priv_id Private ID
 * @param[in] progress Percentage value of progressive data
 * @return NOTIFICATION_ERROR_NONE on success, other value on failure
 * @retval NOTIFICATION_ERROR_NONE Success
 * @retval NOTIFICATION_ERROR_INVALID_PARAMETER Invalid input value
 * @par Sample code:
 * @code
#include <notification.h>
...
 {
	int noti_err = NOTIFICATION_ERROR_NONE;

	noti_err  = notification_update_progress(NULL, APP_NOTI_PRIV_ID, 0.6);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		return;
	}
}
 * @endcode
 */
int notification_update_progress(notification_h noti,
						  int priv_id,
						  double progress);

/**
 * @brief Updates the size of inserted notification data. This only works for the ongoing notification (NOTIFICATION_TYPE_ONGOING).
 * @details Notification view on notification area could be updated.
 * @param[in] noti Notification handle or NULL if priv_id is valid
 * @param[in] priv_id Private ID
 * @param[in] size Bytes of progressive data
 * @return NOTIFICATION_ERROR_NONE if success, other value if failure
 * @retval NOTIFICATION_ERROR_NONE Success
 * @retval NOTIFICATION_ERROR_INVALID_PARAMETER Invalid input value
 * @par Sample code:
 * @code
#include <notification.h>
...
 {
	int noti_err = NOTIFICATION_ERROR_NONE;

	noti_err  = notification_update_size(NULL, APP_NOTI_PRIV_ID, 3000000);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		return;
	}
}
 * @endcode
 */
int notification_update_size(notification_h noti,
					      int priv_id, double size);

/**
 * @brief Updates the content of the inserted notification data. This is only for the ongoing notification (NOTIFICATION_TYPE_ONGOING).
 * @details Notification view on notification area could be updated.
 * @param[in] noti Notification handle or NULL if priv_id is valid
 * @param[in] priv_id Private ID
 * @param[in] content Text to update
 * @return NOTIFICATION_ERROR_NONE on success, other value on failure
 * @retval NOTIFICATION_ERROR_NONE Success
 * @retval NOTIFICATION_ERROR_INVALID_PARAMETER Invalid input value
 * @par Sample code:
 * @code
#include <notification.h>
...
 {
	int noti_err = NOTIFICATION_ERROR_NONE;

	noti_err  = notification_update_content(NULL, APP_NOTI_PRIV_ID, "updated string");
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		return;
	}
}
 * @endcode
 */
int notification_update_content(notification_h noti,
							 int priv_id,
							 const char *content);

/**
 * @brief This function will be deprecated.
 * @see notification_set_image()
 *
 */
NOTIFICATION_DEPRECATED_API int notification_set_icon(notification_h noti,
					   const char *icon_path);

/**
 * @brief This function will be deprecated.
 * @see notification_get_image()
 *
 */
NOTIFICATION_DEPRECATED_API int notification_get_icon(notification_h noti,
					   char **icon_path);

/**
 * @brief This function will be deprecated.
 * @see notification_set_text()
 *
 */
NOTIFICATION_DEPRECATED_API int notification_set_title(notification_h noti,
					    const char *title,
					    const char *loc_title);

/**
 * @brief This function will be deprecated.
 * @see notification_get_text()
 *
 */
NOTIFICATION_DEPRECATED_API int notification_get_title(notification_h noti,
					    char **title,
					    char **loc_title);

/**
 * @brief This function will be deprecated.
 * @see notification_set_text()
 *
 */
NOTIFICATION_DEPRECATED_API int notification_set_content(notification_h noti,
					      const char *content,
					      const char *loc_content);

/**
 * @brief This function will be deprecated.
 * @see notification_get_text()
 *
 */
NOTIFICATION_DEPRECATED_API int notification_get_content(notification_h noti,
					      char **content,
					      char **loc_content);

/**
 * @brief This function will be removed.
 * @see notification_set_execute_option()
 *
 */
NOTIFICATION_DEPRECATED_API int notification_set_application(notification_h noti, const char *pkgname);	/* Do not use this */

/**
 * @brief This function will be removed.
 * @see notification_get_execute_option()
 *
 */
NOTIFICATION_DEPRECATED_API int notification_get_application(notification_h noti, char **pkgname);	/* Do not use this */

/**
 * @brief This function will be deprecated.
 * @see notification_set_execute_option()
 *
 */
NOTIFICATION_DEPRECATED_API int notification_set_args(notification_h noti, bundle *args, bundle *group_args);	/* Do not use this */

/**
 * @brief This function will be deprecated.
 * @see notification_get_execute_option()
 *
 */
NOTIFICATION_DEPRECATED_API int notification_get_args(notification_h noti, bundle **args, bundle **group_args);	/* Do not use this */

/**
 * @brief This function is deprecated.
 * @see notification_get_grouping_list()
 *
 */
NOTIFICATION_DEPRECATED_API int notification_get_grouping_list(notification_type_e type,
						    int count,
						    notification_list_h *list);

/**
 * @brief This function will be deprecated.
 * @see notification_delete_by_priv_id()
 *
 */
NOTIFICATION_DEPRECATED_API int notification_delete_group_by_group_id(const char *pkgname,
							   notification_type_e type,
							   int group_id);

/**
 * @brief This function will be deprecated.
 * @see notification_delete_by_priv_id()
 *
 */
NOTIFICATION_DEPRECATED_API int notification_delete_group_by_priv_id(const char *pkgname,
							  notification_type_e type,
							  int priv_id);
/**
 * @brief This function will be deprecated.
 *
 */
NOTIFICATION_DEPRECATED_API int notification_get_count(notification_type_e type,
					    const char *pkgname,
					    int group_id, int priv_id,
					    int *count);

/**
 * @brief This function will be deprecated.
 *
 */
NOTIFICATION_DEPRECATED_API int notifiation_clear(notification_type_e type);

/**
 * @internal
 * @brief This function will be deprecated.
 * @details Use only for the notification tray's clear button operation.
 * @param[in] type Notification type
 * @return NOTIFICATION_ERROR_NONE on success, other value on failure
 * @retval NOTIFICATION_ERROR_NONE Success
 * @retval NOTIFICATION_ERROR_INVALID_PARAMETER Invalid input value
 * @see #notification_type_e
 * @par Sample code:
 * @code
#include <notification.h>
...
 {
	int noti_err = NOTIFICATION_ERROR_NONE;

	noti_err  = notification_clear(NOTIFICATION_TYPE_NOTI);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		return;
	}
}
 * @endcode
 */
NOTIFICATION_DEPRECATED_API int notification_clear(notification_type_e type);

/**
 * @internal
 * @brief This function will be deprecated.
 * @return 1 on ready, other value on not ready
 */
NOTIFICATION_DEPRECATED_API int notification_is_service_ready(void);

/**
 * @internal
 * @brief This function will be deprecated.
 * @param[in] list Notification list handle
 * @return NOTIFICATION_ERROR_NONE on success, other value on failure
 * @retval NOTIFICATION_ERROR_NONE Success
 * @retval NOTIFICATION_ERROR_INVALID_PARAMETER Invalid parameter
 * @pre notification_get_grouping_list() or notification_get_detail_list().
 * @see #notification_op
 */
NOTIFICATION_DEPRECATED_API int notification_op_get_data(notification_op *noti_op,
						       notification_op_data_type_e type,
						       void *data);

/**
 * @internal
 * @brief This function will be deprecated.
 * @details Caller_pkgname is set automatically when notification_new() is called. We do not recommend to use this API.
 * @param[in] noti Notification handle
 * @param[in] pkgname Caller package name
 * @return NOTIFICATION_ERROR_NONE on success, other value on failure
 * @retval NOTIFICATION_ERROR_NONE Success
 * @retval NOTIFICATION_ERROR_INVALID_PARAMETER Invalid input value
 * @par Sample code:
 * @code
#include <notification.h>
...
{
	notification_h noti = NULL;
	int noti_err = NOTIFICATION_ERROR_NONE;

	noti = notification_new(NOTIFICATION_TYPE_NOTI, APP_GROUP_ID, NOTIFICATION_PRIV_ID_NONE);
	if (noti == NULL) {
		return;
	}

	noti_err  = notification_set_pkgname(noti, "org.tizen.phone");
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		notification_free(noti);
		return;
	}
}
 * @endcode
 */
NOTIFICATION_DEPRECATED_API int notification_set_pkgname(notification_h noti,
					      const char *pkgname);

/**
 * @internal
 * @brief This function will be deprecated.
 * @details If @a pkgname is NULL, caller_pkgname is set internally.
 * @param[in] pkgname Caller application package name or NULL
 * @param[in] type Notification type
 * @return NOTIFICATION_ERROR_NONE if success, other value if failure
 * @retval NOTIFICATION_ERROR_NONE Success
 * @retval NOTIFICATION_ERROR_INVALID_PARAMETER Invalid input value
 * @par Sample code:
 * @code
#include <notification.h>
...
 {
	int noti_err = NOTIFICATION_ERROR_NONE;

	noti_err  = notification_delete_all_by_type(NULL, NOTIFICATION_TYPE_NOTI);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		return;
	}
}
 * @endcode
 */
NOTIFICATION_DEPRECATED_API int notification_delete_all_by_type(const char *pkgname,
						     notification_type_e type);

/**
 * @internal
 * @brief This function will be deprecated.
 * @details If @a pkgname is NULL, caller_pkgname is set internally.
 * @param[in] pkgname Caller application package name or NULL
 * @param[in] type Notification type
 * @param[in] priv_id Priv ID
 * @return NOTIFICATION_ERROR_NONE if success, other value if failure
 * @retval NOTIFICATION_ERROR_NONE Success
 * @retval NOTIFICATION_ERROR_INVALID_PARAMETER Invalid input value
 * @par Sample code:
 * @code
#include <notification.h>
...
 {
	int noti_err = NOTIFICATION_ERROR_NONE;

	noti_err  = notification_delete_by_priv_id(NULL, NOTIFICATION_TYPE_NOTI, APP_PRIV_ID);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		return;
	}
}
 * @endcode
 */
NOTIFICATION_DEPRECATED_API int notification_delete_by_priv_id(const char *pkgname,
						    notification_type_e type,
						    int priv_id);

/**
 * @internal
 * @brief This function will be deprecated.
 * @details When notification data selected in display application, application launched by appsvc_run_service with service_handle.
 * @param[in] noti Notification handle
 * @param[in] type Notification execute type
 * @param[in] text Basic text for button
 * @param[in] key Value for localized text
 * @param[in] service_handle Appsvc bundle data
 * @return NOTIFICATION_ERROR_NONE on success, other value on failure
 * @retval NOTIFICATION_ERROR_NONE Success
 * @retval NOTIFICATION_ERROR_INVALID_PARAMETER Invalid parameter
 * @par Sample code:
 * @code
#include <notification.h>
...
{
	notification_h noti = NULL;
	int noti_err = NOTIFICATION_ERROR_NONE;
	bundle *b = NULL;

	...

	b = bundle_create();
	appsvc_set_operation(b, APPSVC_OPERATION_VIEW);
	appsvc_set_uri(b,"http://www.samsung.com");

	noti_err  = notification_set_execute_option(noti, NOTIFICATION_EXECUTE_TYPE_SINGLE_LAUNCH, NULL, NULL, b);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		notification_free(noti);
		return;
	}

	bundle_free(b);
}
 * @endcode
 */
NOTIFICATION_DEPRECATED_API int notification_set_execute_option(notification_h noti,
						     notification_execute_type_e type,
						     const char *text,
						     const char *key,
						     bundle *service_handle);

/**
 * @internal
 * @brief This function will be deprecated.
 * @remarks ID is valid only after inserting the notification.
 * @param[in] noti Notification handle
 * @param[out] group_id Group ID
 * @param[out] priv_id Private ID
 * @return NOTIFICATION_ERROR_NONE on success, other value on failure
 * @retval NOTIFICATION_ERROR_NONE Success
 * @retval NOTIFICATION_ERROR_INVALID_PARAMETER Invalid parameter
 * @par Sample code:
 * @code
#include <notification.h>
 ...
  {
	 int noti_err = NOTIFICATION_ERROR_NONE;
	 int group_id, priv_id;

	 noti_err  = notification_get_id(noti, &group_id, &priv_id);
	 if (noti_err != NOTIFICATION_ERROR_NONE) {
		 return;
	 }
 }
 * @endcode
 */
NOTIFICATION_DEPRECATED_API int notification_get_id(notification_h noti,
					 int *group_id, int *priv_id);

/**
 * @internal
 * @brief This function will be deprecated.
 * @param[in] type Notification type
 * @param[in] group_id Group ID
 * @param[in] priv_id Priv ID
 * @return Notification handle(#notification_h) on success, NULL on failure
 * @retval #notification_h Success
 * @retval NULL Failure
 * @see #notification_type_e
 * @see #notification_h
 * @par Sample code:
 * @code
#include <notification.h>
...
{
	notification_h noti = NULL;

	noti = notification_load("org.tizen.app", priv_id);
	if (noti == NULL) {
		return;
	}
	...
}
 * @endcode
 */
NOTIFICATION_DEPRECATED_API notification_h notification_load(char *pkgname,
				int priv_id);

/**
 * @internal
 * @brief This function will be deprecated.
 * @details Available types are #NOTIFICATION_TYPE_NOTI and #NOTIFICATION_TYPE_ONGOING.
 * #NOTIFICATION_TYPE_NOTI is remaining notification data even if device is restarted.
 * #NOTIFICATION_TYPE_ONGOING can display progressive feather, but notification data is removed after device is restarted.
 * If group_id is #NOTIFICATION_GROUP_ID_NONE, notification data is not grouped. #NOTIFICATION_GROUP_ID_DEFAULT,
 * notification data is grouped with same title. Positive number ( > 0 ) is grouped with same number.
 * If priv_id is #NOTIFICATION_PRIV_ID_NONE, priv_id is set internally and return it when notification_insert() call.
 * Positive number and zero ( >= 0 ) is application set private ID. These ID should have be unique each application package.
 * @param[in] type Notification type
 * @param[in] group_id Group ID
 * @param[in] priv_id Priv ID
 * @return Notification handle(#notification_h) on success, NULL on failure
 * @retval #notification_h Success
 * @retval NULL Failure
 * @see #notification_type_e
 * @see #notification_h
 * @par Sample code:
 * @code
#include <notification.h>
...
{
	notification_h noti = NULL;

	noti = notification_new(NOTIFICATION_TYPE_NOTI, APP_GROUP_ID, NOTIFICATION_PRIV_ID_NONE);
	if (noti == NULL) {
		return;
	}
	...
}
 * @endcode
 */
NOTIFICATION_DEPRECATED_API notification_h notification_new(notification_type_e type, int group_id,
				int priv_id);

/**
 * @internal
 * @brief This function will be deprecated.
 * @param[in] noti Notification handle
 * @param[in] type Notification execute type
 * @param[out] text Text for button
 * @param[out] service_handle Appsvc bundle data
 * @return NOTIFICATION_ERROR_NONE on success, other value on failure
 * @retval NOTIFICATION_ERROR_NONE Success
 * @retval NOTIFICATION_ERROR_INVALID_PARAMETER Invalid parameter
 * @par Sample code:
 * @code
#include <notification.h>
...
{
	notification_h noti = NULL;
	int noti_err = NOTIFICATION_ERROR_NONE;
	bundle *b = NULL;

	...

	noti_err  = notification_get_execute_option(noti, NOTIFICATION_EXECUTE_TYPE_SINGLE_LAUNCH, NULL, NULL, &b);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		notification_free(noti);
		return;
	}
}
 * @endcode
 */
NOTIFICATION_DEPRECATED_API int notification_get_execute_option(notification_h noti,
						     notification_execute_type_e type,
						     const char **text,
						     bundle **service_handle);

/**
 * @internal
 * @brief Inserts a notification.
 * @details The notification will be inserted to the database and then it will appear in the notification area.
 *          When notification_create() is called, if priv_id is #NOTIFICATION_PRIV_ID_NONE, priv_id returns the internally set priv_id.
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 * @privlevel public
 * @privilege %http://tizen.org/privilege/notification
 * @param[in]  noti    The notification handle
 * @param[out] priv_id The private ID
 * @return #NOTIFICATION_ERROR_NONE on success,
 *         otherwise any other value on failure
 * @retval #NOTIFICATION_ERROR_NONE         Success
 * @retval #NOTIFICATION_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval NOTIFICATION_ERROR_PERMISSION_DENIED The application does not have the privilege to call this method
 * @pre Notification handle should be created by notification_create().
 * @post notification_free().
 * @par Sample code:
 * @code
#include <notification.h>
...
 {
	int noti_err = NOTIFICATION_ERROR_NONE;

	noti_err  = notification_insert(noti, NULL);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		return;
	}
}
 * @endcode
 */
int notification_insert(notification_h noti,
		int *priv_id);

int notification_insert_for_uid(notification_h noti,
		int *priv_id, uid_t uid);

/**
 * @internal
 * @brief Updates a notification, asynchronously.
 * @details The updated notification will appear in the notification area.
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 * @privlevel public
 * @privilege %http://tizen.org/privilege/notification
 * @remarks This function updates the notification asynchronously.
 * @param[in] noti      The notification handle that is created by notification_create()
 * @param[in] result_cb The callback called when an update completed
 * @param[in] user_data The user data which you want to use in callback
 * @return #NOTIFICATION_ERROR_NONE on success,
 *         otherwise any other value on failure
 * @retval #NOTIFICATION_ERROR_NONE         Success
 * @retval #NOTIFICATION_ERROR_INVALID_PARAMETER Invalid input value
 * @retval #NOTIFICATION_ERROR_NOT_EXIST_ID Priv ID does not exist
 * @retval #NOTIFICATION_ERROR_PERMISSION_DENIED The application does not have the privilege to call this method
 * @par Sample code:
 * @code
#include <notification.h>
...
 {
	int noti_err = NOTIFICATION_ERROR_NONE;

	noti_err  = notification_update_async(NULL, result_cb, data);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		return;
	}
}
 * @endcode
 */
int notification_update_async(notification_h noti,
		void (*result_cb)(int priv_id, int result, void *data), void *user_data);
int notification_update_async_for_uid(notification_h noti,
		void (*result_cb)(int priv_id, int result, void *data), void *user_data, uid_t uid);

/**
 * @internal
 * @brief Registers a callback for all notification events.
 * @details The registered callback could be called for all notification events.
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 * @privlevel public
 * @privilege %http://tizen.org/privilege/notification
 * @param[in] changed_cb The callback function
 * @param[in] user_data  The user data
 * @return #NOTIFICATION_ERROR_NONE on success,
 *         otherwise any other value on failure
 * @retval #NOTIFICATION_ERROR_NONE         Success
 * @retval #NOTIFICATION_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #NOTIFICATION_ERROR_PERMISSION_DENIED The application does not have the privilege to call this method
 * @see notification_unregister_detailed_changed_cb()
 * @par Sample code:
 * @code
#include <notification.h>
...
{
	noti_err = notification_register_detailed_changed_cb(app_changed_cb, user_data);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		return;
	}
}
 * @endcode
 */
int notification_register_detailed_changed_cb(
		void (*detailed_changed_cb)(void *data, notification_type_e type, notification_op *op_list, int num_op),
		void *user_data);
int notification_register_detailed_changed_cb_for_uid(
		void (*detailed_changed_cb)(void *data, notification_type_e type, notification_op *op_list, int num_op),
		void *user_data, uid_t uid);

/**
 * @internal
 * @brief Unregisters a callback for all notification events.
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 * @privlevel public
 * @privilege %http://tizen.org/privilege/notification
 * @param[in] changed_cb The callback function
 * @return #NOTIFICATION_ERROR_NONE on success,
 *         otherwise any other value on failure
 * @retval #NOTIFICATION_ERROR_NONE         Success
 * @retval #NOTIFICATION_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #NOTIFICATION_ERROR_PERMISSION_DENIED The application does not have the privilege to call this method
 * @see notification_register_detailed_changed_cb()
 * @par Sample code:
 * @code
#include <notification.h>
...
{
	noti_err = notification_register_detailed_changed_cb(app_changed_cb, user_data);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		return;
	}
}
 * @endcode
 */
int notification_unregister_detailed_changed_cb(
		void (*detailed_changed_cb)(void *data, notification_type_e type, notification_op *op_list, int num_op),
		void *user_data);
int notification_unregister_detailed_changed_cb_for_uid(
		void (*detailed_changed_cb)(void *data, notification_type_e type, notification_op *op_list, int num_op),
		void *user_data, uid_t uid);
/**
 * @brief Sets the default button to display highlight on the notification.
 * @since_tizen 3.0
 * @remarks If you want to default button is off, you set that index is zero
 * @param[in] noti       The notification handle
 * @param[in] index      The notification button index
 * @return #NOTIFICATION_ERROR_NONE on success,
 *         otherwise any other value on failure
 * @retval #NOTIFICATION_ERROR_NONE         Success
 * @retval #NOTIFICATION_ERROR_INVALID_PARAMETER Invalid parameter
 * @pre Notification handle should be created by notification_create().
 * @see #notification_button_index_e
 * @par Sample code:
 * @code
#include <notification.h>
...
{
	notification_h noti = NULL;
	int noti_err = NOTIFICATION_ERROR_NONE;

	noti = notification_create(NOTIFICATION_TYPE_NOTI);
	if(noti == NULL) {
		return;
	}

	noti_err  = notification_set_default_button(noti, NOTIFICATION_BUTTON_1);
	if(noti_err != NOTIFICATION_ERROR_NONE) {
		notification_free(noti);
		return;
	}

	...

}
 * @endcode
 */
int notification_set_default_button(notification_h noti, notification_button_index_e index);

/**
 * @brief Gets the default button to display highlight on the notification.
 * @since_tizen 3.0
 * @param[in] noti       The notification handle
 * @param[out] index     The notification button index
 * @return #NOTIFICATION_ERROR_NONE on success,
 *         otherwise any other value on failure
 * @retval #NOTIFICATION_ERROR_NONE         Success
 * @retval #NOTIFICATION_ERROR_INVALID_PARAMETER Invalid parameter
 * @pre Notification handle should be created by notification_create().
 * @see #notification_button_index_e
 * @par Sample code:
 * @code
#include <notification.h>
...
{
	notification_h noti = NULL;
	notification_button_index_e index;
	int noti_err = NOTIFICATION_ERROR_NONE;

	noti = notification_create(NOTIFICATION_TYPE_NOTI);
	if(noti == NULL) {
		return;
	}

	noti_err  = notification_set_default_button(noti, &index);
	if(noti_err != NOTIFICATION_ERROR_NONE) {
		notification_free(noti);
		return;
	}

	...

}
 * @endcode
 */
int notification_get_default_button(notification_h noti, notification_button_index_e *index);

/**
 * @brief Gets the notification ongoing value type.
 * @since_tizen 3.0
 * @param[in] noti	The notification handle
 * @param[out] type	The notification ongoing value type
 * @return #NOTIFICATION_ERROR_NONE on success,
 *         otherwise any other value on failure
 * @retval #NOTIFICATION_ERROR_NONE         Success
 * @retval #NOTIFICATION_ERROR_INVALID_PARAMETER Invalid parameter
 * @pre Notification handle should be created by notification_create().
 * @see #notification_ongoing_value_type_e
 * @par Sample code:
 * @code
#include <notification.h>
...
{
	notification_h noti = NULL;
	notification_ongoing_value_type_e type;
	int noti_err = NOTIFICATION_ERROR_NONE;

	noti = notification_create(NOTIFICATION_TYPE_NOTI);
	if (noti == NULL) {
		return;
	}

	noti_err  = notification_get_ongoing_value_type(noti, &type);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		notification_free(noti);
		return;
	}
}
 * @endcode
 */
int notification_get_ongoing_value_type(notification_h noti, notification_ongoing_value_type_e *type);

/**
 * @brief Sets the notification ongoing value type.
 * @since_tizen 3.0
 * @param[in] noti	The notification handle
 * @param[in] type	The notification ongoing value type
 * @return #NOTIFICATION_ERROR_NONE on success,
 *         otherwise any other value on failure
 * @retval #NOTIFICATION_ERROR_NONE         Success
 * @retval #NOTIFICATION_ERROR_INVALID_PARAMETER Invalid parameter
 * @pre Notification handle should be created by notification_create().
 * @see #notification_ongoing_value_type_e
 * @par Sample code:
 * @code
#include <notification.h>
...
{
	notification_h noti = NULL;
	notification_ongoing_value_type_e type;
	int noti_err = NOTIFICATION_ERROR_NONE;

	noti = notification_create(NOTIFICATION_TYPE_NOTI);
	if (noti == NULL) {
		return;
	}

	type = NOTIFICATION_ONGOING_VALUE_TYPE_TIME;

	noti_err  = notification_set_ongoing_value_type(noti, type);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		notification_free(noti);
		return;
	}
}
 * @endcode
 */
int notification_set_ongoing_value_type(notification_h noti, notification_ongoing_value_type_e type);

/**
 * @brief Gets the notification ongoing time when ongoint type value is set NOTIFICATION_ONGOING_VALUE_TYPE_TIME.
 * @since_tizen 3.0
 * @param[in] noti	The notification handle
 * @param[out] current	The ongoing current time
 * @param[out] duration	The ongoing duration time
 * @return #NOTIFICATION_ERROR_NONE on success,
 *         otherwise any other value on failure
 * @retval #NOTIFICATION_ERROR_NONE         Success
 * @retval #NOTIFICATION_ERROR_INVALID_PARAMETER Invalid parameter
 * @pre Notification handle should be created by notification_create().
 * @see #notification_ongoing_value_type_e
 * @par Sample code:
 * @code
#include <notification.h>
...
{
	notification_h noti = NULL;
	int noti_err = NOTIFICATION_ERROR_NONE;
	int current;
	int duration;

	noti = notification_create(NOTIFICATION_TYPE_NOTI);
	if (noti == NULL) {
		return;
	}

	noti_err  = notification_get_ongoing_time(noti, &current, &duration);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		notification_free(noti);
		return;
	}
}
 * @endcode
 */
int notification_get_ongoing_time(notification_h noti, int *current, int *duration);

/**
 * @brief Sets the notification ongoing time when ongoint type value is set NOTIFICATION_ONGOING_VALUE_TYPE_TIME.
 * @since_tizen 3.0
 * @param[in] noti	The notification handle
 * @param[in] current	The ongoing current time
 * @param[in] duration	The ongoing duration time
 * @return #NOTIFICATION_ERROR_NONE on success,
 *         otherwise any other value on failure
 * @retval #NOTIFICATION_ERROR_NONE         Success
 * @retval #NOTIFICATION_ERROR_INVALID_PARAMETER Invalid parameter
 * @pre Notification handle should be created by notification_create().
 * @see #notification_ongoing_value_type_e
 * @par Sample code:
 * @code
#include <notification.h>
...
{
	notification_h noti = NULL;
	int noti_err = NOTIFICATION_ERROR_NONE;
	int current;
	int duration;

	noti = notification_create(NOTIFICATION_TYPE_NOTI);
	if (noti == NULL) {
		return;
	}

	current = 0;
	duration = 30;

	noti_err  = notification_set_ongoing_time(noti, current, duration)
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		notification_free(noti);
		return;
	}
}
 * @endcode
 */
int notification_set_ongoing_time(notification_h noti, int current, int duration);

/**
 * @brief Gets timeout value in second when the notification can be hidden from the viewer.
 * @since_tizen 3.0
 * @param[in] noti	The notification handle
 * @param[out] timeout	The timeout time(sec)
 * @return #NOTIFICATION_ERROR_NONE on success,
 *         otherwise any other value on failure
 * @retval #NOTIFICATION_ERROR_NONE         Success
 * @retval #NOTIFICATION_ERROR_INVALID_PARAMETER Invalid parameter
 * @pre Notification handle should be created by notification_create().
 * @see #notification_event_type_extension_e
 * @par Sample code:
 * @code
#include <notification.h>
...
{
	notification_h noti = NULL;
	int noti_err = NOTIFICATION_ERROR_NONE;
	int timeout;

	noti = notification_create(NOTIFICATION_TYPE_NOTI);
	if (noti == NULL) {
		return;
	}

	noti_err  = notification_get_hide_timeout(noti, &timeout)
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		notification_free(noti);
		return;
	}
}
 * @endcode
 */
int notification_get_hide_timeout(notification_h noti, int *timeout);

/**
 * @brief Sets timeout value in second when the notification can be hidden from the viewer.
 * @since_tizen 3.0
 * @param[in] noti	The notification handle
 * @param[in] timeout	The timeout time(sec)
 * @return #NOTIFICATION_ERROR_NONE on success,
 *         otherwise any other value on failure
 * @retval #NOTIFICATION_ERROR_NONE         Success
 * @retval #NOTIFICATION_ERROR_INVALID_PARAMETER Invalid parameter
 * @pre Notification handle should be created by notification_create().
 * @see #notification_event_type_extension_e
 * @par Sample code:
 * @code
#include <notification.h>
...
{
	notification_h noti = NULL;
	int noti_err = NOTIFICATION_ERROR_NONE;

	noti = notification_create(NOTIFICATION_TYPE_NOTI);
	if (noti == NULL) {
		return;
	}

	noti_err  = notification_set_hide_timeout(noti, 10)
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		notification_free(noti);
		return;
	}
}
 * @endcode
 */
int notification_set_hide_timeout(notification_h noti, int timeout);

/**
 * @brief Gets timeout value in second when the notification can be deleted from the viewer.
 * @since_tizen 3.0
 * @param[in] noti	The notification handle
 * @param[out] timeout	The timeout time(sec)
 * @return #NOTIFICATION_ERROR_NONE on success,
 *         otherwise any other value on failure
 * @retval #NOTIFICATION_ERROR_NONE         Success
 * @retval #NOTIFICATION_ERROR_INVALID_PARAMETER Invalid parameter
 * @pre Notification handle should be created by notification_create().
 * @par Sample code:
 * @code
#include <notification.h>
...
{
	notification_h noti = NULL;
	int noti_err = NOTIFICATION_ERROR_NONE;
	int timeout;

	noti = notification_create(NOTIFICATION_TYPE_NOTI);
	if (noti == NULL) {
		return;
	}

	noti_err  = notification_get_delete_timeout(noti, &timeout)
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		notification_free(noti);
		return;
	}
}
 * @endcode
 */
int notification_get_delete_timeout(notification_h noti, int *timeout);

/**
 * @brief Sets timeout value in second when the notification can be deleted from the viewer.
 * @since_tizen 3.0
 * @param[in] noti	The notification handle
 * @param[in] timeout	The timeout time(sec)
 * @return #NOTIFICATION_ERROR_NONE on success,
 *         otherwise any other value on failure
 * @retval #NOTIFICATION_ERROR_NONE         Success
 * @retval #NOTIFICATION_ERROR_INVALID_PARAMETER Invalid parameter
 * @pre Notification handle should be created by notification_create().
 * @par Sample code:
 * @code
#include <notification.h>
...
{
	notification_h noti = NULL;
	int noti_err = NOTIFICATION_ERROR_NONE;

	noti = notification_create(NOTIFICATION_TYPE_NOTI);
	if (noti == NULL) {
		return;
	}

	noti_err  = notification_set_delete_timeout(noti, 10)
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		notification_free(noti);
		return;
	}
}
 * @endcode
 */
int notification_set_delete_timeout(notification_h noti, int timeout);

typedef void (*event_handler_cb)(notification_h noti, int event_type, void *userdata);

/**
 * @brief Posts a notification with event handler callback.
 * @details The registered callback could be called when take notification event
 *	    and the callback is automatically deleted when notification you posted is deleted.
 * @since_tizen 3.0
 * @privlevel public
 * @privilege %http://tizen.org/privilege/notification
 * @remarks Providing one event callback for each notification handle and distinguish the event by parameter.
 * @param[in] noti	The notification handle
 * @param[in] cb	The event handler callback function
 * @param[in] userdata	The user data
 * @return #NOTIFICATION_ERROR_NONE on success,
 *         otherwise any other value on failure
 * @retval #NOTIFICATION_ERROR_NONE         Success
 * @retval #NOTIFICATION_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #NOTIFICATION_ERROR_IO_ERROR I/O Error
 * @retval #NOTIFICATION_PERMISSION_DENIED Permission denied
 * @pre Notification handle should be created by notification_create().
 * @par Sample code:
 * @code
#include <notification.h>

static void event_callback(notification noti, int event_type, void *userdata)
{
	...
}

...
{
	notification_h noti = NULL;
	int noti_err = NOTIFICATION_ERROR_NONE;

	noti = notification_create(NOTIFICATION_TYPE_NOTI);
	if (noti == NULL) {
		return;
	}

	noti_err  = notification_post_with_event_cb(noti, event_callback, NULL);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		notification_free(noti);
		return;
	}

	...

	notification_free(noti);
}
 * @endcode
 */
int notification_post_with_event_cb(notification_h noti, event_handler_cb cb, void *userdata);
int notification_post_with_event_cb_for_uid(notification_h noti, event_handler_cb cb,
					    void *userdata, uid_t uid);

/**
 * @brief Sends a event type to an application that posted notification.
 * @details Sends occured event from viewer application to an application.
 * @since_tizen 3.0
 * @privlevel public
 * @privilege %http://tizen.org/privilege/notification
 * @param[in] noti		The notification handle
 * @param[in] event_type	The event type
 * @return #NOTIFICATION_ERROR_NONE on success,
 *         otherwise any other value on failure
 * @retval #NOTIFICATION_ERROR_NONE         Success
 * @retval #NOTIFICATION_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #NOTIFICATION_ERROR_IO_ERROR I/O Error
 * @retval #NOTIFICATION_PERMISSION_DENIED Permission denied
 * @see #notification_event_type_e
 * @see #notification_event_type_extension_e
 * @par Sample code:
 * @code
#include <notification.h>
...
{
	int noti_err;
	int event_type;

	...

	event_type = NOTIFICATION_EVENT_TYPE_HIDDEN_BY_USER;

	noti_err  = notification_send_event(noti, event_type);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		notification_free(noti);
		return;
	}
}
 * @endcode
 */
int notification_send_event(notification_h noti, int event_type);

/**
 * @brief Gets the event flag.
 * @details When you create a notification handle, a default value of event flag is false.
 *	    The flag automatically set true when post a notification using notification_post_with_event_cb().
 *	    The viewer application for showing the notifications can use this API to check if it needs to call
 *	    notification_send_event() to sends event of notification for making the callback of the processes
 *	    that have posted notification to be called. Call notification_send_event()
 *	    when the notification_get_event_flag() tells that the @event_flag is true.
 * @since_tizen 3.0
 * @param[in] noti	The notification handle
 * @param[out] event_flag	The event flag
 * @return #NOTIFICATION_ERROR_NONE on success,
 *         otherwise any other value on failure
 * @retval #NOTIFICATION_ERROR_NONE         Success
 * @retval #NOTIFICATION_ERROR_INVALID_PARAMETER Invalid parameter
 * @pre Notification handle should be created by notification_create().
 * @par Sample code:
 * @code
#include <notification.h>
...
{
	notification_h noti = NULL;
	int noti_err;
	bool event_flag;

	noti = notification_create(NOTIFICATION_TYPE_NOTI);
	if (noti == NULL) {
		return;
	}

	noti_err  = notification_get_event_flag(noti, &event_flag);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		notification_free(noti);
		return;
	}

	...

	notification_free(noti);
}
 * @endcode
 */
int notification_get_event_flag(notification_h noti, bool *event_flag);

/**
 * @brief Gets whether sending event is available.
 * @details If the application is terminated, registered callback isn't called
 *          for event sent by other process. So you need to check whether the
 *          callback can be called before sending event.
 * @since_tizen 3.0
 * @param[in] noti The notification handle
 * @param[out] available The value whether sending event is available
 * @return #NOTIFICATION_ERROR_NONE on success,
 *         otherwise any other value on failure
 * @retval #NOTIFICATION_ERROR_NONE Success
 * @retval #NOTIFICATION_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #NOTIFICATION_ERROR_IO_ERROR I/O Error
 * @retval #NOTIFICATION_ERROR_PERMISSION_DENIED Permission denied
 * @retval #NOTIFICATION_ERROR_OUT_OF_MEMORY Out of memory
 * @retval #NOTIFICATION_ERROR_SERVICE_NOT_READY No response from notification service
 * @see #notification_send_event()
 * @par Sample code:
 * @code
#include <notification.h>
...
{
	notification_h noti = NULL;
	int noti_err;
	bool available;

	...

	noti_err  = notification_check_event_receiver_available(noti, &available);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		notification_free(noti);
		return;
	}

	...

	notification_free(noti);
}
 * @endcode
 */
int notification_check_event_receiver_available(notification_h noti, bool *available);

/**
 * @brief This function translate localized texts
 * @param[in] noti The notification handle that is created by notification_create()
 * @return #NOTIFICATION_ERROR_NONE if success, other value if failure
 * @see notification_create()
 */
int notification_translate_localized_text(notification_h noti);
void notification_reset_event_handler_list(void);
int notification_set_uid(notification_h noti, uid_t uid);
int notification_get_uid(notification_h noti, uid_t *uid);
int notification_post_for_uid(notification_h noti, uid_t uid);
int notification_update_for_uid(notification_h noti, uid_t uid);
int notification_delete_for_uid(notification_h noti, uid_t uid);
int notification_delete_all_for_uid(notification_type_e type, uid_t uid);
notification_h notification_load_by_tag_for_uid(const char *tag, uid_t uid);
notification_h notification_create_from_package_template(const char *pkgname,
					const char *template_name);
/**
 * @brief Gets a max length of text input.
 * @since_tizen 3.0
 * @return #NOTIFICATION_ERROR_NONE on success,
 *         otherwise any other value on failure
 * @retval #NOTIFICATION_ERROR_NONE	Success
 * @retval #NOTIFICATION_ERROR_INVALID_PARAMETER	Invalid parameter
 * @par Sample code:
 * @code
#include <notification.h>
...
{
	int noti_err = NOTIFICATION_ERROR_NONE;
	int text_input_max_length;

	noti_err = notification_get_text_input_max_length(noti, &text_input_max_length);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		return;
	}
	...
 }
 * @endcode
 */
int notification_get_text_input_max_length(notification_h noti, int *text_input_max_length);

/**
 * @}
 */
#ifdef __cplusplus
}
#endif
#endif
