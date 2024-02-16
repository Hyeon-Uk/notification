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

#ifndef __NOTIFICATION_TEXT_DOMAIN_H__
#define __NOTIFICATION_TEXT_DOMAIN_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Sets the text domain to localize the notification.
 * @since_tizen 2.3
 * @param[in] noti   The notification handle
 * @param[in] domain The text domain
 * @param[in] dir    The text dir
 * @return #NOTIFICATION_ERROR_NONE on success,
 *         otherwise any other value on failure
 * @retval #NOTIFICATION_ERROR_NONE         Success
 * @retval #NOTIFICATION_ERROR_INVALID_PARAMETER Invalid parameter
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

	noti_err  = notification_set_text_domain(noti, PACKAGE, LOCALEDIR);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		notification_free(noti);
		return;
	}
}
 * @endcode
 */
int notification_set_text_domain(notification_h noti,
						  const char *domain,
						  const char *dir);

/**
 * @brief Gets the text domain from the notification handle.
 * @since_tizen 2.3
 * @remarks Do not free returned domain and dir. They are freed when notification_free() or notification_free_list() is called.
 * @param[in]  noti   The notification handle
 * @param[out] domain The domain
 * @param[out] dir    The locale dir
 * @return #NOTIFICATION_ERROR_NONE on success,
 *         otherwise any other value on failure
 * @retval #NOTIFICATION_ERROR_NONE         Success
 * @retval #NOTIFICATION_ERROR_INVALID_PARAMETER Invalid parameter
 * @par Sample code:
 * @code
#include <notification.h>
...
{
	notification_h noti = NULL;
	int noti_err = NOTIFICATION_ERROR_NONE;
	char *domain = NULL;
	char *dir = NULL;

	noti_err  = notification_get_text_domain(noti, &domain, &dir);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		return;
	}
}
 * @endcode
 */
int notification_get_text_domain(notification_h noti,
						  char **domain,
						  char **dir);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __NOTIFICATION_TEXT_DOMAIN_H__ */

