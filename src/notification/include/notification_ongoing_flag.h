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

#ifndef __NOTIFICATION_ONGOING_FLAG_H__
#define __NOTIFICATION_ONGOING_FLAG_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <notification.h>

/**
 * @brief Sets the ongoing flag of the notification
 * @since_tizen 2.4
 * @param[in] noti  The notification handle
 * @param[in] ongoing_flag The ongoing flag
 * @return #NOTIFICATION_ERROR_NONE on success,
 *         otherwise any other value on failure
 * @retval #NOTIFICATION_ERROR_NONE         Success
 * @retval #NOTIFICATION_ERROR_INVALID_PARAMETER Invalid input value
 * @par Sample code:
 * @code
#include <notification.h>
...
{
	notification_h noti = NULL;
	int noti_err = NOTIFICATION_ERROR_NONE;
	bundle *b = NULL;

	noti = notification_create(NOTIFICATION_TYPE_NOTI);
	if (noti == NULL) {
		return;
	}

	noti_err  = notification_set_ongoing_flag(noti, true);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		notification_free(noti);
		return;
	}
}
 * @endcode
 */
int notification_set_ongoing_flag(notification_h noti, bool ongoing_flag);

/**
 * @brief Gets the ongoing flag of the notification
 * @since_tizen 2.4
 * @param[in]  noti  The notification handle
 * @param[out] ongoing_flag The ongoing flag
 * @return #NOTIFICATION_ERROR_NONE on success,
 *         otherwise any other value on failure
 * @retval #NOTIFICATION_ERROR_NONE         Success
 * @retval #NOTIFICATION_ERROR_INVALID_PARAMETER Invalid input value
 * @par Sample code:
 * @code
#include <notification.h>
...
{
	notification_h noti = NULL;
	int noti_err = NOTIFICATION_ERROR_NONE;
	bool ongoing_flag = 0;

	noti_err  = notification_get_ongoing_flag(noti, &ongoing_flag);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		return;
	}
}
 * @endcode
 */
int notification_get_ongoing_flag(notification_h noti, bool *ongoing_flag);

#ifdef __cplusplus
}
#endif
#endif /* __NOTIFICATION_ONGOING_H__ */

