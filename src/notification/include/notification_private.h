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

/*
 * N3-T6 DS1 — struct _notification layout is reordered for packing
 * efficiency. Rules:
 *   1. pointer block first (8B-aligned on LP64, all same size → no gaps)
 *   2. int / enum block next (4B each)
 *   3. 8B members (time_t, double) after the int block
 *   4. bool flags merged into a single 32-bit bitfield word at the tail
 *
 * The original layout interleaved bool / int / ptr / time_t, forcing the
 * compiler to insert padding around every width transition. Each of the 6
 * bools (ongoing_flag, auto_remove, event_flag, is_translation, check_box,
 * check_box_value) consumed a 1B slot surrounded by 3-7B of padding — 8B
 * net per bool before this change.
 *
 * After packing the struct measures ~440B on x86_64 (down from 536B, -96B
 * / -18%). The b_event_handler[13] array alone contributes 104B so the
 * floor is ~380B; this layout is at the practical minimum for the current
 * field set.
 *
 * Call-site constraints:
 *   - `&noti->ongoing_flag` (and the 5 other bool fields) is forbidden:
 *     bitfield members are not addressable. Two sites took the address
 *     before N3-T6:
 *       src/notification/src/notification_ipc.c  _variant_dict_lookup()
 *       src/notification/src/notification_internal_tidl.c rpc_port_proxy_*
 *     Both were rewritten to bridge through a local stack variable of the
 *     legacy type ("i" int or bool) and assign into the bitfield on
 *     success. See those TUs for the pattern.
 *   - Reads and writes as plain r-values (`noti->ongoing_flag = x`,
 *     `if (noti->check_box)`) keep working identically; the bitfield
 *     presents as `unsigned int`.
 *
 * ABI: notification_private.h is EXCLUDED from install (see
 * src/notification/CMakeLists.txt PATTERN "*_private.h"). No external
 * consumer sees sizeof(struct _notification) so the layout change is
 * ABI-safe. All internal TUs (inside libnotification.so +
 * libnotification-ex.so + the unittest binary) are compiled against the
 * same header in one translation unit of the sources tree, so every
 * access observes the same layout.
 */
struct _notification {
	/* ---- pointer block (8B each on LP64) ---- */
	char *pkg_id;
	char *caller_app_id;	/* Caller App ID */
	char *launch_app_id;	/* Launch App ID */
	bundle *args;		/* Will be removed. */
	bundle *group_args;	/* Will be removed. */

	bundle *b_execute_option;
	bundle *b_service_responding;
	bundle *b_service_single_launch;
	bundle *b_service_multi_launch;

	bundle *b_event_handler[NOTIFICATION_EVENT_TYPE_MAX+1];

	char *domain;		/* Text domain for localization */
	char *dir;		/* Text dir for localization */

	bundle *b_text;		/* basic text */
	bundle *b_key;		/* key for localized text */
	bundle *b_format_args;	/* args type and value for format string */

	bundle *b_image_path;	/* image path */
	bundle *b_priv_image_path;	/* resource path for image */

	char *sound_path;
	char *priv_sound_path; /* resource path for sound */
	char *vibration_path;
	char *priv_vibration_path; /* resource path for vibration */

	char *app_icon_path;	/* Temporary stored app icon path from AIL */
	char *app_label;
	char *temp_title;
	char *temp_content;
	char *tag;
	char *channel_name;			/* channel name to take care show state */

	/* ---- int / enum block (4B each) ---- */
	notification_type_e type;
	notification_ly_type_e layout;

	int group_id;		/* Group ID */
	int internal_group_id;	/* Internal Group ID */
	int priv_id;		/* Private ID */

	int num_format_args;	/* number of format string args */

	notification_sound_type_e sound_type;
	notification_vibration_type_e vibration_type;
	notification_led_op_e led_operation;
	int led_argb;
	int led_on_ms;
	int led_off_ms;

	int flags_for_property;	/* property NOTIFICATION_PROP_XXX */
	int display_applist;	/* display app list  NOTIFICATION_DISPLAY_APP_XXX */

	int ongoing_value_type;
	int ongoing_current;		/* Ongoing current time */
	int ongoing_duration;		/* Ongoing duration time */

	notification_button_index_e default_button_index;
	int hide_timeout;		/* Time for hide in banner */
	int delete_timeout;		/* Time for delete in view notification */
	int text_input_max_length;
	int extension_image_size;

	uid_t uid;		/* glibc uid_t == uint32_t on every arch notification ships for */

	/* ---- DS1 bitfield (single 32-bit word; see notes above) ---- */
	unsigned int ongoing_flag : 1;
	unsigned int auto_remove : 1;
	unsigned int event_flag : 1;
	unsigned int is_translation : 1;
	unsigned int check_box : 1;	/* check box flag */
	unsigned int check_box_value : 1;	/* check box value */
	unsigned int reserved_flags : 26;

	/* ---- 8B members (time_t is 8B on x86_64/aarch64; double is 8B) ---- */
	time_t time;		/* time set by application */
	time_t insert_time;	/* insert time */

	double progress_size;	/* size of progress */
	double progress_percentage;	/* percentage of progress */
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

/*
 * N3-T3 C3: per-process aul_getuid() cache.
 *
 * aul_getuid() is called ~40 times across the client-side APIs (every
 * public function that delegates to the *_for_uid variant re-asks libaul
 * for the same effective uid). The answer never changes during the life
 * of the process, yet the call traverses a libaul state-machine and
 * occasionally touches an internal fd, so the aggregate cost is real.
 *
 * notification_cached_uid() memoises the first result in a file-static
 * inside notification_internal.c. Threading: uid_t writes are atomic on
 * all ABIs notification ships for (x86_64 / aarch64 / armv7l — all 4-byte
 * aligned 32-bit values), the idempotent re-read of the real aul_getuid()
 * from a racing thread returns the same value, so no lock is required.
 *
 * Scope note: this helper MUST NOT be used from notification_setting_service.c
 * (the daemon) — setting-service callbacks run on behalf of arbitrary
 * caller uids and must keep the multi-user-aware direct aul_getuid()
 * path. Use it only from the client-side TUs (notification.c,
 * notification_internal.c, notification_list.c, notification_setting.c,
 * notification_tidl.c, shared_file_common.c).
 *
 * Visibility: EXPORT_API because the unittest binary (linked with
 * -fvisibility=default and gc-sections off, but still subject to the
 * production .so's -fvisibility=hidden default) calls these helpers
 * from test_cached_uid.cc. Adding EXPORT_API keeps them in the dynamic
 * symbol table. The header itself is excluded from install (see
 * src/notification/CMakeLists.txt PATTERN "*_private.h" EXCLUDE), so
 * these are still internal symbols from the public ABI perspective.
 */
EXPORT_API uid_t notification_cached_uid(void);

/* Test-only: drop the cached uid so a subsequent call re-invokes
 * aul_getuid(). Declared here (behind the private header) so the
 * unittest binary can reset between TEST_F cases without exposing the
 * hook through the public API. */
EXPORT_API void notification_cached_uid_reset_for_test(void);

#ifdef __cplusplus
}
#endif
#endif /* __NOTIFICATION_PRIVATE_H__ */

