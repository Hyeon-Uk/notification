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

#ifndef __NOTIFICATION_LIST_H__
#define __NOTIFICATION_LIST_H__

#include <sys/types.h>
#include <notification.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file notification_list.h
 * @brief This file contains the notification list APIs.
 */

/**
 * @internal
 * @addtogroup NOTIFICATION_LIST
 * @{
 */

/**
 * @brief Notification list handle.
 * @since_tizen 2.3
 */
typedef struct _notification_list *notification_list_h;


/**
 * @internal
 * @brief Returns the notification list handle.
 * @details If count is equal to @c -1, all notifications are returned.
 * @since_tizen 2.3
 * @privlevel public
 * @privilege %http://tizen.org/privilege/notification
 * @remarks The @a list should be released using notification_free_list().
 * @param[in] type The notification type
 * @param[in] count The returned notification data number
 * @param[out] list The notification list handle
 * @return #NOTIFICATION_ERROR_NONE on success,
 *         otherwise any other value on failure
 * @retval #NOTIFICATION_ERROR_NONE         Success
 * @retval #NOTIFICATION_ERROR_PERMISSION_DENIED Permission denied
 * @retval #NOTIFICATION_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #notification_list_h
 * @par Sample code:
 * @code
#include <notification.h>
...
{
	notification_list_h noti_list = NULL;
	int noti_err = NOTIFICATION_ERROR_NONE;

	noti_err = notification_get_list(NOTIFICATION_TYPE_NONE, -1, &noti_list);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		return;
	}

	notification_free_list(noti_list);
}
 * @endcode
 */
int notification_get_list(notification_type_e type, int count,
			notification_list_h *list);

int notification_get_list_for_uid(notification_type_e type, int count,
			notification_list_h *list, uid_t uid);

/**
 * @internal
 * @brief Gets the notification list associated with the partition into pages.
 * @since_tizen 4.0
 * @privlevel public
 * @privilege %http://tizen.org/privilege/notification
 * @remarks The @a list should be released using notification_free_list().
 * @param[in]  type            The notification type
 * @param[in]  page_number     The page number of the value set \n
 *                             It starts from @c 1.
 * @param[in]  count_per_page  The desired maximum count of the data items per page
 *                             The maximum value is 100, If the value is set more than 100, \n
 *                             it is automatically set 100.
 * @param[out] list            The notification list handle
 * @return #NOTIFICATION_ERROR_NONE on success,
 *         otherwise any other value on failure
 * @retval #NOTIFICATION_ERROR_NONE         Success
 * @retval #NOTIFICATION_ERROR_PERMISSION_DENIED Permission denied
 * @retval #NOTIFICATION_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #notification_list_h
 */
int notification_get_list_by_page(notification_type_e type,
		int page_number, int count_per_page, notification_list_h *list);

int notification_get_list_by_page_for_uid(notification_type_e type,
		int page_number, int count_per_page, notification_list_h *list, uid_t uid);
/**
 * @internal
 * @brief Returns the notification detail list handle of grouping data.
 * @details If count is equal to c -1, all notifications are returned.
 * @since_tizen 2.3
 * @privlevel public
 * @privilege %http://tizen.org/privilege/notification
 * @remarks The @a list should be released using notification_free_list().
 * @param[in]  app_id  The caller application ID
 * @param[in]  group_id The group ID
 * @param[in]  priv_id  The private ID
 * @param[in]  count    The returned notification data number
 * @param[out] list     The notification list handle
 * @return #NOTIFICATION_ERROR_NONE if success,
 *         other value if failure
 * @retval #NOTIFICATION_ERROR_NONE Success
 * @retval #NOTIFICATION_ERROR_PERMISSION_DENIED Permission denied
 * @retval #NOTIFICATION_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #notification_list_h
 * @par Sample code:
 * @code
#include <notification.h>
...
{
	notification_list_h noti_list = NULL;
	int noti_err = NOTIFICATION_ERROR_NONE;

	noti_err = notification_get_detail_list(app_id, group_id, priv_id, -1, &noti_list);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		return;
	}

	notification_free_list(noti_list);
}
 * @endcode
 */
int notification_get_detail_list(const char *app_id,
		int group_id,
		int priv_id,
		int count,
		notification_list_h *list);

int notification_get_detail_list_for_uid(const char *app_id,
		int group_id,
		int priv_id,
		int count,
		notification_list_h *list,
		uid_t uid);

/**
 * @internal
 * @brief Frees a notification list.
 * @since_tizen 2.3
 * @privlevel public
 * @privilege %http://tizen.org/privilege/notification
 * @param[in] list The notification list handle
 * @return #NOTIFICATION_ERROR_NONE on success,
 *         otherwise any other value on failure
 * @retval #NOTIFICATION_ERROR_NONE         Success
 * @retval #NOTIFICATION_ERROR_PERMISSION_DENIED Permission denied
 * @retval #NOTIFICATION_ERROR_INVALID_PARAMETER Invalid parameter
 * @pre notification_get_grouping_list() or notification_get_detail_list().
 * @see #notification_list_h
 * @par Sample code:
 * @code
#include <notification.h>
...
{
	notification_list_h noti_list = NULL;
	int noti_err = NOTIFICATION_ERROR_NONE;

	...

	noti_err = notification_free_list(noti_list);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		return;
	}
}
 * @endcode
 */
int notification_free_list(notification_list_h list);


/**
 * @internal
 * @brief Gets the head pointer of the notification list.
 * @since_tizen 2.3
 * @remarks The specific error code can be obtained using the get_last_result() method. Error codes are described in Exception section.
 *          The returned value should be released using notification_free_list().
 * @param[in] list Notification list handle
 * @return #notification_list_h on success, NULL on failure
 * @exception #NOTIFICATION_ERROR_NONE Success
 * @exception #NOTIFICATION_ERROR_INVALID_PARAMETER Invalid input value
 * @see #notification_list_h
 * @par Sample code:
 * @code
#include <notification.h>
...
{
	notification_list_h noti_list = NULL;
	int noti_err = NOTIFICATION_ERROR_NONE;

	noti_err  = notification_get_grouping_list(NOTIFICATION_TYPE_NONE, -1, &noti_list);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		return;
	}

	noti_list = notification_list_get_head(noti_list);

	notification_free_list(noti_list);
}
 * @endcode
 */
notification_list_h notification_list_get_head(notification_list_h list);

/**
 * @internal
 * @brief Gets the tail pointer to the notification list.
 * @since_tizen 2.3
 * @remarks The specific error code can be obtained using the get_last_result() method. Error codes are described in Exception section.
 *          The returned value should be released using notification_free_list().
 * @param[in] list Notification list handle
 * @return #notification_list_h on success, NULL on failure
 * @exception #NOTIFICATION_ERROR_NONE Success
 * @exception #NOTIFICATION_ERROR_INVALID_PARAMETER Invalid input value
 * @see #notification_list_h
 * @par Sample code:
 * @code
#include <notification.h>
...
{
	notification_list_h noti_list = NULL;
	int noti_err = NOTIFICATION_ERROR_NONE;

	noti_err  = notification_get_grouping_list(NOTIFICATION_TYPE_NONE, -1, &noti_list);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		return;
	}

	noti_list = notification_list_get_tail(noti_list);

	notification_free_list(noti_list);
}
 * @endcode
 */
notification_list_h notification_list_get_tail(notification_list_h list);

/**
 * @internal
 * @brief Gets the previous pointer of the current notification list.
 * @since_tizen 2.3
 * @remarks The specific error code can be obtained using the get_last_result() method. Error codes are described in Exception section.
 *          The returned value should be released using notification_free_list().
 * @param[in] list Notification list handle
 * @return #notification_list_h on success, NULL on failure
 * @exception #NOTIFICATION_ERROR_NONE Success
 * @exception #NOTIFICATION_ERROR_INVALID_PARAMETER Invalid input value
 * @see #notification_list_h
 * @par Sample code:
 * @code
#include <notification.h>
...
{
	notification_list_h noti_list = NULL;
	int noti_err = NOTIFICATION_ERROR_NONE;

	noti_err  = notification_get_grouping_list(NOTIFICATION_TYPE_NONE, -1, &noti_list);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		return;
	}

	noti_list = notification_list_get_prev(noti_list);

	notification_free_list(noti_list);
}
 * @endcode
 */
notification_list_h notification_list_get_prev(notification_list_h list);

/**
 * @internal
 * @brief Gets the next pointer of the current notification list.
 * @since_tizen 2.3
 * @remarks The specific error code can be obtained using the get_last_result() method. Error codes are described in Exception section.
 *          The returned value should be released using notification_free_list().
 * @param[in] list Notification list handle
 * @return #notification_list_h on success, NULL on failure
 * @exception #NOTIFICATION_ERROR_NONE Success
 * @exception #NOTIFICATION_ERROR_INVALID_PARAMETER Invalid input value
 * @see #notification_list_h
 * @par Sample code:
 * @code
#include <notification.h>
...
{
	notification_list_h noti_list = NULL;
	int noti_err;

	noti_err  = notification_get_grouping_list(NOTIFICATION_TYPE_NONE, -1, &noti_list);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		return;
	}

	noti_list = notification_list_get_next(noti_list);

	notification_free_list(noti_list);
}
 * @endcode
 */
notification_list_h notification_list_get_next(notification_list_h list);

/**
 * @internal
 * @brief Gets the notification handle that the list has.
 * @since_tizen 2.3
 * @remarks The specific error code can be obtained using the get_last_result() method. Error codes are described in Exception section.
 *          The returned value should be released using notification_free().
 * @param[in] list Notification list handle
 * @return #notification_h on success, NULL on failure
 * @exception #NOTIFICATION_ERROR_NONE Success
 * @exception #NOTIFICATION_ERROR_INVALID_PARAMETER Invalid input value
 * @see #notification_list_h
 * @see #notification_h
 * @par Sample code:
 * @code
#include <notification.h>
...
{
	notification_h noti = NULL;
	notification_list_h noti_list = NULL;
	int noti_err = NOTIFICATION_ERROR_NONE;

	noti_err  = notification_get_grouping_list(NOTIFICATION_TYPE_NONE, -1, &noti_list);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		return;
	}

	noti = notification_list_get_data(noti_list);

	notification_free(noti);
}
 * @endcode
 */
notification_h notification_list_get_data(notification_list_h list);


/**
 * @internal
 * @brief Gets a number of the notification list.
 * @since_tizen 3.0
 * @param[in] list Notification list handle
 * @return A number of notification list handle on success, 0 on failure
 * @exception #NOTIFICATION_ERROR_NONE Success
 * @exception #NOTIFICATION_ERROR_INVALID_PARAMETER Invalid input value
 * @see #notification_list_h
 * @see #notification_h
 * @par Sample code:
 * @code
include <notification.h>
...
{
	int count = 0;
	notification_list_h noti_list = NULL;
	int noti_err = NOTIFICATION_ERROR_NONE;

	noti_err = notification_get_detail_list(app_id, group_id, priv_id, -1, &noti_list);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		return;
	}

	count = notification_list_get_count(noti_list);
}
 * @endcode
 */
int notification_list_get_count(notification_list_h list);

/**
 * @internal
 * @brief Appends notification data to the notification list.
 * @since_tizen 2.3
 * @remarks The specific error code can be obtained using the get_last_result() method. Error codes are described in Exception section.
 * @param[in] list Notification list handle
 * @param[in] noti Notification handle
 * @return #notification_h on success, NULL on failure
 * @exception #NOTIFICATION_ERROR_NONE Success
 * @exception #NOTIFICATION_ERROR_INVALID_PARAMETER Invalid input value
 * @exception #NOTIFICATION_ERROR_OUT_OF_MEMORY Out of memory
 * @see #notification_list_h
 * @see #notification_h
 * @par Sample code:
 * @code
#include <notification.h>
...
{
	notification_h noti = NULL;
	notification_list_h noti_list = NULL;
	int noti_err = NOTIFICATION_ERROR_NONE;

	noti = notification_new(NOTIFICATION_TYPE_NOTI, NOTIFICATION_GROUP_ID_NONE, NOTIFICATION_PRIV_ID_NONE);
	if (noti == NULL) {
		return;
	}

	noti_list = notification_list_append(noti_list, noti);
}
 * @endcode
 */
notification_list_h notification_list_append(notification_list_h list,
					     notification_h noti);

/**
 * @internal
 * @brief Removes notification data from the notification list.
 * @since_tizen 2.3
 * @remarks The specific error code can be obtained using the get_last_result() method. Error codes are described in Exception section.
 * @param[in] list The notification list handle
 * @param[in] noti The notification handle
 * @return #notification_h on success,
 *         otherwise @c NULL on failure
 * @exception #NOTIFICATION_ERROR_NONE Success
 * @exception #NOTIFICATION_ERROR_INVALID_PARAMETER Invalid input value
 * @see #notification_list_h
 * @see #notification_h
 * @par Sample code:
 * @code
#include <notification.h>
...
{
	notification_h noti = NULL;
	notification_list_h noti_list = NULL;
	int noti_err = NOTIFICATION_ERROR_NONE;

	...

	noti_list = notification_list_remove(noti_list, noti);

}
 * @endcode
 */
notification_list_h notification_list_remove(notification_list_h list,
			notification_h noti);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif
#endif /* __NOTIFICATION_LIST_H__ */

