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

#include <stdlib.h>

#include <tizen.h>

#include <notification.h>
#include <notification_list.h>
#include <notification_noti.h>
#include <notification_debug.h>
#include <notification_private.h>
#include <notification_ipc.h>


struct _notification_list {
	notification_list_h prev;
	notification_list_h next;

	notification_h noti;
};

notification_list_h _notification_list_create(void)
{
	notification_list_h list = NULL;

	list = (notification_list_h) malloc(sizeof(struct _notification_list));
	if (list == NULL) {
		/* LCOV_EXCL_START */
		NOTIFICATION_ERR("NO MEMORY");
		return NULL;
		/* LCOV_EXCL_STOP */
	}

	list->prev = NULL;
	list->next = NULL;

	list->noti = NULL;

	return list;
}

EXPORT_API notification_list_h notification_list_get_head(notification_list_h list)
{
	notification_list_h cur_list = NULL;

	if (list == NULL) {
		NOTIFICATION_ERR("INVALID DATA : list == NULL");
		set_last_result(NOTIFICATION_ERROR_INVALID_PARAMETER);
		return NULL;
	}

	cur_list = list;

	while (cur_list->prev != NULL)
		cur_list = cur_list->prev;

	set_last_result(NOTIFICATION_ERROR_NONE);
	return cur_list;
}

EXPORT_API notification_list_h notification_list_get_tail(notification_list_h list)
{
	notification_list_h cur_list = NULL;

	if (list == NULL) {
		NOTIFICATION_ERR("INVALID DATA : list == NULL");
		set_last_result(NOTIFICATION_ERROR_INVALID_PARAMETER);
		return NULL;
	}

	cur_list = list;

	while (cur_list->next != NULL)
		cur_list = cur_list->next;

	set_last_result(NOTIFICATION_ERROR_NONE);
	return cur_list;
}

EXPORT_API notification_list_h notification_list_get_prev(notification_list_h list)
{
	notification_list_h cur_list = NULL;

	if (list == NULL) {
		NOTIFICATION_ERR("INVALID DATA : list == NULL");
		set_last_result(NOTIFICATION_ERROR_INVALID_PARAMETER);
		return NULL;
	}

	cur_list = list;

	set_last_result(NOTIFICATION_ERROR_NONE);
	return cur_list->prev;
}

EXPORT_API notification_list_h notification_list_get_next(notification_list_h list)
{
	notification_list_h cur_list = NULL;

	if (list == NULL) {
		NOTIFICATION_ERR("INVALID DATA : list == NULL");
		set_last_result(NOTIFICATION_ERROR_INVALID_PARAMETER);
		return NULL;
	}

	cur_list = list;

	set_last_result(NOTIFICATION_ERROR_NONE);
	return cur_list->next;
}

EXPORT_API notification_h notification_list_get_data(notification_list_h list)
{
	notification_list_h cur_list = NULL;

	if (list == NULL) {
		NOTIFICATION_ERR("INVALID DATA : list == NULL");
		set_last_result(NOTIFICATION_ERROR_INVALID_PARAMETER);
		return NULL;
	}

	cur_list = list;

	set_last_result(NOTIFICATION_ERROR_NONE);
	return cur_list->noti;
}

EXPORT_API int notification_list_get_count(notification_list_h list)
{
	int count = 0;
	notification_list_h cur_list = NULL;

	if (list == NULL) {
		NOTIFICATION_ERR("INVALID DATA : list == NULL");
		set_last_result(NOTIFICATION_ERROR_INVALID_PARAMETER);
		return 0;
	}

	cur_list = notification_list_get_head(list);

	while (cur_list != NULL) {
		count++;
		cur_list = cur_list->next;
	}

	set_last_result(NOTIFICATION_ERROR_NONE);
	return count;
}

EXPORT_API notification_list_h notification_list_append(notification_list_h list,
		notification_h noti)
{
	notification_list_h new_list = NULL;
	notification_list_h cur_list = NULL;

	if (noti == NULL) {
		NOTIFICATION_ERR("INVALID DATA : data == NULL");
		set_last_result(NOTIFICATION_ERROR_INVALID_PARAMETER);
		return NULL;
	}

	if (list != NULL) {
		cur_list = notification_list_get_tail(list);

		new_list = _notification_list_create();
		if (new_list == NULL) {
			/* LCOV_EXCL_START */
			NOTIFICATION_ERR("NO MEMORY");
			set_last_result(NOTIFICATION_ERROR_OUT_OF_MEMORY);
			return NULL;
			/* LCOV_EXCL_STOP */
		}

		cur_list->next = new_list;
		new_list->prev = cur_list;

		new_list->noti = noti;
	} else {
		cur_list = _notification_list_create();
		if (cur_list == NULL) {
			/* LCOV_EXCL_START */
			NOTIFICATION_ERR("NO MEMORY");
			set_last_result(NOTIFICATION_ERROR_OUT_OF_MEMORY);
			return NULL;
			/* LCOV_EXCL_STOP */
		}

		new_list = cur_list;
		new_list->noti = noti;
	}

	set_last_result(NOTIFICATION_ERROR_NONE);
	return new_list;
}

EXPORT_API notification_list_h notification_list_remove(notification_list_h list,
		notification_h noti)
{
	notification_list_h cur_list = NULL;
	notification_list_h prev_list = NULL;
	notification_list_h next_list = NULL;

	cur_list = notification_list_get_head(list);
	while (cur_list != NULL) {
		if (cur_list->noti == noti) {
			/* remove */
			prev_list = cur_list->prev;
			next_list = cur_list->next;

			if (next_list != NULL) {
				if (prev_list != NULL) {
					prev_list->next = next_list;
					next_list->prev = prev_list;
				} else {
					next_list->prev = NULL;
				}
			} else {
				if (prev_list != NULL)
					prev_list->next = NULL;
			}

			free(cur_list);
			break;
		}

		cur_list = cur_list->next;
	}

	if (prev_list != NULL)
		return notification_list_get_head(prev_list);
	else if (next_list != NULL)
		return next_list;

	return NULL;
}

EXPORT_API int notification_get_list_for_uid(notification_type_e type,
		int count,
		notification_list_h *list, uid_t uid)
{
	notification_list_h get_list = NULL;
	int ret = 0;

	if (list == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	ret = notification_ipc_request_load_noti_grouping_list(type, count, &get_list, uid);
	if (ret != NOTIFICATION_ERROR_NONE)
		return ret;

	if (get_list != NULL)
		*list = notification_list_get_head(get_list);

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_get_list(notification_type_e type,
		int count,
		notification_list_h *list)
{
	return notification_get_list_for_uid(type, count, list, getuid());
}

EXPORT_API int notification_get_detail_list_for_uid(const char *pkgname,
		int group_id,
		int priv_id,
		int count,
		notification_list_h *list,
		uid_t uid)
{
	notification_list_h get_list = NULL;
	int ret = 0;

	if (list == NULL || pkgname == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	ret = notification_ipc_request_load_noti_detail_list(pkgname, group_id, priv_id, count,
			&get_list, uid);
	if (ret != NOTIFICATION_ERROR_NONE)
		return ret;

	if (get_list != NULL)
		*list = notification_list_get_head(get_list);

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_get_detail_list(const char *pkgname,
		int group_id,
		int priv_id,
		int count,
		notification_list_h *list)
{
	return notification_get_detail_list_for_uid(pkgname, group_id,
			priv_id, count, list, getuid());
}

EXPORT_API int notification_free_list(notification_list_h list)
{
	notification_list_h cur_list = NULL;
	notification_h noti = NULL;

	if (list == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	cur_list = notification_list_get_head(list);

	while (cur_list != NULL) {
		noti = notification_list_get_data(cur_list);
		cur_list = notification_list_remove(cur_list, noti);

		notification_free(noti);
	}

	return NOTIFICATION_ERROR_NONE;
}

