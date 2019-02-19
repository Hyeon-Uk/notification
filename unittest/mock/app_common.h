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

#ifndef MOCK_APP_COMMON_H_
#define MOCK_APP_COMMON_H_

#include "mock.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	APP_ERROR_NONE = 0, /**< Successful */
	APP_ERROR_INVALID_PARAMETER = -22, /**< Invalid parameter */
	APP_ERROR_OUT_OF_MEMORY = -12, /**< Out of memory */
	APP_ERROR_INVALID_CONTEXT = -0x01100000 | 0x01, /**< Invalid application context */
	APP_ERROR_NO_SUCH_FILE = -2, /**< No such file or directory */
	APP_ERROR_NOT_SUPPORTED = -3, /**< Not supported (Since 4.0) */
	APP_ERROR_ALREADY_RUNNING = -4, /**< Application is already running */
	APP_ERROR_PERMISSION_DENIED = -5, /**< Permission denied @if MOBILE (Since 2.3.1) @endif */
} app_error_e;

DECLARE_FAKE_VALUE_FUNC(int, app_get_name, char**);

#ifdef __cplusplus
}
#endif
#endif  /* MOCK_APP_COMMON_H_ */
