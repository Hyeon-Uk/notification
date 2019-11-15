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

#ifndef MOCK_TZPLATFORM_CONFIG_H_
#define MOCK_TZPLATFORM_CONFIG_H_

#include "mock.h"

#ifdef __cplusplus
extern "C" {
#endif

enum tzplatform_variable {
	USER = 0,
	SYSTEM = 1,
};

DECLARE_FAKE_VALUE_FUNC(const char*, tzplatform_getenv, enum tzplatform_variable);

#ifdef __cplusplus
}
#endif
#endif  /* MOCK_TZPLATFORM_CONFIG_H_ */
