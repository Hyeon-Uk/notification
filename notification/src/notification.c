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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <libintl.h>
#include <dbus/dbus.h>

#include <app_control.h>
#include <app_control_internal.h>
#include <package_manager.h>
#include <aul.h>
#include <tizen.h>
#include <pkgmgr-info.h>
#include <pkgmgrinfo_type.h>
#include <vconf.h>

#include <notification.h>
#include <notification_list.h>
#include <notification_debug.h>
#include <notification_private.h>
#include <notification_noti.h>
#include <notification_ongoing.h>
#include <notification_group.h>
#include <notification_ipc.h>
#include <notification_internal.h>
#include <notification_shared_file.h>

static void (*posted_toast_message_cb)(void *data);

#define NOTI_TEXT_RESULT_LEN 4096
#define REGULAR_UID_MIN 5000

static char *_pkg_id = NULL;
static char *_locale_directory = NULL;
static char *_label = NULL;
static int _cb_registered = -1;

char *notification_get_app_id_by_pid(int pid)
{
#define NOTI_APP_ID_LEN 512
	char app_id[NOTI_APP_ID_LEN + 1] = { 0, };
	int ret = AUL_R_OK;
	int fd;
	char *dup_app_id;
	char buf[NOTI_APP_ID_LEN + 1] = { 0, };

	ret = aul_app_get_appid_bypid(pid, app_id, sizeof(app_id));
	if (ret != AUL_R_OK) {

		snprintf(buf, sizeof(buf), "/proc/%d/cmdline", pid);

		fd = open(buf, O_RDONLY);
		if (fd < 0)
			return NULL;

		ret = read(fd, app_id, sizeof(app_id) - 1);
		close(fd);

		if (ret <= 0)
			return NULL;

		app_id[ret] = '\0';
		/*!
		 * \NOTE
		 * "ret" is not able to be larger than "sizeof(app_id) - 1",
		 * if the system is not going wrong.
		 */
	} else {
		if (strlen(app_id) <= 0)
			return NULL;
	}

	dup_app_id = strdup(app_id);
	if (!dup_app_id)
		ERR("Failed to strdup, errno[%d]", errno);

	return dup_app_id;
}

EXPORT_API int notification_set_image(notification_h noti,
						notification_image_type_e type,
						const char *image_path)
{
	bundle *b = NULL;
	bundle *priv_b = NULL;
	char buf_key[32] = { 0, };
	char *ret_val = NULL;
	char *priv_path = NULL;

	if (noti == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	if (type <= NOTIFICATION_IMAGE_TYPE_NONE
	    || type > NOTIFICATION_IMAGE_TYPE_MAX)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	if (noti->b_image_path) {
		b = noti->b_image_path;
		snprintf(buf_key, sizeof(buf_key), "%d", type);

		bundle_get_str(b, buf_key, &ret_val);
		if (ret_val != NULL)
			bundle_del(b, buf_key);

		if (image_path != NULL)
			bundle_add_str(b, buf_key, image_path);
	} else {
		if (image_path == NULL)
			return NOTIFICATION_ERROR_NONE;

		b = bundle_create();
		snprintf(buf_key, sizeof(buf_key), "%d", type);
		bundle_add_str(b, buf_key, image_path);
		noti->b_image_path = b;
	}

	priv_path = notification_check_file_path_is_private(noti->pkg_id, image_path);
	if (noti->b_priv_image_path) {
		priv_b = noti->b_priv_image_path;

		ret_val = NULL;
		bundle_get_str(priv_b, buf_key, &ret_val);
		if (ret_val)
			bundle_del(b, buf_key);

		if (priv_path != NULL)
			bundle_add_str(priv_b, buf_key, priv_path);
	} else if (priv_path != NULL) {
		priv_b = bundle_create();
		bundle_add_str(priv_b, buf_key, priv_path);
		noti->b_priv_image_path = priv_b;
	}

	if (priv_path)
		free(priv_path);

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_get_image(notification_h noti,
						       notification_image_type_e type,
						       char **image_path)
{
	bundle *b = NULL;
	char buf_key[32] = { 0, };
	char *ret_val = NULL;

	if (noti == NULL || image_path == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	if (type <= NOTIFICATION_IMAGE_TYPE_NONE
	    || type > NOTIFICATION_IMAGE_TYPE_MAX)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	if (noti->b_image_path) {
		b = noti->b_image_path;

		snprintf(buf_key, sizeof(buf_key), "%d", type);

		bundle_get_str(b, buf_key, &ret_val);

		*image_path = ret_val;
	} else {
		/* If image path bundle does not exist, image path is NULL */
		*image_path = NULL;
	}

	/* If image path is NULL and type is ICON, icon path set from AIL */
	/* order : user icon -> launch_app_id icon -> caller_app_id icon -> service app icon */
	if (*image_path == NULL && type == NOTIFICATION_IMAGE_TYPE_ICON) {
		if (noti->app_icon_path != NULL)
			*image_path = noti->app_icon_path;
		else
			*image_path = NULL;
	}

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_set_time(notification_h noti, time_t input_time)
{
	if (noti == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	if (input_time == 0)
		noti->time = time(NULL);
	else
		noti->time = input_time;

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_get_time(notification_h noti, time_t *ret_time)
{
	if (noti == NULL || ret_time == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	*ret_time = noti->time;

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_get_insert_time(notification_h noti,
		time_t *ret_time)
{
	if (noti == NULL || ret_time == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	*ret_time = noti->insert_time;

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_set_text(notification_h noti,
		notification_text_type_e type, const char *text,
		const char *key, int args_type, ...)
{
	bundle *b = NULL;
	char buf_key[32] = { 0, };
	char buf_val[NOTI_TEXT_RESULT_LEN] = { 0, };
	char *ret_val = NULL;
	va_list var_args;
	notification_variable_type_e var_type;
	int num_args = 0;
	int noti_err = NOTIFICATION_ERROR_NONE;
	int var_value_int = 0;
	double var_value_double = 0.0;
	char *var_value_string = NULL;
	notification_count_pos_type_e var_value_count =
	    NOTIFICATION_COUNT_POS_NONE;

	if (noti == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	if (type <= NOTIFICATION_TEXT_TYPE_NONE
	    || type > NOTIFICATION_TEXT_TYPE_MAX)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	if (text != NULL) {
		if (noti->b_text != NULL) {
			b = noti->b_text;

			snprintf(buf_key, sizeof(buf_key), "%d", type);

			bundle_get_str(b, buf_key, &ret_val);

			if (ret_val != NULL)
				bundle_del(b, buf_key);

			snprintf(buf_val, sizeof(buf_val), "%s", text);

			bundle_add_str(b, buf_key, buf_val);
		} else {
			b = bundle_create();

			snprintf(buf_key, sizeof(buf_key), "%d", type);
			snprintf(buf_val, sizeof(buf_val), "%s", text);

			bundle_add_str(b, buf_key, buf_val);

			noti->b_text = b;
		}
	} else {
		if (noti->b_text != NULL) {
			b = noti->b_text;

			snprintf(buf_key, sizeof(buf_key), "%d", type);

			bundle_get_str(b, buf_key, &ret_val);
			if (ret_val != NULL)
				bundle_del(b, buf_key);
		}
	}

	if (key != NULL) {
		if (noti->b_key != NULL) {
			b = noti->b_key;

			snprintf(buf_key, sizeof(buf_key), "%d", type);

			bundle_get_str(b, buf_key, &ret_val);
			if (ret_val != NULL)
				bundle_del(b, buf_key);

			snprintf(buf_val, sizeof(buf_val), "%s", key);

			bundle_add_str(b, buf_key, buf_val);
		} else {
			b = bundle_create();

			snprintf(buf_key, sizeof(buf_key), "%d", type);

			snprintf(buf_val, sizeof(buf_val), "%s", key);

			bundle_add_str(b, buf_key, buf_val);

			noti->b_key = b;
		}
	} else {
		if (noti->b_key != NULL) {
			b = noti->b_key;

			snprintf(buf_key, sizeof(buf_key), "%d", type);

			bundle_get_str(b, buf_key, &ret_val);
			if (ret_val != NULL)
				bundle_del(b, buf_key);
		}
	}

	if (noti->b_format_args != NULL)
		b = noti->b_format_args;
	else
		b = bundle_create();

	va_start(var_args, args_type);

	var_type = args_type;
	num_args = 0;

	while (var_type != NOTIFICATION_VARIABLE_TYPE_NONE) {
		/* Type */
		snprintf(buf_key, sizeof(buf_key), "%dtype%d", type, num_args);
		snprintf(buf_val, sizeof(buf_val), "%d", var_type);

		bundle_get_str(b, buf_key, &ret_val);
		if (ret_val != NULL)
			bundle_del(b, buf_key);

		bundle_add_str(b, buf_key, buf_val);

		switch (var_type) {
		case NOTIFICATION_VARIABLE_TYPE_INT:
			var_value_int = va_arg(var_args, int);

			/* Value */
			snprintf(buf_key, sizeof(buf_key), "%dvalue%d", type,
				 num_args);
			snprintf(buf_val, sizeof(buf_val), "%d", var_value_int);

			bundle_get_str(b, buf_key, &ret_val);
			if (ret_val != NULL)
				bundle_del(b, buf_key);

			bundle_add_str(b, buf_key, buf_val);
			break;

		case NOTIFICATION_VARIABLE_TYPE_DOUBLE:
			var_value_double = va_arg(var_args, double);

			/* Value */
			snprintf(buf_key, sizeof(buf_key), "%dvalue%d", type,
				 num_args);
			snprintf(buf_val, sizeof(buf_val), "%.2f",
				 var_value_double);

			bundle_get_str(b, buf_key, &ret_val);
			if (ret_val != NULL)
				bundle_del(b, buf_key);

			bundle_add_str(b, buf_key, buf_val);
			break;

		case NOTIFICATION_VARIABLE_TYPE_STRING:
			var_value_string = va_arg(var_args, char *);

			/* Value */
			snprintf(buf_key, sizeof(buf_key), "%dvalue%d", type,
				 num_args);
			snprintf(buf_val, sizeof(buf_val), "%s",
				 var_value_string);

			bundle_get_str(b, buf_key, &ret_val);
			if (ret_val != NULL)
				bundle_del(b, buf_key);

			bundle_add_str(b, buf_key, buf_val);
			break;

		case NOTIFICATION_VARIABLE_TYPE_COUNT:
			var_value_count =
			    va_arg(var_args, notification_count_pos_type_e);

			/* Value */
			snprintf(buf_key, sizeof(buf_key), "%dvalue%d", type,
				 num_args);
			snprintf(buf_val, sizeof(buf_val), "%d",
				 var_value_count);

			bundle_get_str(b, buf_key, &ret_val);
			if (ret_val != NULL)
				bundle_del(b, buf_key);

			bundle_add_str(b, buf_key, buf_val);
			break;

		default:
			ERR("Invalid variable type. : %d", var_type);
			noti_err = NOTIFICATION_ERROR_INVALID_PARAMETER;
			break;
		}

		num_args++;
		var_type = va_arg(var_args, notification_variable_type_e);
	}
	va_end(var_args);

	if (noti_err == NOTIFICATION_ERROR_NONE)
		noti->num_format_args = num_args;
	else
		noti->num_format_args = 0;

	snprintf(buf_key, sizeof(buf_key), "num%d", type);
	snprintf(buf_val, sizeof(buf_val), "%d", noti->num_format_args);

	bundle_get_str(b, buf_key, &ret_val);
	if (ret_val != NULL)
		bundle_del(b, buf_key);

	bundle_add_str(b, buf_key, buf_val);

	noti->b_format_args = b;

	return noti_err;
}

EXPORT_API int notification_get_text(notification_h noti,
					notification_text_type_e type,
					char **text)
{
	char result_str[NOTI_TEXT_RESULT_LEN] = { 0, };
	char buf_str[32] = { 0, };
	char buf_key[32] = { 0, };
	char *ret_val = NULL;
	char *get_str = NULL;
	char *temp_str = NULL;
	char *translated_str = NULL;
	bundle *b = NULL;
	int num_args = 0;
	int ret_variable_int = 0;
	double ret_variable_double = 0.0;
	notification_text_type_e text_type = NOTIFICATION_TEXT_TYPE_NONE;
	notification_variable_type_e ret_var_type = 0;

	if (noti == NULL || text == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	if (type <= NOTIFICATION_TEXT_TYPE_NONE
	    || type > NOTIFICATION_TEXT_TYPE_MAX)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	text_type = type;

	if (noti->b_key != NULL) {
		b = noti->b_key;

		/* Get text domain and dir */
		/* _notification_get_text_domain(noti); */

		snprintf(buf_key, sizeof(buf_key), "%d", type);
		bundle_get_str(b, buf_key, &ret_val);

		if (noti->is_translation == false) {
			if (ret_val != NULL && noti->domain != NULL
			    && noti->dir != NULL) {
				/* Get application string */
				bindtextdomain(noti->domain, noti->dir);

				get_str = dgettext(noti->domain, ret_val);
				if (get_str == ret_val) /* not found */
					get_str = NULL;
			} else if (ret_val != NULL) {
				/* Get system string */
				get_str = dgettext("sys_string", ret_val);
				if (get_str == ret_val) /* not found */
					get_str = NULL;
			} else {
				get_str = NULL;
			}
		}
	}

	if (get_str == NULL && noti->b_text != NULL) {
		b = noti->b_text;
		snprintf(buf_key, sizeof(buf_key), "%d", type);
		bundle_get_str(b, buf_key, &get_str);
	}

	if (get_str == NULL && ret_val != NULL)
		get_str = ret_val; /* fallback for printing anything */

	if (get_str == NULL) {
		*text = NULL;
		return NOTIFICATION_ERROR_NONE;
	}

	/* Get number format args */
	noti->num_format_args = 0;

	b = noti->b_format_args;
	if (b != NULL) {
		snprintf(buf_key, sizeof(buf_key), "num%d", text_type);

		bundle_get_str(b, buf_key, &ret_val);
		if (ret_val != NULL)
			noti->num_format_args = atoi(ret_val);
	}

	if (noti->num_format_args == 0 || noti->is_translation == true) {
		*text = (char *)get_str;
		return NOTIFICATION_ERROR_NONE;
	}

	/* Check first variable is count, LEFT pos */
	snprintf(buf_key, sizeof(buf_key), "%dtype%d", text_type, num_args);

	bundle_get_str(b, buf_key, &ret_val);
	if (ret_val != NULL)
		ret_var_type = atoi(ret_val);

	if (ret_var_type == NOTIFICATION_VARIABLE_TYPE_COUNT) {
		/* Get var Value */
		snprintf(buf_key, sizeof(buf_key), "%dvalue%d",
			text_type, num_args);

		bundle_get_str(b, buf_key, &ret_val);
		if (ret_val != NULL)
			ret_variable_int = atoi(ret_val);

		if (ret_variable_int == NOTIFICATION_COUNT_POS_LEFT) {
			notification_get_count(noti->type,
					noti->caller_app_id,
					noti->group_id,
					noti->priv_id,
					&ret_variable_int);
			snprintf(buf_str, sizeof(buf_str), "%d ", ret_variable_int);
			strncat(result_str, buf_str, sizeof(result_str) - strlen(result_str) - 1);

			num_args++;
		}
	}

	/* Check variable IN pos */
	for (temp_str = (char *)get_str; *temp_str != '\0'; temp_str++) {
		if (*temp_str != '%') {
			if (NOTI_TEXT_RESULT_LEN - 1 > strlen(result_str)) {
				strncat(result_str, temp_str, 1);
			} else {
				WARN("The buffer is full");
				break;
			}
		} else {
			if (*(temp_str + 1) == '%') {
				if (NOTI_TEXT_RESULT_LEN - 1 > strlen(result_str)) {
					strncat(result_str, temp_str, 1);
				} else {
					WARN("The buffer is full");
					break;
				}
			} else if (*(temp_str + 1) == 'd') {
				/* Get var Type */
				ret_variable_int = 0;

				snprintf(buf_key, sizeof(buf_key), "%dtype%d",
						text_type, num_args);

				bundle_get_str(b, buf_key, &ret_val);
				if (ret_val != NULL)
					ret_var_type = atoi(ret_val);

				if (ret_var_type ==
					NOTIFICATION_VARIABLE_TYPE_COUNT) {
					/* Get notification count */
					notification_get_count(noti->type,
							noti->caller_app_id,
							noti->group_id,
							noti->priv_id,
							&ret_variable_int);
				} else {
					/* Get var Value */
					snprintf(buf_key, sizeof(buf_key),
						"%dvalue%d", text_type, num_args);

					bundle_get_str(b, buf_key, &ret_val);
					if (ret_val != NULL)
						ret_variable_int = atoi(ret_val);
				}

				snprintf(buf_str, sizeof(buf_str), "%d", ret_variable_int);
				strncat(result_str, buf_str, sizeof(result_str) - strlen(result_str) - 1);

				temp_str++;
				num_args++;
			} else if (*(temp_str + 1) == 's') {
				/* Get var Value */
				snprintf(buf_key, sizeof(buf_key), "%dvalue%d",
						text_type, num_args);

				bundle_get_str(b, buf_key, &ret_val);

				if (ret_val != NULL && noti->domain != NULL
						&& noti->dir != NULL) {
					/* Get application string */
					bindtextdomain(noti->domain, noti->dir);
					translated_str =
						dgettext(noti->domain, ret_val);
					INFO("translated_str[%s]", translated_str);
				} else if (ret_val != NULL) {
					/* Get system string */
					translated_str =
						dgettext("sys_string", ret_val);
					INFO("translated_str[%s]", translated_str);
				} else {
					translated_str = NULL;
				}

				if (translated_str != NULL)
					strncat(result_str, translated_str, sizeof(result_str) - strlen(result_str) - 1);

				temp_str++;
				num_args++;
			} else if (*(temp_str + 1) == 'f') {
				/* Get var Value */
				snprintf(buf_key, sizeof(buf_key), "%dvalue%d",
						text_type, num_args);

				bundle_get_str(b, buf_key, &ret_val);
				if (ret_val != NULL)
					ret_variable_double = atof(ret_val);

				snprintf(buf_str, sizeof(buf_str), "%.2f", ret_variable_double);
				strncat(result_str, buf_str, sizeof(result_str) - strlen(result_str) - 1);

				temp_str++;
				num_args++;
			} else if (*(temp_str + 1) >= '1' && *(temp_str + 1) <= '9') {
				if (*(temp_str + 3) == 'd') {
					/* Get var Type */
					ret_variable_int = 0;

					snprintf(buf_key, sizeof(buf_key),
						 "%dtype%d", text_type,
						 num_args + *(temp_str + 1) - 49);

					bundle_get_str(b, buf_key, &ret_val);
					if (ret_val != NULL)
						ret_var_type = atoi(ret_val);

					if (ret_var_type ==
						NOTIFICATION_VARIABLE_TYPE_COUNT) {
						/* Get notification count */
						notification_get_count(noti->type,
								noti->caller_app_id,
								noti->group_id,
								noti->priv_id,
								&ret_variable_int);
					} else {
						/* Get var Value */
						snprintf(buf_key, sizeof(buf_key),
							"%dvalue%d", text_type,
							num_args + *(temp_str + 1) - 49);

						bundle_get_str(b, buf_key, &ret_val);
						if (ret_val != NULL)
							ret_variable_int = atoi(ret_val);
					}

					snprintf(buf_str, sizeof(buf_str), "%d", ret_variable_int);
					strncat(result_str, buf_str, sizeof(result_str) - strlen(result_str) - 1);

					temp_str += 3;
				} else if (*(temp_str + 3) == 's') {
					/* Get var Value */
					snprintf(buf_key, sizeof(buf_key),
						 "%dvalue%d", text_type,
						 num_args + *(temp_str + 1) - 49);

					bundle_get_str(b, buf_key, &ret_val);
					if (ret_val != NULL)
						strncat(result_str, ret_val, sizeof(result_str) - strlen(result_str) - 1);

					temp_str += 3;
				} else if (*(temp_str + 3) == 'f') {
					/* Get var Value */
					snprintf(buf_key, sizeof(buf_key),
						 "%dvalue%d", text_type,
						 num_args + *(temp_str + 1) - 49);

					bundle_get_str(b, buf_key, &ret_val);
					if (ret_val != NULL)
						ret_variable_double = atof(ret_val);

					snprintf(buf_str, sizeof(buf_str), "%.2f", ret_variable_double);
					strncat(result_str, buf_str, sizeof(result_str) - strlen(result_str) - 1);

					temp_str += 3;
				} else {
					if (NOTI_TEXT_RESULT_LEN - 1 > strlen(result_str)) {
						strncat(result_str, temp_str, 1);
					} else {
						WARN("The buffer is full");
						break;
					}
				}
			} else {
				if (NOTI_TEXT_RESULT_LEN - 1 > strlen(result_str)) {
					strncat(result_str, temp_str, 1);
				} else {
					WARN("The buffer is full");
					break;
				}
			}
		}
	}

	/* Check last variable is count, LEFT pos */
	if (num_args < noti->num_format_args) {
		snprintf(buf_key, sizeof(buf_key), "%dtype%d",
				text_type, num_args);

		bundle_get_str(b, buf_key, &ret_val);
		if (ret_val != NULL)
			ret_var_type = atoi(ret_val);

		if (ret_var_type == NOTIFICATION_VARIABLE_TYPE_COUNT) {
			/* Get var Value */
			snprintf(buf_key, sizeof(buf_key), "%dvalue%d",
					text_type, num_args);

			bundle_get_str(b, buf_key, &ret_val);
			if (ret_val != NULL)
				ret_variable_int = atoi(ret_val);

			if (ret_variable_int == NOTIFICATION_COUNT_POS_RIGHT) {
				notification_get_count(noti->type,
						noti->caller_app_id,
						noti->group_id,
						noti->priv_id,
						&ret_variable_int);
				snprintf(buf_str, sizeof(buf_str), " %d", ret_variable_int);
				strncat(result_str, buf_str, sizeof(result_str) - strlen(result_str) - 1);

				num_args++;
			}
		}
	}

	switch (text_type) {
	case NOTIFICATION_TEXT_TYPE_TITLE:
	case NOTIFICATION_TEXT_TYPE_GROUP_TITLE:
		if (noti->temp_title != NULL)
			free(noti->temp_title);

		noti->temp_title = strdup(result_str);

		*text = noti->temp_title;
		break;
	case NOTIFICATION_TEXT_TYPE_CONTENT:
	case NOTIFICATION_TEXT_TYPE_CONTENT_FOR_DISPLAY_OPTION_IS_OFF:
	case NOTIFICATION_TEXT_TYPE_GROUP_CONTENT:
	case NOTIFICATION_TEXT_TYPE_GROUP_CONTENT_FOR_DISPLAY_OPTION_IS_OFF:
		if (noti->temp_content !=
		    NULL)
			free(noti->temp_content);

		noti->temp_content = strdup(result_str);

		*text = noti->temp_content;
		break;
	default:
		break;
	}

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_set_text_domain(notification_h noti,
							     const char *domain,
							     const char *dir)
{
	if (noti == NULL || domain == NULL || dir == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	if (noti->domain)
		free(noti->domain);

	noti->domain = strdup(domain);

	if (noti->dir)
		free(noti->dir);

	noti->dir = strdup(dir);

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_get_text_domain(notification_h noti,
							     char **domain,
							     char **dir)
{
	if (noti == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	if (domain != NULL && noti->domain != NULL)
		*domain = noti->domain;

	if (dir != NULL && noti->dir != NULL)
		*dir = noti->dir;

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_set_time_to_text(notification_h noti, notification_text_type_e type,
								time_t time)
{
	int ret = NOTIFICATION_ERROR_NONE;
	char buf[256] = { 0, };
	char buf_tag[512] = { 0, };

	if (noti == NULL || time <= 0)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	if (type <= NOTIFICATION_TEXT_TYPE_NONE
	    || type > NOTIFICATION_TEXT_TYPE_MAX)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	snprintf(buf, sizeof(buf), "%lu", time);
	ret = notification_noti_set_tag(TAG_TIME, buf, buf_tag, sizeof(buf_tag));

	if (ret != NOTIFICATION_ERROR_NONE)
		return ret;

	return notification_set_text(noti, type, buf_tag, NULL, NOTIFICATION_VARIABLE_TYPE_NONE);
}

EXPORT_API int notification_get_time_from_text(notification_h noti, notification_text_type_e type,
								time_t *time)
{
	int ret = NOTIFICATION_ERROR_NONE;
	char *ret_text = NULL;
	char *tag_value;

	if (noti == NULL || time == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	if (type <= NOTIFICATION_TEXT_TYPE_NONE
	    || type > NOTIFICATION_TEXT_TYPE_MAX)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	ret = notification_get_text(noti, type, &ret_text);
	if (ret != NOTIFICATION_ERROR_NONE || ret_text == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	if (notification_noti_get_tag_type(ret_text) == TAG_TYPE_INVALID)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	tag_value = notification_noti_strip_tag(ret_text);
	if (tag_value == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	*time = atol(tag_value);
	free(tag_value);

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_set_sound(notification_h noti,
						       notification_sound_type_e type,
						       const char *path)
{
	char *priv_path = NULL;

	if (noti == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	if (type < NOTIFICATION_SOUND_TYPE_NONE
	    || type > NOTIFICATION_SOUND_TYPE_MAX)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	noti->sound_type = type;

	/* Save sound path if user data type */
	if (type == NOTIFICATION_SOUND_TYPE_USER_DATA && path != NULL) {
		if (noti->sound_path != NULL)
			free(noti->sound_path);
		noti->sound_path = strdup(path);

		if (noti->priv_sound_path != NULL) {
			free(noti->priv_sound_path);
			noti->priv_sound_path = NULL;
		}

		priv_path = notification_check_file_path_is_private(noti->pkg_id, path);
		if (priv_path)
			noti->priv_sound_path = priv_path;
	} else {
		if (noti->sound_path != NULL) {
			free(noti->sound_path);
			noti->sound_path = NULL;
		}

		if (noti->priv_sound_path != NULL) {
			free(noti->priv_sound_path);
			noti->priv_sound_path = NULL;
		}

		if (type == NOTIFICATION_SOUND_TYPE_USER_DATA) {
			noti->sound_type = NOTIFICATION_SOUND_TYPE_DEFAULT;
			return NOTIFICATION_ERROR_INVALID_PARAMETER;
		}
	}

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_get_sound(notification_h noti,
						       notification_sound_type_e *type,
						       const char **path)
{
	if (noti == NULL || type == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	*type = noti->sound_type;

	/* Set sound path if user data type */
	if (noti->sound_type == NOTIFICATION_SOUND_TYPE_USER_DATA
	    && path != NULL)
		*path = noti->sound_path;

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_set_vibration(notification_h noti,
							   notification_vibration_type_e type,
							   const char *path)
{
	char *priv_path = NULL;

	if (noti == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	if (type < NOTIFICATION_VIBRATION_TYPE_NONE
	    || type > NOTIFICATION_VIBRATION_TYPE_MAX)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	noti->vibration_type = type;

	/* Save sound path if user data type */
	if (type == NOTIFICATION_VIBRATION_TYPE_USER_DATA && path != NULL) {
		if (noti->vibration_path != NULL) {
			free(noti->vibration_path);
			noti->vibration_path = NULL;
		}
		noti->vibration_path = strdup(path);

		if (noti->priv_vibration_path != NULL) {
			free(noti->priv_vibration_path);
			noti->priv_vibration_path = NULL;
		}
		priv_path = notification_check_file_path_is_private(noti->pkg_id, path);
		if (priv_path)
			noti->priv_vibration_path = priv_path;
	} else {
		if (noti->vibration_path != NULL) {
			free(noti->vibration_path);
			noti->vibration_path = NULL;
		}
		if (noti->priv_vibration_path != NULL) {
			free(noti->priv_vibration_path);
			noti->priv_vibration_path = NULL;
		}

		if (type == NOTIFICATION_VIBRATION_TYPE_USER_DATA) {
			noti->vibration_type = NOTIFICATION_VIBRATION_TYPE_DEFAULT;
			return NOTIFICATION_ERROR_INVALID_PARAMETER;
		}
	}

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_get_vibration(notification_h noti,
							   notification_vibration_type_e *type,
							   const char **path)
{
	if (noti == NULL || type == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	*type = noti->vibration_type;
	/* Set sound path if user data type */
	if (noti->vibration_type == NOTIFICATION_VIBRATION_TYPE_USER_DATA
	    && path != NULL)
		*path = noti->vibration_path;

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_set_led(notification_h noti,
							   notification_led_op_e operation,
							   int led_argb)
{
	if (noti == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	if (operation < NOTIFICATION_LED_OP_OFF
	    || operation > NOTIFICATION_LED_OP_MAX)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	noti->led_operation = operation;

	/* Save led argb if operation is turning on LED with custom color */
	if (operation == NOTIFICATION_LED_OP_ON_CUSTOM_COLOR)
		noti->led_argb = led_argb;

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_get_led(notification_h noti,
							   notification_led_op_e *operation,
							   int *led_argb)
{
	if (noti == NULL || operation == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	*operation = noti->led_operation;

	/* Save led argb if operation is turning on LED with custom color */
	if (noti->led_operation == NOTIFICATION_LED_OP_ON_CUSTOM_COLOR
	    && led_argb != NULL)
		*led_argb = noti->led_argb;

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_set_led_time_period(notification_h noti,
									int on_ms, int off_ms)
{
	if (noti == NULL || on_ms < 0 || off_ms < 0)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	noti->led_on_ms = on_ms;
	noti->led_off_ms = off_ms;

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_get_led_time_period(notification_h noti,
									int *on_ms, int *off_ms)
{
	if (noti == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	if (on_ms)
		*(on_ms) = noti->led_on_ms;
	if (off_ms)
		*(off_ms) = noti->led_off_ms;

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_set_launch_option(notification_h noti,
			notification_launch_option_type type, void *option)
{
	int err = NOTIFICATION_ERROR_NONE;
	int ret = 0;
	bundle *b = NULL;
	app_control_h app_control = option;

	if (noti == NULL || app_control == NULL || type != NOTIFICATION_LAUNCH_OPTION_APP_CONTROL) {
		err = NOTIFICATION_ERROR_INVALID_PARAMETER;
		goto out;
	}

	ret = app_control_export_as_bundle(app_control, &b);
	if (ret != APP_CONTROL_ERROR_NONE) {
		ERR("Failed to convert appcontrol to bundle[%d]", ret);
		err = NOTIFICATION_ERROR_INVALID_PARAMETER;
		goto out;
	}

	err = notification_set_execute_option(noti, NOTIFICATION_EXECUTE_TYPE_SINGLE_LAUNCH, NULL, NULL, b);

out:
	if (b)
		bundle_free(b);

	return err;
}

EXPORT_API int notification_get_launch_option(notification_h noti,
								notification_launch_option_type type, void *option)
{
	int ret = 0;
	bundle *b = NULL;
	app_control_h *app_control = (app_control_h *)option;
	app_control_h app_control_new = NULL;

	if (noti == NULL || app_control == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	if (type != NOTIFICATION_LAUNCH_OPTION_APP_CONTROL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	ret = notification_get_execute_option(noti,
				NOTIFICATION_EXECUTE_TYPE_SINGLE_LAUNCH,
				NULL,
				&b);
	if (ret == NOTIFICATION_ERROR_NONE && b != NULL) {
		ret = app_control_create(&app_control_new);
		if (ret == APP_CONTROL_ERROR_NONE && app_control_new != NULL) {
			ret = app_control_import_from_bundle(app_control_new, b);
			if (ret == APP_CONTROL_ERROR_NONE) {
				*app_control = app_control_new;
			} else {
				/* LCOV_EXCL_START */
				app_control_destroy(app_control_new);
				ERR("Failed to import app control from bundle[%d]", ret);
				return NOTIFICATION_ERROR_IO_ERROR;
				/* LCOV_EXCL_STOP */
			}
		} else {
			/* LCOV_EXCL_START */
			ERR("Failed to create app control[%d]", ret);
			return NOTIFICATION_ERROR_IO_ERROR;
			/* LCOV_EXCL_STOP */
		}
	} else {
		/* LCOV_EXCL_START */
		ERR("Failed to get execute option[%d]", ret);
		return ret;
		/* LCOV_EXCL_STOP */
	}

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_set_event_handler(notification_h noti, notification_event_type_e event_type, app_control_h event_handler)
{
	int err = NOTIFICATION_ERROR_NONE;
	bundle *app_control_bundle = NULL;

	if (noti == NULL) {
		err = NOTIFICATION_ERROR_INVALID_PARAMETER;
		ERR("Invalid notification handle");
		goto out;
	}

	if (event_type < NOTIFICATION_EVENT_TYPE_CLICK_ON_BUTTON_1
		|| event_type > NOTIFICATION_EVENT_TYPE_MAX) {
		ERR("Invalid event type");
		err = NOTIFICATION_ERROR_INVALID_PARAMETER;
		goto out;
	}

	err = app_control_export_as_bundle(event_handler, &app_control_bundle);
	if (err != APP_CONTROL_ERROR_NONE) {
		ERR("Failed to export app_control to bundle[%d]", err);
		goto out;
	}

	if (noti->b_event_handler[event_type] != NULL)
		bundle_free(noti->b_event_handler[event_type]);

	noti->b_event_handler[event_type] = app_control_bundle;

out:
	return err;
}

EXPORT_API int notification_get_event_handler(notification_h noti, notification_event_type_e event_type, app_control_h *event_handler)
{
	int err = NOTIFICATION_ERROR_NONE;
	bundle *b = NULL;
	app_control_h app_control_new = NULL;

	if (noti == NULL || event_handler == NULL) {
		err = NOTIFICATION_ERROR_INVALID_PARAMETER;
		ERR("Invalid handle");
		goto out;
	}

	if (event_type < NOTIFICATION_EVENT_TYPE_CLICK_ON_BUTTON_1
		|| event_type > NOTIFICATION_EVENT_TYPE_MAX) {
		/* LCOV_EXCL_START */
		ERR("Invalid event type");
		err = NOTIFICATION_ERROR_INVALID_PARAMETER;
		goto out;
		/* LCOV_EXCL_STOP */
	}

	b = noti->b_event_handler[event_type];
	if (b == NULL) {
		/* LCOV_EXCL_START */
		ERR("No event handler");
		err = NOTIFICATION_ERROR_NOT_EXIST_ID;
		goto out;
		/* LCOV_EXCL_STOP */
	}

	err = app_control_create(&app_control_new);
	if (err != APP_CONTROL_ERROR_NONE || app_control_new == NULL) {
		/* LCOV_EXCL_START */
		ERR("Failed to create app_control[%d]", err);
		err = NOTIFICATION_ERROR_IO_ERROR;
		goto out;
		/* LCOV_EXCL_STOP */
	}

	err = app_control_import_from_bundle(app_control_new, b);
	if (err == APP_CONTROL_ERROR_NONE) {
		*event_handler = app_control_new;
	} else {
		/* LCOV_EXCL_START */
		app_control_destroy(app_control_new);
		app_control_new = NULL;
		ERR("Failed to import app control from bundle[%d]", err);
		err = NOTIFICATION_ERROR_IO_ERROR;
		goto out;
		/* LCOV_EXCL_STOP */
	}

out:
	if (event_handler)
		*event_handler = app_control_new;

	return err;
}

EXPORT_API int notification_set_property(notification_h noti,
							  int flags)
{
	if (noti == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	noti->flags_for_property = flags;

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_get_property(notification_h noti,
							  int *flags)
{
	if (noti == NULL || flags == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	*flags = noti->flags_for_property;

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_set_display_applist(notification_h noti,
								 int applist)
{
	if (noti == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	if (applist == 0xffffffff) /* 0xffffffff means old NOTIFICATION_DISPLAY_APP_ALL */
		applist = NOTIFICATION_DISPLAY_APP_ALL;

	noti->display_applist = applist;

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_get_display_applist(notification_h noti,
								 int *applist)
{
	if (noti == NULL || applist == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	*applist = noti->display_applist;

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_set_size(notification_h noti,
						      double size)
{
	if (noti == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	noti->progress_size = size;

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_get_size(notification_h noti,
						      double *size)
{
	if (noti == NULL || size == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	*size = noti->progress_size;

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_set_progress(notification_h noti,
							  double percentage)
{
	if (noti == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	noti->progress_percentage = percentage;

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_get_progress(notification_h noti,
							  double *percentage)
{
	if (noti == NULL || percentage == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	*percentage = noti->progress_percentage;

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_get_pkgname(notification_h noti,
							 char **pkgname)
{
	if (noti == NULL || pkgname == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	if (noti->caller_app_id)
		*pkgname = noti->caller_app_id;
	else
		*pkgname = NULL;

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_set_layout(notification_h noti,
		notification_ly_type_e layout)
{
	if (noti == NULL || (layout < NOTIFICATION_LY_NONE || layout > NOTIFICATION_LY_MAX))
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	noti->layout = layout;

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_get_layout(notification_h noti,
		notification_ly_type_e *layout)
{
	if (noti == NULL || layout == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	*layout = noti->layout;

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_get_type(notification_h noti,
						      notification_type_e *type)
{
	if (noti == NULL || type == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	*type = noti->type;

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_post(notification_h noti)
{
	return notification_post_for_uid(noti, notification_get_app_owner_uid());
}

EXPORT_API int notification_update(notification_h noti)
{
	return notification_update_for_uid(noti, notification_get_app_owner_uid());
}

EXPORT_API int notification_delete_all(notification_type_e type)
{
	return notification_delete_all_for_uid(type, notification_get_app_owner_uid());
}

EXPORT_API int notification_delete(notification_h noti)
{
	return notification_delete_for_uid(noti, notification_get_app_owner_uid());
}

static int _notification_get_domain_name(const char *app_id, char **name)
{
	char *name_token = NULL;

	if (app_id == NULL)
		return -1;

	/* com.vendor.name -> name */
	name_token = strrchr(app_id, '.');
	if (name_token == NULL)
		return -1;

	name_token++;

	*name = strdup(name_token);
	if (*name == NULL)
		return -1;

	return 0;
}

static void _language_key_changed_cb (keynode_t *node, void *user_data)
{
	int ret;
	char *app_id = NULL;
	char *label = NULL;
	pkgmgrinfo_appinfo_h appinfo = NULL;

	app_id = notification_get_app_id_by_pid(getpid());
	if (app_id == NULL) {
		ERR("Failed to get app_id");
		goto out;
	}

	ret = pkgmgrinfo_appinfo_get_usr_appinfo(app_id, notification_get_app_owner_uid(), &appinfo);
	if (ret != PMINFO_R_OK || appinfo == NULL) {
		WARN("Failed to get appinfo err[%d] app_id[%s]", ret, app_id);
		goto out;
	}

	ret = pkgmgrinfo_appinfo_get_label(appinfo, &label);
	if (ret != PMINFO_R_OK || label == NULL) {
		WARN("Failed to get app_label [%d]", ret);
		goto out;
	}

	free(_label);
	_label = strdup(label);

out:
	if (appinfo)
		pkgmgrinfo_appinfo_destroy_appinfo(appinfo);

	if (app_id)
		free(app_id);
}

static notification_h _notification_create(notification_type_e type)
{
#define NOTI_PKG_ID_LEN 512
	notification_h noti = NULL;
	package_info_h package_info = NULL;
	pkgmgrinfo_appinfo_h appinfo = NULL;
	char *domain_name = NULL;
	char *app_root_path = NULL;
	char *label = NULL;
	char locale_directory[PATH_MAX] = { 0, }; /* PATH_MAX 4096 */
	char pkg_id[NOTI_PKG_ID_LEN + 1] = { 0, };
	int err = 0;

	if (type <= NOTIFICATION_TYPE_NONE || type > NOTIFICATION_TYPE_MAX) {
		ERR("Invalid notification type[%d]", type);
		set_last_result(NOTIFICATION_ERROR_INVALID_PARAMETER);
		return NULL;
	}

	noti = (notification_h)calloc(1, sizeof(struct _notification));
	if (noti == NULL) {
		/* LCOV_EXCL_START */
		ERR("Failed to alloc memory");
		set_last_result(NOTIFICATION_ERROR_OUT_OF_MEMORY);
		return NULL;
		/* LCOV_EXCL_STOP */
	}

	noti->type = type;

	if (type == NOTIFICATION_TYPE_NOTI)
		noti->layout = NOTIFICATION_LY_NOTI_EVENT_SINGLE;
	else if (type == NOTIFICATION_TYPE_ONGOING)
		noti->layout = NOTIFICATION_LY_ONGOING_PROGRESS;

	noti->group_id = NOTIFICATION_GROUP_ID_NONE;
	noti->sound_type = NOTIFICATION_SOUND_TYPE_NONE;
	noti->vibration_type = NOTIFICATION_VIBRATION_TYPE_NONE;
	noti->led_operation = NOTIFICATION_LED_OP_OFF;
	noti->display_applist = NOTIFICATION_DISPLAY_APP_NOTIFICATION_TRAY |
				NOTIFICATION_DISPLAY_APP_LOCK |
				NOTIFICATION_DISPLAY_APP_TICKER |
				NOTIFICATION_DISPLAY_APP_INDICATOR;
	noti->auto_remove = true;
	noti->caller_app_id = notification_get_app_id_by_pid(getpid());
	if (noti->caller_app_id == NULL) {
		ERR("Failed to get caller_app_id");
		err = -1;
		goto out;
	}

	noti->check_box = false;
	noti->check_box_value = false;
	noti->channel_name = NULL;

	if (notification_get_app_owner_uid() < REGULAR_UID_MIN) {
		noti->pkg_id = strdup(noti->caller_app_id);
		if (noti->pkg_id == NULL)
			err = -1;
	} else {
		if (_pkg_id == NULL) {
			err = aul_app_get_pkgid_bypid(getpid(), pkg_id, sizeof(pkg_id));
			if (err != AUL_R_OK) {
				noti->pkg_id = strdup(noti->caller_app_id);
			} else {
				noti->pkg_id = strdup(pkg_id);
				_pkg_id = strdup(pkg_id);
			}

			if (noti->pkg_id == NULL) {
				err = -1;
				goto out;
			}
		} else {
			noti->pkg_id = strdup(_pkg_id);
		}

		err = _notification_get_domain_name(_pkg_id, &domain_name);
		if (err != 0 || domain_name == NULL) {
			WARN("Failed to get domain_name");
			err = 0;
			/* In the case of the web app,
			the domain can not be obtained. */
			goto out;
		}

		noti->domain = strdup(domain_name);

		if (_locale_directory == NULL) {
			err = package_info_create(_pkg_id, &package_info);
			if (err != PACKAGE_MANAGER_ERROR_NONE || package_info == NULL) {
				/* LCOV_EXCL_START */
				WARN("Failed to create package_info err[%d] pkg_id[%s]",
						err, _pkg_id);
				goto out;
				/* LCOV_EXCL_STOP */
			}

			err = package_info_get_root_path(package_info, &app_root_path);
			if (err != PACKAGE_MANAGER_ERROR_NONE || app_root_path == NULL) {
				/* LCOV_EXCL_START */
				WARN("Failed to get root path err[%d] path[%p]",
						err, app_root_path);
				goto out;
				/* LCOV_EXCL_STOP */
			}

			snprintf(locale_directory, PATH_MAX, "%s/res/locale", app_root_path);
			noti->dir = strdup(locale_directory);
			_locale_directory = strdup(locale_directory);
		} else {
			noti->dir = strdup(_locale_directory);
		}

		if (_cb_registered)
			_cb_registered = vconf_notify_key_changed(VCONFKEY_LANGSET,
					_language_key_changed_cb, NULL);

		if (_label == NULL) {
			err = pkgmgrinfo_appinfo_get_usr_appinfo(noti->caller_app_id,
						notification_get_app_owner_uid(), &appinfo);
			if (err != PMINFO_R_OK || appinfo == NULL) {
				WARN("Failed to get appinfo err[%d] caller_app_id[%s]",
						err, noti->caller_app_id);
				err = 0;
				goto out;
			}

			err = pkgmgrinfo_appinfo_get_label(appinfo, &label);
			if (err != PMINFO_R_OK || label == NULL) {
				WARN("Failed to get app_label err[%d]", err);
				err = 0;
				goto out;
			}

			noti->app_label = strdup(label);
			_label = strdup(label);
		} else {
			noti->app_label = strdup(_label);
		}
	}

out:
	if (domain_name)
		free(domain_name);

	if (app_root_path)
		free(app_root_path);

	if (package_info)
		package_info_destroy(package_info);

	if (appinfo)
		pkgmgrinfo_appinfo_destroy_appinfo(appinfo);

	if (err != 0) {
		notification_free(noti);
		noti = NULL;
		set_last_result(NOTIFICATION_ERROR_IO_ERROR);
	} else {
		set_last_result(NOTIFICATION_ERROR_NONE);
	}

	/*!
	 * \NOTE
	 * Other fields are already initialized with ZERO.
	 */

	return noti;
}

EXPORT_API notification_h notification_create(notification_type_e type)
{
	return _notification_create(type);
}

EXPORT_API notification_h notification_load_by_tag(const char *tag)
{
	return notification_load_by_tag_for_uid(tag, notification_get_app_owner_uid());
}

EXPORT_API int notification_clone(notification_h noti, notification_h *clone)
{
	int i = 0;
	notification_h new_noti = NULL;

	if (noti == NULL || clone == NULL) {
		ERR("Invalid handle");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	new_noti = (notification_h) calloc(1, sizeof(struct _notification));
	if (new_noti == NULL) {
		/* LCOV_EXCL_START */
		ERR("Failed to alloc memory");
		return NOTIFICATION_ERROR_OUT_OF_MEMORY;
		/* LCOV_EXCL_STOP */
	}

	new_noti->type = noti->type;
	new_noti->layout = noti->layout;
	new_noti->group_id = noti->group_id;
	new_noti->internal_group_id = noti->internal_group_id;
	new_noti->priv_id = noti->priv_id;

	if (noti->pkg_id != NULL)
		new_noti->pkg_id = strdup(noti->pkg_id);

	if (noti->caller_app_id != NULL)
		new_noti->caller_app_id = strdup(noti->caller_app_id);

	if (noti->launch_app_id != NULL)
		new_noti->launch_app_id = strdup(noti->launch_app_id);

	if (noti->args != NULL)
		new_noti->args = bundle_dup(noti->args);

	if (noti->group_args != NULL)
		new_noti->group_args = bundle_dup(noti->group_args);

	if (noti->b_execute_option != NULL)
		new_noti->b_execute_option = bundle_dup(noti->b_execute_option);

	if (noti->b_service_responding != NULL)
		new_noti->b_service_responding = bundle_dup(noti->b_service_responding);

	if (noti->b_service_single_launch != NULL)
		new_noti->b_service_single_launch = bundle_dup(noti->b_service_single_launch);

	if (noti->b_service_multi_launch != NULL)
		new_noti->b_service_multi_launch = bundle_dup(noti->b_service_multi_launch);

	for (i = 0; i <= NOTIFICATION_EVENT_TYPE_MAX; i++) {
		if (noti->b_event_handler[i] != NULL)
			new_noti->b_event_handler[i] = bundle_dup(noti->b_event_handler[i]);
	}

	if (noti->domain != NULL)
		new_noti->domain = strdup(noti->domain);

	if (noti->dir != NULL)
		new_noti->dir = strdup(noti->dir);

	if (noti->b_text != NULL)
		new_noti->b_text = bundle_dup(noti->b_text);

	if (noti->b_key != NULL)
		new_noti->b_key = bundle_dup(noti->b_key);

	if (noti->b_format_args != NULL)
		new_noti->b_format_args = bundle_dup(noti->b_format_args);

	new_noti->num_format_args = noti->num_format_args;

	if (noti->b_image_path != NULL)
		new_noti->b_image_path = bundle_dup(noti->b_image_path);

	if (noti->b_priv_image_path != NULL)
		new_noti->b_priv_image_path = bundle_dup(noti->b_priv_image_path);

	new_noti->sound_type = noti->sound_type;

	if (noti->sound_path != NULL)
		new_noti->sound_path = strdup(noti->sound_path);

	if (noti->priv_sound_path != NULL)
		new_noti->priv_sound_path = strdup(noti->priv_sound_path);

	new_noti->vibration_type = noti->vibration_type;

	if (noti->vibration_path != NULL)
		new_noti->vibration_path = strdup(noti->vibration_path);

	if (noti->priv_vibration_path != NULL)
		new_noti->priv_vibration_path = strdup(noti->priv_vibration_path);

	new_noti->led_operation = noti->led_operation;
	new_noti->led_argb = noti->led_argb;
	new_noti->led_on_ms = noti->led_on_ms;
	new_noti->led_off_ms = noti->led_off_ms;
	new_noti->time = noti->time;
	new_noti->insert_time = noti->insert_time;
	new_noti->flags_for_property = noti->flags_for_property;
	new_noti->display_applist = noti->display_applist;
	new_noti->progress_size = noti->progress_size;
	new_noti->progress_percentage = noti->progress_percentage;

	if (noti->tag != NULL)
		new_noti->tag = strdup(noti->tag);

	new_noti->ongoing_flag = noti->ongoing_flag;
	new_noti->ongoing_value_type = noti->ongoing_value_type;
	new_noti->ongoing_current = noti->ongoing_current;
	new_noti->ongoing_duration = noti->ongoing_duration;
	new_noti->auto_remove = noti->auto_remove;
	new_noti->default_button_index = noti->default_button_index;
	new_noti->hide_timeout = noti->hide_timeout;
	new_noti->delete_timeout = noti->delete_timeout;
	new_noti->text_input_max_length = noti->text_input_max_length;
	new_noti->event_flag = noti->event_flag;
	new_noti->is_translation = noti->is_translation;
	new_noti->extension_image_size = noti->extension_image_size;

	if (noti->app_label != NULL)
		new_noti->app_label = strdup(noti->app_label);

	new_noti->uid = noti->uid;

	new_noti->check_box = noti->check_box;
	new_noti->check_box_value = noti->check_box_value;

	*clone = new_noti;

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_free(notification_h noti)
{
	int i = 0;

	if (noti == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	if (noti->pkg_id)
		free(noti->pkg_id);

	if (noti->caller_app_id)
		free(noti->caller_app_id);

	if (noti->launch_app_id)
		free(noti->launch_app_id);

	if (noti->args)
		bundle_free(noti->args);

	if (noti->group_args)
		bundle_free(noti->group_args);

	if (noti->b_execute_option)
		bundle_free(noti->b_execute_option);

	if (noti->b_service_responding)
		bundle_free(noti->b_service_responding);

	if (noti->b_service_single_launch)
		bundle_free(noti->b_service_single_launch);

	if (noti->b_service_multi_launch)
		bundle_free(noti->b_service_multi_launch);

	for (i = 0; i <= NOTIFICATION_EVENT_TYPE_MAX; i++) {
		if (noti->b_event_handler[i] != NULL)
			bundle_free(noti->b_event_handler[i]);
	}

	if (noti->b_image_path)
		bundle_free(noti->b_image_path);

	if (noti->b_priv_image_path)
		bundle_free(noti->b_priv_image_path);

	if (noti->sound_path)
		free(noti->sound_path);

	if (noti->priv_sound_path)
		free(noti->priv_sound_path);

	if (noti->vibration_path)
		free(noti->vibration_path);

	if (noti->priv_vibration_path)
		free(noti->priv_vibration_path);

	if (noti->domain)
		free(noti->domain);

	if (noti->dir)
		free(noti->dir);

	if (noti->b_text)
		bundle_free(noti->b_text);

	if (noti->b_key)
		bundle_free(noti->b_key);

	if (noti->b_format_args)
		bundle_free(noti->b_format_args);

	if (noti->app_icon_path)
		free(noti->app_icon_path);

	if (noti->app_label)
		free(noti->app_label);

	if (noti->temp_title)
		free(noti->temp_title);

	if (noti->temp_content)
		free(noti->temp_content);

	if (noti->tag)
		free(noti->tag);

	if (noti->channel_name)
		free(noti->channel_name);

	free(noti);

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_set_tag(notification_h noti, const char *tag)
{
	if (noti == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	if (tag != NULL) {
		if (noti->tag != NULL)
			free(noti->tag);

		noti->tag = strdup(tag);
	}

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_get_tag(notification_h noti, const char **tag)
{
	if (noti == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	*tag = noti->tag;
	return NOTIFICATION_ERROR_NONE;
}

/* LCOV_EXCL_START */
void notification_call_posted_toast_cb(const char *message)
{
	if (posted_toast_message_cb != NULL)
		posted_toast_message_cb((void *)message);
}
/* LCOV_EXCL_STOP */

EXPORT_API int notification_set_ongoing_flag(notification_h noti, bool ongoing_flag)
{
	if (noti == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	noti->ongoing_flag = ongoing_flag;

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_get_ongoing_flag(notification_h noti, bool *ongoing_flag)
{
	if (noti == NULL || ongoing_flag == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	*ongoing_flag = noti->ongoing_flag;

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_add_button(notification_h noti, notification_button_index_e button_index)
{
	if (noti == NULL ||
		!((button_index >= NOTIFICATION_BUTTON_1 && button_index <= NOTIFICATION_BUTTON_6) ||
		(button_index >= NOTIFICATION_BUTTON_7 && button_index <= NOTIFICATION_BUTTON_10)))
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_remove_button(notification_h noti, notification_button_index_e button_index)
{
	if (noti == NULL ||
		!((button_index >= NOTIFICATION_BUTTON_1 && button_index <= NOTIFICATION_BUTTON_6) ||
		(button_index >= NOTIFICATION_BUTTON_7 && button_index <= NOTIFICATION_BUTTON_10)))
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	if (noti->b_event_handler[button_index - 1]) {
		bundle_free(noti->b_event_handler[button_index - 1]);
		noti->b_event_handler[button_index - 1] = NULL;
	}

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_set_auto_remove(notification_h noti, bool auto_remove)
{
	if (noti == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	noti->auto_remove = auto_remove;

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_get_auto_remove(notification_h noti, bool *auto_remove)
{
	if (noti == NULL || auto_remove == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	*auto_remove = noti->auto_remove;

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_save_as_template(notification_h noti, const char *template_name)
{
	if (noti == NULL || template_name == NULL) {
		ERR("Invalid parameter");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	return notification_ipc_request_save_as_template(noti, template_name);
}

EXPORT_API notification_h notification_create_from_template(const char *template_name)
{
	int ret = 0;
	notification_h noti = NULL;

	if (template_name == NULL) {
		ERR("Invalid parameter");
		set_last_result(NOTIFICATION_ERROR_INVALID_PARAMETER);
		return NULL;
	}

	noti = (notification_h)calloc(1, sizeof(struct _notification));
	if (noti == NULL) {
		/* LCOV_EXCL_START */
		ERR("Failed to alloc memory");
		set_last_result(NOTIFICATION_ERROR_OUT_OF_MEMORY);
		return NULL;
		/* LCOV_EXCL_STOP */
	}

	ret = notification_ipc_request_create_from_template(noti, template_name);

	set_last_result(ret);
	if (ret != NOTIFICATION_ERROR_NONE) {
		notification_free(noti);
		return NULL;
	}

	return noti;
}

EXPORT_API int notification_get_noti_block_state(notification_block_state_e *state)
{
	int ret;
	char *app_id;
	int do_not_disturb;
	int do_not_disturb_except;
	int allow_to_notify;

	if (state == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	app_id = notification_get_app_id_by_pid(getpid());

	ret = notification_ipc_get_noti_block_state(app_id, &do_not_disturb, &do_not_disturb_except, &allow_to_notify, notification_get_app_owner_uid());

	if (ret != NOTIFICATION_ERROR_NONE) {
		if (app_id)
			free(app_id);
		return ret;
	}

	if (allow_to_notify) {
		*state = NOTIFICATION_BLOCK_STATE_ALLOWED;
		if (do_not_disturb && !do_not_disturb_except)
			*state = NOTIFICATION_BLOCK_STATE_DO_NOT_DISTURB;
	} else {
		*state = NOTIFICATION_BLOCK_STATE_BLOCKED;
	}

	if (app_id)
		free(app_id);

	return ret;
}

EXPORT_API int notification_set_text_input(notification_h noti, int text_input_max_length)
{
	if (noti == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	noti->text_input_max_length = text_input_max_length;

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_set_extension_image_size(notification_h noti, int height)
{
	if (noti == NULL || height <= 0)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	noti->extension_image_size = height;

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_get_extension_image_size(notification_h noti, int *height)
{
	if (noti == NULL || height == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	*height = noti->extension_image_size;

	return NOTIFICATION_ERROR_NONE;
}
