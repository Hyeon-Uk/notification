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

#ifndef __NOTIFICATION_SETTING_SERVICE_H__
#define __NOTIFICATION_SETTING_SERVICE_H__

#include <sys/types.h>
#include <notification.h>

#ifdef __cplusplus
extern "C" {
#endif

int notification_setting_db_set(const char *pkgname, const char *property, const char *value);
int notification_setting_db_get(const char *pkgname, const char *property, char **value);

int notification_setting_db_update(const char *package_name, const char *app_id, int allow_to_notify, int do_not_disturb_except, int visibility_class,
				   int pop_up_notification, int lock_screen_content_level, uid_t uid);
int notification_setting_db_update_system_setting(int do_not_disturb, int visibility_class,
					int dnd_schedule_enabled, int dnd_schedule_day,
					int dnd_start_hour, int dnd_start_min,
					int dnd_end_hour, int dnd_end_min,
					int lock_screen_content_level, uid_t uid);
int notification_setting_db_update_do_not_disturb(int do_not_disturb, uid_t uid);

int noti_setting_service_get_setting_by_app_id(const char *app_id, notification_setting_h *setting, uid_t uid);
int noti_setting_get_setting_array(notification_setting_h *setting_array, int *count, uid_t uid);
int noti_system_setting_load_system_setting(notification_system_setting_h *system_setting, uid_t uid);
int noti_system_setting_get_do_not_disturb(int *do_not_disturb, uid_t uid);
int notification_system_setting_get_dnd_schedule_enabled_uid(uid_t **uids, int *count);
int notification_get_dnd_and_allow_to_notify(const char *app_id, int *do_not_disturb, int *do_not_disturb_except, int *allow_to_notify, uid_t uid);
int notification_system_setting_load_dnd_allow_exception(dnd_allow_exception_h *dnd_allow_exception, int *count, uid_t uid);
int notification_system_setting_update_dnd_allow_exception(int type, int value, uid_t uid);
int notification_setting_db_update_app_disabled(const char *app_id, bool value, uid_t uid);
int notification_setting_db_update_pkg_disabled(const char *pkg_id, bool value, uid_t uid);

#ifdef __cplusplus
}
#endif
#endif /* __NOTIFICATION_SETTING_SERVICE_H__ */

