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

#ifndef __NOTIFICATION_DEBUG_H__
#define __NOTIFICATION_DEBUG_H__

#define LOG_TAG "NOTIFICATION"
#include <dlog.h>

#ifndef EXPORT_API
#define EXPORT_API __attribute__ ((visibility("default")))
#endif

#ifndef _DLOG_H_
#include <stdio.h>

#define DBG(fmt , args...) \
	do { \
		printf("[D][%s : %d] "fmt"\n", __func__, __LINE__, ##args); \
	} while (0)

#define INFO(fmt , args...) \
	do { \
		printf("[I][%s : %d] "fmt"\n", __func__, __LINE__, ##args); \
	} while (0)

#define WARN(fmt , args...) \
	do { \
		printf("[W][%s : %d] "fmt"\n", __func__, __LINE__, ##args); \
	} while (0)

#define ERR(fmt , args...) \
	do { \
		printf("[E][%s : %d] "fmt"\n", __func__, __LINE__, ##args); \
	} while (0)

#else /* _DLOG_H_ */

#define DBG(fmt , args...) \
	do { \
		SECURE_LOGD("[%s : %d] " fmt "\n", __func__, __LINE__, ##args); \
	} while (0)

#define INFO(fmt , args...) \
	do { \
		SECURE_LOGI("[%s : %d] " fmt "\n", __func__, __LINE__, ##args); \
	} while (0)

#define WARN(fmt , args...) \
	do { \
		SECURE_LOGW("[%s : %d] " fmt "\n", __func__, __LINE__, ##args); \
	} while (0)

#define ERR(fmt , args...) \
	do { \
		SECURE_LOGE("[%s : %d] " fmt "\n", __func__, __LINE__, ##args); \
	} while (0)

#endif /* _DLOG_H_ */

#endif /* __NOTIFICATION_DEBUG_H__ */

