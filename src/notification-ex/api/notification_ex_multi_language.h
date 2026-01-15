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

#ifndef __TIZEN_APPFW_NOTIFICATION_EX_MULTI_LANGUAGE_H__
#define __TIZEN_APPFW_NOTIFICATION_EX_MULTI_LANGUAGE_H__

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
 * @brief The notification_ex multi-language item handle.
 * @since_tizen 5.5
 */
typedef void *noti_ex_multi_lang_h;

/**
 * @deprecated Deprecated since 11.0.
 * @brief Creates the multi-language item handle.
 * @since_tizen 5.5
 * @remarks @a handle must be released using noti_ex_multi_lang_destroy().
 * @param[out] handle The multi-language item handle
 * @param[in] msgid The identifier of the message to be translated. One of the identifiers declared in PO files. \n
 *          The message of @a msgid and @a format must contain the same specifiers and in the same order.
 * @param[in] format The string that contains the text to be written. \n
 *          It can optionally contain embedded format specifiers \n
 *          that are replaced by the values specified in subsequent additional arguments and formatted as requested. \n
 *          Valid specifiers are as follows. \n
 *          %d : Signed decimal integer \n
 *          %f : Decimal floating point \n
 *          %s : String of characters
 * @param[in] ... The additional arguments. The values to be used to replace format specifiers in the format string.
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #NOTI_EX_ERROR_OUT_OF_MEMORY out of memory
 * @see #noti_ex_multi_lang_h
 * @see noti_ex_multi_lang_destroy()
 * @par Sample code:
 * @code
#include <notification_ex.h>

{
	int ret;
	noti_ex_multi_lang_h handle = NULL;

	// _IDS_TEST_ : "Hi %s version %f"
	ret = noti_ex_multi_lang_create(&handle, "_IDS_TEST_", "Hi %s version %f", "Tizen", 5.5);
	noti_ex_multi_lang_destroy(handle);
}
 * @endcode
 */
int noti_ex_multi_lang_create(noti_ex_multi_lang_h *handle,
		const char *msgid, const char *format, ...) TIZEN_DEPRECATED_API;

/**
 * @deprecated Deprecated since 11.0.
 * @brief Destroys the multi-language item handle.
 * @since_tizen 5.5
 * @param[in] handle The multi-language item handle
 * @return #NOTI_EX_ERROR_NONE On success, other value on failure
 * @retval #NOTI_EX_ERROR_NONE Success
 * @retval #NOTI_EX_ERROR_INVALID_PARAMETER Invalid parameter
 * @see #noti_ex_multi_lang_h
 * @see noti_ex_multi_lang_create()
 * @par Sample code:
 * @code
#include <notification_ex.h>

{
	int ret;
	noti_ex_multi_lang_h handle = NULL;

	// _IDS_TEST_ : "Hi %s version %f"
	ret = noti_ex_multi_lang_create(&handle, "_IDS_TEST_", "Hi %s version %f", "Tizen", 5.5);
	noti_ex_multi_lang_destroy(handle);
}
 * @endcode
 */
int noti_ex_multi_lang_destroy(noti_ex_multi_lang_h handle) TIZEN_DEPRECATED_API;

/**
 * @}
 */

#ifdef __cplusplus
}
#endif
#endif  /* __TIZEN_APPFW_NOTIFICATION_EX_MULTI_LANGUAGE_H__ */
