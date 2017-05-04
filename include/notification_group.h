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

#ifndef __NOTIFICATION_GROUP_H__
#define __NOTIFICATION_GROUP_H__

#include <notification.h>

NOTIFICATION_DEPRECATED_API int notification_group_set_title(
		const char *pkgname, int group_id, const char *title,
		const char *loc_title,
		notification_count_display_type_e count_display);

NOTIFICATION_DEPRECATED_API int notification_group_get_title(
		const char *pkgname, int group_id, char **ret_title,
		char **ret_loc_title,
		notification_count_display_type_e *count_display);

NOTIFICATION_DEPRECATED_API int notification_group_set_content(
		const char *pkgname, int group_id, const char *content,
		const char *loc_content,
		notification_count_display_type_e count_display);

NOTIFICATION_DEPRECATED_API int notification_group_get_content(
		const char *pkgname, int group_id, char **ret_content,
		char **ret_loc_content,
		notification_count_display_type_e *count_display);

NOTIFICATION_DEPRECATED_API int notification_group_set_badge(
		const char *pkgname, int group_id, int count);

NOTIFICATION_DEPRECATED_API int notification_group_get_badge(
		const char *pkgname, int group_id, int *count);

#endif /* __NOTIFICATION_GROUP_H__ */

