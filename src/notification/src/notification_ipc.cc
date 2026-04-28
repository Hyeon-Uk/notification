/*
 * Copyright (c) 2000 - 2025 Samsung Electronics Co., Ltd. All rights reserved.
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
#define _GNU_SOURCE
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cerrno>
#include <cstdarg>
#include <bundle_internal.h>

#include <notification_ipc.h>
#include <notification_type.h>
#include <notification_private.h>
#include <notification_debug.h>
#include <notification_internal.h>

#include <gio/gio.h>
#include <gio/gunixfdlist.h>

/*
 * N8-A.2 (Stage 8): migrated from C to C++.  The two EXPORT_API entries
 * (`notification_ipc_make_gvariant_from_noti`,
 * `notification_ipc_make_noti_from_gvariant`) are advertised with C
 * linkage in notification_ipc.h, so the bodies sit inside an extern "C"
 * block here.  Static helpers (`_dup_string`,
 * `_create_bundle_from_bundle_raw`, `_variant_to_int_dict`,
 * `_variant_dict_lookup`) live in the same block.
 *
 * `<stdio.h>`/`<string.h>`/`<stdlib.h>`/`<errno.h>` swap to the C++
 * `<cstdio>`/`<cstring>`/`<cstdlib>`/`<cerrno>`; `<cstdarg>` is required
 * for the va_list dance in `_variant_dict_lookup`.  The implicit
 * `void *` return from `g_hash_table_lookup` now uses an explicit
 * `static_cast<GVariant *>` to satisfy C++'s stricter conversion rules.
 */

#define ERR_BUFFER_SIZE		1024

extern "C" {

/*!
 * utility functions creating notification packet
 */
static inline char *_dup_string(const char *string)
{
	char *ret;
	char err_buf[ERR_BUFFER_SIZE];

	if (string == NULL || string[0] == '\0')
		return NULL;

	ret = strdup(string);
	if (!ret)
		ERR("Failed to strdup[%s]",
				strerror_r(errno, err_buf, sizeof(err_buf)));

	return ret;
}

static inline bundle *_create_bundle_from_bundle_raw(bundle_raw *string)
{
	if (string == NULL || string[0] == '\0')
		return NULL;

	return bundle_decode(string, strlen((char *)string));
}

/* LCOV_EXCL_START */
EXPORT_API GVariant *notification_ipc_make_gvariant_from_noti(notification_h noti, bool translate)
{
	WARN("make gvariant from noti");
	/* N8 NULL-safety pass2: the body below dereferences noti->type as
	 * the very first statement after this guard. The mirror function
	 * notification_ipc_make_noti_from_gvariant() already rejects NULL
	 * with INVALID_PARAMETER (see line ~397); that contract was never
	 * applied here. Returning NULL matches the existing
	 * "marshal-failure -> NULL" pattern downstream callers already
	 * branch on (g_variant builders return NULL on OOM too). */
	if (noti == NULL) {
		ERR("Invalid parameter: noti == NULL");
		return NULL;
	}
	int i = 0;
	int b_encode_len = 0;
	bundle_raw *args = NULL;
	bundle_raw *group_args = NULL;
	bundle_raw *b_image_path = NULL;
	bundle_raw *b_priv_image_path = NULL;
	bundle_raw *b_execute_option = NULL;
	bundle_raw *b_service_responding = NULL;
	bundle_raw *b_service_single_launch = NULL;
	bundle_raw *b_service_multi_launch = NULL;
	bundle_raw *b_event_handler[NOTIFICATION_EVENT_TYPE_MAX+1] = {NULL, };
	bundle_raw *b_text = NULL;
	bundle_raw *b_key = NULL;
	bundle_raw *b_format_args = NULL;
	GVariant *body = NULL;
	GVariant *result_body = NULL;
	GVariantBuilder builder;

	if (translate)
		notification_translate_localized_text(noti);

	g_variant_builder_init(&builder, G_VARIANT_TYPE("a{iv}"));
	g_variant_builder_add(&builder, "{iv}", NOTIFICATION_DATA_TYPE_NOTI_TYPE, g_variant_new_int32(noti->type));
	g_variant_builder_add(&builder, "{iv}", NOTIFICATION_DATA_TYPE_LAYOUT, g_variant_new_int32(noti->layout));
	g_variant_builder_add(&builder, "{iv}", NOTIFICATION_DATA_TYPE_GROUP_ID, g_variant_new_int32(noti->group_id));
	g_variant_builder_add(&builder, "{iv}", NOTIFICATION_DATA_TYPE_INTERNAL_GROUP_ID, g_variant_new_int32(noti->internal_group_id));
	g_variant_builder_add(&builder, "{iv}", NOTIFICATION_DATA_TYPE_PRIV_ID, g_variant_new_int32(noti->priv_id));
	g_variant_builder_add(&builder, "{iv}", NOTIFICATION_DATA_TYPE_PKG_ID, g_variant_new_string((const gchar *)noti->pkg_id));
	g_variant_builder_add(&builder, "{iv}", NOTIFICATION_DATA_TYPE_CALLER_APP_ID, g_variant_new_string((const gchar *)noti->caller_app_id));
	g_variant_builder_add(&builder, "{iv}", NOTIFICATION_DATA_TYPE_DISPLAY_APPLIST, g_variant_new_int32(noti->display_applist));

	if (noti->args) {
		bundle_encode(noti->args, (bundle_raw **)&args, &b_encode_len);
		g_variant_builder_add(&builder, "{iv}", NOTIFICATION_DATA_TYPE_ARGS, g_variant_new_string((const gchar *)args));

		if (args)
			bundle_free_encoded_rawdata(&args);
	}

	if (noti->group_args) {
		bundle_encode(noti->group_args, (bundle_raw **)&group_args,
			      &b_encode_len);
		g_variant_builder_add(&builder, "{iv}", NOTIFICATION_DATA_TYPE_GROUP_ARGS, g_variant_new_string((const gchar *)group_args));

		if (group_args)
			bundle_free_encoded_rawdata(&group_args);
	}

	if (noti->b_execute_option) {
		bundle_encode(noti->b_execute_option,
			      (bundle_raw **)&b_execute_option, &b_encode_len);
		g_variant_builder_add(&builder, "{iv}", NOTIFICATION_DATA_TYPE_EXECUTE_OPTION, g_variant_new_string((const gchar *)b_execute_option));

		if (b_execute_option)
			bundle_free_encoded_rawdata(&b_execute_option);
	}

	if (noti->b_service_responding) {
		bundle_encode(noti->b_service_responding,
			      (bundle_raw **)&b_service_responding, &b_encode_len);
		g_variant_builder_add(&builder, "{iv}", NOTIFICATION_DATA_TYPE_SERVICE_RESPONDING, g_variant_new_string((const gchar *)b_service_responding));

		if (b_service_responding)
			bundle_free_encoded_rawdata(&b_service_responding);
	}

	if (noti->b_service_single_launch) {
		bundle_encode(noti->b_service_single_launch,
			      (bundle_raw **)&b_service_single_launch, &b_encode_len);
		g_variant_builder_add(&builder, "{iv}", NOTIFICATION_DATA_TYPE_SERVICE_SINGLE_LAUNCH, g_variant_new_string((const gchar *)b_service_single_launch));

		if (b_service_single_launch)
			bundle_free_encoded_rawdata(&b_service_single_launch);
	}

	if (noti->b_service_multi_launch) {
		bundle_encode(noti->b_service_multi_launch,
			      (bundle_raw **)&b_service_multi_launch, &b_encode_len);
		g_variant_builder_add(&builder, "{iv}", NOTIFICATION_DATA_TYPE_SERVICE_MULTI_LAUNCH, g_variant_new_string((const gchar *)b_service_multi_launch));

		if (b_service_multi_launch)
			bundle_free_encoded_rawdata(&b_service_multi_launch);
	}

	for (i = 0; i <= NOTIFICATION_EVENT_TYPE_MAX; i++) {
		if (noti->b_event_handler[i]) {
			bundle_encode(noti->b_event_handler[i],
					(bundle_raw **)&b_event_handler[i], &b_encode_len);
			g_variant_builder_add(&builder, "{iv}", NOTIFICATION_DATA_TYPE_BUTTON1_EVENT + i, g_variant_new_string((const gchar *)b_event_handler[i]));

			if (b_event_handler[i])
				bundle_free_encoded_rawdata(&b_event_handler[i]);
		}
	}

	if (noti->launch_app_id)
		g_variant_builder_add(&builder, "{iv}", NOTIFICATION_DATA_TYPE_LAUNCH_APP_ID, g_variant_new_string((const gchar *)noti->launch_app_id));

	if (noti->domain != NULL)
		g_variant_builder_add(&builder, "{iv}", NOTIFICATION_DATA_TYPE_DOMAIN, g_variant_new_string((const gchar *)noti->domain));

	if (noti->dir != NULL)
		g_variant_builder_add(&builder, "{iv}", NOTIFICATION_DATA_TYPE_DIR, g_variant_new_string((const gchar *)noti->dir));

	if (noti->b_text) {
		bundle_encode(noti->b_text, (bundle_raw **)&b_text, &b_encode_len);
		g_variant_builder_add(&builder, "{iv}", NOTIFICATION_DATA_TYPE_TEXT, g_variant_new_string((const gchar *)b_text));

		if (b_text)
			bundle_free_encoded_rawdata(&b_text);
	}

	if (noti->b_key) {
		bundle_encode(noti->b_key, (bundle_raw **)&b_key, &b_encode_len);
		g_variant_builder_add(&builder, "{iv}", NOTIFICATION_DATA_TYPE_KEY, g_variant_new_string((const gchar *)b_key));

		if (b_key)
			bundle_free_encoded_rawdata(&b_key);
	}

	if (noti->b_format_args) {
		bundle_encode(noti->b_format_args,
			      (bundle_raw **)&b_format_args, &b_encode_len);
		g_variant_builder_add(&builder, "{iv}", NOTIFICATION_DATA_TYPE_FORMAT_ARGS, g_variant_new_string((const gchar *)b_format_args));

		if (b_format_args)
			bundle_free_encoded_rawdata(&b_format_args);
	}

	if (noti->num_format_args != 0)
		g_variant_builder_add(&builder, "{iv}", NOTIFICATION_DATA_TYPE_NUM_FORMAT_ARGS, g_variant_new_int32(noti->num_format_args));

	if (noti->b_image_path) {
		bundle_encode(noti->b_image_path,
			      (bundle_raw **)&b_image_path, &b_encode_len);
		g_variant_builder_add(&builder, "{iv}", NOTIFICATION_DATA_TYPE_IMAGE_PATH, g_variant_new_string((const gchar *)b_image_path));

		if (b_image_path)
			bundle_free_encoded_rawdata(&b_image_path);
	}

	if (noti->b_priv_image_path) {
		bundle_encode(noti->b_priv_image_path,
			      (bundle_raw **)&b_priv_image_path, &b_encode_len);
		g_variant_builder_add(&builder, "{iv}", NOTIFICATION_DATA_TYPE_PRIV_IMAGE_PATH, g_variant_new_string((const gchar *)b_priv_image_path));

		if (b_priv_image_path)
			bundle_free_encoded_rawdata(&b_priv_image_path);
	}

	g_variant_builder_add(&builder, "{iv}", NOTIFICATION_DATA_TYPE_SOUND_TYPE, g_variant_new_int32(noti->sound_type));

	if (noti->sound_path)
		g_variant_builder_add(&builder, "{iv}", NOTIFICATION_DATA_TYPE_SOUND_PATH, g_variant_new_string((const gchar *)noti->sound_path));

	if (noti->priv_sound_path)
		g_variant_builder_add(&builder, "{iv}", NOTIFICATION_DATA_TYPE_PRIV_SOUND_PATH, g_variant_new_string((const gchar *)noti->priv_sound_path));

	g_variant_builder_add(&builder, "{iv}", NOTIFICATION_DATA_TYPE_VIBRATION_TYPE, g_variant_new_int32(noti->vibration_type));

	if (noti->vibration_path)
		g_variant_builder_add(&builder, "{iv}", NOTIFICATION_DATA_TYPE_VIBRATION_PATH, g_variant_new_string((const gchar *)noti->vibration_path));

	if (noti->priv_vibration_path)
		g_variant_builder_add(&builder, "{iv}", NOTIFICATION_DATA_TYPE_PRIV_VIBRATION_PATH, g_variant_new_string((const gchar *)noti->priv_vibration_path));

	g_variant_builder_add(&builder, "{iv}", NOTIFICATION_DATA_TYPE_LED_OPERATION, g_variant_new_int32(noti->led_operation));

	if (noti->led_argb != 0)
		g_variant_builder_add(&builder, "{iv}", NOTIFICATION_DATA_TYPE_LED_ARGB, g_variant_new_int32(noti->led_argb));

	if (noti->led_on_ms != 0)
		g_variant_builder_add(&builder, "{iv}", NOTIFICATION_DATA_TYPE_LED_ON_MS, g_variant_new_int32(noti->led_on_ms));

	if (noti->led_off_ms != 0)
		g_variant_builder_add(&builder, "{iv}", NOTIFICATION_DATA_TYPE_LED_OFF_MS, g_variant_new_int32(noti->led_off_ms));

	if (noti->time != 0)
		g_variant_builder_add(&builder, "{iv}", NOTIFICATION_DATA_TYPE_TIME, g_variant_new_int32(noti->time));

	if (noti->insert_time != 0)
		g_variant_builder_add(&builder, "{iv}", NOTIFICATION_DATA_TYPE_INSERT_TIME, g_variant_new_int32(noti->insert_time));

	if (noti->flags_for_property != 0)
		g_variant_builder_add(&builder, "{iv}", NOTIFICATION_DATA_TYPE_FLAGS_FOR_PROPERTY, g_variant_new_int32(noti->flags_for_property));

	if (noti->progress_size != 0.0)
		g_variant_builder_add(&builder, "{iv}", NOTIFICATION_DATA_TYPE_PROGRESS_SIZE, g_variant_new_double(noti->progress_size));

	if (noti->progress_percentage != 0.0)
		g_variant_builder_add(&builder, "{iv}", NOTIFICATION_DATA_TYPE_PROGRESS_PERCENTAGE, g_variant_new_double(noti->progress_percentage));

	if (noti->app_icon_path != NULL)
		g_variant_builder_add(&builder, "{iv}", NOTIFICATION_DATA_TYPE_APP_ICON_PATH, g_variant_new_string((const gchar *)noti->app_icon_path));
	if (noti->app_label != NULL)
		g_variant_builder_add(&builder, "{iv}", NOTIFICATION_DATA_TYPE_APP_LABEL, g_variant_new_string((const gchar *)noti->app_label));
	if (noti->temp_title != NULL)
		g_variant_builder_add(&builder, "{iv}", NOTIFICATION_DATA_TYPE_TEMP_TITLE, g_variant_new_string((const gchar *)noti->temp_title));
	if (noti->temp_content != NULL)
		g_variant_builder_add(&builder, "{iv}", NOTIFICATION_DATA_TYPE_TEMP_CONTENT, g_variant_new_string((const gchar *)noti->temp_content));
	if (noti->tag != NULL)
		g_variant_builder_add(&builder, "{iv}", NOTIFICATION_DATA_TYPE_TAG, g_variant_new_string((const gchar *)noti->tag));

	g_variant_builder_add(&builder, "{iv}", NOTIFICATION_DATA_TYPE_ONGOING_FLAG, g_variant_new_int32(noti->ongoing_flag));
	g_variant_builder_add(&builder, "{iv}", NOTIFICATION_DATA_TYPE_ONGOING_VALUE_TYPE, g_variant_new_int32(noti->ongoing_value_type));
	g_variant_builder_add(&builder, "{iv}", NOTIFICATION_DATA_TYPE_ONGOING_CURRENT, g_variant_new_int32(noti->ongoing_current));
	g_variant_builder_add(&builder, "{iv}", NOTIFICATION_DATA_TYPE_ONGOING_DURATION, g_variant_new_int32(noti->ongoing_duration));
	g_variant_builder_add(&builder, "{iv}", NOTIFICATION_DATA_TYPE_AUTO_REMOVE, g_variant_new_int32(noti->auto_remove));
	g_variant_builder_add(&builder, "{iv}", NOTIFICATION_DATA_TYPE_DEFAULT_BUTTON, g_variant_new_int32(noti->default_button_index));
	g_variant_builder_add(&builder, "{iv}", NOTIFICATION_DATA_TYPE_HIDE_TIMEOUT, g_variant_new_int32(noti->hide_timeout));
	g_variant_builder_add(&builder, "{iv}", NOTIFICATION_DATA_TYPE_DELETE_TIMEOUT, g_variant_new_int32(noti->delete_timeout));
	g_variant_builder_add(&builder, "{iv}", NOTIFICATION_DATA_TYPE_TEXT_INPUT_MAX_LENGTH, g_variant_new_int32(noti->text_input_max_length));
	g_variant_builder_add(&builder, "{iv}", NOTIFICATION_DATA_TYPE_EVENT_FLAG, g_variant_new_int32(noti->event_flag));
	g_variant_builder_add(&builder, "{iv}", NOTIFICATION_DATA_TYPE_TRANSLATION, g_variant_new_int32(noti->is_translation));
	g_variant_builder_add(&builder, "{iv}", NOTIFICATION_DATA_TYPE_EXTENSION_IMAGE_SIZE, g_variant_new_int32(noti->extension_image_size));
	g_variant_builder_add(&builder, "{iv}", NOTIFICATION_DATA_TYPE_CHECK_BOX, g_variant_new_int32(noti->check_box));
	g_variant_builder_add(&builder, "{iv}", NOTIFICATION_DATA_TYPE_CHECK_BOX_VALUE, g_variant_new_int32(noti->check_box_value));
	g_variant_builder_add(&builder, "{iv}", NOTIFICATION_DATA_TYPE_UID, g_variant_new_int32(noti->uid));

	if (noti->channel_name != NULL)
		g_variant_builder_add(&builder, "{iv}", NOTIFICATION_DATA_TYPE_CHANNEL_NAME, g_variant_new_string((const gchar *)noti->channel_name));

	result_body = g_variant_builder_end(&builder);
	body = g_variant_new("(v)", result_body);

	return body;
}

static gboolean _variant_to_int_dict(GHashTable **dict, GVariant *variant)
{
	GVariantIter iter;
	int key;
	int *hash_key;
	GVariant *value;

	*dict = g_hash_table_new_full(g_int_hash, g_int_equal, g_free, (GDestroyNotify)g_variant_unref);
	if (*dict == NULL)
		return FALSE;

	g_variant_iter_init(&iter, variant);
	while (g_variant_iter_next(&iter, "{iv}", &key, &value)) {
		hash_key = (int *)calloc(sizeof(int), 1);
		if (hash_key == NULL) {
			/* LCOV_EXCL_START */
			g_hash_table_unref(*dict);
			return FALSE;
			/* LCOV_EXCL_STOP */
		}
		*hash_key = key;
		g_hash_table_insert(*dict, (gpointer)hash_key, value);
	}
	return TRUE;
}

static gboolean _variant_dict_lookup(GHashTable *dict,
		int key,
		const gchar  *format_string,
		...)
{
	GVariant *value;
	va_list ap;

	value = static_cast<GVariant *>(
			g_hash_table_lookup(dict, (gpointer)&key));

	if (value == NULL || !g_variant_check_format_string(value, format_string, FALSE))
		return FALSE;

	va_start(ap, format_string);
	g_variant_get_va(value, format_string, NULL, &ap);
	va_end(ap);

	return TRUE;
}

/*!
 * functions creating notification packet
 */
EXPORT_API int notification_ipc_make_noti_from_gvariant(notification_h noti,
		GVariant *variant) {

	WARN("make noti from GVariant");
	GHashTable *dict;

	int i;
	char *pkg_id = NULL;
	char *caller_app_id = NULL;
	char *launch_app_id = NULL;
	bundle_raw *args = NULL;
	bundle_raw *group_args = NULL;
	bundle_raw *b_execute_option = NULL;
	bundle_raw *b_service_responding = NULL;
	bundle_raw *b_service_single_launch = NULL;
	bundle_raw *b_service_multi_launch = NULL;
	bundle_raw *b_event_handler[NOTIFICATION_EVENT_TYPE_MAX+1] = { NULL, };
	char *domain = NULL;
	char *dir = NULL;
	bundle_raw *b_text = NULL;
	bundle_raw *b_key = NULL;
	bundle_raw *b_format_args = NULL;
	bundle_raw *b_image_path = NULL;
	bundle_raw *b_priv_image_path = NULL;
	char *sound_path = NULL;
	char *priv_sound_path = NULL;
	char *vibration_path = NULL;
	char *priv_vibration_path = NULL;
	char *app_icon_path = NULL;
	char *app_label = NULL;
	char *temp_title = NULL;
	char *temp_content = NULL;
	char *tag = NULL;
	char *channel_name = NULL;

	if (noti == NULL) {
		ERR("Invalid noti NULL");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	if (variant == NULL) {
		ERR("Invalid variant NULL");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	if (!_variant_to_int_dict(&dict, variant))
		return NOTIFICATION_ERROR_OUT_OF_MEMORY;

	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_NOTI_TYPE, "i", &noti->type);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_LAYOUT, "i", &noti->layout);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_GROUP_ID, "i", &noti->group_id);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_INTERNAL_GROUP_ID, "i", &noti->internal_group_id);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_PRIV_ID, "i", &noti->priv_id);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_PKG_ID, "&s", &pkg_id);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_CALLER_APP_ID, "&s", &caller_app_id);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_LAUNCH_APP_ID, "&s", &launch_app_id);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_ARGS, "&s", &args);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_GROUP_ARGS, "&s", &group_args);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_EXECUTE_OPTION, "&s", &b_execute_option);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_SERVICE_RESPONDING, "&s", &b_service_responding);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_SERVICE_SINGLE_LAUNCH, "&s", &b_service_single_launch);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_SERVICE_MULTI_LAUNCH, "&s", &b_service_multi_launch);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_BUTTON1_EVENT, "&s", &b_event_handler[NOTIFICATION_EVENT_TYPE_CLICK_ON_BUTTON_1]);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_BUTTON2_EVENT, "&s", &b_event_handler[NOTIFICATION_EVENT_TYPE_CLICK_ON_BUTTON_2]);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_BUTTON3_EVENT, "&s", &b_event_handler[NOTIFICATION_EVENT_TYPE_CLICK_ON_BUTTON_3]);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_BUTTON4_EVENT, "&s", &b_event_handler[NOTIFICATION_EVENT_TYPE_CLICK_ON_BUTTON_4]);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_BUTTON5_EVENT, "&s", &b_event_handler[NOTIFICATION_EVENT_TYPE_CLICK_ON_BUTTON_5]);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_BUTTON6_EVENT, "&s", &b_event_handler[NOTIFICATION_EVENT_TYPE_CLICK_ON_BUTTON_6]);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_ICON_EVENT, "&s", &b_event_handler[NOTIFICATION_EVENT_TYPE_CLICK_ON_ICON]);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_THUMBNAIL_EVENT, "&s", &b_event_handler[NOTIFICATION_EVENT_TYPE_CLICK_ON_THUMBNAIL]);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_TEXT_INPUT_BUTTON_EVENT, "&s", &b_event_handler[NOTIFICATION_EVENT_TYPE_CLICK_ON_TEXT_INPUT_BUTTON]);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_BUTTON7_EVENT, "&s", &b_event_handler[NOTIFICATION_EVENT_TYPE_CLICK_ON_BUTTON_7]);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_BUTTON8_EVENT, "&s", &b_event_handler[NOTIFICATION_EVENT_TYPE_CLICK_ON_BUTTON_8]);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_BUTTON9_EVENT, "&s", &b_event_handler[NOTIFICATION_EVENT_TYPE_CLICK_ON_BUTTON_9]);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_BUTTON10_EVENT, "&s", &b_event_handler[NOTIFICATION_EVENT_TYPE_CLICK_ON_BUTTON_10]);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_DOMAIN, "&s", &domain);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_DIR, "&s", &dir);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_TEXT, "&s", &b_text);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_KEY, "&s", &b_key);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_FORMAT_ARGS, "&s", &b_format_args);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_NUM_FORMAT_ARGS, "i", &noti->num_format_args);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_IMAGE_PATH, "&s", &b_image_path);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_PRIV_IMAGE_PATH, "&s", &b_priv_image_path);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_SOUND_TYPE, "i", &noti->sound_type);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_SOUND_PATH, "&s", &sound_path);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_PRIV_SOUND_PATH, "&s", &priv_sound_path);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_VIBRATION_TYPE, "i", &noti->vibration_type);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_VIBRATION_PATH, "&s", &vibration_path);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_PRIV_VIBRATION_PATH, "&s", &priv_vibration_path);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_LED_OPERATION, "i", &noti->led_operation);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_LED_ARGB, "i", &noti->led_argb);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_LED_ON_MS, "i", &noti->led_on_ms);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_LED_OFF_MS, "i", &noti->led_off_ms);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_TIME, "i", &noti->time);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_INSERT_TIME, "i", &noti->insert_time);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_FLAGS_FOR_PROPERTY, "i", &noti->flags_for_property);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_DISPLAY_APPLIST, "i", &noti->display_applist);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_PROGRESS_SIZE, "d", &noti->progress_size);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_PROGRESS_PERCENTAGE, "d", &noti->progress_percentage);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_APP_ICON_PATH, "&s", &app_icon_path);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_APP_LABEL, "&s", &app_label);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_TEMP_TITLE, "&s", &temp_title);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_TEMP_CONTENT, "&s", &temp_content);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_TAG, "&s", &tag);
	/*
	 * N3-T6 DS1: ongoing_flag / auto_remove / event_flag / is_translation /
	 * check_box / check_box_value are bitfield members of struct
	 * _notification (see notification_private.h). Taking their address is
	 * not legal C, and writing "i" (4B) into a 1B bool was a latent
	 * out-of-bounds write anyway. Bridge through local ints so
	 * _variant_dict_lookup() sees a stable 4B r-value slot and the bitfield
	 * receives a clean 0/1 write-through.
	 */
	int _bf_ongoing_flag = noti->ongoing_flag;
	int _bf_auto_remove = noti->auto_remove;
	int _bf_event_flag = noti->event_flag;
	int _bf_is_translation = noti->is_translation;
	int _bf_check_box = noti->check_box;
	int _bf_check_box_value = noti->check_box_value;

	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_ONGOING_FLAG, "i", &_bf_ongoing_flag);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_ONGOING_VALUE_TYPE, "i", &noti->ongoing_value_type);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_ONGOING_CURRENT, "i", &noti->ongoing_current);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_ONGOING_DURATION, "i", &noti->ongoing_duration);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_AUTO_REMOVE, "i", &_bf_auto_remove);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_DEFAULT_BUTTON, "i", &noti->default_button_index);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_HIDE_TIMEOUT, "i", &noti->hide_timeout);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_DELETE_TIMEOUT, "i", &noti->delete_timeout);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_TEXT_INPUT_MAX_LENGTH, "i", &noti->text_input_max_length);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_EVENT_FLAG, "i", &_bf_event_flag);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_TRANSLATION, "i", &_bf_is_translation);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_EXTENSION_IMAGE_SIZE, "i", &noti->extension_image_size);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_CHECK_BOX, "i", &_bf_check_box);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_CHECK_BOX_VALUE, "i", &_bf_check_box_value);

	noti->ongoing_flag = _bf_ongoing_flag ? 1u : 0u;
	noti->auto_remove = _bf_auto_remove ? 1u : 0u;
	noti->event_flag = _bf_event_flag ? 1u : 0u;
	noti->is_translation = _bf_is_translation ? 1u : 0u;
	noti->check_box = _bf_check_box ? 1u : 0u;
	noti->check_box_value = _bf_check_box_value ? 1u : 0u;
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_UID, "i", &noti->uid);
	_variant_dict_lookup(dict, NOTIFICATION_DATA_TYPE_CHANNEL_NAME, "&s", &channel_name);

	if (noti->pkg_id)
		free(noti->pkg_id);
	noti->pkg_id = _dup_string(pkg_id);

	if (noti->caller_app_id)
		free(noti->caller_app_id);
	noti->caller_app_id = _dup_string(caller_app_id);

	noti->launch_app_id = _dup_string(launch_app_id);

	noti->args = _create_bundle_from_bundle_raw(args);
	noti->group_args = _create_bundle_from_bundle_raw(group_args);
	noti->b_execute_option = _create_bundle_from_bundle_raw(b_execute_option);
	noti->b_service_responding = _create_bundle_from_bundle_raw(
			b_service_responding);
	noti->b_service_single_launch = _create_bundle_from_bundle_raw(
			b_service_single_launch);
	noti->b_service_multi_launch = _create_bundle_from_bundle_raw(
			b_service_multi_launch);
	for (i = 0; i <= NOTIFICATION_EVENT_TYPE_MAX; i++) {
		noti->b_event_handler[i] = _create_bundle_from_bundle_raw(
				b_event_handler[i]);
	}

	if (noti->domain)
		free(noti->domain);
	noti->domain = _dup_string(domain);

	if (noti->dir)
		free(noti->dir);
	noti->dir = _dup_string(dir);

	noti->b_text = _create_bundle_from_bundle_raw(b_text);
	noti->b_key = _create_bundle_from_bundle_raw(b_key);
	noti->b_format_args = _create_bundle_from_bundle_raw(b_format_args);
	noti->b_image_path = _create_bundle_from_bundle_raw(b_image_path);
	noti->b_priv_image_path = _create_bundle_from_bundle_raw(b_priv_image_path);
	noti->sound_path = _dup_string(sound_path);
	noti->priv_sound_path = _dup_string(priv_sound_path);
	noti->vibration_path = _dup_string(vibration_path);
	noti->priv_vibration_path = _dup_string(priv_vibration_path);
	noti->app_icon_path = _dup_string(app_icon_path);
	noti->app_label = _dup_string(app_label);
	noti->temp_title = _dup_string(temp_title);
	noti->temp_content = _dup_string(temp_content);
	noti->tag = _dup_string(tag);
	noti->channel_name = _dup_string(channel_name);

	g_hash_table_unref(dict);

	return NOTIFICATION_ERROR_NONE;
}
/* LCOV_EXCL_STOP */

}  // extern "C"
