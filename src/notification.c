/*
 * Copyright (c) 2000 - 2016 Samsung Electronics Co., Ltd. All rights reserved.
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
#include <dbus/dbus-glib-lowlevel.h>

#include <app.h>
#include <app_internal.h>
#include <app_manager.h>
#include <app_control_internal.h>
#include <package_manager.h>
#include <aul.h>
#include <appsvc.h>
#include <tizen.h>
#include <vconf-keys.h>
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

static void (*posted_toast_message_cb) (void *data);

#define NOTI_TEXT_RESULT_LEN 2048
#define NOTI_PKGNAME_LEN	512

#define REGULAR_UID_MIN 5000

char *notification_get_pkgname_by_pid(void)
{
	char pkgname[NOTI_PKGNAME_LEN + 1] = { 0, };
	int pid = 0, ret = AUL_R_OK;
	int fd;
	char  *dup_pkgname;
	char buf[NOTI_PKGNAME_LEN + 1] = { 0, };

	pid = getpid();

	ret = aul_app_get_pkgname_bypid(pid, pkgname, sizeof(pkgname));
	if (ret != AUL_R_OK) {

		snprintf(buf, sizeof(buf), "/proc/%d/cmdline", pid);

		fd = open(buf, O_RDONLY);
		if (fd < 0)
			return NULL;

		ret = read(fd, pkgname, sizeof(pkgname) - 1);
		close(fd);

		if (ret <= 0)
			return NULL;

		pkgname[ret] = '\0';
		/*!
		 * \NOTE
		 * "ret" is not able to be larger than "sizeof(pkgname) - 1",
		 * if the system is not going wrong.
		 */
	} else {
		if (strlen(pkgname) <= 0)
			return NULL;
	}

	dup_pkgname = strdup(pkgname);
	if (!dup_pkgname)
		NOTIFICATION_ERR("Heap: %d\n", errno);

	return dup_pkgname;
}

EXPORT_API int notification_set_image(notification_h noti,
						       notification_image_type_e type,
						       const char *image_path)
{
	bundle *b = NULL;
	char buf_key[32] = { 0, };
	char *ret_val = NULL;

	if (noti == NULL || image_path == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	if (type <= NOTIFICATION_IMAGE_TYPE_NONE
	    || type >= NOTIFICATION_IMAGE_TYPE_MAX)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	if (noti->b_image_path) {
		b = noti->b_image_path;

		snprintf(buf_key, sizeof(buf_key), "%d", type);

		bundle_get_str(b, buf_key, &ret_val);
		if (ret_val != NULL)
			bundle_del(b, buf_key);

		bundle_add_str(b, buf_key, image_path);
	} else {
		b = bundle_create();

		snprintf(buf_key, sizeof(buf_key), "%d", type);

		bundle_add_str(b, buf_key, image_path);

		noti->b_image_path = b;
	}

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
	    || type >= NOTIFICATION_IMAGE_TYPE_MAX)
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
	/* order : user icon -> launch_pkgname icon -> caller_pkgname icon -> service app icon */
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
	char buf_val[1024] = { 0, };
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
	    || type >= NOTIFICATION_TEXT_TYPE_MAX)
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
			NOTIFICATION_ERR("Error. invalid variable type. : %d",
					 var_type);
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
	bundle *b = NULL;
	char buf_key[32] = { 0, };
	char *ret_val = NULL;
	char *get_str = NULL;
	notification_text_type_e check_type = NOTIFICATION_TEXT_TYPE_NONE;

	char *temp_str = NULL;
	char *translated_str = NULL;
	char result_str[NOTI_TEXT_RESULT_LEN] = { 0, };
	char buf_str[1024] = { 0, };
	int num_args = 0;
	notification_variable_type_e ret_var_type = 0;
	int ret_variable_int = 0;
	double ret_variable_double = 0.0;
	int src_len = 0;
	int max_len = 0;

	if (noti == NULL || text == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	if (type <= NOTIFICATION_TEXT_TYPE_NONE
	    || type >= NOTIFICATION_TEXT_TYPE_MAX)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;


	if (noti->b_key != NULL) {
		b = noti->b_key;

		/* Get text domain and dir */
		/* _notification_get_text_domain(noti); */

		snprintf(buf_key, sizeof(buf_key), "%d", type);

		bundle_get_str(b, buf_key, &ret_val);
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

	if (get_str == NULL && noti->b_text != NULL) {
		b = noti->b_text;

		snprintf(buf_key, sizeof(buf_key), "%d", type);

		bundle_get_str(b, buf_key, &get_str);
	}

	if (get_str == NULL && ret_val != NULL)
		get_str = ret_val; /* fallback for printing anything */

	check_type = type;

	if (get_str != NULL) {
		/* Get number format args */
		b = noti->b_format_args;
		noti->num_format_args = 0;

		if (b != NULL) {
			snprintf(buf_key, sizeof(buf_key), "num%d", check_type);
			bundle_get_str(b, buf_key, &ret_val);
			if (ret_val != NULL)
				noti->num_format_args = atoi(ret_val);
		}

		if (noti->num_format_args == 0) {
			*text = (char *)get_str;
		} else {
			/* Check first variable is count, LEFT pos */
			snprintf(buf_key, sizeof(buf_key), "%dtype%d",
				 check_type, num_args);
			bundle_get_str(b, buf_key, &ret_val);
			if (ret_val != NULL)
				ret_var_type = atoi(ret_val);

			if (ret_var_type == NOTIFICATION_VARIABLE_TYPE_COUNT) {
				/* Get var Value */
				snprintf(buf_key, sizeof(buf_key), "%dvalue%d",
					 check_type, num_args);
				bundle_get_str(b, buf_key, &ret_val);
				if (ret_val != NULL)
					ret_variable_int = atoi(ret_val);

				if (ret_variable_int ==
				    NOTIFICATION_COUNT_POS_LEFT) {
					notification_get_count(noti->type,
							noti->caller_pkgname,
							noti->group_id,
							noti->priv_id,
							&ret_variable_int);
					snprintf(buf_str, sizeof(buf_str),
						 "%d ", ret_variable_int);

					src_len = strlen(result_str);
					max_len = NOTI_TEXT_RESULT_LEN - src_len - 1;

					strncat(result_str, buf_str,
							max_len);
					num_args++;
				}
			}

			/* Check variable IN pos */
			for (temp_str = (char *)get_str; *temp_str != '\0';
			     temp_str++) {
				if (*temp_str != '%') {
					strncat(result_str, temp_str, 1);
				} else {
					if (*(temp_str + 1) == '%') {
						strncat(result_str, temp_str,
							1);
					} else if (*(temp_str + 1) == 'd') {
						/* Get var Type */
						ret_variable_int = 0;

						snprintf(buf_key,
							 sizeof(buf_key),
							 "%dtype%d", check_type,
							 num_args);
						bundle_get_str(b, buf_key, &ret_val);
						if (ret_val != NULL)
							ret_var_type = atoi(ret_val);

						if (ret_var_type ==
						    NOTIFICATION_VARIABLE_TYPE_COUNT) {
							/* Get notification count */
							notification_get_count(noti->type,
									noti->caller_pkgname,
									noti->group_id,
									noti->priv_id,
									&ret_variable_int);
						} else {
							/* Get var Value */
							snprintf(buf_key,
								 sizeof(buf_key),
								 "%dvalue%d",
								 check_type,
								 num_args);
							bundle_get_str(b, buf_key, &ret_val);
							if (ret_val != NULL)
								ret_variable_int = atoi(ret_val);
						}

						snprintf(buf_str,
							 sizeof(buf_str), "%d",
							 ret_variable_int);

						src_len = strlen(result_str);
						max_len = NOTI_TEXT_RESULT_LEN - src_len - 1;

						strncat(result_str, buf_str,
								max_len);

						temp_str++;

						num_args++;
					} else if (*(temp_str + 1) == 's') {
						/* Get var Value */
						snprintf(buf_key,
							 sizeof(buf_key),
							 "%dvalue%d",
							 check_type, num_args);
						bundle_get_str(b, buf_key, &ret_val);

						if (ret_val != NULL && noti->domain != NULL	&& noti->dir != NULL) {
							/* Get application string */
							bindtextdomain(noti->domain, noti->dir);
							translated_str = dgettext(noti->domain, ret_val);
							NOTIFICATION_INFO("translated_str[%s]", translated_str);
						} else if (ret_val != NULL) {
							/* Get system string */
							translated_str = dgettext("sys_string", ret_val);
							NOTIFICATION_INFO("translated_str[%s]", translated_str);
						} else {
							translated_str = NULL;
						}

						if (translated_str != NULL) {
							strncpy(buf_str, translated_str, sizeof(buf_str) - 1);
							src_len = strlen(result_str);
							max_len = NOTI_TEXT_RESULT_LEN - src_len - 1;
							strncat(result_str, buf_str, max_len);
						}
						temp_str++;
						num_args++;
					} else if (*(temp_str + 1) == 'f') {
						/* Get var Value */
						snprintf(buf_key,
							 sizeof(buf_key),
							 "%dvalue%d",
							 check_type, num_args);
						bundle_get_str(b, buf_key, &ret_val);
						if (ret_val != NULL)
							ret_variable_double = atof(ret_val);

						snprintf(buf_str,
							 sizeof(buf_str),
							 "%.2f",
							 ret_variable_double);

						src_len = strlen(result_str);
						max_len = NOTI_TEXT_RESULT_LEN - src_len - 1;
						strncat(result_str, buf_str, max_len);

						temp_str++;
						num_args++;
					} else if (*(temp_str + 1) >= '1' && *(temp_str + 1) <= '9') {
						if (*(temp_str + 3) == 'd') {
							/* Get var Type */
							ret_variable_int = 0;

							snprintf(buf_key,
								 sizeof(buf_key),
								 "%dtype%d", check_type,
								 num_args + *(temp_str + 1) - 49);
							bundle_get_str(b, buf_key, &ret_val);
							if (ret_val != NULL)
								ret_var_type = atoi(ret_val);

							if (ret_var_type ==
							    NOTIFICATION_VARIABLE_TYPE_COUNT) {
								/* Get notification count */
								notification_get_count(noti->type,
										noti->caller_pkgname,
										noti->group_id,
										noti->priv_id,
										&ret_variable_int);
							} else {
								/* Get var Value */
								snprintf(buf_key,
									 sizeof
									 (buf_key),
									 "%dvalue%d",
									 check_type,
									 num_args + *(temp_str + 1) - 49);
								bundle_get_str(b, buf_key, &ret_val);
								if (ret_val != NULL)
									ret_variable_int = atoi(ret_val);

							}

							snprintf(buf_str,
								 sizeof(buf_str), "%d",
								 ret_variable_int);

							src_len = strlen(result_str);
							max_len = NOTI_TEXT_RESULT_LEN - src_len - 1;

							strncat(result_str, buf_str, max_len);

							temp_str += 3;
						} else if (*(temp_str + 3) == 's') {
							/* Get var Value */
							snprintf(buf_key,
								 sizeof(buf_key),
								 "%dvalue%d",
								 check_type, num_args + *(temp_str + 1) - 49);
							bundle_get_str(b, buf_key, &ret_val);

							snprintf(buf_str,
								 sizeof(buf_str), "%s",
								 ret_val);

							src_len = strlen(result_str);
							max_len = NOTI_TEXT_RESULT_LEN - src_len - 1;

							strncat(result_str, buf_str, max_len);

							temp_str += 3;
						} else if (*(temp_str + 3) == 'f') {
							/* Get var Value */
							snprintf(buf_key,
								 sizeof(buf_key),
								 "%dvalue%d",
								 check_type, num_args + *(temp_str + 1) - 49);
							bundle_get_str(b, buf_key, &ret_val);
							if (ret_val != NULL)
								ret_variable_double = atof(ret_val);

							snprintf(buf_str,
								 sizeof(buf_str),
								 "%.2f",
								 ret_variable_double);

							src_len = strlen(result_str);
							max_len = NOTI_TEXT_RESULT_LEN - src_len - 1;

							strncat(result_str, buf_str, max_len);

							temp_str += 3;
						}
					}
				}

			}

			/* Check last variable is count, LEFT pos */
			if (num_args < noti->num_format_args) {
				snprintf(buf_key, sizeof(buf_key), "%dtype%d",
					 check_type, num_args);
				bundle_get_str(b, buf_key, &ret_val);
				if (ret_val != NULL)
					ret_var_type = atoi(ret_val);

				if (ret_var_type ==
				    NOTIFICATION_VARIABLE_TYPE_COUNT) {
					/* Get var Value */
					snprintf(buf_key, sizeof(buf_key),
						 "%dvalue%d", check_type,
						 num_args);
					bundle_get_str(b, buf_key, &ret_val);
					if (ret_val != NULL)
						ret_variable_int = atoi(ret_val);

					if (ret_variable_int ==
					    NOTIFICATION_COUNT_POS_RIGHT) {
						notification_get_count(noti->type,
								noti->caller_pkgname,
								noti->group_id,
								noti->priv_id,
								&ret_variable_int);
						snprintf(buf_str,
							 sizeof(buf_str), " %d",
							 ret_variable_int);

						src_len = strlen(result_str);
						max_len = NOTI_TEXT_RESULT_LEN - src_len - 1;

						strncat(result_str, buf_str, max_len);

						num_args++;
					}

				}
			}

			switch (check_type) {
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

		}

	} else {
		*text = NULL;
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

	if (noti == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	if (time <= 0)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	if (type <= NOTIFICATION_TEXT_TYPE_NONE
	    || type >= NOTIFICATION_TEXT_TYPE_MAX)
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

	if (noti == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	if (time == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	if (type <= NOTIFICATION_TEXT_TYPE_NONE
	    || type >= NOTIFICATION_TEXT_TYPE_MAX)
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
	if (noti == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	if (type < NOTIFICATION_SOUND_TYPE_NONE
	    || type >= NOTIFICATION_SOUND_TYPE_MAX)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	noti->sound_type = type;

	/* Save sound path if user data type */
	if (type == NOTIFICATION_SOUND_TYPE_USER_DATA && path != NULL) {
		if (noti->sound_path != NULL)
			free(noti->sound_path);

		noti->sound_path = strdup(path);
	} else {
		if (noti->sound_path != NULL) {
			free(noti->sound_path);
			noti->sound_path = NULL;
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
	if (noti == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	if (type < NOTIFICATION_VIBRATION_TYPE_NONE
	    || type >= NOTIFICATION_VIBRATION_TYPE_MAX)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	noti->vibration_type = type;

	/* Save sound path if user data type */
	if (type == NOTIFICATION_VIBRATION_TYPE_USER_DATA && path != NULL) {
		if (noti->vibration_path != NULL)
			free(noti->vibration_path);

		noti->vibration_path = strdup(path);
	} else {
		if (noti->vibration_path != NULL) {
			free(noti->vibration_path);
			noti->vibration_path = NULL;
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
	    || operation >= NOTIFICATION_LED_OP_MAX)
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

	if ((ret = app_control_export_as_bundle(app_control, &b)) != APP_CONTROL_ERROR_NONE) {
		NOTIFICATION_ERR("Failed to convert appcontrol to bundle:%d", ret);
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

	if (noti == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	if (app_control == NULL)
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
				app_control_destroy(app_control_new);
				NOTIFICATION_ERR("Failed to import app control from bundle:%d", ret);
				return NOTIFICATION_ERROR_IO_ERROR;
			}
		} else {
			NOTIFICATION_ERR("Failed to create app control:%d", ret);
			return NOTIFICATION_ERROR_IO_ERROR;
		}
	} else {
		NOTIFICATION_ERR("Failed to get execute option:%d", ret);
		return ret;
	}

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_set_event_handler(notification_h noti, notification_event_type_e event_type, app_control_h event_handler)
{
	int err = NOTIFICATION_ERROR_NONE;
	bundle *app_control_bundle = NULL;

	if (noti == NULL) {
		err = NOTIFICATION_ERROR_INVALID_PARAMETER;
		NOTIFICATION_ERR("NOTIFICATION_ERROR_INVALID_PARAMETER");
		goto out;
	}

	if (event_type < NOTIFICATION_EVENT_TYPE_CLICK_ON_BUTTON_1
		|| event_type > NOTIFICATION_EVENT_TYPE_CLICK_ON_THUMBNAIL) {
		NOTIFICATION_ERR("NOTIFICATION_ERROR_INVALID_PARAMETER");
		err = NOTIFICATION_ERROR_INVALID_PARAMETER;
		goto out;
	}

	if ((err = app_control_export_as_bundle(event_handler, &app_control_bundle)) != APP_CONTROL_ERROR_NONE) {
		NOTIFICATION_ERR("app_control_to_bundle failed [%d]", err);
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

	if (noti == NULL) {
		err = NOTIFICATION_ERROR_INVALID_PARAMETER;
		NOTIFICATION_ERR("NOTIFICATION_ERROR_INVALID_PARAMETER");
		goto out;
	}

	if (event_handler == NULL) {
		err = NOTIFICATION_ERROR_INVALID_PARAMETER;
		NOTIFICATION_ERR("NOTIFICATION_ERROR_INVALID_PARAMETER");
		goto out;
	}

	if (event_type < NOTIFICATION_EVENT_TYPE_CLICK_ON_BUTTON_1
		|| event_type > NOTIFICATION_EVENT_TYPE_CLICK_ON_THUMBNAIL) {
		NOTIFICATION_ERR("NOTIFICATION_ERROR_INVALID_PARAMETER");
		err = NOTIFICATION_ERROR_INVALID_PARAMETER;
		goto out;
	}

	b = noti->b_event_handler[event_type];
	if (b == NULL) {
		NOTIFICATION_DBG("No event handler\n");
		err = NOTIFICATION_ERROR_NOT_EXIST_ID;
		goto out;
	}

	err = app_control_create(&app_control_new);
	if (err != APP_CONTROL_ERROR_NONE || app_control_new == NULL) {
		NOTIFICATION_ERR("app_control_create failed [%d]", err);
		err = NOTIFICATION_ERROR_IO_ERROR;
		goto out;
	}

	err = app_control_import_from_bundle(app_control_new, b);
	if (err == APP_CONTROL_ERROR_NONE) {
		*event_handler = app_control_new;
	} else {
		app_control_destroy(app_control_new);
		app_control_new = NULL;
		NOTIFICATION_ERR("Failed to import app control from bundle [%d]", err);
		err = NOTIFICATION_ERROR_IO_ERROR;
		goto out;
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

	if (noti->caller_pkgname)
		*pkgname = noti->caller_pkgname;
	else
		*pkgname = NULL;

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_set_layout(notification_h noti,
		notification_ly_type_e layout)
{
	if (noti == NULL || (layout < NOTIFICATION_LY_NONE || layout >= NOTIFICATION_LY_MAX))
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
	return notification_post_for_uid(noti, getuid());
}

EXPORT_API int notification_update(notification_h noti)
{
	return notification_update_for_uid(noti, getuid());
}

EXPORT_API int notification_delete_all(notification_type_e type)
{
	return notification_delete_all_for_uid(type, getuid());
}

EXPORT_API int notification_delete(notification_h noti)
{
	return notification_delete_for_uid(noti, getuid());
}

static notification_h _notification_create(notification_type_e type)
{
	notification_h noti = NULL;
	package_info_h package_info = NULL;
	char *app_id = NULL;
	char *domain_name = NULL;
	char *app_root_path = NULL;
	char locale_directory[PATH_MAX] = { 0, }; /* PATH_MAX 4096 */
	int err_app_manager = APP_MANAGER_ERROR_NONE;

	if (type <= NOTIFICATION_TYPE_NONE || type >= NOTIFICATION_TYPE_MAX) {
		NOTIFICATION_ERR("INVALID TYPE : %d", type);
		set_last_result(NOTIFICATION_ERROR_INVALID_PARAMETER);
		return NULL;
	}

	noti = (notification_h) calloc(1, sizeof(struct _notification));
	if (noti == NULL) {
		NOTIFICATION_ERR("NO MEMORY : noti == NULL");
		set_last_result(NOTIFICATION_ERROR_OUT_OF_MEMORY);
		return NULL;
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
	noti->display_applist = NOTIFICATION_DISPLAY_APP_NOTIFICATION_TRAY | NOTIFICATION_DISPLAY_APP_TICKER | NOTIFICATION_DISPLAY_APP_INDICATOR;
	noti->auto_remove = true;
	noti->ongoing_flag = false;

	if (getuid() >= REGULAR_UID_MIN) {
		noti->caller_pkgname = notification_get_pkgname_by_pid();

		err_app_manager = app_manager_get_app_id(getpid(), &app_id);
		if (err_app_manager != APP_MANAGER_ERROR_NONE || app_id == NULL) {
			NOTIFICATION_WARN("app_manager_get_app_id failed err[%d] app_id[%p]",
					err_app_manager, app_id);
			goto out;
		}

		/* app name is used as domain name */
		/* domain_name is allocated by app_get_package_app_name */
		err_app_manager = app_get_package_app_name(app_id, &domain_name);

		if (err_app_manager != APP_ERROR_NONE || domain_name == NULL) {
			NOTIFICATION_WARN("app_get_package_app_name failed err[%d] domain_name[%p]",
					err_app_manager, domain_name);
			goto out;
		}


		err_app_manager = package_info_create(noti->caller_pkgname, &package_info);

		if (err_app_manager != PACKAGE_MANAGER_ERROR_NONE || package_info == NULL) {
			NOTIFICATION_WARN("package_info_create failed err[%d] package_info[%p] caller_pkgname[%s]",
					err_app_manager, package_info, noti->caller_pkgname);
			goto out;
		}

		err_app_manager = package_info_get_root_path(package_info, &app_root_path);

		if (err_app_manager != PACKAGE_MANAGER_ERROR_NONE || app_root_path == NULL) {
			NOTIFICATION_WARN("package_info_get_root_path failed err[%d] app_root_path[%p]",
					err_app_manager, app_root_path);
			goto out;
		}

		snprintf(locale_directory, PATH_MAX, "%s/res/locale", app_root_path);

		noti->domain = strdup(domain_name);
		noti->dir    = strdup(locale_directory);
	}

out:
	if (domain_name)
		free(domain_name);

	if (app_id)
		free(app_id);

	if (app_root_path)
		free(app_root_path);

	if (package_info)
		package_info_destroy(package_info);

	/*!
	 * \NOTE
	 * Other fields are already initialized with ZERO.
	 */
	set_last_result(NOTIFICATION_ERROR_NONE);
	return noti;
}

EXPORT_API notification_h notification_create(notification_type_e type)
{
	return _notification_create(type);
}

EXPORT_API notification_h notification_load_by_tag(const char *tag)
{
	return notification_load_by_tag_for_uid(tag, getuid());
}

EXPORT_API int notification_clone(notification_h noti, notification_h *clone)
{
	int i = 0;
	notification_h new_noti = NULL;

	if (noti == NULL || clone == NULL) {
		NOTIFICATION_ERR("INVALID PARAMETER.");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	new_noti = (notification_h) calloc(1, sizeof(struct _notification));
	if (new_noti == NULL) {
		NOTIFICATION_ERR("NO MEMORY : noti == NULL");
		return NOTIFICATION_ERROR_OUT_OF_MEMORY;
	}

	new_noti->type = noti->type;
	new_noti->layout = noti->layout;

	new_noti->group_id = noti->group_id;
	new_noti->internal_group_id = noti->internal_group_id;
	new_noti->priv_id = noti->priv_id;

	if (noti->caller_pkgname != NULL)
		new_noti->caller_pkgname = strdup(noti->caller_pkgname);
	else
		new_noti->caller_pkgname = notification_get_pkgname_by_pid();

	if (noti->launch_pkgname != NULL)
		new_noti->launch_pkgname = strdup(noti->launch_pkgname);
	else
		new_noti->launch_pkgname = NULL;

	if (noti->args != NULL)
		new_noti->args = bundle_dup(noti->args);
	else
		new_noti->args = NULL;

	if (noti->group_args != NULL)
		new_noti->group_args = bundle_dup(noti->group_args);
	else
		new_noti->group_args = NULL;

	if (noti->b_execute_option != NULL)
		new_noti->b_execute_option = bundle_dup(noti->b_execute_option);
	else
		new_noti->b_execute_option = NULL;

	if (noti->b_service_responding != NULL)
		new_noti->b_service_responding = bundle_dup(noti->b_service_responding);
	else
		new_noti->b_service_responding = NULL;

	if (noti->b_service_single_launch != NULL)
		new_noti->b_service_single_launch = bundle_dup(noti->b_service_single_launch);
	else
		new_noti->b_service_single_launch = NULL;

	if (noti->b_service_multi_launch != NULL)
		new_noti->b_service_multi_launch = bundle_dup(noti->b_service_multi_launch);
	else
		new_noti->b_service_multi_launch = NULL;

	for (i = 0; i < NOTIFICATION_EVENT_TYPE_MAX; i++) {
		if (noti->b_event_handler[i] != NULL)
			new_noti->b_event_handler[i] = bundle_dup(noti->b_event_handler[i]);
		else
			new_noti->b_event_handler[i] = NULL;
	}

	new_noti->sound_type = noti->sound_type;
	if (noti->sound_path != NULL)
		new_noti->sound_path = strdup(noti->sound_path);
	else
		new_noti->sound_path = NULL;

	new_noti->vibration_type = noti->vibration_type;
	if (noti->vibration_path != NULL)
		new_noti->vibration_path = strdup(noti->vibration_path);
	else
		new_noti->vibration_path = NULL;

	new_noti->led_operation = noti->led_operation;
	new_noti->led_argb = noti->led_argb;
	new_noti->led_on_ms = noti->led_on_ms;
	new_noti->led_off_ms = noti->led_off_ms;

	if (noti->domain != NULL)
		new_noti->domain = strdup(noti->domain);
	else
		new_noti->domain = NULL;

	if (noti->dir != NULL)
		new_noti->dir = strdup(noti->dir);
	else
		new_noti->dir = NULL;

	if (noti->b_text != NULL)
		new_noti->b_text = bundle_dup(noti->b_text);
	else
		new_noti->b_text = NULL;

	if (noti->b_key != NULL)
		new_noti->b_key = bundle_dup(noti->b_key);
	else
		new_noti->b_key = NULL;

	if (noti->tag != NULL)
		new_noti->tag = strdup(noti->tag);
	else
		new_noti->tag = NULL;

	if (noti->b_format_args != NULL)
		new_noti->b_format_args = bundle_dup(noti->b_format_args);
	else
		new_noti->b_format_args = NULL;

	new_noti->num_format_args = noti->num_format_args;

	if (noti->b_image_path != NULL)
		new_noti->b_image_path = bundle_dup(noti->b_image_path);
	else
		new_noti->b_image_path = NULL;

	new_noti->time = noti->time;
	new_noti->insert_time = noti->insert_time;

	new_noti->flags_for_property = noti->flags_for_property;
	new_noti->display_applist = noti->display_applist;

	new_noti->progress_size = noti->progress_size;
	new_noti->progress_percentage = noti->progress_percentage;

	new_noti->ongoing_flag = noti->ongoing_flag;
	new_noti->auto_remove = noti->auto_remove;
	new_noti->uid = noti->uid;

	new_noti->app_icon_path = NULL;
	new_noti->app_name = NULL;
	new_noti->temp_title = NULL;
	new_noti->temp_content = NULL;

	*clone = new_noti;

	return NOTIFICATION_ERROR_NONE;
}


EXPORT_API int notification_free(notification_h noti)
{
	int i = 0;
	if (noti == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	if (noti->caller_pkgname)
		free(noti->caller_pkgname);

	if (noti->launch_pkgname)
		free(noti->launch_pkgname);

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

	for (i = 0; i < NOTIFICATION_EVENT_TYPE_MAX; i++) {
		if (noti->b_event_handler[i] != NULL)
			bundle_free(noti->b_event_handler[i]);
	}

	if (noti->sound_path)
		free(noti->sound_path);

	if (noti->vibration_path)
		free(noti->vibration_path);

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

	if (noti->b_image_path)
		bundle_free(noti->b_image_path);

	if (noti->app_icon_path)
		free(noti->app_icon_path);

	if (noti->app_name)
		free(noti->app_name);

	if (noti->temp_title)
		free(noti->temp_title);

	if (noti->temp_content)
		free(noti->temp_content);

	if (noti->tag)
		free(noti->tag);

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
	if (noti == NULL || button_index < NOTIFICATION_BUTTON_1 || button_index > NOTIFICATION_BUTTON_6)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	return NOTIFICATION_ERROR_NONE;
}

EXPORT_API int notification_remove_button(notification_h noti, notification_button_index_e button_index)
{
	if (noti == NULL || button_index < NOTIFICATION_BUTTON_1 || button_index > NOTIFICATION_BUTTON_6)
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
		NOTIFICATION_ERR("Invalid parameter");
		return NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	return notification_ipc_request_save_as_template(noti, template_name);
}

EXPORT_API notification_h notification_create_from_template(const char *template_name)
{
	int ret = 0;
	notification_h noti = NULL;

	if (template_name == NULL) {
		NOTIFICATION_ERR("Invalid parameter");
		set_last_result(NOTIFICATION_ERROR_INVALID_PARAMETER);
		return NULL;
	}

	noti = (notification_h)calloc(1, sizeof(struct _notification));
	if (noti == NULL) {
		NOTIFICATION_ERR("Failed to alloc a new notification");
		set_last_result(NOTIFICATION_ERROR_OUT_OF_MEMORY);
		return NULL;
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
	char *pkgname;
	int do_not_disturb;
	int do_not_disturb_except;
	int allow_to_notify;

	if (state == NULL)
		return NOTIFICATION_ERROR_INVALID_PARAMETER;

	pkgname = notification_get_pkgname_by_pid();

	ret = notification_ipc_get_noti_block_state(pkgname, &do_not_disturb, &do_not_disturb_except, &allow_to_notify, getuid());

	if (ret != NOTIFICATION_ERROR_NONE) {
		if (pkgname)
			free(pkgname);
		return ret;
	}

	if (allow_to_notify) {
		*state = NOTIFICATION_BLOCK_STATE_ALLOWED;
		if (do_not_disturb && !do_not_disturb_except)
			*state = NOTIFICATION_BLOCK_STATE_DO_NOT_DISTURB;
	} else {
		*state = NOTIFICATION_BLOCK_STATE_BLOCKED;
	}

	if (pkgname)
		free(pkgname);

	return ret;
}
