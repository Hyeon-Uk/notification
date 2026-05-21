/*
 * Copyright (c) 2026 Samsung Electronics Co., Ltd. All rights reserved.
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

#ifndef __CONFIG_H__
#define __CONFIG_H__

typedef enum {
	TIZEN_PROFILE_UNKNOWN = 0,
	TIZEN_PROFILE_MOBILE = 0x1,
	TIZEN_PROFILE_WEARABLE = 0x2,
	TIZEN_PROFILE_TV = 0x4,
	TIZEN_PROFILE_IVI = 0x8,
	TIZEN_PROFILE_COMMON = 0x10,
} tizen_profile_t;

tizen_profile_t _config_get_tizen_profile(void);

#define TIZEN_FEATURE_ENABLE_NOTI_SETTING                               \
	(!(_config_get_tizen_profile() & (TIZEN_PROFILE_TV)))

#endif /* __CONFIG_H__ */
