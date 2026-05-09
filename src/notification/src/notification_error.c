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

#include <string.h>
#include <gio/gio.h>
#include "notification_error.h"

static const GDBusErrorEntry dbus_error_entries[] = {
	{NOTIFICATION_ERROR_INVALID_PARAMETER, "org.freedesktop.Notification.Error.INVALID_PARAMETER"},
	{NOTIFICATION_ERROR_OUT_OF_MEMORY,     "org.freedesktop.Notification.Error.OUT_OF_MEMORY"},
	{NOTIFICATION_ERROR_IO_ERROR,          "org.freedesktop.Notification.Error.IO_ERROR"},
	{NOTIFICATION_ERROR_PERMISSION_DENIED, "org.freedesktop.Notification.Error.PERMISSION_DENIED"},
	{NOTIFICATION_ERROR_FROM_DB,           "org.freedesktop.Notification.Error.FROM_DB"},
	{NOTIFICATION_ERROR_ALREADY_EXIST_ID,  "org.freedesktop.Notification.Error.ALREADY_EXIST_ID"},
	{NOTIFICATION_ERROR_FROM_DBUS,         "org.freedesktop.Notification.Error.FROM_DBUS"},
	{NOTIFICATION_ERROR_NOT_EXIST_ID,      "org.freedesktop.Notification.Error.NOT_EXIST_ID"},
	{NOTIFICATION_ERROR_SERVICE_NOT_READY, "org.freedesktop.Notification.Error.SERVICE_NOT_READY"},
	{NOTIFICATION_ERROR_INVALID_OPERATION, "org.freedesktop.Notification.Error.INVALID_OPERATION"},
	{NOTIFICATION_ERROR_MAX_EXCEEDED,      "org.freedesktop.Notification.Error.MAX_EXCEEDED"},
};

#define NOTIFICATION_ERROR_QUARK "notification-error-quark"

EXPORT_API GQuark notification_error_quark(void)
{
	static volatile gsize quark_volatile = 0;
	static const char *domain_name = NULL;

	/* This is for preventing crash when notification api is used in ui-gadget */
	/* ui-gadget libraries can be unloaded when it is needed and the static string */
	/* parameter to g_dbus_error_register_error_domain may cause crash. */
	GQuark quark = g_quark_try_string(NOTIFICATION_ERROR_QUARK);

	if (quark == 0) {
		if (domain_name == NULL)
			domain_name = strdup(NOTIFICATION_ERROR_QUARK);
	} else {
		domain_name = NOTIFICATION_ERROR_QUARK;
	}

	g_dbus_error_register_error_domain(domain_name,
			&quark_volatile,
			dbus_error_entries,
			G_N_ELEMENTS(dbus_error_entries));
	return (GQuark)quark_volatile;
}

