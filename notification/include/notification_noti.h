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

#ifndef __NOTIFICATION_NOTI_H__
#define __NOTIFICATION_NOTI_H__

#include <sys/types.h>
#include <glib.h>

#include <notification.h>
#include <notification_list.h>

#ifdef __cplusplus
extern "C" {
#endif

#define TAG_TIME "TIME"
#define TAG_TYPE_INVALID -1
#define TAG_TYPE_TIME 1

struct _notification_deleted_list_info {
	int priv_id;
	char *app_id;
};

typedef struct _notification_deleted_list_info notification_deleted_list_info_s;

int notification_noti_set_tag(const char *tag, char *value, char *buf, int buf_len);

char *notification_noti_strip_tag(const char *tagged_str);

int notification_noti_get_tag_type(const char *tagged_str);

int notification_noti_insert(notification_h noti);

int notification_noti_update(notification_h noti);

int notification_noti_delete_all(notification_type_e type, const char *app_id,
		int *deleted_num, int **deleted_list, uid_t uid);

int notification_noti_get_by_priv_id(notification_h noti, int priv_id);
int notification_noti_get_by_tag(notification_h noti, char *app_id, char* tag, uid_t uid);

int notification_noti_delete_by_priv_id(const char *app_id, int priv_id);
int notification_noti_delete_by_priv_id_get_changes(const char *app_id, int priv_id,
						    int *num_changes, uid_t uid);

int notification_noti_delete_by_display_applist(int display_applist,
		int *deleted_num,
		notification_deleted_list_info_s **deleted_list,
		uid_t uid);

int notification_noti_get_count(notification_type_e type,
		const char *app_id,
		int group_id, int priv_id,
		int *count, uid_t uid);

int notification_noti_get_all_count(notification_type_e type, int *count, uid_t uid);

int notification_noti_get_grouping_list(notification_type_e type,
		int page_number,
		int count_per_page,
		notification_list_h *list,
		int *list_count,
		uid_t uid);

int notification_noti_get_detail_list(const char *app_id,
		int group_id,
		int priv_id, int count,
		notification_list_h *list,
		uid_t uid);

int notification_noti_check_tag(notification_h noti);
int notification_noti_check_count_for_template(notification_h noti, int *count);

int notification_noti_add_template(notification_h noti, char *template_name);
int notification_noti_get_package_template(notification_h noti, char *app_id, char *template_name);
int notification_noti_delete_template(const char *pkg_id);

void notification_noti_init_data(void);
int notification_noti_check_limit(notification_h noti, uid_t uid, GList **list);

/* for channel */
int notification_noti_get_channel(const char *app_id,
		const char *channel_name, int *blockable, int *is_blocked);
int notification_noti_insert_channel(const char *app_id,
		const char *channel_name, int blockable, int is_blocked);
int notification_noti_delete_channel(const char *app_id,
		const char *channel_name);
int notification_noti_update_channel(const char *app_id,
		const char *channel_name, int blockable, int is_blocked);
int notification_noti_get_channel_list(const char *app_id,
		GList **channel_list);

#ifdef __cplusplus
}
#endif
#endif /* __NOTIFICATION_NOTI_H__ */

