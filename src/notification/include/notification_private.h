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

#ifndef __NOTIFICATION_PRIVATE_H__
#define __NOTIFICATION_PRIVATE_H__
#include <sys/types.h>
#include <notification_internal.h>

#ifdef __cplusplus
#include <string>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifndef EXPORT_API
#define EXPORT_API __attribute__ ((visibility("default")))
#endif

#define SAFE_STRDUP(s) ((s) ? strdup(s) : NULL)
#define SAFE_FREE(s)			\
	do {				\
		if (s) {		\
			free(s);	\
			s = NULL;	\
		}			\
	} while (0)

struct _notification {
	notification_type_e type;
	notification_ly_type_e layout;

	int group_id;		/* Group ID */
	int internal_group_id;	/* Internal Group ID */
	int priv_id;		/* Private ID */

	std::string pkg_id;
	std::string caller_app_id;	/* Caller App ID */
	std::string launch_app_id;	/* Launch App ID */
	bundle *args;		/* Will be removed. */
	bundle *group_args;	/* Will be removed. */

	bundle *b_execute_option;
	bundle *b_service_responding;
	bundle *b_service_single_launch;
	bundle *b_service_multi_launch;

	bundle *b_event_handler[NOTIFICATION_EVENT_TYPE_MAX+1];

	std::string domain;		/* Text domain for localization */
	std::string dir;		/* Text dir for localization */

	bundle *b_text;		/* basic text */
	bundle *b_key;		/* key for localized text */
	bundle *b_format_args;	/* args type and value for format string */
	int num_format_args;	/* number of format string args */

	bundle *b_image_path;	/* image path */
	bundle *b_priv_image_path;	/* resource path for image */

	notification_sound_type_e sound_type;
	std::string sound_path;
	std::string priv_sound_path; /* resource path for sound */
	notification_vibration_type_e vibration_type;
	std::string vibration_path;
	std::string priv_vibration_path; /* resource path for vibration */
	notification_led_op_e led_operation;
	int led_argb;
	int led_on_ms;
	int led_off_ms;

	time_t time;		/* time set by application */
	time_t insert_time;	/* insert time */

	int flags_for_property;	/* property NOTIFICATION_PROP_XXX */
	int display_applist;	/* display app list  NOTIFICATION_DISPLAY_APP_XXX */

	double progress_size;	/* size of progress */
	double progress_percentage;	/* percentage of progress */

	std::string app_icon_path;	/* Temporary stored app icon path from AIL */
	std::string app_label;
	std::string temp_title;
	std::string temp_content;
	std::string tag;
	bool ongoing_flag;
	int ongoing_value_type;
	int ongoing_current;		/* Ongoing current time */
	int ongoing_duration;		/* Ongoing duration time */
	bool auto_remove;
	notification_button_index_e default_button_index;
	int hide_timeout;		/* Time for hide in banner */
	int delete_timeout;		/* Time for delete in view notification */
	int text_input_max_length;
	bool event_flag;
	bool is_translation;
	int extension_image_size;

	bool check_box;				/* check box flag */
	bool check_box_value;		/* check box value */

	uid_t uid;

	std::string channel_name;			/* channel name to take care show state */
};

typedef enum notification_data_type {
	NOTIFICATION_DATA_TYPE_NOTI_TYPE = 1,
	NOTIFICATION_DATA_TYPE_LAYOUT,
	NOTIFICATION_DATA_TYPE_GROUP_ID,
	NOTIFICATION_DATA_TYPE_INTERNAL_GROUP_ID,
	NOTIFICATION_DATA_TYPE_PRIV_ID,
	NOTIFICATION_DATA_TYPE_PKG_ID,
	NOTIFICATION_DATA_TYPE_CALLER_APP_ID,
	NOTIFICATION_DATA_TYPE_LAUNCH_APP_ID,
	NOTIFICATION_DATA_TYPE_ARGS,
	NOTIFICATION_DATA_TYPE_GROUP_ARGS,
	NOTIFICATION_DATA_TYPE_EXECUTE_OPTION,
	NOTIFICATION_DATA_TYPE_SERVICE_RESPONDING,
	NOTIFICATION_DATA_TYPE_SERVICE_SINGLE_LAUNCH,
	NOTIFICATION_DATA_TYPE_SERVICE_MULTI_LAUNCH,
	NOTIFICATION_DATA_TYPE_BUTTON1_EVENT,
	NOTIFICATION_DATA_TYPE_BUTTON2_EVENT,
	NOTIFICATION_DATA_TYPE_BUTTON3_EVENT,
	NOTIFICATION_DATA_TYPE_BUTTON4_EVENT,
	NOTIFICATION_DATA_TYPE_BUTTON5_EVENT,
	NOTIFICATION_DATA_TYPE_BUTTON6_EVENT,
	NOTIFICATION_DATA_TYPE_ICON_EVENT,
	NOTIFICATION_DATA_TYPE_THUMBNAIL_EVENT,
	NOTIFICATION_DATA_TYPE_TEXT_INPUT_BUTTON_EVENT,
	NOTIFICATION_DATA_TYPE_BUTTON7_EVENT,
	NOTIFICATION_DATA_TYPE_BUTTON8_EVENT,
	NOTIFICATION_DATA_TYPE_BUTTON9_EVENT,
	NOTIFICATION_DATA_TYPE_BUTTON10_EVENT,
	NOTIFICATION_DATA_TYPE_DOMAIN,
	NOTIFICATION_DATA_TYPE_DIR,
	NOTIFICATION_DATA_TYPE_TEXT,
	NOTIFICATION_DATA_TYPE_KEY,
	NOTIFICATION_DATA_TYPE_FORMAT_ARGS,
	NOTIFICATION_DATA_TYPE_NUM_FORMAT_ARGS,
	NOTIFICATION_DATA_TYPE_IMAGE_PATH,
	NOTIFICATION_DATA_TYPE_PRIV_IMAGE_PATH,
	NOTIFICATION_DATA_TYPE_SOUND_TYPE,
	NOTIFICATION_DATA_TYPE_SOUND_PATH,
	NOTIFICATION_DATA_TYPE_PRIV_SOUND_PATH,
	NOTIFICATION_DATA_TYPE_VIBRATION_TYPE,
	NOTIFICATION_DATA_TYPE_VIBRATION_PATH,
	NOTIFICATION_DATA_TYPE_PRIV_VIBRATION_PATH,
	NOTIFICATION_DATA_TYPE_LED_OPERATION,
	NOTIFICATION_DATA_TYPE_LED_ARGB,
	NOTIFICATION_DATA_TYPE_LED_ON_MS,
	NOTIFICATION_DATA_TYPE_LED_OFF_MS,
	NOTIFICATION_DATA_TYPE_TIME,
	NOTIFICATION_DATA_TYPE_INSERT_TIME,
	NOTIFICATION_DATA_TYPE_FLAGS_FOR_PROPERTY,
	NOTIFICATION_DATA_TYPE_DISPLAY_APPLIST,
	NOTIFICATION_DATA_TYPE_PROGRESS_SIZE,
	NOTIFICATION_DATA_TYPE_PROGRESS_PERCENTAGE,
	NOTIFICATION_DATA_TYPE_APP_ICON_PATH,
	NOTIFICATION_DATA_TYPE_APP_LABEL,
	NOTIFICATION_DATA_TYPE_TEMP_TITLE,
	NOTIFICATION_DATA_TYPE_TEMP_CONTENT,
	NOTIFICATION_DATA_TYPE_TAG,
	NOTIFICATION_DATA_TYPE_ONGOING_FLAG,
	NOTIFICATION_DATA_TYPE_ONGOING_VALUE_TYPE,
	NOTIFICATION_DATA_TYPE_ONGOING_CURRENT,
	NOTIFICATION_DATA_TYPE_ONGOING_DURATION,
	NOTIFICATION_DATA_TYPE_AUTO_REMOVE,
	NOTIFICATION_DATA_TYPE_DEFAULT_BUTTON,
	NOTIFICATION_DATA_TYPE_HIDE_TIMEOUT,
	NOTIFICATION_DATA_TYPE_DELETE_TIMEOUT,
	NOTIFICATION_DATA_TYPE_TEXT_INPUT_MAX_LENGTH,
	NOTIFICATION_DATA_TYPE_EVENT_FLAG,
	NOTIFICATION_DATA_TYPE_TRANSLATION,
	NOTIFICATION_DATA_TYPE_EXTENSION_IMAGE_SIZE,
	NOTIFICATION_DATA_TYPE_CHECK_BOX,
	NOTIFICATION_DATA_TYPE_CHECK_BOX_VALUE,
	NOTIFICATION_DATA_TYPE_UID,
	NOTIFICATION_DATA_TYPE_CHANNEL_NAME,
} notification_data_type_e;

void notification_call_changed_cb_for_uid(notification_op *op_list, int op_num, uid_t uid);
void notification_call_dnd_changed_cb_for_uid(int do_not_disturb, uid_t uid);
void notification_call_event_handler_cb(notification_h noti, int event_type);
void notification_delete_event_handler_cb(int priv_id);
char *notification_get_app_id_by_pid(int pid);

void notification_call_disturb_cb(void);

/* for channel */
int notification_channel_create_with_info(const char *app_id, const char *channel_name,
		int blockable, int is_blocked, notification_channel_h *channel);

#ifdef __cplusplus
}
#endif
#endif /* __NOTIFICATION_PRIVATE_H__ */

