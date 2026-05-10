/*
 * Post-install BAT ("Build Acceptance Test") for the notification
 * package. Purpose: prove the freshly installed libnotification.so
 * boots, creates a notification handle, accepts a text field, and
 * frees cleanly. NOT functional coverage -- that lives in
 * tests/unittests/ and tests/integration/.
 *
 * Exit codes (stable -- the GBS %check pre-amble greps for
 * "SMOKE_EXIT=" then this binary's own exit value):
 *
 *   0  every assertion held; library is alive.
 *   1  notification_create returned NULL.
 *   2  notification_set_text returned non-NONE.
 *   3  notification_get_text round-trip mismatch.
 *   4  notification_free returned non-NONE.
 *
 * Any non-zero exit fails %check and bounces the orchestrator to
 * /developer.
 */

#include <stdio.h>
#include <string.h>

#include <notification.h>
#include <notification_error.h>
#include <notification_type.h>

int main(void)
{
	notification_h n = notification_create(NOTIFICATION_TYPE_NOTI);
	if (!n) {
		fprintf(stderr, "smoke: notification_create returned NULL\n");
		return 1;
	}

	int rc = notification_set_text(n, NOTIFICATION_TEXT_TYPE_TITLE,
				       "smoke-title-bat", NULL,
				       NOTIFICATION_VARIABLE_TYPE_NONE);
	if (rc != NOTIFICATION_ERROR_NONE) {
		fprintf(stderr, "smoke: set_text rc=%d\n", rc);
		notification_free(n);
		return 2;
	}

	char *readback = NULL;
	rc = notification_get_text(n, NOTIFICATION_TEXT_TYPE_TITLE, &readback);
	if (rc != NOTIFICATION_ERROR_NONE || readback == NULL ||
	    strcmp(readback, "smoke-title-bat") != 0) {
		fprintf(stderr,
			"smoke: get_text round-trip failed (rc=%d, text=%s)\n",
			rc, readback ? readback : "<NULL>");
		notification_free(n);
		return 3;
	}

	rc = notification_free(n);
	if (rc != NOTIFICATION_ERROR_NONE) {
		fprintf(stderr, "smoke: free rc=%d\n", rc);
		return 4;
	}

	fprintf(stdout, "smoke: PASS\n");
	return 0;
}
